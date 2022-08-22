/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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
#include "astutils.h"

#include "config.h"
#include "errortypes.h"
#include "infer.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "valueflow.h"
#include "valueptr.h"
#include "checkclass.h"

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <utility>

const Token* findExpression(const nonneg int exprid,
                            const Token* start,
                            const Token* end,
                            const std::function<bool(const Token*)>& pred)
{
    if (!precedes(start, end))
        return nullptr;
    if (exprid == 0)
        return nullptr;
    for (const Token* tok = start; tok != end; tok = tok->next()) {
        if (tok->exprId() != exprid)
            continue;
        if (pred(tok))
            return tok;
    }
    return nullptr;
}

static int findArgumentPosRecursive(const Token* tok, const Token* tokToFind,  bool &found, nonneg int depth=0)
{
    ++depth;
    if (!tok || depth >= 100)
        return -1;
    if (tok->str() == ",") {
        int res = findArgumentPosRecursive(tok->astOperand1(), tokToFind, found, depth);
        if (res == -1)
            return -1;
        if (found)
            return res;
        int argn = res;
        res = findArgumentPosRecursive(tok->astOperand2(), tokToFind, found, depth);
        if (res == -1)
            return -1;
        return argn + res;
    } else {
        if (tokToFind == tok)
            found = true;
        return 1;
    }
}

static int findArgumentPos(const Token* tok, const Token* tokToFind){
    bool found = false;
    int argn = findArgumentPosRecursive(tok, tokToFind, found, 0);
    if (found)
        return argn - 1;
    return -1;
}

static int getArgumentPos(const Token* ftok, const Token* tokToFind){
    const Token* tok = ftok;
    if (Token::Match(tok, "%name% (|{"))
        tok = ftok->next();
    if (!Token::Match(tok, "(|{|["))
        return -1;
    const Token* startTok = tok->astOperand2();
    if (!startTok && tok->next() != tok->link())
        startTok = tok->astOperand1();
    return findArgumentPos(startTok, tokToFind);
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static void astFlattenRecursive(T* tok, std::vector<T*>* result, const char* op, nonneg int depth = 0)
{
    ++depth;
    if (!tok || depth >= 100)
        return;
    if (tok->str() == op) {
        astFlattenRecursive(tok->astOperand1(), result, op, depth);
        astFlattenRecursive(tok->astOperand2(), result, op, depth);
    } else {
        result->push_back(tok);
    }
}

std::vector<const Token*> astFlatten(const Token* tok, const char* op)
{
    std::vector<const Token*> result;
    astFlattenRecursive(tok, &result, op);
    return result;
}

std::vector<Token*> astFlatten(Token* tok, const char* op)
{
    std::vector<Token*> result;
    astFlattenRecursive(tok, &result, op);
    return result;
}

nonneg int astCount(const Token* tok, const char* op, int depth)
{
    --depth;
    if (!tok || depth < 0)
        return 0;
    if (tok->str() == op)
        return astCount(tok->astOperand1(), op, depth) + astCount(tok->astOperand2(), op, depth);
    else
        return 1;
}

bool astHasToken(const Token* root, const Token * tok)
{
    if (!root)
        return false;
    while (tok->astParent() && tok != root)
        tok = tok->astParent();
    return root == tok;
}

bool astHasVar(const Token * tok, nonneg int varid)
{
    if (!tok)
        return false;
    if (tok->varId() == varid)
        return true;
    return astHasVar(tok->astOperand1(), varid) || astHasVar(tok->astOperand2(), varid);
}

static bool astIsCharWithSign(const Token *tok, ValueType::Sign sign)
{
    if (!tok)
        return false;
    const ValueType *valueType = tok->valueType();
    if (!valueType)
        return false;
    return valueType->type == ValueType::Type::CHAR && valueType->pointer == 0U && valueType->sign == sign;
}

bool astIsSignedChar(const Token *tok)
{
    return astIsCharWithSign(tok, ValueType::Sign::SIGNED);
}

bool astIsUnknownSignChar(const Token *tok)
{
    return astIsCharWithSign(tok, ValueType::Sign::UNKNOWN_SIGN);
}

bool astIsGenericChar(const Token* tok)
{
    return !astIsPointer(tok) && tok && tok->valueType() && (tok->valueType()->type == ValueType::Type::CHAR || tok->valueType()->type == ValueType::Type::WCHAR_T);
}

bool astIsPrimitive(const Token* tok)
{
    const ValueType* vt = tok ? tok->valueType() : nullptr;
    if (!vt)
        return false;
    return vt->isPrimitive();
}

bool astIsIntegral(const Token *tok, bool unknown)
{
    const ValueType *vt = tok ? tok->valueType() : nullptr;
    if (!vt)
        return unknown;
    return vt->isIntegral() && vt->pointer == 0U;
}

bool astIsUnsigned(const Token* tok)
{
    return tok && tok->valueType() && tok->valueType()->sign == ValueType::UNSIGNED;
}

bool astIsFloat(const Token *tok, bool unknown)
{
    const ValueType *vt = tok ? tok->valueType() : nullptr;
    if (!vt)
        return unknown;
    return vt->type >= ValueType::Type::FLOAT && vt->pointer == 0U;
}

bool astIsBool(const Token *tok)
{
    return tok && (tok->isBoolean() || (tok->valueType() && tok->valueType()->type == ValueType::Type::BOOL && !tok->valueType()->pointer));
}

bool astIsPointer(const Token *tok)
{
    return tok && tok->valueType() && tok->valueType()->pointer;
}

bool astIsSmartPointer(const Token* tok)
{
    return tok && tok->valueType() && tok->valueType()->smartPointerTypeToken;
}

bool astIsUniqueSmartPointer(const Token* tok)
{
    if (!astIsSmartPointer(tok))
        return false;
    if (!tok->valueType()->smartPointer)
        return false;
    return tok->valueType()->smartPointer->unique;
}

bool astIsIterator(const Token *tok)
{
    return tok && tok->valueType() && tok->valueType()->type == ValueType::Type::ITERATOR;
}

bool astIsContainer(const Token* tok) {
    return getLibraryContainer(tok) != nullptr && !astIsIterator(tok);
}

bool astIsContainerView(const Token* tok)
{
    const Library::Container* container = getLibraryContainer(tok);
    return container && !astIsIterator(tok) && container->view;
}

bool astIsContainerOwned(const Token* tok) {
    return astIsContainer(tok) && !astIsContainerView(tok);
}

static const Token* getContainerFunction(const Token* tok)
{
    if (!tok || !tok->valueType() || !tok->valueType()->container)
        return nullptr;
    const Token* parent = tok->astParent();
    if (Token::Match(parent, ". %name% (") && astIsLHS(tok)) {
        return parent->next();
    }
    return nullptr;
}

Library::Container::Action astContainerAction(const Token* tok, const Token** ftok)
{
    const Token* ftok2 = getContainerFunction(tok);
    if (ftok)
        *ftok = ftok2;
    if (!ftok2)
        return Library::Container::Action::NO_ACTION;
    return tok->valueType()->container->getAction(ftok2->str());
}
Library::Container::Yield astContainerYield(const Token* tok, const Token** ftok)
{
    const Token* ftok2 = getContainerFunction(tok);
    if (ftok)
        *ftok = ftok2;
    if (!ftok2)
        return Library::Container::Yield::NO_YIELD;
    return tok->valueType()->container->getYield(ftok2->str());
}

bool astIsRangeBasedForDecl(const Token* tok)
{
    return Token::simpleMatch(tok->astParent(), ":") && Token::simpleMatch(tok->astParent()->astParent(), "(");
}

std::string astCanonicalType(const Token *expr)
{
    if (!expr)
        return "";
    std::pair<const Token*, const Token*> decl = Token::typeDecl(expr);
    if (decl.first && decl.second) {
        std::string ret;
        for (const Token *type = decl.first; Token::Match(type,"%name%|::") && type != decl.second; type = type->next()) {
            if (!Token::Match(type, "const|static"))
                ret += type->str();
        }
        return ret;
    }
    return "";
}

static bool match(const Token *tok, const std::string &rhs)
{
    if (tok->str() == rhs)
        return true;
    if (!tok->varId() && tok->hasKnownIntValue() && MathLib::toString(tok->values().front().intvalue) == rhs)
        return true;
    return false;
}

const Token * astIsVariableComparison(const Token *tok, const std::string &comp, const std::string &rhs, const Token **vartok)
{
    if (!tok)
        return nullptr;

    const Token *ret = nullptr;
    if (tok->isComparisonOp()) {
        if (tok->astOperand1() && match(tok->astOperand1(), rhs)) {
            // Invert comparator
            std::string s = tok->str();
            if (s[0] == '>')
                s[0] = '<';
            else if (s[0] == '<')
                s[0] = '>';
            if (s == comp) {
                ret = tok->astOperand2();
            }
        } else if (tok->str() == comp && tok->astOperand2() && match(tok->astOperand2(), rhs)) {
            ret = tok->astOperand1();
        }
    } else if (comp == "!=" && rhs == std::string("0")) {
        if (tok->str() == "!") {
            ret = tok->astOperand1();
            // handle (!(x==0)) as (x!=0)
            astIsVariableComparison(ret, "==", "0", &ret);
        } else
            ret = tok;
    } else if (comp == "==" && rhs == std::string("0")) {
        if (tok->str() == "!") {
            ret = tok->astOperand1();
            // handle (!(x!=0)) as (x==0)
            astIsVariableComparison(ret, "!=", "0", &ret);
        }
    }
    while (ret && ret->str() == ".")
        ret = ret->astOperand2();
    if (ret && ret->str() == "=" && ret->astOperand1() && ret->astOperand1()->varId())
        ret = ret->astOperand1();
    else if (ret && ret->varId() == 0U)
        ret = nullptr;
    if (vartok)
        *vartok = ret;
    return ret;
}

bool isVariableDecl(const Token* tok)
{
    if (!tok)
        return false;
    const Variable* var = tok->variable();
    if (!var)
        return false;
    if (var->nameToken() == tok)
        return true;
    if (Token::Match(var->declEndToken(), "; %var%") && var->declEndToken()->next() == tok)
        return true;
    return false;
}

bool isTemporary(bool cpp, const Token* tok, const Library* library, bool unknown)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "."))
        return (tok->originalName() != "->" && isTemporary(cpp, tok->astOperand1(), library)) ||
               isTemporary(cpp, tok->astOperand2(), library);
    if (Token::Match(tok, ",|::"))
        return isTemporary(cpp, tok->astOperand2(), library);
    if (tok->isCast() || (cpp && isCPPCast(tok)))
        return isTemporary(cpp, tok->astOperand2(), library);
    if (Token::Match(tok, "?|.|[|++|--|%name%|%assign%"))
        return false;
    if (tok->isUnaryOp("*"))
        return false;
    if (Token::Match(tok, "&|<<|>>") && isLikelyStream(cpp, tok->astOperand1()))
        return false;
    if (Token::simpleMatch(tok, "(") && tok->astOperand1() &&
        (tok->astOperand2() || Token::simpleMatch(tok->next(), ")"))) {
        if (tok->valueType()) {
            return tok->valueType()->reference == Reference::None;
        }
        const Token* ftok = nullptr;
        if (Token::simpleMatch(tok->previous(), ">") && tok->previous()->link())
            ftok = tok->previous()->link()->previous();
        else
            ftok = tok->previous();
        if (!ftok)
            return false;
        if (const Function * f = ftok->function()) {
            return !Function::returnsReference(f, true);
        } else if (ftok->type()) {
            return true;
        } else if (library) {
            std::string returnType = library->returnValueType(ftok);
            return !returnType.empty() && returnType.back() != '&';
        } else {
            return unknown;
        }
    }
    if (tok->isCast())
        return false;
    // Currying a function is unknown in cppcheck
    if (Token::simpleMatch(tok, "(") && Token::simpleMatch(tok->astOperand1(), "("))
        return unknown;
    if (Token::simpleMatch(tok, "{") && Token::simpleMatch(tok->astParent(), "return") && tok->astOperand1() &&
        !tok->astOperand2())
        return isTemporary(cpp, tok->astOperand1(), library);
    return true;
}

static bool isFunctionCall(const Token* tok)
{
    if (Token::Match(tok, "%name% ("))
        return true;
    if (Token::Match(tok, "%name% <") && Token::simpleMatch(tok->next()->link(), "> ("))
        return true;
    if (Token::Match(tok, "%name% ::"))
        return isFunctionCall(tok->tokAt(2));
    return false;
}

