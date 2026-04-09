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

#include "token.h"

#include "astutils.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenlist.h"
#include "utils.h"
#include "tokenrange.h"
#include "valueflow.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <simplecpp.h>

namespace {
    struct less {
        template<class T, class U>
        bool operator()(const T &x, const U &y) const {
            return x < y;
        }
    };
}

const std::list<ValueFlow::Value> Token::mEmptyValueList;
const std::string Token::mEmptyString;

Token::Token(const TokenList& tokenlist, std::shared_ptr<TokensFrontBack> tokensFrontBack)
    : mList(tokenlist)
    , mTokensFrontBack(std::move(tokensFrontBack))
    , mImpl(new Impl)
    , mIsC(mList.isC())
    , mIsCpp(mList.isCPP())
{}

Token::Token(const Token* tok)
    : Token(tok->mList, const_cast<Token*>(tok)->mTokensFrontBack)
{
    fileIndex(tok->fileIndex());
    linenr(tok->linenr());
}

Token::~Token()
{
    delete mImpl;
}

/*
 * Get a TokenRange which starts at this token and contains every token following it in order up to but not including 't'
 * e.g. for the sequence of tokens A B C D E, C.until(E) would yield the Range C D
 * note t can be nullptr to iterate all the way to the end.
 */
// cppcheck-suppress unusedFunction // only used in testtokenrange.cpp
ConstTokenRange Token::until(const Token* t) const
{
    return ConstTokenRange(this, t);
}

static const std::unordered_set<std::string> controlFlowKeywords = {
    "goto",
    "do",
    "if",
    "else",
    "for",
    "while",
    "switch",
    "case",
    "break",
    "continue",
    "return"
};

void Token::update_property_info()
{
    assert(mImpl);

    setFlag(fIsControlFlowKeyword, false);
    // TODO: clear fIsLong
    isStandardType(false);

    if (!mStr.empty()) {
        if (mStr == "true" || mStr == "false") {
            if (mImpl->mVarId) {
                if (mIsCpp)
                    throw InternalError(this, "Internal error. VarId set for bool literal.");
                tokType(eVariable);
            }
            else
                tokType(eBoolean);
        }
        else if (isStringLiteral(mStr)) {
            tokType(eString);
            isLong(isPrefixStringCharLiteral(mStr, '"', "L"));
        }
        else if (isCharLiteral(mStr)) {
            tokType(eChar);
            isLong(isPrefixStringCharLiteral(mStr, '\'', "L"));
        }
        else if (std::isalpha(static_cast<unsigned char>(mStr[0])) || mStr[0] == '_' || mStr[0] == '$') { // Name
            if (mImpl->mVarId)
                tokType(eVariable);
            else if (mList.isKeyword(mStr)) {
                tokType(eKeyword);
                update_property_isStandardType();
                if (mTokType != eType) // cannot be a control-flow keyword when it is a type
                    setFlag(fIsControlFlowKeyword, controlFlowKeywords.find(mStr) != controlFlowKeywords.end());
            }
            else if (mStr == "asm") { // TODO: not a keyword
                tokType(eKeyword);
            }
            else {
                tokType(eName);
                // some types are not being treated as keywords
                update_property_isStandardType();
            }
        } else if (simplecpp::Token::isNumberLike(mStr)) {
            if ((MathLib::isInt(mStr) || MathLib::isFloat(mStr)) && mStr.find('_') == std::string::npos)
                tokType(eNumber);
            else
                tokType(eLiteral); // assume it is a user defined literal
        } else if (mStr == "=" || mStr == "<<=" || mStr == ">>=" ||
                   (mStr.size() == 2U && mStr[1] == '=' && std::strchr("+-*/%&^|", mStr[0])))
            tokType(eAssignmentOp);
        else if (mStr.size() == 1 && mStr.find_first_of(",[]()?:") != std::string::npos)
            tokType(eExtendedOp);
        else if (mStr=="<<" || mStr==">>" || (mStr.size()==1 && mStr.find_first_of("+-*/%") != std::string::npos))
            tokType(eArithmeticalOp);
        else if (mStr.size() == 1 && mStr.find_first_of("&|^~") != std::string::npos)
            tokType(eBitOp);
        else if (mStr.size() <= 2 &&
                 (mStr == "&&" ||
                  mStr == "||" ||
                  mStr == "!"))
            tokType(eLogicalOp);
        // TODO: should link check only apply to < and >? Token::link() suggests so
        else if (mStr.size() <= 2 && !mLink &&
                 (mStr == "==" ||
                  mStr == "!=" ||
                  mStr == "<" ||
                  mStr == "<=" ||
                  mStr == ">" ||
                  mStr == ">="))
            tokType(eComparisonOp);
        else if (mStr == "<=>")
            tokType(eComparisonOp);
        else if (mStr.size() == 2 &&
                 (mStr == "++" ||
                  mStr == "--"))
            tokType(eIncDecOp);
        else if (mStr.size() == 1 && (mStr.find_first_of("{}") != std::string::npos || (mLink && mStr.find_first_of("<>") != std::string::npos)))
            tokType(eBracket);
        else if (mStr == "...")
            tokType(eEllipsis);
        else
            tokType(eOther);
    } else {
        tokType(eNone);
    }
    assert(!mImpl->mVarId || mTokType == eVariable);
    // TODO: validate type for linked token?
}

static const std::unordered_set<std::string> stdTypes = { "bool"
                                                          , "_Bool"
                                                          , "char"
                                                          , "double"
                                                          , "float"
                                                          , "int"
                                                          , "long"
                                                          , "short"
                                                          , "size_t"
                                                          , "void"
                                                          , "wchar_t"
                                                          , "signed"
                                                          , "unsigned"
};

bool Token::isStandardType(const std::string& str)
{
    return stdTypes.find(str) != stdTypes.end();
}

void Token::update_property_isStandardType()
{
    if (mStr.size() < 3 || mStr.size() > 7)
        return;

    if (isStandardType(mStr)) {
        isStandardType(true);
        tokType(eType);
    }
}

bool Token::isUpperCaseName() const
{
    if (!isName())
        return false;
    return std::none_of(mStr.begin(), mStr.end(), [](char c) {
        return std::islower(c);
    });
}

void Token::concatStr(std::string const& b)
{
    mStr.pop_back();
    mStr.append(getStringLiteral(b) + "\"");

    if (isCChar() && isStringLiteral(b) && b[0] != '"') {
        mStr.insert(0, b.substr(0, b.find('"')));
    }
    update_property_info();
}

std::string Token::strValue() const
{
    assert(mTokType == eString);
    std::string ret(getStringLiteral(mStr));
    std::string::size_type pos = 0U;
    while ((pos = ret.find('\\', pos)) != std::string::npos) {
        ret.erase(pos,1U);
        if (ret[pos] >= 'a') {
            if (ret[pos] == 'n')
                ret[pos] = '\n';
            else if (ret[pos] == 'r')
                ret[pos] = '\r';
            else if (ret[pos] == 't')
                ret[pos] = '\t';
        }
        if (ret[pos] == '0')
            return ret.substr(0,pos);
        pos++;
    }
    return ret;
}

void Token::deleteNext(nonneg int count)
{
    while (mNext && count > 0) {
        Token *n = mNext;

        // #8154 we are about to be unknown -> destroy the link to us
        if (n->mLink && n->mLink->mLink == n)
            n->mLink->link(nullptr);

        mNext = n->next();
        delete n;
        --count;
    }

    if (mNext)
        mNext->previous(this);
    else
        mTokensFrontBack->back = this;
}

void Token::deletePrevious(nonneg int count)
{
    while (mPrevious && count > 0) {
        Token *p = mPrevious;

        // #8154 we are about to be unknown -> destroy the link to us
        if (p->mLink && p->mLink->mLink == p)
            p->mLink->link(nullptr);

        mPrevious = p->previous();
        delete p;
        --count;
    }

    if (mPrevious)
        mPrevious->next(this);
    else
        mTokensFrontBack->front = this;
}

void Token::swapWithNext()
{
    if (mNext) {
        std::swap(mStr, mNext->mStr);
        std::swap(mTokType, mNext->mTokType);
        std::swap(mFlags, mNext->mFlags);
        std::swap(mImpl, mNext->mImpl);
        if (mImpl->mTemplateSimplifierPointers)
            // cppcheck-suppress shadowFunction - TODO: fix this
            for (auto *templateSimplifierPointer : *mImpl->mTemplateSimplifierPointers) {
                templateSimplifierPointer->token(this);
            }

        if (mNext->mImpl->mTemplateSimplifierPointers)
            // cppcheck-suppress shadowFunction - TODO: fix this
            for (auto *templateSimplifierPointer : *mNext->mImpl->mTemplateSimplifierPointers) {
                templateSimplifierPointer->token(mNext);
            }
        if (mNext->mLink)
            mNext->mLink->mLink = this;
        if (this->mLink)
            this->mLink->mLink = mNext;
        std::swap(mLink, mNext->mLink);
    }
}

void Token::takeData(Token *fromToken)
{
    mStr = fromToken->mStr;
    tokType(fromToken->mTokType);
    mFlags = fromToken->mFlags;
    delete mImpl;
    mImpl = fromToken->mImpl;
    fromToken->mImpl = nullptr;
    if (mImpl->mTemplateSimplifierPointers)
        // cppcheck-suppress shadowFunction - TODO: fix this
        for (auto *templateSimplifierPointer : *mImpl->mTemplateSimplifierPointers) {
            templateSimplifierPointer->token(this);
        }
    mLink = fromToken->mLink;
    if (mLink)
        mLink->link(this);
}

void Token::deleteThis()
{
    if (mNext) { // Copy next to this and delete next
        takeData(mNext);
        mNext->link(nullptr); // mark as unlinked
        deleteNext();
    } else if (mPrevious) { // Copy previous to this and delete previous
        takeData(mPrevious);
        mPrevious->link(nullptr);
        deletePrevious();
    } else {
        // We are the last token in the list, we can't delete
        // ourselves, so just make us empty
        str(";");
    }
}

