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
#include "tokenize.h"

#include "astutils.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h"
#include "settings.h"
#include "standards.h"
#include "summaries.h"
#include "symboldatabase.h"
#include "templatesimplifier.h"
#include "timer.h"
#include "token.h"
#include "utils.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <simplecpp.h>

//---------------------------------------------------------------------------

namespace {
    // local struct used in setVarId
    // in order to store information about the scope
    struct VarIdScopeInfo {
        VarIdScopeInfo() = default;
        VarIdScopeInfo(bool isExecutable, bool isStructInit, bool isEnum, nonneg int startVarid)
            : isExecutable(isExecutable), isStructInit(isStructInit), isEnum(isEnum), startVarid(startVarid) {}

        const bool isExecutable{};
        const bool isStructInit{};
        const bool isEnum{};
        const nonneg int startVarid{};
    };
}

/** Return whether tok is the "{" that starts an enumerator list */
static bool isEnumStart(const Token* tok)
{
    if (!Token::simpleMatch(tok, "{"))
        return false;
    tok = tok->previous();
    while (tok && (!tok->isKeyword() || Token::isStandardType(tok->str())) && Token::Match(tok, "%name%|::|:"))
        tok = tok->previous();
    if (Token::simpleMatch(tok, "class"))
        tok = tok->previous();
    return Token::simpleMatch(tok, "enum");
}

template<typename T>
static void skipEnumBody(T *&tok)
{
    T *defStart = tok;
    while (Token::Match(defStart, "%name%|::|:"))
        defStart = defStart->next();
    if (defStart && defStart->str() == "{")
        tok = defStart->link()->next();
}

/**
 * is tok the start brace { of a class, struct, union, or enum
 */
static const Token* isClassStructUnionEnumStart(const Token* tok)
{
    if (!Token::Match(tok->previous(), "class|struct|union|enum|%name%|>|>> {"))
        return nullptr;
    const Token * tok2 = tok->previous();
    while (tok2 && !Token::Match(tok2, "class|struct|union|enum|{|}|)|;"))
        tok2 = tok2->previous();
    return (Token::Match(tok2, "class|struct|union|enum") && !Token::simpleMatch(tok2->tokAt(-1), "->")) ? tok2 : nullptr;
}

//---------------------------------------------------------------------------

Tokenizer::Tokenizer(TokenList tokenList, ErrorLogger &errorLogger) :
    list(std::move(tokenList)),
    mSettings(list.getSettings()),
    mErrorLogger(errorLogger),
    mTemplateSimplifier(new TemplateSimplifier(*this))
{}

Tokenizer::~Tokenizer()
{
    delete mSymbolDatabase;
    delete mTemplateSimplifier;
}


//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------

nonneg int Tokenizer::sizeOfType(const std::string& type) const
{
    const auto it = mTypeSize.find(type);
    if (it == mTypeSize.end()) {
        const Library::PodType* podtype = mSettings.library.podtype(type);
        if (!podtype)
            return 0;

        return podtype->size;
    }
    return it->second;
}

nonneg int Tokenizer::sizeOfType(const Token *type) const
{
    if (!type || type->str().empty())
        return 0;

    if (type->tokType() == Token::eString)
        return Token::getStrLength(type) + 1U;

    const auto it = mTypeSize.find(type->str());
    if (it == mTypeSize.end()) {
        const Library::PodType* podtype = mSettings.library.podtype(type->str());
        if (!podtype)
            return 0;

        return podtype->size;
    }
    if (type->isLong()) {
        if (type->str() == "double")
            return mSettings.platform.sizeof_long_double;
        if (type->str() == "long")
            return mSettings.platform.sizeof_long_long;
    }

    return it->second;
}
//---------------------------------------------------------------------------

// check if this statement is a duplicate definition
bool Tokenizer::duplicateTypedef(Token *&tokPtr, const Token *name, const Token *typeDef) const
{
    // check for an end of definition
    Token * tok = tokPtr;
    if (tok && Token::Match(tok->next(), ";|,|[|=|)|>|(|{")) {
        Token * end = tok->next();

        if (end->str() == "[") {
            if (!end->link())
                syntaxError(end); // invalid code
            end = end->link()->next();
        } else if (end->str() == ",") {
            // check for derived class
            if (Token::Match(tok->previous(), "public|private|protected"))
                return false;

            // find end of definition
            while (end && end->next() && !Token::Match(end->next(), ";|)|>")) {
                if (end->strAt(1) == "(")
                    end = end->linkAt(1);

                end = end ? end->next() : nullptr;
            }
            if (end)
                end = end->next();
        } else if (end->str() == "(") {
            if (startsWith(tok->strAt(-1), "operator"))
                // conversion operator
                return false;
            if (tok->strAt(-1) == "typedef")
                // typedef of function returning this type
                return false;
            if (Token::Match(tok->previous(), "public:|private:|protected:"))
                return false;
            if (tok->strAt(-1) == ">") {
                if (!Token::Match(tok->tokAt(-2), "%type%"))
                    return false;

                if (!Token::Match(tok->tokAt(-3), ",|<"))
                    return false;

                tokPtr = end->link();
                return true;
            }
        }

        if (end) {
            if (Token::simpleMatch(end, ") {")) { // function parameter ?
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const|struct")) {
                    // duplicate definition so skip entire function
                    tokPtr = end->linkAt(1);
                    return true;
                }
            } else if (end->str() == ">") { // template parameter ?
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const|volatile")) {
                    // duplicate definition so skip entire template
                    while (end && end->str() != "{")
                        end = end->next();
                    if (end) {
                        tokPtr = end->link();
                        return true;
                    }
                }
            } else {
                // look backwards
                if (Token::Match(tok->previous(), "typedef|}|>") ||
                    (end->str() == ";" && tok->strAt(-1) == ",") ||
                    (tok->strAt(-1) == "*" && tok->strAt(1) != "(") ||
                    (Token::Match(tok->previous(), "%type%") &&
                     (!Token::Match(tok->previous(), "return|new|const|friend|public|private|protected|throw|extern") &&
                      !Token::simpleMatch(tok->tokAt(-2), "friend class")))) {
                    // scan backwards for the end of the previous statement
                    while (tok && tok->previous() && !Token::Match(tok->previous(), ";|{")) {
                        if (tok->strAt(-1) == "}") {
                            tok = tok->linkAt(-1);
                        } else if (tok->strAt(-1) == "typedef") {
                            return true;
                        } else if (tok->strAt(-1) == "enum") {
                            return true;
                        } else if (tok->strAt(-1) == "struct") {
                            if (tok->strAt(-2) == "typedef" &&
                                tok->strAt(1) == "{" &&
                                typeDef->strAt(3) != "{") {
                                // declaration after forward declaration
                                return true;
                            }
                            if (tok->strAt(1) == "{")
                                return true;
                            if (Token::Match(tok->next(), ")|*"))
                                return true;
                            if (tok->strAt(1) == name->str())
                                return true;
                            if (tok->strAt(1) != ";")
                                return true;
                            return false;
                        } else if (tok->strAt(-1) == "union") {
                            return tok->strAt(1) != ";";
                        } else if (tok->isCpp() && tok->strAt(-1) == "class") {
                            return tok->strAt(1) != ";";
                        }
                        if (tok)
                            tok = tok->previous();
                    }

                    if (tokPtr->strAt(1) != "(" || !Token::Match(tokPtr->linkAt(1), ") .|(|["))
                        return true;
                }
            }
        }
    }

    return false;
}

void Tokenizer::unsupportedTypedef(const Token *tok) const
{
    if (!mSettings.debugwarnings)
        return;

    std::ostringstream str;
    const Token *tok1 = tok;
    int level = 0;
    while (tok) {
        if (level == 0 && tok->str() == ";")
            break;
        if (tok->str() == "{")
            ++level;
        else if (tok->str() == "}") {
            if (level == 0)
                break;
            --level;
        }

        if (tok != tok1)
            str << " ";
        str << tok->str();
        tok = tok->next();
    }
    if (tok)
        str << " ;";

    reportError(tok1, Severity::debug, "simplifyTypedef",
                "Failed to parse \'" + str.str() + "\'. The checking continues anyway.");
}

Token * Tokenizer::deleteInvalidTypedef(Token *typeDef)
{
    Token *tok = nullptr;

    // remove typedef but leave ;
    while (typeDef->next()) {
        if (typeDef->strAt(1) == ";") {
            typeDef->deleteNext();
            break;
        }
        if (typeDef->strAt(1) == "{")
            Token::eraseTokens(typeDef, typeDef->linkAt(1));
        else if (typeDef->strAt(1) == "}")
            break;
        typeDef->deleteNext();
    }

    if (typeDef != list.front()) {
        tok = typeDef->previous();
        tok->deleteNext();
    } else {
        list.front()->deleteThis();
        tok = list.front();
    }

    return tok;
}

namespace {
    struct Space {
        std::string className;
        const Token* bodyEnd{};  // for body contains typedef define
        const Token* bodyEnd2{}; // for body contains typedef using
        bool isNamespace{};
        std::set<std::string> recordTypes;
    };
}

static Token *splitDefinitionFromTypedef(Token *tok, nonneg int *unnamedCount)
{
    std::string name;
    std::set<std::string> qualifiers;

    while (Token::Match(tok->next(), "const|volatile")) {
        qualifiers.insert(tok->strAt(1));
        tok->deleteNext();
    }

    // skip "class|struct|union|enum"
    Token *tok1 = tok->tokAt(2);

    const bool hasName = Token::Match(tok1, "%name%");

    // skip name
    if (hasName) {
        name = tok1->str();
        tok1 = tok1->next();
    }

    // skip base classes if present
    if (tok1->str() == ":") {
        tok1 = tok1->next();
        while (tok1 && tok1->str() != "{")
            tok1 = tok1->next();
        if (!tok1)
            return nullptr;
    }

    // skip to end
    tok1 = tok1->link();

    if (!hasName) { // unnamed
        if (tok1->next()) {
            // use typedef name if available
            if (Token::Match(tok1->next(), "%type%"))
                name = tok1->strAt(1);
            else // create a unique name
                name = "Unnamed" + std::to_string((*unnamedCount)++);
            tok->next()->insertToken(name);
        } else
            return nullptr;
    }

    tok1->insertToken(";");
    tok1 = tok1->next();

    if (tok1->next() && tok1->strAt(1) == ";" && tok1->strAt(-1) == "}") {
        tok->deleteThis();
        tok1->deleteThis();
        return nullptr;
    }
    tok1->insertToken("typedef");
    tok1 = tok1->next();
    Token * tok3 = tok1;
    for (const std::string &qualifier : qualifiers) {
        tok1->insertToken(qualifier);
        tok1 = tok1->next();
    }
    tok1->insertToken(tok->strAt(1)); // struct, union or enum
    tok1 = tok1->next();
    tok1->insertToken(name);
    tok->deleteThis();
    tok = tok3;

    return tok;
}

/* This function is called when processing function related typedefs.
 * If simplifyTypedef generates an "Internal Error" message and the
 * code that generated it deals in some way with functions, then this
 * function will probably need to be extended to handle a new function
 * related pattern */
const Token *Tokenizer::processFunc(const Token *tok2, bool inOperator) const
{
    if (tok2->next() && tok2->strAt(1) != ")" &&
        tok2->strAt(1) != ",") {
        // skip over tokens for some types of canonicalization
        if (Token::Match(tok2->next(), "( * %type% ) ("))
            tok2 = tok2->linkAt(5);
        else if (Token::Match(tok2->next(), "* ( * %type% ) ("))
            tok2 = tok2->linkAt(6);
        else if (Token::Match(tok2->next(), "* ( * %type% ) ;"))
            tok2 = tok2->tokAt(5);
        else if (Token::Match(tok2->next(), "* ( %type% [") &&
                 Token::Match(tok2->linkAt(4), "] ) ;|="))
            tok2 = tok2->linkAt(4)->next();
        else if (Token::Match(tok2->next(), "* ( * %type% ("))
            tok2 = tok2->linkAt(5)->next();
        else if (Token::simpleMatch(tok2->next(), "* [") &&
                 Token::simpleMatch(tok2->linkAt(2), "] ;"))
            tok2 = tok2->next();
        else {
            if (tok2->strAt(1) == "(")
                tok2 = tok2->linkAt(1);
            else if (!inOperator && !Token::Match(tok2->next(), "[|>|;")) {
                tok2 = tok2->next();

                while (Token::Match(tok2, "*|&") &&
                       !Token::Match(tok2->next(), "[)>,]"))
                    tok2 = tok2->next();

                // skip over namespace
                while (Token::Match(tok2, "%name% ::"))
                    tok2 = tok2->tokAt(2);

                if (Token::simpleMatch(tok2, "const"))
                    tok2 = tok2->next();

                if (!tok2)
                    return nullptr;

                if (tok2->str() == "(" &&
                    tok2->link()->next() &&
                    tok2->link()->strAt(1) == "(") {
                    tok2 = tok2->link();

                    if (tok2->strAt(1) == "(")
                        tok2 = tok2->linkAt(1);
                }

                // skip over typedef parameter
                if (tok2->next() && tok2->strAt(1) == "(") {
                    tok2 = tok2->linkAt(1);
                    if (!tok2->next())
                        syntaxError(tok2);

                    if (tok2->strAt(1) == "(")
                        tok2 = tok2->linkAt(1);
                }
            }
        }
    }
    return tok2;
}

Token *Tokenizer::processFunc(Token *tok2, bool inOperator)
{
    return const_cast<Token*>(processFunc(const_cast<const Token*>(tok2), inOperator));
}

void Tokenizer::simplifyTypedefLHS()
{
    if (!list.front())
        return;

    for (Token* tok = list.front()->next(); tok; tok = tok->next()) {
        if (tok->str() == "typedef") {
            bool doSimplify = !Token::Match(tok->previous(), ";|{|}|:|public:|private:|protected:");
            if (doSimplify && Token::simpleMatch(tok->previous(), ")") && Token::Match(tok->linkAt(-1)->previous(), "if|for|while"))
                doSimplify = false;
            bool haveStart = false;
            Token* start{};
            if (!doSimplify && Token::simpleMatch(tok->previous(), "}")) {
                start = tok->linkAt(-1)->previous();
                while (Token::Match(start, "%name%")) {
                    if (Token::Match(start, "class|struct|union|enum")) {
                        start = start->previous();
                        doSimplify = true;
                        haveStart = true;
                        break;
                    }
                    start = start->previous();
                }
            }
            if (doSimplify) {
                if (!haveStart) {
                    start = tok;
                    while (start && !Token::Match(start, "[;{}]"))
                        start = start->previous();
                }
                if (start)
                    start = start->next();
                else
                    start = list.front();
                start->insertTokenBefore(tok->str());
                tok->deleteThis();
            }
        }
    }
}

namespace {
    class TypedefSimplifier {
    private:
        Token* mTypedefToken;  // The "typedef" token
        Token* mEndToken{nullptr};  // Semicolon
        std::pair<Token*, Token*> mRangeType;
        std::pair<Token*, Token*> mRangeTypeQualifiers;
        std::pair<Token*, Token*> mRangeAfterVar;
        Token* mNameToken{nullptr};
        bool mFail = false;
        bool mReplaceFailed = false;
        bool mUsed = false;

    public:
        explicit TypedefSimplifier(Token* typedefToken) : mTypedefToken(typedefToken) {
            Token* start = typedefToken->next();
            if (Token::simpleMatch(start, "typename"))
                start = start->next();

            // TODO handle unnamed structs etc
            if (Token::Match(start, "const| enum|struct|union|class %name%| {")) {
                const std::pair<Token*, Token*> rangeBefore(start, Token::findsimplematch(start, "{"));

                // find typedef name token
                Token* nameToken = rangeBefore.second->link()->next();
                while (Token::Match(nameToken, "%name%|* %name%|*"))
                    nameToken = nameToken->next();
                const std::pair<Token*, Token*> rangeQualifiers(rangeBefore.second->link()->next(), nameToken);

                if (Token::Match(nameToken, "%name% ;")) {
                    if (Token::Match(rangeBefore.second->previous(), "enum|struct|union|class {"))
                        rangeBefore.second->previous()->insertToken(nameToken->str());
                    mRangeType = rangeBefore;
                    mRangeTypeQualifiers = rangeQualifiers;
                    Token* typeName = rangeBefore.second->previous();
                    if (typeName->isKeyword()) {
                        // TODO typeName->insertToken("T:" + std::to_string(num++));
                        typeName->insertToken(nameToken->str());
                    }
                    mNameToken = nameToken;
                    mEndToken = nameToken->next();
                    return;
                }
            }

            for (Token* type = start; Token::Match(type, "%name%|*|&|&&"); type = type->next()) {
                if (type != start && Token::Match(type, "%name% ;") && !type->isStandardType()) {
                    mRangeType.first = start;
                    mRangeType.second = type;
                    mNameToken = type;
                    mEndToken = mNameToken->next();
                    return;
                }
                if (type != start && Token::Match(type, "%name% [")) {
                    Token* end = type->linkAt(1);
                    while (Token::simpleMatch(end, "] ["))
                        end = end->linkAt(1);
                    if (!Token::simpleMatch(end, "] ;"))
                        break;
                    mRangeType.first = start;
                    mRangeType.second = type;
                    mNameToken = type;
                    mEndToken = end->next();
                    mRangeAfterVar.first = mNameToken->next();
                    mRangeAfterVar.second = mEndToken;
                    return;
                }
                if (Token::Match(type->next(), "( * const| %name% ) (") && Token::simpleMatch(type->linkAt(1)->linkAt(1), ") ;")) {
                    mNameToken = type->linkAt(1)->previous();
                    mEndToken = type->linkAt(1)->linkAt(1)->next();
                    mRangeType.first = start;
                    mRangeType.second = mNameToken;
                    mRangeAfterVar.first = mNameToken->next();
                    mRangeAfterVar.second = mEndToken;
                    return;
                }
                if (type != start && Token::Match(type, "%name% ( !!(") && Token::simpleMatch(type->linkAt(1), ") ;") && !type->isStandardType()) {
                    mNameToken = type;
                    mEndToken = type->linkAt(1)->next();
                    mRangeType.first = start;
                    mRangeType.second = type;
                    mRangeAfterVar.first = mNameToken->next();
                    mRangeAfterVar.second = mEndToken;
                    return;
                }
            }
            // TODO: handle all typedefs
            if ((false)) // NOLINT(readability-redundant-parentheses)
                printTypedef(typedefToken, std::cout);
            mFail = true;
        }

        const Token* getTypedefToken() const {
            return mTypedefToken;
        }

        bool isUsed() const {
            return mUsed;
        }

        bool isInvalidConstFunctionType(const std::map<std::string, TypedefSimplifier>& m) const {
            if (!Token::Match(mTypedefToken, "typedef const %name% %name% ;"))
                return false;
            const auto it = m.find(mTypedefToken->strAt(2));
            if (it == m.end())
                return false;
            return Token::Match(it->second.mNameToken, "%name% (");
        }

        bool fail() const {
            return mFail;
        }

        bool replaceFailed() const {
            return mReplaceFailed;
        }

        bool isStructEtc() const {
            return mRangeType.second && mRangeType.second->str() == "{";
        }

        std::string name() const {
            return mNameToken ? mNameToken->str() : "";
        }

        /**
         * @throws InternalError thrown if simplification failed
         */
        void replace(Token* tok, const std::string &originalname) {
            if (tok == mNameToken)
                return;

            mUsed = true;
            const bool isFunctionPointer = Tokenizer::isFunctionPointer(mNameToken);

            // Special handling for T(...) when T is a pointer
            if (Token::Match(tok, "%name% [({]") && !isFunctionPointer && !Token::simpleMatch(tok->linkAt(1), ") (")) {
                bool pointerType = false;
                for (const Token* type = mRangeType.first; type != mRangeType.second; type = type->next()) {
                    if (type->str() == "*" || type->str() == "&") {
                        pointerType = true;
                        break;
                    }
                }
                for (const Token* type = mRangeTypeQualifiers.first; type != mRangeTypeQualifiers.second; type = type->next()) {
                    if (type->str() == "*" || type->str() == "&") {
                        pointerType = true;
                        break;
                    }
                }
                if (pointerType) {
                    tok->tokAt(1)->str("(");
                    tok->linkAt(1)->str(")");
                    if (tok->linkAt(1) == tok->tokAt(2)) { // T() or T{}
                        tok->deleteThis();
                        tok->next()->insertToken("0");
                        Token* tok2 = insertTokens(tok, mRangeType);
                        insertTokens(tok2, mRangeTypeQualifiers);
                    }
                    else { // functional-style cast
                        tok->originalName(originalname);
                        tok->isSimplifiedTypedef(true);
                        tok->str("(");
                        Token* tok2 = insertTokens(tok, mRangeType);
                        tok2 = insertTokens(tok2, mRangeTypeQualifiers);
                        Token* tok3 = tok2->insertToken(")");
                        Token::createMutualLinks(tok, tok3);
                        tok->insertTokenBefore("(");
                        tok3 = tok3->linkAt(1);
                        tok3 = tok3->insertToken(")");
                        Token::createMutualLinks(tok->tokAt(-1), tok3);
                    }
                    return;
                }
            }

            // Special handling of function pointer cast
            if (isFunctionPointer && isCast(tok->previous())) {
                tok->insertToken("*");
                Token* const tok_1 = insertTokens(tok, std::pair<Token*, Token*>(mRangeType.first, mNameToken->linkAt(1)));
                tok_1->originalName(originalname);
                tok->deleteThis();
                return;
            }

            // Inherited type => skip "struct" / "class"
            if (Token::Match(mRangeType.first, "const| struct|class %name% {") && Token::Match(tok->previous(), "public|protected|private|<")) {
                tok->originalName(originalname);
                tok->str(mRangeType.second->strAt(-1));
                return;
            }

            if (Token::Match(tok, "%name% ::")) {
                if (Token::Match(mRangeType.first, "const| struct|class|union|enum %name% %name%|{") ||
                    Token::Match(mRangeType.first, "%name% %name% ;")) {
                    tok->originalName(originalname);
                    tok->str(mRangeType.second->strAt(-1));
                } else {
                    mReplaceFailed = true;
                }
                return;
            }

            // pointer => move "const"
            if (Token::simpleMatch(tok->previous(), "const")) {
                bool pointerType = false;
                for (const Token* type = mRangeType.first; type != mRangeType.second; type = type->next()) {
                    if (type->str() == "*") {
                        pointerType = true;
                        break;
                    }
                }
                if (pointerType) {
                    tok->insertToken("const");
                    tok->next()->column(tok->column());
                    if (!tok->previous()->getMacroName().empty())
                        tok->next()->setMacroName(tok->previous()->getMacroName());
                    tok->deletePrevious();
                }
            }

            // Do not duplicate class/struct/enum/union
            if (Token::Match(tok->previous(), "enum|union|struct|class")) {
                bool found = false;
                const std::string &kw = tok->strAt(-1);
                for (const Token* type = mRangeType.first; type != mRangeType.second; type = type->next()) {
                    if (type->str() == kw) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    tok->deletePrevious();
                else {
                    mReplaceFailed = true;
                    return;
                }
            }

            // don't add class|struct|union in inheritance list
            auto rangeType = mRangeType;
            if (Token::Match(tok->previous(), "public|private|protected")) {
                while (Token::Match(rangeType.first, "const|class|struct|union"))
                    rangeType.first = rangeType.first->next();
            }

            Token* const tok2 = insertTokens(tok, rangeType);
            Token* const tok3 = insertTokens(tok2, mRangeTypeQualifiers);

            tok2->originalName(originalname);
            tok3->originalName(originalname);
            Token *after = tok3;
            while (Token::Match(after, "%name%|*|&|&&|::"))
                after = after->next();
            if (Token::Match(mNameToken, "%name% (") && Token::Match(tok3->next(), "*|const")) {
                while (Token::Match(after, "(|["))
                    after = after->link()->next();
                if (after) {
                    tok3->insertToken("(");
                    after->previous()->insertToken(")");
                    Token::createMutualLinks(tok3->next(), after->previous());
                }
            }
            if (!after) {
                mReplaceFailed = true;
                return;
            }

            bool useAfterVarRange = true;
            if (Token::simpleMatch(mRangeAfterVar.first, "[")) {
                if (Token::Match(after->previous(), "%name% ( !!*")) {
                    useAfterVarRange = false;
                    // Function return type => replace array with "*"
                    for (const Token* a = mRangeAfterVar.first; Token::simpleMatch(a, "["); a = a->link()->next())
                        tok3->insertToken("*");
                } else if (Token::Match(after->previous(), "%name% ( * %name% ) [")) {
                    after = after->linkAt(4)->next();
                } else {
                    Token* prev = after->previous();
                    if (prev->isName() && prev != tok3)
                        prev = prev->previous();
                    if (Token::Match(prev, "*|&|&&") && prev != tok3) {
                        while (Token::Match(prev, "*|&|&&") && prev != tok3)
                            prev = prev->previous();
                        prev->insertToken("(");
                        after->previous()->insertToken(")");
                    }
                }
            }

            if (isFunctionPointer) {
                if (Token::Match(after, "( * %name% ) ("))
                    after = after->link()->linkAt(1)->next();
                else if (after->str() == "(") {
                    useAfterVarRange = false;
                    if (Token::simpleMatch(tok3->previous(), "( *"))
                        tok3->deletePrevious();
                    else if (Token::simpleMatch(tok3->tokAt(-2), "( * const"))
                        tok3->tokAt(-1)->deletePrevious();
                }
                else if (after->str() == "[") {
                    while (after && after->str() == "[")
                        after = after->link()->next();
                }
            }
            else {
                while (Token::simpleMatch(after, "["))
                    after = after->link()->next();
            }

            if (!after)
                throw InternalError(tok, "Failed to simplify typedef. Is the code valid?");

            Token* const tok4 = useAfterVarRange ? insertTokens(after->previous(), mRangeAfterVar)->next() : tok3->next();

            if (tok->next() == tok4)
                throw InternalError(tok, "Failed to simplify typedef. Is the code valid?");

            tok->deleteThis();

            // Unsplit variable declarations
            if (tok4 && tok4->isSplittedVarDeclEq() &&
                ((tok4->isCpp() && Token::Match(tok4->tokAt(-2), "&|&& %name% ;")) || Token::Match(tok4->previous(), "] ; %name% = {"))) {
                tok4->deleteNext();
                tok4->deleteThis();
            }

            // Set links
            std::stack<Token*> brackets;
            for (; tok != tok4; tok = tok->next()) {
                if (Token::Match(tok, "[{([]"))
                    brackets.push(tok);
                else if (Token::Match(tok, "[})]]")) {
                    Token::createMutualLinks(brackets.top(), tok);
                    brackets.pop();
                }
            }
        }

        void removeDeclaration() {
            if (Token::simpleMatch(mRangeType.second, "{")) {
                while (Token::Match(mTypedefToken, "typedef|const"))
                    mTypedefToken->deleteThis();
                Token::eraseTokens(mRangeType.second->link(), mEndToken);
            } else {
                Token::eraseTokens(mTypedefToken, mEndToken);
                mTypedefToken->deleteThis();
            }
        }

        static int canReplaceStatic(const Token* tok) {
            if (!Token::Match(tok, "%name% %name%|*|&|&&|;|(|)|,|::")) {
                if (Token::Match(tok->previous(), "( %name% =") && Token::Match(tok->linkAt(-1), ") %name%|{") && !tok->tokAt(-2)->isKeyword())
                    return true;
                if (Token::Match(tok->previous(), ", %name% ="))
                    return true;
                if (Token::Match(tok->previous(), "new %name% ["))
                    return true;
                if (Token::Match(tok->previous(), "< %name%") && tok->previous()->findClosingBracket())
                    return true;
                if (Token::Match(tok->previous(), ", %name% >|>>")) {
                    for (const Token* prev = tok->previous(); prev; prev = prev->previous()) {
                        if (Token::Match(prev, "[;{}(]"))
                            break;
                        if (prev->str() == "<" && prev->findClosingBracket() == tok->next())
                            return true;
                        if (prev->str() == ")")
                            prev = prev->link();
                    }
                    return true;
                }
                if (Token::Match(tok->previous(), "public|protected|private"))
                    return true;
                if (Token::Match(tok->previous(), ", %name% :")) {
                    bool isGeneric = false;
                    for (; tok; tok = tok->previous()) {
                        if (Token::Match(tok, ")|]"))
                            tok = tok->link();
                        else if (Token::Match(tok, "[;{}(]")) {
                            isGeneric = Token::simpleMatch(tok->previous(), "_Generic (");
                            break;
                        }
                    }
                    return isGeneric;
                }
                return false;
            }
            return -1;
        }

        bool canReplace(const Token* tok) {
            if (mNameToken == tok)
                return false;
            if (!Token::Match(tok->previous(), "%name%|;|{|}|(|,|<") && !Token::Match(tok->previous(), "!!. %name% ("))
                return false;
            {
                const int res = canReplaceStatic(tok);
                if (res == 0 || res == 1)
                    return res != 0;
            }
            if (Token::Match(tok->previous(), "%name%") && !tok->previous()->isKeyword())
                return false;
            if (Token::simpleMatch(tok->next(), "(") && Token::Match(tok->linkAt(1), ") %name%|{"))
                return false;
            if (Token::Match(tok->previous(), "struct|union|class|enum %name% %name%") &&
                Token::simpleMatch(mRangeType.second, "{") &&
                tok->str() != mRangeType.second->strAt(-1))
                return true;
            if (Token::Match(tok->previous(), "; %name% ;"))
                return false;
            if (Token::Match(tok->previous(), "<|, %name% * ,|>"))
                return true;
            for (const Token* after = tok->next(); after; after = after->next()) {
                if (Token::Match(after, "%name%|::|&|*|&&"))
                    continue;
                if (after->str() == "<" && after->link())
                    break;
                if (after->isNumber())
                    return false;
                if (after->isComparisonOp() || after->isArithmeticalOp())
                    return false;
                break;
            }
            for (const Token* before = tok->previous(); before; before = before->previous()) {
                if (Token::Match(before, "[+-*/&|~!]"))
                    return false;
                if (Token::Match(before, "struct|union|class|enum") || before->isStandardType())
                    return false;
                if (before->str() == "::")
                    return false;
                if (before->isName())
                    continue;
                break;
            }
            return true;
        }

        Token* endToken() const {
            return mEndToken;
        }

        Token* nameToken() const {
            return mNameToken;
        }

    private:
        static bool isCast(const Token* tok) {
            if (Token::Match(tok, "( %name% ) (|%name%|%num%"))
                return !tok->tokAt(3)->isKeyword();
            if (Token::Match(tok, "< %name% > (") && tok->previous() && endsWith(tok->strAt(-1), "_cast", 5))
                return true;
            return false;
        }

        static Token* insertTokens(Token* to, std::pair<Token*,Token*> range) {
            for (const Token* from = range.first; from != range.second; from = from->next()) {
                to->insertToken(from->str());
                to->next()->column(to->column());
                to = to->next();
                to->isSimplifiedTypedef(true);
                to->isExternC(from->isExternC());
            }
            return to;
        }

        static void printTypedef(const Token *tok, std::ostream& out) {
            int indent = 0;
            while (tok && (indent > 0 || tok->str() != ";")) {
                if (tok->str() == "{")
                    ++indent;
                else if (tok->str() == "}")
                    --indent;
                out << " " << tok->str();
                tok = tok->next();
            }
            out << "\n";
        }
    };
}

bool Tokenizer::isFunctionPointer(const Token* tok) {
    return Token::Match(tok, "%name% ) (");
}

static bool matchCurrentType(const std::string& typeStr, const std::map<int, std::string>& types)
{
    return std::any_of(types.begin(), types.end(), [&](const std::pair<int, std::string>& element) {
        return typeStr == element.second;
    });
}

void Tokenizer::simplifyTypedef()
{
    // Simplify global typedefs that are not redefined with the fast 1-pass simplification.
    // Then use the slower old typedef simplification.
    std::map<std::string, std::set<std::string>> numberOfTypedefs;
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "typedef") {
            TypedefSimplifier ts(tok);
            if (ts.fail() || !ts.nameToken())
                continue;
            std::string existing_data_type;
            for (const Token* t = ts.getTypedefToken()->next(); t != ts.endToken(); t = t->next()) {
                if (t != ts.nameToken())
                    existing_data_type += t->str() + " ";
            }
            numberOfTypedefs[ts.name()].emplace(std::move(existing_data_type));
            continue;
        }
    }

    int indentlevel = 0;
    std::map<std::string, TypedefSimplifier> typedefs;
    std::map<int, std::string> inType;
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName()) {
            if (tok->str()[0] == '{') {
                ++indentlevel;
                if (const Token* typeStart = isClassStructUnionEnumStart(tok)) {
                    inType.emplace(indentlevel, typeStart->strAt(1));
                }
            }
            else if (tok->str()[0] == '}') {
                inType.erase(indentlevel);
                --indentlevel;
            }
            continue;
        }

        if (indentlevel == 0 && tok->str() == "typedef") {
            TypedefSimplifier ts(tok);
            if (!ts.fail() && numberOfTypedefs[ts.name()].size() == 1) {
                if (mSettings.severity.isEnabled(Severity::portability) && ts.isInvalidConstFunctionType(typedefs))
                    invalidConstFunctionTypeError(tok->next());
                typedefs.emplace(ts.name(), ts);
                if (!ts.isStructEtc())
                    tok = ts.endToken();
            }
            continue;
        }

        auto it = typedefs.find(tok->str());
        if (it != typedefs.end() && it->second.canReplace(tok) && !matchCurrentType(tok->str(), inType)) {
            std::set<std::string> r;
            std::string originalname;
            while (it != typedefs.end() && r.insert(tok->str()).second) {
                if (originalname.empty())
                    originalname = tok->str();
                it->second.replace(tok, originalname);
                it = typedefs.find(tok->str());
            }
        } else if (tok->str() == "enum") {
            while (Token::Match(tok, "%name%|:|::"))
                tok = tok->next();
            if (!tok)
                break;
            if (tok->str() == "{")
                tok = tok->link();
        }
    }

    if (!typedefs.empty())
    {
        // remove typedefs
        for (auto &t: typedefs) {
            if (t.second.replaceFailed()) {
                syntaxError(t.second.getTypedefToken());
            } else {
                const Token* const typedefToken = t.second.getTypedefToken();
                TypedefInfo typedefInfo;
                typedefInfo.name = t.second.name();
                typedefInfo.filename = list.file(typedefToken);
                typedefInfo.lineNumber = typedefToken->linenr();
                typedefInfo.column = typedefToken->column();
                if (Token::Match(typedefToken->next(), "struct|enum|class|union %name% {") && typedefToken->strAt(2) == typedefInfo.name) {
                    typedefInfo.tagLine = typedefToken->tokAt(2)->linenr();
                    typedefInfo.tagColumn = typedefToken->tokAt(2)->column();
                }
                typedefInfo.used = t.second.isUsed();
                typedefInfo.isFunctionPointer = isFunctionPointer(t.second.nameToken());
                if (typedefInfo.isFunctionPointer) {
                    const Token* tok = typedefToken;
                    while (tok != t.second.endToken()) {
                        TypedefToken ttok;
                        ttok.name = tok->str();
                        ttok.lineNumber = tok->linenr();
                        ttok.column = tok->column();
                        typedefInfo.typedefInfoTokens.emplace_back(ttok);
                        tok = tok->next();
                    }
                }
                mTypedefInfo.push_back(std::move(typedefInfo));

                t.second.removeDeclaration();
            }
        }

        while (Token::Match(list.front(), "; %any%"))
            list.front()->deleteThis();
    }

    validate();

    simplifyTypedefCpp();
}

static Token* simplifyTypedefCopyTokens(Token* to, const Token* fromStart, const Token* toEnd, const Token* location) {
    Token* ret = TokenList::copyTokens(to, fromStart, toEnd);
    for (Token* tok = to->next(); tok != ret->next(); tok = tok->next()) {
        tok->linenr(location->linenr());
        tok->column(location->column());
        tok->isSimplifiedTypedef(true);
    }
    return ret;
}

static Token* simplifyTypedefInsertToken(Token* tok, const std::string& str, const Token* location) {
    tok = tok->insertToken(str);
    tok->linenr(location->linenr());
    tok->column(location->column());
    tok->isSimplifiedTypedef(true);
    return tok;
}

// TODO: rename - it is not C++ specific
void Tokenizer::simplifyTypedefCpp()
{
    const bool cpp = isCPP();
    bool isNamespace = false;
    std::string className, fullClassName;
    bool hasClass = false;
    bool goback = false;

    // add global namespace
    std::vector<Space> spaceInfo(1);

    const std::time_t maxTime = mSettings.typedefMaxTime > 0 ? std::time(nullptr) + mSettings.typedefMaxTime: 0;
    ProgressReporter progressReporter(mErrorLogger, mSettings.reportProgress, list.getSourceFilePath(), "Tokenize (typedef)");

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        progressReporter.report(tok->progressValue());

        if (Settings::terminated())
            return;

        if (maxTime > 0 && std::time(nullptr) > maxTime) {
            if (mSettings.debugwarnings) {
                ErrorMessage::FileLocation loc(list.getFiles()[0], 0, 0);
                ErrorMessage errmsg({std::move(loc)},
                                    "",
                                    Severity::debug,
                                    "Typedef simplification instantiation maximum time exceeded",
                                    "typedefMaxTime",
                                    Certainty::normal);
                mErrorLogger.reportErr(errmsg);
            }
            return;
        }

        if (goback) {
            //jump back once, see the comment at the end of the function
            goback = false;
            tok = tok->previous();
        }

        if (tok->str() != "typedef") {
            if (Token::simpleMatch(tok, "( typedef")) {
                // Skip typedefs inside parentheses (#2453 and #4002)
                tok = tok->next();
            } else if (Token::Match(tok, "class|struct|namespace %any%") &&
                       (!tok->previous() || tok->strAt(-1) != "enum")) {
                isNamespace = (tok->str() == "namespace");
                hasClass = true;
                className = tok->strAt(1);
                const Token *tok1 = tok->next();
                fullClassName = className;
                while (Token::Match(tok1, "%name% :: %name%")) {
                    tok1 = tok1->tokAt(2);
                    fullClassName += " :: " + tok1->str();
                }
            } else if (hasClass && tok->str() == ";") {
                hasClass = false;
            } else if (hasClass && tok->str() == "{") {
                if (!isNamespace)
                    spaceInfo.back().recordTypes.insert(fullClassName);

                Space info;
                info.isNamespace = isNamespace;
                info.className = className;
                info.bodyEnd = tok->link();
                info.bodyEnd2 = tok->link();
                spaceInfo.push_back(std::move(info));

                hasClass = false;
            } else if (spaceInfo.size() > 1 && tok->str() == "}" && spaceInfo.back().bodyEnd == tok) {
                spaceInfo.pop_back();
            }
            continue;
        }

        // pull struct, union, enum or class definition out of typedef
        // use typedef name for unnamed struct, union, enum or class
        const Token* tokClass = tok->next();
        while (Token::Match(tokClass, "const|volatile"))
            tokClass = tokClass->next();
        if (Token::Match(tokClass, "struct|enum|union|class %type%| {|:")) {
            Token *tok1 = splitDefinitionFromTypedef(tok, &mUnnamedCount);
            if (!tok1)
                continue;
            tok = tok1;
        }

        /** @todo add support for union */
        if (Token::Match(tok->next(), "enum %type% %type% ;") && tok->strAt(2) == tok->strAt(3)) {
            tok->deleteNext(3);
            tok->deleteThis();
            if (tok->next())
                tok->deleteThis();
            //now the next token to process is 'tok', not 'tok->next()';
            goback = true;
            continue;
        }

        Token *typeName;
        Token *typeStart = nullptr;
        Token *typeEnd = nullptr;
        Token *argStart = nullptr;
        Token *argEnd = nullptr;
        Token *arrayStart = nullptr;
        Token *arrayEnd = nullptr;
        Token *specStart = nullptr;
        Token *specEnd = nullptr;
        Token *typeDef = tok;
        Token *argFuncRetStart = nullptr;
        Token *argFuncRetEnd = nullptr;
        Token *funcStart = nullptr;
        Token *funcEnd = nullptr;
        Token *tokOffset = tok->next();
        bool function = false;
        bool functionPtr = false;
        bool functionRetFuncPtr = false;
        bool functionPtrRetFuncPtr = false;
        bool ptrToArray = false;
        bool refToArray = false;
        bool ptrMember = false;
        bool typeOf = false;
        const Token *namespaceStart = nullptr;
        Token *namespaceEnd = nullptr;

        // check for invalid input
        if (!tokOffset || tokOffset->isControlFlowKeyword())
            syntaxError(tok);

        if (tokOffset->str() == "::") {
            typeStart = tokOffset;
            tokOffset = tokOffset->next();

            while (Token::Match(tokOffset, "%type% ::"))
                tokOffset = tokOffset->tokAt(2);

            typeEnd = tokOffset;

            if (Token::Match(tokOffset, "%type%"))
                tokOffset = tokOffset->next();
        } else if (Token::Match(tokOffset, "%type% ::")) {
            typeStart = tokOffset;

            do {
                tokOffset = tokOffset->tokAt(2);
            } while (Token::Match(tokOffset, "%type% ::"));

            typeEnd = tokOffset;

            if (Token::Match(tokOffset, "%type%"))
                tokOffset = tokOffset->next();
        } else if (Token::Match(tokOffset, "%type%")) {
            typeStart = tokOffset;

            while (Token::Match(tokOffset, "const|struct|enum %type%") ||
                   (tokOffset->next() && tokOffset->next()->isStandardType() && !Token::Match(tokOffset->next(), "%name% ;")))
                tokOffset = tokOffset->next();

            typeEnd = tokOffset;
            if (!Token::Match(tokOffset->next(), "%name% ;"))
                tokOffset = tokOffset->next();

            while (Token::Match(tokOffset, "%type%") &&
                   (tokOffset->isStandardType() || Token::Match(tokOffset, "unsigned|signed")) &&
                   !Token::Match(tokOffset->next(), "%name% ;")) {
                typeEnd = tokOffset;
                tokOffset = tokOffset->next();
            }

            bool atEnd = false;
            while (!atEnd) {
                if (tokOffset && tokOffset->str() == "::") {
                    typeEnd = tokOffset;
                    tokOffset = tokOffset->next();
                }

                if (Token::Match(tokOffset, "%type%") &&
                    tokOffset->next() && !Token::Match(tokOffset->next(), "[|;|,|(")) {
                    typeEnd = tokOffset;
                    tokOffset = tokOffset->next();
                } else if (Token::simpleMatch(tokOffset, "const (")) {
                    typeEnd = tokOffset;
                    tokOffset = tokOffset->next();
                    atEnd = true;
                } else
                    atEnd = true;
            }
        } else
            continue; // invalid input

        // check for invalid input
        if (!tokOffset)
            syntaxError(tok);

        // check for template
        if (!isC() && tokOffset->str() == "<") {
            typeEnd = tokOffset->findClosingBracket();

            while (typeEnd && Token::Match(typeEnd->next(), ":: %type%"))
                typeEnd = typeEnd->tokAt(2);

            if (!typeEnd) {
                // internal error
                return;
            }

            while (Token::Match(typeEnd->next(), "const|volatile"))
                typeEnd = typeEnd->next();

            tok = typeEnd;
            tokOffset = tok->next();
        }

        std::list<std::string> pointers;
        // check for pointers and references
        while (Token::Match(tokOffset, "*|&|&&|const")) {
            pointers.push_back(tokOffset->str());
            tokOffset = tokOffset->next();
        }

        // check for invalid input
        if (!tokOffset)
            syntaxError(tok);

        if (tokOffset->isName() && !tokOffset->isKeyword()) {
            // found the type name
            typeName = tokOffset;
            tokOffset = tokOffset->next();

            // check for array
            while (tokOffset && tokOffset->str() == "[") {
                if (!arrayStart)
                    arrayStart = tokOffset;
                arrayEnd = tokOffset->link();
                tokOffset = arrayEnd->next();
            }

            // check for end or another
            if (Token::Match(tokOffset, ";|,"))
                tok = tokOffset;

            // or a function typedef
            else if (tokOffset && tokOffset->str() == "(") {
                Token *tokOffset2 = nullptr;
                if (Token::Match(tokOffset, "( *|%name%")) {
                    tokOffset2 = tokOffset->next();
                    if (tokOffset2->str() == "typename")
                        tokOffset2 = tokOffset2->next();
                    while (Token::Match(tokOffset2, "%type% ::"))
                        tokOffset2 = tokOffset2->tokAt(2);
                }

                // unhandled typedef, skip it and continue
                if (typeName->str() == "void") {
                    unsupportedTypedef(typeDef);
                    tok = deleteInvalidTypedef(typeDef);
                    if (tok == list.front())
                        //now the next token to process is 'tok', not 'tok->next()';
                        goback = true;
                    continue;
                }

                // function pointer
                if (Token::Match(tokOffset2, "* %name% ) (")) {
                    // name token wasn't a name, it was part of the type
                    typeEnd = typeEnd->next();
                    functionPtr = true;
                    funcStart = funcEnd = tokOffset2; // *
                    tokOffset = tokOffset2->tokAt(3); // (
                    typeName = tokOffset->tokAt(-2);
                    argStart = tokOffset;
                    argEnd = tokOffset->link();
                    tok = argEnd->next();
                }

                // function
                else if (TokenList::isFunctionHead(tokOffset->link(), ";,")) {
                    function = true;
                    if (tokOffset->link()->strAt(1) == "const") {
                        specStart = tokOffset->link()->next();
                        specEnd = specStart;
                    }
                    argStart = tokOffset;
                    argEnd = tokOffset->link();
                    tok = argEnd->next();
                    if (specStart)
                        tok = tok->next();
                }

                // syntax error
                else
                    syntaxError(tok);
            }

            // unhandled typedef, skip it and continue
            else {
                unsupportedTypedef(typeDef);
                tok = deleteInvalidTypedef(typeDef);
                if (tok == list.front())
                    //now the next token to process is 'tok', not 'tok->next()';
                    goback = true;
                continue;
            }
        }

        // typeof: typedef typeof ( ... ) type;
        else if (Token::simpleMatch(tokOffset->previous(), "typeof (") &&
                 Token::Match(tokOffset->link(), ") %type% ;")) {
            argStart = tokOffset;
            argEnd = tokOffset->link();
            typeName = tokOffset->link()->next();
            tok = typeName->next();
            typeOf = true;
        }

        // function: typedef ... ( ... type )( ... );
        //           typedef ... (( ... type )( ... ));
        //           typedef ... ( * ( ... type )( ... ));
        else if (tokOffset->str() == "(" && (
                     (tokOffset->link() && Token::Match(tokOffset->link()->previous(), "%type% ) (") &&
                      Token::Match(tokOffset->link()->linkAt(1), ") const|volatile|;")) ||
                     (Token::simpleMatch(tokOffset, "( (") &&
                      tokOffset->next() && Token::Match(tokOffset->linkAt(1)->previous(), "%type% ) (") &&
                      Token::Match(tokOffset->linkAt(1)->linkAt(1), ") const|volatile| ) ;|,")) ||
                     (Token::simpleMatch(tokOffset, "( * (") &&
                      tokOffset->linkAt(2) && Token::Match(tokOffset->linkAt(2)->previous(), "%type% ) (") &&
                      Token::Match(tokOffset->linkAt(2)->linkAt(1), ") const|volatile| ) ;|,")))) {
            if (tokOffset->strAt(1) == "(")
                tokOffset = tokOffset->next();
            else if (Token::simpleMatch(tokOffset, "( * (")) {
                pointers.emplace_back("*");
                tokOffset = tokOffset->tokAt(2);
            }

            if (tokOffset->link()->strAt(-2) == "*")
                functionPtr = true;
            else
                function = true;
            funcStart = tokOffset->next();
            tokOffset = tokOffset->link();
            funcEnd = tokOffset->tokAt(-2);
            typeName = tokOffset->previous();
            argStart = tokOffset->next();
            argEnd = tokOffset->linkAt(1);
            if (!argEnd)
                syntaxError(argStart);
            const Token* tok2 = argEnd->next();
            while (tok2 && (tok2->isKeyword() || tok2->str() == ")"))
                tok2 = tok2->next();
            if (!Token::simpleMatch(tok2, ";"))
                syntaxError(tok2);

            tok = argEnd->next();
            Token *spec = tok;
            if (Token::Match(spec, "const|volatile")) {
                specStart = spec;
                specEnd = spec;
                while (Token::Match(spec->next(), "const|volatile")) {
                    specEnd = spec->next();
                    spec = specEnd;
                }
                tok = specEnd->next();
            }
            if (!tok)
                syntaxError(specEnd);

            if (tok->str() == ")")
                tok = tok->next();
        }

        else if (Token::Match(tokOffset, "( %type% (")) {
            function = true;
            if (tokOffset->link()->next()) {
                tok = tokOffset->link()->next();
                tokOffset = tokOffset->tokAt(2);
                typeName = tokOffset->previous();
                argStart = tokOffset;
                argEnd = tokOffset->link();
            } else {
                // internal error
                continue;
            }
        }

        // pointer to function returning pointer to function
        else if (Token::Match(tokOffset, "( * ( * %type% ) (") &&
                 Token::simpleMatch(tokOffset->linkAt(6), ") ) (") &&
                 Token::Match(tokOffset->linkAt(6)->linkAt(2), ") ;|,")) {
            functionPtrRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(6);
            typeName = tokOffset->tokAt(-2);
            argStart = tokOffset;
            argEnd = tokOffset->link();
            if (!argEnd)
                syntaxError(arrayStart);

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argFuncRetStart->link();
            if (!argFuncRetEnd)
                syntaxError(argFuncRetStart);

            tok = argFuncRetEnd->next();
        }

        // function returning pointer to function
        else if (Token::Match(tokOffset, "( * %type% (") &&
                 Token::simpleMatch(tokOffset->linkAt(3), ") ) (") &&
                 Token::Match(tokOffset->linkAt(3)->linkAt(2), ") ;|,")) {
            functionRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(3);
            typeName = tokOffset->previous();
            argStart = tokOffset;
            argEnd = tokOffset->link();

            argFuncRetStart = argEnd->tokAt(2);
            if (!argFuncRetStart)
                syntaxError(tokOffset);

            argFuncRetEnd = argFuncRetStart->link();
            if (!argFuncRetEnd)
                syntaxError(tokOffset);

            tok = argFuncRetEnd->next();
        } else if (Token::Match(tokOffset, "( * ( %type% ) (")) {
            functionRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(5);
            typeName = tokOffset->tokAt(-2);
            argStart = tokOffset;
            argEnd = tokOffset->link();
            if (!argEnd)
                syntaxError(arrayStart);

            argFuncRetStart = argEnd->tokAt(2);
            if (!argFuncRetStart)
                syntaxError(tokOffset);

            argFuncRetEnd = argFuncRetStart->link();
            if (!argFuncRetEnd)
                syntaxError(tokOffset);

            tok = argFuncRetEnd->next();
        }

        // pointer/reference to array
        else if (Token::Match(tokOffset, "( *|& %type% ) [")) {
            ptrToArray = (tokOffset->strAt(1) == "*");
            refToArray = !ptrToArray;
            tokOffset = tokOffset->tokAt(2);
            typeName = tokOffset;
            arrayStart = tokOffset->tokAt(2);
            arrayEnd = arrayStart->link();
            if (!arrayEnd)
                syntaxError(arrayStart);

            tok = arrayEnd->next();
        }

        // pointer to class member
        else if (Token::Match(tokOffset, "( %type% :: * %type% ) ;")) {
            tokOffset = tokOffset->tokAt(2);
            namespaceStart = tokOffset->previous();
            namespaceEnd = tokOffset;
            ptrMember = true;
            tokOffset = tokOffset->tokAt(2);
            typeName = tokOffset;
            tok = tokOffset->tokAt(2);
        }

        // unhandled typedef, skip it and continue
        else {
            unsupportedTypedef(typeDef);
            tok = deleteInvalidTypedef(typeDef);
            if (tok == list.front())
                //now the next token to process is 'tok', not 'tok->next()';
                goback = true;
            continue;
        }

        bool done = false;
        bool ok = true;

        TypedefInfo typedefInfo;
        typedefInfo.name = typeName->str();
        typedefInfo.filename = list.file(typeName);
        typedefInfo.lineNumber = typeDef->linenr();
        typedefInfo.column = typeDef->column();
        if (Token::Match(typeDef->next(), "struct|enum|class|union %name% {") && typeDef->strAt(2) == typedefInfo.name) {
            typedefInfo.tagLine = typeDef->tokAt(2)->linenr();
            typedefInfo.tagColumn = typeDef->tokAt(2)->column();
        }
        typedefInfo.used = false;
        typedefInfo.isFunctionPointer = isFunctionPointer(typeName);
        if (typedefInfo.isFunctionPointer) {
            const Token* t = typeDef;
            while (t != tok) {
                TypedefToken ttok;
                ttok.name = t->str();
                ttok.lineNumber = t->linenr();
                ttok.column = t->column();
                typedefInfo.typedefInfoTokens.emplace_back(ttok);
                t = t->next();
            }
        }
        mTypedefInfo.push_back(std::move(typedefInfo));

        while (!done) {
            std::string pattern = typeName->str();
            int scope = 0;
            bool simplifyType = false;
            bool inMemberFunc = false;
            int memberScope = 0;
            bool globalScope = false;
            int classLevel = spaceInfo.size();
            bool inTypeDef = false;
            bool inEnum = false;
            std::string removed;
            std::string classPath;
            for (size_t i = 1; i < spaceInfo.size(); ++i) {
                if (!classPath.empty())
                    classPath += " :: ";
                classPath += spaceInfo[i].className;
            }

            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (Settings::terminated())
                    return;

                removed.clear();

                if (Token::simpleMatch(tok2, "typedef"))
                    inTypeDef = true;

                if (inTypeDef && Token::simpleMatch(tok2, ";"))
                    inTypeDef = false;

                // Check for variable declared with the same name
                if (!inTypeDef && spaceInfo.size() == 1 && Token::Match(tok2->previous(), "%name%") &&
                    !tok2->previous()->isKeyword()) {
                    Token* varDecl = tok2;
                    while (Token::Match(varDecl, "*|&|&&|const"))
                        varDecl = varDecl->next();
                    if (Token::Match(varDecl, "%name% ;|,|)|=") && varDecl->str() == typeName->str()) {
                        // Skip to the next closing brace
                        if (Token::Match(varDecl, "%name% ) {")) { // is argument variable
                            tok2 = varDecl->linkAt(2)->next();
                        } else {
                            tok2 = varDecl;
                            while (tok2 && !Token::simpleMatch(tok2, "}")) {
                                if (Token::Match(tok2, "(|{|["))
                                    tok2 = tok2->link();
                                tok2 = tok2->next();
                            }
                        }
                        if (!tok2)
                            break;
                        continue;
                    }
                }

                if (tok2->link()) { // Pre-check for performance
                    // check for end of scope
                    if (tok2->str() == "}") {
                        // check for end of member function
                        if (inMemberFunc) {
                            --memberScope;
                            if (memberScope == 0)
                                inMemberFunc = false;
                        }
                        inEnum = false;

                        if (classLevel > 1 && tok2 == spaceInfo[classLevel - 1].bodyEnd2) {
                            --classLevel;
                            pattern.clear();

                            for (std::size_t i = classLevel; i < spaceInfo.size(); ++i)
                                pattern += (spaceInfo[i].className + " :: ");

                            pattern += typeName->str();
                        } else {
                            if (scope == 0 && !(classLevel > 1 && tok2 == spaceInfo[classLevel - 1].bodyEnd))
                                break;
                            scope = std::max(scope - 1, 0);
                        }
                    }

                    // check for member functions
                    else if (cpp && tok2->str() == "(" && TokenList::isFunctionHead(tok2, "{:")) {
                        const Token *func = tok2->previous();

                        /** @todo add support for multi-token operators */
                        if (func->strAt(-1) == "operator")
                            func = func->previous();

                        if (!func->previous())
                            syntaxError(func);

                        // check for qualifier
                        if (Token::Match(func->tokAt(-2), "%name% ::")) {
                            int offset = -2;
                            while (Token::Match(func->tokAt(offset - 2), "%name% ::"))
                                offset -= 2;
                            // check for available and matching class name
                            if (spaceInfo.size() > 1 && classLevel < spaceInfo.size() &&
                                func->strAt(offset) == spaceInfo[classLevel].className) {
                                memberScope = 0;
                                inMemberFunc = true;
                            }
                        }
                    }

                    // check for entering a new scope
                    else if (tok2->str() == "{") {
                        // check for entering a new namespace
                        if (cpp) {
                            if (tok2->strAt(-2) == "namespace") {
                                if (classLevel < spaceInfo.size() &&
                                    spaceInfo[classLevel].isNamespace &&
                                    spaceInfo[classLevel].className == tok2->strAt(-1)) {
                                    spaceInfo[classLevel].bodyEnd2 = tok2->link();
                                    ++classLevel;
                                    pattern.clear();
                                    for (std::size_t i = classLevel; i < spaceInfo.size(); ++i)
                                        pattern += spaceInfo[i].className + " :: ";

                                    pattern += typeName->str();
                                }
                                ++scope;
                            }
                            if (isEnumStart(tok2))
                                inEnum = true;
                        }

                        // keep track of scopes within member function
                        if (inMemberFunc)
                            ++memberScope;

                        ++scope;
                    }
                }

                // check for operator typedef
                /** @todo add support for multi-token operators */
                else if (cpp &&
                         tok2->str() == "operator" &&
                         tok2->next() &&
                         tok2->strAt(1) == typeName->str() &&
                         tok2->linkAt(2) &&
                         tok2->strAt(2) == "(" &&
                         Token::Match(tok2->linkAt(2), ") const| {")) {
                    // check for qualifier
                    if (tok2->strAt(-1) == "::") {
                        // check for available and matching class name
                        if (spaceInfo.size() > 1 && classLevel < spaceInfo.size() &&
                            tok2->strAt(-2) == spaceInfo[classLevel].className) {
                            tok2 = tok2->next();
                            simplifyType = true;
                        }
                    }
                }

                else if (Token::Match(tok2->previous(), "class|struct %name% [:{]")) {
                    // don't replace names in struct/class definition
                }

                // check for typedef that can be substituted
                else if ((tok2->isNameOnly() || (tok2->isName() && (tok2->isExpandedMacro() || tok2->isInline() || tok2->isExternC()))) &&
                         (Token::simpleMatch(tok2, pattern.c_str(), pattern.size()) ||
                          (inMemberFunc && tok2->str() == typeName->str()))) {
                    // member function class variables don't need qualification
                    if (!(inMemberFunc && tok2->str() == typeName->str()) && pattern.find("::") != std::string::npos) { // has a "something ::"
                        Token *start = tok2;
                        int count = 0;
                        int back = classLevel - 1;
                        bool good = true;
                        // check for extra qualification
                        while (back >= 1) {
                            Token *qualificationTok = start->tokAt(-2);
                            if (!Token::Match(qualificationTok, "%type% ::"))
                                break;
                            if (qualificationTok->str() == spaceInfo[back].className) {
                                start = qualificationTok;
                                back--;
                                count++;
                            } else {
                                good = false;
                                break;
                            }
                        }
                        // check global namespace
                        if (good && back == 1 && start->strAt(-1) == "::")
                            good = false;

                        if (good) {
                            // remove any extra qualification if present
                            while (count) {
                                if (!removed.empty())
                                    removed.insert(0, " ");
                                removed.insert(0, tok2->strAt(-2) + " " + tok2->strAt(-1));
                                tok2->tokAt(-3)->deleteNext(2);
                                --count;
                            }

                            // remove global namespace if present
                            if (tok2->strAt(-1) == "::") {
                                removed.insert(0, ":: ");
                                tok2->tokAt(-2)->deleteNext();
                                globalScope = true;
                            }

                            // remove qualification if present
                            for (std::size_t i = classLevel; i < spaceInfo.size(); ++i) {
                                if (!removed.empty())
                                    removed += " ";
                                removed += (tok2->str() + " " + tok2->strAt(1));
                                tok2->deleteThis();
                                tok2->deleteThis();
                            }
                            simplifyType = true;
                        }
                    } else {
                        if (tok2->strAt(-1) == "::") {
                            int relativeSpaceInfoSize = spaceInfo.size();
                            Token * tokBeforeType = tok2->previous();
                            while (relativeSpaceInfoSize > 1 &&
                                   tokBeforeType && tokBeforeType->str() == "::" &&
                                   tokBeforeType->strAt(-1) == spaceInfo[relativeSpaceInfoSize-1].className) {
                                tokBeforeType = tokBeforeType->tokAt(-2);
                                --relativeSpaceInfoSize;
                            }
                            if (tokBeforeType && tokBeforeType->str() != "::") {
                                Token::eraseTokens(tokBeforeType, tok2);
                                simplifyType = true;
                            }
                        } else if (Token::Match(tok2->previous(), "case|;|{|} %type% :")) {
                            tok2 = tok2->next();
                        } else if (duplicateTypedef(tok2, typeName, typeDef)) {
                            // skip to end of scope if not already there
                            if (tok2->str() != "}") {
                                while (tok2->next()) {
                                    if (tok2->strAt(1) == "{")
                                        tok2 = tok2->linkAt(1)->previous();
                                    else if (tok2->strAt(1) == "}")
                                        break;

                                    tok2 = tok2->next();
                                }
                            }
                        } else if (Token::Match(tok2->tokAt(-2), "%type% *|&")) {
                            // Ticket #5868: Don't substitute variable names
                        } else if (tok2->strAt(-1) != ".") {
                            simplifyType = (TypedefSimplifier::canReplaceStatic(tok2) != 0);
                        }
                    }
                }

                simplifyType = simplifyType && (!inEnum || !Token::simpleMatch(tok2->next(), "="));
                simplifyType = simplifyType && !(Token::simpleMatch(tok2->next(), "<") && Token::simpleMatch(typeEnd, ">"));

                if (simplifyType) {
                    mTypedefInfo.back().used = true;

                    // can't simplify 'operator functionPtr ()' and 'functionPtr operator ... ()'
                    if (functionPtr && (tok2->strAt(-1) == "operator" ||
                                        (tok2->next() && tok2->strAt(1) == "operator"))) {
                        simplifyType = false;
                        tok2 = tok2->next();
                        continue;
                    }

                    // There are 2 categories of typedef substitutions:
                    // 1. variable declarations that preserve the variable name like
                    //    global, local, and function parameters
                    // 2. not variable declarations that have no name like derived
                    //    classes, casts, operators, and template parameters

                    // try to determine which category this substitution is
                    bool inCast = false;
                    bool inTemplate = false;
                    bool inOperator = false;
                    bool inSizeof = false;

                    const bool sameStartEnd = (typeStart == typeEnd);

                    // check for derived class: class A : some_typedef {
                    const bool isDerived = Token::Match(tok2->previous(), "public|protected|private|: %type% {|,");

                    // check for cast: (some_typedef) A or static_cast<some_typedef>(A)
                    // todo: check for more complicated casts like: (const some_typedef *)A
                    if ((tok2->strAt(-1) == "(" && tok2->strAt(1) == ")" && tok2->strAt(-2) != "sizeof") ||
                        (tok2->strAt(-1) == "<" && Token::simpleMatch(tok2->next(), "> (")) ||
                        Token::Match(tok2->tokAt(-2), "( const %name% )"))
                        inCast = true;

                    // check for template parameters: t<some_typedef> t1
                    else if (Token::Match(tok2->previous(), "<|,") &&
                             Token::Match(tok2->next(), "&|*| &|*| >|,"))
                        inTemplate = true;

                    else if (Token::Match(tok2->tokAt(-2), "sizeof ( %type% )"))
                        inSizeof = true;

                    // check for operator
                    if (tok2->strAt(-1) == "operator" ||
                        Token::simpleMatch(tok2->tokAt(-2), "operator const"))
                        inOperator = true;

                    if (typeStart->str() == "typename" && tok2->strAt(-1)=="typename") {
                        // Remove one typename if it is already contained in the goal
                        typeStart = typeStart->next();
                    }

                    // skip over class or struct in derived class declaration
                    bool structRemoved = false;
                    if ((isDerived || inTemplate) && Token::Match(typeStart, "class|struct")) {
                        if (typeStart->str() == "struct")
                            structRemoved = true;
                        typeStart = typeStart->next();
                    }
                    if (Token::Match(typeStart, "struct|class|union") && Token::Match(tok2, "%name% ::"))
                        typeStart = typeStart->next();

                    if (sameStartEnd)
                        typeEnd = typeStart;

                    // Is this a "T()" expression where T is a pointer type?
                    const bool isPointerTypeCall = !inOperator && Token::Match(tok2, "%name% ( )") && !pointers.empty();

                    // start substituting at the typedef name by replacing it with the type
                    const Token* location = tok2;
                    for (Token* tok3 = typeStart; tok3 && (tok3->str() != ";"); tok3 = tok3->next())
                        tok3->isSimplifiedTypedef(true);
                    if (isPointerTypeCall) {
                        tok2->deleteThis();
                        tok2 = simplifyTypedefInsertToken(tok2, "0", location);
                        simplifyTypedefInsertToken(tok2->next(), "0", location);
                    }
                    if (Token::Match(tok2->tokAt(-1), "class|struct|union") && tok2->strAt(-1) == typeStart->str())
                        tok2->deletePrevious();

                    if (cpp && Token::Match(tok2->previous(), "using %name% ::|;")) {
                        tok2->previous()->str("typedef");
                        tok2->insertToken(tok2->str());
                    }
                    tok2->str(typeStart->str());

                    // restore qualification if it was removed
                    if (Token::Match(typeStart, "class|struct|union") || structRemoved) {
                        if (structRemoved)
                            tok2 = tok2->previous();

                        if (globalScope) {
                            tok2 = simplifyTypedefInsertToken(tok2, "::", location);
                        }

                        for (std::size_t i = classLevel; i < spaceInfo.size(); ++i) {
                            tok2 = simplifyTypedefInsertToken(tok2, spaceInfo[i].className, location);
                            tok2 = simplifyTypedefInsertToken(tok2, "::", location);
                        }
                    }

                    // add some qualification back if needed
                    Token *start = tok2;
                    std::string removed1 = removed;
                    std::string::size_type idx = removed1.rfind(" ::");

                    if (idx != std::string::npos)
                        removed1.resize(idx);
                    if (removed1 == classPath && !removed1.empty()) {
                        for (auto it = spaceInfo.crbegin(); it != spaceInfo.crend(); ++it) {
                            if (it->recordTypes.find(start->str()) != it->recordTypes.end()) {
                                std::string::size_type spaceIdx = 0;
                                std::string::size_type startIdx = 0;
                                while ((spaceIdx = removed1.find(' ', startIdx)) != std::string::npos) {
                                    simplifyTypedefInsertToken(tok2->previous(), removed1.substr(startIdx, spaceIdx - startIdx), location);
                                    startIdx = spaceIdx + 1;
                                }
                                simplifyTypedefInsertToken(tok2->previous(), removed1.substr(startIdx), location);
                                simplifyTypedefInsertToken(tok2->previous(), "::", location);
                                break;
                            }
                            idx = removed1.rfind(" ::");
                            if (idx == std::string::npos)
                                break;

                            removed1.resize(idx);
                        }
                    }
                    Token* constTok = Token::simpleMatch(tok2->previous(), "const") ? tok2->previous() : nullptr;
                    // add remainder of type
                    tok2 = simplifyTypedefCopyTokens(tok2, typeStart->next(), typeEnd, location);

                    if (!pointers.empty()) {
                        for (const std::string &p : pointers)
                            // cppcheck-suppress useStlAlgorithm
                            tok2 = simplifyTypedefInsertToken(tok2, p, location);
                        if (constTok && !functionPtr) {
                            tok2 = simplifyTypedefInsertToken(tok2, "const", location);
                            constTok->deleteThis();
                            location = constTok;
                        }
                    }

                    if (funcStart && funcEnd) {
                        tok2 = simplifyTypedefInsertToken(tok2, "(", location);
                        Token *paren = tok2;
                        tok2 = simplifyTypedefCopyTokens(tok2, funcStart, funcEnd, location);

                        if (!inCast)
                            tok2 = processFunc(tok2, inOperator);

                        if (!tok2)
                            break;

                        while (Token::Match(tok2, "%name%|] ["))
                            tok2 = tok2->linkAt(1);

                        tok2 = simplifyTypedefInsertToken(tok2, ")", location);
                        Token::createMutualLinks(tok2, paren);

                        tok2 = simplifyTypedefCopyTokens(tok2, argStart, argEnd, location);

                        if (specStart) {
                            Token *spec = specStart;
                            tok2 = simplifyTypedefInsertToken(tok2, spec->str(), location);
                            while (spec != specEnd) {
                                spec = spec->next();
                                tok2 = simplifyTypedefInsertToken(tok2, spec->str(), location);
                            }
                        }
                    }

                    else if (functionPtr || function) {
                        // don't add parentheses around function names because it
                        // confuses other simplifications
                        bool needParen = true;
                        if (!inTemplate && function && tok2->next() && tok2->strAt(1) != "*")
                            needParen = false;
                        if (needParen) {
                            tok2 = simplifyTypedefInsertToken(tok2, "(", location);
                        }
                        Token *tok3 = tok2;
                        if (namespaceStart) {
                            const Token *tok4 = namespaceStart;

                            while (tok4 != namespaceEnd) {
                                tok2 = simplifyTypedefInsertToken(tok2, tok4->str(), location);
                                tok4 = tok4->next();
                            }
                            tok2 = simplifyTypedefInsertToken(tok2, namespaceEnd->str(), location);
                        }
                        if (functionPtr) {
                            tok2 = simplifyTypedefInsertToken(tok2, "*", location);
                        }

                        if (!inCast)
                            tok2 = processFunc(tok2, inOperator);

                        if (needParen) {
                            if (!tok2)
                                syntaxError(nullptr);

                            tok2 = simplifyTypedefInsertToken(tok2, ")", location);
                            Token::createMutualLinks(tok2, tok3);
                        }
                        if (!tok2)
                            syntaxError(nullptr);

                        tok2 = simplifyTypedefCopyTokens(tok2, argStart, argEnd, location);
                        if (inTemplate) {
                            tok2 = tok2->next();
                        }

                        if (specStart) {
                            Token *spec = specStart;
                            tok2 = simplifyTypedefInsertToken(tok2, spec->str(), location);
                            while (spec != specEnd) {
                                spec = spec->next();
                                tok2 = simplifyTypedefInsertToken(tok2, spec->str(), location);
                            }
                        }
                    } else if (functionRetFuncPtr || functionPtrRetFuncPtr) {
                        tok2 = simplifyTypedefInsertToken(tok2, "(", location);
                        Token *tok3 = tok2;
                        tok2 = simplifyTypedefInsertToken(tok2, "*", location);

                        Token * tok4 = nullptr;
                        if (functionPtrRetFuncPtr) {
                            tok2 = simplifyTypedefInsertToken(tok2, "(", location);
                            tok4 = tok2;
                            tok2 = simplifyTypedefInsertToken(tok2, "*", location);
                        }

                        // skip over variable name if there
                        if (!inCast) {
                            if (!tok2 || !tok2->next())
                                syntaxError(nullptr);

                            if (tok2->strAt(1) != ")")
                                tok2 = tok2->next();
                        }

                        if (tok4 && functionPtrRetFuncPtr) {
                            tok2 = simplifyTypedefInsertToken(tok2,")", location);
                            Token::createMutualLinks(tok2, tok4);
                        }

                        tok2 = simplifyTypedefCopyTokens(tok2, argStart, argEnd, location);

                        tok2 = simplifyTypedefInsertToken(tok2, ")", location);
                        Token::createMutualLinks(tok2, tok3);

                        tok2 = simplifyTypedefCopyTokens(tok2, argFuncRetStart, argFuncRetEnd, location);
                    } else if (ptrToArray || refToArray) {
                        tok2 = simplifyTypedefInsertToken(tok2, "(", location);
                        Token *tok3 = tok2;

                        if (ptrToArray)
                            tok2 = simplifyTypedefInsertToken(tok2, "*", location);
                        else
                            tok2 = simplifyTypedefInsertToken(tok2, "&", location);

                        bool hasName = false;
                        // skip over name
                        if (tok2->next() && tok2->strAt(1) != ")" && tok2->strAt(1) != "," &&
                            tok2->strAt(1) != ">") {
                            hasName = true;
                            if (tok2->strAt(1) != "(")
                                tok2 = tok2->next();

                            // check for function and skip over args
                            if (tok2 && tok2->next() && tok2->strAt(1) == "(")
                                tok2 = tok2->linkAt(1);

                            // check for array
                            if (tok2 && tok2->next() && tok2->strAt(1) == "[")
                                tok2 = tok2->linkAt(1);
                        }

                        simplifyTypedefInsertToken(tok2, ")", location);
                        Token::createMutualLinks(tok2->next(), tok3);

                        if (!hasName)
                            tok2 = tok2->next();
                    } else if (ptrMember) {
                        if (Token::simpleMatch(tok2, "* (")) {
                            tok2 = simplifyTypedefInsertToken(tok2, "*", location);
                        } else {
                            // This is the case of casting operator.
                            // Name is not available, and () should not be
                            // inserted
                            const bool castOperator = inOperator && Token::Match(tok2, "%type% (");
                            Token *openParenthesis = nullptr;

                            if (!castOperator) {
                                tok2 = simplifyTypedefInsertToken(tok2, "(", location);

                                openParenthesis = tok2;
                            }

                            const Token *tok4 = namespaceStart;

                            while (tok4 != namespaceEnd) {
                                tok2 = simplifyTypedefInsertToken(tok2, tok4->str(), location);
                                tok4 = tok4->next();
                            }
                            tok2 = simplifyTypedefInsertToken(tok2, namespaceEnd->str(), location);

                            tok2 = simplifyTypedefInsertToken(tok2, "*", location);

                            if (openParenthesis) {
                                // Skip over name, if any
                                if (Token::Match(tok2->next(), "%name%"))
                                    tok2 = tok2->next();

                                tok2 = simplifyTypedefInsertToken(tok2, ")", location);

                                Token::createMutualLinks(tok2, openParenthesis);
                            }
                        }
                    } else if (typeOf) {
                        tok2 = simplifyTypedefCopyTokens(tok2, argStart, argEnd, location);
                    } else if (Token::Match(tok2, "%name% [")) {
                        while (Token::Match(tok2, "%name%|] [")) {
                            tok2 = tok2->linkAt(1);
                        }
                        tok2 = tok2->previous();
                    }

                    if (arrayStart && arrayEnd) {
                        do {
                            if (!tok2->next())
                                syntaxError(tok2); // can't recover so quit

                            if (!inCast && !inSizeof && !inTemplate)
                                tok2 = tok2->next();

                            if (tok2->str() == "const")
                                tok2 = tok2->next();

                            // reference or pointer to array?
                            if (Token::Match(tok2, "&|*|&&")) {
                                tok2 = tok2->previous();
                                Token *tok3 = simplifyTypedefInsertToken(tok2, "(", location);

                                // handle missing variable name
                                if (Token::Match(tok3, "( *|&|&& *|&|&& %name%"))
                                    tok2 = tok3->tokAt(3);
                                else if (Token::Match(tok2->tokAt(3), "[(),;]"))
                                    tok2 = tok2->tokAt(2);
                                else if (Token::simpleMatch(tok2->tokAt(3), ">"))
                                    tok2 = tok2->tokAt(2);
                                else
                                    tok2 = tok2->tokAt(3);
                                if (!tok2)
                                    syntaxError(nullptr);

                                while (tok2->strAt(1) == "::")
                                    tok2 = tok2->tokAt(2);

                                // skip over function parameters
                                if (tok2->str() == "(")
                                    tok2 = tok2->link();

                                if (tok2->strAt(1) == "(")
                                    tok2 = tok2->linkAt(1);

                                // skip over const/noexcept
                                while (Token::Match(tok2->next(), "const|noexcept")) {
                                    tok2 = tok2->next();
                                    if (Token::Match(tok2->next(), "( true|false )"))
                                        tok2 = tok2->tokAt(3);
                                }

                                tok2 = simplifyTypedefInsertToken(tok2, ")", location);
                                Token::createMutualLinks(tok2, tok3);
                            }

                            if (!tok2->next())
                                syntaxError(tok2); // can't recover so quit

                            // skip over array dimensions
                            while (tok2->strAt(1) == "[")
                                tok2 = tok2->linkAt(1);

                            tok2 = simplifyTypedefCopyTokens(tok2, arrayStart, arrayEnd, location);
                            if (!tok2->next())
                                syntaxError(tok2);

                            if (Token::Match(tok2, "] ; %name% = {") && tok2->next()->isSplittedVarDeclEq())
                                tok2->deleteNext(2);

                            if (tok2->str() == "=") {
                                if (tok2->strAt(1) == "{")
                                    tok2 = tok2->linkAt(1)->next();
                                else if (tok2->strAt(1).at(0) == '\"')
                                    tok2 = tok2->tokAt(2);
                            }
                        } while (Token::Match(tok2, ", %name% ;|=|,"));
                    }

                    simplifyType = false;
                }
                if (!tok2)
                    break;
            }

            if (!tok)
                syntaxError(nullptr);

            if (tok->str() == ";")
                done = true;
            else if (tok->str() == ",") {
                arrayStart = nullptr;
                arrayEnd = nullptr;
                tokOffset = tok->next();
                pointers.clear();

                while (Token::Match(tokOffset, "*|&")) {
                    pointers.push_back(tokOffset->str());
                    tokOffset = tokOffset->next();
                }

                if (Token::Match(tokOffset, "%type%")) {
                    typeName = tokOffset;
                    tokOffset = tokOffset->next();

                    if (tokOffset && tokOffset->str() == "[") {
                        arrayStart = tokOffset;

                        for (;;) {
                            while (tokOffset->next() && !Token::Match(tokOffset->next(), ";|,"))
                                tokOffset = tokOffset->next();

                            if (!tokOffset->next())
                                return; // invalid input
                            if (tokOffset->strAt(1) == ";")
                                break;
                            if (tokOffset->str() == "]")
                                break;
                            tokOffset = tokOffset->next();
                        }

                        arrayEnd = tokOffset;
                        tokOffset = tokOffset->next();
                    }

                    if (Token::Match(tokOffset, ";|,"))
                        tok = tokOffset;
                    else {
                        // we encountered a typedef we don't support yet so just continue
                        done = true;
                        ok = false;
                    }
                } else {
                    // we encountered a typedef we don't support yet so just continue
                    done = true;
                    ok = false;
                }
            } else {
                // something is really wrong (internal error)
                done = true;
                ok = false;
            }
        }

        if (ok) {
            // remove typedef
            Token::eraseTokens(typeDef, tok);

            if (typeDef != list.front()) {
                tok = typeDef->previous();
                tok->deleteNext();
                //no need to remove last token in the list
                if (tok->tokAt(2))
                    tok->deleteNext();
            } else {
                list.front()->deleteThis();
                //no need to remove last token in the list
                if (list.front()->next())
                    list.front()->deleteThis();
                tok = list.front();
                //now the next token to process is 'tok', not 'tok->next()';
                goback = true;
            }
        }
    }
}

namespace {
    struct ScopeInfo3 {
        enum Type : std::uint8_t { Global, Namespace, Record, MemberFunction, Other };
        ScopeInfo3() : parent(nullptr), type(Global), bodyStart(nullptr), bodyEnd(nullptr) {}
        ScopeInfo3(ScopeInfo3 *parent_, Type type_, std::string name_, const Token *bodyStart_, const Token *bodyEnd_)
            : parent(parent_), type(type_), name(std::move(name_)), bodyStart(bodyStart_), bodyEnd(bodyEnd_) {
            if (name.empty())
                return;
            fullName = name;
            ScopeInfo3 *scope = parent;
            while (scope && scope->parent) {
                if (scope->name.empty())
                    break;
                fullName.insert(0, " :: ");
                fullName.insert(0, scope->name);
                scope = scope->parent;
            }
        }
        ScopeInfo3 *parent;
        std::list<ScopeInfo3> children;
        Type type;
        std::string fullName;
        std::string name;
        const Token * bodyStart;
        const Token * bodyEnd;
        std::set<std::string> usingNamespaces;
        std::set<std::string> recordTypes;
        std::set<std::string> baseTypes;

        ScopeInfo3 *addChild(Type scopeType, std::string scopeName, const Token *bodyStartToken, const Token *bodyEndToken) {
            children.emplace_back(this, scopeType, std::move(scopeName), bodyStartToken, bodyEndToken);
            return &children.back();
        }

        bool hasChild(const std::string &childName) const {
            return std::any_of(children.cbegin(), children.cend(), [&](const ScopeInfo3& child) {
                return child.name == childName;
            });
        }

        const ScopeInfo3 * findInChildren(const std::string & scope) const {
            for (const auto & child : children) {
                if (child.type == Record && (child.name == scope || child.fullName == scope))
                    return &child;

                const ScopeInfo3 * temp = child.findInChildren(scope);
                if (temp)
                    return temp;
            }
            return nullptr;
        }

        const ScopeInfo3 * findScope(const std::string & scope) const {
            const ScopeInfo3 * tempScope = this;
            while (tempScope) {
                // check children
                auto it = std::find_if(tempScope->children.cbegin(), tempScope->children.cend(), [&](const ScopeInfo3& child) {
                    return &child != this && child.type == Record && (child.name == scope || child.fullName == scope);
                });
                if (it != tempScope->children.end())
                    return &*it;
                // check siblings for same name
                if (tempScope->parent) {
                    for (const auto &sibling : tempScope->parent->children) {
                        if (sibling.name == tempScope->name && &sibling != this) {
                            const ScopeInfo3 * temp = sibling.findInChildren(scope);
                            if (temp)
                                return temp;
                        }
                    }
                }
                tempScope = tempScope->parent;
            }
            return nullptr;
        }

        bool findTypeInBase(const std::string &scope) const {
            if (scope.empty())
                return false;
            // check in base types first
            if (baseTypes.find(scope) != baseTypes.end())
                return true;
            // check in base types base types
            for (const std::string & base : baseTypes) {
                const ScopeInfo3 * baseScope = findScope(base);
                // bail on uninstantiated recursive template
                if (baseScope == this)
                    return false;
                if (baseScope && baseScope->fullName == scope)
                    return true;
                if (baseScope && baseScope->findTypeInBase(scope))
                    return true;
            }
            return false;
        }

        ScopeInfo3 * findScope(const ScopeInfo3 * scope) {
            if (scope->bodyStart == bodyStart)
                return this;
            for (auto & child : children) {
                ScopeInfo3 * temp = child.findScope(scope);
                if (temp)
                    return temp;
            }
            return nullptr;
        }
    };

    void setScopeInfo(Token *tok, ScopeInfo3 *&scopeInfo, bool debug=false)
    {
        if (!tok)
            return;
        if (tok->str() == "{" && scopeInfo->parent && tok == scopeInfo->bodyStart)
            return;
        if (tok->str() == "}") {
            if (scopeInfo->parent && tok == scopeInfo->bodyEnd)
                scopeInfo = scopeInfo->parent;
            else {
                // Try to find parent scope
                ScopeInfo3 *parent = scopeInfo->parent;
                while (parent && parent->bodyEnd != tok)
                    parent = parent->parent;
                if (parent) {
                    scopeInfo = parent;
                    if (debug)
                        throw std::runtime_error("Internal error: unmatched }");
                }
            }
            return;
        }
        if (!Token::Match(tok, "namespace|class|struct|union %name% {|:|::|<")) {
            // check for using namespace
            if (Token::Match(tok, "using namespace %name% ;|::")) {
                const Token * tok1 = tok->tokAt(2);
                std::string nameSpace;
                while (tok1 && tok1->str() != ";") {
                    if (!nameSpace.empty())
                        nameSpace += " ";
                    nameSpace += tok1->str();
                    tok1 = tok1->next();
                }
                scopeInfo->usingNamespaces.insert(std::move(nameSpace));
            }
            // check for member function
            else if (tok->str() == "{") {
                bool added = false;
                Token *tok1 = tok;
                while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                    tok1 = tok1->previous();
                if (tok1->previous() && (tok1->strAt(-1) == ")" || tok->strAt(-1) == "}")) {
                    tok1 = tok1->linkAt(-1);
                    if (Token::Match(tok1->previous(), "throw|noexcept (")) {
                        tok1 = tok1->previous();
                        while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                            tok1 = tok1->previous();
                        if (tok1->strAt(-1) != ")")
                            return;
                        tok1 = tok1->linkAt(-1);
                    } else {
                        while (Token::Match(tok1->tokAt(-2), ":|, %name%")) {
                            tok1 = tok1->tokAt(-2);
                            if (tok1->strAt(-1) != ")" && tok1->strAt(-1) != "}")
                                return;
                            tok1 = tok1->linkAt(-1);
                        }
                    }
                    if (tok1->strAt(-1) == ">")
                        tok1 = tok1->previous()->findOpeningBracket();
                    if (tok1 && (Token::Match(tok1->tokAt(-3), "%name% :: %name%") ||
                                 Token::Match(tok1->tokAt(-4), "%name% :: ~ %name%"))) {
                        tok1 = tok1->tokAt(-2);
                        if (tok1->str() == "~")
                            tok1 = tok1->previous();
                        std::string scope = tok1->strAt(-1);
                        while (Token::Match(tok1->tokAt(-2), ":: %name%")) {
                            scope = tok1->strAt(-3) + " :: " + scope;
                            tok1 = tok1->tokAt(-2);
                        }
                        scopeInfo = scopeInfo->addChild(ScopeInfo3::MemberFunction, std::move(scope), tok, tok->link());
                        added = true;
                    }
                    // inline member function
                    else if ((scopeInfo->type == ScopeInfo3::Record || scopeInfo->type == ScopeInfo3::Namespace) && tok1 && Token::Match(tok1->tokAt(-1), "%name% (")) {
                        std::string scope = scopeInfo->name + "::" + tok1->strAt(-1);
                        scopeInfo = scopeInfo->addChild(ScopeInfo3::MemberFunction, std::move(scope), tok, tok->link());
                        added = true;
                    }
                }

                if (!added)
                    scopeInfo = scopeInfo->addChild(ScopeInfo3::Other, "", tok, tok->link());
            }
            return;
        }

        const bool record = Token::Match(tok, "class|struct|union %name%");
        tok = tok->next();
        std::string classname = tok->str();
        while (Token::Match(tok, "%name% :: %name%")) {
            tok = tok->tokAt(2);
            classname += " :: " + tok->str();
        }

        // add record type to scope info
        if (record)
            scopeInfo->recordTypes.insert(classname);
        tok = tok->next();

        // skip template parameters
        if (tok && tok->str() == "<") {
            tok = tok->findClosingBracket();
            if (tok)
                tok = tok->next();
        }

        // get base class types
        std::set<std::string> baseTypes;
        if (tok && tok->str() == ":") {
            do {
                tok = tok->next();
                while (Token::Match(tok, "public|protected|private|virtual"))
                    tok = tok->next();
                std::string base;
                while (tok && !Token::Match(tok, ";|,|{")) {
                    if (!base.empty())
                        base += ' ';
                    base += tok->str();
                    tok = tok->next();
                    // add template parameters
                    if (tok && tok->str() == "<") {
                        const Token* endTok = tok->findClosingBracket();
                        if (endTok) {
                            endTok = endTok->next();
                            while (tok != endTok) {
                                base += tok->str();
                                tok = tok->next();
                            }
                        }
                    }
                }
                baseTypes.insert(std::move(base));
            } while (tok && !Token::Match(tok, ";|{"));
        }

        if (tok && tok->str() == "{") {
            scopeInfo = scopeInfo->addChild(record ? ScopeInfo3::Record : ScopeInfo3::Namespace, std::move(classname), tok, tok->link());
            scopeInfo->baseTypes = std::move(baseTypes);
        }
    }

    Token *findSemicolon(Token *tok)
    {
        int level = 0;

        for (; tok && (level > 0 || tok->str() != ";"); tok = tok->next()) {
            if (tok->str() == "{")
                ++level;
            else if (level > 0 && tok->str() == "}")
                --level;
        }

        return tok;
    }

    bool usingMatch(
        const Token *nameToken,
        const std::string &scope,
        Token *&tok,
        const std::string &scope1,
        const ScopeInfo3 *currentScope,
        const ScopeInfo3 *memberClassScope)
    {
        Token *tok1 = tok;

        if (tok1 && tok1->str() != nameToken->str())
            return false;

        // skip this using
        if (tok1 == nameToken) {
            tok = findSemicolon(tok1);
            return false;
        }

        // skip other using with this name
        if (tok1->strAt(-1) == "using") {
            // fixme: this is wrong
            // skip to end of scope
            if (currentScope->bodyEnd)
                tok = const_cast<Token*>(currentScope->bodyEnd->previous());
            return false;
        }

        if (tok1->tokAt(-1)->tokType() == Token::eType || tok1->tokAt(-1)->tokType() == Token::eName)
            return false;

        if (Token::Match(tok1->tokAt(-1), "class|struct|union|enum|namespace")) {
            // fixme
            return false;
        }

        if (Token::simpleMatch(tok1->tokAt(-2), "> ::")) {
            return false;
        }

        if (Token::Match(tok1, "%name% (") && TokenList::isFunctionHead(tok1->next(), "{;:")) {
            if (Token::Match(tok1->previous(), "%name%") && !tok1->previous()->isControlFlowKeyword())
                return false;
            if (Token::Match(tok1->previous(), ">|>>") && tok1->linkAt(-1))
                return false;
            if (Token::Match(tok1->previous(), "*|&|&&")) {
                const Token* prev = tok1->previous();
                while (Token::Match(prev, "%name%|*|&|&&|::") && !prev->isControlFlowKeyword())
                    prev = prev->previous();
                if (Token::Match(prev, ">|>>") && tok1->linkAt(-1))
                    return false;
                if (Token::Match(prev, "[;{}] %name%"))
                    return false;
            }
        }

        // get qualification
        std::string qualification;
        const Token* tok2 = tok1;
        std::string::size_type index = scope.size();
        std::string::size_type new_index = std::string::npos;
        bool match = true;
        while (Token::Match(tok2->tokAt(-2), "%name% ::") && !tok2->tokAt(-2)->isKeyword()) {
            std::string last;
            if (match && !scope1.empty()) {
                new_index = scope1.rfind(' ', index - 1);
                if (new_index != std::string::npos)
                    last = scope1.substr(new_index, index - new_index);
                else if (!qualification.empty())
                    last.clear();
                else
                    last = scope1;
            } else
                match = false;
            if (match && tok2->strAt(-2) == last)
                index = new_index;
            else {
                if (!qualification.empty())
                    qualification = " :: " + qualification;
                qualification = tok2->strAt(-2) + qualification;
            }
            tok2 = tok2->tokAt(-2);
        }

        std::string fullScope1 = scope1;
        if (!scope1.empty() && !qualification.empty())
            fullScope1 += " :: ";
        fullScope1 += qualification;

        if (scope == fullScope1)
            return true;

        const ScopeInfo3 *scopeInfo = memberClassScope ? memberClassScope : currentScope;

        // check in base types
        if (qualification.empty() && scopeInfo->findTypeInBase(scope))
            return true;

        // check using namespace
        const ScopeInfo3 * tempScope = scopeInfo;
        while (tempScope) {
            //if (!tempScope->parent->usingNamespaces.empty()) {
            const std::set<std::string>& usingNS = tempScope->usingNamespaces;
            if (!usingNS.empty()) {
                if (qualification.empty()) {
                    if (usingNS.find(scope) != usingNS.end())
                        return true;
                } else {
                    const std::string suffix = " :: " + qualification;
                    if (std::any_of(usingNS.cbegin(), usingNS.cend(), [&](const std::string& ns) {
                        return scope == ns + suffix;
                    }))
                        return true;
                }
            }
            tempScope = tempScope->parent;
        }

        std::string newScope1 = scope1;

        // scopes didn't match so try higher scopes
        index = newScope1.size();
        while (!newScope1.empty()) {
            const std::string::size_type separator = newScope1.rfind(" :: ", index - 1);
            if (separator != std::string::npos)
                newScope1.resize(separator);
            else
                newScope1.clear();

            std::string newFullScope1 = newScope1;
            if (!newScope1.empty() && !qualification.empty())
                newFullScope1 += " :: ";
            newFullScope1 += qualification;

            if (scope == newFullScope1)
                return true;
        }

        return false;
    }

    std::string memberFunctionScope(const Token *tok)
    {
        std::string qualification;
        const Token *qualTok = tok->strAt(-2) == "~" ? tok->tokAt(-4) : tok->tokAt(-3);
        while (Token::Match(qualTok, "%type% ::")) {
            if (!qualification.empty())
                qualification = " :: " + qualification;
            qualification = qualTok->str() + qualification;
            qualTok = qualTok->tokAt(-2);
        }
        return qualification;
    }

    const Token * memberFunctionEnd(const Token *tok)
    {
        if (tok->str() != "(")
            return nullptr;
        const Token *end = tok->link()->next();
        while (end) {
            if (end->str() == "{" && !Token::Match(end->tokAt(-2), ":|, %name%"))
                return end;
            if (end->str() == ";")
                break;
            end = end->next();
        }
        return nullptr;
    }
} // namespace

bool Tokenizer::isMemberFunction(const Token *openParen)
{
    return (Token::Match(openParen->tokAt(-2), ":: %name% (") ||
            Token::Match(openParen->tokAt(-3), ":: ~ %name% (")) &&
           TokenList::isFunctionHead(openParen, "{:");
}

static bool scopesMatch(const std::string &scope1, const std::string &scope2, const ScopeInfo3 *globalScope)
{
    if (scope1.empty() || scope2.empty())
        return false;

    // check if scopes match
    if (scope1 == scope2)
        return true;

    // check if scopes only differ by global qualification
    if (scope1 == (":: " + scope2)) {
        std::string::size_type end = scope2.find_first_of(' ');
        if (end == std::string::npos)
            end = scope2.size();
        if (globalScope->hasChild(scope2.substr(0, end)))
            return true;
    } else if (scope2 == (":: " + scope1)) {
        std::string::size_type end = scope1.find_first_of(' ');
        if (end == std::string::npos)
            end = scope1.size();
        if (globalScope->hasChild(scope1.substr(0, end)))
            return true;
    }

    return false;
}

static unsigned int tokDistance(const Token* tok1, const Token* tok2) { // use when index() is not available yet
    unsigned int dist = 0;
    const Token* tok = tok1;
    while (tok != tok2) {
        ++dist;
        tok = tok->next();
    }
    return dist;
}

static const Token* skipConstVolatileBackwards(const Token* tok) {
    while (Token::Match(tok, "const|volatile"))
        tok = tok->previous();
    return tok;
}

bool Tokenizer::simplifyUsing()
{
    if (!isCPP())
        return false;

    // simplify using N::x; to using x = N::x;
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "using ::| %name% ::"))
            continue;
        const Token* ns = tok->tokAt(tok->strAt(1) == "::" ? 2 : 1);
        if (ns->isKeyword())
            continue;
        Token* end = tok->tokAt(3);
        while (end && !Token::Match(end, "[;,]")) {
            if (end->str() == "<") // skip template args
                end = end->findClosingBracket();
            else
                end = end->next();
        }
        if (!end)
            continue;
        if (!end->tokAt(-1)->isNameOnly() || end->tokAt(-2)->isLiteral()) // e.g. operator=, operator""sv
            continue;
        tok->insertToken(end->strAt(-1))->insertToken("=")->isSimplifiedTypedef(true);
        if (end->str() == ",") { // comma-separated list
            end->str(";");
            end->insertToken("using");
        }
        tok = end;
    }

    const unsigned int maxReplacementTokens = 1000; // limit the number of tokens we replace

    bool substitute = false;
    ScopeInfo3 scopeInfo;
    ScopeInfo3 *currentScope = &scopeInfo;
    struct Using {
        Using(Token *start, Token *end) : startTok(start), endTok(end) {}
        Token *startTok;
        Token *endTok;
    };
    std::list<Using> usingList;

    ProgressReporter progressReporter(mErrorLogger, mSettings.reportProgress, list.getSourceFilePath(), "Tokenize (using)");

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        progressReporter.report(tok->progressValue());

        if (Settings::terminated())
            return substitute;

        if (Token::Match(tok, "enum class|struct")) {
            Token *bodyStart = tok;
            while (Token::Match(bodyStart, "%name%|:|::|<")) {
                if (bodyStart->str() == "<")
                    bodyStart = bodyStart->findClosingBracket();
                bodyStart = bodyStart ? bodyStart->next() : nullptr;
            }
            if (Token::simpleMatch(bodyStart, "{"))
                tok = bodyStart->link();
            continue;
        }

        if (Token::Match(tok, "{|}|namespace|class|struct|union") ||
            Token::Match(tok, "using namespace %name% ;|::")) {
            try {
                setScopeInfo(tok, currentScope, mSettings.debugwarnings);
            } catch (const std::runtime_error &) {
                reportError(tok, Severity::debug, "simplifyUsingUnmatchedBodyEnd",
                            "simplifyUsing: unmatched body end");
            }
            continue;
        }

        // skip template declarations
        if (Token::Match(tok, "template < !!>")) {
            // add template record type to scope info
            const Token *end = tok->next()->findClosingBracket();
            if (end && Token::Match(end->next(), "class|struct|union %name%"))
                currentScope->recordTypes.insert(end->strAt(2));

            Token *declEndToken = TemplateSimplifier::findTemplateDeclarationEnd(tok);
            if (declEndToken)
                tok = declEndToken;
            continue;
        }

        // look for non-template type aliases
        if (!(tok->strAt(-1) != ">" &&
              (Token::Match(tok, "using %name% = ::| %name%") ||
               (Token::Match(tok, "using %name% [ [") &&
                Token::Match(tok->linkAt(2), "] ] = ::| %name%")))))
            continue;

        const std::string& name = tok->strAt(1);
        const Token *nameToken = tok->next();
        std::string scope = currentScope->fullName;
        Token *usingStart = tok;
        Token *start;
        if (tok->strAt(2) == "=") {
            if (currentScope->type == ScopeInfo3::Record && tok->tokAt(2)->isSimplifiedTypedef())  // don't simplify within class definition
                continue;
            start = tok->tokAt(3);
        }
        else
            start = tok->linkAt(2)->tokAt(3);
        Token *usingEnd = findSemicolon(start);
        if (!usingEnd)
            continue;

        for (Token *typeTok = start; typeTok != usingEnd; typeTok = typeTok->next())
            typeTok->isSimplifiedTypedef(true);

        // Move struct defined in using out of using.
        // using T = struct t { }; => struct t { }; using T = struct t;
        // fixme: this doesn't handle attributes
        if (Token::Match(start, "class|struct|union|enum %name%| {|:")) {
            Token *structEnd = start->tokAt(1);
            const bool hasName = Token::Match(structEnd, "%name%");

            // skip over name if present
            if (hasName)
                structEnd = structEnd->next();

            // skip over base class information
            if (structEnd->str() == ":") {
                structEnd = structEnd->next(); // skip over ":"
                while (structEnd && structEnd->str() != "{")
                    structEnd = structEnd->next();
                if (!structEnd)
                    continue;
            }

            // use link to go to end
            structEnd = structEnd->link();

            // add ';' after end of struct
            structEnd->insertToken(";");

            // add name for anonymous struct
            if (!hasName) {
                std::string newName;
                if (structEnd->strAt(2) == ";")
                    newName = name;
                else
                    newName = "Unnamed" + std::to_string(mUnnamedCount++);
                TokenList::copyTokens(structEnd->next(), tok, start);
                structEnd->tokAt(5)->insertToken(newName);
                start->insertToken(newName);
            } else
                TokenList::copyTokens(structEnd->next(), tok, start->next());

            // add using after end of struct
            usingStart = structEnd->tokAt(2);
            nameToken = usingStart->next();
            if (usingStart->strAt(2) == "=")
                start = usingStart->tokAt(3);
            else
                start = usingStart->linkAt(2)->tokAt(3);
            usingEnd = findSemicolon(start);

            // delete original using before struct
            tok->deleteThis();
            tok->deleteThis();
            tok->deleteThis();
            tok = usingStart;
        }

        // remove 'typename' and 'template'
        else if (start->str() == "typename") {
            start->deleteThis();
            Token *temp = start;
            while (Token::Match(temp, "%name% ::"))
                temp = temp->tokAt(2);
            if (Token::Match(temp, "template %name%"))
                temp->deleteThis();
        }

        if (usingEnd)
            tok = usingEnd;

        if (Token::Match(start, "class|struct|union|enum"))
            start = start->next();

        // Unfortunately we have to start searching from the beginning
        // of the token stream because templates are instantiated at
        // the end of the token stream and it may be used before then.
        ScopeInfo3 scopeInfo1;
        ScopeInfo3 *currentScope1 = &scopeInfo1;
        Token *startToken = list.front();
        const Token *endToken = nullptr;
        bool inMemberFunc = false;
        const ScopeInfo3 * memberFuncScope = nullptr;
        const Token * memberFuncEnd = nullptr;

        // We can limit the search to the current function when the type alias
        // is defined in that function.
        if (currentScope->type == ScopeInfo3::Other ||
            currentScope->type == ScopeInfo3::MemberFunction) {
            scopeInfo1 = scopeInfo;
            currentScope1 = scopeInfo1.findScope(currentScope);
            if (!currentScope1)
                return substitute; // something bad happened
            startToken = usingEnd->next();
            endToken = currentScope->bodyEnd->next();
            if (currentScope->type == ScopeInfo3::MemberFunction) {
                const ScopeInfo3 * temp = currentScope->findScope(currentScope->fullName);
                if (temp) {
                    inMemberFunc = true;
                    memberFuncScope = temp;
                    memberFuncEnd = endToken;
                }
            }
        }

        bool isTypedefInfoAdded = false; // TODO should we add a separate mUsingInfo?

        std::string scope1 = currentScope1->fullName;
        bool skip = false; // don't erase type aliases we can't parse
        Token *enumOpenBrace = nullptr;
        for (Token* tok1 = startToken; !skip && tok1 && tok1 != endToken; tok1 = tok1->next()) {
            // skip enum body
            if (tok1 && tok1 == enumOpenBrace) {
                tok1 = tok1->link();
                enumOpenBrace = nullptr;
                continue;
            }

            if ((Token::Match(tok1, "{|}|namespace|class|struct|union") && tok1->strAt(-1) != "using") ||
                Token::Match(tok1, "using namespace %name% ;|::")) {
                try {
                    setScopeInfo(tok1, currentScope1, mSettings.debugwarnings);
                } catch (const std::runtime_error &) {
                    reportError(tok1, Severity::debug, "simplifyUsingUnmatchedBodyEnd",
                                "simplifyUsing: unmatched body end");
                }
                scope1 = currentScope1->fullName;
                if (inMemberFunc && memberFuncEnd && tok1 == memberFuncEnd) {
                    inMemberFunc = false;
                    memberFuncScope = nullptr;
                    memberFuncEnd = nullptr;
                }
                continue;
            }

            // skip template definitions
            if (Token::Match(tok1, "template < !!>")) {
                Token *declEndToken = TemplateSimplifier::findTemplateDeclarationEnd(tok1);
                if (declEndToken)
                    tok1 = declEndToken;
                continue;
            }

            // check for enum with body
            if (tok1->str() == "enum") {
                if (Token::Match(tok1, "enum class|struct"))
                    tok1 = tok1->next();
                Token *defStart = tok1;
                while (Token::Match(defStart, "%name%|::|:"))
                    defStart = defStart->next();
                if (Token::simpleMatch(defStart, "{"))
                    enumOpenBrace = defStart;
                continue;
            }

            // check for member function and adjust scope
            if (isMemberFunction(tok1)) {
                if (!scope1.empty())
                    scope1 += " :: ";
                scope1 += memberFunctionScope(tok1);
                const ScopeInfo3 * temp = currentScope1->findScope(scope1);
                if (temp) {
                    const Token *end = memberFunctionEnd(tok1);
                    if (end) {
                        inMemberFunc = true;
                        memberFuncScope = temp;
                        memberFuncEnd = end;
                    }
                }
                continue;
            }
            if (inMemberFunc && memberFuncScope) {
                if (!usingMatch(nameToken, scope, tok1, scope1, currentScope1, memberFuncScope))
                    continue;
            } else if (!usingMatch(nameToken, scope, tok1, scope1, currentScope1, nullptr))
                continue;

            if (!isTypedefInfoAdded && Token::Match(tok1, "%name% (")) {
                isTypedefInfoAdded = true;
                TypedefInfo usingInfo;
                usingInfo.name = name;
                usingInfo.filename = list.file(nameToken);
                usingInfo.lineNumber = nameToken->linenr();
                usingInfo.column = nameToken->column();
                usingInfo.used = true;
                usingInfo.isFunctionPointer = false;
                mTypedefInfo.push_back(std::move(usingInfo));
            }

            const auto nReplace = tokDistance(start, usingEnd);
            if (nReplace > maxReplacementTokens) {
                simplifyUsingError(usingStart, usingEnd);
                continue;
            }

            // remove the qualification
            std::string fullScope = scope;
            std::string removed;
            while (Token::Match(tok1->tokAt(-2), "%name% ::") && !tok1->tokAt(-2)->isKeyword()) {
                removed = (tok1->strAt(-2) + " :: ") + removed;
                if (fullScope == tok1->strAt(-2)) {
                    tok1->deletePrevious();
                    tok1->deletePrevious();
                    break;
                }
                const std::string::size_type idx = fullScope.rfind("::");

                if (idx == std::string::npos)
                    break;

                if (tok1->strAt(-2) == fullScope.substr(idx + 3)) {
                    tok1->deletePrevious();
                    tok1->deletePrevious();
                    fullScope.resize(idx - 1);
                } else
                    break;
            }

            // remove global namespace if present
            if (tok1->strAt(-1) == "::") {
                removed.insert(0, ":: ");
                tok1->deletePrevious();
            }

            Token * arrayStart = nullptr;
            Token * fpArgList = nullptr;
            Token * fpQual = nullptr;

            // parse the type
            Token *type = start;
            if (type->str() == "::") {
                type = type->next();
                while (Token::Match(type, "%type% ::"))
                    type = type->tokAt(2);
                if (Token::Match(type, "%type%"))
                    type = type->next();
            } else if (Token::Match(type, "%type% ::")) {
                do {
                    type = type->tokAt(2);
                } while (Token::Match(type, "%type% ::"));
                if (Token::Match(type, "%type%"))
                    type = type->next();
            } else if (Token::Match(type, "%type%")) {
                while (Token::Match(type, "const|class|struct|union|enum %type%") ||
                       (type->next() && type->next()->isStandardType()))
                    type = type->next();

                type = type->next();

                while (Token::Match(type, "%type%") &&
                       (type->isStandardType() || Token::Match(type, "unsigned|signed"))) {
                    type = type->next();
                }

                bool atEnd = false;
                while (!atEnd) {
                    if (type && type->str() == "::") {
                        type = type->next();
                    }

                    if (Token::Match(type, "%type%") &&
                        type->next() && !Token::Match(type->next(), "[|,|(")) {
                        type = type->next();
                    } else if (Token::simpleMatch(type, "const (")) {
                        type = type->next();
                        atEnd = true;
                    } else
                        atEnd = true;
                }
            } else
                syntaxError(type);

            // check for invalid input
            if (!type)
                syntaxError(tok1);

            // check for template
            if (type->str() == "<") {
                type = type->findClosingBracket();

                while (type && Token::Match(type->next(), ":: %type%"))
                    type = type->tokAt(2);

                if (!type) {
                    syntaxError(tok1);
                }

                while (Token::Match(type->next(), "const|volatile"))
                    type = type->next();

                type = type->next();
            }

            // check for pointers and references
            std::list<std::string> pointers;
            while (Token::Match(type, "*|&|&&|const")) {
                pointers.push_back(type->str());
                type = type->next();
            }

            // check for array
            if (type && type->str() == "[") {
                do {
                    if (!arrayStart)
                        arrayStart = type;

                    bool atEnd = false;
                    while (!atEnd) {
                        while (type->next() && !Token::Match(type->next(), ";|,")) {
                            type = type->next();
                        }

                        if (!type->next())
                            syntaxError(type); // invalid input
                        else if (type->strAt(1) == ";")
                            atEnd = true;
                        else if (type->str() == "]")
                            atEnd = true;
                        else
                            type = type->next();
                    }

                    type = type->next();
                } while (type && type->str() == "[");
            }

            // check for function pointer
            if (type && type->str() == "(") {
                if (Token::simpleMatch(type->link(), ") (")) {
                    fpArgList = type->link()->next();
                    fpQual = type;
                    type = type->link()->linkAt(1)->next();
                } else if (type->link()->next() == usingEnd) {
                    fpArgList = type;
                    type = usingEnd;
                }
            }

            // make sure we are in a good state
            if (!tok1 || !tok1->next())
                break; // bail

            Token* after = tok1->next();
            // check if type was parsed
            if (type && type == usingEnd) {
                // check for array syntax and add type around variable
                if (arrayStart) {
                    if (Token::Match(tok1->next(), "%name%")) {
                        TokenList::copyTokens(tok1->next(), arrayStart, usingEnd->previous());
                        TokenList::copyTokens(tok1, start, arrayStart->previous());
                        tok1->deleteThis();
                        substitute = true;
                    }
                } else if (fpArgList && fpQual && Token::Match(tok1->next(), "%name%")) {
                    // function pointer
                    const bool isFuncDecl = Token::simpleMatch(tok1->tokAt(2), "(");
                    TokenList::copyTokens(tok1->next(), fpArgList, usingEnd->previous());
                    Token* const copyEnd = TokenList::copyTokens(tok1, start, fpQual->link()->previous());
                    Token* leftPar = copyEnd->previous();
                    while (leftPar->str() != "(")
                        leftPar = leftPar->previous();
                    Token* const insertTok = isFuncDecl ? copyEnd->linkAt(2) : copyEnd->next();
                    Token* const rightPar = insertTok->insertToken(")");
                    Token::createMutualLinks(leftPar, rightPar);
                    tok1->deleteThis();
                    substitute = true;
                } else if (fpArgList && !fpQual && Token::Match(tok1->next(), "* const| %name%")) {
                    // function pointer
                    Token* const dest = tok1->tokAt(tok1->strAt(2) == "const" ? 3 : 2);
                    Token* const copyEndArgs = TokenList::copyTokens(dest, fpArgList, usingEnd->previous());
                    Token* const copyEndRet = TokenList::copyTokens(tok1, start, fpArgList->previous());
                    Token* const leftPar = copyEndRet->insertToken("(");
                    Token* const rightPar = copyEndArgs->link()->tokAt(-1)->insertToken(")");
                    Token::createMutualLinks(leftPar, rightPar);
                    tok1->deleteThis();
                    substitute = true;
                } else {
                    // add some qualification back if needed
                    std::string removed1 = std::move(removed);
                    std::string::size_type idx = removed1.rfind(" ::");
                    if (idx != std::string::npos)
                        removed1.resize(idx);
                    if (scopesMatch(removed1, scope, &scopeInfo1)) {
                        ScopeInfo3 * tempScope = currentScope;
                        while (tempScope->parent) {
                            if (tempScope->recordTypes.find(start->str()) != tempScope->recordTypes.end()) {
                                std::string::size_type spaceIdx = 0;
                                std::string::size_type startIdx = 0;
                                while ((spaceIdx = removed1.find(' ', startIdx)) != std::string::npos) {
                                    tok1->previous()->insertToken(removed1.substr(startIdx, spaceIdx - startIdx));
                                    startIdx = spaceIdx + 1;
                                }
                                tok1->previous()->insertToken(removed1.substr(startIdx));
                                tok1->previous()->insertToken("::");
                                break;
                            }
                            idx = removed1.rfind(" ::");
                            if (idx == std::string::npos)
                                break;

                            removed1.resize(idx);
                            tempScope = tempScope->parent;
                        }
                    }

                    // Is this a "T(...)" expression where T is a pointer type?
                    if (Token::Match(tok1, "%name% [({]") && !pointers.empty() && !Token::simpleMatch(skipConstVolatileBackwards(tok1->tokAt(-1)), ".")) {
                        tok1->tokAt(1)->str("(");
                        tok1->linkAt(1)->str(")");
                        if (tok1->linkAt(1) == tok1->tokAt(2)) { // T() or T{}
                            Token* tok2 = tok1->linkAt(1);
                            tok1->deleteThis();
                            TokenList::copyTokens(tok1, start, usingEnd->previous());
                            tok2->insertToken("0");
                            after = tok2->next();
                        }
                        else { // functional-style cast
                            Token* tok2 = tok1->linkAt(1);
                            tok1->originalName(tok1->str());
                            tok1->isSimplifiedTypedef(true);
                            tok1->str("(");
                            Token* tok3 = TokenList::copyTokens(tok1, start, usingEnd->previous());
                            tok3->insertToken(")");
                            Token::createMutualLinks(tok1, tok3->next());
                            after = tok2->next();
                        }
                    }
                    else { // just replace simple type aliases
                        TokenList::copyTokens(tok1, start, usingEnd->previous());
                        tok1->deleteThis();
                    }
                    substitute = true;
                }
            } else {
                skip = true;
                simplifyUsingError(usingStart, usingEnd);
            }
            tok1 = after->previous();
        }

        if (!skip)
            usingList.emplace_back(usingStart, usingEnd);
    }

    // delete all used type alias definitions
    for (auto it = usingList.rbegin(); it != usingList.rend(); ++it) {
        Token *usingStart = it->startTok;
        Token *usingEnd = it->endTok;
        if (usingStart->previous()) {
            Token::eraseTokens(usingStart->previous(), usingEnd->next());
        } else {
            if (usingEnd->next()) {
                Token::eraseTokens(usingStart, usingEnd->next());
                usingStart->deleteThis();
            } else {
                // this is the only code being checked so leave ';'
                Token::eraseTokens(usingStart, usingEnd);
                usingStart->deleteThis();
            }
        }
    }

    return substitute;
}

void Tokenizer::simplifyUsingError(const Token* usingStart, const Token* usingEnd)
{
    if (mSettings.debugwarnings) {
        std::string str;
        for (const Token *tok = usingStart; tok && tok != usingEnd; tok = tok->next()) {
            if (!str.empty())
                str += ' ';
            str += tok->str();
        }
        str += " ;";
        std::list<const Token *> callstack(1, usingStart);
        mErrorLogger.reportErr(ErrorMessage(callstack, &list, Severity::debug, "simplifyUsing",
                                            "Failed to parse \'" + str + "\'. The checking continues anyway.", Certainty::normal));
    }
}

bool Tokenizer::simplifyTokens1(const std::string &configuration, int fileIndex)
{
    // Fill the map mTypeSize..
    fillTypeSizes();

    mConfiguration = configuration;

    if (mTimerResults) {
        Timer t("Tokenizer::simplifyTokens1::simplifyTokenList1", mSettings.showtime, mTimerResults);
        if (!simplifyTokenList1(list.getFiles().front().c_str()))
            return false;
    } else {
        if (!simplifyTokenList1(list.getFiles().front().c_str()))
            return false;
    }

    const ShowTime showTime = mTimerResults ? mSettings.showtime : ShowTime::NONE;

    Timer::run("Tokenizer::simplifyTokens1::createAst", showTime, mTimerResults, [&]() {
        list.createAst();
        list.validateAst(mSettings.debugnormal);
    });

    Timer::run("Tokenizer::simplifyTokens1::createSymbolDatabase", showTime, mTimerResults, [&]() {
        createSymbolDatabase();
    });

    Timer::run("Tokenizer::simplifyTokens1::setValueType", showTime, mTimerResults, [&]() {
        mSymbolDatabase->setValueTypeInTokenList(false);
        mSymbolDatabase->setValueTypeInTokenList(true);
    });

    if (!mSettings.buildDir.empty())
        Summaries::create(*this, configuration, fileIndex);

    // TODO: apply this through Settings::ValueFlowOptions
    // TODO: do not run valueflow if no checks are being performed at all - e.g. unusedFunctions only
    // TODO: log message when this is active?
    const char* disableValueflowEnv = std::getenv("DISABLE_VALUEFLOW");
    const bool doValueFlow = !disableValueflowEnv || (std::strcmp(disableValueflowEnv, "1") != 0);

    if (doValueFlow) {
        Timer::run("Tokenizer::simplifyTokens1::ValueFlow", showTime, mTimerResults, [&]() {
            ValueFlow::setValues(list, *mSymbolDatabase, mErrorLogger, mSettings, mTimerResults);
        });

        arraySizeAfterValueFlow();
    }

    // Warn about unhandled character literals
    if (mSettings.severity.isEnabled(Severity::portability)) {
        for (const Token *tok = tokens(); tok; tok = tok->next()) {
            if (tok->tokType() == Token::eChar && tok->values().empty()) {
                try {
                    simplecpp::characterLiteralToLL(tok->str());
                } catch (const std::runtime_error &e) {
                    unhandledCharLiteral(tok, e.what());
                }
            }
        }
    }

    if (doValueFlow) {
        mSymbolDatabase->setArrayDimensionsUsingValueFlow();
    }

    printDebugOutput(std::cout);

    return true;
}

//---------------------------------------------------------------------------

void Tokenizer::findComplicatedSyntaxErrorsInTemplates()
{
    validate();
    mTemplateSimplifier->checkComplicatedSyntaxErrorsInTemplates();
}

void Tokenizer::checkForEnumsWithTypedef()
{
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "enum %name% {")) {
            tok = tok->tokAt(2);
            const Token *tok2 = Token::findsimplematch(tok, "typedef", tok->link());
            if (tok2)
                syntaxError(tok2);
            tok = tok->link();
        }
    }
}

void Tokenizer::fillTypeSizes()
{
    mTypeSize.clear();
    mTypeSize["char"] = 1;
    mTypeSize["_Bool"] = mSettings.platform.sizeof_bool;
    mTypeSize["bool"] = mSettings.platform.sizeof_bool;
    mTypeSize["short"] = mSettings.platform.sizeof_short;
    mTypeSize["int"] = mSettings.platform.sizeof_int;
    mTypeSize["long"] = mSettings.platform.sizeof_long;
    mTypeSize["long long"] = mSettings.platform.sizeof_long_long;
    mTypeSize["float"] = mSettings.platform.sizeof_float;
    mTypeSize["double"] = mSettings.platform.sizeof_double;
    mTypeSize["long double"] = mSettings.platform.sizeof_long_double;
    mTypeSize["wchar_t"] = mSettings.platform.sizeof_wchar_t;
    mTypeSize["size_t"] = mSettings.platform.sizeof_size_t;
    mTypeSize["*"] = mSettings.platform.sizeof_pointer;
}

void Tokenizer::combineOperators()
{
    const bool cpp = isCPP();

    // Combine tokens..
    for (Token *tok = list.front(); tok && tok->next(); tok = tok->next()) {
        const char c1 = tok->str()[0];

        if (tok->str().length() == 1 && tok->strAt(1).length() == 1) {
            const char c2 = tok->strAt(1)[0];

            // combine +-*/ and =
            if (c2 == '=' && (std::strchr("+-*/%|^=!<>", c1)) && !Token::Match(tok->previous(), "%type% *")) {
                // skip templates
                if (cpp && (tok->str() == ">" || Token::simpleMatch(tok->previous(), "> *"))) {
                    const Token* opening =
                        tok->str() == ">" ? tok->findOpeningBracket() : tok->previous()->findOpeningBracket();
                    if (opening && Token::Match(opening->previous(), "%name%"))
                        continue;
                }
                tok->str(tok->str() + c2);
                tok->deleteNext();
                continue;
            }
        } else if (tok->strAt(1) == "=") {
            if (tok->str() == ">>") {
                tok->str(">>=");
                tok->deleteNext();
            } else if (tok->str() == "<<") {
                tok->str("<<=");
                tok->deleteNext();
            }
        } else if (cpp && (c1 == 'p' || c1 == '_') &&
                   Token::Match(tok, "private|protected|public|__published : !!:")) {
            bool simplify = false;
            int par = 0;
            for (const Token *prev = tok->previous(); prev; prev = prev->previous()) {
                if (prev->str() == ")") {
                    ++par;
                } else if (prev->str() == "(") {
                    if (par == 0U)
                        break;
                    --par;
                }
                if (par != 0U || prev->str() == "(")
                    continue;
                if (Token::Match(prev, "[;{}]")) {
                    simplify = true;
                    break;
                }
                if (prev->isName() && prev->isUpperCaseName())
                    continue;
                if (prev->isName() && endsWith(prev->str(), ':'))
                    simplify = true;
                break;
            }
            if (simplify) {
                tok->str(tok->str() + ":");
                tok->tokType(Token::Type::eKeyword); // we need to preserve the keyword type after setting a non-keyword string
                tok->deleteNext();
            }
        } else if (tok->str() == "->") {
            // If the preceding sequence is "( & %name% )", replace it by "%name%"
            Token* t = tok->tokAt(-4);
            if (Token::Match(t, "( & %name% )") && !Token::simpleMatch(t->previous(), ">")) {
                t->deleteThis();
                t->deleteThis();
                t->deleteNext();
                tok->str(".");
            } else {
                tok->str(".");
                tok->originalName("->");
            }
        }
    }
}

void Tokenizer::combineStringAndCharLiterals()
{
    // Combine strings
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!isStringLiteral(tok->str()))
            continue;

        tok->str(simplifyString(tok->str()));

        while (Token::Match(tok->next(), "%str%") || Token::Match(tok->next(), "_T|_TEXT|TEXT ( %str% )")) {
            if (tok->next()->isName()) {
                if (!mSettings.platform.isWindows())
                    break;
                tok->deleteNext(2);
                tok->next()->deleteNext();
            }
            // Two strings after each other, combine them
            tok->concatStr(simplifyString(tok->strAt(1)));
            tok->deleteNext();
        }
    }
}

void Tokenizer::concatenateNegativeNumberAndAnyPositive()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "?|:|,|(|[|{|return|case|sizeof|%op% +|-") || tok->tokType() == Token::eIncDecOp)
            continue;

        if (tok->findOpeningBracket())
            continue;

        while (tok->str() != ">" && tok->next() && tok->strAt(1) == "+" && (!Token::Match(tok->tokAt(2), "%name% (|;") || Token::Match(tok, "%op%")))
            tok->deleteNext();

        if (Token::Match(tok->next(), "+|- %num%")) {
            std::string prefix = tok->strAt(1);
            tok->deleteNext();
            tok->next()->str(prefix + tok->strAt(1));
        }
    }
}

void Tokenizer::simplifyExternC()
{
    if (isC())
        return;

    // Add attributes to all tokens within `extern "C"` inlines and blocks, and remove the `extern "C"` tokens.
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "extern \"C\"|\"C++\"")) {
            Token *tok2 = tok->next();
            const bool isExtC = tok->strAt(1).size() == 3;
            if (tok->strAt(2) == "{") {
                tok2 = tok2->next(); // skip {
                while ((tok2 = tok2->next()) && tok2 != tok->linkAt(2))
                    tok2->isExternC(isExtC);
                tok->linkAt(2)->deleteThis(); // }
                tok->deleteNext(2); // "C" {
            } else {
                while ((tok2 = tok2->next()) && !Token::Match(tok2, "[;{]"))
                    tok2->isExternC(isExtC);
                tok->deleteNext(); // "C"
            }
            tok->deleteThis(); // extern
        }
    }
}

void Tokenizer::simplifyCompoundStatements()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "[;{}:] ( {") &&
               Token::simpleMatch(tok->linkAt(2), "} ) ;")) {
            if (tok->str() == ":" && !Token::Match(tok->tokAt(-2),"[;{}] %type% :"))
                break;
            Token *end = tok->linkAt(2)->tokAt(-3);
            if (Token::Match(end, "[;{}] %num%|%str% ;"))
                end->deleteNext(2);
            tok->linkAt(2)->previous()->deleteNext(3);
            tok->deleteNext(2);
        }
        if (Token::Match(tok, "( { %bool%|%char%|%num%|%str%|%name% ; } )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext(3);
        }
        else if (tok->str() == "(")
            tok = tok->link();
    }
}

void Tokenizer::simplifySQL()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL"))
            continue;

        const Token *end = findSQLBlockEnd(tok);
        if (end == nullptr)
            syntaxError(nullptr);

        const std::string instruction = tok->stringifyList(end);
        // delete all tokens until the embedded SQL block end
        Token::eraseTokens(tok, end);

        // insert "asm ( "instruction" ) ;"
        tok->str("asm");
        // it can happen that 'end' is NULL when wrong code is inserted
        if (!tok->next())
            tok->insertToken(";");
        tok->insertToken(")");
        tok->insertToken("\"" + instruction + "\"");
        tok->insertToken("(");
        // jump to ';' and continue
        tok = tok->tokAt(3);
    }
}

void Tokenizer::simplifyParenthesizedLibraryFunctions()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, ") ("))
            continue;
        Token *rpar = tok, *lpar = tok->link();
        if (!lpar || (Token::Match(lpar->previous(), "%name%") && !Token::Match(lpar->previous(), "return|delete|throw")))
            continue;
        const Token *ftok = rpar->previous();
        if (mSettings.library.isNotLibraryFunction(ftok))
            continue;
        lpar->deleteThis();
        rpar->deleteThis();
    }
}

void Tokenizer::simplifyArrayAccessSyntax()
{
    // 0[a] -> a[0]
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isNumber() && Token::Match(tok, "%num% [ %name% ]")) {
            const std::string number(tok->str());
            Token* indexTok = tok->tokAt(2);
            tok->str(indexTok->str());
            tok->varId(indexTok->varId());
            indexTok->str(number);
        }
    }
}

void Tokenizer::simplifyParameterVoid()
{
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% ( void )") && !Token::Match(tok, "sizeof|decltype|typeof|return")) {
            tok->next()->deleteNext();
            tok->next()->setRemovedVoidParameter(true);
        }
    }
}

void Tokenizer::simplifyRedundantConsecutiveBraces()
{
    // Remove redundant consecutive braces, i.e. '.. { { .. } } ..' -> '.. { .. } ..'.
    for (Token *tok = list.front(); tok;) {
        if (Token::simpleMatch(tok, "= {")) {
            tok = tok->linkAt(1);
        } else if (Token::simpleMatch(tok, "{ {") && Token::simpleMatch(tok->linkAt(1), "} }")) {
            //remove internal parentheses
            tok->linkAt(1)->deleteThis();
            tok->deleteNext();
        } else
            tok = tok->next();
    }
}

void Tokenizer::simplifyDoublePlusAndDoubleMinus()
{
    // Convert - - into + and + - into -
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (tok->next()) {
            if (tok->str() == "+") {
                if (tok->strAt(1)[0] == '-') {
                    tok = tok->next();
                    if (tok->str().size() == 1) {
                        tok = tok->previous();
                        tok->str("-");
                        tok->deleteNext();
                    } else if (tok->isNumber()) {
                        tok->str(tok->str().substr(1));
                        tok = tok->previous();
                        tok->str("-");
                    }
                    continue;
                }
            } else if (tok->str() == "-") {
                if (tok->strAt(1)[0] == '-') {
                    tok = tok->next();
                    if (tok->str().size() == 1) {
                        tok = tok->previous();
                        tok->str("+");
                        tok->deleteNext();
                    } else if (tok->isNumber()) {
                        tok->str(tok->str().substr(1));
                        tok = tok->previous();
                        tok->str("+");
                    }
                    continue;
                }
            }

            break;
        }
    }
}

/** Specify array size if it hasn't been given */

void Tokenizer::arraySize()
{
    auto getStrTok = [](Token* tok, bool addLength, Token*& endStmt) -> Token* {
        if (addLength) {
            endStmt = tok->tokAt(5);
            return tok->tokAt(4);
        }
        if (Token::Match(tok, "%var% [ ] =")) {
            tok = tok->tokAt(4);
            int parCount = 0;
            while (Token::simpleMatch(tok, "(")) {
                ++parCount;
                tok = tok->next();
            }
            if (Token::Match(tok, "%str%")) {
                endStmt = tok->tokAt(parCount + 1);
                return tok;
            }
        }
        return nullptr;
    };

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || !Token::Match(tok, "%var% [ ] ="))
            continue;
        bool addlength = false;
        if (Token::Match(tok->previous(), "!!* %var% [ ] = { %str% } ;")) {
            Token *t = tok->tokAt(3);
            t->deleteNext();
            t->next()->deleteNext();
            addlength = true;
        }

        Token* endStmt{};
        if (const Token* strTok = getStrTok(tok, addlength, endStmt)) {
            const int sz = Token::getStrArraySize(strTok);
            tok->next()->insertToken(std::to_string(sz));
            tok = endStmt;
        }

        else if (Token::Match(tok, "%var% [ ] = {")) {
            MathLib::biguint sz = 1;
            tok = tok->next();
            Token *end = tok->linkAt(3);
            for (Token *tok2 = tok->tokAt(4); tok2 && tok2 != end; tok2 = tok2->next()) {
                if (tok2->link() && Token::Match(tok2, "{|(|[|<")) {
                    if (tok2->str() == "[" && tok2->link()->strAt(1) == "=") { // designated initializer
                        if (Token::Match(tok2, "[ %num% ]"))
                            sz = std::max(sz, MathLib::toBigUNumber(tok2->tokAt(1)) + 1U);
                        else {
                            sz = 0;
                            break;
                        }
                    }
                    tok2 = tok2->link();
                } else if (tok2->str() == ",") {
                    if (!Token::Match(tok2->next(), "[},]"))
                        ++sz;
                    else {
                        tok2 = tok2->previous();
                        tok2->deleteNext();
                    }
                }
            }

            if (sz != 0)
                tok->insertToken(MathLib::toString(sz));

            tok = end->next() ? end->next() : end;
        }
    }
}

// cppcheck-suppress functionConst
void Tokenizer::arraySizeAfterValueFlow()
{
    // After ValueFlow, adjust array sizes.
    for (const Variable* var: mSymbolDatabase->variableList()) {
        if (!var || !var->isArray())
            continue;
        if (!Token::Match(var->nameToken(), "%name% [ ] = { ["))
            continue;
        MathLib::bigint maxIndex = -1;
        const Token* const startToken = var->nameToken()->tokAt(4);
        const Token* const endToken = startToken->link();
        for (const Token* tok = startToken; tok != endToken; tok = tok->next()) {
            if (!Token::Match(tok, "[{,] [") || !Token::simpleMatch(tok->linkAt(1), "] ="))
                continue;
            const Token* expr = tok->next()->astOperand1();
            if (expr && expr->hasKnownIntValue())
                maxIndex = std::max(maxIndex, expr->getKnownIntValue());
        }
        if (maxIndex >= 0) {
            // insert array size
            auto* tok = const_cast<Token*>(var->nameToken()->next());
            tok->insertToken(MathLib::toString(maxIndex + 1));
            // ast
            tok->astOperand2(tok->next());
            // Token::scope
            tok->next()->scope(tok->scope());
            // Value flow
            ValueFlow::Value value(maxIndex + 1);
            value.setKnown();
            tok->next()->addValue(value);
            // Set array dimensions
            Dimension d;
            d.num = maxIndex + 1;
            std::vector<Dimension> dimensions{d};
            const_cast<Variable*>(var)->setDimensions(dimensions);
        }
    }
}

static Token *skipTernaryOp(Token *tok)
{
    int colonLevel = 1;
    while (nullptr != (tok = tok->next())) {
        if (tok->str() == "?") {
            ++colonLevel;
        } else if (tok->str() == ":") {
            --colonLevel;
            if (colonLevel == 0) {
                tok = tok->next();
                break;
            }
        }
        if (tok->link() && Token::Match(tok, "[(<]"))
            tok = tok->link();
        else if (Token::Match(tok->next(), "[{};)]"))
            break;
    }
    if (colonLevel > 0) // Ticket #5214: Make sure the ':' matches the proper '?'
        return nullptr;
    return tok;
}

// Skips until the colon at the end of the case label, the argument must point to the "case" token.
// In case of success returns the colon token.
// In case of failure returns the token that caused the error.
static Token *skipCaseLabel(Token *tok)
{
    assert(tok->str() == "case");
    while (nullptr != (tok = tok->next())) {
        if (Token::Match(tok, "(|["))
            tok = tok->link();
        else if (tok->str() == "?") {
            Token * tok1 = skipTernaryOp(tok);
            if (!tok1)
                return tok;
            tok = tok1;
        }
        if (Token::Match(tok, "[:{};]"))
            return tok;
    }
    return nullptr;
}

const Token * Tokenizer::startOfExecutableScope(const Token * tok)
{
    if (tok->str() != ")")
        return nullptr;

    tok = TokenList::isFunctionHead(tok, ":{");

    if (Token::Match(tok, ": %name% [({]")) {
        while (Token::Match(tok, "[:,] %name% [({]"))
            tok = tok->linkAt(2)->next();
    }

    return (tok && tok->str() == "{") ? tok : nullptr;
}


/** simplify labels and case|default in the code: add a ";" if not already in.*/

void Tokenizer::simplifyLabelsCaseDefault()
{
    const bool cpp = isCPP();
    bool executablescope = false;
    int indentLevel = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // Simplify labels in the executable scope..
        auto *start = const_cast<Token *>(startOfExecutableScope(tok));
        if (start) {
            tok = start;
            executablescope = true;
        }

        if (!executablescope)
            continue;

        if (tok->str() == "{") {
            if (tok->strAt(-1) == "=")
                tok = tok->link();
            else
                ++indentLevel;
        } else if (tok->str() == "}") {
            --indentLevel;
            if (indentLevel == 0) {
                executablescope = false;
                continue;
            }
        } else if (Token::Match(tok, "(|["))
            tok = tok->link();

        if (Token::Match(tok, "[;{}:] case")) {
            tok = skipCaseLabel(tok->next());
            if (!tok)
                break;
            if (tok->str() != ":" || tok->strAt(-1) == "case" || !tok->next())
                syntaxError(tok);
            if (tok->strAt(1) != ";" && tok->strAt(1) != "case")
                tok->insertToken(";");
            else
                tok = tok->previous();
        } else if (Token::Match(tok, "[;{}] %name% : !!;")) {
            if (!cpp || !Token::Match(tok->next(), "class|struct|enum")) {
                tok = tok->tokAt(2);
                tok->insertToken(";");
            }
        }
    }
}


void Tokenizer::simplifyCaseRange()
{
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "case %num%|%char% ... %num%|%char% :")) {
            const MathLib::bigint start = MathLib::toBigNumber(tok->tokAt(1));
            MathLib::bigint end = MathLib::toBigNumber(tok->tokAt(3));
            end = std::min(start + 50, end); // Simplify it 50 times at maximum
            if (start < end) {
                tok = tok->tokAt(2);
                tok->str(":");
                tok->insertToken("case");
                for (MathLib::bigint i = end-1; i > start; i--) {
                    tok->insertToken(":");
                    tok->insertToken(MathLib::toString(i));
                    tok->insertToken("case");
                }
            }
        }
    }
}

void Tokenizer::calculateScopes()
{
    for (auto *tok = list.front(); tok; tok = tok->next())
        tok->scopeInfo(nullptr);

    std::string nextScopeNameAddition;
    std::shared_ptr<ScopeInfo2> primaryScope = std::make_shared<ScopeInfo2>("", nullptr);
    list.front()->scopeInfo(std::move(primaryScope));

    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (tok == list.front() || !tok->scopeInfo()) {
            if (tok != list.front())
                tok->scopeInfo(tok->previous()->scopeInfo());

            if (Token::Match(tok, "using namespace %name% ::|<|;")) {
                std::string usingNamespaceName;
                for (const Token* namespaceNameToken = tok->tokAt(2);
                     namespaceNameToken && namespaceNameToken->str() != ";";
                     namespaceNameToken = namespaceNameToken->next()) {
                    usingNamespaceName += namespaceNameToken->str();
                    usingNamespaceName += " ";
                }
                if (!usingNamespaceName.empty())
                    usingNamespaceName.pop_back();
                tok->scopeInfo()->usingNamespaces.insert(std::move(usingNamespaceName));
            } else if (Token::Match(tok, "namespace|class|struct|union %name% {|::|:|<")) {
                for (Token* nameTok = tok->next(); nameTok && !Token::Match(nameTok, "{|:"); nameTok = nameTok->next()) {
                    if (Token::Match(nameTok, ";|<")) {
                        nextScopeNameAddition = "";
                        break;
                    }
                    nextScopeNameAddition.append(nameTok->str());
                    nextScopeNameAddition.append(" ");
                }
                if (!nextScopeNameAddition.empty())
                    nextScopeNameAddition.pop_back();
            }

            if (Token::simpleMatch(tok, "{")) {
                // This might be the opening of a member function
                Token *tok1 = tok;
                while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                    tok1 = tok1->previous();
                if (tok1->previous() && tok1->strAt(-1) == ")") {
                    bool member = true;
                    tok1 = tok1->linkAt(-1);
                    if (Token::Match(tok1->previous(), "throw|noexcept")) {
                        tok1 = tok1->previous();
                        while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                            tok1 = tok1->previous();
                        if (tok1->strAt(-1) != ")")
                            member = false;
                    } else if (Token::Match(tok->tokAt(-2), ":|, %name%")) {
                        tok1 = tok1->tokAt(-2);
                        if (tok1->strAt(-1) != ")")
                            member = false;
                    }
                    if (member) {
                        if (tok1->strAt(-1) == ">")
                            tok1 = tok1->previous()->findOpeningBracket();
                        if (tok1 && Token::Match(tok1->tokAt(-3), "%name% :: %name%")) {
                            tok1 = tok1->tokAt(-2);
                            std::string scope = tok1->strAt(-1);
                            while (Token::Match(tok1->tokAt(-2), ":: %name%")) {
                                scope = tok1->strAt(-3) + " :: " + scope;
                                tok1 = tok1->tokAt(-2);
                            }

                            if (!nextScopeNameAddition.empty() && !scope.empty())
                                nextScopeNameAddition += " :: ";
                            nextScopeNameAddition += scope;
                        }
                    }
                }

                // New scope is opening, record it here
                std::shared_ptr<ScopeInfo2> newScopeInfo = std::make_shared<ScopeInfo2>(tok->scopeInfo()->name, tok->link(), tok->scopeInfo()->usingNamespaces);

                if (!newScopeInfo->name.empty() && !nextScopeNameAddition.empty())
                    newScopeInfo->name.append(" :: ");
                newScopeInfo->name.append(nextScopeNameAddition);
                nextScopeNameAddition = "";

                if (tok->link())
                    tok->link()->scopeInfo(tok->scopeInfo());
                tok->scopeInfo(std::move(newScopeInfo));
            }
        }
    }
}

void Tokenizer::simplifyTemplates()
{
    if (isC())
        return;

    const std::time_t maxTime = mSettings.templateMaxTime > 0 ? std::time(nullptr) + mSettings.templateMaxTime : 0;
    mTemplateSimplifier->simplifyTemplates(
        maxTime);
}
//---------------------------------------------------------------------------


namespace {
    /** Class used in Tokenizer::setVarIdPass1 */
    class VariableMap {
    public:
        struct VarInfo { nonneg int id{}; bool assigned{}; };
        using MapType = std::unordered_map<std::string, VarInfo>;
    private:
        MapType mVariableId;
        MapType mVariableId_global;
        std::stack<std::vector<MapType::value_type>> mScopeInfo;
        mutable nonneg int mVarId{};
    public:
        VariableMap() = default;
        void enterScope();
        bool leaveScope();
        void addVariable(const std::string& varname, bool globalNamespace);
        bool hasVariable(const std::string& varname) const {
            return mVariableId.find(varname) != mVariableId.end();
        }

        const MapType& map(bool global) const {
            return global ? mVariableId_global : mVariableId;
        }
        MapType& map(bool global) {
            return global ? mVariableId_global : mVariableId;
        }
        nonneg int& getVarId() {
            return mVarId;
        }
    };
}


void VariableMap::enterScope()
{
    mScopeInfo.emplace(/*std::vector<std::pair<std::string, nonneg int>>()*/);
}

bool VariableMap::leaveScope()
{
    if (mScopeInfo.empty())
        return false;

    for (const MapType::value_type& outerVariable : mScopeInfo.top()) {
        if (outerVariable.second.id != 0)
            mVariableId[outerVariable.first] = outerVariable.second;
        else
            mVariableId.erase(outerVariable.first);
    }
    mScopeInfo.pop();
    return true;
}

void VariableMap::addVariable(const std::string& varname, bool globalNamespace)
{
    if (mScopeInfo.empty()) {
        mVariableId[varname].id = ++mVarId;
        if (globalNamespace)
            mVariableId_global[varname] = mVariableId[varname];
        return;
    }
    const auto it = mVariableId.find(varname);
    if (it == mVariableId.end()) {
        mScopeInfo.top().emplace_back(varname, VarInfo{});
        mVariableId[varname].id = ++mVarId;
        if (globalNamespace)
            mVariableId_global[varname] = mVariableId[varname];
        return;
    }
    mScopeInfo.top().emplace_back(varname, it->second);
    it->second.id = ++mVarId;
    it->second.assigned = false;
}

/**
 * @throws Token* thrown when closing brackets are missing
 */
static bool setVarIdParseDeclaration(Token*& tok, const VariableMap& variableMap, bool executableScope, Standards::cstd_t cStandard)
{
    const Token* const tok1 = tok;
    Token* tok2 = tok;
    if (!tok2->isName() || (tok2->tokType() != Token::eType && tok2->tokType() != Token::eName && tok2->tokType() != Token::eKeyword))
        return false;

    nonneg int typeCount = 0;
    nonneg int singleNameCount = 0;
    bool hasstruct = false;   // Is there a "struct" or "class"?
    bool bracket = false;
    bool ref = false;
    while (tok2) {
        if (tok2->isName()) {
            if (Token::simpleMatch(tok2, "alignas (")) {
                tok2 = tok2->linkAt(1)->next();
                continue;
            }
            if (tok2->isCpp() && Token::Match(tok2, "namespace|public|private|protected"))
                return false;
            bool isC23 = tok2->isC() && cStandard >= Standards::C23;
            if (((tok2->isCpp() || isC23) && Token::Match(tok2, "decltype|typeof (")) ||
                (tok2->isC() && Token::simpleMatch(tok2, "__typeof ("))) {
                typeCount = 1;
                tok2 = tok2->linkAt(1)->next();
                continue;
            }
            if (Token::Match(tok2, "struct|union|enum") || (tok2->isCpp() && Token::Match(tok2, "class|typename"))) {
                hasstruct = true;
                typeCount = 0;
                singleNameCount = 0;
            } else if (Token::Match(tok2, "const|extern")) {
                // just skip "const", "extern"
            } else if (!hasstruct && variableMap.map(false).count(tok2->str()) && tok2->strAt(-1) != "::") {
                ++typeCount;
                tok2 = tok2->next();
                if (!tok2 || tok2->str() != "::")
                    break;
            } else {
                if (tok2->str() != "void" || Token::Match(tok2, "void const| *|(")) // just "void" cannot be a variable type
                    ++typeCount;
                ++singleNameCount;
            }
        } else if (tok2->isCpp() && ((TemplateSimplifier::templateParameters(tok2) > 0) ||
                                     Token::simpleMatch(tok2, "< >") /* Ticket #4764 */)) {
            const Token *start = tok;
            if (Token::Match(start->previous(), "%or%|%oror%|&&|&|^|+|-|*|/"))
                return false;
            Token* const closingBracket = tok2->findClosingBracket();
            if (closingBracket == nullptr) { /* Ticket #8151 */
                throw tok2;
            }
            tok2 = closingBracket;
            if (tok2->str() != ">")
                break;
            singleNameCount = 1;
            if (Token::Match(tok2, "> %name% %or%|%oror%|&&|&|^|+|-|*|/") && !Token::Match(tok2, "> const [*&]"))
                return false;
            if (Token::Match(tok2, "> %name% )")) {
                if (Token::Match(tok2->linkAt(2)->previous(), "if|for|while ("))
                    return false;
                if (!Token::Match(tok2->linkAt(2)->previous(), "%name%|] ("))
                    return false;
            }
        } else if (Token::Match(tok2, "&|&&")) {
            ref = !bracket;
        } else if (singleNameCount >= 1 && Token::Match(tok2, "( [*&]") && Token::Match(tok2->link(), ") (|[")) {
            for (const Token* tok3 = tok2->tokAt(2); Token::Match(tok3, "!!)"); tok3 = tok3->next()) {
                if (Token::Match(tok3, "(|["))
                    tok3 = tok3->link();
                if (tok3->str() == ",")
                    return false;
            }
            bracket = true; // Skip: Seems to be valid pointer to array or function pointer
        } else if (singleNameCount >= 1 && Token::Match(tok2, "( * %name% [") && Token::Match(tok2->linkAt(3), "] ) [;,]") && !variableMap.map(false).count(tok2->strAt(2))) {
            bracket = true;
        } else if (singleNameCount >= 1 && tok2->previous() && tok2->previous()->isStandardType() && Token::Match(tok2, "( *|&| %name% ) ;")) {
            bracket = true;
        } else if (tok2->str() == "::") {
            singleNameCount = 0;
        } else if (tok2->str() != "*" && tok2->str() != "...") {
            break;
        }
        tok2 = tok2->next();
    }

    if (tok2) {
        bool isLambdaArg = false;
        {
            const Token *tok3 = tok->previous();
            if (tok3 && tok3->str() == ",") {
                while (tok3 && !Token::Match(tok3,";|(|[|{")) {
                    if (Token::Match(tok3, ")|]"))
                        tok3 = tok3->link();
                    tok3 = tok3->previous();
                }

                if (tok3 && executableScope && Token::Match(tok3->previous(), "%name% (")) {
                    const Token *fdecl = tok3->previous();
                    int count = 0;
                    while (Token::Match(fdecl, "%name%|*")) {
                        fdecl = fdecl->previous();
                        count++;
                    }
                    if (!Token::Match(fdecl, "[;{}] %name%") || count <= 1)
                        return false;
                }
            }

            if (tok3 && tok3->isCpp() && Token::simpleMatch(tok3->previous(), "] (") &&
                (Token::simpleMatch(tok3->link(), ") {") || Token::Match(tok3->link(), ") . %name%")))
                isLambdaArg = true;
        }


        tok = tok2;

        // In executable scopes, references must be assigned
        // Catching by reference is an exception
        if (executableScope && ref && !isLambdaArg) {
            if (Token::Match(tok2, "(|=|{|:"))
                ;   // reference is assigned => ok
            else if (tok2->str() != ")" || tok2->link()->strAt(-1) != "catch")
                return false;   // not catching by reference => not declaration
        }
    }

    // Check if array declaration is valid (#2638)
    // invalid declaration: AAA a[4] = 0;
    if (typeCount >= 2 && executableScope && Token::Match(tok2, ")| [")) {
        const Token *tok3 = tok2->str() == ")" ? tok2->next() : tok2;
        while (tok3 && tok3->str() == "[") {
            tok3 = tok3->link()->next();
        }
        if (Token::Match(tok3, "= %num%"))
            return false;
        if (bracket && Token::Match(tok1->previous(), "[(,]") && Token::Match(tok3, ",|)|%cop%"))
            return false;
    }

    return (typeCount >= 2 && tok2 && Token::Match(tok2->tokAt(-2), "!!:: %type%"));
}


static void setVarIdStructMembers(Token *&tok1,
                                  std::map<nonneg int, std::map<std::string, nonneg int>>& structMembers,
                                  nonneg int &varId)
{
    Token *tok = tok1;

    if (Token::Match(tok, "%name% = { . %name% =|{")) {
        const nonneg int struct_varid = tok->varId();
        if (struct_varid == 0)
            return;

        std::map<std::string, nonneg int>& members = structMembers[struct_varid];

        tok = tok->tokAt(3);
        while (tok->str() != "}") {
            if (Token::Match(tok, "{|[|("))
                tok = tok->link();
            if (Token::Match(tok->previous(), "[,{] . %name% =|{")) {
                tok = tok->next();
                const auto it = utils::as_const(members).find(tok->str());
                if (it == members.cend()) {
                    members[tok->str()] = ++varId;
                    tok->varId(varId);
                } else {
                    tok->varId(it->second);
                }
            }
            tok = tok->next();
        }

        return;
    }

    while (Token::Match(tok->next(), ")| . %name% !!(")) {
        // Don't set varid for trailing return type
        if (tok->strAt(1) == ")" && Token::Match(tok->linkAt(1)->tokAt(-1), "%name%|]") && !tok->linkAt(1)->tokAt(-1)->isKeyword() &&
            TokenList::isFunctionHead(tok->linkAt(1), "{;")) {
            tok = tok->tokAt(3);
            continue;
        }
        const nonneg int struct_varid = tok->varId();
        tok = tok->tokAt(2);
        if (struct_varid == 0)
            continue;

        if (tok->str() == ".")
            tok = tok->next();

        // Don't set varid for template function
        if (TemplateSimplifier::templateParameters(tok->next()) > 0)
            break;

        std::map<std::string, nonneg int>& members = structMembers[struct_varid];
        const auto it = utils::as_const(members).find(tok->str());
        if (it == members.cend()) {
            members[tok->str()] = ++varId;
            tok->varId(varId);
        } else {
            tok->varId(it->second);
        }
    }
    // tok can't be null
    tok1 = tok;
}

static void addTemplateVarIdUsage(const std::string &tokstr,
                                  const std::map<std::string, std::set<std::string>>& templateVarUsage,
                                  const VariableMap::MapType& variableMap,
                                  std::set<nonneg int>& templateVarIdUsage) {
    const auto v = templateVarUsage.find(tokstr);
    if (v != templateVarUsage.end()) {
        for (const std::string& varname: v->second) {
            const auto it = variableMap.find(varname);
            if (it != variableMap.end())
                templateVarIdUsage.insert(it->second.id);
        }
    }
}

static bool setVarIdClassDeclaration(Token* const startToken,
                                     VariableMap& variableMap,
                                     const nonneg int scopeStartVarId,
                                     const std::map<std::string, std::set<std::string>>& templateVarUsage,
                                     std::map<nonneg int, std::map<std::string, nonneg int>>& structMembers,
                                     std::set<nonneg int>& templateVarIdUsage)
{
    // end of scope
    const Token* const endToken = startToken->link();

    // determine class name
    std::string className;
    for (const Token *tok = startToken->previous(); tok; tok = tok->previous()) {
        if (!tok->isName() && tok->str() != ":")
            break;
        if (Token::Match(tok, "class|struct|enum %type% [:{]")) {
            className = tok->strAt(1);
            break;
        }
    }

    // replace varids..
    int indentlevel = 0;
    bool initList = false;
    bool inEnum = false;
    const Token *initListArgLastToken = nullptr;
    for (Token *tok = startToken->next(); tok != endToken; tok = tok->next()) {
        if (!tok)
            return false;
        if (initList) {
            if (tok == initListArgLastToken)
                initListArgLastToken = nullptr;
            else if (!initListArgLastToken &&
                     Token::Match(tok->previous(), "%name%|>|>> {|(") &&
                     Token::Match(tok->link(), "}|) ,|{"))
                initListArgLastToken = tok->link();
        }
        if (tok->str() == "{") {
            inEnum = isEnumStart(tok);
            if (!inEnum && isClassStructUnionEnumStart(tok)) { // nested type
                tok = tok->link();
                continue;
            }
            if (initList && !initListArgLastToken)
                initList = false;
            ++indentlevel;
        } else if (tok->str() == "}") {
            --indentlevel;
            inEnum = false;
        } else if (initList && indentlevel == 0 && Token::Match(tok->previous(), "[,:] %name% [({]")) {
            const auto it = variableMap.map(false).find(tok->str());
            if (it != variableMap.map(false).end()) {
                tok->varId(it->second.id);
            }
        } else if (tok->isName() && tok->varId() <= scopeStartVarId) {
            if (indentlevel > 0 || initList) {
                if (Token::Match(tok->previous(), "::|.") && tok->strAt(-2) != "this" && !Token::simpleMatch(tok->tokAt(-5), "( * this ) ."))
                    continue;
                if (!tok->next())
                    return false;
                if (tok->strAt(1) == "::") {
                    if (tok->str() == className)
                        tok = tok->tokAt(2);
                    else
                        continue;
                }

                if (!inEnum) {
                    const auto it = variableMap.map(false).find(tok->str());
                    if (it != variableMap.map(false).end()) {
                        tok->varId(it->second.id);
                        setVarIdStructMembers(tok, structMembers, variableMap.getVarId());
                    } else if (tok->str().back() == '>') {
                        addTemplateVarIdUsage(tok->str(), templateVarUsage, variableMap.map(false), templateVarIdUsage);
                    }
                }
            }
        } else if (indentlevel == 0 && tok->str() == ":" && !initListArgLastToken)
            initList = true;
    }
    return true;
}



// Update the variable ids..
// Parse each function..
void Tokenizer::setVarIdClassFunction(const std::string &classname,
                                      Token * const startToken,
                                      const Token * const endToken,
                                      const std::map<std::string, nonneg int> &varlist,
                                      std::map<nonneg int, std::map<std::string, nonneg int>>& structMembers,
                                      nonneg int &varId_)
{
    const auto pos = classname.rfind(' '); // TODO handle multiple scopes
    const std::string lastScope = classname.substr(pos == std::string::npos ? 0 : pos + 1);
    for (Token *tok2 = startToken; tok2 && tok2 != endToken; tok2 = tok2->next()) {
        if (tok2->varId() != 0 || !tok2->isName())
            continue;
        if (Token::Match(tok2->tokAt(-2), ("!!" + lastScope + " ::").c_str()))
            continue;
        if (Token::Match(tok2->tokAt(-4), "%name% :: %name% ::")) // Currently unsupported
            continue;
        if (Token::Match(tok2->tokAt(-2), "!!this .") && !Token::simpleMatch(tok2->tokAt(-5), "( * this ) ."))
            continue;
        if (Token::Match(tok2, "%name% ::"))
            continue;

        const auto it = varlist.find(tok2->str());
        if (it != varlist.end()) {
            tok2->varId(it->second);
            setVarIdStructMembers(tok2, structMembers, varId_);
        }
    }
}



void Tokenizer::setVarId()
{
    // Clear all variable ids
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isName())
            tok->varId(0);
    }

    setVarIdPass1();

    setPodTypes();

    setVarIdPass2();
}


// Variable declarations can't start with "return" etc.
#define NOTSTART_C "NOT", "case", "default", "goto", "not", "return", "sizeof", "typedef"
static const std::unordered_set<std::string> notstart_c = { NOTSTART_C };
static const std::unordered_set<std::string> notstart_cpp = { NOTSTART_C,
                                                              "delete", "friend", "new", "throw", "using", "virtual", "explicit", "const_cast", "dynamic_cast", "reinterpret_cast", "static_cast", "template"
};

void Tokenizer::setVarIdPass1()
{
    const bool cpp = isCPP();
    // Variable declarations can't start with "return" etc.
    const std::unordered_set<std::string>& notstart = (isC()) ? notstart_c : notstart_cpp;

    VariableMap variableMap;
    std::map<nonneg int, std::map<std::string, nonneg int>> structMembers;

    std::stack<VarIdScopeInfo> scopeStack;

    scopeStack.emplace(/*VarIdScopeInfo()*/);
    std::stack<const Token *> functionDeclEndStack;
    const Token *functionDeclEndToken = nullptr;
    bool initlist = false;
    bool inlineFunction = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isOp())
            continue;
        if (cpp && Token::simpleMatch(tok, "template <")) {
            Token* closingBracket = tok->next()->findClosingBracket();
            if (closingBracket)
                tok = closingBracket;
            continue;
        }

        if (tok == functionDeclEndToken) {
            functionDeclEndStack.pop();
            functionDeclEndToken = functionDeclEndStack.empty() ? nullptr : functionDeclEndStack.top();
            if (tok->str() == ":")
                initlist = true;
            else if (tok->str() == ";") {
                if (!variableMap.leaveScope())
                    cppcheckError(tok);
            } else if (tok->str() == "{") {
                scopeStack.emplace(true, scopeStack.top().isStructInit || tok->strAt(-1) == "=", /*isEnum=*/ false, variableMap.getVarId());

                // check if this '{' is a start of an "if" body
                const Token * ifToken = tok->previous();
                if (ifToken && ifToken->str() == ")")
                    ifToken = ifToken->link();
                else
                    ifToken = nullptr;
                if (ifToken)
                    ifToken = ifToken->previous();
                if (ifToken && ifToken->str() == "if") {
                    // open another scope to differentiate between variables declared in the "if" condition and in the "if" body
                    variableMap.enterScope();
                }
            }
        } else if (!initlist && tok->str()=="(") {
            const Token * newFunctionDeclEnd = nullptr;
            if (!scopeStack.top().isExecutable)
                newFunctionDeclEnd = TokenList::isFunctionHead(tok, "{:;");
            else {
                const Token* tokenLinkNext = tok->link()->next();
                if (Token::simpleMatch(tokenLinkNext, ".")) { // skip trailing return type
                    tokenLinkNext = tokenLinkNext->next();
                    while (Token::Match(tokenLinkNext, "%name%|::")) {
                        tokenLinkNext = tokenLinkNext->next();
                        if (Token::simpleMatch(tokenLinkNext, "<") && tokenLinkNext->link())
                            tokenLinkNext = tokenLinkNext->link()->next();
                    }
                }
                if (tokenLinkNext && tokenLinkNext->str() == "{") // might be for- or while-loop or if-statement
                    newFunctionDeclEnd = tokenLinkNext;
            }
            if (newFunctionDeclEnd && newFunctionDeclEnd != functionDeclEndToken) {
                functionDeclEndStack.push(newFunctionDeclEnd);
                functionDeclEndToken = newFunctionDeclEnd;
                variableMap.enterScope();
            }
        } else if (Token::Match(tok, "{|}")) {
            inlineFunction = false;

            const Token * const startToken = (tok->str() == "{") ? tok : tok->link();

            // parse anonymous namespaces as part of the current scope
            if (!Token::Match(startToken->previous(), "union|struct|enum|namespace {") &&
                !(initlist && Token::Match(startToken->previous(), "%name%|>|>>|(") && Token::Match(startToken->link(), "} ,|{|)|..."))) {

                if (tok->str() == "{") {
                    bool isExecutable;
                    const Token *prev = tok->previous();
                    while (Token::Match(prev, "%name%|."))
                        prev = prev->previous();
                    const bool isLambda = prev && prev->str() == ")" && Token::simpleMatch(prev->link()->previous(), "] (");
                    if ((!isLambda && (tok->strAt(-1) == ")" || Token::Match(tok->tokAt(-2), ") %type%"))) ||
                        (initlist && tok->strAt(-1) == "}")) {
                        isExecutable = true;
                    } else {
                        isExecutable = ((scopeStack.top().isExecutable || initlist || tok->strAt(-1) == "else") &&
                                        !isClassStructUnionEnumStart(tok));
                        if (!(scopeStack.top().isStructInit || tok->strAt(-1) == "="))
                            variableMap.enterScope();
                    }
                    const bool isStructInit = scopeStack.top().isStructInit || tok->strAt(-1) == "=" || (initlist && !Token::Match(tok->tokAt(-1), ")|}|..."));
                    scopeStack.emplace(isExecutable, isStructInit, isEnumStart(tok), variableMap.getVarId());
                    initlist = false;
                } else { /* if (tok->str() == "}") */
                    bool isNamespace = false;
                    for (const Token *tok1 = tok->link()->previous(); tok1 && tok1->isName(); tok1 = tok1->previous()) {
                        if (tok1->str() == "namespace") {
                            isNamespace = true;
                            break;
                        }
                    }
                    // Set variable ids in class declaration..
                    if (!initlist && !isC() && !scopeStack.top().isExecutable && tok->link() && !isNamespace) {
                        if (!setVarIdClassDeclaration(tok->link(),
                                                      variableMap,
                                                      scopeStack.top().startVarid,
                                                      mTemplateSimplifier->getUsedVariables(),
                                                      structMembers,
                                                      mTemplateVarIdUsage)) {
                            syntaxError(nullptr);
                        }
                    }

                    if (!scopeStack.top().isStructInit) {
                        variableMap.leaveScope();

                        // check if this '}' is an end of an "else" body or an "if" body without an "else" part
                        const Token * ifToken = startToken->previous();
                        if (ifToken && ifToken->str() == ")")
                            ifToken = ifToken->link()->previous();
                        else
                            ifToken = nullptr;
                        if (startToken->strAt(-1) == "else" || (ifToken && ifToken->str() == "if" && tok->strAt(1) != "else")) {
                            // leave the extra scope used to differentiate between variables declared in the "if" condition and in the "if" body
                            variableMap.leaveScope();
                        }
                    }

                    scopeStack.pop();
                    if (scopeStack.empty()) {  // should be impossible
                        scopeStack.emplace(/*VarIdScopeInfo()*/);
                    }
                }
            }
        }

        if ((!scopeStack.top().isStructInit &&
             (tok == list.front() ||
              Token::Match(tok, "[;{}]") ||
              (tok->str() == "(" && !scopeStack.top().isExecutable && TokenList::isFunctionHead(tok,";:")) ||
              (tok->str() == "," && (!scopeStack.top().isExecutable || inlineFunction || !tok->previous()->varId())) ||
              (tok->isName() && endsWith(tok->str(), ':')))) ||
            (tok->str() == "(" && TokenList::isFunctionHead(tok, "{"))) {

            // No variable declarations in sizeof
            if (Token::simpleMatch(tok->previous(), "sizeof (")) {
                continue;
            }

            if (Settings::terminated())
                return;

            // locate the variable name..
            Token* tok2 = (tok->isName()) ? tok : tok->next();

            // private: protected: public: etc
            while (tok2 && endsWith(tok2->str(), ':')) {
                tok2 = tok2->next();
            }
            if (!tok2)
                break;

            // Variable declaration can't start with "return", etc
            if (notstart.find(tok2->str()) != notstart.end())
                continue;

            if (!isC() && Token::simpleMatch(tok2, "const new"))
                continue;

            bool decl;
            if (cpp && mSettings.standards.cpp >= Standards::CPP17 && Token::Match(tok, "[(;{}] const| auto &|&&| [")) {
                // Structured bindings
                tok2 = Token::findsimplematch(tok, "[");
                if ((Token::simpleMatch(tok->previous(), "for (") && Token::simpleMatch(tok2->link(), "] :")) ||
                    Token::simpleMatch(tok2->link(), "] =")) {
                    while (tok2 && tok2->str() != "]") {
                        if (Token::Match(tok2, "%name% [,]]"))
                            variableMap.addVariable(tok2->str(), false);
                        tok2 = tok2->next();
                    }
                    continue;
                }
            }

            try { /* Ticket #8151 */
                decl = setVarIdParseDeclaration(tok2, variableMap, scopeStack.top().isExecutable, mSettings.standards.c);
            } catch (const Token * errTok) {
                syntaxError(errTok);
            }

            if (tok->str() == "(" && TokenList::isFunctionHead(tok, "{") && scopeStack.top().isExecutable)
                inlineFunction = true;

            if (decl) {
                if (cpp) {
                    for (const Token* tok3 = tok->next(); tok3->isName(); tok3 = tok3->next()) {
                        addTemplateVarIdUsage(tok3->str(),
                                              mTemplateSimplifier->getUsedVariables(),
                                              variableMap.map(false),
                                              mTemplateVarIdUsage);
                        addTemplateVarIdUsage(tok3->str(),
                                              mTemplateSimplifier->getUsedVariables(),
                                              variableMap.map(true),
                                              mTemplateVarIdUsage);
                        if (!tok3->next())
                            syntaxError(tok3);
                    }
                }

                Token *declTypeTok = nullptr;
                if (cpp || mSettings.standards.c >= Standards::C23) {
                    declTypeTok = Token::findmatch(tok, "decltype|typeof (", tok2);
                } else {
                    declTypeTok = Token::findsimplematch(tok, "__typeof (", tok2);
                }

                if (declTypeTok) {
                    for (Token *declTok = declTypeTok->linkAt(1); declTok != declTypeTok; declTok = declTok->previous()) {
                        if (declTok->isName() && !Token::Match(declTok->previous(), "::|.") && variableMap.hasVariable(declTok->str()))
                            declTok->varId(variableMap.map(false).find(declTok->str())->second.id);
                    }
                }

                const Token* prev2 = tok2->previous();
                if (Token::Match(prev2, "%type% [;[=,)]") && tok2->strAt(-1) != "const")
                    ;
                else if (Token::Match(prev2, "%type% [:({]") && tok->strAt(-1) == "for")
                    ;
                else if (Token::Match(prev2, "%type% ( !!)") && Token::simpleMatch(tok2->link(), ") ;")) {
                    // In C++ , a variable can't be called operator+ or something like that.
                    if (cpp &&
                        prev2->isOperatorKeyword())
                        continue;

                    const Token *tok3 = tok2->next();
                    if (!tok3->isStandardType() && tok3->str() != "void" && !Token::Match(tok3, "struct|union|class %type%") && tok3->str() != "." && !Token::Match(tok2->link()->previous(), "[&*]")) {
                        if (!scopeStack.top().isExecutable) {
                            // Detecting initializations with () in non-executable scope is hard and often impossible to be done safely. Thus, only treat code as a variable that definitely is one.
                            decl = false;
                            bool rhs = false;
                            for (; tok3; tok3 = tok3->nextArgumentBeforeCreateLinks2()) {
                                if (tok3->str() == "=") {
                                    rhs = true;
                                    continue;
                                }

                                if (tok3->str() == ",") {
                                    rhs = false;
                                    continue;
                                }

                                if (rhs)
                                    continue;

                                if (tok3->isLiteral() ||
                                    (tok3->isName() && (variableMap.hasVariable(tok3->str()) ||
                                                        (tok3->strAt(-1) == "(" && Token::simpleMatch(tok3->next(), "(") && !Token::simpleMatch(tok3->linkAt(1)->next(), "(")))) ||
                                    tok3->isOp() ||
                                    tok3->str() == "(" ||
                                    notstart.find(tok3->str()) != notstart.end()) {
                                    decl = true;
                                    break;
                                }
                            }
                        }
                    } else
                        decl = false;
                } else if (cpp && Token::Match(prev2, "%type% {") && Token::simpleMatch(tok2->link(), "} ;")) { // C++11 initialization style
                    if (tok2->link() != tok2->next() && // add value-initialized variable T x{};
                        (Token::Match(prev2, "do|try|else") || Token::Match(prev2->tokAt(-2), "struct|class|:")))
                        continue;
                } else
                    decl = false;

                if (decl) {
                    if (isC() && Token::Match(prev2->previous(), "&|&&"))
                        syntaxErrorC(prev2, prev2->strAt(-2) + prev2->strAt(-1) + " " + prev2->str());
                    variableMap.addVariable(prev2->str(), scopeStack.size() <= 1);

                    if (Token::simpleMatch(tok->previous(), "for (") && Token::Match(prev2, "%name% [=[({,]")) {
                        for (const Token *tok3 = prev2->next(); tok3 && tok3->str() != ";"; tok3 = tok3->next()) {
                            if (Token::Match(tok3, "[([]"))
                                tok3 = tok3->link();
                            if (Token::Match(tok3, ", %name% [=[({,;]"))
                                variableMap.addVariable(tok3->strAt(1), false);
                        }
                    }

                    // set varid for template parameters..
                    tok = tok->next();
                    while (Token::Match(tok, "%name%|::"))
                        tok = tok->next();
                    if (tok && tok->str() == "<") {
                        const Token *end = tok->findClosingBracket();
                        while (tok != end) {
                            if (tok->isName() && !(Token::simpleMatch(tok->next(), "<") &&
                                                   Token::Match(tok->tokAt(-1), ":: %name%"))) {
                                const auto it = variableMap.map(false).find(tok->str());
                                if (it != variableMap.map(false).end())
                                    tok->varId(it->second.id);
                            }
                            tok = tok->next();
                        }
                    }

                    tok = tok2->previous();
                }
            }
        }

        if (tok->isName() && !tok->isKeyword() && !tok->isStandardType()) {
            // don't set variable id after a struct|enum|union
            if (Token::Match(tok->previous(), "struct|enum|union") || (cpp && tok->strAt(-1) == "class"))
                continue;

            bool globalNamespace = false;
            if (!isC()) {
                if (tok->previous() && tok->strAt(-1) == "::") {
                    if (Token::Match(tok->tokAt(-2), ")|]|%name%"))
                        continue;
                    globalNamespace = true;
                }
                if (tok->next() && tok->strAt(1) == "::")
                    continue;
                if (Token::simpleMatch(tok->tokAt(-2), ":: template"))
                    continue;
            }

            if (tok->str().back() == '>') {
                addTemplateVarIdUsage(tok->str(),
                                      mTemplateSimplifier->getUsedVariables(),
                                      variableMap.map(globalNamespace),
                                      mTemplateVarIdUsage);
            }

            // function declaration inside executable scope? Function declaration is of form: type name "(" args ")"
            if (scopeStack.top().isExecutable && !scopeStack.top().isStructInit && Token::Match(tok, "%name% [,)[]")) {
                bool par = false;
                const Token* start;
                Token* end;

                // search begin of function declaration
                for (start = tok; Token::Match(start, "%name%|*|&|,|("); start = start->previous()) {
                    if (start->str() == "(") {
                        if (par)
                            break;
                        par = true;
                    }
                    if (Token::Match(start, "[(,]")) {
                        if (!Token::Match(start, "[(,] %type% %name%|*|&"))
                            break;
                    }
                    if (start->varId() > 0)
                        break;
                }

                // search end of function declaration
                for (end = tok->next(); Token::Match(end, "%name%|*|&|,|[|]|%num%"); end = end->next()) {}

                // there are tokens which can't appear at the begin of a function declaration such as "return"
                const bool isNotstartKeyword = start->next() && notstart.find(start->strAt(1)) != notstart.end();

                // now check if it is a function (pointer) declaration
                if ((Token::simpleMatch(start, ") (") || Token::Match(start, "[;{}] %type% %name%|*")) && par && Token::Match(end, ") [;=]") && !isNotstartKeyword) {
                    // function declaration => don't set varid
                    tok = end;
                    continue;
                }
            }

            if (tok->varId() == 0 && (!scopeStack.top().isEnum || !(Token::Match(tok->previous(), "{|,") && Token::Match(tok->next(), ",|=|}"))) &&
                !Token::simpleMatch(tok->next(), ": ;") && !(tok->tokAt(-1) && Token::Match(tok->tokAt(-2), "{|, ."))) {
                const auto it = variableMap.map(globalNamespace).find(tok->str());
                if (it != variableMap.map(globalNamespace).end()) {
                    if (!it->second.assigned || !Token::Match(tok->previous(), "%type% %name% (") || tok->previous()->isKeyword()) {
                        it->second.assigned = true;
                        tok->varId(it->second.id);
                        setVarIdStructMembers(tok, structMembers, variableMap.getVarId());
                    }
                }
            }
        } else if (Token::Match(tok, "::|. %name%") && Token::Match(tok->previous(), ")|]|>|%name%")) {
            // Don't set varid after a :: or . token
            tok = tok->next();
        } else if (tok->str() == ":" && Token::Match(tok->tokAt(-2), "class %type%")) {
            do {
                tok = tok->next();
            } while (tok && (tok->isName() || tok->str() == ","));
            if (!tok)
                break;
            tok = tok->previous();
        }
    }

    mVarId = variableMap.getVarId();
}

namespace {
    struct Member {
        Member(std::list<std::string> s, std::list<const Token *> ns, Token *t) : usingnamespaces(std::move(ns)), scope(std::move(s)), tok(t) {}
        std::list<const Token *> usingnamespaces;
        std::list<std::string> scope;
        Token *tok;
    };
}

static std::string getScopeName(const std::list<ScopeInfo2> &scopeInfo)
{
    std::string ret;
    for (const ScopeInfo2 &si : scopeInfo)
        ret += (ret.empty() ? "" : " :: ") + (si.name);
    return ret;
}

static Token * matchMemberName(const std::list<std::string> &scope, const Token *nsToken, Token *memberToken, const std::list<ScopeInfo2> &scopeInfo)
{
    auto scopeIt = scopeInfo.cbegin();

    // Current scope..
    for (auto it = scope.cbegin(); it != scope.cend(); ++it) {
        if (scopeIt == scopeInfo.cend() || scopeIt->name != *it)
            return nullptr;
        ++scopeIt;
    }

    // using namespace..
    if (nsToken) {
        while (Token::Match(nsToken, "%name% ::")) {
            if (scopeIt != scopeInfo.end() && nsToken->str() == scopeIt->name) {
                nsToken = nsToken->tokAt(2);
                ++scopeIt;
            } else {
                return nullptr;
            }
        }
        if (!Token::Match(nsToken, "%name% ;"))
            return nullptr;
        if (scopeIt == scopeInfo.end() || nsToken->str() != scopeIt->name)
            return nullptr;
        ++scopeIt;
    }

    // Parse member tokens..
    while (scopeIt != scopeInfo.end()) {
        if (!Token::Match(memberToken, "%name% ::|<"))
            return nullptr;
        if (memberToken->str() != scopeIt->name)
            return nullptr;
        if (memberToken->strAt(1) == "<") {
            memberToken = memberToken->next()->findClosingBracket();
            if (!Token::simpleMatch(memberToken, "> ::"))
                return nullptr;
        }
        memberToken = memberToken->tokAt(2);
        ++scopeIt;
    }

    return Token::Match(memberToken, "~| %name%") ? memberToken : nullptr;
}

static Token * matchMemberName(const Member &member, const std::list<ScopeInfo2> &scopeInfo)
{
    if (scopeInfo.empty())
        return nullptr;

    // Does this member match without "using namespace"..
    Token *ret = matchMemberName(member.scope, nullptr, member.tok, scopeInfo);
    if (ret)
        return ret;

    // Try to match member using the "using namespace ..." namespaces..
    for (const Token *ns : member.usingnamespaces) {
        ret = matchMemberName(member.scope, ns, member.tok, scopeInfo);
        if (ret)
            return ret;
    }

    return nullptr;
}

static Token * matchMemberVarName(const Member &var, const std::list<ScopeInfo2> &scopeInfo)
{
    Token *tok = matchMemberName(var, scopeInfo);
    if (Token::Match(tok, "%name%")) {
        if (!tok->next() || tok->strAt(1) != "(" || (tok->tokAt(2) && tok->tokAt(2)->isLiteral()))
            return tok;
    }
    return nullptr;
}

static Token * matchMemberFunctionName(const Member &func, const std::list<ScopeInfo2> &scopeInfo)
{
    Token *tok = matchMemberName(func, scopeInfo);
    return Token::Match(tok, "~| %name% (") ? tok : nullptr;
}

template<typename T>
static T* skipInitializerList(T* tok)
{
    T* const start = tok;
    while (Token::Match(tok, "[:,] ::| %name%")) {
        tok = tok->tokAt(tok->strAt(1) == "::" ? 1 : 2);
        while (Token::Match(tok, ":: %name%"))
            tok = tok->tokAt(2);
        if (!Token::Match(tok, "[({<]") || !tok->link())
            return start;
        const bool isTemplate = tok->str() == "<";
        tok = tok->link()->next();
        if (isTemplate && tok && tok->link())
            tok = tok->link()->next();
    }
    return tok;
}

void Tokenizer::setVarIdPass2()
{
    std::map<nonneg int, std::map<std::string, nonneg int>> structMembers;

    // Member functions and variables in this source
    std::list<Member> allMemberFunctions;
    std::list<Member> allMemberVars;
    if (!isC()) {
        std::map<const Token *, std::string> endOfScope;
        std::list<std::string> scope;
        std::list<const Token *> usingnamespaces;
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (!tok->previous() || Token::Match(tok->previous(), "[;{}]")) {
                if (Token::Match(tok, "using namespace %name% ::|;")) {
                    Token *endtok = tok->tokAt(2);
                    while (Token::Match(endtok, "%name% ::"))
                        endtok = endtok->tokAt(2);
                    if (Token::Match(endtok, "%name% ;"))
                        usingnamespaces.push_back(tok->tokAt(2));
                    tok = endtok;
                    continue;
                }
                if (Token::Match(tok, "namespace %name% {")) {
                    scope.push_back(tok->strAt(1));
                    endOfScope[tok->linkAt(2)] = tok->strAt(1);
                }
            }

            if (tok->str() == "}") {
                const auto it = utils::as_const(endOfScope).find(tok);
                if (it != endOfScope.cend())
                    scope.remove(it->second);
            }

            Token* const tok1 = tok;
            if (Token::Match(tok, "%name% :: ~| %name%"))
                tok = tok->next();
            else if (Token::Match(tok, "%name% <") && Token::Match(tok->next()->findClosingBracket(),"> :: ~| %name%"))
                tok = tok->next()->findClosingBracket()->next();
            else if (usingnamespaces.empty() || tok->varId() || !tok->isName() || tok->isStandardType() || tok->tokType() == Token::eKeyword || tok->tokType() == Token::eBoolean ||
                     Token::Match(tok->previous(), ".|namespace|class|struct|&|&&|*|> %name%") || Token::Match(tok->previous(), "%type%| %name% ( %type%|)") || Token::Match(tok, "public:|private:|protected:") ||
                     (!tok->next() && Token::Match(tok->previous(), "}|; %name%")))
                continue;

            if (tok->strAt(-1) == "::" && tok->tokAt(-2) && tok->tokAt(-2)->isName())
                continue;

            while (Token::Match(tok, ":: ~| %name%")) {
                tok = tok->next();
                if (tok->str() == "~")
                    tok = tok->next();
                else if (Token::Match(tok, "%name% <") && Token::Match(tok->next()->findClosingBracket(),"> :: ~| %name%"))
                    tok = tok->next()->findClosingBracket()->next();
                else if (Token::Match(tok, "%name% ::"))
                    tok = tok->next();
                else
                    break;
            }
            if (!tok->next())
                syntaxError(tok);
            if (Token::Match(tok, "%name% (") && !(tok->tokAt(2) && tok->tokAt(2)->isLiteral()))
                allMemberFunctions.emplace_back(scope, usingnamespaces, tok1);
            else
                allMemberVars.emplace_back(scope, usingnamespaces, tok1);
        }
    }

    std::list<ScopeInfo2> scopeInfo;

    // class members..
    std::map<std::string, std::map<std::string, nonneg int>> varsByClass;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (tok->str() == "}" && !scopeInfo.empty() && tok == scopeInfo.back().bodyEnd)
            scopeInfo.pop_back();

        if (!Token::Match(tok, "namespace|class|struct %name% {|:|::|<"))
            continue;

        const std::string &scopeName(getScopeName(scopeInfo));
        const std::string scopeName2(scopeName.empty() ? std::string() : (scopeName + " :: "));

        std::list<const Token*> classnameTokens{ tok->next() };
        Token* tokStart = tok->tokAt(2);
        while (Token::Match(tokStart, ":: %name%") || tokStart->str() == "<") {
            if (tokStart->str() == "<") {
                // skip the template part
                Token* closeTok = tokStart->findClosingBracket();
                if (!closeTok)
                    syntaxError(tok);
                tokStart = closeTok->next();
            } else {
                classnameTokens.push_back(tokStart->next());
                tokStart = tokStart->tokAt(2);
            }
            if (!tokStart)
                syntaxError(tok);
        }

        std::string classname;
        for (const Token *it : classnameTokens)
            classname += (classname.empty() ? "" : " :: ") + it->str();

        std::map<std::string, nonneg int> &thisClassVars = varsByClass[scopeName2 + classname];
        while (Token::Match(tokStart, ":|::|,|%name%")) {
            if (Token::Match(tokStart, "%name% <")) { // TODO: why skip templates?
                tokStart = tokStart->next()->findClosingBracket();
                if (tokStart)
                    tokStart = tokStart->next();
                continue;
            }
            if (Token::Match(tokStart, "%name% ,|{")) {
                std::string baseClassName = tokStart->str();
                const Token* baseStart = tokStart;
                while (Token::Match(baseStart->tokAt(-2), "%name% ::")) { // build base class name
                    baseClassName.insert(0, baseStart->strAt(-2) + " :: ");
                    baseStart = baseStart->tokAt(-2);
                }
                std::string scopeName3(scopeName2);
                while (!scopeName3.empty()) {
                    std::string name = scopeName3 + baseClassName;
                    if (varsByClass.find(name) != varsByClass.end()) {
                        baseClassName = std::move(name);
                        break;
                    }
                    // Remove last scope name
                    if (scopeName3.size() <= 8)
                        break;
                    scopeName3.erase(scopeName3.size() - 4);
                    const std::string::size_type pos = scopeName3.rfind(" :: ");
                    if (pos == std::string::npos)
                        break;
                    scopeName3.erase(pos + 4);
                }
                const std::map<std::string, nonneg int>& baseClassVars = varsByClass[baseClassName];
                thisClassVars.insert(baseClassVars.cbegin(), baseClassVars.cend());
            }
            tokStart = tokStart->next();
        }
        if (!Token::simpleMatch(tokStart, "{"))
            continue;

        // What member variables are there in this class?
        std::transform(classnameTokens.cbegin(), classnameTokens.cend(), std::back_inserter(scopeInfo), [&](const Token* tok) {
            return ScopeInfo2(tok->str(), tokStart->link());
        });

        for (Token *tok2 = tokStart->next(); tok2 && tok2 != tokStart->link(); tok2 = tok2->next()) {
            // skip parentheses..
            if (tok2->link()) {
                if (tok2->str() == "(") {
                    Token *funcstart = const_cast<Token*>(TokenList::isFunctionHead(tok2, "{"));
                    if (funcstart) {
                        setVarIdClassFunction(scopeName2 + classname, funcstart, funcstart->link(), thisClassVars, structMembers, mVarId);
                        tok2 = funcstart->link();
                        continue;
                    }
                }
                if (tok2->str() == "{" && !Token::simpleMatch(tok2->previous(), "union")) {
                    if (tok2->strAt(-1) == ")")
                        setVarIdClassFunction(scopeName2 + classname, tok2, tok2->link(), thisClassVars, structMembers, mVarId);
                    tok2 = tok2->link();
                } else if (Token::Match(tok2, "( %name%|)")) {
                    tok2 = tok2->link();

                    // Skip initialization list
                    if (Token::simpleMatch(tok2, ") :")) {
                        tok2 = skipInitializerList(tok2->next());
                        if (Token::simpleMatch(tok2, "{"))
                            tok2 = tok2->link();
                    }
                }
            }

            // Found a member variable..
            else if (tok2->varId() > 0)
                thisClassVars[tok2->str()] = tok2->varId();
        }

        // Are there any member variables in this class?
        if (thisClassVars.empty())
            continue;

        // Member variables
        for (const Member &var : allMemberVars) {
            Token *tok2 = matchMemberVarName(var, scopeInfo);
            if (!tok2)
                continue;
            if (tok2->varId() == 0)
                tok2->varId(thisClassVars[tok2->str()]);
        }

        if (isC() || tok->str() == "namespace")
            continue;

        // Set variable ids in member functions for this class..
        for (const Member &func : allMemberFunctions) {
            Token *tok2 = matchMemberFunctionName(func, scopeInfo);
            if (!tok2)
                continue;

            if (tok2->str() == "~")
                tok2 = tok2->linkAt(2);
            else
                tok2 = tok2->linkAt(1);

            // If this is a function implementation.. add it to funclist
            Token * start = const_cast<Token *>(TokenList::isFunctionHead(tok2, "{"));
            if (start) {
                setVarIdClassFunction(classname, start, start->link(), thisClassVars, structMembers, mVarId);
            }

            if (Token::Match(tok2, ") %name% ("))
                tok2 = tok2->linkAt(2);

            // constructor with initializer list
            if (!Token::Match(tok2, ") : ::| %name%"))
                continue;

            Token *tok3 = tok2;
            while (Token::Match(tok3, "[)}] [,:]")) {
                tok3 = tok3->tokAt(2);
                if (Token::Match(tok3, ":: %name%"))
                    tok3 = tok3->next();
                while (Token::Match(tok3, "%name% :: %name%"))
                    tok3 = tok3->tokAt(2);
                if (!Token::Match(tok3, "%name% (|{|<"))
                    break;

                // set varid
                const auto varpos = utils::as_const(thisClassVars).find(tok3->str());
                if (varpos != thisClassVars.end())
                    tok3->varId(varpos->second);

                // goto end of var
                if (tok3->strAt(1) == "<") {
                    tok3 = tok3->next()->findClosingBracket();
                    if (tok3 && tok3->next() && tok3->linkAt(1))
                        tok3 = tok3->linkAt(1);
                } else
                    tok3 = tok3->linkAt(1);
            }
            if (Token::Match(tok3, ")|} {")) {
                setVarIdClassFunction(classname, tok2, tok3->linkAt(1), thisClassVars, structMembers, mVarId);
            }
        }
    }
}

static void linkBrackets(const Tokenizer & tokenizer, std::stack<const Token*>& type, std::stack<Token*>& links, Token * const token, const char open, const char close)
{
    if (token->str()[0] == open) {
        links.push(token);
        type.push(token);
    } else if (token->str()[0] == close) {
        if (links.empty()) {
            // Error, { and } don't match.
            tokenizer.unmatchedToken(token);
        }
        if (type.top()->str()[0] != open) {
            tokenizer.unmatchedToken(type.top());
        }
        type.pop();

        Token::createMutualLinks(links.top(), token);
        links.pop();
    }
}

void Tokenizer::createLinks()
{
    std::stack<const Token*> type;
    std::stack<Token*> links1;
    std::stack<Token*> links2;
    std::stack<Token*> links3;
    for (Token *token = list.front(); token; token = token->next()) {
        if (token->link()) {
            token->link(nullptr);
        }

        linkBrackets(*this, type, links1, token, '{', '}');

        linkBrackets(*this, type, links2, token, '(', ')');

        linkBrackets(*this, type, links3, token, '[', ']');
    }

    if (!links1.empty()) {
        // Error, { and } don't match.
        unmatchedToken(links1.top());
    }

    if (!links2.empty()) {
        // Error, ( and ) don't match.
        unmatchedToken(links2.top());
    }

    if (!links3.empty()) {
        // Error, [ and ] don't match.
        unmatchedToken(links3.top());
    }
}

void Tokenizer::createLinks2()
{
    if (isC())
        return;

    bool isStruct = false;

    std::stack<Token*> type;
    std::stack<Token*> templateTokens;
    for (Token *token = list.front(); token; token = token->next()) {
        if (Token::Match(token, "%name%|> %name% [:<]"))
            isStruct = true;
        else if (Token::Match(token, "[;{}]"))
            isStruct = false;

        if (token->link()) {
            if (Token::Match(token, "{|[|("))
                type.push(token);
            else if (!type.empty() && Token::Match(token, "}|]|)")) {
                while (type.top()->str() == "<") {
                    if (!templateTokens.empty() && templateTokens.top()->next() == type.top())
                        templateTokens.pop();
                    type.pop();
                }
                type.pop();
            }
        } else if (templateTokens.empty() && !isStruct && Token::Match(token, "%oror%|&&|;")) {
            if (Token::Match(token, "&& [,>]"))
                continue;
            // If there is some such code:  A<B||C>..
            // Then this is probably a template instantiation if either "B" or "C" has comparisons
            if (token->tokType() == Token::eLogicalOp && !type.empty() && type.top()->str() == "<") {
                const Token *prev = token->previous();
                bool foundComparison = false;
                while (Token::Match(prev, "%name%|%num%|%str%|%cop%|)|]") && prev != type.top()) {
                    if (prev->str() == ")" || prev->str() == "]")
                        prev = prev->link();
                    else if (prev->tokType() == Token::eLogicalOp)
                        break;
                    else if (prev->isComparisonOp())
                        foundComparison = true;
                    prev = prev->previous();
                }
                if (prev == type.top() && foundComparison)
                    continue;
                const Token *next = token->next();
                foundComparison = false;
                while (Token::Match(next, "%name%|%num%|%str%|%cop%|(|[") && next->str() != ">") {
                    if (next->str() == "(" || next->str() == "[")
                        next = next->link();
                    else if (next->tokType() == Token::eLogicalOp)
                        break;
                    else if (next->isComparisonOp())
                        foundComparison = true;
                    next = next->next();
                }
                if (next && next->str() == ">" && foundComparison)
                    continue;
            }

            while (!type.empty() && type.top()->str() == "<") {
                const Token* end = type.top()->findClosingBracket();
                if (Token::Match(end, "> %comp%|;|.|=|{|}|(|)|::"))
                    break;
                // Variable declaration
                if (Token::Match(end, "> %var% ;") && (type.top()->tokAt(-2) == nullptr || Token::Match(type.top()->tokAt(-2), ";|}|{")))
                    break;
                type.pop();
            }
        } else if (token->str() == "<" &&
                   ((token->previous() && (token->previous()->isTemplate() ||
                                           (token->previous()->isName() && !token->previous()->varId()) ||
                                           (token->strAt(-1) == "]" && (!Token::Match(token->linkAt(-1)->previous(), "%name%|)") || token->linkAt(-1)->previous()->isKeyword())) ||
                                           (token->strAt(-1) == ")" && token->linkAt(-1)->strAt(-1) == "operator"))) ||
                    Token::Match(token->next(), ">|>>"))) {
            type.push(token);
            if (token->strAt(-1) == "template")
                templateTokens.push(token);
        } else if (token->str() == ">" || token->str() == ">>") {
            if (type.empty() || type.top()->str() != "<") // < and > don't match.
                continue;
            Token * const top1 = type.top();
            type.pop();
            Token * const top2 = type.empty() ? nullptr : type.top();
            type.push(top1);
            if (!top2 || top2->str() != "<") {
                if (token->str() == ">>")
                    continue;
                if (!Token::Match(token->next(), "%name%|%cop%|%assign%|::|,|(|)|{|}|;|[|]|:|.|=|?|...") &&
                    !Token::Match(token->next(), "&& %name% ="))
                    continue;
            }

            if (token->str() == ">>" && top1 && top2) {
                type.pop();
                type.pop();
                // Split the angle brackets
                token->str(">");
                Token::createMutualLinks(top1, token->insertTokenBefore(">"));
                Token::createMutualLinks(top2, token);
                if (templateTokens.size() == 2 && (top1 == templateTokens.top() || top2 == templateTokens.top())) {
                    templateTokens.pop();
                    templateTokens.pop();
                }
            } else {
                type.pop();
                if (Token::Match(token, "> %name%") && !token->next()->isKeyword() &&
                    Token::Match(top1->tokAt(-2), "%op% %name% <") && top1->strAt(-2) != "<" &&
                    (templateTokens.empty() || top1 != templateTokens.top()))
                    continue;
                Token::createMutualLinks(top1, token);
                if (!templateTokens.empty() && top1 == templateTokens.top())
                    templateTokens.pop();
            }
        }
    }
}

void Tokenizer::markCppCasts()
{
    if (isC())
        return;
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "const_cast|dynamic_cast|reinterpret_cast|static_cast")) {
            if (!Token::simpleMatch(tok->next(), "<") || !Token::simpleMatch(tok->linkAt(1), "> ("))
                syntaxError(tok);
            tok = tok->linkAt(1)->next();
            tok->isCast(true);
        }
    }

}

void Tokenizer::sizeofAddParentheses()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "sizeof !!("))
            continue;
        if (tok->next()->isLiteral() || Token::Match(tok->next(), "%name%|*|~|!|&")) {
            Token *endToken = tok->next();
            while (Token::simpleMatch(endToken, "* *"))
                endToken = endToken->next();
            while (Token::Match(endToken->next(), "%name%|%num%|%str%|[|(|.|::|++|--|!|~") || (Token::Match(endToken, "%type% * %op%|?|:|const|;|,"))) {
                if (Token::Match(endToken->next(), "(|["))
                    endToken = endToken->linkAt(1);
                else
                    endToken = endToken->next();
            }

            // Add ( after sizeof and ) behind endToken
            tok->insertToken("(");
            endToken->insertToken(")");
            Token::createMutualLinks(tok->next(), endToken->next());
        }
    }
}

bool Tokenizer::simplifyTokenList1(const char FileName[])
{
    if (Settings::terminated())
        return false;

    // if MACRO
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if|for|while %name% (")) {
            if (Token::simpleMatch(tok, "for each")) {
                // 'for each (x in y )' -> 'for (x : y)'
                if (Token* in = Token::findsimplematch(tok->tokAt(2), "in", tok->linkAt(2)))
                    in->str(":");
                tok->deleteNext();
            } else if (tok->strAt(1) == "constexpr") {
                tok->deleteNext();
                tok->isConstexpr(true);
            } else {
                syntaxError(tok);
            }
        }
    }

    // Is there C++ code in C file?
    validateC();

    // Combine strings and character literals, e.g. L"string", L'c', "string1" "string2"
    combineStringAndCharLiterals();

    // replace inline SQL with "asm()" (Oracle PRO*C). Ticket: #1959
    simplifySQL();

    createLinks();

    // replace library function calls such as (std::min)(a, b) with std::min(a, b)
    simplifyParenthesizedLibraryFunctions();

    // Simplify debug intrinsics
    simplifyDebug();

    removePragma();

    // Simplify the C alternative tokens (and, or, etc.)
    simplifyCAlternativeTokens();

    simplifyFunctionTryCatch();

    simplifyHeadersAndUnusedTemplates();

    // Remove __asm..
    simplifyAsm();

    // foo < bar < >> => foo < bar < > >
    if (isCPP() || mSettings.daca)
        splitTemplateRightAngleBrackets(!isCPP());

    // Remove extra "template" tokens that are not used by cppcheck
    removeExtraTemplateKeywords();

    simplifySpaceshipOperator();

    // @..
    simplifyAt();

    // Remove __declspec()
    simplifyDeclspec();

    // Remove "inline", "register", and "restrict"
    simplifyKeyword();

    // Remove [[attribute]]
    simplifyCPPAttribute();

    // remove __attribute__((?))
    simplifyAttribute();

    validate();

    const ShowTime showTime = mTimerResults ? mSettings.showtime : ShowTime::NONE;

    // Bail out if code is garbage
    Timer::run("Tokenizer::simplifyTokens1::simplifyTokenList1::findGarbageCode", showTime, mTimerResults, [&]() {
        findGarbageCode();
    });

    if (Settings::terminated())
        return false;

    // convert C++17 style nested namespaces to old style namespaces
    simplifyNestedNamespace();

    // convert c++20 coroutines
    simplifyCoroutines();

    // simplify namespace aliases
    simplifyNamespaceAliases();

    // simplify cppcheck attributes __cppcheck_?__(?)
    simplifyCppcheckAttribute();

    // Combine tokens..
    combineOperators();

    // combine "- %num%"
    concatenateNegativeNumberAndAnyPositive();

    // remove extern "C" and extern "C" {}
    if (isCPP())
        simplifyExternC();

    validate();

    // simplify compound statements: "[;{}] ( { code; } ) ;"->"[;{}] code;"
    simplifyCompoundStatements();

    if (!simplifyAddBraces())
        return false;

    sizeofAddParentheses();

    // Simplify: 0[foo] -> *(foo)
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "0 [") && tok->linkAt(1)) {
            tok->str("*");
            tok->next()->str("(");
            tok->linkAt(1)->str(")");
        }
    }

    if (Settings::terminated())
        return false;

    validate();

    // simplify simple calculations inside <..>
    if (isCPP()) {
        Token *lt = nullptr;
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "[;{}]"))
                lt = nullptr;
            else if (Token::Match(tok, "%type% <"))
                lt = tok->next();
            else if (lt && Token::Match(tok, ">|>> %name%|::|(")) {
                const Token * const end = tok;
                for (tok = lt; tok != end; tok = tok->next()) {
                    if (tok->isNumber())
                        TemplateSimplifier::simplifyNumericCalculations(tok);
                }
                lt = tok->next();
            }
        }
    }

    // Convert K&R function declarations to modern C
    simplifyVarDecl(true);
    simplifyFunctionParameters();

    // simplify case ranges (gcc extension)
    simplifyCaseRange();

    // simplify labels and 'case|default'-like syntaxes
    simplifyLabelsCaseDefault();

    if (!isC() && !mSettings.library.markupFile(FileName)) {
        findComplicatedSyntaxErrorsInTemplates();
    }

    if (Settings::terminated())
        return false;

    // remove calling conventions __cdecl, __stdcall..
    simplifyCallingConvention();

    addSemicolonAfterUnknownMacro();

    // remove some unhandled macros in global scope
    removeMacrosInGlobalScope();

    // remove undefined macro in class definition:
    // class DLLEXPORT Fred { };
    // class Fred FINAL : Base { };
    removeMacroInClassDef();

    // That call here fixes #7190
    validate();

    // remove unnecessary member qualification..
    removeUnnecessaryQualification();

    // convert Microsoft memory functions
    simplifyMicrosoftMemoryFunctions();

    // convert Microsoft string functions
    simplifyMicrosoftStringFunctions();

    if (Settings::terminated())
        return false;

    // remove Borland stuff..
    simplifyBorland();

    // syntax error: enum with typedef in it
    checkForEnumsWithTypedef();

    // Add parentheses to ternary operator where necessary
    prepareTernaryOpForAST();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl(false);

    reportUnknownMacros();

    simplifyTypedefLHS();

    // typedef..
    Timer::run("Tokenizer::simplifyTokens1::simplifyTokenList1::simplifyTypedef", showTime, mTimerResults, [&]() {
        simplifyTypedef();
    });

    // using A = B;
    while (simplifyUsing())
        ;

    // Add parentheses to ternary operator where necessary
    // TODO: this is only necessary if one typedef simplification had a comma and was used within ?:
    // If typedef handling is refactored and moved to symboldatabase someday we can remove this
    prepareTernaryOpForAST();

    // class x y {
    if (isCPP() && mSettings.severity.isEnabled(Severity::information)) {
        for (const Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "class %type% %type% [:{]")) {
                unhandled_macro_class_x_y(tok, tok->str(), tok->strAt(1), tok->strAt(2), tok->strAt(3));
            }
        }
    }

    // catch bad typedef canonicalization
    //
    // to reproduce bad typedef, download upx-ucl from:
    // http://packages.debian.org/sid/upx-ucl
    // analyse the file src/stub/src/i386-linux.elf.interp-main.c
    validate();

    // The simplify enum have inner loops
    if (Settings::terminated())
        return false;

    // Put ^{} statements in asm()
    simplifyAsm2();

    // When the assembly code has been cleaned up, no @ is allowed
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            const Token *tok1 = tok;
            tok = tok->link();
            if (!tok)
                syntaxError(tok1);
        } else if (tok->str() == "@") {
            syntaxError(tok);
        }
    }

    // Order keywords "static" and "const"
    simplifyStaticConst();

    // convert platform dependent types to standard types
    // 32 bits: size_t -> unsigned long
    // 64 bits: size_t -> unsigned long long
    list.simplifyPlatformTypes();

    // collapse compound standard types into a single token
    // unsigned long long int => long (with _isUnsigned=true,_isLong=true)
    list.simplifyStdType();

    if (Settings::terminated())
        return false;

    // simplify bit fields..
    simplifyBitfields();

    if (Settings::terminated())
        return false;

    // struct simplification "struct S {} s; => struct S { } ; S s ;
    simplifyStructDecl();

    if (Settings::terminated())
        return false;

    // x = ({ 123; });  =>   { x = 123; }
    simplifyAssignmentBlock();

    if (Settings::terminated())
        return false;

    simplifyVariableMultipleAssign();

    // Collapse operator name tokens into single token
    // operator = => operator=
    simplifyOperatorName();

    // Remove redundant parentheses
    simplifyRedundantParentheses();

    if (isCPP()) {
        simplifyTypeIntrinsics();

        // Handle templates..
        Timer::run("Tokenizer::simplifyTokens1::simplifyTokenList1::simplifyTemplates", showTime, mTimerResults, [&]() {
            simplifyTemplates();
        });

        // The simplifyTemplates have inner loops
        if (Settings::terminated())
            return false;

        validate(); // #6847 - invalid code
    }

    // Simplify pointer to standard types (C only)
    simplifyPointerToStandardType();

    // simplify function pointers
    simplifyFunctionPointers();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl(false);

    elseif();

    validate(); // #6772 "segmentation fault (invalid code) in Tokenizer::setVarId"

    Timer::run("Tokenizer::simplifyTokens1::simplifyTokenList1::setVarId", showTime, mTimerResults, [&](){
        setVarId();
    });

    // Link < with >
    createLinks2();

    // Mark C++ casts
    markCppCasts();

    // specify array size
    arraySize();

    // The simplify enum might have inner loops
    if (Settings::terminated())
        return false;

    // Add std:: in front of std classes, when using namespace std; was given
    simplifyNamespaceStd();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    simplifyDoublePlusAndDoubleMinus();

    simplifyArrayAccessSyntax();

    Token::assignProgressValues(list.front());

    removeRedundantSemicolons();

    simplifyParameterVoid();

    simplifyRedundantConsecutiveBraces();

    simplifyEmptyNamespaces();

    simplifyIfSwitchForInit();

    simplifyOverloadedOperators();

    validate();

    list.front()->assignIndexes();

    return true;
}
//---------------------------------------------------------------------------

void Tokenizer::printDebugOutput(std::ostream &out) const
{
    if (!list.front())
        return;

    const bool debug = mSettings.debugSimplified || mSettings.debugnormal || mSettings.debugsymdb || mSettings.debugast || mSettings.debugvalueflow;
    if (!debug)
        return;

    const bool xml = (mSettings.outputFormat == Settings::OutputFormat::xml);

    if (xml)
        out << "<debug>" << std::endl;

    if (mSettings.debugSimplified || mSettings.debugnormal)
        list.front()->printOut(out, xml, nullptr, list.getFiles());

    if (mSymbolDatabase) {
        if (xml)
            mSymbolDatabase->printXml(out);
        else if (mSettings.debugsymdb)
            mSymbolDatabase->printOut("Symbol database");
    }

    if (mSettings.debugast)
        list.front()->printAst(xml, list.getFiles(), out);

    if (mSettings.debugvalueflow)
        list.front()->printValueFlow(list.getFiles(), xml, out);

    if (xml)
        out << "</debug>" << std::endl;
}

void Tokenizer::dump(std::ostream &out) const
{
    // Create a xml data dump.
    // The idea is not that this will be readable for humans. It's a
    // data dump that 3rd party tools could load and get useful info from.

    std::string outs;

    std::set<const Library::Container*> containers;

    outs += "  <directivelist>";
    outs += '\n';
    for (const Directive &dir : mDirectives) {
        outs += "    <directive ";
        outs += "file=\"";
        outs += ErrorLogger::toxml(Path::getRelativePath(dir.file, mSettings.basePaths));
        outs += "\" ";
        outs += "linenr=\"";
        outs += std::to_string(dir.linenr);
        outs += "\" ";
        // str might contain characters such as '"', '<' or '>' which
        // could result in invalid XML, so run it through toxml().
        outs += "str=\"";
        outs += ErrorLogger::toxml(dir.str);
        outs +="\">";
        outs += '\n';
        for (const auto & strToken : dir.strTokens) {
            outs += "      <token ";
            outs += "column=\"";
            outs += std::to_string(strToken.column);
            outs += "\" ";
            outs += "str=\"";
            outs += ErrorLogger::toxml(strToken.tokStr);
            outs +="\"/>";
            outs += '\n';
        }
        outs += "    </directive>";
        outs += '\n';
    }
    outs += "  </directivelist>";
    outs += '\n';

    // tokens..
    outs += "  <tokenlist>";
    outs += '\n';
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        outs += "    <token id=\"";
        outs += id_string(tok);
        outs += "\" file=\"";
        outs += ErrorLogger::toxml(list.file(tok));
        outs += "\" linenr=\"";
        outs += std::to_string(tok->linenr());
        outs += "\" column=\"";
        outs += std::to_string(tok->column());
        outs += "\"";

        outs += " str=\"";
        outs += ErrorLogger::toxml(tok->str());
        outs += '\"';

        outs += " scope=\"";
        outs += id_string(tok->scope());
        outs += '\"';
        if (tok->isName()) {
            outs += " type=\"name\"";
            if (tok->isUnsigned())
                outs += " isUnsigned=\"true\"";
            else if (tok->isSigned())
                outs += " isSigned=\"true\"";
        } else if (tok->isNumber()) {
            outs += " type=\"number\"";
            if (MathLib::isInt(tok->str()))
                outs += " isInt=\"true\"";
            if (MathLib::isFloat(tok->str()))
                outs += " isFloat=\"true\"";
        } else if (tok->tokType() == Token::eString) {
            outs += " type=\"string\" strlen=\"";
            outs += std::to_string(Token::getStrLength(tok));
            outs += '\"';
        }
        else if (tok->tokType() == Token::eChar)
            outs += " type=\"char\"";
        else if (tok->isBoolean())
            outs += " type=\"boolean\"";
        else if (tok->isOp()) {
            outs += " type=\"op\"";
            if (tok->isArithmeticalOp())
                outs += " isArithmeticalOp=\"true\"";
            else if (tok->isAssignmentOp())
                outs += " isAssignmentOp=\"true\"";
            else if (tok->isComparisonOp())
                outs += " isComparisonOp=\"true\"";
            else if (tok->tokType() == Token::eLogicalOp)
                outs += " isLogicalOp=\"true\"";
        }
        if (tok->isCast())
            outs += " isCast=\"true\"";
        if (tok->isExternC())
            outs += " externLang=\"C\"";
        if (tok->isExpandedMacro())
            outs += " macroName=\"" + tok->getMacroName() + "\"";
        if (tok->isTemplateArg()) {
            outs += " isTemplateArg=\"true\"";
            outs += " templateArgFileIndex=\"" + std::to_string(tok->templateArgFileIndex()) + "\"";
            outs += " templateArgLineNumber=\"" + std::to_string(tok->templateArgLineNumber()) + "\"";
            outs += " templateArgColumn=\"" + std::to_string(tok->templateArgColumn()) + "\"";
        }
        if (tok->isRemovedVoidParameter())
            outs += " isRemovedVoidParameter=\"true\"";
        if (tok->isSplittedVarDeclComma())
            outs += " isSplittedVarDeclComma=\"true\"";
        if (tok->isSplittedVarDeclEq())
            outs += " isSplittedVarDeclEq=\"true\"";
        if (tok->isImplicitInt())
            outs += " isImplicitInt=\"true\"";
        if (tok->isComplex())
            outs += " isComplex=\"true\"";
        if (tok->isRestrict())
            outs += " isRestrict=\"true\"";
        if (tok->isAtomic())
            outs += " isAtomic=\"true\"";
        if (tok->isAttributeExport())
            outs += " isAttributeExport=\"true\"";
        if (tok->isAttributeMaybeUnused())
            outs += " isAttributeMaybeUnused=\"true\"";
        if (tok->isAttributeUnused())
            outs += " isAttributeUnused=\"true\"";
        if (tok->isAttributeFallthrough())
            outs += " isAttributeFallthrough=\"true\"";
        if (tok->isInitBracket())
            outs += " isInitBracket=\"true\"";
        if (tok->isAnonymous())
            outs += " isAnonymous=\"true\"";
        if (tok->hasAttributeAlignas()) {
            const std::vector<std::string>& a = tok->getAttributeAlignas();
            outs += " alignas=\"" + ErrorLogger::toxml(a[0]) + "\"";
            if (a.size() > 1)
                // we could write all alignas expressions but currently we only need 2
                outs += " alignas2=\"" + ErrorLogger::toxml(a[1]) + "\"";
        }
        if (tok->link()) {
            outs += " link=\"";
            outs += id_string(tok->link());
            outs += '\"';
        }
        if (tok->varId() > 0) {
            outs += " varId=\"";
            outs += std::to_string(tok->varId());
            outs += '\"';
        }
        if (tok->exprId() > 0) {
            outs += " exprId=\"";
            outs += std::to_string(tok->exprId());
            outs += '\"';
        }
        if (tok->variable()) {
            outs += " variable=\"";
            outs += id_string(tok->variable());
            outs += '\"';
        }
        if (tok->function()) {
            outs += " function=\"";
            outs += id_string(tok->function());
            outs += '\"';
        }
        if (!tok->values().empty()) {
            outs += " values=\"";
            outs += id_string(&tok->values());
            outs += '\"';
        }
        if (tok->type()) {
            outs += " type-scope=\"";
            outs += id_string(tok->type()->classScope);
            outs += '\"';
        }
        if (tok->astParent()) {
            outs += " astParent=\"";
            outs += id_string(tok->astParent());
            outs += '\"';
        }
        if (tok->astOperand1()) {
            outs += " astOperand1=\"";
            outs += id_string(tok->astOperand1());
            outs += '\"';
        }
        if (tok->astOperand2()) {
            outs += " astOperand2=\"";
            outs += id_string(tok->astOperand2());
            outs += '\"';
        }
        if (!tok->originalName().empty()) {
            outs += " originalName=\"";
            outs += tok->originalName();
            outs += '\"';
        }
        if (tok->valueType()) {
            const std::string vt = tok->valueType()->dump();
            if (!vt.empty()) {
                outs += ' ';
                outs += vt;
            }
            containers.insert(tok->valueType()->container);
        }
        if (!tok->varId() && tok->scope()->isExecutable() && Token::Match(tok, "%name% (")) {
            if (mSettings.library.isnoreturn(tok))
                outs += " noreturn=\"true\"";
        }

        outs += "/>";
        outs += '\n';
    }
    outs += "  </tokenlist>";
    outs += '\n';

    out << outs;
    outs.clear();

    if (mSymbolDatabase)
        mSymbolDatabase->printXml(out);

    containers.erase(nullptr);
    if (!containers.empty()) {
        outs += "  <containers>";
        outs += '\n';
        for (const Library::Container* c: containers) {
            outs += "    <container id=\"";
            outs += id_string(c);
            outs += "\" array-like-index-op=\"";
            outs += bool_to_string(c->arrayLike_indexOp);
            outs += "\" ";
            outs += "std-string-like=\"";
            outs += bool_to_string(c->stdStringLike);
            outs += "\"";
            if (c->functions.empty()) {
                outs += "/>\n";
                continue;
            }
            outs += ">\n";
            for (const auto& fp: c->functions) {
                std::string action;
                std::string yield;
                if (fp.second.action != Library::Container::Action::NO_ACTION)
                    action = " action=\"" + Library::Container::toString(fp.second.action) + "\"";
                if (fp.second.yield != Library::Container::Yield::NO_YIELD)
                    yield = " yield=\"" + Library::Container::toString(fp.second.yield) + "\"";
                if (!action.empty() || !yield.empty())
                    outs += "      <f name=\"" + fp.first + "\"" + action + yield + "/>\n";
            }
            outs += "    </container>\n";
        }
        outs += "  </containers>";
        outs += '\n';
    }

    if (list.front())
        list.front()->printValueFlow(list.getFiles(), true, out);

    outs += dumpTypedefInfo();

    outs += mTemplateSimplifier->dump();
    if (!mTemplateVarIdUsage.empty()) {
        outs += "  <template-varid-usage>\n";
        for (nonneg int id: mTemplateVarIdUsage)
            outs += "    <var id=\"" + std::to_string(id) + "\"/>\n";
        outs += "  </template-varid-usage>\n";
    }

    out << outs;
}

std::string Tokenizer::dumpTypedefInfo() const
{
    if (mTypedefInfo.empty())
        return "";
    std::string outs = "  <typedef-info>";
    outs += '\n';
    for (const TypedefInfo &typedefInfo: mTypedefInfo) {
        const bool toks = !typedefInfo.typedefInfoTokens.empty();
        outs += "    <info";

        outs += " name=\"";
        outs += typedefInfo.name;
        outs += "\"";

        outs += " file=\"";
        outs += ErrorLogger::toxml(typedefInfo.filename);
        outs += "\"";

        outs += " line=\"";
        outs += std::to_string(typedefInfo.lineNumber);
        outs += "\"";

        outs += " column=\"";
        outs += std::to_string(typedefInfo.column);
        outs += "\"";
        if (typedefInfo.tagLine > -1 && typedefInfo.tagColumn > -1) {
            outs += " tagline=\"";
            outs += std::to_string(typedefInfo.tagLine);
            outs += "\"";

            outs += " tagcolumn=\"";
            outs += std::to_string(typedefInfo.tagColumn);
            outs += "\"";
        }
        outs += " used=\"";
        outs += std::to_string(typedefInfo.used?1:0);
        outs += "\"";

        outs += " isFunctionPointer=\"";
        outs += std::to_string(typedefInfo.isFunctionPointer);
        outs += "\"";
        if (toks)
            outs += ">";
        else
            outs += "/>";
        outs += '\n';
        for (const auto& t : typedefInfo.typedefInfoTokens) {
            outs += "      <token ";
            outs += "line=\"";
            outs += std::to_string(t.lineNumber);
            outs += "\" ";
            outs += "column=\"";
            outs += std::to_string(t.column);
            outs += "\" ";
            outs += "str=\"";
            outs += ErrorLogger::toxml(t.name);
            outs += "\"/>";
            outs += '\n';
        }
        if (toks)
            outs += "    </info>\n";
    }
    outs += "  </typedef-info>";
    outs += '\n';

    return outs;
}

void Tokenizer::simplifyHeadersAndUnusedTemplates()
{
    if (mSettings.checkHeaders && mSettings.checkUnusedTemplates)
        // Full analysis. All information in the headers are kept.
        return;

    const bool checkHeaders = mSettings.checkHeaders;
    const bool removeUnusedIncludedFunctions = !mSettings.checkHeaders;
    const bool removeUnusedIncludedClasses   = !mSettings.checkHeaders;
    const bool removeUnusedIncludedTemplates = !mSettings.checkUnusedTemplates || !mSettings.checkHeaders;
    const bool removeUnusedTemplates = !mSettings.checkUnusedTemplates;

    // checkHeaders:
    //
    // If it is true then keep all code in the headers. It's possible
    // to remove unused types/variables if false positives / false
    // negatives can be avoided.
    //
    // If it is false, then we want to remove selected stuff from the
    // headers but not *everything*. The intention here is to not damage
    // the analysis of the source file. You should get all warnings in
    // the source file. You should not get false positives.

    // functions and types to keep
    std::set<std::string> keep;
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isCpp() && Token::simpleMatch(tok, "template <")) {
            const Token *closingBracket = tok->next()->findClosingBracket();
            if (Token::Match(closingBracket, "> class|struct %name% {"))
                tok = closingBracket->linkAt(3);
        }

        if (!tok->isName() || tok->isKeyword())
            continue;

        if (!checkHeaders && tok->fileIndex() != 0)
            continue;

        if (Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->linkAt(1), ") {")) {
            keep.insert(tok->str());
            continue;
        }

        if (Token::Match(tok, "%name% %name%|::|*|&|<")) {
            keep.insert(tok->str());
        }
    }

    const std::set<std::string> functionStart{"static", "const", "unsigned", "signed", "void", "bool", "char", "short", "int", "long", "float", "*"};
    bool goBack = false;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        const bool isIncluded = (tok->fileIndex() != 0);

        if (goBack) {
            tok = tok->previous();
        }
        goBack = false;

        // Remove executable code
        if (isIncluded && !mSettings.checkHeaders && tok->str() == "{") {
            // TODO: We probably need to keep the executable code if this function is called from the source file.
            const Token *prev = tok->previous();
            while (prev && prev->isName())
                prev = prev->previous();
            if (Token::simpleMatch(prev, ")")) {
                // Replace all tokens from { to } with a ";".
                Token::eraseTokens(tok,tok->link()->next());
                tok->str(";");
                tok->link(nullptr);
            }
        }

        if (!tok->previous() || Token::Match(tok->previous(), "[;{}]")) {
            // Remove unused function declarations
            if (isIncluded && removeUnusedIncludedFunctions) {
                while (true) {
                    Token *start = tok;
                    while (start && functionStart.find(start->str()) != functionStart.end())
                        start = start->next();
                    if (Token::Match(start, "%name% (") && Token::Match(start->linkAt(1), ") const| ;") && keep.find(start->str()) == keep.end()) {
                        Token::eraseTokens(tok, start->linkAt(1)->tokAt(2));
                        tok->deleteThis();
                    } else
                        break;
                }
            }

            if (isIncluded && removeUnusedIncludedClasses) {
                if (Token::Match(tok, "class|struct %name% [:{]") && keep.find(tok->strAt(1)) == keep.end()) {
                    // Remove this class/struct
                    const Token *endToken = tok->tokAt(2);
                    if (endToken->str() == ":") {
                        endToken = endToken->next();
                        while (Token::Match(endToken, "%name%|,"))
                            endToken = endToken->next();
                    }
                    if (endToken && endToken->str() == "{" && Token::simpleMatch(endToken->link(), "} ;")) {
                        Token::eraseTokens(tok, endToken->link()->next());
                        tok->deleteThis();
                    }
                }
            }

            if (removeUnusedTemplates || (isIncluded && removeUnusedIncludedTemplates)) {
                if (Token::Match(tok, "template < %name%")) {
                    const Token *closingBracket = tok->next()->findClosingBracket();
                    if (Token::Match(closingBracket, "> class|struct %name% [;:{]") && keep.find(closingBracket->strAt(2)) == keep.end()) {
                        const Token *endToken = closingBracket->tokAt(3);
                        if (endToken->str() == ":") {
                            endToken = endToken->next();
                            while (Token::Match(endToken, "%name%|,"))
                                endToken = endToken->next();
                        }
                        if (endToken && endToken->str() == "{")
                            endToken = endToken->link()->next();
                        if (endToken && endToken->str() == ";") {
                            Token::eraseTokens(tok, endToken);
                            tok->deleteThis();
                        }
                    } else if (Token::Match(closingBracket, "> %type% %name% (") && Token::simpleMatch(closingBracket->linkAt(3), ") {") && keep.find(closingBracket->strAt(2)) == keep.end()) {
                        const Token *endToken = closingBracket->linkAt(3)->linkAt(1)->next();
                        Token::eraseTokens(tok, endToken);
                        tok->deleteThis();
                        goBack = true;
                    }
                }
            }
        }
    }
}

void Tokenizer::removeExtraTemplateKeywords()
{
    if (isCPP()) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "%name%|>|) .|:: template %name%")) {
                tok->next()->deleteNext();
                Token* templateName = tok->tokAt(2);
                while (Token::Match(templateName, "%name%|::")) {
                    templateName->isTemplate(true);
                    templateName = templateName->next();
                }
                if (!templateName)
                    syntaxError(tok);
                if (Token::Match(templateName->previous(), "operator %op%|(")) {
                    templateName->isTemplate(true);
                    if (templateName->str() == "(" && templateName->link())
                        templateName->link()->isTemplate(true);
                }
            }
        }
    }
}

static std::string getExpression(const Token *tok)
{
    std::string line;
    for (const Token *prev = tok->previous(); prev && !Token::Match(prev, "[;{}]"); prev = prev->previous())
        line = prev->str() + " " + line;
    line += "!!!" + tok->str() + "!!!";
    for (const Token *next = tok->next(); next && !Token::Match(next, "[;{}]"); next = next->next())
        line += " " + next->str();
    return line;
}

void Tokenizer::splitTemplateRightAngleBrackets(bool check)
{
    std::vector<std::pair<std::string, int>> vars;

    int scopeLevel = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            ++scopeLevel;
        else if (tok->str() == "}") {
            vars.erase(std::remove_if(vars.begin(), vars.end(), [scopeLevel](const std::pair<std::string, int>& v) {
                return v.second == scopeLevel;
            }), vars.end());
            --scopeLevel;
        }
        if (Token::Match(tok, "[;{}] %type% %type% [;,=]") && tok->next()->isStandardType())
            vars.emplace_back(tok->strAt(2), scopeLevel);

        // Ticket #6181: normalize C++11 template parameter list closing syntax
        if (tok->previous() && tok->str() == "<" && TemplateSimplifier::templateParameters(tok) && std::none_of(vars.begin(), vars.end(), [&](const std::pair<std::string, int>& v) {
            return v.first == tok->strAt(-1);
        })) {
            Token *endTok = tok->findClosingBracket();
            if (check) {
                if (Token::Match(endTok, ">>|>>="))
                    reportError(tok, Severity::debug, "dacaWrongSplitTemplateRightAngleBrackets", "bad closing bracket for !!!<!!!: " + getExpression(tok), false);
                continue;
            }
            if (endTok && endTok->str() == ">>") {
                endTok->str(">");
                endTok->insertToken(">");
            } else if (endTok && endTok->str() == ">>=") {
                endTok->str(">");
                endTok->insertToken("=");
                endTok->insertToken(">");
            }
        } else if (Token::Match(tok, "class|struct|union|=|:|public|protected|private %name% <") && std::none_of(vars.begin(), vars.end(), [&](const std::pair<std::string, int>& v) {
            return v.first == tok->strAt(1);
        })) {
            Token *endTok = tok->tokAt(2)->findClosingBracket();
            if (check) {
                if (Token::simpleMatch(endTok, ">>"))
                    reportError(tok, Severity::debug, "dacaWrongSplitTemplateRightAngleBrackets", "bad closing bracket for !!!<!!!: " + getExpression(tok), false);
                continue;
            }
            if (Token::Match(endTok, ">> ;|{|%type%")) {
                endTok->str(">");
                endTok->insertToken(">");
            }
        }
    }
}

void Tokenizer::removeMacrosInGlobalScope()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::Match(tok, ") %type% {") &&
                !tok->next()->isStandardType() &&
                !tok->next()->isKeyword() &&
                !Token::Match(tok->next(), "override|final") &&
                tok->next()->isUpperCaseName())
                tok->deleteNext();
        }

        if (Token::Match(tok, "%type%") && tok->isUpperCaseName() &&
            (!tok->previous() || Token::Match(tok->previous(), "[;{}]") || (tok->previous()->isName() && endsWith(tok->strAt(-1), ':')))) {
            const Token *tok2 = tok->next();
            if (tok2 && tok2->str() == "(")
                tok2 = tok2->link()->next();

            // Several unknown macros...
            while (Token::Match(tok2, "%type% (") && tok2->isUpperCaseName())
                tok2 = tok2->linkAt(1)->next();

            if (Token::Match(tok, "%name% (") && Token::Match(tok2, "%name% *|&|::|<| %name%") &&
                !Token::Match(tok2, "requires|namespace|class|struct|union|private:|protected:|public:"))
                unknownMacroError(tok);

            if (Token::Match(tok, "%type% (") && Token::Match(tok2, "%type% (") && !Token::Match(tok2, "noexcept|throw") && TokenList::isFunctionHead(tok2->next(), ":;{"))
                unknownMacroError(tok);

            // remove unknown macros before namespace|class|struct|union
            if (Token::Match(tok2, "namespace|class|struct|union")) {
                // is there a "{" for?
                const Token *tok3 = tok2;
                while (tok3 && !Token::Match(tok3,"[;{}()]"))
                    tok3 = tok3->next();
                if (tok3 && tok3->str() == "{") {
                    Token::eraseTokens(tok, tok2);
                    tok->deleteThis();
                }
                continue;
            }

            // replace unknown macros before foo(
            /*
                        if (Token::Match(tok2, "%type% (") && TokenList::isFunctionHead(tok2->next(), "{")) {
                            std::string typeName;
                            for (const Token* tok3 = tok; tok3 != tok2; tok3 = tok3->next())
                                typeName += tok3->str();
                            Token::eraseTokens(tok, tok2);
                            tok->str(typeName);
                        }
             */
            // remove unknown macros before foo::foo(
            if (Token::Match(tok2, "%type% :: %type%")) {
                const Token *tok3 = tok2;
                while (Token::Match(tok3, "%type% :: %type% ::"))
                    tok3 = tok3->tokAt(2);
                if (Token::Match(tok3, "%type% :: %type% (") && tok3->str() == tok3->strAt(2)) {
                    Token::eraseTokens(tok, tok2);
                    tok->deleteThis();
                }
                continue;
            }
        }

        // Skip executable scopes
        if (tok->str() == "{") {
            const Token *prev = tok->previous();
            while (prev && prev->isName())
                prev = prev->previous();
            if (prev && prev->str() == ")")
                tok = tok->link();
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::removePragma()
{
    if (isC() && mSettings.standards.c == Standards::C89)
        return;
    if (isCPP() && mSettings.standards.cpp == Standards::CPP03)
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::simpleMatch(tok, "_Pragma (")) {
            Token::eraseTokens(tok, tok->linkAt(1)->next());
            tok->deleteThis();
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::removeMacroInClassDef()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "class|struct %name% %name% final| {|:"))
            continue;

        const bool nextIsUppercase = tok->next()->isUpperCaseName();
        const bool afterNextIsUppercase = tok->tokAt(2)->isUpperCaseName();
        if (nextIsUppercase && !afterNextIsUppercase)
            tok->deleteNext();
        else if (!nextIsUppercase && afterNextIsUppercase)
            tok->next()->deleteNext();
    }
}

//---------------------------------------------------------------------------

void Tokenizer::addSemicolonAfterUnknownMacro()
{
    if (!isCPP())
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != ")")
            continue;
        const Token *macro = tok->link() ? tok->link()->previous() : nullptr;
        if (!macro || !macro->isName())
            continue;
        if (Token::simpleMatch(tok, ") try") && !Token::Match(macro, "if|for|while"))
            tok->insertToken(";");
        else if (Token::simpleMatch(tok, ") using"))
            tok->insertToken(";");
    }
}
//---------------------------------------------------------------------------

void Tokenizer::simplifyEmptyNamespaces()
{
    if (isC())
        return;

    bool goback = false;
    for (Token *tok = list.front(); tok; tok = tok ? tok->next() : nullptr) {
        if (goback) {
            tok = tok->previous();
            goback = false;
        }
        if (Token::Match(tok, "(|[|{")) {
            tok = tok->link();
            continue;
        }
        if (!Token::Match(tok, "namespace %name%| {"))
            continue;
        const bool isAnonymousNS = tok->strAt(1) == "{";
        if (tok->strAt(3 - isAnonymousNS) == "}") {
            tok->deleteNext(3 - isAnonymousNS); // remove '%name%| { }'
            if (!tok->previous()) {
                // remove 'namespace' or replace it with ';' if isolated
                tok->deleteThis();
                goback = true;
            } else {                    // '%any% namespace %any%'
                tok = tok->previous();  // goto previous token
                tok->deleteNext();      // remove next token: 'namespace'
                if (tok->str() == "{") {
                    // Go back in case we were within a namespace that's empty now
                    tok = tok->tokAt(-2) ? tok->tokAt(-2) : tok->previous();
                    goback = true;
                }
            }
        } else {
            tok = tok->tokAt(2 - isAnonymousNS);
        }
    }
}

void Tokenizer::removeRedundantSemicolons()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->link() && tok->str() == "(") {
            tok = tok->link();
            continue;
        }
        for (;;) {
            if (Token::simpleMatch(tok, "; ;")) {
                tok->deleteNext();
            } else if (Token::simpleMatch(tok, "; { ; }")) {
                tok->deleteNext(3);
            } else {
                break;
            }
        }
    }
}


bool Tokenizer::simplifyAddBraces()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        Token const * tokRet=simplifyAddBracesToCommand(tok);
        if (!tokRet)
            return false;
    }
    return true;
}

Token *Tokenizer::simplifyAddBracesToCommand(Token *tok)
{
    Token * tokEnd=tok;
    if (Token::Match(tok,"for|switch|BOOST_FOREACH")) {
        tokEnd=simplifyAddBracesPair(tok,true);
    } else if (tok->str()=="while") {
        Token *tokPossibleDo=tok->previous();
        if (Token::simpleMatch(tok->previous(), "{"))
            tokPossibleDo = nullptr;
        else if (Token::simpleMatch(tokPossibleDo,"}"))
            tokPossibleDo = tokPossibleDo->link();
        if (!tokPossibleDo || tokPossibleDo->strAt(-1) != "do")
            tokEnd=simplifyAddBracesPair(tok,true);
    } else if (tok->str()=="do") {
        tokEnd=simplifyAddBracesPair(tok,false);
        if (tokEnd!=tok) {
            // walk on to next token, i.e. "while"
            // such that simplifyAddBracesPair does not close other braces
            // before the "while"
            if (tokEnd) {
                tokEnd=tokEnd->next();
                if (!tokEnd || tokEnd->str()!="while") // no while
                    syntaxError(tok);
            }
        }
    } else if (tok->str()=="if" && !Token::simpleMatch(tok->tokAt(-2), "operator \"\"")) {
        tokEnd=simplifyAddBracesPair(tok,true);
        if (!tokEnd)
            return nullptr;
        if (tokEnd->strAt(1) == "else") {
            Token * tokEndNextNext= tokEnd->tokAt(2);
            if (!tokEndNextNext || tokEndNextNext->str() == "}")
                syntaxError(tokEndNextNext);
            if (tokEndNextNext->str() == "if")
                // do not change "else if ..." to "else { if ... }"
                tokEnd=simplifyAddBracesToCommand(tokEndNextNext);
            else
                tokEnd=simplifyAddBracesPair(tokEnd->next(),false);
        }
    }

    return tokEnd;
}

Token *Tokenizer::simplifyAddBracesPair(Token *tok, bool commandWithCondition)
{
    Token * tokCondition=tok->next();
    if (!tokCondition) // Missing condition
        return tok;

    Token *tokAfterCondition=tokCondition;
    if (commandWithCondition) {
        if (tokCondition->str()=="(")
            tokAfterCondition=tokCondition->link();
        else
            syntaxError(tok); // Bad condition

        if (!tokAfterCondition || tokAfterCondition->strAt(1) == "]")
            syntaxError(tok); // Bad condition

        tokAfterCondition=tokAfterCondition->next();
        if (!tokAfterCondition || Token::Match(tokAfterCondition, ")|}|,")) {
            // No tokens left where to add braces around
            return tok;
        }
    }
    // Skip labels
    Token * tokStatement = tokAfterCondition;
    while (true) {
        if (Token::Match(tokStatement, "%name% :"))
            tokStatement = tokStatement->tokAt(2);
        else if (tokStatement->str() == "case") {
            tokStatement = skipCaseLabel(tokStatement);
            if (!tokStatement)
                return tok;
            if (tokStatement->str() != ":")
                syntaxError(tokStatement);
            tokStatement = tokStatement->next();
        } else
            break;
        if (!tokStatement)
            return tok;
    }
    Token * tokBracesEnd=nullptr;
    if (tokStatement->str() == "{") {
        // already surrounded by braces
        if (tokStatement != tokAfterCondition) {
            // Move the opening brace before labels
            Token::move(tokStatement, tokStatement, tokAfterCondition->previous());
        }
        tokBracesEnd = tokStatement->link();
    } else if (Token::simpleMatch(tokStatement, "try {") &&
               Token::simpleMatch(tokStatement->linkAt(1), "} catch (")) {
        tokAfterCondition->previous()->insertToken("{");
        Token * tokOpenBrace = tokAfterCondition->previous();
        Token * tokEnd = tokStatement->linkAt(1)->linkAt(2)->linkAt(1);
        if (!tokEnd) {
            syntaxError(tokStatement);
        }
        tokEnd->insertToken("}");
        Token * tokCloseBrace = tokEnd->next();
        tokCloseBrace->column(tokEnd->column());

        Token::createMutualLinks(tokOpenBrace, tokCloseBrace);
        tokBracesEnd = tokCloseBrace;
    } else {
        Token * tokEnd = simplifyAddBracesToCommand(tokStatement);
        if (!tokEnd) // Ticket #4887
            return tok;
        if (tokEnd->str()!="}") {
            // Token does not end with brace
            // Look for ; to add own closing brace after it
            while (tokEnd && !Token::Match(tokEnd, ";|)|}")) {
                if (tokEnd->tokType()==Token::eBracket || tokEnd->str() == "(") {
                    tokEnd = tokEnd->link();
                    if (!tokEnd) {
                        // Inner bracket does not close
                        return tok;
                    }
                }
                tokEnd=tokEnd->next();
            }
            if (!tokEnd || tokEnd->str() != ";") {
                // No trailing ;
                if (tokStatement->isUpperCaseName())
                    unknownMacroError(tokStatement);
                else
                    syntaxError(tokStatement);
            }
        }

        tokAfterCondition->previous()->insertToken("{");
        Token * tokOpenBrace=tokAfterCondition->previous();

        tokEnd->insertToken("}");
        Token * tokCloseBrace=tokEnd->next();
        tokCloseBrace->column(tokEnd->column());

        Token::createMutualLinks(tokOpenBrace,tokCloseBrace);
        tokBracesEnd=tokCloseBrace;
    }

    return tokBracesEnd;
}

void Tokenizer::simplifyFunctionParameters()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->link() && Token::Match(tok, "{|[|(")) {
            tok = tok->link();
        }

        // Find the function e.g. foo( x ) or foo( x, y )
        else if (Token::Match(tok, "%name% ( %name% [,)]") && !Token::Match(tok->tokAt(-1), ":|,|::|=")) {
            // We have found old style function, now we need to change it

            // First step: Get list of argument names in parentheses
            std::map<std::string, Token *> argumentNames;
            bool bailOut = false;
            const Token * tokparam = nullptr;

            //take count of the function name..
            const std::string& funcName(tok->str());

            //floating token used to check for parameters
            Token *tok1 = tok;

            while (nullptr != (tok1 = tok1->tokAt(2))) {
                if (!Token::Match(tok1, "%name% [,)]")) {
                    bailOut = true;
                    break;
                }

                //same parameters: take note of the parameter
                if (argumentNames.find(tok1->str()) != argumentNames.end())
                    tokparam = tok1;
                else if (tok1->str() != funcName)
                    argumentNames[tok1->str()] = tok1;
                else {
                    if (tok1->strAt(1) == ")") {
                        if (tok1->strAt(-1) == ",") {
                            tok1 = tok1->tokAt(-2);
                            tok1->deleteNext(2);
                        } else {
                            tok1 = tok1->previous();
                            tok1->deleteNext();
                            bailOut = true;
                            break;
                        }
                    } else {
                        tok1 = tok1->tokAt(-2);
                        tok1->next()->deleteNext(2);
                    }
                }

                if (tok1->strAt(1) == ")") {
                    tok1 = tok1->tokAt(2);
                    //expect at least a type name after round brace..
                    if (!tok1 || !tok1->isName())
                        bailOut = true;
                    break;
                }
            }

            //goto '('
            tok = tok->next();

            if (bailOut) {
                tok = tok->link();
                continue;
            }

            tok1 = tok->link()->next();

            // there should be the sequence '; {' after the round parentheses
            for (const Token* tok2 = tok1; tok2; tok2 = tok2->next()) {
                if (Token::simpleMatch(tok2, "; {"))
                    break;
                if (tok2->str() == "{") {
                    bailOut = true;
                    break;
                }
            }

            if (bailOut) {
                tok = tok->link();
                continue;
            }

            // Last step: check out if the declarations between ')' and '{' match the parameters list
            std::map<std::string, Token *> argumentNames2;

            while (tok1 && tok1->str() != "{") {
                if (Token::Match(tok1, "(|)")) {
                    bailOut = true;
                    break;
                }
                if (tok1->str() == ";") {
                    if (tokparam) {
                        syntaxError(tokparam);
                    }
                    Token *tok2 = tok1->previous();
                    while (tok2->str() == "]")
                        tok2 = tok2->link()->previous();

                    //it should be a name..
                    if (!tok2->isName()) {
                        bailOut = true;
                        break;
                    }

                    if (argumentNames2.find(tok2->str()) != argumentNames2.end()) {
                        //same parameter names...
                        syntaxError(tok1);
                    } else
                        argumentNames2[tok2->str()] = tok2;

                    if (argumentNames.find(tok2->str()) == argumentNames.end()) {
                        //non-matching parameter... bailout
                        bailOut = true;
                        break;
                    }
                }
                tok1 = tok1->next();
            }

            if (bailOut || !tok1) {
                tok = tok->link();
                continue;
            }

            //the two containers may not hold the same size...
            //in that case, the missing parameters are defined as 'int'
            if (argumentNames.size() != argumentNames2.size()) {
                //move back 'tok1' to the last ';'
                tok1 = tok1->previous();
                for (const std::pair<const std::string, Token *>& argumentName : argumentNames) {
                    if (argumentNames2.find(argumentName.first) == argumentNames2.end()) {
                        //add the missing parameter argument declaration
                        tok1->insertToken(";");
                        tok1->insertToken(argumentName.first);
                        //register the change inside argumentNames2
                        argumentNames2[argumentName.first] = tok1->next();
                        tok1->insertToken("int");
                    }
                }
            }

            while (tok->str() != ")") {
                //initialize start and end tokens to be moved
                Token *declStart = argumentNames2[tok->strAt(1)];
                Token *declEnd = declStart;
                while (declStart->strAt(-1) != ";" && declStart->strAt(-1) != ")")
                    declStart = declStart->previous();
                while (declEnd->strAt(1) != ";" && declEnd->strAt(1) != "{")
                    declEnd = declEnd->next();

                //remove ';' after declaration
                declEnd->deleteNext();

                //replace the parameter name in the parentheses with all the declaration
                Token::replace(tok->next(), declStart, declEnd);

                //since there are changes to tokens, put tok where tok1 is
                tok = declEnd->next();

                //fix up line number
                if (tok->str() == ",")
                    tok->linenr(tok->previous()->linenr());
            }
            //goto forward and continue
            tok = tok->linkAt(1);
        }
    }
}

void Tokenizer::simplifyPointerToStandardType()
{
    if (!isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "& %name% [ 0 ] !!["))
            continue;

        if (!Token::Match(tok->previous(), "[,(=]"))
            continue;

        // Remove '[ 0 ]' suffix
        Token::eraseTokens(tok->next(), tok->tokAt(5));
        // Remove '&' prefix
        tok = tok->previous();
        if (!tok)
            break;
        tok->deleteNext();
    }
}

void Tokenizer::simplifyFunctionPointers()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // #2873 - do not simplify function pointer usage here:
        // (void)(xy(*p)(0));
        if (Token::simpleMatch(tok, ") (")) {
            tok = tok->linkAt(1);
            continue;
        }

        // check for function pointer cast
        if (Token::Match(tok, "( %type% %type%| *| *| ( * ) (") ||
            Token::Match(tok, "static_cast < %type% %type%| *| *| ( * ) (")) {
            Token *tok1 = tok;

            if (tok1->isCpp() && tok1->str() == "static_cast")
                tok1 = tok1->next();

            tok1 = tok1->next();

            if (Token::Match(tok1->next(), "%type%"))
                tok1 = tok1->next();

            while (tok1->strAt(1) == "*")
                tok1 = tok1->next();

            // check that the cast ends
            if (!Token::Match(tok1->linkAt(4), ") )|>"))
                continue;

            // ok simplify this function pointer cast to an ordinary pointer cast
            tok1->deleteNext();
            tok1->next()->deleteNext();
            Token::eraseTokens(tok1->next(), tok1->linkAt(2)->next());
            continue;
        }

        // check for start of statement
        if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|,|(|public:|protected:|private:"))
            continue;

        if (Token::Match(tok, "delete|else|return|throw|typedef"))
            continue;

        while (Token::Match(tok, "%type%|:: %type%|::"))
            tok = tok->next();

        Token *tok2 = (tok && tok->isName()) ? tok->next() : nullptr;
        while (Token::Match(tok2, "*|&"))
            tok2 = tok2->next();
        if (!tok2 || tok2->str() != "(")
            continue;
        while (Token::Match(tok2, "(|:: %type%"))
            tok2 = tok2->tokAt(2);
        if (!Token::Match(tok2, "(|:: * *| %name%"))
            continue;
        tok2 = tok2->tokAt(2);
        if (tok2->str() == "*")
            tok2 = tok2->next();
        while (Token::Match(tok2, "%type%|:: %type%|::"))
            tok2 = tok2->next();

        if (!isFunctionPointer(tok2) &&
            !Token::Match(tok2, "%name% [ ] ) (") &&
            !(Token::Match(tok2, "%name% (") && Token::simpleMatch(tok2->linkAt(1), ") ) (")))
            continue;

        while (tok && tok->str() != "(")
            tok = tok->next();

        // check that the declaration ends
        if (!tok || !tok->link() || !tok->link()->next()) {
            syntaxError(nullptr);
        }
        Token *endTok = tok->link()->linkAt(1);
        if (Token::simpleMatch(endTok, ") throw ("))
            endTok = endTok->linkAt(2);
        if (!Token::Match(endTok, ") const|volatile| const|volatile| ;|,|)|=|[|{"))
            continue;

        while (Token::Match(endTok->next(), "const|volatile"))
            endTok->deleteNext();

        // ok simplify this function pointer to an ordinary pointer
        if (Token::simpleMatch(tok->link()->previous(), ") )")) {
            // Function returning function pointer
            // void (*dostuff(void))(void) {}
            Token::eraseTokens(tok->link(), endTok->next());
            tok->link()->deleteThis();
            tok->deleteThis();
        } else {
            Token::eraseTokens(tok->link()->linkAt(1), endTok->next());

            // remove variable names
            int indent = 0;
            for (Token* tok3 = tok->link()->tokAt(2); Token::Match(tok3, "%name%|*|&|[|(|)|::|,|<"); tok3 = tok3->next()) {
                if (tok3->str() == ")" && --indent < 0)
                    break;
                if (tok3->str() == "<" && tok3->link())
                    tok3 = tok3->link();
                else if (Token::Match(tok3, "["))
                    tok3 = tok3->link();
                else if (tok3->str() == "(") {
                    tok3 = tok3->link();
                    if (Token::simpleMatch(tok3, ") (")) {
                        tok3 = tok3->next();
                        ++indent;
                    } else
                        break;
                }
                if (Token::Match(tok3, "%type%|*|&|> %name% [,)[]"))
                    tok3->deleteNext();
            }

            // TODO Keep this info
            while (Token::Match(tok, "( %type% ::"))
                tok->deleteNext(2);
        }
    }
}

void Tokenizer::simplifyVarDecl(const bool only_k_r_fpar)
{
    simplifyVarDecl(list.front(), nullptr, only_k_r_fpar);
}

// cppcheck-suppress functionConst - has side effects
void Tokenizer::simplifyVarDecl(Token * tokBegin, const Token * const tokEnd, const bool only_k_r_fpar)
{
    const bool cpp = isCPP();
    const bool isCPP11 = cpp && (mSettings.standards.cpp >= Standards::CPP11);

    // Split up variable declarations..
    // "int a=4;" => "int a; a=4;"
    bool finishedwithkr = true;
    bool scopeDecl = false;
    for (Token *tok = tokBegin; tok != tokEnd; tok = tok->next()) {
        if (Token::Match(tok, "{|;"))
            scopeDecl = false;
        if (cpp) {
            if (Token::Match(tok, "class|struct|namespace|union"))
                scopeDecl = true;
            if (Token::Match(tok, "decltype|noexcept (")) {
                tok = tok->linkAt(1);
                // skip decltype(...){...}
                if (tok && Token::simpleMatch(tok->previous(), ") {"))
                    tok = tok->link();
            } else if (Token::simpleMatch(tok, "= {") ||
                       (!scopeDecl && Token::Match(tok, "%name%|> {") &&
                        !Token::Match(tok, "else|try|do|const|constexpr|override|volatile|noexcept"))) {
                if (!tok->linkAt(1))
                    syntaxError(tokBegin);
                // Check for lambdas before skipping
                if (Token::Match(tok->tokAt(-2), ") . %name%")) { // trailing return type
                    // TODO: support lambda without parameter clause?
                    Token* lambdaStart = tok->linkAt(-2)->previous();
                    if (Token::simpleMatch(lambdaStart, "]"))
                        lambdaStart = lambdaStart->link();
                    Token* lambdaEnd = findLambdaEndScope(lambdaStart);
                    if (lambdaEnd)
                        simplifyVarDecl(lambdaEnd->link()->next(), lambdaEnd, only_k_r_fpar);
                } else {
                    for (Token* tok2 = tok->next(); tok2 != tok->linkAt(1); tok2 = tok2->next()) {
                        Token* lambdaEnd = findLambdaEndScope(tok2);
                        if (!lambdaEnd)
                            continue;
                        simplifyVarDecl(lambdaEnd->link()->next(), lambdaEnd, only_k_r_fpar);
                    }
                }
                tok = tok->linkAt(1);
            }

        } else if (Token::simpleMatch(tok, "= {")) {
            tok = tok->linkAt(1);
        }
        if (!tok) {
            syntaxError(tokBegin);
        }
        if (only_k_r_fpar && finishedwithkr) {
            if (Token::Match(tok, "(|[|{")) {
                tok = tok->link();
                if (tok->next() && Token::Match(tok, ") !!{"))
                    tok = tok->next();
                else
                    continue;
            } else
                continue;
        } else if (tok->str() == "(") {
            if (cpp) {
                for (Token * tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                    if (Token::Match(tok2, "[(,] [")) {
                        // lambda function at tok2->next()
                        // find start of lambda body
                        Token * lambdaBody = tok2;
                        while (lambdaBody && lambdaBody != tok2->link() && lambdaBody->str() != "{")
                            lambdaBody = lambdaBody->next();
                        if (lambdaBody && lambdaBody != tok2->link() && lambdaBody->link())
                            simplifyVarDecl(lambdaBody, lambdaBody->link()->next(), only_k_r_fpar);
                    }
                }
            }
            tok = tok->link();
        }

        if (!tok)
            syntaxError(nullptr); // #7043 invalid code
        if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|)|public:|protected:|private:"))
            continue;
        if (Token::simpleMatch(tok, "template <"))
            continue;

        Token *type0 = tok;
        if (!Token::Match(type0, "::|extern| %type%"))
            continue;
        if (Token::Match(type0, "else|return|public:|protected:|private:"))
            continue;
        if (isCPP11 && type0->str() == "using")
            continue;
        if (cpp && Token::Match(type0, "namespace|delete"))
            continue;

        bool isconst = false;
        bool isstatic = false;
        Token *tok2 = type0;
        int typelen = 1;

        if (Token::Match(tok2, "::|extern")) {
            tok2 = tok2->next();
            typelen++;
        }

        //check if variable is declared 'const' or 'static' or both
        while (tok2) {
            if (!Token::Match(tok2, "const|static|constexpr") && Token::Match(tok2, "%type% const|static")) {
                tok2 = tok2->next();
                ++typelen;
            }

            if (Token::Match(tok2, "const|constexpr"))
                isconst = true;

            else if (Token::Match(tok2, "static|constexpr"))
                isstatic = true;

            else if (Token::Match(tok2, "%type% :: %type%")) {
                tok2 = tok2->next();
                ++typelen;
            }

            else
                break;

            if (tok2->strAt(1) == "*")
                break;

            if (Token::Match(tok2->next(), "& %name% ,"))
                break;

            tok2 = tok2->next();
            ++typelen;
        }

        // strange looking variable declaration => don't split up.
        if (Token::Match(tok2, "%type% *|&| %name% , %type% *|&| %name%"))
            continue;

        if (Token::Match(tok2, "struct|union|class %type%")) {
            tok2 = tok2->next();
            ++typelen;
        }

        // check for qualification..
        if (Token::Match(tok2,  ":: %type%")) {
            ++typelen;
            tok2 = tok2->next();
        }

        //skip combinations of templates and namespaces
        while (!isC() && (Token::Match(tok2, "%type% <") || Token::Match(tok2, "%type% ::"))) {
            if (tok2->strAt(1) == "<" && !TemplateSimplifier::templateParameters(tok2->next())) {
                tok2 = nullptr;
                break;
            }
            typelen += 2;
            tok2 = tok2->tokAt(2);
            if (tok2 && tok2->strAt(-1) == "::")
                continue;
            int indentlevel = 0;
            int parens = 0;

            for (Token *tok3 = tok2; tok3; tok3 = tok3->next()) {
                ++typelen;

                if (!parens && tok3->str() == "<") {
                    ++indentlevel;
                } else if (!parens && tok3->str() == ">") {
                    if (indentlevel == 0) {
                        tok2 = tok3->next();
                        break;
                    }
                    --indentlevel;
                } else if (!parens && tok3->str() == ">>") {
                    if (indentlevel <= 1) {
                        tok2 = tok3->next();
                        break;
                    }
                    indentlevel -= 2;
                } else if (tok3->str() == "(") {
                    ++parens;
                } else if (tok3->str() == ")") {
                    if (!parens) {
                        tok2 = nullptr;
                        break;
                    }
                    --parens;
                } else if (tok3->str() == ";") {
                    break;
                }
            }

            if (Token::Match(tok2,  ":: %type%")) {
                ++typelen;
                tok2 = tok2->next();
            }

            // east const
            if (Token::simpleMatch(tok2, "const"))
                isconst = true;
        }

        //pattern: "%type% *| ... *| const| %name% ,|="
        if (Token::Match(tok2, "%type%") ||
            (tok2 && tok2->previous() && tok2->strAt(-1) == ">")) {
            Token *varName = tok2;
            if (!tok2->previous() || tok2->strAt(-1) != ">")
                varName = varName->next();
            else
                --typelen;
            if (cpp && Token::Match(varName, "public:|private:|protected:|using"))
                continue;
            //skip all the pointer part
            bool isPointerOrRef = false;
            while (Token::simpleMatch(varName, "*") || Token::Match(varName, "& %name% ,")) {
                isPointerOrRef = true;
                varName = varName->next();
            }

            while (Token::Match(varName, "%type% %type%")) {
                if (varName->str() != "const" && varName->str() != "volatile") {
                    ++typelen;
                }
                varName = varName->next();
            }
            // Function pointer
            if (Token::simpleMatch(varName, "( *") &&
                isFunctionPointer(varName->link()->previous()) &&
                Token::simpleMatch(varName->link()->linkAt(1), ") =")) {
                Token *endDecl = varName->link()->linkAt(1);
                varName = varName->link()->previous();
                endDecl->insertToken(";");
                endDecl = endDecl->next();
                endDecl->next()->isSplittedVarDeclEq(true);
                endDecl->insertToken(varName->str());
                if (!varName->getMacroName().empty())
                    endDecl->next()->setMacroName(varName->getMacroName());
                continue;
            }
            //non-VLA case
            if (Token::Match(varName, "%name% ,|=")) {
                if (varName->str() != "operator") {
                    tok2 = varName->next(); // The ',' or '=' token

                    if (tok2->str() == "=" && (isstatic || (isconst && !isPointerOrRef))) {
                        //do not split const non-pointer variables..
                        while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                            if (Token::Match(tok2, "{|(|["))
                                tok2 = tok2->link();
                            const Token *tok3 = tok2;
                            if (!isC() && tok2->str() == "<" && TemplateSimplifier::templateParameters(tok2) > 0) {
                                tok2 = tok2->findClosingBracket();
                            }
                            if (!tok2)
                                syntaxError(tok3); // #6881 invalid code
                            tok2 = tok2->next();
                        }
                        if (tok2 && tok2->str() == ";")
                            tok2 = nullptr;
                    }
                } else
                    tok2 = nullptr;
            }

            //VLA case
            else if (Token::Match(varName, "%name% [")) {
                tok2 = varName->next();

                while (Token::Match(tok2->link(), "] [,=[{]"))
                    tok2 = tok2->link()->next();
                if (!Token::Match(tok2, "[=,{]"))
                    tok2 = nullptr;
                if (tok2 && tok2->str() == "=") {
                    while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                        if (Token::Match(tok2, "{|(|["))
                            tok2 = tok2->link();
                        tok2 = tok2->next();
                    }
                    if (tok2 && tok2->str() == ";")
                        tok2 = nullptr;
                }
            }

            // brace initialization
            else if (Token::Match(varName, "%name% {")) {
                tok2 = varName->next();
                tok2 = tok2->link();
                if (tok2)
                    tok2 = tok2->next();
                if (tok2 && tok2->str() != ",")
                    tok2 = nullptr;
            }

            // function declaration
            else if (Token::Match(varName, "%name% (")) {
                Token* commaTok = varName->linkAt(1)->next();
                while (Token::Match(commaTok, "const|noexcept|override|final")) {
                    commaTok = commaTok->next();
                    if (Token::Match(commaTok, "( true|false )"))
                        commaTok = commaTok->link()->next();
                }
                tok2 = Token::simpleMatch(commaTok, ",") ? commaTok : nullptr;
            }

            else
                tok2 = nullptr;
        } else {
            tok2 = nullptr;
        }

        if (!tok2) {
            if (only_k_r_fpar)
                finishedwithkr = false;
            continue;
        }

        if (tok2->str() == ",") {
            tok2->str(";");
            tok2->isSplittedVarDeclComma(true);
            //TODO: should we have to add also template '<>' links?
            TokenList::insertTokens(tok2, type0, typelen);
        }

        else {
            Token *eq = tok2;

            while (tok2) {
                if (Token::Match(tok2, "{|(|["))
                    tok2 = tok2->link();

                else if (!isC() && tok2->str() == "<" && ((tok2->previous()->isName() && !tok2->previous()->varId()) || tok2->strAt(-1) == "]"))
                    tok2 = tok2->findClosingBracket();

                else if (std::strchr(";,", tok2->str()[0])) {
                    // "type var ="   =>   "type var; var ="
                    const Token *varTok = type0->tokAt(typelen);
                    while (Token::Match(varTok, "%name%|*|& %name%|*|&"))
                        varTok = varTok->next();
                    if (!varTok)
                        syntaxError(tok2); // invalid code
                    if (eq->str() == "=") {
                        TokenList::insertTokens(eq, varTok, 2);
                        eq->str(";");
                        eq->isSplittedVarDeclEq(true);
                    }

                    // "= x, "   =>   "= x; type "
                    if (tok2->str() == ",") {
                        tok2->str(";");
                        tok2->isSplittedVarDeclComma(true);
                        TokenList::insertTokens(tok2, type0, typelen);
                    }
                    break;
                }
                if (tok2)
                    tok2 = tok2->next();
            }
        }
        finishedwithkr = (only_k_r_fpar && tok2 && tok2->strAt(1) == "{");
    }
}

void Tokenizer::simplifyStaticConst()
{
    // This function will simplify the token list so that the qualifiers "extern", "static"
    // and "const" appear in the same order as in the array below.
    static const std::array<std::string, 3> qualifiers = {"extern", "static", "const"};

    // Move 'const' before all other qualifiers and types and then
    // move 'static' before all other qualifiers and types, ...
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        bool continue2 = false;
        for (std::size_t i = 0; i < qualifiers.size(); i++) {

            // Keep searching for a qualifier
            if (!tok->next() || tok->strAt(1) != qualifiers[i])
                continue;

            // Look backwards to find the beginning of the declaration
            Token* leftTok = tok;
            bool behindOther = false;
            for (; leftTok; leftTok = leftTok->previous()) {
                behindOther = std::any_of(qualifiers.cbegin(), qualifiers.cbegin() + i + 1, [&](const std::string& q) {
                    return q == leftTok->str();
                });
                if (behindOther)
                    break;
                if (isCPP() && Token::simpleMatch(leftTok, ">")) {
                    Token* opening = leftTok->findOpeningBracket();
                    if (opening) {
                        leftTok = opening;
                        continue;
                    }
                }
                if (!Token::Match(leftTok, "%type%|struct|::") ||
                    (isCPP() && Token::Match(leftTok, "private:|protected:|public:|operator|template"))) {
                    break;
                }
            }

            // The token preceding the declaration should indicate the start of a declaration
            if (leftTok == tok)
                continue;

            if (leftTok && !behindOther && !Token::Match(leftTok, ";|{|}|(|,|private:|protected:|public:")) {
                continue2 = true;
                break;
            }

            // Move the qualifier to the left-most position in the declaration
            const int column = tok->next()->column();
            tok->deleteNext();
            if (!leftTok) {
                list.front()->insertToken(qualifiers[i]);
                list.front()->swapWithNext();
                list.front()->column(column);
                tok = list.front();
            } else if (leftTok->next()) {
                leftTok->next()->insertTokenBefore(qualifiers[i]);
                leftTok->next()->column(column);
                tok = leftTok->next();
            } else {
                leftTok->insertToken(qualifiers[i]);
                leftTok->next()->column(column);
                tok = leftTok;
            }
        }
        if (continue2)
            continue;
    }
}

void Tokenizer::simplifyVariableMultipleAssign()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% = %name% = %num%|%name% ;")) {
            // skip intermediate assignments
            Token *tok2 = tok->previous();
            while (tok2 &&
                   tok2->str() == "=" &&
                   Token::Match(tok2->previous(), "%name%")) {
                tok2 = tok2->tokAt(-2);
            }

            if (!tok2 || tok2->str() != ";") {
                continue;
            }

            Token *stopAt = tok->tokAt(2);
            const Token *valueTok = stopAt->tokAt(2);
            const std::string& value(valueTok->str());
            tok2 = tok2->next();

            while (tok2 != stopAt) {
                tok2->next()->insertToken(";");
                tok2->next()->insertToken(value);
                tok2 = tok2->tokAt(4);
            }
        }
    }
}

// Binary operators simplification map
static const std::unordered_map<std::string, std::string> cAlternativeTokens = {
    std::make_pair("and", "&&")
    , std::make_pair("and_eq", "&=")
    , std::make_pair("bitand", "&")
    , std::make_pair("bitor", "|")
    , std::make_pair("not_eq", "!=")
    , std::make_pair("or", "||")
    , std::make_pair("or_eq", "|=")
    , std::make_pair("xor", "^")
    , std::make_pair("xor_eq", "^=")
};

// Simplify the C alternative tokens:
//  and      =>     &&
//  and_eq   =>     &=
//  bitand   =>     &
//  bitor    =>     |
//  compl    =>     ~
//  not      =>     !
//  not_eq   =>     !=
//  or       =>     ||
//  or_eq    =>     |=
//  xor      =>     ^
//  xor_eq   =>     ^=
bool Tokenizer::simplifyCAlternativeTokens()
{
    /* executable scope level */
    int executableScopeLevel = 0;

    std::vector<Token *> alt;
    bool replaceAll = false;  // replace all or none

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == ")") {
            if (const Token *end = TokenList::isFunctionHead(tok, "{")) {
                ++executableScopeLevel;
                tok = const_cast<Token *>(end);
                continue;
            }
        }

        if (tok->str() == "{") {
            if (executableScopeLevel > 0)
                ++executableScopeLevel;
            continue;
        }

        if (tok->str() == "}") {
            if (executableScopeLevel > 0)
                --executableScopeLevel;
            continue;
        }

        if (!tok->isName())
            continue;

        const auto cOpIt = cAlternativeTokens.find(tok->str());
        if (cOpIt != cAlternativeTokens.end()) {
            alt.push_back(tok);

            // Is this a variable declaration..
            if (isC() && Token::Match(tok->previous(), "%type%|* %name% [;,=)]"))
                return false;

            if (!Token::Match(tok->previous(), "%name%|%num%|%char%|%str%|)|]|> %name% %name%|%num%|%char%|%op%|%str%|("))
                continue;
            if (Token::Match(tok->next(), "%assign%|%or%|%oror%|&&|*|/|%|^") && !Token::Match(tok->previous(), "%num%|%char%|) %name% *"))
                continue;
            if (executableScopeLevel == 0 && Token::Match(tok, "%name% (")) {
                const Token *start = tok;
                while (Token::Match(start, "%name%|*"))
                    start = start->previous();
                if (!start || Token::Match(start, "[;}]"))
                    continue;
            }
            replaceAll = true;
        } else if (Token::Match(tok, "not|compl")) {
            alt.push_back(tok);

            // Is this a variable/parameter declaration
            if (isC() && Token::Match(tok->previous(), "%type%|* %name% [;,=)]"))
                return false;

            if ((Token::Match(tok->previous(), "%assign%") || Token::Match(tok->next(), "%num%")) && !Token::Match(tok->next(), ".|->")) {
                replaceAll = true;
                continue;
            }

            // Don't simplify 'not p;' (in case 'not' is a type)
            if (!Token::Match(tok->next(), "%name%|(") ||
                Token::Match(tok->previous(), "[;{}]") ||
                (executableScopeLevel == 0U && tok->strAt(-1) == "("))
                continue;

            replaceAll = true;
        }
    }

    if (!replaceAll)
        return false;

    for (Token *tok: alt) {
        const auto cOpIt = cAlternativeTokens.find(tok->str());
        if (cOpIt != cAlternativeTokens.end())
            tok->str(cOpIt->second);
        else if (tok->str() == "not")
            tok->str("!");
        else
            tok->str("~");
    }

    return !alt.empty();
}

// int i(0); => int i; i = 0;
// int i(0), j; => int i; i = 0; int j;
void Tokenizer::simplifyInitVar()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || (tok->previous() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        if (tok->str() == "return")
            continue;

        if (Token::Match(tok, "%type% *|&| %name% (|{")) {
            bool isNamespace = Token::simpleMatch(tok, "namespace");
            while (tok && !Token::Match(tok, "(|{"))
                tok = tok->next();
            if (isNamespace)
                continue;
            if (tok)
                tok->isInitBracket(true);
            /* tok = initVar(tok);
               } else if (Token::Match(tok, "%type% *| %name% ( %type% (")) {
               const Token* tok2 = tok->tokAt(2);
               if (!tok2->link())
                tok2 = tok2->next();
               if (!tok2->link() || (tok2->link()->strAt(1) == ";" && !Token::simpleMatch(tok2->linkAt(2), ") (")))
                tok = initVar(tok);
               } else if (Token::Match(tok, "class|struct|union| %type% *| %name% ( &| %any% ) ,") && tok->str() != "new") {
               Token *tok1 = tok->tokAt(5);
               while (tok1->str() != ",")
                tok1 = tok1->next();
               tok1->str(";");

               const int numTokens = (Token::Match(tok, "class|struct|union")) ? 2 : 1;
               TokenList::insertTokens(tok1, tok, numTokens);
               tok = initVar(tok); */
        }
    }
}

Token * Tokenizer::initVar(Token * tok)
{
    // call constructor of class => no simplification
    if (Token::Match(tok, "class|struct|union")) {
        if (tok->strAt(2) != "*")
            return tok;

        tok = tok->next();
    } else if (!tok->isStandardType() && tok->str() != "auto" && tok->strAt(1) != "*")
        return tok;

    // goto variable name..
    tok = tok->next();
    if (tok->str() == "*")
        tok = tok->next();

    // sizeof is not a variable name..
    if (tok->str() == "sizeof")
        return tok;

    // check initializer..
    if (tok->tokAt(2)->isStandardType() || tok->strAt(2) == "void")
        return tok;
    if (!tok->tokAt(2)->isNumber() && !Token::Match(tok->tokAt(2), "%type% (|{") && tok->strAt(2) != "&" && tok->tokAt(2)->varId() == 0)
        return tok;

    // insert '; var ='
    tok->insertToken(";");
    tok->next()->insertToken(tok->str());
    tok->next()->isSplittedVarDeclEq(true);
    tok->tokAt(2)->varId(tok->varId());
    tok = tok->tokAt(2);
    tok->insertToken("=");

    // goto '('..
    tok = tok->tokAt(2);

    // delete ')'
    tok->link()->deleteThis();

    // delete this
    tok->deleteThis();

    return tok;
}

void Tokenizer::elseif()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "else")
            continue;

        if (!Token::Match(tok->previous(), ";|}"))
            syntaxError(tok->previous());

        if (!Token::Match(tok->next(), "%name%"))
            continue;

        if (tok->strAt(1) != "if")
            unknownMacroError(tok->next());

        for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "(|{|["))
                tok2 = tok2->link();

            if (Token::Match(tok2, "}|;")) {
                if (tok2->next() && tok2->strAt(1) != "else") {
                    tok->insertToken("{")->isSimplifiedScope(true);
                    tok2->insertToken("}")->isSimplifiedScope(true);
                    Token::createMutualLinks(tok->next(), tok2->next());
                    break;
                }
            }
        }
    }
}


void Tokenizer::simplifyIfSwitchForInit()
{
    if (!isCPP() || mSettings.standards.cpp < Standards::CPP17)
        return;

    const bool forInit = (mSettings.standards.cpp >= Standards::CPP20);

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "if|switch|for ("))
            continue;

        Token *semicolon = tok->tokAt(2);
        while (!Token::Match(semicolon, "[;)]")) {
            if (Token::Match(semicolon, "(|{|[") && semicolon->link())
                semicolon = semicolon->link();
            semicolon = semicolon->next();
        }
        if (semicolon->str() != ";")
            continue;

        if (tok->str() ==  "for") {
            if (!forInit)
                continue;

            // Is it a for range..
            const Token *tok2 = semicolon->next();
            bool rangeFor = false;
            while (!Token::Match(tok2, "[;)]")) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (!rangeFor && tok2->str() == "?")
                    break;
                else if (tok2->str() == ":")
                    rangeFor = true;
                tok2 = tok2->next();
            }
            if (!rangeFor || tok2->str() != ")")
                continue;
        }

        Token *endpar = tok->linkAt(1);
        if (!Token::simpleMatch(endpar, ") {"))
            continue;

        Token *endscope = endpar->linkAt(1);
        if (Token::simpleMatch(endscope, "} else {"))
            endscope = endscope->linkAt(2);

        // Simplify, the initialization expression is broken out..
        semicolon->insertToken(tok->str());
        semicolon->next()->insertToken("(");
        Token::createMutualLinks(semicolon->tokAt(2), endpar);
        tok->deleteNext();
        tok->str("{");
        endscope->insertToken("}");
        Token::createMutualLinks(tok, endscope->next());
        tok->isSimplifiedScope(true);
    }
}


bool Tokenizer::simplifyRedundantParentheses()
{
    bool ret = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "(")
            continue;

        if (tok->isCpp() && Token::simpleMatch(tok->previous(), "} (")) {
            const Token* plp = tok->linkAt(-1)->previous();
            if (Token::Match(plp, "%name%|>|] {") || (Token::simpleMatch(plp, ")") && Token::simpleMatch(plp->link()->previous(), "]")))
                continue;
        }

        if (Token::simpleMatch(tok, "( {"))
            continue;

        if (Token::Match(tok->link(), ") %num%")) {
            tok = tok->link();
            continue;
        }

        // Do not simplify if there is comma inside parentheses..
        if (Token::Match(tok->previous(), "%op% (") || Token::Match(tok->link(), ") %op%")) {
            bool innerComma = false;
            for (const Token *inner = tok->link()->previous(); inner != tok; inner = inner->previous()) {
                if (inner->str() == ")")
                    inner = inner->link();
                if (inner->str() == ",") {
                    innerComma = true;
                    break;
                }
            }
            if (innerComma)
                continue;
        }

        // !!operator = ( x ) ;
        if (tok->strAt(-2) != "operator" &&
            tok->previous() && tok->strAt(-1) == "=" &&
            tok->next() && tok->strAt(1) != "{" &&
            Token::simpleMatch(tok->link(), ") ;")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            continue;
        }

        if (isCPP() && Token::Match(tok->tokAt(-2), "[;{}=(] new (") && Token::Match(tok->link(), ") [;,{}[]")) {
            // Remove the parentheses in "new (type)" constructs
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "! ( %name% )")) {
            // Remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(,;{}] ( %name% ) .")) {
            // Remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(,;{}] ( %name% (") && !tok->next()->isKeyword() &&
            tok->link()->previous() == tok->linkAt(2)) {
            // We have "( func ( *something* ))", remove the outer
            // parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[,;{}] ( delete [| ]| %name% ) ;")) {
            // We have "( delete [| ]| var )", remove the outer
            // parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (!Token::simpleMatch(tok->tokAt(-2), "operator delete") &&
            Token::Match(tok->previous(), "delete|; (") &&
            (tok->strAt(-1) != "delete" || tok->next()->varId() > 0) &&
            Token::Match(tok->link(), ") ;|,")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(!*;{}] ( %name% )") &&
            (tok->next()->varId() != 0 || Token::Match(tok->tokAt(3), "[+-/=]")) && !tok->next()->isStandardType()) {
            // We have "( var )", remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{}[(,!*] ( %name% .")) {
            Token *tok2 = tok->tokAt(2);
            while (Token::Match(tok2, ". %name%")) {
                tok2 = tok2->tokAt(2);
            }
            if (tok2 != tok->link())
                break;
            // We have "( var . var . ... . var )", remove the parentheses
            tok = tok->previous();
            tok->deleteNext();
            tok2->deleteThis();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[{([,] ( !!{") &&
               Token::Match(tok->link(), ") [;,])] !!{") &&
               !Token::simpleMatch(tok->tokAt(-2), "operator ,") && // Ticket #5709
               !Token::findsimplematch(tok, ",", tok->link())) {
            // We have "( ... )", remove the parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::simpleMatch(tok->previous(), ", (") &&
            Token::simpleMatch(tok->link(), ") =")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        // Simplify "!!operator !!%name%|)|]|>|>> ( %num%|%bool% ) %op%|;|,|)"
        if (Token::Match(tok, "( %bool%|%num% ) %cop%|;|,|)") &&
            tok->strAt(-2) != "operator" &&
            tok->previous() &&
            !Token::Match(tok->previous(), "%name%|)|]") &&
            (!(isCPP() && Token::Match(tok->previous(),">|>>")))) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "*|& ( %name% )")) {
            // We may have a variable declaration looking like "type_name *(var_name)"
            Token *tok2 = tok->tokAt(-2);
            while (Token::Match(tok2, "%type%|static|const|extern") && tok2->str() != "operator") {
                tok2 = tok2->previous();
            }
            if (tok2 && !Token::Match(tok2, "[;{]")) {
                // Not a variable declaration
            } else {
                tok->deleteThis();
                tok->deleteNext();
            }
        }
    }
    return ret;
}

void Tokenizer::simplifyTypeIntrinsics()
{
    static const std::unordered_map<std::string, std::string> intrinsics = {
        { "__has_nothrow_assign", "has_nothrow_assign" },
        { "__has_nothrow_constructor", "has_nothrow_constructor" },
        { "__has_nothrow_copy", "has_nothrow_copy" },
        { "__has_trivial_assign", "has_trivial_assign" },
        { "__has_trivial_constructor", "has_trivial_constructor" },
        { "__has_trivial_copy", "has_trivial_copy" },
        { "__has_trivial_destructor", "has_trivial_destructor" },
        { "__has_virtual_destructor", "has_virtual_destructor" },
        { "__is_abstract", "is_abstract" },
        { "__is_aggregate", "is_aggregate" },
        { "__is_assignable", "is_assignable" },
        { "__is_base_of", "is_base_of" },
        { "__is_class", "is_class" },
        { "__is_constructible", "is_constructible" },
        { "__is_convertible_to", "is_convertible_to" },
        { "__is_destructible", "is_destructible" },
        { "__is_empty", "is_empty" },
        { "__is_enum", "is_enum" },
        { "__is_final", "is_final" },
        { "__is_nothrow_assignable", "is_nothrow_assignable" },
        { "__is_nothrow_constructible", "is_nothrow_constructible" },
        { "__is_nothrow_destructible", "is_nothrow_destructible" },
        { "__is_pod", "is_pod" },
        { "__is_polymorphic", "is_polymorphic" },
        { "__is_trivially_assignable", "is_trivially_assignable" },
        { "__is_trivially_constructible", "is_trivially_constructible" },
        { "__is_union", "is_union" },
    };
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;
        auto p = intrinsics.find(tok->str());
        if (p == intrinsics.end())
            continue;
        Token * end = tok->linkAt(1);
        Token * prev = tok->previous();
        tok->str(p->second);
        prev->insertToken("::");
        prev->insertToken("std");
        tok->next()->str("<");
        end->str(">");
        end->insertToken("}");
        end->insertToken("{");
        Token::createMutualLinks(end->tokAt(1), end->tokAt(2));
    }
}

//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

bool Tokenizer::isScopeNoReturn(const Token *endScopeToken, bool *unknown) const
{
    std::string unknownFunc;
    const bool ret = mSettings.library.isScopeNoReturn(endScopeToken,&unknownFunc);
    if (!unknownFunc.empty() && mSettings.summaryReturn.find(unknownFunc) != mSettings.summaryReturn.end()) {
        return false;
    }
    if (unknown)
        *unknown = !unknownFunc.empty();
    if (!unknownFunc.empty() && mSettings.checkLibrary) {
        bool warn = true;
        if (Token::simpleMatch(endScopeToken->tokAt(-2), ") ; }")) {
            const Token * const ftok = endScopeToken->linkAt(-2)->previous();
            if (ftok && (ftok->type() || ftok->function() || ftok->variable())) // constructor call
                warn = false;
        }

        if (warn) {
            reportError(endScopeToken->previous(),
                        Severity::information,
                        "checkLibraryNoReturn",
                        "--check-library: Function " + unknownFunc + "() should have <noreturn> configuration");
        }
    }
    return ret;
}

//---------------------------------------------------------------------------

void Tokenizer::syntaxError(const Token *tok, const std::string &code) const
{
    if (mSettings.debugSimplified || mSettings.debugnormal)
        printDebugOutput(std::cout);
    throw InternalError(tok, code.empty() ? "syntax error" : "syntax error: " + code, InternalError::SYNTAX);
}

void Tokenizer::unmatchedToken(const Token *tok) const
{
    if (mSettings.debugSimplified || mSettings.debugnormal)
        printDebugOutput(std::cout);
    throw InternalError(tok,
                        "Unmatched '" + tok->str() + "'. Configuration: '" + mConfiguration + "'.",
                        InternalError::SYNTAX);
}

void Tokenizer::syntaxErrorC(const Token *tok, const std::string &what) const
{
    if (mSettings.debugSimplified || mSettings.debugnormal)
        printDebugOutput(std::cout);
    throw InternalError(tok, "Code '"+what+"' is invalid C code.", "Use --std, -x or --language to enforce C++. Or --cpp-header-probe to identify C++ headers via the Emacs marker.", InternalError::SYNTAX);
}

void Tokenizer::unknownMacroError(const Token *tok1) const
{
    if (mSettings.debugSimplified || mSettings.debugnormal)
        printDebugOutput(std::cout);
    throw InternalError(tok1, "There is an unknown macro here somewhere. Configuration is required. If " + tok1->str() + " is a macro then please configure it.", InternalError::UNKNOWN_MACRO);
}

void Tokenizer::unhandled_macro_class_x_y(const Token *tok, const std::string& type, const std::string& x, const std::string& y, const std::string& bracket) const
{
    reportError(tok,
                Severity::information,
                "class_X_Y",
                "The code '" +
                type + " " +
                x + " " +
                y + " " +
                bracket + "' is not handled. You can use -I or --include to add handling of this code.");
}

void Tokenizer::invalidConstFunctionTypeError(const Token *tok) const
{
    reportError(tok,
                Severity::portability,
                "invalidConstFunctionType",
                "It is unspecified behavior to const qualify a function type.");
}

void Tokenizer::cppcheckError(const Token *tok) const
{
    printDebugOutput(std::cout);
    throw InternalError(tok, "Analysis failed. If the code is valid then please report this failure.", InternalError::INTERNAL);
}

void Tokenizer::unhandledCharLiteral(const Token *tok, const std::string& msg) const
{
    std::string s = tok ? (" " + tok->str()) : "";
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (static_cast<unsigned char>(s[i]) >= 0x80)
            s.clear();
    }

    reportError(tok,
                Severity::portability,
                "nonStandardCharLiteral",
                "Non-standard character literal" + s + ". " + msg);
}

/**
 * Helper function to check whether number is equal to integer constant X
 * or floating point pattern X.0
 * @param s the string to check
 * @param intConstant the integer constant to check against
 * @param floatConstant the string with stringified float constant to check against
 * @return true in case s is equal to X or X.0 and false otherwise.
 */
static bool isNumberOneOf(const std::string &s, MathLib::bigint intConstant, const char* floatConstant)
{
    if (MathLib::isInt(s)) {
        if (MathLib::toBigNumber(s) == intConstant)
            return true;
    } else if (MathLib::isFloat(s)) {
        if (MathLib::toString(MathLib::toDoubleNumber(s)) == floatConstant)
            return true;
    }
    return false;
}

// ------------------------------------------------------------------------
// Helper function to check whether number is one (1 or 0.1E+1 or 1E+0) or not?
// @param s the string to check
// @return true in case s is one and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isOneNumber(const std::string &s)
{
    if (!MathLib::isPositive(s))
        return false;
    return isNumberOneOf(s, 1L, "1.0");
}
// ------------------------------------------------------------------------

void Tokenizer::validateC() const
{
    if (isCPP())
        return;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        // That might trigger false positives, but it's much faster to have this truncated pattern
        if (Token::Match(tok, "const_cast|dynamic_cast|reinterpret_cast|static_cast <"))
            syntaxErrorC(tok, "C++ cast <...");
        // Template function..
        if (Token::Match(tok, "%name% < %name% > (")) {
            const Token *tok2 = tok->tokAt(5);
            while (tok2 && !Token::Match(tok2, "[()]"))
                tok2 = tok2->next();
            if (Token::simpleMatch(tok2, ") {"))
                syntaxErrorC(tok, tok->str() + '<' + tok->strAt(2) + ">() {}");
        }
        if (tok->previous() && !Token::Match(tok->previous(), "[;{}]"))
            continue;
        if (Token::Match(tok, "using namespace %name% ;"))
            syntaxErrorC(tok, "using namespace " + tok->strAt(2));
        if (Token::Match(tok, "template < class|typename %name% [,>]"))
            syntaxErrorC(tok, "template<...");
        if (Token::Match(tok, "%name% :: %name%"))
            syntaxErrorC(tok, tok->str() + tok->strAt(1) + tok->strAt(2));
        if (Token::Match(tok, "class|namespace %name% :|::|{"))
            syntaxErrorC(tok, tok->str() + tok->strAt(1) + tok->strAt(2));
    }
}

void Tokenizer::validate() const
{
    std::stack<const Token *> linkTokens;
    const Token *lastTok = nullptr;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        lastTok = tok;
        if (Token::Match(tok, "[{([]") || (tok->str() == "<" && tok->link())) {
            if (tok->link() == nullptr)
                cppcheckError(tok);

            linkTokens.push(tok);
        }

        else if (Token::Match(tok, "[})]]") || (Token::Match(tok, ">|>>") && tok->link())) {
            if (tok->link() == nullptr)
                cppcheckError(tok);

            if (linkTokens.empty())
                cppcheckError(tok);

            if (tok->link() != linkTokens.top())
                cppcheckError(tok);

            if (tok != tok->link()->link())
                cppcheckError(tok);

            linkTokens.pop();
        }

        else if (tok->link() != nullptr)
            cppcheckError(tok);
    }

    if (!linkTokens.empty())
        cppcheckError(linkTokens.top());

    // Validate that the Tokenizer::list.back() is updated correctly during simplifications
    if (lastTok != list.back())
        cppcheckError(lastTok);
}

static const Token *findUnmatchedTernaryOp(const Token * const begin, const Token * const end, int depth = 0)
{
    std::stack<const Token *> ternaryOp;
    for (const Token *tok = begin; tok != end && tok->str() != ";"; tok = tok->next()) {
        if (tok->str() == "?")
            ternaryOp.push(tok);
        else if (!ternaryOp.empty() && tok->str() == ":")
            ternaryOp.pop();
        else if (depth < 100 && Token::Match(tok,"(|[")) {
            const Token *inner = findUnmatchedTernaryOp(tok->next(), tok->link(), depth+1);
            if (inner)
                return inner;
            tok = tok->link();
        }
    }
    return ternaryOp.empty() ? nullptr : ternaryOp.top();
}

static bool isCPPAttribute(const Token * tok)
{
    return Token::simpleMatch(tok, "[ [") && tok->link() && tok->link()->previous() == tok->linkAt(1);
}

static bool isAlignAttribute(const Token * tok)
{
    return Token::simpleMatch(tok, "alignas (") && tok->linkAt(1);
}

template<typename T>
static T* skipCPPOrAlignAttribute(T * tok)
{
    if (isCPPAttribute(tok))
        return tok->link();
    if (isAlignAttribute(tok)) {
        return tok->linkAt(1);
    }
    return tok;
}

static bool isNonMacro(const Token* tok)
{
    if (tok->isKeyword() || tok->isStandardType())
        return true;
    if (cAlternativeTokens.count(tok->str()) > 0)
        return true;
    if (startsWith(tok->str(), "__")) // attribute/annotation
        return true;
    if (Token::simpleMatch(tok, "alignas ("))
        return true;
    return false;
}

void Tokenizer::reportUnknownMacros() const
{
    // Report unknown macros used in expressions "%name% %num%"
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% %num%")) {
            // A keyword is not an unknown macro
            if (tok->isKeyword())
                continue;

            if (Token::Match(tok->previous(), "%op%|("))
                unknownMacroError(tok);
        }
    }

    // Report unknown macros before } "{ .. if (x) MACRO }"
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, ")|; %name% } !!)")) {
            if (tok->link() && !Token::simpleMatch(tok->link()->tokAt(-1), "if"))
                continue;
            const Token* prev = tok->linkAt(2);
            while (Token::simpleMatch(prev, "{"))
                prev = prev->previous();
            if (Token::Match(prev, ";|)"))
                unknownMacroError(tok->next());
        }
    }

    // Report unknown macros that contain several statements "MACRO(a;b;c)"
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;
        if (!tok->isUpperCaseName())
            continue;
        const Token *endTok = tok->linkAt(1);
        for (const Token *inner = tok->tokAt(2); inner != endTok; inner = inner->next()) {
            if (Token::Match(inner, "[[({]"))
                inner = inner->link();
            else if (inner->str() == ";")
                unknownMacroError(tok);
        }
    }

    // Report unknown macros that contain struct initialization "MACRO(a, .b=3)"
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;
        const Token *endTok = tok->linkAt(1);
        for (const Token *inner = tok->tokAt(2); inner != endTok; inner = inner->next()) {
            if (Token::Match(inner, "[[({]"))
                inner = inner->link();
            else if (Token::Match(inner->previous(), "[,(] . %name% =|{"))
                unknownMacroError(tok);
        }
    }

    const bool cpp = isCPP();

    // Report unknown macros in non-executable scopes..
    std::set<std::string> possible;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        // Skip executable scopes..
        if (tok->str() == "{") {
            const Token *prev = tok->previous();
            while (prev && prev->isName())
                prev = prev->previous();
            if (prev && prev->str() == ")")
                tok = tok->link();
            else
                possible.clear();
        } else if (tok->str() == "}")
            possible.clear();

        if (Token::Match(tok, "%name% (") && tok->isUpperCaseName() && Token::simpleMatch(tok->linkAt(1), ") (") && Token::simpleMatch(tok->linkAt(1)->linkAt(1), ") {")) {
            // A keyword is not an unknown macro
            if (tok->isKeyword())
                continue;

            const Token *bodyStart = tok->linkAt(1)->linkAt(1)->tokAt(2);
            const Token *bodyEnd = tok->link();
            for (const Token *tok2 = bodyStart; tok2 && tok2 != bodyEnd; tok2 = tok2->next()) {
                if (Token::Match(tok2, "if|switch|for|while|return"))
                    unknownMacroError(tok);
            }
        } else if (Token::Match(tok, "%name% (") && tok->isUpperCaseName() && Token::Match(tok->linkAt(1), ") %name% (") && Token::Match(tok->linkAt(1)->linkAt(2), ") [;{]")) {
            if (!(tok->linkAt(1)->next() && tok->linkAt(1)->next()->isKeyword())) { // e.g. noexcept(true)
                if (possible.count(tok->str()) == 0)
                    possible.insert(tok->str());
                else
                    unknownMacroError(tok);
            }
        } else if (cpp && Token::Match(tok, "public|private|protected %name% :")) {
            unknownMacroError(tok->next());
        }
    }

    // String concatenation with unknown macros
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if ((Token::Match(tok, "%str% %name% (") && Token::Match(tok->linkAt(2), ") %str%")) ||
            (Token::Match(tok, "%str% %name% %str%") && !(startsWith(tok->strAt(1), "PRI") || startsWith(tok->strAt(1), "SCN")))) { // TODO: implement macros in std.cfg
            if (tok->next()->isKeyword())
                continue;
            unknownMacroError(tok->next());
        }
        if (Token::Match(tok, "[(,] %name% (") && Token::Match(tok->linkAt(2), ") %name% %name%|,|)")) {
            if (tok->next()->isKeyword() || tok->linkAt(2)->next()->isKeyword())
                continue;
            if (cAlternativeTokens.count(tok->linkAt(2)->strAt(1)) > 0)
                continue;
            if (startsWith(tok->strAt(1), "__")) // attribute/annotation
                continue;
            if (tok->next()->isStandardType() && !tok->linkAt(2)->next()->isStandardType())
                unknownMacroError(tok->linkAt(2)->next());
            else
                unknownMacroError(tok->next());
        }
    }

    // Report unknown macros without commas or operators inbetween statements: MACRO1() MACRO2()
    for (const Token* tok = tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;
        if (isNonMacro(tok) && !tok->isStandardType())
            continue;

        const Token* endTok = tok->linkAt(1);
        if (!Token::Match(endTok, ") %name% (|."))
            continue;

        const Token* tok2 = endTok->next();
        if (isNonMacro(tok2))
            continue;

        if (tok2->strAt(1) == "(") {
            if (Token::Match(tok->previous(), "%name%|::|>"))
                continue;
        }

        unknownMacroError(tok->isStandardType() ? tok2 : tok);
    }
}

void Tokenizer::findGarbageCode() const
{
    const bool cpp = isCPP();
    const bool isCPP11 = cpp && mSettings.standards.cpp >= Standards::CPP11;

    static const std::unordered_set<std::string> nonConsecutiveKeywords{ "break",
                                                                         "continue",
                                                                         "for",
                                                                         "goto",
                                                                         "if",
                                                                         "return",
                                                                         "switch",
                                                                         "throw",
                                                                         "typedef",
                                                                         "while" };

    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        // initialization: = {
        if (Token::simpleMatch(tok, "= {") && Token::simpleMatch(tok->linkAt(1), "} ("))
            syntaxError(tok->linkAt(1));

        // Inside [] there can't be ; or various keywords
        else if (tok->str() == "[") {
            for (const Token* inner = tok->next(); inner != tok->link(); inner = inner->next()) {
                if (Token::Match(inner, "(|[|{"))
                    inner = inner->link();
                else if (Token::Match(inner, ";|goto|return|typedef"))
                    syntaxError(inner);
            }
        }

        // array assignment
        else if (Token::Match(tok, "%assign% [") && Token::simpleMatch(tok->linkAt(1), "] ;"))
            syntaxError(tok, tok->str() + "[...];");

        else if (Token::Match(tok, "[({<] %assign%"))
            syntaxError(tok);

        else if (Token::Match(tok, "%assign% >"))
            syntaxError(tok);

        else if (Token::Match(tok, "[`\\@]"))
            syntaxError(tok);

        // UNKNOWN_MACRO(return)
        if (tok->isKeyword() && Token::Match(tok, "throw|return )") && Token::Match(tok->linkAt(1)->previous(), "%name% ("))
            unknownMacroError(tok->linkAt(1)->previous());

        // UNKNOWN_MACRO(return)
        else if (!tok->isKeyword() && (Token::Match(tok, "%name% return") || (isCPP() && Token::Match(tok, "%name% throw"))))
            unknownMacroError(tok);

        // Assign/increment/decrement literal
        else if (Token::Match(tok, "!!) %num%|%str%|%char% %assign%|++|--")) {
            if (!cpp || mSettings.standards.cpp < Standards::CPP20 || !Token::Match(tok->previous(), "%name% : %num% ="))
                syntaxError(tok, tok->strAt(1) + " " + tok->strAt(2));
        }
        else if (Token::simpleMatch(tok, ") return") && !Token::Match(tok->link()->previous(), "if|while|for (")) {
            if (tok->link()->previous() && tok->link()->previous()->isUpperCaseName())
                unknownMacroError(tok->link()->previous());
            else
                syntaxError(tok);
        }

        if (tok->isControlFlowKeyword()) {
            if (Token::Match(tok, "if|while|for|switch")) { // if|while|for|switch (EXPR) { ... }
                if (tok->previous() && !Token::Match(tok->previous(), "%name%|:|;|{|}|)")) {
                    if (Token::Match(tok->previous(), "[,(]")) {
                        const Token *prev = tok->previous();
                        while (prev && prev->str() != "(") {
                            if (prev->str() == ")")
                                prev = prev->link();
                            prev = prev->previous();
                        }
                        if (prev && Token::Match(prev->previous(), "%name% ("))
                            unknownMacroError(prev->previous());
                    }
                    if (!Token::simpleMatch(tok->tokAt(-2), "operator \"\" if"))
                        syntaxError(tok);
                }
                if (!Token::Match(tok->next(), "( !!)"))
                    syntaxError(tok);
                if (Token::simpleMatch(tok->linkAt(1), ") }")) {
                    syntaxError(tok->linkAt(1)->next());
                }
                if (tok->str() != "for") {
                    if (isGarbageExpr(tok->next(), tok->linkAt(1), cpp && (mSettings.standards.cpp>=Standards::cppstd_t::CPP17)))
                        syntaxError(tok);
                }
            }
            if (Token::simpleMatch(tok, "do {")) {
                if (!Token::simpleMatch(tok->linkAt(1), "} while ("))
                    syntaxError(tok->linkAt(1));
                if (!Token::simpleMatch(tok->linkAt(1)->linkAt(2), ") ;"))
                    syntaxError(tok->linkAt(1)->linkAt(2));
            }
            // if (x) MACRO() ..
            if (Token::simpleMatch(tok, "if (")) {
                const Token* tok2 = tok->linkAt(1);
                if (Token::Match(tok2, ") %name% (") && tok2->next()->isUpperCaseName() &&
                    Token::Match(tok2->linkAt(2), ") {|else"))
                    syntaxError(tok2->next());
            }
        }

        // keyword keyword
        if (tok->isKeyword() && nonConsecutiveKeywords.count(tok->str()) != 0) {
            if (Token::Match(tok, "%name% %name%") && nonConsecutiveKeywords.count(tok->strAt(1)) == 1)
                syntaxError(tok);
            const Token* prev = tok;
            while (prev && prev->isName())
                prev = prev->previous();
            if (Token::Match(prev, "%op%|%num%|%str%|%char%")) {
                if (!Token::simpleMatch(tok->tokAt(-2), "operator \"\" if") &&
                    !Token::simpleMatch(tok->tokAt(-2), "extern \"C\"") &&
                    !Token::simpleMatch(prev, "> typedef"))
                    syntaxError(tok, prev == tok->previous() ? (prev->str() + " " + tok->str()) : (prev->str() + " .. " + tok->str()));
            }
        }
        else if (tok->isStandardType() && tok->next() && tok->str() == tok->strAt(1) && tok->str() != "long")
            syntaxError(tok);
    }

    // invalid struct declaration
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "struct|class|enum %name%| {") && (!tok->previous() || Token::Match(tok->previous(), "[;{}]"))) {
            const Token *tok2 = tok->linkAt(tok->next()->isName() ? 2 : 1);
            if (Token::Match(tok2, "} %op%")) {
                tok2 = tok2->next();
                if (!Token::Match(tok2, "*|&|&&"))
                    syntaxError(tok2, "Unexpected token '" + tok2->str() + "'");
                while (Token::Match(tok2, "*|&|&&"))
                    tok2 = tok2->next();
                if (!Token::Match(tok2, "%name%"))
                    syntaxError(tok2, "Unexpected token '" + (tok2 ? tok2->str() : "") + "'");
            }
        }
        if (tok->str() == "enum") {
            if (Token::Match(tok->next(), ": %num%| {"))
                syntaxError(tok->tokAt(2), "Unexpected token '" + tok->strAt(2) + "'");
            if (const Token* start = SymbolDatabase::isEnumDefinition(tok)) {
                int nEquals = 0;
                for (const Token* tok2 = start->next(); tok2 && tok2 != start->link(); tok2 = tok2->next()) {
                    if (Token::simpleMatch(tok2, "sizeof (")) {
                        tok2 = tok2->linkAt(1);
                        continue;
                    }
                    if (const Token* lam = findLambdaEndTokenWithoutAST(tok2)) {
                        tok2 = lam;
                        continue;
                    }
                    if (tok2->str() == ";")
                        syntaxError(tok2);
                    if (tok2->str() == "=")
                        ++nEquals;
                    else if (tok2->str() == ",")
                        nEquals = 0;
                    if (nEquals > 1)
                        syntaxError(tok2);
                }
            }
        }
    }

    // Keywords in global scope
    static const std::unordered_set<std::string> nonGlobalKeywords{"break",
                                                                   "continue",
                                                                   "for",
                                                                   "goto",
                                                                   "if",
                                                                   "return",
                                                                   "switch",
                                                                   "while",
                                                                   "try",
                                                                   "catch"};
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            tok = tok->link();
        else if (tok->isKeyword() && nonGlobalKeywords.count(tok->str()) && !Token::Match(tok->tokAt(-2), "operator %str%"))
            syntaxError(tok, "keyword '" + tok->str() + "' is not allowed in global scope");
    }
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (tok->str() == "{" && isClassStructUnionEnumStart(tok)) {
            for (const Token* tok2 = tok->next(); tok2 != tok->link(); tok2 = tok2->next()) {
                if (tok2->str() == "{")
                    tok2 = tok2->link();
                else if (tok2->isKeyword() && nonGlobalKeywords.count(tok2->str()) && !Token::Match(tok2->tokAt(-2), "operator %str%"))
                    syntaxError(tok2, "keyword '" + tok2->str() + "' is not allowed in class/struct/union/enum scope");
            }
        }
    }

    // case keyword must be inside switch
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "switch (")) {
            if (Token::simpleMatch(tok->linkAt(1), ") {")) {
                tok = tok->linkAt(1)->linkAt(1);
                continue;
            }
            const Token *switchToken = tok;
            tok = tok->linkAt(1);
            if (!tok)
                syntaxError(switchToken);
            // Look for the end of the switch statement, i.e. the first semi-colon or '}'
            for (; tok; tok = tok->next()) {
                if (tok->str() == "{") {
                    tok = tok->link();
                }
                if (Token::Match(tok, ";|}")) {
                    // We're at the end of the switch block
                    if (tok->str() == "}" && tok->strAt(-1) == ":") // Invalid case
                        syntaxError(switchToken);
                    break;
                }
            }
            if (!tok)
                break;
        } else if (tok->str() == "(") {
            tok = tok->link();
        } else if (tok->str() == "case") {
            syntaxError(tok);
        }
    }

    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "for (")) // find for loops
            continue;
        // count number of semicolons
        int semicolons = 0, colons = 0;
        const Token* const endTok = tok->linkAt(1);
        for (tok = tok->tokAt(2); tok != endTok; tok = tok->next()) {
            if (const Token* lam = findLambdaEndTokenWithoutAST(tok)) {
                tok = lam;
                continue;
            }
            if (tok->str() == ";") { // do the counting
                semicolons++;
            } else if (tok->str() == ":") {
                if (tok->strAt(-1) == ",")
                    syntaxError(tok);
                colons++;
            } else if (tok->str() == "(") { // skip pairs of ( )
                tok = tok->link();
            }
        }
        // if we have an invalid number of semicolons inside for( ), assume syntax error
        if (semicolons > 2)
            syntaxError(tok);
        if (semicolons == 1 && !(cpp && mSettings.standards.cpp >= Standards::CPP20))
            syntaxError(tok);
        if (semicolons == 0 && colons == 0)
            syntaxError(tok);
    }

    // Operators without operands..
    const Token *templateEndToken = nullptr;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (!templateEndToken) {
            if (tok->str() == "<" && cpp)
                templateEndToken = tok->findClosingBracket();
        } else {
            if (templateEndToken == tok)
                templateEndToken = nullptr;
            if (Token::Match(tok, "> %cop%"))
                continue;
        }
        // skip C++ attributes [[...]]
        if (isCPP11 && (isCPPAttribute(tok) || isAlignAttribute(tok))) {
            tok = skipCPPOrAlignAttribute(tok);
            continue;
        }
        {
            bool match1 = Token::Match(tok, "%or%|%oror%|==|!=|+|-|/|!|>=|<=|~|^|++|--|::|sizeof");
            bool match2 = Token::Match(tok->next(), "{|if|else|while|do|for|return|switch|break");
            if (cpp) {
                match1 = match1 || Token::Match(tok, "throw|decltype|typeof");
                match2 = match2 || Token::Match(tok->next(), "try|catch|namespace");
            }
            if (match1 && !tok->isIncDecOp()) {
                match2 = match2 || Token::Match(tok->next(), "%assign%");
            }
            if (match1 && match2)
                syntaxError(tok);
        }
        if (Token::Match(tok, "%or%|%oror%|~|^|!|%comp%|+|-|/|%")) {
            std::string code;
            if (Token::Match(tok->next(), ")|]|}"))
                code = tok->str() + tok->strAt(1);
            if (Token::simpleMatch(tok->next(), "( )"))
                code = tok->str() + "()";
            if (!code.empty()) {
                if (isC() || (tok->str() != ">" && !Token::simpleMatch(tok->previous(), "operator")))
                    syntaxError(tok, code);
            }
        }
        if (Token::Match(tok, "%num%|%bool%|%char%|%str% %num%|%bool%|%char%|%str%|::") && !Token::Match(tok, "%str% %str%"))
            syntaxError(tok);
        if (Token::Match(tok, "%num%|%bool%|%char%|%str% {|(")) {
            if (tok->strAt(1) == "(")
                syntaxError(tok);
            else if (!(tok->tokType() == Token::Type::eString && Token::simpleMatch(tok->tokAt(-1), "extern")) &&
                     !(tok->tokType() == Token::Type::eBoolean && cpp && Token::simpleMatch(tok->tokAt(-1), "requires")))
                syntaxError(tok);
        }
        if (Token::Match(tok, "( ) %num%|%bool%|%char%|%str%"))
            syntaxError(tok);
        if (Token::Match(tok, "%assign% typename|class %assign%"))
            syntaxError(tok);
        if (Token::Match(tok, "%assign% [;)}]") && (!cpp || !Token::simpleMatch(tok->previous(), "operator")))
            syntaxError(tok);
        if (Token::Match(tok, "; %assign%"))
            syntaxError(tok);
        if (Token::Match(tok, "%assign% %name%") && tok->next()->isControlFlowKeyword())
            syntaxError(tok);
        if (Token::Match(tok, "%cop%|=|,|[ %or%|%oror%|/|%"))
            syntaxError(tok);
        if (Token::Match(tok, "[;([{] %comp%|%oror%|%or%|%|/"))
            syntaxError(tok);
        if (Token::Match(tok, "%cop%|= ]") && !Token::simpleMatch(tok, "*") && !(cpp && Token::Match(tok->previous(), "%type%|[|,|%num% &|=|> ]")))
            syntaxError(tok);
        if (Token::Match(tok, "[+-] [;,)]}]") && !(cpp && Token::simpleMatch(tok->previous(), "operator")))
            syntaxError(tok);
        if (Token::simpleMatch(tok, ",") &&
            !Token::Match(tok->tokAt(-2), "[ = , &|%name%")) {
            if (Token::Match(tok->previous(), "(|[|{|<|%assign%|%or%|%oror%|==|!=|+|-|/|!|>=|<=|~|^|::|sizeof"))
                syntaxError(tok);
            if (cpp && Token::Match(tok->previous(), "throw|decltype|typeof"))
                syntaxError(tok);
            if (Token::Match(tok->next(), ")|]|>|%assign%|%or%|%oror%|==|!=|/|>=|<=|&&"))
                syntaxError(tok);
        }
        if (Token::simpleMatch(tok, ".") &&
            !Token::simpleMatch(tok->previous(), ".") &&
            !Token::simpleMatch(tok->next(), ".") &&
            !Token::Match(tok->previous(), "{|, . %name% =|.|[|{") &&
            !(tok->previous() && tok->previous()->isLiteral()) &&
            !Token::Match(tok->previous(), ", . %name%")) {
            if (!Token::Match(tok->previous(), "%name%|)|]|>|}"))
                syntaxError(tok, tok->strAt(-1) + " " + tok->str() + " " + tok->strAt(1));
            if (!Token::Match(tok->next(), "%name%|*|~"))
                syntaxError(tok, tok->strAt(-1) + " " + tok->str() + " " + tok->strAt(1));
        }
        if (Token::Match(tok, ".|-> .|->"))
            syntaxError(tok);
        if (Token::Match(tok, "[{,] . %name%") && !Token::Match(tok->tokAt(3), "[.=[{]"))
            syntaxError(tok->next());
        if (Token::Match(tok, "%name% %op% %name%") && !tok->isKeyword() && tok->next()->isIncDecOp())
            syntaxError(tok->next());
        if (!tok->isKeyword() && Token::Match(tok, "%name% .|-> %name% %name%") && !tok->tokAt(2)->isKeyword()) {
            if (tok->tokAt(3)->isUpperCaseName())
                unknownMacroError(tok->tokAt(3));
            else
                syntaxError(tok);
        }
        if (Token::Match(tok, "[!|+-/%^~] )|]"))
            syntaxError(tok);
        if (Token::Match(tok, "==|!=|<=|>= %comp%") && tok->strAt(-1) != "operator")
            syntaxError(tok, tok->str() + " " + tok->strAt(1));
        if (Token::simpleMatch(tok, "::") && (!Token::Match(tok->next(), "%name%|*|~") ||
                                              (tok->next()->isKeyword() && !Token::Match(tok->next(), "new|delete|operator"))))
            syntaxError(tok);
        if (Token::Match(tok, "& %comp%|&&|%oror%|&|%or%") && tok->strAt(1) != ">")
            syntaxError(tok);
        if (Token::Match(tok, "%comp%|&&|%oror%|&|%or% }") && tok->str() != ">")
            syntaxError(tok);
        if (Token::Match(tok, "^ %op%") && !Token::Match(tok->next(), "[>*+-!~]"))
            syntaxError(tok);
        if (Token::Match(tok, ": [)]=]"))
            syntaxError(tok);
        if (Token::Match(tok, "typedef [,;:]"))
            syntaxError(tok);
        if (Token::Match(tok, "? %assign%"))
            syntaxError(tok);
        if (Token::Match(tok, "[!~+-/] %comp%") &&
            !(cpp && Token::Match(tok->next(), "[<>]") && Token::simpleMatch(tok->tokAt(-1), "operator")))
            syntaxError(tok);
        if (Token::Match(tok, "%comp% {") && (!cpp || tok->str() != ">"))
            syntaxError(tok);
        if (Token::Match(tok, "] %name%") && (!cpp || !(tok->tokAt(1)->isKeyword() || (tok->tokAt(-1) && Token::simpleMatch(tok->tokAt(-2), "delete ["))))) {
            if (tok->next()->isUpperCaseName())
                unknownMacroError(tok->next());
            else
                syntaxError(tok);
        }

        if (tok->link() && Token::Match(tok, "[([]") && (!tok->tokAt(-1) || !tok->tokAt(-1)->isControlFlowKeyword())) {
            const Token* const end = tok->link();
            for (const Token* inner = tok->next(); inner != end; inner = inner->next()) {
                if (inner->str() == "{")
                    inner = inner->link();
                else if (inner->str() == ";" || (Token::simpleMatch(inner, ", ,") && (!cpp || !Token::simpleMatch(inner->previous(), "operator")))) {
                    if (tok->tokAt(-1) && tok->tokAt(-1)->isUpperCaseName())
                        unknownMacroError(tok->tokAt(-1));
                    else
                        syntaxError(inner);
                }
            }
        }

        if ((!cpp || !Token::simpleMatch(tok->previous(), "operator")) && Token::Match(tok, "[,;] ,"))
            syntaxError(tok);
        if (tok->str() == "typedef") {
            for (const Token* tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->str() == "{") {
                    tok2 = tok2->link();
                    continue;
                }
                if (isUnevaluated(tok2)) {
                    tok2 = tok2->linkAt(1);
                    continue;
                }
                if (!tok2->next() || tok2->isControlFlowKeyword() || Token::Match(tok2, "typedef|static|."))
                    syntaxError(tok);
                if (Token::Match(tok2, "%name% %name%") && tok2->str() == tok2->strAt(1)) {
                    if (Token::simpleMatch(tok2->tokAt(2), ";"))
                        continue;
                    if (tok2->isStandardType() && tok2->str() == "long")
                        continue;
                    if (Token::Match(tok2->tokAt(-1), "enum|struct|union") || (isCPP() && Token::Match(tok2->tokAt(-1), "class|::")))
                        continue;
                    syntaxError(tok2);
                }
            }
        }
        if (cpp && tok->str() == "namespace") {
            if (tok->tokAt(-1) && !Token::Match(tok->tokAt(-1), ";|{|}|using|inline")) {
                if (tok->tokAt(-1)->isUpperCaseName())
                    unknownMacroError(tok->tokAt(-1));
                else if (tok->linkAt(-1) && tok->linkAt(-1)->tokAt(-1) && tok->linkAt(-1)->tokAt(-1)->isUpperCaseName())
                    unknownMacroError(tok->linkAt(-1)->tokAt(-1));
                else
                    syntaxError(tok);
            }
            if (!tok->next() || (Token::Match(tok->next(), "%name% =") && !Token::Match(tok->tokAt(3), "::|%name%")))
                syntaxError(tok);
        }
        if (cpp && tok->str() == "using" && !Token::Match(tok->next(), "::|%name%"))
            syntaxError(tok);
    }

    // ternary operator without :
    if (const Token *ternaryOp = findUnmatchedTernaryOp(tokens(), nullptr))
        syntaxError(ternaryOp);

    // Code must not start with an arithmetical operand
    if (Token::Match(list.front(), "%cop%"))
        syntaxError(list.front());

    // Code must end with } ; ) NAME
    if (!Token::Match(list.back(), "%name%|;|}|)"))
        syntaxError(list.back());
    if (list.back()->str() == ")" && !Token::Match(list.back()->link()->previous(), "%name%|> ("))
        syntaxError(list.back());
    for (const Token *end = list.back(); end && end->isName(); end = end->previous()) {
        if (Token::Match(end, "void|char|short|int|long|float|double|const|volatile|static|inline|struct|class|enum|union|template|sizeof|case|break|continue|typedef"))
            syntaxError(list.back());
    }
    if ((list.back()->str()==")" || list.back()->str()=="}") && list.back()->previous() && list.back()->previous()->isControlFlowKeyword())
        syntaxError(list.back()->previous());

    // Garbage templates..
    if (cpp) {
        for (const Token *tok = tokens(); tok; tok = tok->next()) {
            if (Token::simpleMatch(tok, "< >") && !(Token::Match(tok->tokAt(-1), "%name%") || (tok->tokAt(-1) && Token::Match(tok->tokAt(-2), "operator %op%"))))
                syntaxError(tok);
            if (Token::simpleMatch(tok, ": template") && !Token::Match(tok->tokAt(-1), "public|private|protected"))
                syntaxError(tok);
            if (Token::Match(tok, "> class|struct|union {") && Token::simpleMatch(tok->linkAt(2), "} ;"))
                syntaxError(tok);
            if (!Token::simpleMatch(tok, "template <"))
                continue;
            if (!tok->tokAt(2) || tok->tokAt(2)->isLiteral())
                syntaxError(tok);
            if (tok->previous() && !Token::Match(tok->previous(), ":|,|;|{|}|)|<|>|\"C++\"")) {
                if (tok->previous()->isUpperCaseName())
                    unknownMacroError(tok->previous());
                else
                    syntaxError(tok);
            }
            const Token * const tok1 = tok->next()->findClosingBracket();
            if (!tok1)
                syntaxError(tok);
            if (!Token::Match(tok1, ">|>> ::|...| %name%") &&
                !Token::Match(tok1, ">|>> [ [ %name%") &&
                !Token::Match(tok1, "> >|*"))
                syntaxError(tok1->next() ? tok1->next() : tok);
        }
    }

    // Objective C/C++
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] [ %name% %name% ] ;"))
            syntaxError(tok->next());
    }
}


bool Tokenizer::isGarbageExpr(const Token *start, const Token *end, bool allowSemicolon)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->isControlFlowKeyword())
            return true;
        if (!allowSemicolon && tok->str() == ";")
            return true;
        if (tok->str() == "{")
            tok = tok->link();
    }
    return false;
}

std::string Tokenizer::simplifyString(const std::string &source)
{
    std::string str = source;

    for (std::string::size_type i = 0; i + 1U < str.size(); ++i) {
        if (str[i] != '\\')
            continue;

        int c = 'a';   // char
        int sz = 0;    // size of stringdata
        if (str[i+1] == 'x') {
            sz = 2;
            while (sz < 4 && std::isxdigit(static_cast<unsigned char>(str[i+sz])))
                sz++;
            if (sz > 2) {
                std::istringstream istr(str.substr(i+2, sz-2));
                istr >> std::hex >> c;
            }
        } else if (MathLib::isOctalDigit(str[i+1])) {
            sz = 2;
            while (sz < 4 && MathLib::isOctalDigit(str[i+sz]))
                sz++;
            std::istringstream istr(str.substr(i+1, sz-1));
            istr >> std::oct >> c;
            str = str.replace(i, sz, std::string(1U, static_cast<char>(c)));
            continue;
        }

        if (sz <= 2)
            i++;
        else if (i+sz < str.size())
            str.replace(i, sz, std::string(1U, static_cast<char>(c)));
        else
            str.replace(i, str.size() - i - 1U, "a");
    }

    return str;
}

void Tokenizer::simplifyFunctionTryCatch()
{
    if (!isCPP())
        return;

    for (Token * tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "try {|:"))
            continue;
        const Token* par = tok->previous();
        while (par && par->isKeyword())
            par = par->previous();
        if (!TokenList::isFunctionHead(par, "try")) // TODO: this is supposed to a list of characters and not strings
            continue;

        Token* tryStartToken = skipInitializerList(tok->next());

        if (!Token::simpleMatch(tryStartToken, "{"))
            syntaxError(tryStartToken, "Invalid function-try-catch block code. Did not find '{' for try body.");

        // find the end of the last catch block
        Token * const tryEndToken = tryStartToken->link();
        Token * endToken = tryEndToken;
        while (Token::simpleMatch(endToken, "} catch (")) {
            endToken = endToken->linkAt(2)->next();
            if (!endToken)
                break;
            if (endToken->str() != "{") {
                endToken = nullptr;
                break;
            }
            endToken = endToken->link();
        }
        if (!endToken || endToken == tryEndToken)
            continue;

        tok->previous()->insertToken("{");
        endToken->insertToken("}");
        Token::createMutualLinks(tok->previous(), endToken->next());
    }
}

static bool isAnonymousEnum(const Token* tok)
{
    if (!Token::Match(tok, "enum {|:"))
        return false;
    if (tok->index() > 2 && Token::Match(tok->tokAt(-3), "using %name% ="))
        return false;
    const Token* end = tok->next();
    if (end->str() == ":") {
        end = end->next();
        while (Token::Match(end, "%name%|::"))
            end = end->next();
    }
    return end && Token::Match(end->link(), "} (| %type%| )| [*,;[({=]");
}

void Tokenizer::simplifyStructDecl()
{
    const bool cpp = isCPP();

    // A counter that is used when giving unique names for anonymous structs.
    int count = 0;

    // Add names for anonymous structs
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName())
            continue;
        // check for anonymous struct/union
        if (Token::Match(tok, "struct|union {")) {
            if (Token::Match(tok->linkAt(1), "} const| *|&| const| %type% ,|;|[|(|{|=")) {
                tok->insertToken("Anonymous" + std::to_string(count++));
            }
        }
        // check for derived anonymous class/struct
        else if (cpp && Token::Match(tok, "class|struct :")) {
            const Token *tok1 = Token::findsimplematch(tok, "{");
            if (tok1 && Token::Match(tok1->link(), "} const| *|&| const| %type% ,|;|[|(|{")) {
                tok->insertToken("Anonymous" + std::to_string(count++));
            }
        }
        // check for anonymous enum
        else if (isAnonymousEnum(tok)) {
            Token *start = tok->strAt(1) == ":" ? tok->linkAt(3) : tok->linkAt(1);
            if (start && Token::Match(start->next(), "( %type% )")) {
                start->linkAt(1)->deleteThis();
                start->next()->deleteThis();
            }
            tok->insertToken("Anonymous" + std::to_string(count++));
        }
    }

    // "{" token for current scope
    std::stack<const Token*> scopeStart;
    const Token* functionEnd = nullptr;

    for (Token *tok = list.front(); tok; tok = tok->next()) {

        // check for start of scope and determine if it is in a function
        if (tok->str() == "{") {
            scopeStart.push(tok);
            if (!functionEnd && Token::Match(tok->previous(), "const|)"))
                functionEnd = tok->link();
        }

        // end of scope
        else if (tok->str() == "}") {
            if (!scopeStart.empty())
                scopeStart.pop();
            if (tok == functionEnd)
                functionEnd = nullptr;
        }

        // check for named struct/union
        else if (Token::Match(tok, "class|struct|union|enum %type% :|{")) {
            Token *start = tok;
            while (Token::Match(start->previous(), "%type%"))
                start = start->previous();
            const Token * const type = tok->next();
            Token *next = tok->tokAt(2);

            while (next && !Token::Match(next, "[{;]"))
                next = next->next();
            if (!next || next->str() == ";")
                continue;
            Token* after = next->link();
            if (!after)
                break; // see #4869 segmentation fault in Tokenizer::simplifyStructDecl (invalid code)

            // check for named type
            if (Token::Match(after->next(), "const|static|volatile| *|&| const| (| %type% )| ,|;|[|=|(|{")) {
                after->insertToken(";");
                after = after->next();
                while (!Token::Match(start, "struct|class|union|enum")) {
                    after->insertToken(start->str());
                    after = after->next();
                    after->column(start->column());
                    start->deleteThis();
                }
                tok = start;
                if (!after)
                    break; // see #4869 segmentation fault in Tokenizer::simplifyStructDecl (invalid code)
                after->insertToken(type->str());
                after->next()->column(type->column());
                if (start->str() != "class") {
                    after->insertToken(start->str());
                    after = after->next();
                    after->column(start->column());
                }

                after = after->tokAt(2);

                if (Token::Match(after, "( %type% )")) {
                    after->link()->deleteThis();
                    after->deleteThis();
                }

                // check for initialization
                bool isFuncDecl = Token::Match(after, "%name% (") && Token::simpleMatch(after->linkAt(1), ") {");
                if (Token::Match(after, "%any% (|{") && !isFuncDecl) {
                    after->insertToken("=");
                    after = after->next();
                    const bool isEnum = start->str() == "enum";
                    if (!isEnum && cpp) {
                        after->insertToken(type->str());
                        after = after->next();
                    }

                    if (isEnum) {
                        if (Token::Match(after->next(), "{ !!}")) {
                            after->next()->str("(");
                            after->linkAt(1)->str(")");
                        }
                    }
                }
            }
        }

        // check for anonymous struct/union
        else {
            // unnamed anonymous struct/union so possibly remove it
            bool done = false;
            while (!done && Token::Match(tok, "struct|union {") && Token::simpleMatch(tok->linkAt(1), "} ;")) {
                done = true;

                // is this a class/struct/union scope?
                bool isClassStructUnionScope = false;
                if (!scopeStart.empty()) {
                    for (const Token* tok2 = scopeStart.top()->previous(); tok2 && !Token::Match(tok2, "[;{}]"); tok2 = tok2->previous()) {
                        if (Token::Match(tok2, "class|struct|union")) {
                            isClassStructUnionScope = true;
                            break;
                        }
                    }
                }

                // remove unnamed anonymous struct/union
                // * not in class/struct/union scopes
                if (Token::simpleMatch(tok->linkAt(1), "} ;") && !isClassStructUnionScope && tok->str() != "union") {
                    tok->linkAt(1)->previous()->deleteNext(2);
                    tok->deleteNext();
                    tok->deleteThis();
                    done = false;
                }
            }
        }
    }
}

void Tokenizer::simplifyCallingConvention()
{
    const bool windows = mSettings.platform.isWindows();

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "__cdecl|__stdcall|__fastcall|__thiscall|__clrcall|__syscall|__pascal|__fortran|__far|__near") || (windows && Token::Match(tok, "WINAPI|APIENTRY|CALLBACK"))) {
            tok->deleteThis();
        }
    }
}

static bool isAttribute(const Token* tok, bool gcc) {
    return gcc ? Token::Match(tok, "__attribute__|__attribute (") : Token::Match(tok, "__declspec|_declspec (");
}

static Token* getTokenAfterAttributes(Token* tok, bool gccattr) {
    Token* after = tok;
    while (isAttribute(after, gccattr))
        after = after->linkAt(1)->next();
    return after;
}

static Token* getVariableTokenAfterAttributes(Token* tok) {
    Token *vartok = nullptr;
    Token *after = getTokenAfterAttributes(tok, true);

    // check if after variable name
    if (Token::Match(after, "[;={]")) {
        Token *prev = tok->previous();
        while (Token::simpleMatch(prev, "]"))
            prev = prev->link()->previous();
        if (Token::Match(prev, "%type%"))
            vartok = prev;
    }


    // check if before variable name
    else {
        while (Token::Match(after->next(), "const|volatile|static|*|&|&&|%type%")) {
            after = after->next();
        }
        if (Token::Match(after, "%name%")) {
            vartok = after;
        }
    }

    return vartok;
}

Token* Tokenizer::getAttributeFuncTok(Token* tok, bool gccattr) const {
    if (!Token::Match(tok, "%name% ("))
        return nullptr;
    Token* const after = getTokenAfterAttributes(tok, gccattr);
    if (!after)
        syntaxError(tok);

    if (Token::Match(after, "%name%|*|&|(")) {
        Token *ftok = after;
        while (Token::Match(ftok, "%name%|::|<|*|& !!(")) {
            if (ftok->str() == "<") {
                ftok = ftok->findClosingBracket();
                if (!ftok)
                    break;
            }
            ftok = ftok->next();
        }
        if (Token::simpleMatch(ftok, "( *"))
            ftok = ftok->tokAt(2);
        if (Token::Match(ftok, "%name% (|)"))
            return ftok;
    } else if (Token::Match(after, "[;{=:]")) {
        Token *prev = tok->previous();
        while (Token::Match(prev, "%name%"))
            prev = prev->previous();
        if (Token::simpleMatch(prev, ")")) {
            if (Token::Match(prev->link()->previous(), "%name% ("))
                return prev->link()->previous();
            if (isFunctionPointer(prev->link()->tokAt(-2)))
                return prev->link()->tokAt(-2);
        }
        if (Token::simpleMatch(prev, ")") && Token::Match(prev->link()->tokAt(-2), "operator %op% (") && isCPP())
            return prev->link()->tokAt(-2);
        if ((!prev || Token::Match(prev, "[;{}*]")) && Token::Match(tok->previous(), "%name%"))
            return tok->previous();
    }
    return nullptr;
}

void Tokenizer::simplifyDeclspec()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (isAttribute(tok, false)) {
            if (Token::Match(tok->tokAt(2), "noreturn|nothrow|dllexport")) {
                Token *functok = getAttributeFuncTok(tok, false);
                if (functok) {
                    if (tok->strAt(2) == "noreturn")
                        functok->isAttributeNoreturn(true);
                    else if (tok->strAt(2) == "nothrow")
                        functok->isAttributeNothrow(true);
                    else
                        functok->isAttributeExport(true);
                }
            } else if (tok->strAt(2) == "property")
                tok->linkAt(1)->insertToken("__property");

            Token::eraseTokens(tok, tok->linkAt(1)->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyAttribute()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isKeyword() && Token::Match(tok, "%type% (") && !mSettings.library.isNotLibraryFunction(tok)) {
            if (mSettings.library.isFunctionConst(tok->str(), true))
                tok->isAttributePure(true);
            if (mSettings.library.isFunctionConst(tok->str(), false))
                tok->isAttributeConst(true);
        }
        while (isAttribute(tok, true)) {
            Token *functok = getAttributeFuncTok(tok, true);

            for (Token *attr = tok->tokAt(2); attr->str() != ")"; attr = attr->next()) {
                if (Token::Match(attr, "%name% ("))
                    attr = attr->linkAt(1);

                if (Token::Match(attr, "[(,] constructor|__constructor__ [,()]")) {
                    if (!functok)
                        syntaxError(tok);
                    functok->isAttributeConstructor(true);
                }

                else if (Token::Match(attr, "[(,] destructor|__destructor__ [,()]")) {
                    if (!functok)
                        syntaxError(tok);
                    functok->isAttributeDestructor(true);
                }

                else if (Token::Match(attr, "[(,] unused|__unused__|used|__used__ [,)]")) {
                    Token *vartok = getVariableTokenAfterAttributes(tok);
                    if (!vartok)
                        vartok = functok;
                    if (vartok) {
                        const std::string &attribute(attr->strAt(1));
                        if (attribute.find("unused") != std::string::npos)
                            vartok->isAttributeUnused(true);
                        else
                            vartok->isAttributeUsed(true);
                    }
                }

                else if (Token::Match(attr, "[(,] pure|__pure__|const|__const__|noreturn|__noreturn__|nothrow|__nothrow__|warn_unused_result [,)]")) {
                    if (!functok)
                        syntaxError(tok);

                    const std::string &attribute(attr->strAt(1));
                    if (attribute.find("pure") != std::string::npos)
                        functok->isAttributePure(true);
                    else if (attribute.find("const") != std::string::npos)
                        functok->isAttributeConst(true);
                    else if (attribute.find("noreturn") != std::string::npos)
                        functok->isAttributeNoreturn(true);
                    else if (attribute.find("nothrow") != std::string::npos)
                        functok->isAttributeNothrow(true);
                    else if (attribute.find("warn_unused_result") != std::string::npos)
                        functok->isAttributeNodiscard(true);
                }

                else if (Token::Match(attr, "[(,] packed [,)]") && Token::simpleMatch(tok->previous(), "}"))
                    tok->previous()->isAttributePacked(true);

                else if (functok && Token::simpleMatch(attr, "( __visibility__ ( \"default\" ) )"))
                    functok->isAttributeExport(true);

                else if (Token::Match(attr, "[(,] cleanup ( %name% )")) {
                    Token *vartok = getVariableTokenAfterAttributes(tok);
                    if (vartok) {
                        const std::string& funcname = attr->strAt(3);
                        vartok->addAttributeCleanup(funcname);
                    }
                }
            }

            Token::eraseTokens(tok, tok->linkAt(1)->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyCppcheckAttribute()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "(")
            continue;
        if (!tok->previous())
            continue;
        const std::string &attr = tok->strAt(-1);
        if (!startsWith(attr, "__cppcheck_"))
            continue;
        if (attr.compare(attr.size()-2, 2, "__") != 0) // TODO: ends_with("__")
            continue;

        Token *vartok = tok->link();
        while (Token::Match(vartok->next(), "%name%|*|&|::")) {
            vartok = vartok->next();
            if (Token::Match(vartok, "%name% (") && startsWith(vartok->str(),"__cppcheck_"))
                vartok = vartok->linkAt(1);
        }

        if (vartok->isName()) {
            if (Token::Match(tok->previous(), "__cppcheck_low__ ( %num% )"))
                vartok->setCppcheckAttribute(Token::CppcheckAttributesType::LOW,
                                             MathLib::toBigNumber(tok->tokAt(1)));
            else if (Token::Match(tok->previous(), "__cppcheck_high__ ( %num% )"))
                vartok->setCppcheckAttribute(Token::CppcheckAttributesType::HIGH,
                                             MathLib::toBigNumber(tok->tokAt(1)));
        }

        // Delete cppcheck attribute..
        if (tok->tokAt(-2)) {
            tok = tok->tokAt(-2);
            Token::eraseTokens(tok, tok->linkAt(2)->next());
        } else {
            tok = tok->previous();
            Token::eraseTokens(tok, tok->linkAt(1)->next());
            tok->str(";");
        }
    }
}

void Tokenizer::simplifyCPPAttribute()
{
    // According to cppreference alignas is a c21 feature however the macro is often available when compiling c11
    const bool hasAlignas = ((isCPP() && mSettings.standards.cpp >= Standards::CPP11) || (isC() && mSettings.standards.c >= Standards::C11));
    const bool hasCppAttribute = ((isCPP() && mSettings.standards.cpp >= Standards::CPP11) || (isC() && mSettings.standards.c >= Standards::C23));
    const bool hasMaybeUnused =((isCPP() && mSettings.standards.cpp >= Standards::CPP17) || (isC() && mSettings.standards.c >= Standards::C23));
    const bool hasMaybeUnusedUnderscores = (isC() && mSettings.standards.c >= Standards::C23);

    if (!hasAlignas && !hasCppAttribute)
        return;

    for (Token *tok = list.front(); tok;) {
        if (!isCPPAttribute(tok) && !isAlignAttribute(tok)) {
            tok = tok->next();
            continue;
        }
        if (isCPPAttribute(tok)) {
            if (!hasCppAttribute) {
                tok = skipCPPOrAlignAttribute(tok)->next();
                continue;
            }
            if (Token::findsimplematch(tok->tokAt(2), "noreturn", tok->link())) {
                Token * head = skipCPPOrAlignAttribute(tok)->next();
                while (isCPPAttribute(head) || isAlignAttribute(head))
                    head = skipCPPOrAlignAttribute(head)->next();
                while (Token::Match(head, "%name%|::|*|&|<|>|,")) // skip return type
                    head = head->next();
                if (head && head->str() == "(" && (TokenList::isFunctionHead(head, "{;") || Token::simpleMatch(head->link(), ") __attribute__"))) {
                    head->previous()->isAttributeNoreturn(true);
                }
            } else if (Token::findsimplematch(tok->tokAt(2), "nodiscard", tok->link())) {
                Token * head = skipCPPOrAlignAttribute(tok)->next();
                while (isCPPAttribute(head) || isAlignAttribute(head))
                    head = skipCPPOrAlignAttribute(head)->next();
                while (Token::Match(head, "%name%|::|*|&|<|>|,"))
                    head = head->next();
                if (head && head->str() == "(" && TokenList::isFunctionHead(head, "{;")) {
                    head->previous()->isAttributeNodiscard(true);
                }
            } else if (Token::findsimplematch(tok->tokAt(2), "fallthrough", tok->link()) || Token::findsimplematch(tok->tokAt(2), "__fallthrough__", tok->link())) {
                Token * head = skipCPPOrAlignAttribute(tok)->next();
                while (isCPPAttribute(head) || isAlignAttribute(head))
                    head = skipCPPOrAlignAttribute(head)->next();
                while (head && head->str() == ";") // we have semicollon after the attribute which would be removed in 'removeRedundantSemicolons()' so we skip it
                    head = head->next();
                if (head)
                    head->isAttributeFallthrough(true);
            } else if ((hasMaybeUnusedUnderscores && Token::findsimplematch(tok->tokAt(2), "__maybe_unused__", tok->link()))
                       || (hasMaybeUnused && Token::findsimplematch(tok->tokAt(2), "maybe_unused", tok->link()))) {
                Token* head = skipCPPOrAlignAttribute(tok)->next();
                while (isCPPAttribute(head) || isAlignAttribute(head))
                    head = skipCPPOrAlignAttribute(head)->next();

                if (!head)
                    syntaxError(tok);

                if (Token::simpleMatch(head, ";")) {
                    Token *backTok = tok;
                    while (Token::Match(backTok, "]|[|)")) {
                        if (Token::Match(backTok, "]|)"))
                            backTok = backTok->link();
                        backTok = backTok->previous();
                    }
                    if (Token::Match(backTok, "%name%")) {
                        backTok->isAttributeMaybeUnused(true);
                    }
                } else {
                    while (Token::Match(head->next(), "%name%|::|*|&|&&"))
                        head = head->next();
                    if (Token::Match(head, "%name%") && !Token::Match(head, "auto ["))
                        head->isAttributeMaybeUnused(true);
                    else if (Token::Match(tok->previous(), "%name%") && Token::Match(tok->link(), "] [;={]")) {
                        tok->previous()->isAttributeMaybeUnused(true);
                    } else if (Token::simpleMatch(head->next(), "[")) {
                        head = head->next();
                        const Token *end = head->link();
                        for (head = head->next(); head != end; head = head->next()) {
                            if (Token::Match(head, "%name%")) {
                                head->isAttributeMaybeUnused(true);
                            }
                        }
                    }
                }
            } else if (Token::findsimplematch(tok->tokAt(2), "unused", tok->link())) {
                Token* head = skipCPPOrAlignAttribute(tok)->next();
                while (isCPPAttribute(head) || isAlignAttribute(head))
                    head = skipCPPOrAlignAttribute(head)->next();
                if (!head)
                    syntaxError(tok);
                head->isAttributeUnused(true);
            } else if (Token::Match(tok->previous(), ") [ [ expects|ensures|assert default|audit|axiom| : %name% <|<=|>|>= %num% ] ]")) {
                const Token *vartok = tok->tokAt(4);
                if (vartok->str() == ":")
                    vartok = vartok->next();
                Token *argtok = tok->tokAt(-2);
                while (argtok && argtok->str() != "(") {
                    if (argtok->str() == vartok->str())
                        break;
                    if (argtok->str() == ")")
                        argtok = argtok->link();
                    argtok = argtok->previous();
                }
                if (argtok && argtok->str() == vartok->str()) {
                    if (vartok->strAt(1) == ">=")
                        argtok->setCppcheckAttribute(Token::CppcheckAttributesType::LOW,
                                                     MathLib::toBigNumber(vartok->tokAt(2)));
                    else if (vartok->strAt(1) == ">")
                        argtok->setCppcheckAttribute(Token::CppcheckAttributesType::LOW,
                                                     MathLib::toBigNumber(vartok->tokAt(2)) + 1);
                    else if (vartok->strAt(1) == "<=")
                        argtok->setCppcheckAttribute(Token::CppcheckAttributesType::HIGH,
                                                     MathLib::toBigNumber(vartok->tokAt(2)));
                    else if (vartok->strAt(1) == "<")
                        argtok->setCppcheckAttribute(Token::CppcheckAttributesType::HIGH,
                                                     MathLib::toBigNumber(vartok->tokAt(2)) - 1);
                }
            }
        } else {
            // alignas(expr)

            if (!hasAlignas) {
                tok = skipCPPOrAlignAttribute(tok)->next();
                continue;
            }

            // alignment requirements could be checked here

            Token* atok = nullptr;
            if (Token::Match(tok->previous(), "%name%"))
                atok = tok->previous();
            else if (Token::simpleMatch(tok->previous(), "]")) {
                atok = tok;
                while (atok && Token::simpleMatch(atok->previous(), "]")) {
                    atok = atok->linkAt(-1);
                    atok = atok ? atok->previous() : nullptr;
                }
                if (!Token::Match(atok, "%name%"))
                    atok = nullptr;
            } else {
                atok = tok;
                while (isCPPAttribute(atok) || isAlignAttribute(atok))
                    atok = skipCPPOrAlignAttribute(atok)->next();
                atok = atok ? getVariableTokenAfterAttributes(atok) : atok;
            }
            if (atok) {
                std::string a;
                for (const Token* t = tok->tokAt(2); t && t->str() != ")"; t = t->next())
                    a += " " + t->str();
                if (a.size() > 1)
                    atok->addAttributeAlignas(a.substr(1));
            }
        }
        Token::eraseTokens(tok, skipCPPOrAlignAttribute(tok)->next());
        tok->deleteThis();
    }
}

void Tokenizer::simplifySpaceshipOperator()
{
    if (isCPP() && mSettings.standards.cpp >= Standards::CPP20) {
        for (Token *tok = list.front(); tok && tok->next(); tok = tok->next()) {
            if (Token::simpleMatch(tok, "<= >")) {
                tok->str("<=>");
                tok->deleteNext();
            }
        }
    }
}

static const std::unordered_set<std::string> keywords = {
    "inline"
    , "_inline"
    , "__inline"
    , "__forceinline"
    , "register"
    , "__restrict"
    , "__restrict__"
    , "__thread"
};
// Remove "inline", "register", "restrict", "override", "static" and "constexpr"
// "restrict" keyword
//   - New to 1999 ANSI/ISO C standard
//   - Not in C++ standard yet
void Tokenizer::simplifyKeyword()
{
    // FIXME: There is a risk that "keywords" are removed by mistake. This
    // code should be fixed so it doesn't remove variables etc. Nonstandard
    // keywords should be defined with a library instead. For instance the
    // linux kernel code at least uses "_inline" as struct member name at some
    // places.

    const bool c99 = isC() && mSettings.standards.c >= Standards::C99;
    const bool cpp11 = isCPP() && mSettings.standards.cpp >= Standards::CPP11;
    const bool cpp20 = isCPP() && mSettings.standards.cpp >= Standards::CPP20;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (keywords.find(tok->str()) != keywords.end()) {
            // Don't remove struct members
            if (!Token::simpleMatch(tok->previous(), ".")) {
                const bool isinline = (tok->str().find("inline") != std::string::npos);
                const bool isrestrict = (tok->str().find("restrict") != std::string::npos);
                if (isinline || isrestrict) {
                    for (Token *temp = tok->next(); Token::Match(temp, "%name%"); temp = temp->next()) {
                        if (isinline)
                            temp->isInline(true);
                        if (isrestrict)
                            temp->isRestrict(true);
                    }
                }
                tok->deleteThis(); // Simplify..
            }
        }

        if (isC() || mSettings.standards.cpp == Standards::CPP03) {
            if (tok->str() == "auto")
                tok->deleteThis();
        }

        // simplify static keyword:
        // void foo( int [ static 5 ] ); ==> void foo( int [ 5 ] );
        if (Token::Match(tok, "[ static %num%"))
            tok->deleteNext();

        if (c99) {
            auto getTypeTokens = [tok]() {
                std::vector<Token*> ret;
                for (Token *temp = tok; Token::Match(temp, "%name%"); temp = temp->previous()) {
                    if (!temp->isKeyword())
                        ret.emplace_back(temp);
                }
                for (Token *temp = tok->next(); Token::Match(temp, "%name%"); temp = temp->next()) {
                    if (!temp->isKeyword())
                        ret.emplace_back(temp);
                }
                return ret;
            };

            if (tok->str() == "restrict") {
                for (Token* temp: getTypeTokens())
                    temp->isRestrict(true);
                tok->deleteThis();
            }

            if (mSettings.standards.c >= Standards::C11) {
                while (tok->str() == "_Atomic") {
                    if (Token::simpleMatch(tok->next(), "(")) {
                        tok->linkAt(1)->deleteThis();
                        tok->next()->deleteThis();
                    }
                    for (Token* temp: getTypeTokens())
                        temp->isAtomic(true);
                    tok->deleteThis();
                }
            }
        }

        else if (cpp11) {
            if (cpp20 && tok->str() == "consteval") {
                tok->originalName(tok->str());
                tok->str("constexpr");
            } else if (cpp20 && tok->str() == "constinit") {
                tok->deleteThis();
            }

            // final:
            // 1) struct name final { };   <- struct is final
            if (Token::Match(tok->previous(), "struct|class|union %type%")) {
                Token* finalTok = tok->next();
                if (tok->isUpperCaseName() && Token::Match(finalTok, "%type%") && finalTok->str() != "final") {
                    tok = finalTok;
                    finalTok = finalTok->next();
                }
                if (Token::simpleMatch(finalTok, "<")) { // specialization
                    finalTok = finalTok->findClosingBracket();
                    if (finalTok)
                        finalTok = finalTok->next();
                }
                if (Token::Match(finalTok, "final [:{]")) {
                    finalTok->deleteThis();
                    tok->previous()->isFinalType(true);
                }
            }

            // noexcept -> noexcept(true)
            // 2) void f() noexcept; -> void f() noexcept(true);
            else if (Token::Match(tok, ") const|override|final| noexcept :|{|;|,|const|override|final")) {
                // Insertion is done in inverse order
                // The brackets are linked together accordingly afterwards
                Token* tokNoExcept = tok->next();
                while (tokNoExcept->str() != "noexcept")
                    tokNoExcept = tokNoExcept->next();
                tokNoExcept->insertToken(")");
                Token * braceEnd = tokNoExcept->next();
                tokNoExcept->insertToken("true");
                tokNoExcept->insertToken("(");
                Token * braceStart = tokNoExcept->next();
                tok = tok->tokAt(3);
                Token::createMutualLinks(braceStart, braceEnd);
            }

            // 3) thread_local -> static
            //    on single thread thread_local has the effect of static
            else if (tok->str() == "thread_local") {
                tok->originalName(tok->str());
                tok->str("static");
            }
        }
    }
}

static Token* setTokenDebug(Token* start, TokenDebug td)
{
    if (!start->link())
        return nullptr;
    Token* end = start->link();
    start->deleteThis();
    for (Token* tok = start; tok != end; tok = tok->next()) {
        tok->setTokenDebug(td);
    }
    end->deleteThis();
    return end;
}

void Tokenizer::simplifyDebug()
{
    if (!mSettings.debugnormal && !mSettings.debugwarnings)
        return;
    static const std::unordered_map<std::string, TokenDebug> m = {{"debug_valueflow", TokenDebug::ValueFlow},
        {"debug_valuetype", TokenDebug::ValueType}};
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;
        auto it = m.find(tok->str());
        if (it != m.end()) {
            tok->deleteThis();
            tok = setTokenDebug(tok, it->second);
        }
    }
}

void Tokenizer::simplifyAssignmentBlock()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] %name% = ( {")) {
            const std::string &varname = tok->strAt(1);

            // goto the "} )"
            int indentlevel = 0;
            Token *tok2 = tok;
            while (nullptr != (tok2 = tok2->next())) {
                if (Token::Match(tok2, "(|{"))
                    ++indentlevel;
                else if (Token::Match(tok2, ")|}")) {
                    if (indentlevel <= 2)
                        break;
                    --indentlevel;
                } else if (indentlevel == 2 && tok2->str() == varname && Token::Match(tok2->previous(), "%type%|*"))
                    // declaring variable in inner scope with same name as lhs variable
                    break;
            }
            if (indentlevel == 2 && Token::simpleMatch(tok2, "} )")) {
                tok2 = tok2->tokAt(-3);
                if (Token::Match(tok2, "[;{}] %num%|%name% ;")) {
                    tok2->insertToken("=");
                    tok2->insertToken(tok->strAt(1));
                    tok2->next()->varId(tok->next()->varId());
                    tok->deleteNext(3);
                    tok2->tokAt(5)->deleteNext();
                }
            }
        }
    }
}

// Remove __asm..
void Tokenizer::simplifyAsm()
{
    std::string instruction;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "__asm|_asm|asm {") &&
            tok->linkAt(1)->next()) {
            instruction = tok->tokAt(2)->stringifyList(tok->linkAt(1));
            Token::eraseTokens(tok, tok->linkAt(1)->next());
        }

        else if (Token::Match(tok, "asm|__asm|__asm__ volatile|__volatile|__volatile__|goto|inline| (")) {
            // Goto "("
            Token *partok = tok->next();
            if (partok->str() != "(")
                partok = partok->next();
            instruction = partok->next()->stringifyList(partok->link());
            Token::eraseTokens(tok, partok->link()->next());
        }

        else if (Token::Match(tok, "_asm|__asm")) {
            Token *endasm = tok->next();
            const Token *firstSemiColon = nullptr;
            int comment = 0;
            while (Token::Match(endasm, "%num%|%name%|,|:|;|*|(") || (endasm && (endasm->isLiteral() || endasm->linenr() == comment))) {
                if (Token::Match(endasm, "_asm|__asm|__endasm"))
                    break;
                if (endasm->str() == ";") {
                    comment = endasm->linenr();
                    if (!firstSemiColon)
                        firstSemiColon = endasm;
                }
                if (endasm->str() == "(") {
                    if (!firstSemiColon)
                        endasm = endasm->link();
                    break;
                }
                endasm = endasm->next();
            }
            if (Token::simpleMatch(endasm, "__endasm")) {
                instruction = tok->next()->stringifyList(endasm);
                Token::eraseTokens(tok, endasm->next());
                if (!Token::simpleMatch(tok->next(), ";"))
                    tok->insertToken(";");
            } else if (firstSemiColon) {
                instruction = tok->next()->stringifyList(firstSemiColon);
                Token::eraseTokens(tok, firstSemiColon);
            } else if (Token::Match(endasm, ") { !!}")) {
                tok->deleteThis();
                tok = endasm->tokAt(2);
                endasm = endasm->linkAt(1);
                instruction = tok->stringifyList(endasm);
                Token::eraseTokens(tok, endasm);
            } else if (!endasm) {
                if (!tok->next())
                    syntaxError(tok);
                instruction = tok->next()->stringifyList(endasm);
                Token::eraseTokens(tok, endasm);
                tok->insertToken(";");
            } else
                continue;
        }

        else
            continue;

        if (Token::Match(tok->previous(), ") %name% %name% (")) {
            tok->deleteThis();
            continue;
        }

        // insert "asm ( "instruction" )"
        tok->str("asm");
        if (tok->strAt(1) != ";" && tok->strAt(1) != "{")
            tok->insertToken(";");
        tok->insertToken(")");
        tok->insertToken("\"" + instruction + "\"");
        tok->insertToken("(");

        tok = tok->next();
        Token::createMutualLinks(tok, tok->tokAt(2));

        //move the new tokens in the same line as ";" if available
        tok = tok->tokAt(2);
        if (tok->next() && tok->strAt(1) == ";" &&
            tok->next()->linenr() != tok->linenr()) {
            const int endposition = tok->next()->linenr();
            tok = tok->tokAt(-3);
            for (int i = 0; i < 4; ++i) {
                tok = tok->next();
                tok->linenr(endposition);
            }
        }
    }
}

void Tokenizer::simplifyAsm2()
{
    // Block declarations: ^{}
    // A C extension used to create lambda like closures.

    // Put ^{} statements in asm()
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "^")
            continue;

        if (Token::simpleMatch(tok, "^ {") || (Token::simpleMatch(tok->linkAt(1), ") {") && tok->strAt(-1) != "operator")) {
            Token * start = tok;
            while (start && !Token::Match(start, "[,(;{}=]")) {
                if (start->link() && Token::Match(start, ")|]|>"))
                    start = start->link();
                start = start->previous();
            }

            const Token *last = tok->linkAt(1);
            if (Token::simpleMatch(last, ") {"))
                last = last->linkAt(1);
            last = last->next();
            while (last && !Token::Match(last, "%cop%|,|;|{|}|)")) {
                if (Token::Match(last, "(|["))
                    last = last->link();
                last = last->next();
            }

            if (start && last) {
                std::string asmcode;
                while (start->next() != last) {
                    asmcode += start->strAt(1);
                    start->deleteNext();
                }
                if (last->str() == "}")
                    start->insertToken(";");
                start->insertToken(")");
                start->insertToken("\"" + asmcode + "\"");
                start->insertToken("(");
                start->insertToken("asm");
                start->tokAt(2)->link(start->tokAt(4));
                start->tokAt(4)->link(start->tokAt(2));
                tok = start->tokAt(4);
            }
        }
    }
}

void Tokenizer::simplifyAt()
{
    std::set<std::string> var;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name%|]|) @ %num%|%name%|%str%|(")) {
            const Token *end = tok->tokAt(2);
            if (end->isLiteral())
                end = end->next();
            else if (end->str() == "(") {
                int par = 0;
                while ((end = end->next()) != nullptr) {
                    if (end->str() == "(")
                        par++;
                    else if (end->str() == ")") {
                        if (--par < 0)
                            break;
                    }
                }
                end = end ? end->next() : nullptr;
            } else if (var.find(end->str()) != var.end())
                end = end->next();
            else
                continue;

            if (Token::Match(end, ": %num% ;"))
                end = end->tokAt(2);

            if (Token::Match(end, "[;=]")) {
                if (tok->isName())
                    var.insert(tok->str());
                tok->isAtAddress(true);
                Token::eraseTokens(tok, end);
            }
        }

        // keywords in compiler from cosmic software for STM8
        // TODO: Should use platform configuration.
        if (Token::Match(tok, "@ builtin|eeprom|far|inline|interrupt|near|noprd|nostack|nosvf|packed|stack|svlreg|tiny|vector")) {
            tok->str(tok->strAt(1) + "@");
            tok->deleteNext();
        }
    }
}

// Simplify bitfields
void Tokenizer::simplifyBitfields()
{
    std::size_t anonymousBitfieldCounter = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        Token *last = nullptr;

        if (Token::simpleMatch(tok, "for ("))
            tok = tok->linkAt(1);

        if (!Token::Match(tok, ";|{|}|public:|protected:|private:"))
            continue;

        const bool isEnum = tok->str() == "}" && isEnumStart(tok->link());

        const auto tooLargeError = [this](const Token *tok) {
            const auto max = std::numeric_limits<short>::max();
            reportError(tok,
                        Severity::warning,
                        "tooLargeBitField",
                        "Bit-field size exceeds max number of bits " + std::to_string(max));
        };

        Token* typeTok = tok->next();
        while (Token::Match(typeTok, "const|volatile"))
            typeTok = typeTok->next();
        if (Token::Match(typeTok, ":: %name%"))
            typeTok = typeTok->next();
        while (Token::Match(typeTok, "%name% :: %name%"))
            typeTok = typeTok->tokAt(2);
        if (Token::Match(typeTok, "%type% %name% :") &&
            !Token::Match(tok->next(), "case|public|protected|private|class|struct") &&
            !Token::simpleMatch(tok->tokAt(2), "default :")) {
            Token *tok1 = typeTok->next();
            if (Token::Match(tok1, "%name% : %num% [;=,]"))
                if (!tok1->setBits(MathLib::toBigNumber(tok1->tokAt(2))))
                    tooLargeError(tok1->tokAt(2));
            if (tok1 && tok1->tokAt(2) &&
                (Token::Match(tok1->tokAt(2), "%bool%|%num%") ||
                 !Token::Match(tok1->tokAt(2), "public|protected|private| %type% ::|<|,|{|;"))) {
                while (tok1->next() && !Token::Match(tok1->next(), "[;,)]{}=]")) {
                    if (Token::Match(tok1->next(), "[([]"))
                        Token::eraseTokens(tok1, tok1->linkAt(1));
                    tok1->deleteNext();
                }

                last = tok1->next();
            }
        } else if (isEnum && Token::Match(tok, "} %name%| : %num% ;")) {
            if (tok->strAt(1) == ":") {
                tok->deleteNext(2);
                tok->insertToken("Anonymous");
            } else {
                tok->next()->deleteNext(2);
            }
        } else if (Token::Match(typeTok, "%type% : %num%|%bool% ;") &&
                   typeTok->str() != "default") {
            const std::size_t id = anonymousBitfieldCounter++;
            const std::string name = "anonymous@" + std::to_string(id);
            Token *newTok = typeTok->insertToken(name);
            newTok->isAnonymous(true);
            bool failed;
            if (newTok->tokAt(2)->isBoolean())
                failed = !newTok->setBits(newTok->strAt(2) == "true");
            else
                failed = !newTok->setBits(MathLib::toBigNumber(newTok->tokAt(2)));
            if (failed)
                tooLargeError(newTok->tokAt(2));
            newTok->deleteNext(2);
        }

        if (last && last->str() == ",") {
            Token * tok1 = last;
            tok1->str(";");

            const Token *const tok2 = tok->next();
            tok1->insertToken(tok2->str());
            tok1 = tok1->next();
            tok1->isSigned(tok2->isSigned());
            tok1->isUnsigned(tok2->isUnsigned());
            tok1->isLong(tok2->isLong());
        }
    }
}

static bool isStdContainerOrIterator(const Token* tok, const Settings& settings)
{
    const Library::Container* ctr = settings.library.detectContainerOrIterator(tok, nullptr, /*withoutStd*/ true);
    return ctr && startsWith(ctr->startPattern, "std ::");
}

static bool isStdSmartPointer(const Token* tok, const Settings& settings)
{
    const Library::SmartPointer* ptr = settings.library.detectSmartPointer(tok, /*withoutStd*/ true);
    return ptr && startsWith(ptr->name, "std::");
}

// Add std:: in front of std classes, when using namespace std; was given
void Tokenizer::simplifyNamespaceStd()
{
    if (!isCPP())
        return;

    std::set<std::string> userFunctions;

    for (Token* tok = Token::findsimplematch(list.front(), "using namespace std ;"); tok; tok = tok->next()) {
        bool insert = false;
        if (Token::Match(tok, "enum class|struct| %name%| :|{")) { // Don't replace within enum definitions
            skipEnumBody(tok);
        }
        if (!tok->isName() || tok->isKeyword() || tok->isStandardType() || tok->varId())
            continue;
        if (Token::Match(tok->previous(), ".|::|namespace"))
            continue;
        if (Token::simpleMatch(tok->next(), "(")) {
            if (TokenList::isFunctionHead(tok->next(), "{"))
                userFunctions.insert(tok->str());
            else if (TokenList::isFunctionHead(tok->next(), ";")) {
                const Token *start = tok;
                while (Token::Match(start->previous(), "%type%|*|&"))
                    start = start->previous();
                if (start != tok && start->isName() && !start->isKeyword() && (!start->previous() || Token::Match(start->previous(), "[;{}]")))
                    userFunctions.insert(tok->str());
            }
            if (userFunctions.find(tok->str()) == userFunctions.end() && mSettings.library.matchArguments(tok, "std::" + tok->str()))
                insert = true;
        } else if (Token::simpleMatch(tok->next(), "<") &&
                   (isStdContainerOrIterator(tok, mSettings) || isStdSmartPointer(tok, mSettings)))
            insert = true;
        else if (mSettings.library.hasAnyTypeCheck("std::" + tok->str()) ||
                 mSettings.library.podtype("std::" + tok->str()) ||
                 isStdContainerOrIterator(tok, mSettings))
            insert = true;

        if (insert) {
            tok->previous()->insertToken("std");
            tok->previous()->linenr(tok->linenr()); // For stylistic reasons we put the std:: in the same line as the following token
            tok->previous()->fileIndex(tok->fileIndex());
            tok->previous()->insertToken("::");
        }
    }

    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "using namespace std ;")) {
            Token::eraseTokens(tok, tok->tokAt(4));
            tok->deleteThis();
        }
    }
}


void Tokenizer::simplifyMicrosoftMemoryFunctions()
{
    // skip if not Windows
    if (!mSettings.platform.isWindows())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->strAt(1) != "(")
            continue;

        if (Token::Match(tok, "CopyMemory|RtlCopyMemory|RtlCopyBytes")) {
            tok->str("memcpy");
        } else if (Token::Match(tok, "MoveMemory|RtlMoveMemory")) {
            tok->str("memmove");
        } else if (Token::Match(tok, "FillMemory|RtlFillMemory|RtlFillBytes")) {
            // FillMemory(dst, len, val) -> memset(dst, val, len)
            tok->str("memset");

            Token *tok1 = tok->tokAt(2);
            if (tok1)
                tok1 = tok1->nextArgument(); // Second argument
            if (tok1) {
                Token *tok2 = tok1->nextArgument(); // Third argument

                if (tok2)
                    Token::move(tok1->previous(), tok2->tokAt(-2), tok->linkAt(1)->previous()); // Swap third with second argument
            }
        } else if (Token::Match(tok, "ZeroMemory|RtlZeroMemory|RtlZeroBytes|RtlSecureZeroMemory")) {
            // ZeroMemory(dst, len) -> memset(dst, 0, len)
            tok->str("memset");

            Token *tok1 = tok->tokAt(2);
            if (tok1)
                tok1 = tok1->nextArgument(); // Second argument

            if (tok1) {
                tok1 = tok1->previous();
                tok1->insertToken("0");
                tok1 = tok1->next();
                tok1->insertToken(",");
            }
        } else if (Token::simpleMatch(tok, "RtlCompareMemory")) {
            // RtlCompareMemory(src1, src2, len) -> memcmp(src1, src2, len)
            tok->str("memcmp");
            // For the record, when memcmp returns 0, both strings are equal.
            // When RtlCompareMemory returns len, both strings are equal.
            // It might be needed to improve this replacement by something
            // like ((len - memcmp(src1, src2, len)) % (len + 1)) to
            // respect execution path (if required)
        }
    }
}

namespace {
    struct triplet {
        triplet(const char* m, const char* u) :  mbcs(m), unicode(u) {}
        std::string mbcs, unicode;
    };

    const std::map<std::string, triplet> apis = {
        std::make_pair("_topen", triplet("open", "_wopen")),
        std::make_pair("_tsopen_s", triplet("_sopen_s", "_wsopen_s")),
        std::make_pair("_tfopen", triplet("fopen", "_wfopen")),
        std::make_pair("_tfopen_s", triplet("fopen_s", "_wfopen_s")),
        std::make_pair("_tfreopen", triplet("freopen", "_wfreopen")),
        std::make_pair("_tfreopen_s", triplet("freopen_s", "_wfreopen_s")),
        std::make_pair("_tcscat", triplet("strcat", "wcscat")),
        std::make_pair("_tcschr", triplet("strchr", "wcschr")),
        std::make_pair("_tcscmp", triplet("strcmp", "wcscmp")),
        std::make_pair("_tcsdup", triplet("strdup", "wcsdup")),
        std::make_pair("_tcscpy", triplet("strcpy", "wcscpy")),
        std::make_pair("_tcslen", triplet("strlen", "wcslen")),
        std::make_pair("_tcsncat", triplet("strncat", "wcsncat")),
        std::make_pair("_tcsncpy", triplet("strncpy", "wcsncpy")),
        std::make_pair("_tcsnlen", triplet("strnlen", "wcsnlen")),
        std::make_pair("_tcsrchr", triplet("strrchr", "wcsrchr")),
        std::make_pair("_tcsstr", triplet("strstr", "wcsstr")),
        std::make_pair("_tcstok", triplet("strtok", "wcstok")),
        std::make_pair("_ftprintf", triplet("fprintf", "fwprintf")),
        std::make_pair("_tprintf", triplet("printf", "wprintf")),
        std::make_pair("_stprintf", triplet("sprintf", "swprintf")),
        std::make_pair("_sntprintf", triplet("_snprintf", "_snwprintf")),
        std::make_pair("_ftscanf", triplet("fscanf", "fwscanf")),
        std::make_pair("_tscanf", triplet("scanf", "wscanf")),
        std::make_pair("_stscanf", triplet("sscanf", "swscanf")),
        std::make_pair("_ftprintf_s", triplet("fprintf_s", "fwprintf_s")),
        std::make_pair("_tprintf_s", triplet("printf_s", "wprintf_s")),
        std::make_pair("_stprintf_s", triplet("sprintf_s", "swprintf_s")),
        std::make_pair("_sntprintf_s", triplet("_snprintf_s", "_snwprintf_s")),
        std::make_pair("_ftscanf_s", triplet("fscanf_s", "fwscanf_s")),
        std::make_pair("_tscanf_s", triplet("scanf_s", "wscanf_s")),
        std::make_pair("_stscanf_s", triplet("sscanf_s", "swscanf_s"))
    };
}

void Tokenizer::simplifyMicrosoftStringFunctions()
{
    // skip if not Windows
    if (!mSettings.platform.isWindows())
        return;

    const bool ansi = (mSettings.platform.type == Platform::Type::Win32A); // TODO: check for UNICODE define instead
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->strAt(1) != "(")
            continue;

        const auto match = apis.find(tok->str());
        if (match!=apis.end()) {
            tok->str(ansi ? match->second.mbcs : match->second.unicode);
            tok->originalName(match->first);
        } else if (Token::Match(tok, "_T|_TEXT|TEXT ( %char%|%str% )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext();
            if (!ansi) {
                tok->isLong(true);
                if (tok->str()[0] != 'L')
                    tok->str("L" + tok->str());
            }
            while (Token::Match(tok->next(), "_T|_TEXT|TEXT ( %char%|%str% )")) {
                tok->next()->deleteNext();
                tok->next()->deleteThis();
                tok->next()->deleteNext();
                tok->concatStr(tok->strAt(1));
                tok->deleteNext();
            }
        }
    }
}

// Remove Borland code
void Tokenizer::simplifyBorland()
{
    // skip if not Windows
    if (!mSettings.platform.isWindows())
        return;
    if (isC())
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "( __closure * %name% )")) {
            tok->deleteNext();
        }
    }

    // I think that these classes are always declared at the outer scope
    // I save some time by ignoring inner classes.
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{" && !Token::Match(tok->tokAt(-2), "namespace %type%")) {
            tok = tok->link();
            if (!tok)
                break;
        } else if (Token::Match(tok, "class %name% :|{")) {
            while (tok && tok->str() != "{" && tok->str() != ";")
                tok = tok->next();
            if (!tok)
                break;
            if (tok->str() == ";")
                continue;

            const Token* end = tok->link()->next();
            for (Token *tok2 = tok->next(); tok2 != end; tok2 = tok2->next()) {
                if (tok2->str() == "__property" &&
                    Token::Match(tok2->previous(), ";|{|}|protected:|public:|__published:")) {
                    while (tok2->next() && !Token::Match(tok2->next(), "{|;"))
                        tok2->deleteNext();
                    tok2->deleteThis();
                    if (tok2->str() == "{") {
                        Token::eraseTokens(tok2, tok2->link());
                        tok2->deleteNext();
                        tok2->deleteThis();

                        // insert "; __property ;"
                        tok2->previous()->insertToken(";");
                        tok2->previous()->insertToken("__property");
                        tok2->previous()->insertToken(";");
                    }
                }
            }
        }
    }
}

void Tokenizer::createSymbolDatabase()
{
    if (!mSymbolDatabase)
        mSymbolDatabase = new SymbolDatabase(*this);
    mSymbolDatabase->validate();
}

bool Tokenizer::operatorEnd(const Token * tok)
{
    if (tok && tok->str() == ")") {
        if (TokenList::isFunctionHead(tok, "{;?:["))
            return true;

        tok = tok->next();
        while (tok && !Token::Match(tok, "[=;{}),]")) {
            if (Token::Match(tok, "const|volatile|override")) {
                tok = tok->next();
            } else if (tok->str() == "noexcept") {
                tok = tok->next();
                if (tok && tok->str() == "(") {
                    tok = tok->link()->next();
                }
            } else if (tok->str() == "throw" && tok->next() && tok->strAt(1) == "(") {
                tok = tok->linkAt(1)->next();
            }
            // unknown macros ") MACRO {" and ") MACRO(...) {"
            else if (tok->isUpperCaseName()) {
                tok = tok->next();
                if (tok && tok->str() == "(") {
                    tok = tok->link()->next();
                }
            } else if (Token::Match(tok, "%op% !!(") ||
                       (Token::Match(tok, "%op% (") && !TokenList::isFunctionHead(tok->next(), "{")))
                break;
            else
                return false;
        }

        return true;
    }

    return false;
}

void Tokenizer::simplifyOperatorName()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "using|:: operator %op%|%name% ;")) {
            tok->next()->str("operator" + tok->strAt(2));
            tok->next()->tokType(Token::Type::eKeyword); // we need to preserve the keyword type after setting a non-keyword string
            // TODO: tok->next()->isOperatorKeyword(true);
            tok->next()->deleteNext();
            continue;
        }

        if (tok->str() != "operator")
            continue;
        // operator op
        if (Token::Match(tok, "operator %op% (") && !operatorEnd(tok->linkAt(2))) {
            tok->str(tok->str() + tok->strAt(1));
            tok->tokType(Token::Type::eKeyword); // we need to preserve the keyword type after setting a non-keyword string
            // TODO: tok->isOperatorKeyword(true);
            tok->deleteNext();
            continue;
        }
        std::string op;
        Token *par = tok->next();
        bool done = false;
        while (!done && par) {
            done = true;
            if (par->isName()) {
                op += par->str();
                par = par->next();
                // merge namespaces eg. 'operator std :: string () const {'
                if (Token::Match(par, ":: %name%|%op%|.")) {
                    op += par->str();
                    par = par->next();
                }
                done = false;
            } else if (Token::Match(par, ".|%op%|,")) {
                // check for operator in template
                if (par->str() == "," && !op.empty())
                    break;
                if (!(Token::Match(par, "<|>") && !op.empty())) {
                    op += par->str() == "." ? par->originalName() : par->str();
                    par = par->next();
                    done = false;
                }
            } else if (Token::simpleMatch(par, "[ ]")) {
                op += "[]";
                par = par->tokAt(2);
                done = false;
            } else if (Token::Match(par, "( *| )")) {
                // break out and simplify..
                if (operatorEnd(par->next()))
                    break;

                while (par->str() != ")") {
                    op += par->str();
                    par = par->next();
                }
                op += ")";
                par = par->next();
                if (Token::simpleMatch(par, "...")) {
                    op.clear();
                    par = nullptr;
                    break;
                }
                done = false;
            } else if (Token::Match(par, "\"\" %name% )| (|;|<")) {
                op += "\"\"";
                op += par->strAt(1);
                par = par->tokAt(2);
                if (par->str() == ")") {
                    par->link()->deleteThis();
                    par = par->next();
                    par->deletePrevious();
                    tok = par->tokAt(-3);
                }
                done = true;
            } else if (par->str() == "::") {
                op += par->str();
                par = par->next();
                done = false;
            } else if (par->str() == ";" || par->str() == ")") {
                done = true;
            } else if (par->str() != "(") {
                syntaxError(par, "operator");
            }
        }

        const bool returnsRef = Token::simpleMatch(par, "( & (") && tok->next()->isName();
        if (par && !op.empty()) {
            if (returnsRef) {
                Token* tok_op = par->next()->insertToken("operator" + op);
                // TODO: tok_op->tokType(Token::Type::eKeyword); // the given token is not a keyword but should be treated as such
                tok_op->isOperatorKeyword(true);
                tok->deleteThis();
            }
            else {
                tok->str("operator" + op);
                tok->tokType(Token::Type::eKeyword); // we need to preserve the keyword type after setting a non-keyword string
                // TODO: tok->isOperatorKeyword(true);
                Token::eraseTokens(tok, par);
            }
        }

        if (!op.empty() && !returnsRef)
            tok->isOperatorKeyword(true);
    }

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%op% %str% %name%")) {
            const std::string name = tok->strAt(2);
            Token * const str = tok->next();
            str->deleteNext();
            tok->insertToken("operator\"\"" + name);
            tok = tok->next();
            tok->isOperatorKeyword(true);
            tok->insertToken("(");
            str->insertToken(")");
            Token::createMutualLinks(tok->next(), str->next());
            str->insertToken(std::to_string(Token::getStrLength(str)));
            str->insertToken(",");
        }
    }

    if (mSettings.debugwarnings) {
        const Token *tok = list.front();

        while ((tok = Token::findsimplematch(tok, "operator")) != nullptr) {
            reportError(tok, Severity::debug, "debug",
                        "simplifyOperatorName: found unsimplified operator name");
            tok = tok->next();
        }
    }
}

void Tokenizer::simplifyOverloadedOperators()
{
    if (isC())
        return;
    std::set<std::string> classNames;
    std::set<nonneg int> classVars;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName())
            continue;

        if (Token::simpleMatch(tok, "this ) (") && Token::simpleMatch(tok->tokAt(-2), "( *")) {
            tok = tok->next();
            tok->insertToken("operator()");
            tok->insertToken(".");
            continue;
        }

        // Get classes that have operator() member
        if (Token::Match(tok, "class|struct %name% [:{]")) {
            int indent = 0;
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "}")
                    break;
                if (indent == 0 && tok2->str() == ";")
                    break;
                if (tok2->str() == "{") {
                    if (indent == 0)
                        ++indent;
                    else
                        tok2 = tok2->link();
                } else if (indent == 1 && Token::simpleMatch(tok2, "operator() (") && TokenList::isFunctionHead(tok2->next(), ";{")) {
                    classNames.insert(tok->strAt(1));
                    break;
                }
            }
        }

        // Get variables that have operator() member
        if (Token::Match(tok, "%type% &| %var%") && classNames.find(tok->str()) != classNames.end()) {
            tok = tok->next();
            while (!tok->isName())
                tok = tok->next();
            classVars.insert(tok->varId());
        }

        // Simplify operator() calls
        if (Token::Match(tok, "%var% (") && classVars.find(tok->varId()) != classVars.end()) {
            // constructor init list..
            if (Token::Match(tok->previous(), "[:,]")) {
                const Token *start = tok->previous();
                while (Token::simpleMatch(start, ",")) {
                    if (Token::simpleMatch(start->previous(), ")"))
                        start = start->linkAt(-1);
                    else
                        break;
                    if (Token::Match(start->previous(), "%name%"))
                        start = start->tokAt(-2);
                    else
                        break;
                }
                const Token *after = tok->linkAt(1);
                while (Token::Match(after, ")|} , %name% (|{"))
                    after = after->linkAt(3);

                // Do not simplify initlist
                if (Token::simpleMatch(start, ":") && Token::simpleMatch(after, ") {"))
                    continue;
            }

            tok->insertToken("operator()");
            tok->insertToken(".");
        }
    }
}

// remove unnecessary member qualification..
void Tokenizer::removeUnnecessaryQualification()
{
    if (isC())
        return;

    std::vector<Space> classInfo;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "class|struct|namespace %type% :|{") &&
            (!tok->previous() || tok->strAt(-1) != "enum")) {
            Space info;
            info.isNamespace = tok->str() == "namespace";
            tok = tok->next();
            info.className = tok->str();
            tok = tok->next();
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (!tok)
                return;
            info.bodyEnd = tok->link();
            classInfo.push_back(std::move(info));
        } else if (!classInfo.empty()) {
            if (tok == classInfo.back().bodyEnd)
                classInfo.pop_back();
            else if (tok->str() == classInfo.back().className &&
                     !classInfo.back().isNamespace && tok->strAt(-1) != ":" &&
                     (Token::Match(tok, "%type% :: ~| %type% (") ||
                      Token::Match(tok, "%type% :: operator"))) {
                const Token *tok1 = tok->tokAt(3);
                if (tok->strAt(2) == "operator") {
                    // check for operator ()
                    if (tok1->str() == "(")
                        tok1 = tok1->next();

                    while (tok1 && tok1->str() != "(") {
                        if (tok1->str() == ";")
                            break;
                        tok1 = tok1->next();
                    }
                    if (!tok1 || tok1->str() != "(")
                        continue;
                } else if (tok->strAt(2) == "~")
                    tok1 = tok1->next();

                if (!tok1 || !Token::Match(tok1->link(), ") const| {|;|:")) {
                    continue;
                }

                const bool isConstructorOrDestructor =
                    Token::Match(tok, "%type% :: ~| %type%") && (tok->strAt(2) == tok->str() || (tok->strAt(2) == "~" && tok->strAt(3) == tok->str()));
                if (!isConstructorOrDestructor) {
                    bool isPrependedByType = Token::Match(tok->previous(), "%type%");
                    if (!isPrependedByType) {
                        const Token* tok2 = tok->tokAt(-2);
                        isPrependedByType = Token::Match(tok2, "%type% *|&");
                    }
                    if (!isPrependedByType) {
                        const Token* tok3 = tok->tokAt(-3);
                        isPrependedByType = Token::Match(tok3, "%type% * *|&");
                    }
                    if (!isPrependedByType) {
                        // It's not a constructor declaration and it's not a function declaration so
                        // this is a function call which can have all the qualifiers just fine - skip.
                        continue;
                    }
                }
            }
        }
    }
}

void Tokenizer::prepareTernaryOpForAST()
{
    // http://en.cppreference.com/w/cpp/language/operator_precedence says about ternary operator:
    //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
    // The AST parser relies on this function to add such parentheses where necessary.
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "?") {
            bool parenthesesNeeded = false;
            int depth = 0;
            Token* tok2 = tok->next();
            for (; tok2; tok2 = tok2->next()) {
                if (tok2->link() && Token::Match(tok2, "[|(|<"))
                    tok2 = tok2->link();
                else if (tok2->str() == ":") {
                    if (depth == 0)
                        break;
                    depth--;
                } else if (tok2->str() == ";" || (tok2->link() && tok2->str() != "{" && tok2->str() != "}"))
                    break;
                else if (tok2->str() == ",")
                    parenthesesNeeded = true;
                else if (tok2->str() == "<")
                    parenthesesNeeded = true;
                else if (tok2->str() == "?") {
                    depth++;
                    parenthesesNeeded = true;
                }
            }
            if (parenthesesNeeded && tok2 && tok2->str() == ":") {
                tok->insertToken("(");
                tok2->insertTokenBefore(")");
                Token::createMutualLinks(tok->next(), tok2->previous());
            }
        }
    }
}

void Tokenizer::reportError(const Token* tok, const Severity severity, const std::string& id, const std::string& msg, bool inconclusive) const
{
    const std::list<const Token*> callstack(1, tok);
    reportError(callstack, severity, id, msg, inconclusive);
}

void Tokenizer::reportError(const std::list<const Token*>& callstack, Severity severity, const std::string& id, const std::string& msg, bool inconclusive) const
{
    const ErrorMessage errmsg(callstack, &list, severity, id, msg, inconclusive ? Certainty::inconclusive : Certainty::normal);
    mErrorLogger.reportErr(errmsg);
}

void Tokenizer::setPodTypes()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || tok->varId())
            continue;

        // pod type
        const Library::PodType *podType = mSettings.library.podtype(tok->str());
        if (podType) {
            const Token *prev = tok->previous();
            while (prev && prev->isName())
                prev = prev->previous();
            if (prev && !Token::Match(prev, ";|{|}|,|("))
                continue;
            tok->isStandardType(true);
        }
    }
}

const Token *Tokenizer::findSQLBlockEnd(const Token *tokSQLStart)
{
    const Token *tokLastEnd = nullptr;
    for (const Token *tok = tokSQLStart->tokAt(2); tok != nullptr; tok = tok->next()) {
        if (tokLastEnd == nullptr && tok->str() == ";")
            tokLastEnd = tok;
        else if (tok->str() == "__CPPCHECK_EMBEDDED_SQL_EXEC__") {
            if (Token::simpleMatch(tok->tokAt(-2), "END - __CPPCHECK_EMBEDDED_SQL_EXEC__ ;"))
                return tok->next();
            return tokLastEnd;
        } else if (Token::Match(tok, "{|}|==|&&|!|^|<<|>>|++|+=|-=|/=|*=|>>=|<<=|~"))
            break; // We are obviously outside the SQL block
    }

    return tokLastEnd;
}

void Tokenizer::simplifyNestedNamespace()
{
    if (!isCPP())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "namespace %name% ::") && tok->strAt(-1) != "using") {
            Token * tok2 = tok->tokAt(2);

            // validate syntax
            while (Token::Match(tok2, ":: %name%"))
                tok2 = tok2->tokAt(2);

            if (!tok2 || tok2->str() != "{")
                return; // syntax error

            std::stack<Token *> links;
            tok2 = tok->tokAt(2);

            while (tok2->str() == "::") {
                links.push(tok2);
                tok2->str("{");
                tok2->insertToken("namespace");
                tok2 = tok2->tokAt(3);
            }

            tok = tok2;

            if (!links.empty() && tok2->str() == "{") {
                tok2 = tok2->link();
                while (!links.empty()) {
                    tok2->insertToken("}");
                    tok2 = tok2->next();
                    Token::createMutualLinks(links.top(), tok2);
                    links.pop();
                }
            }
        }
    }
}

void Tokenizer::simplifyCoroutines()
{
    if (!isCPP() || mSettings.standards.cpp < Standards::CPP20)
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || !Token::Match(tok, "co_return|co_yield|co_await"))
            continue;
        Token *end = tok->next();
        while (end && end->str() != ";") {
            if (Token::Match(end, "[({[]"))
                end = end->link();
            else if (Token::Match(end, "[)]}]"))
                break;
            end = end->next();
        }
        if (Token::simpleMatch(end, ";")) {
            tok->insertToken("(");
            end->previous()->insertToken(")");
            Token::createMutualLinks(tok->next(), end->previous());
        }
    }
}

static bool sameTokens(const Token *first, const Token *last, const Token *other)
{
    while (other && first->str() == other->str()) {
        if (first == last)
            return true;
        first = first->next();
        other = other->next();
    }

    return false;
}

static bool alreadyHasNamespace(const Token *first, const Token *last, const Token *end)
{
    while (end && last->str() == end->str()) {
        if (first == last)
            return true;
        last = last->previous();
        end = end->previous();
    }

    return false;
}

static Token * deleteAlias(Token * tok)
{
    Token::eraseTokens(tok, Token::findsimplematch(tok, ";"));

    // delete first token
    tok->deleteThis();

    // delete ';' if not last token
    tok->deleteThis();

    return tok;
}

void Tokenizer::simplifyNamespaceAliases()
{
    if (!isCPP())
        return;

    int scope = 0;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        bool isPrev{};
        if (tok->str() == "{")
            scope++;
        else if (tok->str() == "}")
            scope--;
        else if (Token::Match(tok, "namespace %name% =") || (isPrev = Token::Match(tok->previous(), "namespace %name% ="))) {
            if (isPrev)
                tok = tok->previous();
            if (tok->tokAt(-1) && !Token::Match(tok->tokAt(-1), "[;{}]"))
                syntaxError(tok->tokAt(-1));
            const std::string name(tok->strAt(1));
            Token * tokNameStart = tok->tokAt(3);
            Token * tokNameEnd = tokNameStart;

            while (tokNameEnd && tokNameEnd->next() && tokNameEnd->strAt(1) != ";") {
                if (tokNameEnd->str() == "(") {
                    if (tokNameEnd->previous()->isName())
                        unknownMacroError(tokNameEnd->previous());
                    else
                        syntaxError(tokNameEnd);
                }
                tokNameEnd = tokNameEnd->next();
            }

            if (!tokNameEnd)
                return; // syntax error

            int endScope = scope;
            Token * tokLast = tokNameEnd->next();
            if (!tokLast)
                return;
            Token * tokNext = tokLast->next();
            Token * tok2 = tokNext;

            while (tok2 && endScope >= scope) {
                if (Token::simpleMatch(tok2, "{"))
                    endScope++;
                else if (Token::simpleMatch(tok2, "}"))
                    endScope--;
                else if (tok2->str() == name) {
                    if (Token::Match(tok2->previous(), "namespace %name% =")) {
                        // check for possible duplicate aliases
                        if (sameTokens(tokNameStart, tokNameEnd, tok2->tokAt(2))) {
                            // delete duplicate
                            tok2 = deleteAlias(tok2->previous());
                            continue;
                        }
                        // conflicting declaration (syntax error)
                        // cppcheck-suppress duplicateBranch - remove when TODO below is addressed
                        if (endScope == scope) {
                            // delete conflicting declaration
                            tok2 = deleteAlias(tok2->previous());
                        }

                        // new declaration
                        else {
                            // TODO: use the new alias in this scope
                            tok2 = deleteAlias(tok2->previous());
                        }
                        continue;
                    }

                    if (tok2->strAt(1) == "::" && !alreadyHasNamespace(tokNameStart, tokNameEnd, tok2)) {
                        if (Token::simpleMatch(tok2->tokAt(-1), "::") && tokNameStart->str() == "::")
                            tok2->deletePrevious();
                        tok2->str(tokNameStart->str());
                        Token * tok3 = tokNameStart;
                        while (tok3 != tokNameEnd) {
                            tok2->insertToken(tok3->strAt(1));
                            tok2 = tok2->next();
                            tok3 = tok3->next();
                        }
                    }
                }
                tok2 = tok2->next();
            }

            if (tok->previous() && tokNext) {
                Token::eraseTokens(tok->previous(), tokNext);
                tok = tokNext->previous();
            } else if (tok->previous()) {
                Token::eraseTokens(tok->previous(), tokLast);
                tok = tokLast;
            } else if (tokNext) {
                Token::eraseTokens(tok, tokNext);
                tok->deleteThis();
            } else {
                Token::eraseTokens(tok, tokLast);
                tok->deleteThis();
            }
        }
    }
}

void Tokenizer::setDirectives(std::list<Directive> directives)
{
    mDirectives = std::move(directives);
}

bool Tokenizer::hasIfdef(const Token *start, const Token *end) const
{
    const auto& directives = mDirectives;
    return std::any_of(directives.cbegin(), directives.cend(), [&](const Directive& d) {
        return startsWith(d.str, "#if") &&
               d.linenr >= start->linenr() &&
               d.linenr <= end->linenr() &&
               start->fileIndex() < list.getFiles().size() &&
               d.file == list.getFiles()[start->fileIndex()];
    });
}

bool Tokenizer::isPacked(const Token * bodyStart) const
{
    const auto& directives = mDirectives;
    // TODO: should this return true if the #pragma exists in any line before the start token?
    return std::any_of(directives.cbegin(), directives.cend(), [&](const Directive& d) {
        return d.linenr < bodyStart->linenr() && d.str == "#pragma pack(1)" && d.file == list.getFiles().front();
    });
}

void Tokenizer::getErrorMessages(ErrorLogger& errorLogger, const Settings& settings)
{
    TokenList tokenlist{settings, Standards::Language::C};
    Tokenizer tokenizer(std::move(tokenlist), errorLogger);
    tokenizer.invalidConstFunctionTypeError(nullptr);
    // checkLibraryNoReturn
    tokenizer.unhandled_macro_class_x_y(nullptr, "", "", "", "");
    tokenizer.unhandledCharLiteral(nullptr, "");
}