static bool hasToken(const Token * startTok, const Token * stopTok, const Token * tok)
{
    for (const Token * tok2 = startTok; tok2 != stopTok; tok2 = tok2->next()) {
        if (tok2 == tok)
            return true;
    }
    return false;
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* previousBeforeAstLeftmostLeafGeneric(T* tok)
{
    if (!tok)
        return nullptr;
    T* leftmostLeaf = tok;
    while (leftmostLeaf->astOperand1())
        leftmostLeaf = leftmostLeaf->astOperand1();
    return leftmostLeaf->previous();
}

const Token* previousBeforeAstLeftmostLeaf(const Token* tok)
{
    return previousBeforeAstLeftmostLeafGeneric(tok);
}
Token* previousBeforeAstLeftmostLeaf(Token* tok)
{
    return previousBeforeAstLeftmostLeafGeneric(tok);
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* nextAfterAstRightmostLeafGeneric(T* tok)
{
    const Token * rightmostLeaf = tok;
    if (!rightmostLeaf || !rightmostLeaf->astOperand1())
        return nullptr;
    do {
        if (const Token* lam = findLambdaEndToken(rightmostLeaf)) {
            rightmostLeaf = lam;
            break;
        }
        if (rightmostLeaf->astOperand2() && precedes(rightmostLeaf, rightmostLeaf->astOperand2()))
            rightmostLeaf = rightmostLeaf->astOperand2();
        else if (rightmostLeaf->astOperand1() && precedes(rightmostLeaf, rightmostLeaf->astOperand1()))
            rightmostLeaf = rightmostLeaf->astOperand1();
        else
            break;
    } while (rightmostLeaf->astOperand1() || rightmostLeaf->astOperand2());
    while (Token::Match(rightmostLeaf->next(), "]|)") && !hasToken(rightmostLeaf->next()->link(), rightmostLeaf->next(), tok))
        rightmostLeaf = rightmostLeaf->next();
    if (Token::Match(rightmostLeaf, "{|(|[") && rightmostLeaf->link())
        rightmostLeaf = rightmostLeaf->link();
    return rightmostLeaf->next();
}

const Token* nextAfterAstRightmostLeaf(const Token* tok)
{
    return nextAfterAstRightmostLeafGeneric(tok);
}
Token* nextAfterAstRightmostLeaf(Token* tok)
{
    return nextAfterAstRightmostLeafGeneric(tok);
}

const Token* astParentSkipParens(const Token* tok)
{
    return astParentSkipParens(const_cast<Token*>(tok));
}
Token* astParentSkipParens(Token* tok)
{
    if (!tok)
        return nullptr;
    Token * parent = tok->astParent();
    if (!Token::simpleMatch(parent, "("))
        return parent;
    if (parent->link() != nextAfterAstRightmostLeaf(tok))
        return parent;
    if (Token::Match(parent->previous(), "%name% (") ||
        (Token::simpleMatch(parent->previous(), "> (") && parent->previous()->link()))
        return parent;
    return astParentSkipParens(parent);
}

const Token* getParentMember(const Token * tok)
{
    if (!tok)
        return tok;
    const Token * parent = tok->astParent();
    if (!Token::simpleMatch(parent, "."))
        return tok;
    if (astIsRHS(tok)) {
        if (Token::simpleMatch(parent->astOperand1(), "."))
            return parent->astOperand1()->astOperand2();
        return parent->astOperand1();
    }
    const Token * gparent = parent->astParent();
    if (!Token::simpleMatch(gparent, ".") || gparent->astOperand2() != parent)
        return tok;
    if (gparent->astOperand1())
        return gparent->astOperand1();
    return tok;
}

const Token* getParentLifetime(const Token* tok)
{
    if (!tok)
        return tok;
    // Skipping checking for variable if its a pointer-to-member
    if (!Token::simpleMatch(tok->previous(), ". *")) {
        const Variable* var = tok->variable();
        // TODO: Call getLifetimeVariable for deeper analysis
        if (!var)
            return tok;
        if (var->isLocal() || var->isArgument())
            return tok;
    }
    const Token* parent = getParentMember(tok);
    if (parent != tok)
        return getParentLifetime(parent);
    return tok;
}

static std::vector<const Token*> getParentMembers(const Token* tok)
{
    if (!tok)
        return {};
    if (!Token::simpleMatch(tok->astParent(), "."))
        return {tok};
    const Token* parent = tok;
    while (Token::simpleMatch(parent->astParent(), "."))
        parent = parent->astParent();
    std::vector<const Token*> result;
    for (const Token* tok2 : astFlatten(parent, ".")) {
        if (Token::simpleMatch(tok2, "(") && Token::simpleMatch(tok2->astOperand1(), ".")) {
            std::vector<const Token*> sub = getParentMembers(tok2->astOperand1());
            result.insert(result.end(), sub.begin(), sub.end());
        }
        result.push_back(tok2);
    }
    return result;
}

const Token* getParentLifetime(bool cpp, const Token* tok, const Library* library)
{
    std::vector<const Token*> members = getParentMembers(tok);
    if (members.size() < 2)
        return tok;
    // Find the first local variable or temporary
    auto it = std::find_if(members.rbegin(), members.rend(), [&](const Token* tok2) {
        const Variable* var = tok2->variable();
        if (var) {
            return var->isLocal() || var->isArgument();
        } else {
            return isTemporary(cpp, tok2, library);
        }
    });
    if (it == members.rend())
        return tok;
    // If any of the submembers are borrowed types then stop
    if (std::any_of(it.base() - 1, members.end() - 1, [&](const Token* tok2) {
        if (astIsPointer(tok2) || astIsContainerView(tok2) || astIsIterator(tok2))
            return true;
        if (!astIsUniqueSmartPointer(tok2)) {
            if (astIsSmartPointer(tok2))
                return true;
            const Token* dotTok = tok2->next();
            if (!Token::simpleMatch(dotTok, ".")) {
                const Token* endTok = nextAfterAstRightmostLeaf(tok2);
                if (!endTok)
                    dotTok = tok2->next();
                else if (Token::simpleMatch(endTok, "."))
                    dotTok = endTok;
                else if (Token::simpleMatch(endTok->next(), "."))
                    dotTok = endTok->next();
            }
            // If we are dereferencing the member variable then treat it as borrowed
            if (Token::simpleMatch(dotTok, ".") && dotTok->originalName() == "->")
                return true;
        }
        const Variable* var = tok2->variable();
        return var && var->isReference();
    }))
        return nullptr;
    return *it;
}

bool astIsLHS(const Token* tok)
{
    if (!tok)
        return false;
    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    if (!parent->astOperand1())
        return false;
    if (!parent->astOperand2())
        return false;
    return parent->astOperand1() == tok;
}
bool astIsRHS(const Token* tok)
{
    if (!tok)
        return false;
    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    if (!parent->astOperand1())
        return false;
    if (!parent->astOperand2())
        return false;
    return parent->astOperand2() == tok;
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* getCondTokImpl(T* tok)
{
    if (!tok)
        return nullptr;
    if (Token::simpleMatch(tok, "("))
        return getCondTok(tok->previous());
    if (Token::simpleMatch(tok, "for") && Token::simpleMatch(tok->next()->astOperand2(), ";") &&
        tok->next()->astOperand2()->astOperand2())
        return tok->next()->astOperand2()->astOperand2()->astOperand1();
    if (Token::simpleMatch(tok->next()->astOperand2(), ";"))
        return tok->next()->astOperand2()->astOperand1();
    return tok->next()->astOperand2();
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* getCondTokFromEndImpl(T* endBlock)
{
    if (!Token::simpleMatch(endBlock, "}"))
        return nullptr;
    T* startBlock = endBlock->link();
    if (!Token::simpleMatch(startBlock, "{"))
        return nullptr;
    if (Token::simpleMatch(startBlock->previous(), ")")) {
        return getCondTok(startBlock->previous()->link());
    } else if (Token::simpleMatch(startBlock->tokAt(-2), "} else {")) {
        return getCondTokFromEnd(startBlock->tokAt(-2));
    }
    return nullptr;
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* getInitTokImpl(T* tok)
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok, "%name% ("))
        return getInitTokImpl(tok->next());
    if (tok->str() != "(")
        return nullptr;
    if (!Token::simpleMatch(tok->astOperand2(), ";"))
        return nullptr;
    if (Token::simpleMatch(tok->astOperand2()->astOperand1(), ";"))
        return nullptr;
    return tok->astOperand2()->astOperand1();
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* getStepTokImpl(T* tok)
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok, "%name% ("))
        return getStepTokImpl(tok->next());
    if (tok->str() != "(")
        return nullptr;
    if (!Token::simpleMatch(tok->astOperand2(), ";"))
        return nullptr;
    if (!Token::simpleMatch(tok->astOperand2()->astOperand2(), ";"))
        return nullptr;
    return tok->astOperand2()->astOperand2()->astOperand2();
}

Token* getCondTok(Token* tok)
{
    return getCondTokImpl(tok);
}
const Token* getCondTok(const Token* tok)
{
    return getCondTokImpl(tok);
}

Token* getCondTokFromEnd(Token* endBlock)
{
    return getCondTokFromEndImpl(endBlock);
}
const Token* getCondTokFromEnd(const Token* endBlock)
{
    return getCondTokFromEndImpl(endBlock);
}

Token* getInitTok(Token* tok) {
    return getInitTokImpl(tok);
}
const Token* getInitTok(const Token* tok) {
    return getInitTokImpl(tok);
}

Token* getStepTok(Token* tok) {
    return getStepTokImpl(tok);
}
const Token* getStepTok(const Token* tok) {
    return getStepTokImpl(tok);
}

const Token *findNextTokenFromBreak(const Token *breakToken)
{
    const Scope *scope = breakToken->scope();
    while (scope) {
        if (scope->isLoopScope() || scope->type == Scope::ScopeType::eSwitch) {
            if (scope->type == Scope::ScopeType::eDo && Token::simpleMatch(scope->bodyEnd, "} while ("))
                return scope->bodyEnd->linkAt(2)->next();
            return scope->bodyEnd;
        }
        scope = scope->nestedIn;
    }
    return nullptr;
}

bool extractForLoopValues(const Token *forToken,
                          nonneg int * const varid,
                          bool * const knownInitValue,
                          MathLib::bigint * const initValue,
                          bool * const partialCond,
                          MathLib::bigint * const stepValue,
                          MathLib::bigint * const lastValue)
{
    if (!Token::simpleMatch(forToken, "for (") || !Token::simpleMatch(forToken->next()->astOperand2(), ";"))
        return false;
    const Token *initExpr = forToken->next()->astOperand2()->astOperand1();
    const Token *condExpr = forToken->next()->astOperand2()->astOperand2()->astOperand1();
    const Token *incExpr  = forToken->next()->astOperand2()->astOperand2()->astOperand2();
    if (!initExpr || !initExpr->isBinaryOp() || initExpr->str() != "=" || !Token::Match(initExpr->astOperand1(), "%var%"))
        return false;
    std::vector<MathLib::bigint> minInitValue = getMinValue(makeIntegralInferModel(), initExpr->astOperand2()->values());
    *varid = initExpr->astOperand1()->varId();
    *knownInitValue = initExpr->astOperand2()->hasKnownIntValue();
    *initValue = minInitValue.empty() ? 0 : minInitValue.front();
    *partialCond = Token::Match(condExpr, "%oror%|&&");
    visitAstNodes(condExpr, [varid, &condExpr](const Token *tok) {
        if (Token::Match(tok, "%oror%|&&"))
            return ChildrenToVisit::op1_and_op2;
        if (Token::Match(tok, "<|<=") && tok->isBinaryOp() && tok->astOperand1()->varId() == *varid && tok->astOperand2()->hasKnownIntValue()) {
            if (Token::Match(condExpr, "%oror%|&&") || tok->astOperand2()->getKnownIntValue() < condExpr->astOperand2()->getKnownIntValue())
                condExpr = tok;
        }
        return ChildrenToVisit::none;
    });
    if (!Token::Match(condExpr, "<|<=") || !condExpr->isBinaryOp() || condExpr->astOperand1()->varId() != *varid || !condExpr->astOperand2()->hasKnownIntValue())
        return false;
    if (!incExpr || !incExpr->isUnaryOp("++") || incExpr->astOperand1()->varId() != *varid)
        return false;
    *stepValue = 1;
    if (condExpr->str() == "<")
        *lastValue = condExpr->astOperand2()->getKnownIntValue() - 1;
    else
        *lastValue = condExpr->astOperand2()->getKnownIntValue();
    return true;
}


static const Token * getVariableInitExpression(const Variable * var)
{
    if (!var)
        return nullptr;
    const Token *varDeclEndToken = var->declEndToken();
    if (!varDeclEndToken)
        return nullptr;
    if (Token::Match(varDeclEndToken, "; %varid% =", var->declarationId()))
        return varDeclEndToken->tokAt(2)->astOperand2();
    return varDeclEndToken->astOperand2();
}

static bool isInLoopCondition(const Token * tok)
{
    return Token::Match(tok->astTop()->previous(), "for|while (");
}

/// If tok2 comes after tok1
bool precedes(const Token * tok1, const Token * tok2)
{
    if (tok1 == tok2)
        return false;
    if (!tok1)
        return false;
    if (!tok2)
        return true;
    return tok1->index() < tok2->index();
}

/// If tok1 comes after tok2
bool succeeds(const Token* tok1, const Token* tok2)
{
    if (tok1 == tok2)
        return false;
    if (!tok1)
        return false;
    if (!tok2)
        return true;
    return tok1->index() > tok2->index();
}

bool isAliasOf(const Token *tok, nonneg int varid, bool* inconclusive)
{
    if (tok->varId() == varid)
        return false;
    for (const ValueFlow::Value &val : tok->values()) {
        if (!val.isLocalLifetimeValue())
            continue;
        if (val.tokvalue->varId() == varid) {
            if (val.isInconclusive()) {
                if (inconclusive)
                    *inconclusive = true;
                else
                    continue;
            }
            return true;
        }
    }
    return false;
}

bool isAliasOf(const Token* tok, const Token* expr, bool* inconclusive)
{
    const bool pointer = astIsPointer(tok);
    const ValueFlow::Value* value = nullptr;
    const Token* r = findAstNode(expr, [&](const Token* childTok) {
        for (const ValueFlow::Value& val : tok->values()) {
            if (val.isImpossible())
                continue;
            if (val.isLocalLifetimeValue() || (pointer && val.isSymbolicValue() && val.intvalue == 0)) {
                if (findAstNode(val.tokvalue,
                                [&](const Token* aliasTok) {
                    return aliasTok->exprId() == childTok->exprId();
                })) {
                    if (val.isInconclusive() && inconclusive) { // NOLINT
                        value = &val;
                    } else {
                        return true;
                    }
                }
            }
        }
        return false;
    });
    if (!r && value && inconclusive)
        *inconclusive = true;
    return r || value;
}

static bool isAliased(const Token *startTok, const Token *endTok, nonneg int varid)
{
    if (!precedes(startTok, endTok))
        return false;
    for (const Token *tok = startTok; tok != endTok; tok = tok->next()) {
        if (Token::Match(tok, "= & %varid% ;", varid))
            return true;
        if (isAliasOf(tok, varid))
            return true;
    }
    return false;
}

bool isAliased(const Variable *var)
{
    if (!var)
        return false;
    if (!var->scope())
        return false;
    const Token *start = var->declEndToken();
    if (!start)
        return false;
    return isAliased(start, var->scope()->bodyEnd, var->declarationId());
}

bool exprDependsOnThis(const Token* expr, bool onVar, nonneg int depth)
{
    if (!expr)
        return false;
    if (expr->str() == "this")
        return true;
    if (depth >= 1000)
        // Abort recursion to avoid stack overflow
        return true;
    ++depth;

    // calling nonstatic method?
    if (Token::Match(expr, "%name% (") && expr->function() && expr->function()->nestedIn && expr->function()->nestedIn->isClassOrStruct() && !expr->function()->isStatic()) {
        // is it a method of this?
        const Scope* fScope = expr->scope();
        while (!fScope->functionOf && fScope->nestedIn)
            fScope = fScope->nestedIn;

        const Scope* classScope = fScope->functionOf;
        if (classScope && classScope->function)
            classScope = classScope->function->token->scope();

        if (classScope && classScope->isClassOrStruct())
            return contains(classScope->findAssociatedScopes(), expr->function()->nestedIn);
        return false;
    } else if (onVar && expr->variable()) {
        const Variable* var = expr->variable();
        return (var->isPrivate() || var->isPublic() || var->isProtected());
    }
    if (Token::simpleMatch(expr, "."))
        return exprDependsOnThis(expr->astOperand1(), onVar, depth);
    return exprDependsOnThis(expr->astOperand1(), onVar, depth) || exprDependsOnThis(expr->astOperand2(), onVar, depth);
}

static bool hasUnknownVars(const Token* startTok)
{
    bool result = false;
    visitAstNodes(startTok, [&](const Token* tok) {
        if (tok->varId() > 0 && !tok->variable()) {
            result = true;
            return ChildrenToVisit::done;
        }
        return ChildrenToVisit::op1_and_op2;
    });
    return result;
}

bool isStructuredBindingVariable(const Variable* var)
{
    if (!var)
        return false;
    const Token* tok = var->nameToken();
    while (tok && Token::Match(tok->astParent(), "[|,|:"))
        tok = tok->astParent();
    return tok && (tok->str() == "[" || Token::simpleMatch(tok->previous(), "] :")); // TODO: remove workaround when #11105 is fixed
}

/// This takes a token that refers to a variable and it will return the token
/// to the expression that the variable is assigned to. If its not valid to
/// make such substitution then it will return the original token.
static const Token * followVariableExpression(const Token * tok, bool cpp, const Token * end = nullptr)
{
    if (!tok)
        return tok;
    // Skip following variables that is across multiple files
    if (end && end->fileIndex() != tok->fileIndex())
        return tok;
    // Skip array access
    if (Token::Match(tok, "%var% ["))
        return tok;
    // Skip pointer indirection
    if (tok->astParent() && tok->isUnaryOp("*"))
        return tok;
    // Skip following variables if it is used in an assignment
    if (Token::Match(tok->next(), "%assign%"))
        return tok;
    const Variable * var = tok->variable();
    const Token * varTok = getVariableInitExpression(var);
    if (!varTok)
        return tok;
    if (hasUnknownVars(varTok))
        return tok;
    if (var->isVolatile())
        return tok;
    if (!var->isLocal() && !var->isConst())
        return tok;
    if (var->isStatic() && !var->isConst())
        return tok;
    if (var->isArgument())
        return tok;
    if (isStructuredBindingVariable(var))
        return tok;
    // assigning a floating point value to an integer does not preserve the value
    if (var->valueType() && var->valueType()->isIntegral() && varTok->valueType() && varTok->valueType()->isFloat())
        return tok;
    const Token * lastTok = precedes(tok, end) ? end : tok;
    // If this is in a loop then check if variables are modified in the entire scope
    const Token * endToken = (isInLoopCondition(tok) || isInLoopCondition(varTok) || var->scope() != tok->scope()) ? var->scope()->bodyEnd : lastTok;
    if (!var->isConst() && (!precedes(varTok, endToken) || isVariableChanged(varTok, endToken, tok->varId(), false, nullptr, cpp)))
        return tok;
    if (precedes(varTok, endToken) && isAliased(varTok, endToken, tok->varId()))
        return tok;
    const Token* startToken = nextAfterAstRightmostLeaf(varTok);
    if (!startToken)
        startToken = varTok;
    if (varTok->exprId() == 0) {
        if (!varTok->isLiteral())
            return tok;
    } else if (!precedes(startToken, endToken)) {
        return tok;
    } else if (isExpressionChanged(varTok, startToken, endToken, nullptr, cpp)) {
        return tok;
    }
    return varTok;
}