void Token::replace(Token *replaceThis, Token *start, Token *end)
{
    // Fix the whole in the old location of start and end
    if (start->previous())
        start->previous()->next(end->next());

    if (end->next())
        end->next()->previous(start->previous());

    // Move start and end to their new location
    if (replaceThis->previous())
        replaceThis->previous()->next(start);

    if (replaceThis->next())
        replaceThis->next()->previous(end);

    start->previous(replaceThis->previous());
    end->next(replaceThis->next());

    if (end->mTokensFrontBack->back == end) {
        while (end->next())
            end = end->next();
        end->mTokensFrontBack->back = end;
    }

    // Update mProgressValue, fileIndex and linenr
    for (Token *tok = start; tok != end->next(); tok = tok->next())
        tok->mImpl->mProgressValue = replaceThis->mImpl->mProgressValue;

    // Delete old token, which is replaced
    delete replaceThis;
}

/**
 * @throws InternalError thrown on unexpected command or missing varid with %varid%
 */
static
#if defined(__GNUC__)
// GCC does not inline this by itself
// need to use the old syntax since the C++11 [[xxx:always_inline]] cannot be used here
inline __attribute__((always_inline))
#endif
int multiComparePercent(const Token *tok, const char*& haystack, nonneg int varid)
{
    ++haystack;
    // Compare only the first character of the string for optimization reasons
    switch (haystack[0]) {
    case 'v':
        if (haystack[3] == '%') { // %var%
            haystack += 4;
            if (tok->varId() != 0)
                return 1;
        } else { // %varid%
            if (varid == 0) {
                throw InternalError(tok, "Internal error. Token::Match called with varid 0.");
            }

            haystack += 6;

            if (tok->varId() == varid)
                return 1;
        }
        break;
    case 't':
        // Type (%type%)
    {
        haystack += 5;
        if (tok->isName() && tok->varId() == 0)
            return 1;
    }
    break;
    case 'a':
        // Accept any token (%any%) or assign (%assign%)
    {
        if (haystack[3] == '%') { // %any%
            haystack += 4;
            return 1;
        }
        // %assign%
        haystack += 7;
        if (tok->isAssignmentOp())
            return 1;
    }
    break;
    case 'n':
        // Number (%num%) or name (%name%)
    {
        if (haystack[4] == '%') { // %name%
            haystack += 5;
            if (tok->isName())
                return 1;
        } else {
            haystack += 4;
            if (tok->isNumber())
                return 1;
        }
    }
    break;
    case 'c': {
        haystack += 1;
        // Character (%char%)
        if (haystack[0] == 'h') {
            haystack += 4;
            if (tok->tokType() == Token::eChar)
                return 1;
        }
        // Const operator (%cop%)
        else if (haystack[1] == 'p') {
            haystack += 3;
            if (tok->isConstOp())
                return 1;
        }
        // Comparison operator (%comp%)
        else {
            haystack += 4;
            if (tok->isComparisonOp())
                return 1;
        }
    }
    break;
    case 's':
        // String (%str%)
    {
        haystack += 4;
        if (tok->tokType() == Token::eString)
            return 1;
    }
    break;
    case 'b':
        // Bool (%bool%)
    {
        haystack += 5;
        if (tok->isBoolean())
            return 1;
    }
    break;
    case 'o': {
        ++haystack;
        if (haystack[1] == '%') {
            // Op (%op%)
            if (haystack[0] == 'p') {
                haystack += 2;
                if (tok->isOp())
                    return 1;
            }
            // Or (%or%)
            else {
                haystack += 2;
                if (tok->tokType() == Token::eBitOp && tok->str() == "|")
                    return 1;
            }
        }

        // Oror (%oror%)
        else {
            haystack += 4;
            if (tok->tokType() == Token::eLogicalOp && tok->str() == "||")
                return 1;
        }
    }
    break;
    default:
        //unknown %cmd%, abort
        throw InternalError(tok, "Unexpected command");
    }

    if (*haystack == '|')
        haystack += 1;
    else
        return -1;

    return 0xFFFF;
}

static
#if defined(__GNUC__)
// need to use the old syntax since the C++11 [[xxx:always_inline]] cannot be used here
inline __attribute__((always_inline))
#endif
int multiCompareImpl(const Token *tok, const char *haystack, nonneg int varid)
{
    const char *needle = tok->str().c_str();
    const char *needlePointer = needle;
    for (;;) {
        if (needlePointer == needle && haystack[0] == '%' && haystack[1] != '|' && haystack[1] != '\0' && haystack[1] != ' ' && haystack[1] != '=') {
            const int ret = multiComparePercent(tok, haystack, varid);
            if (ret < 2)
                return ret;
        } else if (*haystack == '|') {
            if (*needlePointer == 0) {
                // If needle is at the end, we have a match.
                return 1;
            }

            needlePointer = needle;
            ++haystack;
        } else if (*needlePointer == *haystack) {
            if (*needlePointer == '\0')
                return 1;
            ++needlePointer;
            ++haystack;
        } else if (*haystack == ' ' || *haystack == '\0') {
            if (needlePointer == needle)
                return 0;
            break;
        }
        // If haystack and needle don't share the same character,
        // find next '|' character.
        else {
            needlePointer = needle;

            do {
                ++haystack;

                if (*haystack == ' ' || *haystack == '\0') {
                    return -1;
                }
                if (*haystack == '|') {
                    break;
                }
            } while (true);

            ++haystack;
        }
    }

    if (*needlePointer == '\0')
        return 1;

    return -1;
}

// cppcheck-suppress unusedFunction - used in tests only
int Token::multiCompare(const Token *tok, const char *haystack, nonneg int varid)
{
    return multiCompareImpl(tok, haystack, varid);
}

bool Token::simpleMatch(const Token *tok, const char pattern[], size_t pattern_len)
{
    if (!tok)
        return false; // shortcut
    const char *current = pattern;
    const char *end = pattern + pattern_len;
    // cppcheck-suppress shadowFunction - TODO: fix this
    const char *next = static_cast<const char*>(std::memchr(pattern, ' ', pattern_len));
    if (!next)
        next = end;

    while (*current) {
        const std::size_t length = next - current;

        if (!tok || length != tok->mStr.length() || std::strncmp(current, tok->mStr.c_str(), length) != 0)
            return false;

        current = next;
        if (*next) {
            next = std::strchr(++current, ' ');
            if (!next)
                next = end;
        }
        tok = tok->next();
    }

    return true;
}

bool Token::firstWordEquals(const char *str, const char *word)
{
    for (;;) {
        if (*str != *word)
            return (*str == ' ' && *word == 0);
        if (*str == 0)
            break;

        ++str;
        ++word;
    }

    return true;
}

const char *Token::chrInFirstWord(const char *str, char c)
{
    for (;;) {
        if (*str == ' ' || *str == 0)
            return nullptr;

        if (*str == c)
            return str;

        ++str;
    }
}

bool Token::Match(const Token *tok, const char pattern[], nonneg int varid)
{
    if (!(*pattern))
        return true;

    const char *p = pattern;
    while (true) {
        // Skip spaces in pattern..
        while (*p == ' ')
            ++p;

        // No token => Success!
        if (*p == '\0')
            break;

        if (!tok) {
            // If we have no tokens, pattern "!!else" should return true
            if (p[0] == '!' && p[1] == '!' && p[2] != '\0') {
                while (*p && *p != ' ')
                    ++p;
                continue;
            }

            return false;
        }

        // [.. => search for a one-character token..
        if (p[0] == '[' && chrInFirstWord(p, ']')) {
            if (tok->str().length() != 1)
                return false;

            const char *temp = p+1;
            bool chrFound = false;
            int count = 0;
            while (*temp && *temp != ' ') {
                if (*temp == ']') {
                    ++count;
                }

                else if (*temp == tok->str()[0]) {
                    chrFound = true;
                    break;
                }

                ++temp;
            }

            if (count > 1 && tok->str()[0] == ']')
                chrFound = true;

            if (!chrFound)
                return false;

            p = temp;
        }

        // Parse "not" options. Token can be anything except the given one
        else if (p[0] == '!' && p[1] == '!' && p[2] != '\0') {
            p += 2;
            if (firstWordEquals(p, tok->str().c_str()))
                return false;
        }

        // Parse multi options, such as void|int|char (accept token which is one of these 3)
        else {
            const int res = multiCompareImpl(tok, p, varid);
            if (res == 0) {
                // Empty alternative matches, use the same token on next round
                while (*p && *p != ' ')
                    ++p;
                continue;
            }
            if (res == -1) {
                // No match
                return false;
            }
        }

        // using strchr() for the other instances leads to a performance decrease
        if (!(p = strchr(p, ' ')))
            break;

        tok = tok->next();
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}

nonneg int Token::getStrLength(const Token *tok)
{
    assert(tok != nullptr);
    assert(tok->mTokType == eString);

    const std::string s(replaceEscapeSequences(getStringLiteral(tok->str())));

    const auto pos = s.find('\0');
    return pos < s.size() ? pos : s.size();
}

nonneg int Token::getStrArraySize(const Token *tok)
{
    assert(tok != nullptr);
    assert(tok->tokType() == eString);
    // cppcheck-suppress shadowFunction - TODO: fix this
    const std::string str(getStringLiteral(tok->str()));
    int sizeofstring = 1;
    for (int i = 0; i < static_cast<int>(str.size()); i++) {
        if (str[i] == '\\')
            ++i;
        ++sizeofstring;
    }
    return sizeofstring;
}

nonneg int Token::getStrSize(const Token *tok, const Settings &settings)
{
    assert(tok != nullptr && tok->tokType() == eString);
    nonneg int sizeofType = 1;
    if (tok->valueType()) {
        ValueType vt(*tok->valueType());
        vt.pointer = 0;
        sizeofType = vt.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
    }
    return getStrArraySize(tok) * sizeofType;
}

void Token::move(Token *srcStart, Token *srcEnd, Token *newLocation)
{
    /**[newLocation] -> b -> c -> [srcStart] -> [srcEnd] -> f */

    // Fix the gap, which tokens to be moved will leave
    srcStart->previous()->next(srcEnd->next());
    srcEnd->next()->previous(srcStart->previous());

    // Fix the tokens to be moved
    srcEnd->next(newLocation->next());
    srcStart->previous(newLocation);

    // Fix the tokens at newLocation
    newLocation->next()->previous(srcEnd);
    newLocation->next(srcStart);

    // Update _progressValue
    for (Token *tok = srcStart; tok != srcEnd->next(); tok = tok->next())
        tok->mImpl->mProgressValue = newLocation->mImpl->mProgressValue;
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* nextArgumentImpl(T *thisTok)
{
    for (T* tok = thisTok; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        if (tok->link() && Token::Match(tok, "(|{|[|<"))
            tok = tok->link();
        else if (Token::Match(tok, ")|;"))
            return nullptr;
    }
    return nullptr;
}

const Token* Token::nextArgument() const
{
    return nextArgumentImpl(this);
}

Token *Token::nextArgument()
{
    return nextArgumentImpl(this);
}

const Token* Token::nextArgumentBeforeCreateLinks2() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        if (tok->link() && Token::Match(tok, "(|{|["))
            tok = tok->link();
        else if (tok->str() == "<") {
            const Token* temp = tok->findClosingBracket();
            if (temp)
                tok = temp;
        } else if (Token::Match(tok, ")|;"))
            return nullptr;
    }
    return nullptr;
}