static void followVariableExpressionError(const Token *tok1, const Token *tok2, ErrorPath* errors)
{
    if (!errors)
        return;
    if (!tok1)
        return;
    if (!tok2)
        return;
    ErrorPathItem item = std::make_pair(tok2, "'" + tok1->str() + "' is assigned value '" + tok2->expressionString() + "' here.");
    if (std::find(errors->begin(), errors->end(), item) != errors->end())
        return;
    errors->push_back(item);
}

std::vector<ReferenceToken> followAllReferences(const Token* tok,
                                                bool temporary,
                                                bool inconclusive,
                                                ErrorPath errors,
                                                int depth)
{
    struct ReferenceTokenLess {
        bool operator()(const ReferenceToken& x, const ReferenceToken& y) const {
            return x.token < y.token;
        }
    };
    if (!tok)
        return std::vector<ReferenceToken> {};
    if (depth < 0)
        return {{tok, std::move(errors)}};
    const Variable *var = tok->variable();
    if (var && var->declarationId() == tok->varId()) {
        if (var->nameToken() == tok || isStructuredBindingVariable(var)) {
            return {{tok, std::move(errors)}};
        } else if (var->isReference() || var->isRValueReference()) {
            if (!var->declEndToken())
                return {{tok, std::move(errors)}};
            if (var->isArgument()) {
                errors.emplace_back(var->declEndToken(), "Passed to reference.");
                return {{tok, std::move(errors)}};
            } else if (Token::simpleMatch(var->declEndToken(), "=")) {
                if (astHasToken(var->declEndToken(), tok))
                    return std::vector<ReferenceToken>{};
                errors.emplace_back(var->declEndToken(), "Assigned to reference.");
                const Token *vartok = var->declEndToken()->astOperand2();
                if (vartok == tok || (!temporary && isTemporary(true, vartok, nullptr, true) &&
                                      (var->isConst() || var->isRValueReference())))
                    return {{tok, std::move(errors)}};
                if (vartok)
                    return followAllReferences(vartok, temporary, inconclusive, std::move(errors), depth - 1);
            } else {
                return {{tok, std::move(errors)}};
            }
        }
    } else if (Token::simpleMatch(tok, "?") && Token::simpleMatch(tok->astOperand2(), ":")) {
        std::set<ReferenceToken, ReferenceTokenLess> result;
        const Token* tok2 = tok->astOperand2();

        std::vector<ReferenceToken> refs;
        refs = followAllReferences(tok2->astOperand1(), temporary, inconclusive, errors, depth - 1);
        result.insert(refs.begin(), refs.end());
        refs = followAllReferences(tok2->astOperand2(), temporary, inconclusive, errors, depth - 1);
        result.insert(refs.begin(), refs.end());

        if (!inconclusive && result.size() != 1)
            return {{tok, std::move(errors)}};

        if (!result.empty())
            return std::vector<ReferenceToken>(result.begin(), result.end());

    } else if (Token::Match(tok->previous(), "%name% (")) {
        const Function *f = tok->previous()->function();
        if (f) {
            if (!Function::returnsReference(f))
                return {{tok, std::move(errors)}};
            std::set<ReferenceToken, ReferenceTokenLess> result;
            std::vector<const Token*> returns = Function::findReturns(f);
            for (const Token* returnTok : returns) {
                if (returnTok == tok)
                    continue;
                for (const ReferenceToken& rt :
                     followAllReferences(returnTok, temporary, inconclusive, errors, depth - returns.size())) {
                    const Variable* argvar = rt.token->variable();
                    if (!argvar)
                        return {{tok, std::move(errors)}};
                    if (argvar->isArgument() && (argvar->isReference() || argvar->isRValueReference())) {
                        int n = getArgumentPos(argvar, f);
                        if (n < 0)
                            return {{tok, std::move(errors)}};
                        std::vector<const Token*> args = getArguments(tok->previous());
                        if (n >= args.size())
                            return {{tok, std::move(errors)}};
                        const Token* argTok = args[n];
                        ErrorPath er = errors;
                        er.emplace_back(returnTok, "Return reference.");
                        er.emplace_back(tok->previous(), "Called function passing '" + argTok->expressionString() + "'.");
                        std::vector<ReferenceToken> refs =
                            followAllReferences(argTok, temporary, inconclusive, std::move(er), depth - returns.size());
                        result.insert(refs.begin(), refs.end());
                        if (!inconclusive && result.size() > 1)
                            return {{tok, std::move(errors)}};
                    }
                }
            }
            if (!result.empty())
                return std::vector<ReferenceToken>(result.begin(), result.end());
        }
    }
    return {{tok, std::move(errors)}};
}

const Token* followReferences(const Token* tok, ErrorPath* errors)
{
    if (!tok)
        return nullptr;
    std::vector<ReferenceToken> refs = followAllReferences(tok, true, false);
    if (refs.size() == 1) {
        if (errors)
            *errors = refs.front().errors;
        return refs.front().token;
    }
    return nullptr;
}

static bool isSameLifetime(const Token * const tok1, const Token * const tok2)
{
    ValueFlow::Value v1 = getLifetimeObjValue(tok1);
    ValueFlow::Value v2 = getLifetimeObjValue(tok2);
    if (!v1.isLifetimeValue() || !v2.isLifetimeValue())
        return false;
    return v1.tokvalue == v2.tokvalue;
}

static bool compareKnownValue(const Token * const tok1, const Token * const tok2, std::function<bool(const ValueFlow::Value&, const ValueFlow::Value&, bool)> compare)
{
    static const auto isKnownFn = std::mem_fn(&ValueFlow::Value::isKnown);

    const auto v1 = std::find_if(tok1->values().begin(), tok1->values().end(), isKnownFn);
    if (v1 == tok1->values().end()) {
        return false;
    }
    if (v1->isNonValue() || v1->isContainerSizeValue() || v1->isSymbolicValue())
        return false;
    const auto v2 = std::find_if(tok2->values().begin(), tok2->values().end(), isKnownFn);
    if (v2 == tok2->values().end()) {
        return false;
    }
    if (v1->valueType != v2->valueType) {
        return false;
    }
    const bool sameLifetime = isSameLifetime(tok1, tok2);
    return compare(*v1, *v2, sameLifetime);
}

bool isEqualKnownValue(const Token * const tok1, const Token * const tok2)
{
    return compareKnownValue(tok1, tok2, [&](const ValueFlow::Value& v1, const ValueFlow::Value& v2, bool sameLifetime) {
        bool r = v1.equalValue(v2);
        if (v1.isIteratorValue()) {
            r &= sameLifetime;
        }
        return r;
    });
}

static inline bool isDifferentKnownValues(const Token * const tok1, const Token * const tok2)
{
    return compareKnownValue(tok1, tok2, [&](const ValueFlow::Value& v1, const ValueFlow::Value& v2, bool sameLifetime) {
        bool r = v1.equalValue(v2);
        if (v1.isIteratorValue()) {
            r &= sameLifetime;
        }
        return !r;
    });
}

static inline bool isSameConstantValue(bool macro, const Token* tok1, const Token* tok2)
{
    if (tok1 == nullptr || tok2 == nullptr)
        return false;

    auto adjustForCast = [](const Token* tok) {
        if (Token::Match(tok->previous(), "%type% (|{") && tok->previous()->isStandardType() && tok->astOperand2())
            return tok->astOperand2();
        return tok;
    };
    tok1 = adjustForCast(tok1);
    tok2 = adjustForCast(tok2);

    if (!tok1->isNumber() || !tok2->isNumber())
        return false;

    if (macro && (tok1->isExpandedMacro() || tok2->isExpandedMacro() || tok1->isTemplateArg() || tok2->isTemplateArg()))
        return false;

    const ValueType * v1 = tok1->valueType();
    const ValueType * v2 = tok2->valueType();

    if (!v1 || !v2 || v1->sign != v2->sign || v1->type != v2->type || v1->pointer != v2->pointer)
        return false;

    return isEqualKnownValue(tok1, tok2);
}


static bool isForLoopCondition(const Token * const tok)
{
    if (!tok)
        return false;
    const Token *const parent = tok->astParent();
    return Token::simpleMatch(parent, ";") && parent->astOperand1() == tok &&
           Token::simpleMatch(parent->astParent(), ";") &&
           Token::simpleMatch(parent->astParent()->astParent(), "(") &&
           parent->astParent()->astParent()->astOperand1()->str() == "for";
}

static bool isZeroConstant(const Token *tok)
{
    while (tok && tok->isCast())
        tok = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
    return Token::simpleMatch(tok, "0") && !tok->isExpandedMacro();
}

/**
 * Is token used a boolean (cast to a bool, or used as a condition somewhere)
 * @param tok the token to check
 * @param checkingParent true if we are checking a parent. This is used to know
 * what we are checking. For instance in `if (i == 2)`, isUsedAsBool("==") is
 * true whereas isUsedAsBool("i") is false, but it might call
 * isUsedAsBool_internal("==") which must not return true
 */
static bool isUsedAsBool_internal(const Token * const tok, bool checkingParent)
{
    if (!tok)
        return false;
    const Token::Type type = tok->tokType();
    if (type == Token::eBitOp || type == Token::eIncDecOp || (type == Token::eArithmeticalOp && !tok->isUnaryOp("*")))
        // those operators don't return a bool
        return false;
    if (type == Token::eComparisonOp) {
        if (!checkingParent)
            // this operator returns a bool
            return true;
        if (Token::Match(tok, "==|!="))
            return isZeroConstant(tok->astOperand1()) || isZeroConstant(tok->astOperand2());
        return false;
    }
    if (type == Token::eLogicalOp)
        return true;
    if (astIsBool(tok))
        return true;

    const Token * const parent = tok->astParent();
    if (!parent)
        return false;
    if (parent->str() == "(" && parent->astOperand2() == tok) {
        if (Token::Match(parent->astOperand1(), "if|while"))
            return true;

        if (!parent->isCast()) { // casts are handled via the recursive call, as astIsBool will be true
            // is it a call to a function ?
            int argnr;
            const Token *const func = getTokenArgumentFunction(tok, argnr);
            if (!func || !func->function())
                return false;
            const Variable *var = func->function()->getArgumentVar(argnr);
            return var && (var->getTypeName() == "bool");
        }
    } else if (isForLoopCondition(tok))
        return true;
    else if (Token::simpleMatch(parent, "?") && astIsLHS(tok))
        return true;

    return isUsedAsBool_internal(parent, true);
}

bool isUsedAsBool(const Token * const tok)
{
    return isUsedAsBool_internal(tok, false);
}

static bool astIsBoolLike(const Token* tok)
{
    return astIsBool(tok) || isUsedAsBool(tok);
}