const Token* Token::nextTemplateArgument() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        if (tok->link() && Token::Match(tok, "(|{|[|<"))
            tok = tok->link();
        else if (Token::Match(tok, ">|;"))
            return nullptr;
    }
    return nullptr;
}

static bool isOperator(const Token *tok)
{
    if (tok->link())
        tok = tok->link();
    // TODO handle multi token operators
    return tok->strAt(-1) == "operator";
}

const Token * Token::findClosingBracket() const
{
    if (mStr != "<")
        return nullptr;

    if (!mPrevious)
        return nullptr;

    if (!(mPrevious->isName() || Token::simpleMatch(mPrevious, "]") ||
          Token::Match(mPrevious->previous(), "operator %op% <") ||
          Token::Match(mPrevious->tokAt(-2), "operator [([] [)]] <")))
        return nullptr;

    const Token *closing = nullptr;
    const bool templateParameter(strAt(-1) == "template");
    std::set<std::string> templateParameters;

    bool isDecl = true;
    for (const Token *prev = previous(); prev; prev = prev->previous()) {
        if (prev->str() == "=")
            isDecl = false;
        if (Token::simpleMatch(prev, "template <"))
            isDecl = true;
        if (Token::Match(prev, "[;{}]"))
            break;
    }

    unsigned int depth = 0;
    for (closing = this; closing != nullptr; closing = closing->next()) {
        if (Token::Match(closing, "{|[|(")) {
            closing = closing->link();
            if (!closing)
                return nullptr; // #6803
        } else if (Token::Match(closing, "}|]|)|;"))
            return nullptr;
        // we can make some guesses for template parameters
        else if (closing->str() == "<" && closing->previous() &&
                 (closing->previous()->isName() || Token::simpleMatch(closing->previous(), "]") || isOperator(closing->previous())) &&
                 (templateParameter ? templateParameters.find(closing->strAt(-1)) == templateParameters.end() : true))
            ++depth;
        else if (closing->str() == ">") {
            if (--depth == 0)
                return closing;
        } else if (closing->str() == ">>" || closing->str() == ">>=") {
            if (!isDecl && depth == 1)
                continue;
            if (depth <= 2)
                return closing;
            depth -= 2;
        }
        // save named template parameter
        else if (templateParameter && depth == 1 && Token::Match(closing, "[,=]") &&
                 closing->previous()->isName() && !Token::Match(closing->previous(), "class|typename|.") && !Token::Match(closing->tokAt(-2), "=|::"))
            templateParameters.insert(closing->strAt(-1));
    }

    return closing;
}

Token * Token::findClosingBracket()
{
    // return value of const function
    return const_cast<Token*>(static_cast<const Token*>(this)->findClosingBracket());
}

const Token * Token::findOpeningBracket() const
{
    if (mStr != ">")
        return nullptr;

    const Token *opening = nullptr;

    unsigned int depth = 0;
    for (opening = this; opening != nullptr; opening = opening->previous()) {
        if (Token::Match(opening, "}|]|)")) {
            opening = opening->link();
            if (!opening)
                return nullptr;
        } else if (Token::Match(opening, "{|{|(|;"))
            return nullptr;
        else if (opening->str() == ">")
            ++depth;
        else if (opening->str() == "<") {
            if (--depth == 0)
                return opening;
        }
    }

    return opening;
}

Token * Token::findOpeningBracket()
{
    // return value of const function
    return const_cast<Token*>(static_cast<const Token*>(this)->findOpeningBracket());
}

//---------------------------------------------------------------------------

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T *findsimplematchImpl(T * const startTok, const char pattern[], size_t pattern_len)
{
    for (T* tok = startTok; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern, pattern_len))
            return tok;
    }
    return nullptr;
}

const Token *Token::findsimplematch(const Token * const startTok, const char pattern[], size_t pattern_len)
{
    return findsimplematchImpl(startTok, pattern, pattern_len);
}

Token *Token::findsimplematch(Token * const startTok, const char pattern[], size_t pattern_len)
{
    return findsimplematchImpl(startTok, pattern, pattern_len);
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T *findsimplematchImpl(T * const startTok, const char pattern[], size_t pattern_len, const Token * const end)
{
    for (T* tok = startTok; tok && tok != end; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern, pattern_len))
            return tok;
    }
    return nullptr;
}

const Token *Token::findsimplematch(const Token * const startTok, const char pattern[], size_t pattern_len, const Token * const end)
{
    return findsimplematchImpl(startTok, pattern, pattern_len, end);
}