bool isSameExpression(bool cpp, bool macro, const Token *tok1, const Token *tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors)
{
    if (tok1 == nullptr && tok2 == nullptr)
        return true;
    if (tok1 == nullptr || tok2 == nullptr)
        return false;
    if (cpp) {
        if (tok1->str() == "." && tok1->astOperand1() && tok1->astOperand1()->str() == "this")
            tok1 = tok1->astOperand2();
        if (tok2->str() == "." && tok2->astOperand1() && tok2->astOperand1()->str() == "this")
            tok2 = tok2->astOperand2();
    }
    // Skip double not
    if (Token::simpleMatch(tok1, "!") && Token::simpleMatch(tok1->astOperand1(), "!") && !Token::simpleMatch(tok1->astParent(), "=")) {
        return isSameExpression(cpp, macro, tok1->astOperand1()->astOperand1(), tok2, library, pure, followVar, errors);
    }
    if (Token::simpleMatch(tok2, "!") && Token::simpleMatch(tok2->astOperand1(), "!") && !Token::simpleMatch(tok2->astParent(), "=")) {
        return isSameExpression(cpp, macro, tok1, tok2->astOperand1()->astOperand1(), library, pure, followVar, errors);
    }
    const bool tok_str_eq = tok1->str() == tok2->str();
    if (!tok_str_eq && isDifferentKnownValues(tok1, tok2))
        return false;
    if (isSameConstantValue(macro, tok1, tok2))
        return true;

    // Follow variable
    if (followVar && !tok_str_eq && (tok1->varId() || tok2->varId())) {
        const Token * varTok1 = followVariableExpression(tok1, cpp, tok2);
        if ((varTok1->str() == tok2->str()) || isSameConstantValue(macro, varTok1, tok2)) {
            followVariableExpressionError(tok1, varTok1, errors);
            return isSameExpression(cpp, macro, varTok1, tok2, library, true, followVar, errors);
        }
        const Token * varTok2 = followVariableExpression(tok2, cpp, tok1);
        if ((tok1->str() == varTok2->str()) || isSameConstantValue(macro, tok1, varTok2)) {
            followVariableExpressionError(tok2, varTok2, errors);
            return isSameExpression(cpp, macro, tok1, varTok2, library, true, followVar, errors);
        }
        if ((varTok1->str() == varTok2->str()) || isSameConstantValue(macro, varTok1, varTok2)) {
            followVariableExpressionError(tok1, varTok1, errors);
            followVariableExpressionError(tok2, varTok2, errors);
            return isSameExpression(cpp, macro, varTok1, varTok2, library, true, followVar, errors);
        }
    }
    // Follow references
    if (!tok_str_eq) {
        const Token* refTok1 = followReferences(tok1, errors);
        const Token* refTok2 = followReferences(tok2, errors);
        if (refTok1 != tok1 || refTok2 != tok2)
            return isSameExpression(cpp, macro, refTok1, refTok2, library, pure, followVar, errors);
    }
    if (tok1->varId() != tok2->varId() || !tok_str_eq || tok1->originalName() != tok2->originalName()) {
        if ((Token::Match(tok1,"<|>") && Token::Match(tok2,"<|>")) ||
            (Token::Match(tok1,"<=|>=") && Token::Match(tok2,"<=|>="))) {
            return isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand2(), library, pure, followVar, errors) &&
                   isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand1(), library, pure, followVar, errors);
        }
        const Token* condTok = nullptr;
        const Token* exprTok = nullptr;
        if (Token::Match(tok1, "==|!=")) {
            condTok = tok1;
            exprTok = tok2;
        } else if (Token::Match(tok2, "==|!=")) {
            condTok = tok2;
            exprTok = tok1;
        }
        if (condTok && condTok->astOperand1() && condTok->astOperand2() && !Token::Match(exprTok, "%comp%")) {
            const Token* varTok1 = nullptr;
            const Token* varTok2 = exprTok;
            const ValueFlow::Value* value = nullptr;
            if (condTok->astOperand1()->hasKnownIntValue()) {
                value = &condTok->astOperand1()->values().front();
                varTok1 = condTok->astOperand2();
            } else if (condTok->astOperand2()->hasKnownIntValue()) {
                value = &condTok->astOperand2()->values().front();
                varTok1 = condTok->astOperand1();
            }
            if (Token::simpleMatch(exprTok, "!"))
                varTok2 = exprTok->astOperand1();
            bool compare = false;
            if (value) {
                if (value->intvalue == 0 && Token::simpleMatch(exprTok, "!") && Token::simpleMatch(condTok, "==")) {
                    compare = true;
                } else if (value->intvalue == 0 && !Token::simpleMatch(exprTok, "!") && Token::simpleMatch(condTok, "!=")) {
                    compare = true;
                } else if (value->intvalue != 0 && Token::simpleMatch(exprTok, "!") && Token::simpleMatch(condTok, "!=")) {
                    compare = true;
                } else if (value->intvalue != 0 && !Token::simpleMatch(exprTok, "!") && Token::simpleMatch(condTok, "==")) {
                    compare = true;
                }

            }
            if (compare && astIsBoolLike(varTok1) && astIsBoolLike(varTok2))
                return isSameExpression(cpp, macro, varTok1, varTok2, library, pure, followVar, errors);

        }
        return false;
    }
    auto flagsDiffer = [](const Token* tok1, const Token* tok2, bool macro) {
        if (macro && (tok1->isExpandedMacro() || tok2->isExpandedMacro() || tok1->isTemplateArg() || tok2->isTemplateArg()))
            return true;
        if (tok1->isComplex() != tok2->isComplex())
            return true;
        if (tok1->isLong() != tok2->isLong())
            return true;
        if (tok1->isUnsigned() != tok2->isUnsigned())
            return true;
        if (tok1->isSigned() != tok2->isSigned())
            return true;
        return false;
    };
    if (flagsDiffer(tok1, tok2, macro))
        return false;

    if (pure && tok1->isName() && tok1->next()->str() == "(" && tok1->str() != "sizeof" && !(tok1->variable() && tok1 == tok1->variable()->nameToken())) {
        if (!tok1->function()) {
            if (Token::simpleMatch(tok1->previous(), ".")) {
                const Token *lhs = tok1->previous();
                while (Token::Match(lhs, "(|.|["))
                    lhs = lhs->astOperand1();
                if (!lhs)
                    return false;
                const bool lhsIsConst = (lhs->variable() && lhs->variable()->isConst()) ||
                                        (lhs->valueType() && lhs->valueType()->constness > 0) ||
                                        (Token::Match(lhs, "%var% . %name% (") && library.isFunctionConst(lhs->tokAt(2)));
                if (!lhsIsConst)
                    return false;
            } else {
                const Token * ftok = tok1;
                if (Token::simpleMatch(tok1->previous(), "::"))
                    ftok = tok1->previous();
                if (!library.isFunctionConst(ftok) && !ftok->isAttributeConst() && !ftok->isAttributePure())
                    return false;
            }
        } else {
            if (tok1->function() && !tok1->function()->isConst() && !tok1->function()->isAttributeConst() && !tok1->function()->isAttributePure())
                return false;
        }
    }
    // templates/casts
    if ((Token::Match(tok1, "%name% <") && tok1->next()->link()) ||
        (Token::Match(tok2, "%name% <") && tok2->next()->link())) {

        // non-const template function that is not a dynamic_cast => return false
        if (pure && Token::simpleMatch(tok1->next()->link(), "> (") &&
            !(tok1->function() && tok1->function()->isConst()) &&
            tok1->str() != "dynamic_cast")
            return false;

        // some template/cast stuff.. check that the template arguments are same
        const Token *t1 = tok1->next();
        const Token *t2 = tok2->next();
        const Token *end1 = t1->link();
        const Token *end2 = t2->link();
        while (t1 && t2 && t1 != end1 && t2 != end2) {
            if (t1->str() != t2->str() || flagsDiffer(t1, t2, macro))
                return false;
            t1 = t1->next();
            t2 = t2->next();
        }
        if (t1 != end1 || t2 != end2)
            return false;
    }
    if (tok1->tokType() == Token::eIncDecOp || tok1->isAssignmentOp())
        return false;
    // bailout when we see ({..})
    if (tok1->str() == "{")
        return false;
    // cast => assert that the casts are equal
    if (tok1->str() == "(" && tok1->previous() &&
        !tok1->previous()->isName() &&
        !(tok1->previous()->str() == ">" && tok1->previous()->link())) {
        const Token *t1 = tok1->next();
        const Token *t2 = tok2->next();
        while (t1 && t2 &&
               t1->str() == t2->str() &&
               !flagsDiffer(t1, t2, macro) &&
               (t1->isName() || t1->str() == "*")) {
            t1 = t1->next();
            t2 = t2->next();
        }
        if (!t1 || !t2 || t1->str() != ")" || t2->str() != ")")
            return false;
    }
    bool noncommutativeEquals =
        isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand1(), library, pure, followVar, errors);
    noncommutativeEquals = noncommutativeEquals &&
                           isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand2(), library, pure, followVar, errors);

    if (noncommutativeEquals)
        return true;

    // in c++, a+b might be different to b+a, depending on the type of a and b
    if (cpp && tok1->str() == "+" && tok1->isBinaryOp()) {
        const ValueType* vt1 = tok1->astOperand1()->valueType();
        const ValueType* vt2 = tok1->astOperand2()->valueType();
        if (!(vt1 && (vt1->type >= ValueType::VOID || vt1->pointer) && vt2 && (vt2->type >= ValueType::VOID || vt2->pointer)))
            return false;
    }

    const bool commutative = tok1->isBinaryOp() && Token::Match(tok1, "%or%|%oror%|+|*|&|&&|^|==|!=");
    bool commutativeEquals = commutative &&
                             isSameExpression(cpp, macro, tok1->astOperand2(), tok2->astOperand1(), library, pure, followVar, errors);
    commutativeEquals = commutativeEquals &&
                        isSameExpression(cpp, macro, tok1->astOperand1(), tok2->astOperand2(), library, pure, followVar, errors);


    return commutativeEquals;
}

static bool isZeroBoundCond(const Token * const cond)
{
    if (cond == nullptr)
        return false;
    // Assume unsigned
    // TODO: Handle reverse conditions
    const bool isZero = cond->astOperand2()->getValue(0);
    if (cond->str() == "==" || cond->str() == ">=")
        return isZero;
    if (cond->str() == "<=")
        return true;
    if (cond->str() == "<")
        return !isZero;
    if (cond->str() == ">")
        return false;
    return false;
}

bool isOppositeCond(bool isNot, bool cpp, const Token * const cond1, const Token * const cond2, const Library& library, bool pure, bool followVar, ErrorPath* errors)
{
    if (!cond1 || !cond2)
        return false;

    if (!isNot && cond1->str() == "&&" && cond2->str() == "&&") {
        for (const Token* tok1: {
            cond1->astOperand1(), cond1->astOperand2()
        }) {
            for (const Token* tok2: {
                cond2->astOperand1(), cond2->astOperand2()
            }) {
                if (isSameExpression(cpp, true, tok1, tok2, library, pure, followVar, errors)) {
                    if (isOppositeCond(isNot, cpp, tok1->astSibling(), tok2->astSibling(), library, pure, followVar, errors))
                        return true;
                }
            }
        }
    }

    if (cond1->str() == "!") {
        if (cond2->str() == "!=") {
            if (cond2->astOperand1() && cond2->astOperand1()->str() == "0")
                return isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand2(), library, pure, followVar, errors);
            if (cond2->astOperand2() && cond2->astOperand2()->str() == "0")
                return isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure, followVar, errors);
        }
        if (!isUsedAsBool(cond2))
            return false;
        return isSameExpression(cpp, true, cond1->astOperand1(), cond2, library, pure, followVar, errors);
    }

    if (cond2->str() == "!")
        return isOppositeCond(isNot, cpp, cond2, cond1, library, pure, followVar, errors);

    if (!isNot) {
        if (cond1->str() == "==" && cond2->str() == "==") {
            if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure, followVar, errors))
                return isDifferentKnownValues(cond1->astOperand2(), cond2->astOperand2());
            if (isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand2(), library, pure, followVar, errors))
                return isDifferentKnownValues(cond1->astOperand1(), cond2->astOperand1());
        }
        // TODO: Handle reverse conditions
        if (Library::isContainerYield(cond1, Library::Container::Yield::EMPTY, "empty") &&
            Library::isContainerYield(cond2->astOperand1(), Library::Container::Yield::SIZE, "size") &&
            isSameExpression(cpp,
                             true,
                             cond1->astOperand1()->astOperand1(),
                             cond2->astOperand1()->astOperand1()->astOperand1(),
                             library,
                             pure,
                             followVar,
                             errors)) {
            return !isZeroBoundCond(cond2);
        }

        if (Library::isContainerYield(cond2, Library::Container::Yield::EMPTY, "empty") &&
            Library::isContainerYield(cond1->astOperand1(), Library::Container::Yield::SIZE, "size") &&
            isSameExpression(cpp,
                             true,
                             cond2->astOperand1()->astOperand1(),
                             cond1->astOperand1()->astOperand1()->astOperand1(),
                             library,
                             pure,
                             followVar,
                             errors)) {
            return !isZeroBoundCond(cond1);
        }
    }


    if (!cond1->isComparisonOp() || !cond2->isComparisonOp())
        return false;

    const std::string &comp1 = cond1->str();

    // condition found .. get comparator
    std::string comp2;
    if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand1(), library, pure, followVar, errors) &&
        isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand2(), library, pure, followVar, errors)) {
        comp2 = cond2->str();
    } else if (isSameExpression(cpp, true, cond1->astOperand1(), cond2->astOperand2(), library, pure, followVar, errors) &&
               isSameExpression(cpp, true, cond1->astOperand2(), cond2->astOperand1(), library, pure, followVar, errors)) {
        comp2 = cond2->str();
        if (comp2[0] == '>')
            comp2[0] = '<';
        else if (comp2[0] == '<')
            comp2[0] = '>';
    }

    if (!isNot && comp2.empty()) {
        const Token *expr1 = nullptr, *value1 = nullptr, *expr2 = nullptr, *value2 = nullptr;
        std::string op1 = cond1->str(), op2 = cond2->str();
        if (cond1->astOperand2()->hasKnownIntValue()) {
            expr1 = cond1->astOperand1();
            value1 = cond1->astOperand2();
        } else if (cond1->astOperand1()->hasKnownIntValue()) {
            expr1 = cond1->astOperand2();
            value1 = cond1->astOperand1();
            if (op1[0] == '>')
                op1[0] = '<';
            else if (op1[0] == '<')
                op1[0] = '>';
        }
        if (cond2->astOperand2()->hasKnownIntValue()) {
            expr2 = cond2->astOperand1();
            value2 = cond2->astOperand2();
        } else if (cond2->astOperand1()->hasKnownIntValue()) {
            expr2 = cond2->astOperand2();
            value2 = cond2->astOperand1();
            if (op2[0] == '>')
                op2[0] = '<';
            else if (op2[0] == '<')
                op2[0] = '>';
        }
        if (!expr1 || !value1 || !expr2 || !value2) {
            return false;
        }
        if (!isSameExpression(cpp, true, expr1, expr2, library, pure, followVar, errors))
            return false;

        const ValueFlow::Value &rhsValue1 = value1->values().front();
        const ValueFlow::Value &rhsValue2 = value2->values().front();

        if (op1 == "<" || op1 == "<=")
            return (op2 == "==" || op2 == ">" || op2 == ">=") && (rhsValue1.intvalue < rhsValue2.intvalue);
        else if (op1 == ">=" || op1 == ">")
            return (op2 == "==" || op2 == "<" || op2 == "<=") && (rhsValue1.intvalue > rhsValue2.intvalue);

        return false;
    }

    // is condition opposite?
    return ((comp1 == "==" && comp2 == "!=") ||
            (comp1 == "!=" && comp2 == "==") ||
            (comp1 == "<" && comp2 == ">=") ||
            (comp1 == "<=" && comp2 == ">") ||
            (comp1 == ">" && comp2 == "<=") ||
            (comp1 == ">=" && comp2 == "<") ||
            (!isNot && ((comp1 == "<" && comp2 == ">") ||
                        (comp1 == ">" && comp2 == "<") ||
                        (comp1 == "==" && (comp2 == "!=" || comp2 == ">" || comp2 == "<")) ||
                        ((comp1 == "!=" || comp1 == ">" || comp1 == "<") && comp2 == "==")
                        )));
}

bool isOppositeExpression(bool cpp, const Token * const tok1, const Token * const tok2, const Library& library, bool pure, bool followVar, ErrorPath* errors)
{
    if (!tok1 || !tok2)
        return false;
    if (isOppositeCond(true, cpp, tok1, tok2, library, pure, followVar, errors))
        return true;
    if (tok1->isUnaryOp("-") && !(tok2->astParent() && tok2->astParent()->tokType() == Token::eBitOp))
        return isSameExpression(cpp, true, tok1->astOperand1(), tok2, library, pure, followVar, errors);
    if (tok2->isUnaryOp("-") && !(tok2->astParent() && tok2->astParent()->tokType() == Token::eBitOp))
        return isSameExpression(cpp, true, tok2->astOperand1(), tok1, library, pure, followVar, errors);
    return false;
}

static bool functionModifiesArguments(const Function* f)
{
    return std::any_of(f->argumentList.begin(), f->argumentList.end(), [](const Variable& var) {
        if (var.isReference() || var.isPointer())
            return !var.isConst();
        return true;
    });
}

bool isConstFunctionCall(const Token* ftok, const Library& library)
{
    if (!Token::Match(ftok, "%name% ("))
        return false;
    if (const Function* f = ftok->function()) {
        if (f->isAttributePure() || f->isAttributeConst())
            return true;
        // Any modified arguments
        if (functionModifiesArguments(f))
            return false;
        if (Function::returnsVoid(f))
            return false;
        // Member function call
        if (Token::simpleMatch(ftok->previous(), ".") || exprDependsOnThis(ftok->next())) {
            if (f->isConst())
                return true;
            // Check for const overloaded function that just return the const version
            if (!Function::returnsConst(f)) {
                std::vector<const Function*> fs = f->getOverloadedFunctions();
                if (std::any_of(fs.begin(), fs.end(), [&](const Function* g) {
                    if (f == g)
                        return false;
                    if (f->argumentList.size() != g->argumentList.size())
                        return false;
                    if (functionModifiesArguments(g))
                        return false;
                    if (g->isConst() && Function::returnsConst(g))
                        return true;
                    return false;
                }))
                    return true;
            }
            return false;
        } else if (f->argumentList.empty()) {
            return f->isConstexpr();
        }
    } else if (Token::Match(ftok->previous(), ". %name% (") && ftok->previous()->originalName() != "->" &&
               astIsSmartPointer(ftok->previous()->astOperand1())) {
        return Token::Match(ftok, "get|get_deleter ( )");
    } else if (Token::Match(ftok->previous(), ". %name% (") && astIsContainer(ftok->previous()->astOperand1())) {
        const Library::Container* container = ftok->previous()->astOperand1()->valueType()->container;
        if (!container)
            return false;
        if (container->getYield(ftok->str()) != Library::Container::Yield::NO_YIELD)
            return true;
        if (container->getAction(ftok->str()) == Library::Container::Action::FIND)
            return true;
        return false;
    } else if (const Library::Function* lf = library.getFunction(ftok)) {
        if (lf->ispure)
            return true;
        if (lf->containerYield != Library::Container::Yield::NO_YIELD)
            return true;
        if (lf->containerAction == Library::Container::Action::FIND)
            return true;
        return false;
    } else {
        bool memberFunction = Token::Match(ftok->previous(), ". %name% (");
        bool constMember = !memberFunction;
        if (Token::Match(ftok->tokAt(-2), "%var% . %name% (")) {
            const Variable* var = ftok->tokAt(-2)->variable();
            if (var)
                constMember = var->isConst();
        }
        // TODO: Only check const on lvalues
        std::vector<const Token*> args = getArguments(ftok);
        if (args.empty())
            return false;
        return constMember && std::all_of(args.begin(), args.end(), [](const Token* tok) {
            const Variable* var = tok->variable();
            if (var)
                return var->isConst();
            return false;
        });
    }
    return true;
}

bool isConstExpression(const Token *tok, const Library& library, bool pure, bool cpp)
{
    if (!tok)
        return true;
    if (tok->variable() && tok->variable()->isVolatile())
        return false;
    if (tok->isName() && tok->next()->str() == "(") {
        if (!isConstFunctionCall(tok, library))
            return false;
    }
    if (tok->tokType() == Token::eIncDecOp)
        return false;
    if (tok->isAssignmentOp())
        return false;
    if (isLikelyStreamRead(cpp, tok))
        return false;
    // bailout when we see ({..})
    if (tok->str() == "{")
        return false;
    return isConstExpression(tok->astOperand1(), library, pure, cpp) && isConstExpression(tok->astOperand2(), library, pure, cpp);
}

bool isWithoutSideEffects(bool cpp, const Token* tok, bool checkArrayAccess, bool checkReference)
{
    if (!cpp)
        return true;

    while (tok && tok->astOperand2() && tok->astOperand2()->str() != "(")
        tok = tok->astOperand2();
    if (tok && tok->varId()) {
        const Variable* var = tok->variable();
        return var && ((!var->isClass() && (checkReference || !var->isReference())) || var->isPointer() || (checkArrayAccess ? var->isStlType() && !var->isStlType(CheckClass::stl_containers_not_const) : var->isStlType()));
    }
    return true;
}

bool isUniqueExpression(const Token* tok)
{
    if (!tok)
        return true;
    if (tok->function()) {
        const Function * fun = tok->function();
        const Scope * scope = fun->nestedIn;
        if (!scope)
            return true;
        const std::string returnType = fun->retType ? fun->retType->name() : fun->retDef->stringifyList(fun->tokenDef);
        for (const Function& f:scope->functionList) {
            if (f.type != Function::eFunction)
                continue;

            const std::string freturnType = f.retType ? f.retType->name() : f.retDef->stringifyList(f.returnDefEnd());
            if (f.argumentList.size() == fun->argumentList.size() &&
                returnType == freturnType &&
                f.name() != fun->name()) {
                return false;
            }
        }
    } else if (tok->variable()) {
        const Variable * var = tok->variable();
        const Scope * scope = var->scope();
        if (!scope)
            return true;
        const Type * varType = var->type();
        // Iterate over the variables in scope and the parameters of the function if possible
        const Function * fun = scope->function;
        const std::list<Variable>* setOfVars[] = {&scope->varlist, fun ? &fun->argumentList : nullptr};

        for (const std::list<Variable>* vars:setOfVars) {
            if (!vars)
                continue;
            bool other = std::any_of(vars->cbegin(), vars->cend(), [=](const Variable &v) {
                if (varType)
                    return v.type() && v.type()->name() == varType->name() && v.name() != var->name();
                return v.isFloatingType() == var->isFloatingType() &&
                v.isEnumType() == var->isEnumType() &&
                v.isClass() == var->isClass() &&
                v.isArray() == var->isArray() &&
                v.isPointer() == var->isPointer() &&
                v.name() != var->name();
            });
            if (other)
                return false;
        }
    } else if (!isUniqueExpression(tok->astOperand1())) {
        return false;
    }

    return isUniqueExpression(tok->astOperand2());
}

static bool isEscaped(const Token* tok, bool functionsScope, const Library* library)
{
    if (library && library->isnoreturn(tok))
        return true;
    if (functionsScope)
        return Token::simpleMatch(tok, "throw");
    else
        return Token::Match(tok, "return|throw");
}

static bool isEscapedOrJump(const Token* tok, bool functionsScope, const Library* library)
{
    if (library && library->isnoreturn(tok))
        return true;
    if (functionsScope)
        return Token::simpleMatch(tok, "throw");
    else
        return Token::Match(tok, "return|goto|throw|continue|break");
}

bool isEscapeFunction(const Token* ftok, const Library* library)
{
    if (!Token::Match(ftok, "%name% ("))
        return false;
    if (Token::Match(ftok, "exit|abort"))
        return true;
    const Function* function = ftok->function();
    if (function) {
        if (function->isEscapeFunction())
            return true;
        if (function->isAttributeNoreturn())
            return true;
    } else if (library) {
        if (library->isnoreturn(ftok))
            return true;
    }
    return false;
}

static bool hasNoreturnFunction(const Token* tok, const Library* library, const Token** unknownFunc)
{
    if (!tok)
        return false;
    const Token* ftok = tok->str() == "(" ? tok->previous() : nullptr;
    while (Token::simpleMatch(ftok, "("))
        ftok = ftok->astOperand1();
    if (ftok) {
        const Function * function = ftok->function();
        if (function) {
            if (function->isEscapeFunction())
                return true;
            if (function->isAttributeNoreturn())
                return true;
        } else if (library && library->isnoreturn(ftok)) {
            return true;
        } else if (Token::Match(ftok, "exit|abort")) {
            return true;
        }
        if (unknownFunc && !function && library && library->functions.count(library->getFunctionName(ftok)) == 0)
            *unknownFunc = ftok;
        return false;
    } else if (tok->isConstOp()) {
        return hasNoreturnFunction(tok->astOperand1(), library, unknownFunc) || hasNoreturnFunction(tok->astOperand2(), library, unknownFunc);
    }

    return false;
}

bool isReturnScope(const Token* const endToken, const Library* library, const Token** unknownFunc, bool functionScope)
{
    if (!endToken || endToken->str() != "}")
        return false;

    const Token *prev = endToken->previous();
    while (prev && Token::simpleMatch(prev->previous(), "; ;"))
        prev = prev->previous();
    if (prev && Token::simpleMatch(prev->previous(), "} ;"))
        prev = prev->previous();

    if (Token::simpleMatch(prev, "}")) {
        if (Token::simpleMatch(prev->link()->tokAt(-2), "} else {"))
            return isReturnScope(prev, library, unknownFunc, functionScope) &&
                   isReturnScope(prev->link()->tokAt(-2), library, unknownFunc, functionScope);
        // TODO: Check all cases
        if (!functionScope && Token::simpleMatch(prev->link()->previous(), ") {") &&
            Token::simpleMatch(prev->link()->linkAt(-1)->previous(), "switch (") &&
            !Token::findsimplematch(prev->link(), "break", prev)) {
            return isReturnScope(prev, library, unknownFunc, functionScope);
        }
        if (isEscaped(prev->link()->astTop(), functionScope, library))
            return true;
        if (Token::Match(prev->link()->previous(), "[;{}] {"))
            return isReturnScope(prev, library, unknownFunc, functionScope);
    } else if (Token::simpleMatch(prev, ";")) {
        if (prev->tokAt(-2) && hasNoreturnFunction(prev->tokAt(-2)->astTop(), library, unknownFunc))
            return true;
        // Unknown symbol
        if (Token::Match(prev->tokAt(-2), ";|}|{ %name% ;") && prev->previous()->isIncompleteVar()) {
            if (unknownFunc)
                *unknownFunc = prev->previous();
            return false;
        }
        if (Token::simpleMatch(prev->previous(), ") ;") && prev->previous()->link() &&
            isEscaped(prev->previous()->link()->astTop(), functionScope, library))
            return true;
        if (isEscaped(prev->previous()->astTop(), functionScope, library))
            return true;
        // return/goto statement
        prev = prev->previous();
        while (prev && !Token::Match(prev, ";|{|}") && !isEscapedOrJump(prev, functionScope, library))
            prev = prev->previous();
        return prev && prev->isName();
    }
    return false;
}

bool isWithinScope(const Token* tok, const Variable* var, Scope::ScopeType type)
{
    if (!tok || !var)
        return false;
    const Scope* scope = tok->scope();
    while (scope && scope != var->scope()) {
        if (scope->type == type)
            return true;
        scope = scope->nestedIn;
    }
    return false;
}

bool isVariableChangedByFunctionCall(const Token *tok, int indirect, nonneg int varid, const Settings *settings, bool *inconclusive)
{
    if (!tok)
        return false;
    if (tok->varId() == varid)
        return isVariableChangedByFunctionCall(tok, indirect, settings, inconclusive);
    return isVariableChangedByFunctionCall(tok->astOperand1(), indirect, varid, settings, inconclusive) ||
           isVariableChangedByFunctionCall(tok->astOperand2(), indirect, varid, settings, inconclusive);
}

bool isScopeBracket(const Token* tok)
{
    if (!Token::Match(tok, "{|}"))
        return false;
    if (!tok->scope())
        return false;
    if (tok->str() == "{")
        return tok->scope()->bodyStart == tok;
    if (tok->str() == "}")
        return tok->scope()->bodyEnd == tok;
    return false;
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* getTokenArgumentFunctionImpl(T* tok, int& argn)
{
    argn = -1;
    {
        T* parent = tok->astParent();
        if (parent && parent->isUnaryOp("&"))
            parent = parent->astParent();
        while (parent && parent->isCast())
            parent = parent->astParent();
        if (Token::Match(parent, "[+-]") && parent->valueType() && parent->valueType()->pointer)
            parent = parent->astParent();

        // passing variable to subfunction?
        if (Token::Match(parent, "[(,{]"))
            ;
        else if (Token::simpleMatch(parent, ":")) {
            while (Token::Match(parent, "[?:]"))
                parent = parent->astParent();
            while (Token::simpleMatch(parent, ","))
                parent = parent->astParent();
            if (!parent || parent->str() != "(")
                return nullptr;
        } else
            return nullptr;
    }

    T* argtok = tok;
    while (argtok && argtok->astParent() && (!Token::Match(argtok->astParent(), ",|(|{") || argtok->astParent()->isCast())) {
        argtok = argtok->astParent();
    }
    if (!argtok)
        return nullptr;
    if (Token::simpleMatch(argtok, ","))
        argtok = argtok->astOperand1();
    if (Token::simpleMatch(argtok, "(") && argtok->astOperand2())
        argtok = argtok->astOperand2();
    tok = argtok;
    while (Token::Match(tok->astParent(), ",|(|{")) {
        tok = tok->astParent();
        if (Token::Match(tok, "(|{"))
            break;
    }
    argn = getArgumentPos(tok, argtok);
    if (argn == -1)
        return nullptr;
    if (!Token::Match(tok, "{|("))
        return nullptr;
    if (tok->astOperand2())
        tok = tok->astOperand1();
    while (tok && (tok->isUnaryOp("*") || tok->str() == "["))
        tok = tok->astOperand1();
    while (Token::simpleMatch(tok, "."))
        tok = tok->astOperand2();
    while (Token::simpleMatch(tok, "::")) {
        // If there is only a op1 and not op2, then this is a global scope
        if (!tok->astOperand2() && tok->astOperand1()) {
            tok = tok->astOperand1();
            break;
        }
        tok = tok->astOperand2();
        if (Token::simpleMatch(tok, "<") && tok->link())
            tok = tok->astOperand1();
    }
    if (tok && tok->link() && tok->str() == ">")
        tok = tok->link()->previous();
    if (!Token::Match(tok, "%name%|(|{"))
        return nullptr;
    // Skip labels
    if (Token::Match(tok, "%name% :"))
        return nullptr;
    return tok;
}

const Token* getTokenArgumentFunction(const Token* tok, int& argn) {
    return getTokenArgumentFunctionImpl(tok, argn);
}

Token* getTokenArgumentFunction(Token* tok, int& argn) {
    return getTokenArgumentFunctionImpl(tok, argn);
}

std::vector<const Variable*> getArgumentVars(const Token* tok, int argnr)
{
    std::vector<const Variable*> result;
    if (!tok)
        return result;
    if (tok->function()) {
        const Variable* argvar = tok->function()->getArgumentVar(argnr);
        if (argvar)
            return {argvar};
        else
            return result;
    }
    if (Token::Match(tok->previous(), "%type% (|{") || Token::simpleMatch(tok, "{") || tok->variable()) {
        const bool constructor = Token::simpleMatch(tok, "{") || (tok->variable() && tok->variable()->nameToken() == tok);
        const Type* type = Token::typeOf(tok);
        if (!type)
            return result;
        const Scope* typeScope = type->classScope;
        if (!typeScope)
            return result;
        // Aggregate constructor
        if (Token::simpleMatch(tok, "{") && typeScope->numConstructors == 0 && argnr < typeScope->varlist.size()) {
            auto it = std::next(typeScope->varlist.begin(), argnr);
            return {&*it};
        }
        const int argCount = numberOfArguments(tok);
        for (const Function &function : typeScope->functionList) {
            if (function.argCount() < argCount)
                continue;
            if (constructor && !function.isConstructor())
                continue;
            if (!constructor && !Token::simpleMatch(function.token, "operator()"))
                continue;
            const Variable* argvar = function.getArgumentVar(argnr);
            if (argvar)
                result.push_back(argvar);
        }
    }
    return result;
}

static bool isCPPCastKeyword(const Token* tok)
{
    if (!tok)
        return false;
    return endsWith(tok->str(), "_cast");
}

static bool isTrivialConstructor(const Token* tok)
{
    const Token* typeTok = nullptr;
    const Type* t = Token::typeOf(tok, &typeTok);
    if (t)
        return false;
    if (typeTok->valueType() && typeTok->valueType()->isPrimitive())
        return true;
    return false;
}

static bool isArray(const Token* tok)
{
    if (!tok)
        return false;
    if (tok->variable())
        return tok->variable()->isArray();
    if (Token::simpleMatch(tok, "."))
        return isArray(tok->astOperand2());
    return false;
}

bool isVariableChangedByFunctionCall(const Token *tok, int indirect, const Settings *settings, bool *inconclusive)
{
    if (!tok)
        return false;

    if (Token::simpleMatch(tok, ","))
        return false;

    const Token * const tok1 = tok;

    // address of variable
    if (tok->astParent() && tok->astParent()->isUnaryOp("&"))
        indirect++;

    int argnr;
    tok = getTokenArgumentFunction(tok, argnr);
    if (!tok)
        return false; // not a function => variable not changed
    if (Token::simpleMatch(tok, "{") && isTrivialConstructor(tok))
        return false;
    if (tok->isKeyword() && !isCPPCastKeyword(tok) && tok->str().compare(0,8,"operator") != 0)
        return false;
    // A functional cast won't modify the variable
    if (Token::Match(tok, "%type% (|{") && tok->tokType() == Token::eType && astIsPrimitive(tok->next()))
        return false;
    const Token * parenTok = tok->next();
    if (Token::simpleMatch(parenTok, "<") && parenTok->link())
        parenTok = parenTok->link()->next();
    const bool possiblyPassedByReference = (parenTok->next() == tok1 || Token::Match(tok1->previous(), ", %name% [,)}]"));

    if (!tok->function() && !tok->variable() && tok->isName()) {
        if (settings) {
            const bool requireInit = settings->library.isuninitargbad(tok, 1 + argnr);
            const bool requireNonNull = settings->library.isnullargbad(tok, 1 + argnr);
            // Check if direction (in, out, inout) is specified in the library configuration and use that
            const Library::ArgumentChecks::Direction argDirection = settings->library.getArgDirection(tok, 1 + argnr);
            if (argDirection == Library::ArgumentChecks::Direction::DIR_IN)
                return false;
            else if (argDirection == Library::ArgumentChecks::Direction::DIR_OUT ||
                     argDirection == Library::ArgumentChecks::Direction::DIR_INOUT) {
                if (indirect == 0 && isArray(tok1))
                    return true;
                // Assume that if the variable must be initialized then the indirection is 1
                if (indirect > 0 && requireInit && requireNonNull)
                    return true;
            }

            // if the library says 0 is invalid
            // => it is assumed that parameter is an in parameter (TODO: this is a bad heuristic)
            if (indirect == 0 && requireNonNull)
                return false;
        }
        // possible pass-by-reference => inconclusive
        if (possiblyPassedByReference) {
            if (inconclusive != nullptr)
                *inconclusive = true;
            return false;
        }
        // Safe guess: Assume that parameter is changed by function call
        return true;
    }

    std::vector<const Variable*> args = getArgumentVars(tok, argnr);
    bool conclusive = false;
    for (const Variable *arg:args) {
        if (!arg)
            continue;
        conclusive = true;
        if (indirect > 0) {
            if (!arg->isConst() && arg->isPointer())
                return true;
            // If const is applied to the pointer, then the value can still be modified
            if (Token::simpleMatch(arg->typeEndToken(), "* const"))
                return true;
            if (!arg->isPointer())
                return true;
        }
        if (!arg->isConst() && arg->isReference())
            return true;
    }
    if (!conclusive && inconclusive) {
        *inconclusive = true;
    }
    return false;
}

bool isVariableChanged(const Token *tok, int indirect, const Settings *settings, bool cpp, int depth)
{
    if (!tok)
        return false;

    if (indirect == 0 && isConstVarExpression(tok))
        return false;

    const Token *tok2 = tok;
    int derefs = 0;
    while (Token::simpleMatch(tok2->astParent(), "*") ||
           (Token::simpleMatch(tok2->astParent(), ".") && !Token::simpleMatch(tok2->astParent()->astParent(), "(")) ||
           (tok2->astParent() && tok2->astParent()->isUnaryOp("&") && !tok2->astParent()->astOperand2() && Token::simpleMatch(tok2->astParent()->astParent(), ".") && tok2->astParent()->astParent()->originalName()=="->") ||
           (Token::simpleMatch(tok2->astParent(), "[") && tok2 == tok2->astParent()->astOperand1())) {
        if (tok2->astParent() && (tok2->astParent()->isUnaryOp("*") || (astIsLHS(tok2) && tok2->astParent()->originalName() == "->")))
            derefs++;
        if (derefs > indirect)
            break;
        if (tok2->astParent() && tok2->astParent()->isUnaryOp("&") && Token::simpleMatch(tok2->astParent()->astParent(), ".") && tok2->astParent()->astParent()->originalName()=="->")
            tok2 = tok2->astParent();
        tok2 = tok2->astParent();
    }

    while (Token::simpleMatch(tok2->astParent(), "?") || (Token::simpleMatch(tok2->astParent(), ":") && Token::simpleMatch(tok2->astParent()->astParent(), "?")))
        tok2 = tok2->astParent();

    if (tok2->astParent() && tok2->astParent()->tokType() == Token::eIncDecOp)
        return true;

    auto skipRedundantPtrOp = [](const Token* tok, const Token* parent) {
        const Token* gparent = parent ? parent->astParent() : nullptr;
        while (parent && gparent && ((parent->isUnaryOp("*") && gparent->isUnaryOp("&")) || (parent->isUnaryOp("&") && gparent->isUnaryOp("*")))) {
            tok = gparent;
            parent = gparent->astParent();
            if (parent)
                gparent = parent->astParent();
        }
        return tok;
    };
    tok2 = skipRedundantPtrOp(tok2, tok2->astParent());

    if (tok2->astParent() && tok2->astParent()->isAssignmentOp()) {
        if (tok2 == tok2->astParent()->astOperand1())
            return true;
        // Check if assigning to a non-const lvalue
        const Variable * var = getLHSVariable(tok2->astParent());
        if (var && var->isReference() && !var->isConst() && var->nameToken() && var->nameToken()->next() == tok2->astParent()) {
            if (!var->isLocal() || isVariableChanged(var, settings, cpp, depth - 1))
                return true;
        }
    }

    if (cpp && Token::Match(tok2->astParent(), ">>|&") && astIsRHS(tok2) && isLikelyStreamRead(cpp, tok2->astParent()))
        return true;

    if (isLikelyStream(cpp, tok2))
        return true;

    // Member function call
    if (tok->variable() && Token::Match(tok2->astParent(), ". %name%") && isFunctionCall(tok2->astParent()->next()) && tok2->astParent()->astOperand1() == tok2) {
        const Variable * var = tok->variable();
        // Member function cannot change what `this` points to
        if (indirect == 0 && astIsPointer(tok))
            return false;
        bool isConst = var && var->isConst();
        if (!isConst) {
            const ValueType * valueType = var->valueType();
            isConst = (valueType && valueType->pointer == 1 && valueType->constness == 1);
        }
        if (isConst)
            return false;

        const Token *ftok = tok->tokAt(2);
        if (settings)
            return !settings->library.isFunctionConst(ftok);

        const Function * fun = ftok->function();
        if (!fun)
            return true;
        return !fun->isConst();
    }

    const Token *ftok = tok2;
    while (ftok && (!Token::Match(ftok, "[({]") || ftok->isCast()))
        ftok = ftok->astParent();

    if (ftok && Token::Match(ftok->link(), ")|} !!{")) {
        const Token * ptok = tok2;
        while (Token::Match(ptok->astParent(), ".|::|["))
            ptok = ptok->astParent();
        bool inconclusive = false;
        bool isChanged = isVariableChangedByFunctionCall(ptok, indirect, settings, &inconclusive);
        isChanged |= inconclusive;
        if (isChanged)
            return true;
    }

    const Token *parent = tok2->astParent();
    while (Token::Match(parent, ".|::"))
        parent = parent->astParent();
    if (parent && parent->tokType() == Token::eIncDecOp)
        return true;

    // structured binding, nonconst reference variable in lhs
    if (Token::Match(tok2->astParent(), ":|=") && tok2 == tok2->astParent()->astOperand2() && Token::simpleMatch(tok2->astParent()->previous(), "]")) {
        const Token *typeStart = tok2->astParent()->previous()->link()->previous();
        if (Token::simpleMatch(typeStart, "&"))
            typeStart = typeStart->previous();
        if (typeStart && Token::Match(typeStart->previous(), "[;{}(] auto &| [")) {
            for (const Token *vartok = typeStart->tokAt(2); vartok != tok2; vartok = vartok->next()) {
                if (vartok->varId()) {
                    const Variable* refvar = vartok->variable();
                    if (!refvar || (!refvar->isConst() && refvar->isReference()))
                        return true;
                }
            }
        }
    }

    if (Token::simpleMatch(tok2->astParent(), ":") && tok2->astParent()->astParent() && Token::simpleMatch(tok2->astParent()->astParent()->previous(), "for (")) {
        // TODO: Check if container is empty or not
        if (astIsLHS(tok2))
            return true;
        const Token * varTok = tok2->astParent()->previous();
        if (!varTok)
            return false;
        const Variable * loopVar = varTok->variable();
        if (!loopVar)
            return false;
        if (!loopVar->isConst() && loopVar->isReference() && isVariableChanged(loopVar, settings, cpp, depth - 1))
            return true;
        return false;
    }

    if (indirect > 0) {
        // check for `*(ptr + 1) = new_value` case
        parent = tok2->astParent();
        while (parent && parent->isArithmeticalOp() && parent->isBinaryOp()) {
            parent = parent->astParent();
        }
        if (Token::simpleMatch(parent, "*")) {
            if (parent->astParent() && parent->astParent()->isAssignmentOp() &&
                (parent->astParent()->astOperand1() == parent)) {
                return true;
            }
        }
    }

    return false;
}

bool isVariableChanged(const Token *start, const Token *end, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth)
{
    return findVariableChanged(start, end, 0, exprid, globalvar, settings, cpp, depth) != nullptr;
}

bool isVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth)
{
    return findVariableChanged(start, end, indirect, exprid, globalvar, settings, cpp, depth) != nullptr;
}