Token *Token::findsimplematch(Token * const startTok, const char pattern[], size_t pattern_len, const Token * const end) {
    return findsimplematchImpl(startTok, pattern, pattern_len, end);
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T *findmatchImpl(T * const startTok, const char pattern[], const nonneg int varId)
{
    for (T* tok = startTok; tok; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return nullptr;
}

const Token *Token::findmatch(const Token * const startTok, const char pattern[], const nonneg int varId)
{
    return findmatchImpl(startTok, pattern, varId);
}

Token *Token::findmatch(Token * const startTok, const char pattern[], const nonneg int varId) {
    return findmatchImpl(startTok, pattern, varId);
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T *findmatchImpl(T * const startTok, const char pattern[], const Token * const end, const nonneg int varId)
{
    for (T* tok = startTok; tok && tok != end; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return nullptr;
}

const Token *Token::findmatch(const Token * const startTok, const char pattern[], const Token * const end, const nonneg int varId)
{
    return findmatchImpl(startTok, pattern, end, varId);
}

Token *Token::findmatch(Token * const startTok, const char pattern[], const Token * const end, const nonneg int varId) {
    return findmatchImpl(startTok, pattern, end, varId);
}

void Token::function(const Function *f)
{
    mImpl->mFunction = f;
    if (f) {
        if (f->isLambda())
            tokType(eLambda);
        else
            tokType(eFunction);
    } else if (mTokType == eFunction)
        tokType(eName);
}

Token* Token::insertToken(const std::string& tokenStr, bool prepend)
{
    Token *newToken;
    if (mStr.empty())
        newToken = this;
    else
        newToken = new Token(mList, mTokensFrontBack);
    newToken->str(tokenStr);

    if (newToken != this) {
        newToken->mImpl->mLineNumber = mImpl->mLineNumber;
        newToken->mImpl->mFileIndex = mImpl->mFileIndex;
        newToken->mImpl->mProgressValue = mImpl->mProgressValue;

        if (prepend) {
            if (this->previous()) {
                newToken->previous(this->previous());
                newToken->previous()->next(newToken);
            } else {
                mTokensFrontBack->front = newToken;
            }
            this->previous(newToken);
            newToken->next(this);
        } else {
            if (this->next()) {
                newToken->next(this->next());
                newToken->next()->previous(newToken);
            } else {
                mTokensFrontBack->back = newToken;
            }
            this->next(newToken);
            newToken->previous(this);
        }

        if (mImpl->mScopeInfo) {
            // If the brace is immediately closed there is no point opening a new scope for it
            if (newToken->str() == "{") {
                std::string nextScopeNameAddition;
                // This might be the opening of a member function
                Token *tok1 = newToken;
                while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                    tok1 = tok1->previous();
                if (tok1->previous() && tok1->strAt(-1) == ")") {
                    tok1 = tok1->linkAt(-1);
                    if (Token::Match(tok1->previous(), "throw|noexcept")) {
                        tok1 = tok1->previous();
                        while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                            tok1 = tok1->previous();
                        if (tok1->strAt(-1) != ")")
                            return newToken;
                    } else if (Token::Match(newToken->tokAt(-2), ":|, %name%")) {
                        tok1 = tok1->tokAt(-2);
                        if (tok1->strAt(-1) != ")")
                            return newToken;
                    }
                    if (tok1->strAt(-1) == ">")
                        tok1 = tok1->previous()->findOpeningBracket();
                    if (tok1 && Token::Match(tok1->tokAt(-3), "%name% :: %name%")) {
                        tok1 = tok1->tokAt(-2);
                        // cppcheck-suppress shadowFunction - TODO: fix this
                        std::string scope = tok1->strAt(-1);
                        while (Token::Match(tok1->tokAt(-2), ":: %name%")) {
                            scope = tok1->strAt(-3) + " :: " + scope;
                            tok1 = tok1->tokAt(-2);
                        }
                        nextScopeNameAddition += scope;
                    }
                }

                // Or it might be a namespace/class/struct
                if (Token::Match(newToken->previous(), "%name%|>")) {
                    Token* nameTok = newToken->previous();
                    while (nameTok && !Token::Match(nameTok, "namespace|class|struct|union %name% {|::|:|<")) {
                        nameTok = nameTok->previous();
                    }
                    if (nameTok) {
                        for (nameTok = nameTok->next(); nameTok && !Token::Match(nameTok, "{|:|<"); nameTok = nameTok->next()) {
                            nextScopeNameAddition.append(nameTok->str());
                            nextScopeNameAddition.append(" ");
                        }
                        if (!nextScopeNameAddition.empty())
                            nextScopeNameAddition.pop_back();
                    }
                }

                // New scope is opening, record it here
                std::shared_ptr<ScopeInfo2> newScopeInfo = std::make_shared<ScopeInfo2>(mImpl->mScopeInfo->name, nullptr, mImpl->mScopeInfo->usingNamespaces);

                if (!newScopeInfo->name.empty() && !nextScopeNameAddition.empty()) newScopeInfo->name.append(" :: ");
                newScopeInfo->name.append(nextScopeNameAddition);
                nextScopeNameAddition = "";

                newToken->scopeInfo(std::move(newScopeInfo));
            } else if (newToken->str() == "}") {
                Token* matchingTok = newToken->previous();
                int depth = 0;
                while (matchingTok && (depth != 0 || !Token::simpleMatch(matchingTok, "{"))) {
                    if (Token::simpleMatch(matchingTok, "}")) depth++;
                    if (Token::simpleMatch(matchingTok, "{")) depth--;
                    matchingTok = matchingTok->previous();
                }
                if (matchingTok && matchingTok->previous()) {
                    newToken->mImpl->mScopeInfo = matchingTok->previous()->scopeInfo();
                }
            } else {
                if (prepend && newToken->previous()) {
                    newToken->mImpl->mScopeInfo = newToken->previous()->scopeInfo();
                } else {
                    newToken->mImpl->mScopeInfo = mImpl->mScopeInfo;
                }
                if (newToken->str() == ";") {
                    const Token* statementStart = newToken;
                    while (statementStart->previous() && !Token::Match(statementStart->previous(), ";|{")) {
                        statementStart = statementStart->previous();
                    }
                    if (Token::Match(statementStart, "using namespace %name% ::|;")) {
                        const Token * tok1 = statementStart->tokAt(2);
                        std::string nameSpace;
                        while (tok1 && tok1->str() != ";") {
                            if (!nameSpace.empty())
                                nameSpace += " ";
                            nameSpace += tok1->str();
                            tok1 = tok1->next();
                        }
                        mImpl->mScopeInfo->usingNamespaces.insert(std::move(nameSpace));
                    }
                }
            }
        }
    }
    return newToken;
}

Token* Token::insertToken(const std::string& tokenStr, const std::string& originalNameStr, bool prepend)
{
    Token* const newToken = insertToken(tokenStr, prepend);
    if (!originalNameStr.empty())
        newToken->originalName(originalNameStr);
    return newToken;
}

Token* Token::insertToken(const std::string& tokenStr, const std::string& originalNameStr, const std::string& macroNameStr, bool prepend)
{
    Token* const newToken = insertToken(tokenStr, originalNameStr, prepend);
    if (!macroNameStr.empty())
        newToken->setMacroName(macroNameStr);
    return newToken;
}

void Token::eraseTokens(Token *begin, const Token *end)
{
    if (!begin || begin == end)
        return;

    while (begin->next() && begin->next() != end) {
        begin->deleteNext();
    }
}

void Token::createMutualLinks(Token *begin, Token *end)
{
    assert(begin != nullptr);
    assert(end != nullptr);
    assert(begin != end);
    begin->link(end);
    end->link(begin);
}

void Token::printOut() const
{
    printOut(std::cout, "");
}

void Token::printOut(std::ostream& out, const char *title) const
{
    if (title && title[0])
        out << "\n### " << title << " ###\n";
    out << stringifyList(stringifyOptions::forPrintOut(), nullptr, nullptr) << std::endl;
}

void Token::printOut(std::ostream& out, bool xml, const char *title, const std::vector<std::string> &fileNames) const
{
    if (xml)
    {
        out << "<file>" << std::endl;
        out << "<![CDATA[";
    }
    if (title && title[0])
        out << "\n### " << title << " ###\n";
    out << stringifyList(stringifyOptions::forPrintOut(), &fileNames, nullptr) << std::endl;
    if (xml)
    {
        out << "]]>" << std::endl;
        out << "</file>" << std::endl;
    }
}

// cppcheck-suppress unusedFunction - used for debugging
void Token::printLines(std::ostream& out, int lines) const
{
    const Token *end = this;
    while (end && end->linenr() < lines + linenr())
        end = end->next();
    out << stringifyList(stringifyOptions::forDebugExprId(), nullptr, end) << std::endl;
}

std::string Token::stringify(const stringifyOptions& options) const
{
    std::string ret;
    if (options.attributes) {
        if (isUnsigned())
            ret += "unsigned ";
        else if (isSigned())
            ret += "signed ";
        if (isComplex())
            ret += "_Complex ";
        if (isLong()) {
            if (!(mTokType == eString || mTokType == eChar))
                ret += "long ";
        }
    }
    if (options.macro && isExpandedMacro())
        ret += '$';
    if (isName() && mStr.find(' ') != std::string::npos) {
        for (const char i : mStr) {
            if (i != ' ')
                ret += i;
        }
    } else if (mStr[0] != '\"' || mStr.find('\0') == std::string::npos)
        ret += mStr;
    else {
        for (const char i : mStr) {
            if (i == '\0')
                ret += "\\0";
            else
                ret += i;
        }
    }
    if (options.varid && mImpl->mVarId != 0) {
        ret += '@';
        ret += (options.idtype ? "var" : "");
        ret += std::to_string(mImpl->mVarId);
    } else if (options.exprid && mImpl->mExprId != 0) {
        ret += '@';
        ret += (options.idtype ? "expr" : "");
        if ((mImpl->mExprId & (1U << efIsUnique)) != 0)
            ret += "UNIQUE";
        else
            ret += std::to_string(mImpl->mExprId);
    }

    return ret;
}

std::string Token::stringify(bool varid, bool attributes, bool macro) const
{
    stringifyOptions options;
    options.varid = varid;
    options.attributes = attributes;
    options.macro = macro;
    return stringify(options);
}

std::string Token::stringifyList(const stringifyOptions& options, const std::vector<std::string>* fileNames, const Token* end) const
{
    if (this == end)
        return "";

    std::string ret;

    unsigned int lineNumber = mImpl->mLineNumber - (options.linenumbers ? 1U : 0U);
    // cppcheck-suppress shadowFunction - TODO: fix this
    unsigned int fileIndex = options.files ? ~0U : mImpl->mFileIndex;
    std::map<int, unsigned int> lineNumbers;
    for (const Token *tok = this; tok != end; tok = tok->next()) {
        assert(tok && "end precedes token");
        if (!tok)
            return ret;
        bool fileChange = false;
        if (tok->mImpl->mFileIndex != fileIndex) {
            if (fileIndex != ~0U) {
                lineNumbers[fileIndex] = tok->mImpl->mFileIndex;
            }

            fileIndex = tok->mImpl->mFileIndex;
            if (options.files) {
                ret += "\n\n##file ";
                if (fileNames && fileNames->size() > tok->mImpl->mFileIndex)
                    ret += fileNames->at(tok->mImpl->mFileIndex);
                else
                    ret += std::to_string(fileIndex);
                ret += '\n';
            }

            lineNumber = lineNumbers[fileIndex];
            fileChange = true;
        }

        if (options.linebreaks && (lineNumber != tok->linenr() || fileChange)) {
            if (lineNumber+4 < tok->linenr() && fileIndex == tok->mImpl->mFileIndex) {
                ret += '\n';
                ret += std::to_string(lineNumber+1);
                ret += ":\n|\n";
                ret += std::to_string(tok->linenr()-1);
                ret += ":\n";
                ret += std::to_string(tok->linenr());
                ret += ": ";
            } else if (this == tok && options.linenumbers) {
                ret += std::to_string(tok->linenr());
                ret += ": ";
            } else if (lineNumber > tok->linenr()) {
                lineNumber = tok->linenr();
                ret += '\n';
                if (options.linenumbers) {
                    ret += std::to_string(lineNumber);
                    ret += ':';
                    ret += ' ';
                }
            } else {
                while (lineNumber < tok->linenr()) {
                    ++lineNumber;
                    ret += '\n';
                    if (options.linenumbers) {
                        ret += std::to_string(lineNumber);
                        ret += ':';
                        if (lineNumber == tok->linenr())
                            ret += ' ';
                    }
                }
            }
            lineNumber = tok->linenr();
        }

        ret += tok->stringify(options); // print token
        if (tok->next() != end && (!options.linebreaks || (tok->next()->linenr() == tok->linenr() && tok->next()->fileIndex() == tok->fileIndex())))
            ret += ' ';
    }
    if (options.linebreaks && (options.files || options.linenumbers))
        ret += '\n';
    return ret;
}
std::string Token::stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames, const Token* end) const
{
    stringifyOptions options;
    options.varid = varid;
    options.attributes = attributes;
    options.macro = attributes;
    options.linenumbers = linenumbers;
    options.linebreaks = linebreaks;
    options.files = files;
    return stringifyList(options, fileNames, end);
}

std::string Token::stringifyList(const Token* end, bool attributes) const
{
    return stringifyList(false, attributes, false, false, false, nullptr, end);
}

void Token::astParent(Token* tok)
{
    const Token* tok2 = tok;
    while (tok2) {
        if (this == tok2)
            throw InternalError(this, "Internal error. AST cyclic dependency.");
        tok2 = tok2->astParent();
    }
    // Clear children to avoid nodes referenced twice
    if (this->astParent()) {
        Token* parent = this->astParent();
        if (parent->astOperand1() == this)
            parent->mImpl->mAstOperand1 = nullptr;
        if (parent->astOperand2() == this)
            parent->mImpl->mAstOperand2 = nullptr;
    }
    mImpl->mAstParent = tok;
}

void Token::astOperand1(Token *tok)
{
    if (mImpl->mAstOperand1)
        mImpl->mAstOperand1->astParent(nullptr);
    // goto parent operator
    if (tok) {
        tok = tok->astTop();
        tok->astParent(this);
    }
    mImpl->mAstOperand1 = tok;
}

void Token::astOperand2(Token *tok)
{
    if (mImpl->mAstOperand2)
        mImpl->mAstOperand2->astParent(nullptr);
    // goto parent operator
    if (tok) {
        tok = tok->astTop();
        tok->astParent(this);
    }
    mImpl->mAstOperand2 = tok;
}

static const Token* goToLeftParenthesis(const Token* start, const Token* end)
{
    // move start to lpar in such expression: '(*it).x'
    int par = 0;
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        if (tok->str() == "(")
            ++par;
        else if (tok->str() == ")") {
            if (par == 0)
                start = tok->link();
            else
                --par;
        }
    }
    return start;
}

static const Token* goToRightParenthesis(const Token* start, const Token* end)
{
    // move end to rpar in such expression: '2>(x+1)'
    int par = 0;
    for (const Token *tok = end; tok && tok != start; tok = tok->previous()) {
        if (tok->str() == ")")
            ++par;
        else if (tok->str() == "(") {
            if (par == 0)
                end = tok->link();
            else
                --par;
        }
    }
    return end;
}

std::pair<const Token *, const Token *> Token::findExpressionStartEndTokens() const
{
    const Token * const top = this;

    // find start node in AST tree
    const Token *start = top;
    while (start->astOperand1() && precedes(start->astOperand1(), start))
        start = start->astOperand1();

    // find end node in AST tree
    const Token *end = top;
    while (end->astOperand1() && (end->astOperand2() || end->isUnaryPreOp())) {
        // lambda..
        if (end->str() == "[") {
            const Token *lambdaEnd = findLambdaEndToken(end);
            if (lambdaEnd) {
                end = lambdaEnd;
                break;
            }
        }
        if (Token::Match(end,"(|[|{") &&
            !(Token::Match(end, "( ::| %type%") && !end->astOperand2())) {
            end = end->link();
            break;
        }
        end = end->astOperand2() ? end->astOperand2() : end->astOperand1();
    }

    // skip parentheses
    start = goToLeftParenthesis(start, end);
    end = goToRightParenthesis(start, end);
    if (Token::simpleMatch(end, "{"))
        end = end->link();

    if (precedes(top, start))
        throw InternalError(start, "Cannot find start of expression");
    if (succeeds(top, end))
        throw InternalError(end, "Cannot find end of expression");

    return std::pair<const Token *, const Token *>(start,end);
}

bool Token::isCalculation() const
{
    if (!Token::Match(this, "%cop%|++|--"))
        return false;

    if (Token::Match(this, "*|&")) {
        // dereference or address-of?
        if (!this->astOperand2())
            return false;

        if (this->astOperand2()->str() == "[")
            return false;

        // type specification?
        std::stack<const Token *> operands;
        operands.push(this);
        while (!operands.empty()) {
            const Token *op = operands.top();
            operands.pop();
            if (op->isNumber() || op->varId() > 0)
                return true;
            if (op->astOperand1())
                operands.push(op->astOperand1());
            if (op->astOperand2())
                operands.push(op->astOperand2());
            else if (Token::Match(op, "*|&"))
                return false;
        }

        // type specification => return false
        return false;
    }

    return true;
}

bool Token::isUnaryPreOp() const
{
    if (!astOperand1() || astOperand2())
        return false;
    if (this->tokType() != Token::eIncDecOp)
        return true;
    const Token *tokbefore = mPrevious;
    const Token *tokafter = mNext;
    for (int distance = 1; distance < 10 && tokbefore; distance++) {
        if (tokbefore == mImpl->mAstOperand1)
            return false;
        if (tokafter == mImpl->mAstOperand1)
            return true;
        tokbefore = tokbefore->mPrevious;
        tokafter  = tokafter->mPrevious;
    }
    return false; // <- guess
}

static std::string stringFromTokenRange(const Token* start, const Token* end)
{
    std::string ret;
    if (end)
        end = end->next();
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        if (tok->isUnsigned())
            ret += "unsigned ";
        if (tok->isLong() && !tok->isLiteral())
            ret += "long ";
        if (tok->tokType() == Token::eString) {
            for (const unsigned char c: tok->str()) {
                if (c == '\n')
                    ret += "\\n";
                else if (c == '\r')
                    ret += "\\r";
                else if (c == '\t')
                    ret += "\\t";
                else if (c >= ' ' && c <= 126)
                    ret += c;
                else {
                    char str[5];
                    snprintf(str, sizeof(str), "\\x%02x", c);
                    ret += str;
                }
            }
        } else if (tok->originalName().empty() || tok->isUnsigned() || tok->isLong()) {
            ret += tok->str();
        } else
            ret += tok->originalName();
        if (Token::Match(tok, "%name%|%num% %name%|%num%"))
            ret += ' ';
    }
    return ret;
}

std::string Token::expressionString() const
{
    const auto tokens = findExpressionStartEndTokens();
    return stringFromTokenRange(tokens.first, tokens.second);
}

static void astStringXml(const Token *tok, nonneg int indent, std::ostream &out)
{
    const std::string strindent(indent, ' ');

    out << strindent << "<token str=\"" << tok->str() << '\"';
    if (tok->varId())
        out << " varId=\"" << tok->varId() << '\"';
    if (tok->variable())
        out << " variable=\"" << tok->variable() << '\"';
    if (tok->function())
        out << " function=\"" << tok->function() << '\"';
    if (!tok->values().empty())
        out << " values=\"" << &tok->values() << '\"';

    if (!tok->astOperand1() && !tok->astOperand2()) {
        out << "/>" << std::endl;
    }

    else {
        out << '>' << std::endl;
        if (tok->astOperand1())
            astStringXml(tok->astOperand1(), indent+2U, out);
        if (tok->astOperand2())
            astStringXml(tok->astOperand2(), indent+2U, out);
        out << strindent << "</token>" << std::endl;
    }
}

void Token::printAst(bool xml, const std::vector<std::string> &fileNames, std::ostream &out) const
{
    if (!xml)
        out << "\n\n##AST" << std::endl;

    std::set<const Token *> printed;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (!tok->mImpl->mAstParent && tok->mImpl->mAstOperand1) {
            if (printed.find(tok) != printed.end())
                continue;
            printed.insert(tok);

            if (xml) {
                out << "<ast scope=\"" << tok->scope() << "\" fileIndex=\"" << tok->fileIndex() << "\" linenr=\"" << tok->linenr()
                    << "\" column=\"" << tok->column() << "\">" << std::endl;
                astStringXml(tok, 2U, out);
                out << "</ast>" << std::endl;
            } else
                out << "[" << fileNames[tok->fileIndex()] << ":" << tok->linenr() << "]" << std::endl << tok->astStringVerbose() << std::endl;
            if (tok->str() == "(")
                tok = tok->link();
        }
    }
}