const Token* findExpression(const Token* start, const nonneg int exprid)
{
    Function * f = Scope::nestedInFunction(start->scope());
    if (!f)
        return nullptr;
    const Scope* scope = f->functionScope;
    if (!scope)
        return nullptr;
    for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        if (tok->exprId() != exprid)
            continue;
        return tok;
    }
    return nullptr;
}

// Thread-unsafe memoization
template<class F, class R=decltype(std::declval<F>()())>
static std::function<R()> memoize(F f)
{
    bool init = false;
    R result{};
    return [=]() mutable -> R {
        if (init)
            return result;
        result = f();
        init = true;
        return result;
    };
}

template<class F,
         REQUIRES("F must be a function that returns a Token class",
                  std::is_convertible<decltype(std::declval<F>()()), const Token*> )>
static bool isExpressionChangedAt(const F& getExprTok,
                                  const Token* tok,
                                  int indirect,
                                  const nonneg int exprid,
                                  bool globalvar,
                                  const Settings* settings,
                                  bool cpp,
                                  int depth)
{
    if (depth < 0)
        return true;
    if (tok->exprId() != exprid) {
        if (globalvar && !tok->isKeyword() && Token::Match(tok, "%name% (") && !(tok->function() && tok->function()->isAttributePure()))
            // TODO: Is global variable really changed by function call?
            return true;
        bool aliased = false;
        // If we can't find the expression then assume it is an alias
        if (!getExprTok())
            aliased = true;
        if (!aliased)
            aliased = isAliasOf(tok, getExprTok());
        if (!aliased)
            return false;
        if (isVariableChanged(tok, 1, settings, cpp, depth))
            return true;
        // TODO: Try to traverse the lambda function
        if (Token::Match(tok, "%var% ("))
            return true;
        return false;
    }
    return (isVariableChanged(tok, indirect, settings, cpp, depth));
}

bool isExpressionChangedAt(const Token* expr,
                           const Token* tok,
                           int indirect,
                           bool globalvar,
                           const Settings* settings,
                           bool cpp,
                           int depth)
{
    return isExpressionChangedAt([&] {
        return expr;
    }, tok, indirect, expr->exprId(), globalvar, settings, cpp, depth);
}

Token* findVariableChanged(Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth)
{
    if (!precedes(start, end))
        return nullptr;
    if (depth < 0)
        return start;
    auto getExprTok = memoize([&] {
        return findExpression(start, exprid);
    });
    for (Token *tok = start; tok != end; tok = tok->next()) {
        if (isExpressionChangedAt(getExprTok, tok, indirect, exprid, globalvar, settings, cpp, depth))
            return tok;
    }
    return nullptr;
}

const Token* findVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings *settings, bool cpp, int depth)
{
    return findVariableChanged(const_cast<Token*>(start), end, indirect, exprid, globalvar, settings, cpp, depth);
}

bool isVariableChanged(const Variable * var, const Settings *settings, bool cpp, int depth)
{
    if (!var)
        return false;
    if (!var->scope())
        return false;
    const Token * start = var->declEndToken();
    if (!start)
        return false;
    if (Token::Match(start, "; %varid% =", var->declarationId()))
        start = start->tokAt(2);
    return isExpressionChanged(var->nameToken(), start->next(), var->scope()->bodyEnd, settings, cpp, depth);
}

bool isVariablesChanged(const Token* start,
                        const Token* end,
                        int indirect,
                        std::vector<const Variable*> vars,
                        const Settings* settings,
                        bool cpp)
{
    std::set<int> varids;
    std::transform(vars.begin(), vars.end(), std::inserter(varids, varids.begin()), [](const Variable* var) {
        return var->declarationId();
    });
    const bool globalvar = std::any_of(vars.begin(), vars.end(), [](const Variable* var) {
        return var->isGlobal();
    });
    for (const Token* tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() == 0 || varids.count(tok->varId()) == 0) {
            if (globalvar && Token::Match(tok, "%name% ("))
                // TODO: Is global variable really changed by function call?
                return true;
            continue;
        }
        if (isVariableChanged(tok, indirect, settings, cpp))
            return true;
    }
    return false;
}

bool isThisChanged(const Token* tok, int indirect, const Settings* settings, bool cpp)
{
    if ((Token::Match(tok->previous(), "%name% (") && !Token::simpleMatch(tok->astOperand1(), ".")) ||
        Token::Match(tok->tokAt(-3), "this . %name% (")) {
        if (tok->previous()->function()) {
            return (!tok->previous()->function()->isConst());
        } else if (!tok->previous()->isKeyword()) {
            return true;
        }
    }
    if (isVariableChanged(tok, indirect, settings, cpp))
        return true;
    return false;
}

bool isThisChanged(const Token* start, const Token* end, int indirect, const Settings* settings, bool cpp)
{
    if (!precedes(start, end))
        return false;
    for (const Token* tok = start; tok != end; tok = tok->next()) {
        if (!exprDependsOnThis(tok))
            continue;
        if (isThisChanged(tok, indirect, settings, cpp))
            return true;
    }
    return false;
}

bool isExpressionChanged(const Token* expr, const Token* start, const Token* end, const Settings* settings, bool cpp, int depth)
{
    if (depth < 0)
        return true;
    if (!precedes(start, end))
        return false;
    const Token* result = findAstNode(expr, [&](const Token* tok) {
        if (exprDependsOnThis(tok) && isThisChanged(start, end, false, settings, cpp)) {
            return true;
        }
        bool global = false;
        if (tok->variable()) {
            if (tok->variable()->isConst())
                return false;
            global = !tok->variable()->isLocal() && !tok->variable()->isArgument();
        } else if (tok->isIncompleteVar() && !tok->isIncompleteConstant()) {
            global = true;
        }

        if (tok->exprId() > 0) {
            for (const Token* tok2 = start; tok2 != end; tok2 = tok2->next()) {
                if (isExpressionChangedAt(
                        tok, tok2, tok->valueType() ? tok->valueType()->pointer : 0, global, settings, cpp, depth))
                    return true;
            }
        }
        return false;
    });
    return result;
}

const Token* getArgumentStart(const Token* ftok)
{
    const Token* tok = ftok;
    if (Token::Match(tok, "%name% (|{"))
        tok = ftok->next();
    if (!Token::Match(tok, "(|{|["))
        return nullptr;
    const Token* startTok = tok->astOperand2();
    if (!startTok && tok->next() != tok->link())
        startTok = tok->astOperand1();
    return startTok;
}

int numberOfArguments(const Token* ftok) {
    return astCount(getArgumentStart(ftok), ",");
}

int numberOfArgumentsWithoutAst(const Token* start)
{
    int arguments=0;
    const Token* const openBracket = start->next();
    if (openBracket && openBracket->str()=="(" && openBracket->next() && openBracket->next()->str()!=")") {
        const Token* argument=openBracket->next();
        while (argument) {
            ++arguments;
            argument = argument->nextArgument();
        }
    }
    return arguments;
}

std::vector<const Token*> getArguments(const Token* ftok) {
    return astFlatten(getArgumentStart(ftok), ",");
}

int getArgumentPos(const Variable* var, const Function* f)
{
    auto arg_it = std::find_if(f->argumentList.begin(), f->argumentList.end(), [&](const Variable& v) {
        return v.nameToken() == var->nameToken();
    });
    if (arg_it == f->argumentList.end())
        return -1;
    return std::distance(f->argumentList.begin(), arg_it);
}

bool isIteratorPair(std::vector<const Token*> args)
{
    return args.size() == 2 &&
           ((astIsIterator(args[0]) && astIsIterator(args[1])) || (astIsPointer(args[0]) && astIsPointer(args[1])));
}

const Token *findLambdaStartToken(const Token *last)
{
    if (!last || last->str() != "}")
        return nullptr;
    const Token* tok = last->link();
    if (Token::simpleMatch(tok->astParent(), "("))
        tok = tok->astParent();
    if (Token::simpleMatch(tok->astParent(), "["))
        return tok->astParent();
    return nullptr;
}

template<class T>
T* findLambdaEndTokenGeneric(T* first)
{
    if (!first || first->str() != "[")
        return nullptr;
    if (!Token::Match(first->link(), "] (|{"))
        return nullptr;
    if (first->astOperand1() != first->link()->next())
        return nullptr;
    const Token * tok = first;

    if (tok->astOperand1() && tok->astOperand1()->str() == "(")
        tok = tok->astOperand1();
    if (tok->astOperand1() && tok->astOperand1()->str() == "{")
        return tok->astOperand1()->link();
    return nullptr;
}

const Token* findLambdaEndToken(const Token* first)
{
    return findLambdaEndTokenGeneric(first);
}
Token* findLambdaEndToken(Token* first)
{
    return findLambdaEndTokenGeneric(first);
}

bool isLikelyStream(bool cpp, const Token *stream)
{
    if (!cpp)
        return false;

    if (!stream)
        return false;

    if (!Token::Match(stream->astParent(), "&|<<|>>") || !stream->astParent()->isBinaryOp())
        return false;

    if (stream->astParent()->astOperand1() != stream)
        return false;

    return !astIsIntegral(stream, false);
}

bool isLikelyStreamRead(bool cpp, const Token *op)
{
    if (!cpp)
        return false;

    if (!Token::Match(op, "&|>>") || !op->isBinaryOp())
        return false;

    if (!Token::Match(op->astOperand2(), "%name%|.|*|[") && op->str() != op->astOperand2()->str())
        return false;

    const Token *parent = op;
    while (parent->astParent() && parent->astParent()->str() == op->str())
        parent = parent->astParent();
    if (parent->astParent() && !Token::Match(parent->astParent(), "%oror%|&&|(|,|.|!|;"))
        return false;
    if (op->str() == "&" && parent->astParent())
        return false;
    if (!parent->astOperand1() || !parent->astOperand2())
        return false;
    return (!parent->astOperand1()->valueType() || !parent->astOperand1()->valueType()->isIntegral());
}

bool isCPPCast(const Token* tok)
{
    return tok && Token::simpleMatch(tok->previous(), "> (") && tok->astOperand2() && tok->astOperand1() && isCPPCastKeyword(tok->astOperand1());
}

bool isConstVarExpression(const Token *tok, const char* skipMatch)
{
    if (!tok)
        return false;
    if (tok->str() == "?" && tok->astOperand2() && tok->astOperand2()->str() == ":") // ternary operator
        return isConstVarExpression(tok->astOperand2()->astOperand1()) && isConstVarExpression(tok->astOperand2()->astOperand2()); // left and right of ":"
    if (skipMatch && Token::Match(tok, skipMatch))
        return false;
    if (Token::simpleMatch(tok->previous(), "sizeof ("))
        return true;
    if (Token::Match(tok->previous(), "%name% (")) {
        if (Token::simpleMatch(tok->astOperand1(), ".") && !isConstVarExpression(tok->astOperand1(), skipMatch))
            return false;
        std::vector<const Token *> args = getArguments(tok);
        return std::all_of(args.begin(), args.end(), [&](const Token* t) {
            return isConstVarExpression(t, skipMatch);
        });
    }
    if (isCPPCast(tok)) {
        return isConstVarExpression(tok->astOperand2(), skipMatch);
    }
    if (Token::Match(tok, "( %type%"))
        return isConstVarExpression(tok->astOperand1(), skipMatch);
    if (tok->str() == "::" && tok->hasKnownValue())
        return isConstVarExpression(tok->astOperand2(), skipMatch);
    if (Token::Match(tok, "%cop%|[|.")) {
        if (tok->astOperand1() && !isConstVarExpression(tok->astOperand1(), skipMatch))
            return false;
        if (tok->astOperand2() && !isConstVarExpression(tok->astOperand2(), skipMatch))
            return false;
        return true;
    }
    if (Token::Match(tok, "%bool%|%num%|%str%|%char%|nullptr|NULL"))
        return true;
    if (tok->isEnumerator())
        return true;
    if (tok->variable())
        return tok->variable()->isConst() && tok->variable()->nameToken() && tok->variable()->nameToken()->hasKnownValue();
    return false;
}

static void getLHSVariablesRecursive(std::vector<const Variable*>& vars, const Token* tok)
{
    if (!tok)
        return;
    if (vars.empty() && Token::Match(tok, "*|&|&&|[")) {
        getLHSVariablesRecursive(vars, tok->astOperand1());
        if (!vars.empty() || Token::simpleMatch(tok, "["))
            return;
        getLHSVariablesRecursive(vars, tok->astOperand2());
    } else if (Token::Match(tok->previous(), "this . %var%")) {
        getLHSVariablesRecursive(vars, tok->next());
    } else if (Token::simpleMatch(tok, ".")) {
        getLHSVariablesRecursive(vars, tok->astOperand1());
        getLHSVariablesRecursive(vars, tok->astOperand2());
    } else if (Token::simpleMatch(tok, "::")) {
        getLHSVariablesRecursive(vars, tok->astOperand2());
    } else if (tok->variable()) {
        vars.push_back(tok->variable());
    }
}

std::vector<const Variable*> getLHSVariables(const Token* tok)
{
    std::vector<const Variable*> result;
    if (!Token::Match(tok, "%assign%|(|{"))
        return result;
    if (!tok->astOperand1())
        return result;
    if (tok->astOperand1()->varId() > 0 && tok->astOperand1()->variable())
        return {tok->astOperand1()->variable()};
    getLHSVariablesRecursive(result, tok->astOperand1());
    return result;
}

static const Token* getLHSVariableRecursive(const Token* tok)
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok, "*|&|&&|[")) {
        const Token* vartok = getLHSVariableRecursive(tok->astOperand1());
        if ((vartok && vartok->variable()) || Token::simpleMatch(tok, "["))
            return vartok;
        return getLHSVariableRecursive(tok->astOperand2());
    }
    if (Token::Match(tok->previous(), "this . %var%"))
        return tok->next();
    return tok;
}

const Variable *getLHSVariable(const Token *tok)
{
    if (!tok || !tok->isAssignmentOp())
        return nullptr;
    if (!tok->astOperand1())
        return nullptr;
    if (tok->astOperand1()->varId() > 0 && tok->astOperand1()->variable())
        return tok->astOperand1()->variable();
    const Token* vartok = getLHSVariableRecursive(tok->astOperand1());
    if (!vartok)
        return nullptr;
    return vartok->variable();
}

const Token* getLHSVariableToken(const Token* tok)
{
    if (!Token::Match(tok, "%assign%"))
        return nullptr;
    if (!tok->astOperand1())
        return nullptr;
    if (tok->astOperand1()->varId() > 0)
        return tok->astOperand1();
    const Token* vartok = getLHSVariableRecursive(tok->astOperand1());
    if (vartok && vartok->variable() && vartok->variable()->nameToken() == vartok)
        return vartok;
    return tok->astOperand1();
}

const Token* findAllocFuncCallToken(const Token *expr, const Library &library)
{
    if (!expr)
        return nullptr;
    if (Token::Match(expr, "[+-]")) {
        const Token *tok1 = findAllocFuncCallToken(expr->astOperand1(), library);
        return tok1 ? tok1 : findAllocFuncCallToken(expr->astOperand2(), library);
    }
    if (expr->isCast())
        return findAllocFuncCallToken(expr->astOperand2() ? expr->astOperand2() : expr->astOperand1(), library);
    if (Token::Match(expr->previous(), "%name% (") && library.getAllocFuncInfo(expr->astOperand1()))
        return expr->astOperand1();
    return (Token::simpleMatch(expr, "new") && expr->astOperand1()) ? expr : nullptr;
}

static bool nonLocal(const Variable* var, bool deref)
{
    return !var || (!var->isLocal() && !var->isArgument()) || (deref && var->isArgument() && var->isPointer()) || var->isStatic() || var->isReference() || var->isExtern();
}

static bool hasGccCompoundStatement(const Token *tok)
{
    if (!tok)
        return false;
    if (tok->str() == "{" && Token::simpleMatch(tok->previous(), "( {"))
        return true;
    return hasGccCompoundStatement(tok->astOperand1()) || hasGccCompoundStatement(tok->astOperand2());
}

static bool hasFunctionCall(const Token *tok)
{
    if (!tok)
        return false;
    if (Token::Match(tok, "%name% ("))
        // todo, const/pure function?
        return true;
    return hasFunctionCall(tok->astOperand1()) || hasFunctionCall(tok->astOperand2());
}

static bool isUnchanged(const Token *startToken, const Token *endToken, const std::set<nonneg int> &exprVarIds, bool local)
{
    for (const Token *tok = startToken; tok != endToken; tok = tok->next()) {
        if (!local && Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->linkAt(1), ") {"))
            // TODO: this is a quick bailout
            return false;
        if (tok->varId() == 0 || exprVarIds.find(tok->varId()) == exprVarIds.end())
            continue;
        const Token *parent = tok;
        while (parent->astParent() && !parent->astParent()->isAssignmentOp() && parent->astParent()->tokType() != Token::Type::eIncDecOp) {
            if (parent->str() == "," || parent->isUnaryOp("&"))
                // TODO: This is a quick bailout
                return false;
            parent = parent->astParent();
        }
        if (parent->astParent()) {
            if (parent->astParent()->tokType() == Token::Type::eIncDecOp)
                return false;
            else if (parent->astParent()->isAssignmentOp() && parent == parent->astParent()->astOperand1())
                return false;
        }
    }
    return true;
}

bool isNullOperand(const Token *expr)
{
    if (!expr)
        return false;
    if (Token::Match(expr, "static_cast|const_cast|dynamic_cast|reinterpret_cast <"))
        expr = expr->astParent();
    else if (!expr->isCast())
        return Token::Match(expr, "NULL|nullptr");
    if (expr->valueType() && expr->valueType()->pointer == 0)
        return false;
    const Token *castOp = expr->astOperand2() ? expr->astOperand2() : expr->astOperand1();
    return Token::Match(castOp, "NULL|nullptr") || (MathLib::isInt(castOp->str()) && MathLib::isNullValue(castOp->str()));
}