static void indent(std::string &str, const nonneg int indent1, const nonneg int indent2)
{
    for (int i = 0; i < indent1; ++i)
        str += ' ';
    for (int i = indent1; i < indent2; i += 2)
        str += "| ";
}

void Token::astStringVerboseRecursive(std::string& ret, const nonneg int indent1, const nonneg int indent2) const
{
    if (isExpandedMacro())
        ret += '$';
    ret += mStr;
    if (mImpl->mValueType)
        ret += " \'" + mImpl->mValueType->str() + '\'';
    if (function()) {
        std::ostringstream ostr;
        ostr << std::hex << function();
        ret += " f:" + ostr.str();
    }
    ret += '\n';

    if (mImpl->mAstOperand1) {
        int i1 = indent1, i2 = indent2 + 2;
        if (indent1 == indent2 && !mImpl->mAstOperand2)
            i1 += 2;
        indent(ret, indent1, indent2);
        ret += mImpl->mAstOperand2 ? "|-" : "`-";
        mImpl->mAstOperand1->astStringVerboseRecursive(ret, i1, i2);
    }
    if (mImpl->mAstOperand2) {
        int i1 = indent1, i2 = indent2 + 2;
        if (indent1 == indent2)
            i1 += 2;
        indent(ret, indent1, indent2);
        ret += "`-";
        mImpl->mAstOperand2->astStringVerboseRecursive(ret, i1, i2);
    }
}

std::string Token::astStringVerbose() const
{
    std::string ret;
    astStringVerboseRecursive(ret);
    return ret;
}

// cppcheck-suppress unusedFunction // used in test
std::string Token::astStringZ3() const
{
    if (!astOperand1())
        return str();
    if (!astOperand2())
        return "(" + str() + " " + astOperand1()->astStringZ3() + ")";
    return "(" + str() + " " + astOperand1()->astStringZ3() + " " + astOperand2()->astStringZ3() + ")";
}

void Token::printValueFlow(const std::vector<std::string>& files, bool xml, std::ostream &out) const
{
    std::string outs;

    // cppcheck-suppress shadowFunction
    int fileIndex = -1;
    int line = 0;
    if (xml)
        outs += "  <valueflow>\n";
    else
        outs += "\n\n##Value flow\n";
    for (const Token *tok = this; tok; tok = tok->next()) {
        // cppcheck-suppress shadowFunction - TODO: fix this
        const auto* const values = tok->mImpl->mValues;
        if (!values)
            continue;
        if (values->empty()) // Values might be removed by removeContradictions
            continue;
        if (xml) {
            outs += "    <values id=\"";
            outs += id_string(values);
            outs +=  "\">";
            outs += '\n';
        }
        else {
            if (fileIndex != tok->fileIndex()) {
                outs += "File ";
                outs += files[tok->fileIndex()];
                outs += '\n';
                line = 0;
            }
            if (line != tok->linenr()) {
                outs += "Line ";
                outs += std::to_string(tok->linenr());
                outs += '\n';
            }
        }
        fileIndex = tok->fileIndex();
        line = tok->linenr();
        if (!xml) {
            ValueFlow::Value::ValueKind valueKind = values->front().valueKind;
            const bool same = std::all_of(values->begin(), values->end(), [&](const ValueFlow::Value& value) {
                return value.valueKind == valueKind;
            });
            outs += "  ";
            outs += tok->str();
            outs += " ";
            if (same) {
                switch (valueKind) {
                case ValueFlow::Value::ValueKind::Impossible:
                case ValueFlow::Value::ValueKind::Known:
                    outs += "always ";
                    break;
                case ValueFlow::Value::ValueKind::Inconclusive:
                    outs += "inconclusive ";
                    break;
                case ValueFlow::Value::ValueKind::Possible:
                    outs += "possible ";
                    break;
                }
            }
            if (values->size() > 1U)
                outs += '{';
        }
        for (const ValueFlow::Value& value : *values) {
            if (xml) {
                outs += "      <value ";
                switch (value.valueType) {
                case ValueFlow::Value::ValueType::INT:
                    if (tok->valueType() && tok->valueType()->sign == ValueType::UNSIGNED) {
                        outs += "intvalue=\"";
                        outs += MathLib::toString(static_cast<MathLib::biguint>(value.intvalue));
                        outs += '\"';
                    }
                    else {
                        outs += "intvalue=\"";
                        outs += MathLib::toString(value.intvalue);
                        outs += '\"';
                    }
                    break;
                case ValueFlow::Value::ValueType::TOK:
                    outs +=  "tokvalue=\"";
                    outs += id_string(value.tokvalue);
                    outs += '\"';
                    break;
                case ValueFlow::Value::ValueType::FLOAT:
                    outs += "floatvalue=\"";
                    outs += MathLib::toString(value.floatValue);
                    outs += '\"';
                    break;
                case ValueFlow::Value::ValueType::MOVED:
                    outs += "movedvalue=\"";
                    outs += ValueFlow::Value::toString(value.moveKind);
                    outs += '\"';
                    break;
                case ValueFlow::Value::ValueType::UNINIT:
                    outs +=  "uninit=\"1\"";
                    break;
                case ValueFlow::Value::ValueType::BUFFER_SIZE:
                    outs += "buffer-size=\"";
                    outs += MathLib::toString(value.intvalue);
                    outs += "\"";
                    break;
                case ValueFlow::Value::ValueType::CONTAINER_SIZE:
                    outs += "container-size=\"";
                    outs += MathLib::toString(value.intvalue);
                    outs += '\"';
                    break;
                case ValueFlow::Value::ValueType::ITERATOR_START:
                    outs +=  "iterator-start=\"";
                    outs += MathLib::toString(value.intvalue);
                    outs += '\"';
                    break;
                case ValueFlow::Value::ValueType::ITERATOR_END:
                    outs +=  "iterator-end=\"";
                    outs += MathLib::toString(value.intvalue);
                    outs += '\"';
                    break;
                case ValueFlow::Value::ValueType::LIFETIME:
                    outs += "lifetime=\"";
                    outs += id_string(value.tokvalue);
                    outs += '\"';
                    outs += " lifetime-scope=\"";
                    outs += ValueFlow::Value::toString(value.lifetimeScope);
                    outs += "\"";
                    outs += " lifetime-kind=\"";
                    outs += ValueFlow::Value::toString(value.lifetimeKind);
                    outs += "\"";
                    break;
                case ValueFlow::Value::ValueType::SYMBOLIC:
                    outs += "symbolic=\"";
                    outs += id_string(value.tokvalue);
                    outs += '\"';
                    outs += " symbolic-delta=\"";
                    outs += MathLib::toString(value.intvalue);
                    outs += '\"';
                    break;
                }
                outs += " bound=\"";
                outs += ValueFlow::Value::toString(value.bound);
                outs += "\"";
                if (value.condition) {
                    outs += " condition-line=\"";
                    outs += std::to_string(value.condition->linenr());
                    outs += '\"';
                }
                if (value.isKnown())
                    outs += " known=\"true\"";
                else if (value.isPossible())
                    outs += " possible=\"true\"";
                else if (value.isImpossible())
                    outs += " impossible=\"true\"";
                else if (value.isInconclusive())
                    outs += " inconclusive=\"true\"";

                outs += " path=\"";
                outs += MathLib::toString(value.path);
                outs += "\"";

                outs += "/>\n";
            }
            else {
                if (&value != &values->front())
                    outs += ",";
                outs += value.toString();
            }
        }
        if (xml)
            outs += "    </values>\n";
        else if (values->size() > 1U)
            outs += "}\n";
        else
            outs += '\n';
    }
    if (xml)
        outs += "  </valueflow>\n";

    out << outs;
}

const ValueFlow::Value * Token::getValueLE(const MathLib::bigint val, const Settings &settings) const
{
    if (!mImpl->mValues)
        return nullptr;
    return ValueFlow::findValue(*mImpl->mValues, settings, [&](const ValueFlow::Value& v) {
        return !v.isImpossible() && v.isIntValue() && v.intvalue <= val;
    });
}

const ValueFlow::Value * Token::getValueGE(const MathLib::bigint val, const Settings &settings) const
{
    if (!mImpl->mValues)
        return nullptr;
    return ValueFlow::findValue(*mImpl->mValues, settings, [&](const ValueFlow::Value& v) {
        return !v.isImpossible() && v.isIntValue() && v.intvalue >= val;
    });
}

const ValueFlow::Value * Token::getValueNE(MathLib::bigint val) const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->cbegin(), mImpl->mValues->cend(), [=](const ValueFlow::Value& value) {
        return value.isIntValue() && !value.isImpossible() && value.intvalue != val;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

const ValueFlow::Value * Token::getInvalidValue(const Token *ftok, nonneg int argnr, const Settings &settings) const
{
    if (!mImpl->mValues)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    for (auto it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isImpossible())
            continue;
        if ((it->isIntValue() && !settings.library.isIntArgValid(ftok, argnr, it->intvalue, settings)) ||
            (it->isFloatValue() && !settings.library.isFloatArgValid(ftok, argnr, it->floatValue, settings))) {
            if (!ret || ret->isInconclusive() || (ret->condition && !it->isInconclusive()))
                ret = &(*it);
            if (!ret->isInconclusive() && !ret->condition)
                break;
        }
    }
    if (ret) {
        if (ret->isInconclusive() && !settings.certainty.isEnabled(Certainty::inconclusive))
            return nullptr;
        if (ret->condition && !settings.severity.isEnabled(Severity::warning))
            return nullptr;
    }
    return ret;
}

const Token *Token::getValueTokenMinStrSize(const Settings &settings, MathLib::bigint* path) const
{
    if (!mImpl->mValues)
        return nullptr;
    const Token *ret = nullptr;
    int minsize = INT_MAX;
    for (auto it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isTokValue() && it->tokvalue && it->tokvalue->tokType() == Token::eString) {
            const int size = getStrSize(it->tokvalue, settings);
            if (!ret || size < minsize) {
                minsize = size;
                ret = it->tokvalue;
                if (path)
                    *path = it->path;
            }
        }
    }
    return ret;
}

const Token *Token::getValueTokenMaxStrLength() const
{
    if (!mImpl->mValues)
        return nullptr;
    const Token *ret = nullptr;
    int maxlength = 0;
    for (auto it = mImpl->mValues->cbegin(); it != mImpl->mValues->end(); ++it) {
        if (it->isTokValue() && it->tokvalue && it->tokvalue->tokType() == Token::eString) {
            const int length = getStrLength(it->tokvalue);
            if (!ret || length > maxlength) {
                maxlength = length;
                ret = it->tokvalue;
            }
        }
    }
    return ret;
}

static bool isAdjacent(const ValueFlow::Value& x, const ValueFlow::Value& y)
{
    if (x.bound != ValueFlow::Value::Bound::Point && x.bound == y.bound)
        return true;
    if (x.valueType == ValueFlow::Value::ValueType::FLOAT)
        return false;
    return std::abs(x.intvalue - y.intvalue) == 1;
}

static bool removePointValue(std::list<ValueFlow::Value>& values, std::list<ValueFlow::Value>::iterator& x)
{
    const bool isPoint = x->bound == ValueFlow::Value::Bound::Point;
    if (!isPoint)
        x->decreaseRange();
    else
        x = values.erase(x);
    return isPoint;
}

static bool removeContradiction(std::list<ValueFlow::Value>& values)
{
    bool result = false;
    for (auto itx = values.begin(); itx != values.end(); ++itx) {
        if (itx->isNonValue())
            continue;

        auto ity = itx;
        ++ity;
        for (; ity != values.end(); ++ity) {
            if (ity->isNonValue())
                continue;
            if (*itx == *ity)
                continue;
            if (itx->valueType != ity->valueType)
                continue;
            if (itx->isImpossible() == ity->isImpossible())
                continue;
            if (itx->isSymbolicValue() && !ValueFlow::Value::sameToken(itx->tokvalue, ity->tokvalue))
                continue;
            if (!itx->equalValue(*ity)) {
                auto compare = [](const std::list<ValueFlow::Value>::const_iterator& x, const std::list<ValueFlow::Value>::const_iterator& y) {
                    return x->compareValue(*y, less{});
                };
                auto itMax = std::max(itx, ity, compare);
                auto itMin = std::min(itx, ity, compare);
                // TODO: Adjust non-points instead of removing them
                if (itMax->isImpossible() && itMax->bound == ValueFlow::Value::Bound::Upper) {
                    values.erase(itMin);
                    return true;
                }
                if (itMin->isImpossible() && itMin->bound == ValueFlow::Value::Bound::Lower) {
                    values.erase(itMax);
                    return true;
                }
                continue;
            }
            const bool removex = !itx->isImpossible() || ity->isKnown();
            const bool removey = !ity->isImpossible() || itx->isKnown();
            if (itx->bound == ity->bound) {
                if (removex)
                    values.erase(itx);
                if (removey)
                    values.erase(ity);
                // itx and ity are invalidated
                return true;
            }
            result = removex || removey;
            bool bail = false;
            if (removex && removePointValue(values, itx))
                bail = true;
            if (removey && removePointValue(values, ity))
                bail = true;
            if (bail)
                return true;
        }
    }
    return result;
}

using ValueIterator = std::list<ValueFlow::Value>::iterator;

template<class Iterator>
// NOLINTNEXTLINE(performance-unnecessary-value-param) - false positive
static ValueIterator removeAdjacentValues(std::list<ValueFlow::Value>& values, ValueIterator x, Iterator start, Iterator last)
{
    if (!isAdjacent(*x, **start))
        return std::next(x);
    auto it = std::adjacent_find(start, last, [](ValueIterator x, ValueIterator y) {
        return !isAdjacent(*x, *y);
    });
    if (it == last)
        it--;
    (*it)->bound = x->bound;
    std::for_each(std::move(start), std::move(it), [&](ValueIterator y) {
        values.erase(y);
    });
    return values.erase(x);
}