bool isGlobalData(const Token *expr, bool cpp)
{
    // function call that returns reference => assume global data
    if (expr && expr->str() == "(" && expr->valueType() && expr->valueType()->reference != Reference::None) {
        if (expr->isBinaryOp())
            return true;
        if (expr->astOperand1() && precedes(expr->astOperand1(), expr))
            return true;
    }

    bool globalData = false;
    bool var = false;
    visitAstNodes(expr,
                  [expr, cpp, &globalData, &var](const Token *tok) {
        if (tok->varId())
            var = true;
        if (tok->varId() && !tok->variable()) {
            // Bailout, this is probably global
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (tok->originalName() == "->") {
            // TODO check if pointer points at local data
            globalData = true;
            return ChildrenToVisit::none;
        } else if (Token::Match(tok, "[*[]") && tok->astOperand1() && tok->astOperand1()->variable()) {
            // TODO check if pointer points at local data
            const Variable *lhsvar = tok->astOperand1()->variable();
            const ValueType *lhstype = tok->astOperand1()->valueType();
            if (lhsvar->isPointer()) {
                globalData = true;
                return ChildrenToVisit::none;
            } else if (lhsvar->isArgument() && lhsvar->isArray()) {
                globalData = true;
                return ChildrenToVisit::none;
            } else if (lhsvar->isArgument() && (!lhstype || (lhstype->type <= ValueType::Type::VOID && !lhstype->container))) {
                globalData = true;
                return ChildrenToVisit::none;
            }
        }
        if (tok->varId() == 0 && tok->isName() && tok->previous()->str() != ".") {
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (tok->variable()) {
            // TODO : Check references
            if (tok->variable()->isReference() && tok != tok->variable()->nameToken()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->variable()->isExtern()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->previous()->str() != "." && !tok->variable()->isLocal() && !tok->variable()->isArgument()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->variable()->isArgument() && tok->variable()->isPointer() && tok != expr) {
                globalData = true;
                return ChildrenToVisit::none;
            }
            if (tok->variable()->isPointerArray()) {
                globalData = true;
                return ChildrenToVisit::none;
            }
        }
        // Unknown argument type => it might be some reference type..
        if (cpp && tok->str() == "." && tok->astOperand1() && tok->astOperand1()->variable() && !tok->astOperand1()->valueType()) {
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (Token::Match(tok, ".|["))
            return ChildrenToVisit::op1;
        return ChildrenToVisit::op1_and_op2;
    });
    return globalData || !var;
}

struct FwdAnalysis::Result FwdAnalysis::checkRecursive(const Token *expr, const Token *startToken, const Token *endToken, const std::set<nonneg int> &exprVarIds, bool local, bool inInnerClass, int depth)
{
    // Parse the given tokens
    if (++depth > 1000)
        return Result(Result::Type::BAILOUT);

    for (const Token* tok = startToken; precedes(tok, endToken); tok = tok->next()) {
        if (Token::simpleMatch(tok, "try {")) {
            // TODO: handle try
            return Result(Result::Type::BAILOUT);
        }

        if (Token::simpleMatch(tok, "break ;")) {
            return Result(Result::Type::BREAK, tok);
        }

        if (Token::simpleMatch(tok, "goto"))
            return Result(Result::Type::BAILOUT);

        if (!inInnerClass && tok->str() == "{" && tok->scope()->isClassOrStruct()) {
            // skip returns from local class definition
            FwdAnalysis::Result result = checkRecursive(expr, tok, tok->link(), exprVarIds, local, true, depth);
            if (result.type != Result::Type::NONE)
                return result;
            tok=tok->link();
        }

        if (tok->str() == "continue")
            // TODO
            return Result(Result::Type::BAILOUT);

        if (const Token *lambdaEndToken = findLambdaEndToken(tok)) {
            tok = lambdaEndToken;
            const Result lambdaResult = checkRecursive(expr, lambdaEndToken->link()->next(), lambdaEndToken, exprVarIds, local, inInnerClass, depth);
            if (lambdaResult.type == Result::Type::READ || lambdaResult.type == Result::Type::BAILOUT)
                return lambdaResult;
        }

        if (Token::Match(tok, "return|throw")) {
            // TODO: Handle these better
            // Is expr variable used in expression?

            const Token* opTok = tok->astOperand1();
            if (!opTok)
                opTok = tok->next();
            std::pair<const Token*, const Token*> startEndTokens = opTok->findExpressionStartEndTokens();
            FwdAnalysis::Result result =
                checkRecursive(expr, startEndTokens.first, startEndTokens.second->next(), exprVarIds, local, true, depth);
            if (result.type != Result::Type::NONE)
                return result;

            // #9167: if the return is inside an inner class, it does not tell us anything
            if (!inInnerClass) {
                if (!local && mWhat == What::Reassign)
                    return Result(Result::Type::BAILOUT);

                return Result(Result::Type::RETURN);
            }
        }

        if (tok->str() == "}") {
            // Known value => possible value
            if (tok->scope() == expr->scope())
                mValueFlowKnown = false;

            if (tok->scope()->isLoopScope()) {
                // check condition
                const Token *conditionStart = nullptr;
                const Token *conditionEnd = nullptr;
                if (Token::simpleMatch(tok->link()->previous(), ") {")) {
                    conditionEnd = tok->link()->previous();
                    conditionStart = conditionEnd->link();
                } else if (Token::simpleMatch(tok->link()->previous(), "do {") && Token::simpleMatch(tok, "} while (")) {
                    conditionStart = tok->tokAt(2);
                    conditionEnd = conditionStart->link();
                }
                if (conditionStart && conditionEnd) {
                    bool used = false;
                    for (const Token *condTok = conditionStart; condTok != conditionEnd; condTok = condTok->next()) {
                        if (exprVarIds.find(condTok->varId()) != exprVarIds.end()) {
                            used = true;
                            break;
                        }
                    }
                    if (used)
                        return Result(Result::Type::BAILOUT);
                }

                // check loop body again..
                const struct FwdAnalysis::Result &result = checkRecursive(expr, tok->link(), tok, exprVarIds, local, inInnerClass, depth);
                if (result.type == Result::Type::BAILOUT || result.type == Result::Type::READ)
                    return result;
            }
        }

        if (Token::simpleMatch(tok, "else {"))
            tok = tok->linkAt(1);

        if (Token::simpleMatch(tok, "asm ("))
            return Result(Result::Type::BAILOUT);

        if (mWhat == What::ValueFlow && (Token::Match(tok, "while|for (") || Token::simpleMatch(tok, "do {"))) {
            const Token *bodyStart = nullptr;
            const Token *conditionStart = nullptr;
            if (Token::simpleMatch(tok, "do {")) {
                bodyStart = tok->next();
                if (Token::simpleMatch(bodyStart->link(), "} while ("))
                    conditionStart = bodyStart->link()->tokAt(2);
            } else {
                conditionStart = tok->next();
                if (Token::simpleMatch(conditionStart->link(), ") {"))
                    bodyStart = conditionStart->link()->next();
            }

            if (!bodyStart || !conditionStart)
                return Result(Result::Type::BAILOUT);

            // Is expr changed in condition?
            if (!isUnchanged(conditionStart, conditionStart->link(), exprVarIds, local))
                return Result(Result::Type::BAILOUT);

            // Is expr changed in loop body?
            if (!isUnchanged(bodyStart, bodyStart->link(), exprVarIds, local))
                return Result(Result::Type::BAILOUT);
        }

        if (mWhat == What::ValueFlow && Token::simpleMatch(tok, "if (") && Token::simpleMatch(tok->linkAt(1), ") {")) {
            const Token *bodyStart = tok->linkAt(1)->next();
            const Token *conditionStart = tok->next();
            const Token *condTok = conditionStart->astOperand2();
            if (condTok->hasKnownIntValue()) {
                bool cond = condTok->values().front().intvalue;
                if (cond) {
                    FwdAnalysis::Result result = checkRecursive(expr, bodyStart, bodyStart->link(), exprVarIds, local, true, depth);
                    if (result.type != Result::Type::NONE)
                        return result;
                } else if (Token::simpleMatch(bodyStart->link(), "} else {")) {
                    bodyStart = bodyStart->link()->tokAt(2);
                    FwdAnalysis::Result result = checkRecursive(expr, bodyStart, bodyStart->link(), exprVarIds, local, true, depth);
                    if (result.type != Result::Type::NONE)
                        return result;
                }
            }
            tok = bodyStart->link();
            if (isReturnScope(tok, &mLibrary))
                return Result(Result::Type::BAILOUT);
            if (Token::simpleMatch(tok, "} else {"))
                tok = tok->linkAt(2);
            if (!tok)
                return Result(Result::Type::BAILOUT);

            // Is expr changed in condition?
            if (!isUnchanged(conditionStart, conditionStart->link(), exprVarIds, local))
                return Result(Result::Type::BAILOUT);

            // Is expr changed in condition body?
            if (!isUnchanged(bodyStart, bodyStart->link(), exprVarIds, local))
                return Result(Result::Type::BAILOUT);
        }

        if (!local && Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->linkAt(1), ") {")) {
            // TODO: this is a quick bailout
            return Result(Result::Type::BAILOUT);
        }

        if (mWhat == What::Reassign &&
            Token::simpleMatch(tok, ";") &&
            Token::simpleMatch(tok->astParent(), ";") &&
            Token::simpleMatch(tok->astParent()->astParent(), "(") &&
            Token::simpleMatch(tok->astParent()->astParent()->previous(), "for (") &&
            !isUnchanged(tok, tok->astParent()->astParent()->link(), exprVarIds, local))
            // TODO: This is a quick bailout to avoid FP #9420, there are false negatives (TODO_ASSERT_EQUALS)
            return Result(Result::Type::BAILOUT);

        if (expr->isName() && Token::Match(tok, "%name% (") && tok->str().find("<") != std::string::npos && tok->str().find(expr->str()) != std::string::npos)
            return Result(Result::Type::BAILOUT);

        if (exprVarIds.find(tok->varId()) != exprVarIds.end()) {
            const Token *parent = tok;
            bool other = false;
            bool same = tok->astParent() && isSameExpression(mCpp, false, expr, tok, mLibrary, true, false, nullptr);
            while (!same && Token::Match(parent->astParent(), "*|.|::|[|(|%cop%")) {
                parent = parent->astParent();
                if (parent->str() == "(" && !parent->isCast())
                    break;
                if (isSameExpression(mCpp, false, expr, parent, mLibrary, true, false, nullptr)) {
                    same = true;
                    if (mWhat == What::ValueFlow) {
                        KnownAndToken v;
                        v.known = mValueFlowKnown;
                        v.token = parent;
                        mValueFlow.push_back(v);
                    }
                }
                if (Token::Match(parent, ". %var%") && parent->next()->varId() && exprVarIds.find(parent->next()->varId()) == exprVarIds.end() &&
                    isSameExpression(mCpp, false, expr->astOperand1(), parent->astOperand1(), mLibrary, true, false, nullptr)) {
                    other = true;
                    break;
                }
            }
            if (mWhat != What::ValueFlow && same && Token::simpleMatch(parent->astParent(), "[") && parent == parent->astParent()->astOperand2()) {
                return Result(Result::Type::READ);
            }
            if (other)
                continue;
            if (Token::simpleMatch(parent->astParent(), "=") && parent == parent->astParent()->astOperand1()) {
                if (!local && hasFunctionCall(parent->astParent()->astOperand2())) {
                    // TODO: this is a quick bailout
                    return Result(Result::Type::BAILOUT);
                }
                if (hasOperand(parent->astParent()->astOperand2(), expr)) {
                    if (mWhat == What::Reassign)
                        return Result(Result::Type::READ);
                    continue;
                }
                const auto startEnd = parent->astParent()->astOperand2()->findExpressionStartEndTokens();
                for (const Token* tok2 = startEnd.first; tok2 != startEnd.second; tok2 = tok2->next()) {
                    if (tok2->tokType() == Token::eLambda)
                        return Result(Result::Type::BAILOUT);
                    // TODO: analyze usage in lambda
                }
                // ({ .. })
                if (hasGccCompoundStatement(parent->astParent()->astOperand2()))
                    return Result(Result::Type::BAILOUT);
                const bool reassign = isSameExpression(mCpp, false, expr, parent, mLibrary, false, false, nullptr);
                if (reassign)
                    return Result(Result::Type::WRITE, parent->astParent());
                return Result(Result::Type::READ);
            } else if (mWhat == What::Reassign && parent->valueType() && parent->valueType()->pointer && Token::Match(parent->astParent(), "%assign%") && parent == parent->astParent()->astOperand1()) {
                return Result(Result::Type::READ);
            } else if (Token::Match(parent->astParent(), "%assign%") && !parent->astParent()->astParent() && parent == parent->astParent()->astOperand1()) {
                if (mWhat == What::Reassign)
                    return Result(Result::Type::BAILOUT, parent->astParent());
                if (mWhat == What::UnusedValue && (!parent->valueType() || parent->valueType()->reference != Reference::None))
                    return Result(Result::Type::BAILOUT, parent->astParent());
                continue;
            } else if (mWhat == What::UnusedValue && parent->isUnaryOp("&") && Token::Match(parent->astParent(), "[,(]")) {
                // Pass variable to function the writes it
                const Token *ftok = parent->astParent();
                while (Token::simpleMatch(ftok, ","))
                    ftok = ftok->astParent();
                if (ftok && Token::Match(ftok->previous(), "%name% (")) {
                    const std::vector<const Token *> args = getArguments(ftok);
                    int argnr = 0;
                    while (argnr < args.size() && args[argnr] != parent)
                        argnr++;
                    if (argnr < args.size()) {
                        const Library::Function* functionInfo = mLibrary.getFunction(ftok->astOperand1());
                        if (functionInfo) {
                            const auto it = functionInfo->argumentChecks.find(argnr + 1);
                            if (it != functionInfo->argumentChecks.end() && it->second.direction == Library::ArgumentChecks::Direction::DIR_OUT)
                                continue;
                        }
                    }
                }
                return Result(Result::Type::BAILOUT, parent->astParent());
            } else {
                // TODO: this is a quick bailout
                return Result(Result::Type::BAILOUT, parent->astParent());
            }
        }

        if (Token::Match(tok, ")|do {")) {
            if (tok->str() == ")" && Token::simpleMatch(tok->link()->previous(), "switch ("))
                // TODO: parse switch
                return Result(Result::Type::BAILOUT);
            const Result &result1 = checkRecursive(expr, tok->tokAt(2), tok->linkAt(1), exprVarIds, local, inInnerClass, depth);
            if (result1.type == Result::Type::READ || result1.type == Result::Type::BAILOUT)
                return result1;
            if (mWhat == What::ValueFlow && result1.type == Result::Type::WRITE)
                mValueFlowKnown = false;
            if (mWhat == What::Reassign && result1.type == Result::Type::BREAK) {
                const Token *scopeEndToken = findNextTokenFromBreak(result1.token);
                if (scopeEndToken) {
                    const Result &result2 = checkRecursive(expr, scopeEndToken->next(), endToken, exprVarIds, local, inInnerClass, depth);
                    if (result2.type == Result::Type::BAILOUT)
                        return result2;
                }
            }
            if (Token::simpleMatch(tok->linkAt(1), "} else {")) {
                const Token *elseStart = tok->linkAt(1)->tokAt(2);
                const Result &result2 = checkRecursive(expr, elseStart, elseStart->link(), exprVarIds, local, inInnerClass, depth);
                if (mWhat == What::ValueFlow && result2.type == Result::Type::WRITE)
                    mValueFlowKnown = false;
                if (result2.type == Result::Type::READ || result2.type == Result::Type::BAILOUT)
                    return result2;
                if (result1.type == Result::Type::WRITE && result2.type == Result::Type::WRITE)
                    return result1;
                tok = elseStart->link();
            } else {
                tok = tok->linkAt(1);
            }
        }
    }

    return Result(Result::Type::NONE);
}

static bool hasVolatileCastOrVar(const Token *expr)
{
    bool ret = false;
    visitAstNodes(expr,
                  [&ret](const Token *tok) {
        if (Token::simpleMatch(tok, "( volatile"))
            ret = true;
        else if (tok->variable() && tok->variable()->isVolatile())
            ret = true;
        return ret ? ChildrenToVisit::none : ChildrenToVisit::op1_and_op2;
    });
    return ret;
}

bool FwdAnalysis::isGlobalData(const Token *expr) const
{
    return ::isGlobalData(expr, mCpp);
}

std::set<nonneg int> FwdAnalysis::getExprVarIds(const Token* expr, bool* localOut, bool* unknownVarIdOut) const
{
    // all variable ids in expr.
    std::set<nonneg int> exprVarIds;
    bool local = true;
    bool unknownVarId = false;
    visitAstNodes(expr,
                  [&](const Token *tok) {
        if (tok->str() == "[" && mWhat == What::UnusedValue)
            return ChildrenToVisit::op1;
        if (tok->varId() == 0 && tok->isName() && tok->previous()->str() != ".") {
            // unknown variable
            unknownVarId = true;
            return ChildrenToVisit::none;
        }
        if (tok->varId() > 0) {
            exprVarIds.insert(tok->varId());
            if (!Token::simpleMatch(tok->previous(), ".")) {
                const Variable *var = tok->variable();
                if (var && var->isReference() && var->isLocal() && Token::Match(var->nameToken(), "%var% [=(]") && !isGlobalData(var->nameToken()->next()->astOperand2()))
                    return ChildrenToVisit::none;
                const bool deref = tok->astParent() && (tok->astParent()->isUnaryOp("*") || (tok->astParent()->str() == "[" && tok == tok->astParent()->astOperand1()));
                local &= !nonLocal(tok->variable(), deref);
            }
        }
        return ChildrenToVisit::op1_and_op2;
    });
    if (localOut)
        *localOut = local;
    if (unknownVarIdOut)
        *unknownVarIdOut = unknownVarId;
    return exprVarIds;
}

FwdAnalysis::Result FwdAnalysis::check(const Token* expr, const Token* startToken, const Token* endToken)
{
    // all variable ids in expr.
    bool local = true;
    bool unknownVarId = false;
    std::set<nonneg int> exprVarIds = getExprVarIds(expr, &local, &unknownVarId);

    if (unknownVarId)
        return Result(FwdAnalysis::Result::Type::BAILOUT);

    if (mWhat == What::Reassign && isGlobalData(expr))
        local = false;

    // In unused values checking we do not want to check assignments to
    // global data.
    if (mWhat == What::UnusedValue && isGlobalData(expr))
        return Result(FwdAnalysis::Result::Type::BAILOUT);

    Result result = checkRecursive(expr, startToken, endToken, exprVarIds, local, false);

    // Break => continue checking in outer scope
    while (mWhat!=What::ValueFlow && result.type == FwdAnalysis::Result::Type::BREAK) {
        const Token *scopeEndToken = findNextTokenFromBreak(result.token);
        if (!scopeEndToken)
            break;
        result = checkRecursive(expr, scopeEndToken->next(), endToken, exprVarIds, local, false);
    }

    return result;
}

bool FwdAnalysis::hasOperand(const Token *tok, const Token *lhs) const
{
    if (!tok)
        return false;
    if (isSameExpression(mCpp, false, tok, lhs, mLibrary, false, false, nullptr))
        return true;
    return hasOperand(tok->astOperand1(), lhs) || hasOperand(tok->astOperand2(), lhs);
}

const Token *FwdAnalysis::reassign(const Token *expr, const Token *startToken, const Token *endToken)
{
    if (hasVolatileCastOrVar(expr))
        return nullptr;
    mWhat = What::Reassign;
    Result result = check(expr, startToken, endToken);
    return result.type == FwdAnalysis::Result::Type::WRITE ? result.token : nullptr;
}

bool FwdAnalysis::unusedValue(const Token *expr, const Token *startToken, const Token *endToken)
{
    if (isEscapedAlias(expr))
        return false;
    if (hasVolatileCastOrVar(expr))
        return false;
    mWhat = What::UnusedValue;
    Result result = check(expr, startToken, endToken);
    return (result.type == FwdAnalysis::Result::Type::NONE || result.type == FwdAnalysis::Result::Type::RETURN) && !possiblyAliased(expr, startToken);
}

bool FwdAnalysis::possiblyAliased(const Token *expr, const Token *startToken) const
{
    if (expr->isUnaryOp("*"))
        return true;

    const bool macro = false;
    const bool pure = false;
    const bool followVar = false;
    for (const Token *tok = startToken; tok; tok = tok->previous()) {
        if (tok->str() == "{" && tok->scope()->type == Scope::eFunction && !(tok->astParent() && tok->astParent()->str() == ","))
            break;

        if (Token::Match(tok, "%name% (") && !Token::Match(tok, "if|while|for")) {
            // Is argument passed by reference?
            const std::vector<const Token*> args = getArguments(tok);
            for (int argnr = 0; argnr < args.size(); ++argnr) {
                if (!Token::Match(args[argnr], "%name%|.|::"))
                    continue;
                if (tok->function() && tok->function()->getArgumentVar(argnr) && !tok->function()->getArgumentVar(argnr)->isReference() && !tok->function()->isConst())
                    continue;
                for (const Token *subexpr = expr; subexpr; subexpr = subexpr->astOperand1()) {
                    if (isSameExpression(mCpp, macro, subexpr, args[argnr], mLibrary, pure, followVar)) {
                        const Scope* scope = expr->scope(); // if there is no other variable, assume no aliasing
                        if (scope->varlist.size() > 1)
                            return true;
                    }
                }
            }
            continue;
        }

        const Token *addrOf = nullptr;
        if (Token::Match(tok, "& %name% ="))
            addrOf = tok->tokAt(2)->astOperand2();
        else if (tok->isUnaryOp("&"))
            addrOf = tok->astOperand1();
        else if (Token::simpleMatch(tok, "std :: ref ("))
            addrOf = tok->tokAt(3)->astOperand2();
        else
            continue;

        for (const Token *subexpr = expr; subexpr; subexpr = subexpr->astOperand1()) {
            if (isSameExpression(mCpp, macro, subexpr, addrOf, mLibrary, pure, followVar))
                return true;
        }
    }
    return false;
}

bool FwdAnalysis::isEscapedAlias(const Token* expr)
{
    for (const Token *subexpr = expr; subexpr; subexpr = subexpr->astOperand1()) {
        for (const ValueFlow::Value &val : subexpr->values()) {
            if (!val.isLocalLifetimeValue())
                continue;
            const Variable* var = val.tokvalue->variable();
            if (!var)
                continue;
            if (!var->isLocal())
                return true;
            if (var->isArgument())
                return true;

        }
    }
    return false;
}

bool isSizeOfEtc(const Token *tok)
{
    return Token::Match(tok, "sizeof|typeof|offsetof|decltype|__typeof__ (");
}