static void mergeAdjacent(std::list<ValueFlow::Value>& values)
{
    for (auto x = values.begin(); x != values.end();) {
        if (x->isNonValue()) {
            x++;
            continue;
        }
        if (x->bound == ValueFlow::Value::Bound::Point) {
            x++;
            continue;
        }
        std::vector<ValueIterator> adjValues;
        for (auto y = values.begin(); y != values.end(); y++) {
            if (x == y)
                continue;
            if (y->isNonValue())
                continue;
            if (x->valueType != y->valueType)
                continue;
            if (x->valueKind != y->valueKind)
                continue;
            if (x->isSymbolicValue() && !ValueFlow::Value::sameToken(x->tokvalue, y->tokvalue))
                continue;
            if (x->bound != y->bound) {
                if (y->bound != ValueFlow::Value::Bound::Point && isAdjacent(*x, *y)) {
                    adjValues.clear();
                    break;
                }
                // No adjacent points for floating points
                if (x->valueType == ValueFlow::Value::ValueType::FLOAT)
                    continue;
                if (y->bound != ValueFlow::Value::Bound::Point)
                    continue;
            }
            if (x->bound == ValueFlow::Value::Bound::Lower && !y->compareValue(*x, less{}))
                continue;
            if (x->bound == ValueFlow::Value::Bound::Upper && !x->compareValue(*y, less{}))
                continue;
            adjValues.push_back(y);
        }
        if (adjValues.empty()) {
            x++;
            continue;
        }
        std::sort(adjValues.begin(), adjValues.end(), [&values](ValueIterator xx, ValueIterator yy) {
            (void)values;
            assert(xx != values.end() && yy != values.end());
            return xx->compareValue(*yy, less{});
        });
        if (x->bound == ValueFlow::Value::Bound::Lower)
            x = removeAdjacentValues(values, x, adjValues.rbegin(), adjValues.rend());
        else if (x->bound == ValueFlow::Value::Bound::Upper)
            x = removeAdjacentValues(values, x, adjValues.begin(), adjValues.end());
    }
}

static void removeOverlaps(std::list<ValueFlow::Value>& values)
{
    for (const ValueFlow::Value& x : values) {
        if (x.isNonValue())
            continue;
        values.remove_if([&](const ValueFlow::Value& y) {
            if (y.isNonValue())
                return false;
            if (&x == &y)
                return false;
            if (x.valueType != y.valueType)
                return false;
            if (x.valueKind != y.valueKind)
                return false;
            // TODO: Remove points covered in a lower or upper bound
            // TODO: Remove lower or upper bound already covered by a lower and upper bound
            if (!x.equalValue(y))
                return false;
            if (x.bound != y.bound)
                return false;
            return true;
        });
    }
    mergeAdjacent(values);
}

// Removing contradictions is an NP-hard problem. Instead we run multiple
// passes to try to catch most contradictions
static void removeContradictions(std::list<ValueFlow::Value>& values)
{
    removeOverlaps(values);
    for (int i = 0; i < 4; i++) {
        if (!removeContradiction(values))
            return;
        removeOverlaps(values);
    }
}

static bool sameValueType(const ValueFlow::Value& x, const ValueFlow::Value& y)
{
    if (x.valueType != y.valueType)
        return false;
    // Symbolic are the same type if they share the same tokvalue
    if (x.isSymbolicValue())
        return x.tokvalue->exprId() == 0 || x.tokvalue->exprId() == y.tokvalue->exprId();
    return true;
}

bool Token::addValue(const ValueFlow::Value &value)
{
    if (value.isKnown() && mImpl->mValues) {
        // Clear all other values of the same type since value is known
        mImpl->mValues->remove_if([&](const ValueFlow::Value& x) {
            return sameValueType(x, value);
        });
    }

    // Don't add a value if its already known
    if (!value.isKnown() && mImpl->mValues &&
        std::any_of(mImpl->mValues->begin(), mImpl->mValues->end(), [&](const ValueFlow::Value& x) {
        return x.isKnown() && sameValueType(x, value) && !x.equalValue(value);
    }))
        return false;

    // assert(value.isKnown() || !mImpl->mValues || std::none_of(mImpl->mValues->begin(), mImpl->mValues->end(),
    // [&](const ValueFlow::Value& x) {
    //     return x.isKnown() && sameValueType(x, value);
    // }));

    if (mImpl->mValues) {
        // Don't handle more than 10 values for performance reasons
        // TODO: add setting?
        if (mImpl->mValues->size() >= 10U)
            return false;

        // if value already exists, don't add it again
        auto it = mImpl->mValues->begin();
        for (; it != mImpl->mValues->end(); ++it) {
            // different types => continue
            if (it->valueType != value.valueType)
                continue;

            if (it->isImpossible() != value.isImpossible())
                continue;

            // different value => continue
            if (!it->equalValue(value))
                continue;

            if ((value.isTokValue() || value.isLifetimeValue()) && (it->tokvalue != value.tokvalue) && (it->tokvalue->str() != value.tokvalue->str()))
                continue;

            // same value, but old value is inconclusive so replace it
            if (it->isInconclusive() && !value.isInconclusive() && !value.isImpossible()) {
                *it = value;
                if (it->varId == 0)
                    it->varId = mImpl->mVarId;
                break;
            }

            // Same value already exists, don't  add new value
            return false;
        }

        // Add value
        if (it == mImpl->mValues->end()) {
            ValueFlow::Value v(value);
            if (v.varId == 0)
                v.varId = mImpl->mVarId;
            if (v.isKnown() && v.isIntValue())
                mImpl->mValues->push_front(std::move(v));
            else
                mImpl->mValues->push_back(std::move(v));
        }
    } else {
        ValueFlow::Value v(value);
        if (v.varId == 0)
            v.varId = mImpl->mVarId;
        mImpl->mValues = new std::list<ValueFlow::Value>;
        mImpl->mValues->push_back(std::move(v));
    }

    removeContradictions(*mImpl->mValues);

    return true;
}

void Token::assignProgressValues(Token *tok)
{
    int total_count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        ++total_count;
    int count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        tok2->mImpl->mProgressValue = count++ *100 / total_count;
}

void Token::assignIndexes()
{
    // cppcheck-suppress shadowFunction - TODO: fix this
    int index = (mPrevious ? mPrevious->mImpl->mIndex : 0) + 1;
    for (Token *tok = this; tok; tok = tok->next())
        tok->mImpl->mIndex = index++;
}

void Token::setValueType(ValueType *vt)
{
    if (vt != mImpl->mValueType) {
        delete mImpl->mValueType;
        mImpl->mValueType = vt;
    }
}

const ValueType *Token::argumentType() const {
    const Token *top = this;
    while (top && !Token::Match(top->astParent(), ",|("))
        top = top->astParent();
    return top ? top->mImpl->mValueType : nullptr;
}

void Token::type(const ::Type *t)
{
    mImpl->mType = t;
    if (t) {
        tokType(eType);
        isEnumType(mImpl->mType->isEnumType());
    } else if (mTokType == eType)
        tokType(eName);
}

const ::Type* Token::typeOf(const Token* tok, const Token** typeTok)
{
    if (!tok)
        return nullptr;
    if (typeTok != nullptr)
        *typeTok = tok;
    const Token* lhsVarTok{};
    if (tok->type())
        return tok->type();
    if (tok->variable())
        return tok->variable()->type();
    if (tok->function())
        return tok->function()->retType;
    if (Token::simpleMatch(tok, "return")) {
        // cppcheck-suppress shadowFunction - TODO: fix this
        const Scope *scope = tok->scope();
        if (!scope)
            return nullptr;
        // cppcheck-suppress shadowFunction - TODO: fix this
        const Function *function = scope->function;
        if (!function)
            return nullptr;
        return function->retType;
    }
    if (Token::Match(tok->previous(), "%type%|= (|{"))
        return typeOf(tok->previous(), typeTok);
    if (Token::simpleMatch(tok, "=") && (lhsVarTok = getLHSVariableToken(tok)) != tok->next())
        return Token::typeOf(lhsVarTok, typeTok);
    if (Token::simpleMatch(tok, "."))
        return Token::typeOf(tok->astOperand2(), typeTok);
    if (Token::simpleMatch(tok, "["))
        return Token::typeOf(tok->astOperand1(), typeTok);
    if (Token::simpleMatch(tok, "{")) {
        int argnr;
        const Token* ftok = getTokenArgumentFunction(tok, argnr);
        if (argnr < 0)
            return nullptr;
        if (!ftok)
            return nullptr;
        if (ftok == tok)
            return nullptr;
        std::vector<const Variable*> vars = getArgumentVars(ftok, argnr);
        if (vars.empty())
            return nullptr;
        if (std::all_of(
                vars.cbegin(), vars.cend(), [&](const Variable* var) {
            return var->type() == vars.front()->type();
        }))
            return vars.front()->type();
    }

    return nullptr;
}

std::pair<const Token*, const Token*> Token::typeDecl(const Token* tok, bool pointedToType)
{
    if (!tok)
        return {};
    if (tok->type())
        return {tok, tok->next()};
    if (tok->variable()) {
        const Variable *var = tok->variable();
        if (!var->typeStartToken() || !var->typeEndToken())
            return {};
        if (pointedToType && astIsSmartPointer(var->nameToken())) {
            const ValueType* vt = var->valueType();
            if (vt && vt->smartPointerTypeToken)
                return { vt->smartPointerTypeToken, vt->smartPointerTypeToken->linkAt(-1) };
        }
        if (pointedToType && astIsIterator(var->nameToken())) {
            const ValueType* vt = var->valueType();
            if (vt && vt->containerTypeToken)
                return { vt->containerTypeToken, vt->containerTypeToken->linkAt(-1) };
        }
        std::pair<const Token*, const Token*> result;
        if (Token::simpleMatch(var->typeStartToken(), "auto")) {
            const Token * tok2 = var->declEndToken();
            if (Token::Match(tok2, "; %varid% =", var->declarationId()))
                tok2 = tok2->tokAt(2);
            if (Token::simpleMatch(tok2, "=") && Token::Match(tok2->astOperand2(), "!!=") && tok != tok2->astOperand2()) {
                tok2 = tok2->astOperand2();

                if (Token::simpleMatch(tok2, "[") && tok2->astOperand1()) {
                    const ValueType* vt = tok2->astOperand1()->valueType();
                    if (vt && vt->containerTypeToken)
                        return { vt->containerTypeToken, vt->containerTypeToken->linkAt(-1) };
                }

                const Token* varTok = tok2; // try to find a variable
                if (Token::Match(varTok, ":: %name%"))
                    varTok = varTok->next();
                while (Token::Match(varTok, "%name% ::"))
                    varTok = varTok->tokAt(2);
                std::pair<const Token*, const Token*> r = typeDecl(varTok);
                if (r.first)
                    return r;

                if (pointedToType && tok2->astOperand1() && Token::simpleMatch(tok2, "new")) {
                    if (Token::simpleMatch(tok2->astOperand1(), "("))
                        return { tok2->next(), tok2->astOperand1() };
                    const Token* declEnd = nextAfterAstRightmostLeaf(tok2->astOperand1());
                    if (Token::simpleMatch(declEnd, "<") && declEnd->link())
                        declEnd = declEnd->link()->next();
                    return { tok2->next(), declEnd };
                }
                const Token *typeBeg{}, *typeEnd{};
                if (tok2->str() == "::" && Token::simpleMatch(tok2->astOperand2(), "{")) { // empty initlist
                    typeBeg = previousBeforeAstLeftmostLeaf(tok2);
                    typeEnd = tok2->astOperand2();
                }
                else if (tok2->str() == "{") {
                    typeBeg = previousBeforeAstLeftmostLeaf(tok2);
                    typeEnd = tok2;
                }
                if (typeBeg && typeBeg != typeEnd)
                    result = { typeBeg->next(), typeEnd }; // handle smart pointers/iterators first
            }
            if (astIsRangeBasedForDecl(var->nameToken()) && astIsContainer(var->nameToken()->astParent()->astOperand2())) { // range-based for
                const ValueType* vt = var->nameToken()->astParent()->astOperand2()->valueType();
                if (vt && vt->containerTypeToken)
                    return { vt->containerTypeToken, vt->containerTypeToken->linkAt(-1) };
            }
        }
        if (result.first)
            return result;
        return {var->typeStartToken(), var->typeEndToken()->next()};
    }
    if (Token::simpleMatch(tok, "return")) {
        // cppcheck-suppress shadowFunction - TODO: fix this
        const Scope* scope = tok->scope();
        if (!scope)
            return {};
        // cppcheck-suppress shadowFunction - TODO: fix this
        const Function* function = scope->function;
        if (!function)
            return {};
        return { function->retDef, function->returnDefEnd() };
    }
    if (tok->previous() && tok->previous()->function()) {
        // cppcheck-suppress shadowFunction - TODO: fix this
        const Function *function = tok->previous()->function();
        return {function->retDef, function->returnDefEnd()};
    }
    if (Token::simpleMatch(tok, "="))
        return Token::typeDecl(tok->astOperand1());
    if (Token::simpleMatch(tok, "."))
        return Token::typeDecl(tok->astOperand2());

    const ::Type * t = typeOf(tok);
    if (!t || !t->classDef)
        return {};
    return {t->classDef->next(), t->classDef->tokAt(2)};
}
std::string Token::typeStr(const Token* tok)
{
    if (tok->valueType()) {
        const ValueType * vt = tok->valueType();
        std::string ret = vt->str();
        if (!ret.empty())
            return ret;
    }
    std::pair<const Token*, const Token*> r = Token::typeDecl(tok);
    if (!r.first || !r.second)
        return "";
    return r.first->stringifyList(r.second, false);
}

void Token::scopeInfo(std::shared_ptr<ScopeInfo2> newScopeInfo)
{
    mImpl->mScopeInfo = std::move(newScopeInfo);
}
std::shared_ptr<ScopeInfo2> Token::scopeInfo() const
{
    return mImpl->mScopeInfo;
}

// if there is a known INT value it will always be the first entry
bool Token::hasKnownIntValue() const
{
    if (!mImpl->mValues)
        return false;
    if (mImpl->mValues->empty())
        return false;
    const ValueFlow::Value& value = mImpl->mValues->front();
    return value.isIntValue() && value.isKnown();
}

bool Token::hasKnownValue() const
{
    return mImpl->mValues && std::any_of(mImpl->mValues->begin(), mImpl->mValues->end(), std::mem_fn(&ValueFlow::Value::isKnown));
}

bool Token::hasKnownValue(ValueFlow::Value::ValueType t) const
{
    return mImpl->mValues &&
           std::any_of(mImpl->mValues->begin(), mImpl->mValues->end(), [&](const ValueFlow::Value& value) {
        return value.isKnown() && value.valueType == t;
    });
}

bool Token::hasKnownSymbolicValue(const Token* tok) const
{
    if (tok->exprId() == 0)
        return false;
    return mImpl->mValues &&
           std::any_of(mImpl->mValues->begin(), mImpl->mValues->end(), [&](const ValueFlow::Value& value) {
        return value.isKnown() && value.isSymbolicValue() && value.tokvalue &&
               value.tokvalue->exprId() == tok->exprId();
    });
}

const ValueFlow::Value* Token::getKnownValue(ValueFlow::Value::ValueType t) const
{
    if (!mImpl->mValues)
        return nullptr;
    if (mImpl->mValues->empty())
        return nullptr;
    // known INT values are always the first entry
    if (t == ValueFlow::Value::ValueType::INT) {
        const auto& v = mImpl->mValues->front();
        if (!v.isKnown() || !v.isIntValue())
            return nullptr;
        return &v;
    }
    auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [&](const ValueFlow::Value& value) {
        return value.isKnown() && value.valueType == t;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

const ValueFlow::Value* Token::getValue(const MathLib::bigint val) const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [=](const ValueFlow::Value& value) {
        return value.isIntValue() && !value.isImpossible() && value.intvalue == val;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

template<class Compare>
static const ValueFlow::Value* getCompareValue(const std::list<ValueFlow::Value>& values,
                                               bool condition,
                                               MathLib::bigint path,
                                               Compare compare)
{
    const ValueFlow::Value* ret = nullptr;
    for (const ValueFlow::Value& value : values) {
        if (!value.isIntValue())
            continue;
        if (value.isImpossible())
            continue;
        if (path > -0 && value.path != 0 && value.path != path)
            continue;
        if ((!ret || compare(value.intvalue, ret->intvalue)) && ((value.condition != nullptr) == condition))
            ret = &value;
    }
    return ret;
}

const ValueFlow::Value* Token::getMaxValue(bool condition, MathLib::bigint path) const
{
    if (!mImpl->mValues)
        return nullptr;
    return getCompareValue(*mImpl->mValues, condition, path, std::greater<MathLib::bigint>{});
}

const ValueFlow::Value* Token::getMinValue(bool condition, MathLib::bigint path) const
{
    if (!mImpl->mValues)
        return nullptr;
    return getCompareValue(*mImpl->mValues, condition, path, std::less<MathLib::bigint>{});
}

const ValueFlow::Value* Token::getMovedValue() const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [](const ValueFlow::Value& value) {
        return value.isMovedValue() && !value.isImpossible() &&
               value.moveKind != ValueFlow::Value::MoveKind::NonMovedVariable;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

const ValueFlow::Value* Token::getContainerSizeValue(const MathLib::bigint val) const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [=](const ValueFlow::Value& value) {
        return value.isContainerSizeValue() && !value.isImpossible() && value.intvalue == val;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

Token::Impl::~Impl()
{
    delete mMacroName;
    delete mOriginalName;
    delete mValueType;
    delete mValues;

    if (mTemplateSimplifierPointers) {
        for (auto *p : *mTemplateSimplifierPointers) {
            p->token(nullptr);
        }
    }
    delete mTemplateSimplifierPointers;

    while (mCppcheckAttributes) {
        CppcheckAttributes *c = mCppcheckAttributes;
        mCppcheckAttributes = mCppcheckAttributes->next;
        delete c;
    }
}

void Token::Impl::setCppcheckAttribute(CppcheckAttributesType type, MathLib::bigint value)
{
    CppcheckAttributes *attr = mCppcheckAttributes;
    while (attr && attr->type != type)
        attr = attr->next;
    if (attr)
        attr->value = value;
    else {
        attr = new CppcheckAttributes;
        attr->type = type;
        attr->value = value;
        attr->next = mCppcheckAttributes;
        mCppcheckAttributes = attr;
    }
}

bool Token::Impl::getCppcheckAttribute(CppcheckAttributesType type, MathLib::bigint &value) const
{
    const CppcheckAttributes *attr = mCppcheckAttributes;
    while (attr && attr->type != type)
        attr = attr->next;
    if (attr)
        value = attr->value;
    return attr != nullptr;
}

Token* findTypeEnd(Token* tok)
{
    while (Token::Match(tok, "%name%|.|::|*|&|&&|<|(|template|decltype|sizeof")) {
        if (Token::Match(tok, "(|<"))
            tok = tok->link();
        if (!tok)
            return nullptr;
        tok = tok->next();
    }
    return tok;
}

Token* findLambdaEndScope(Token* tok)
{
    if (!Token::simpleMatch(tok, "["))
        return nullptr;
    tok = tok->link();
    if (!Token::Match(tok, "] (|{"))
        return nullptr;
    tok = tok->linkAt(1);
    if (Token::simpleMatch(tok, "}"))
        return tok;
    if (Token::simpleMatch(tok, ") {"))
        return tok->linkAt(1);
    if (!Token::simpleMatch(tok, ")"))
        return nullptr;
    tok = tok->next();
    while (Token::Match(tok, "mutable|constexpr|consteval|noexcept|.")) {
        if (Token::simpleMatch(tok, "noexcept ("))
            tok = tok->linkAt(1);
        if (Token::simpleMatch(tok, ".")) {
            tok = findTypeEnd(tok);
            break;
        }
        tok = tok->next();
    }
    if (Token::simpleMatch(tok, "{"))
        return tok->link();
    return nullptr;
}
const Token* findLambdaEndScope(const Token* tok) {
    return findLambdaEndScope(const_cast<Token*>(tok));
}

void Token::templateArgFrom(const Token* fromToken) {
    setFlag(fIsTemplateArg, fromToken != nullptr);
    mImpl->mTemplateArgFileIndex = fromToken ? fromToken->mImpl->mFileIndex : -1;
    mImpl->mTemplateArgLineNumber = fromToken ? fromToken->mImpl->mLineNumber : -1;
    mImpl->mTemplateArgColumn = fromToken ? fromToken->mImpl->mColumn : -1;
}

const SmallVector<ReferenceToken>& Token::refs(bool temporary) const
{
    if (temporary) {
        if (!mImpl->mRefsTemp)
            mImpl->mRefsTemp.reset(new SmallVector<ReferenceToken>(followAllReferences(this, true)));
        return *mImpl->mRefsTemp;
    }

    if (!mImpl->mRefs)
        mImpl->mRefs.reset(new SmallVector<ReferenceToken>(followAllReferences(this, false)));
    return *mImpl->mRefs;
}

bool Token::isMutableExpr() const
{
    if (mImpl->mMutableExpr == -1)
        mImpl->mMutableExpr = isMutableExpression(this);
    return !!mImpl->mMutableExpr;
}
