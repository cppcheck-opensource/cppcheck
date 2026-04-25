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
#include "astutils.h"

#include "config.h"
#include "errortypes.h"
#include "findtoken.h"
#include "infer.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "valueflow.h"
#include "valueptr.h"
#include "vfvalue.h"

#include "checkclass.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <list>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <utility>

const Token* findExpression(const nonneg int exprid,
                            const Token* start,
                            const Token* end,
                            const std::function<bool(const Token*)>& pred)
{
    if (exprid == 0)
        return nullptr;
    if (!precedes(start, end))
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
        const int argn = res;
        res = findArgumentPosRecursive(tok->astOperand2(), tokToFind, found, depth);
        if (res == -1)
            return -1;
        return argn + res;
    }
    if (tokToFind == tok)
        found = true;
    return 1;
}

static int findArgumentPos(const Token* tok, const Token* tokToFind){
    bool found = false;
    const int argn = findArgumentPosRecursive(tok, tokToFind, found, 0);
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

template<class T, class OuputIterator, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static void astFlattenCopy(T* tok, const char* op, OuputIterator out, int depth = 100)
{
    --depth;
    if (!tok || depth < 0)
        return;
    if (strcmp(tok->str().c_str(), op) == 0) {
        astFlattenCopy(tok->astOperand1(), op, out, depth);
        astFlattenCopy(tok->astOperand2(), op, out, depth);
    } else {
        *out = tok;
        ++out;
    }
}

std::vector<const Token*> astFlatten(const Token* tok, const char* op)
{
    std::vector<const Token*> result;
    astFlattenCopy(tok, op, std::back_inserter(result));
    return result;
}

std::vector<Token*> astFlatten(Token* tok, const char* op)
{
    std::vector<Token*> result;
    astFlattenCopy(tok, op, std::back_inserter(result));
    return result;
}

nonneg int astCount(const Token* tok, const char* op, int depth)
{
    --depth;
    if (!tok || depth < 0)
        return 0;
    if (strcmp(tok->str().c_str(), op) == 0)
        return astCount(tok->astOperand1(), op, depth) + astCount(tok->astOperand2(), op, depth);
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

bool astHasExpr(const Token* tok, nonneg int exprid)
{
    if (!tok)
        return false;
    if (tok->exprId() == exprid)
        return true;
    return astHasExpr(tok->astOperand1(), exprid) || astHasExpr(tok->astOperand2(), exprid);
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

bool astIsNonStringContainer(const Token* tok)
{
    const Library::Container* container = getLibraryContainer(tok);
    return container && !container->stdStringLike && !astIsIterator(tok);
}

bool astIsContainerView(const Token* tok)
{
    const Library::Container* container = getLibraryContainer(tok);
    return container && !astIsIterator(tok) && container->view;
}

bool astIsContainerOwned(const Token* tok) {
    return astIsContainer(tok) && !astIsContainerView(tok);
}

bool astIsContainerString(const Token* tok)
{
    if (!tok)
        return false;
    if (!tok->valueType())
        return false;
    const Library::Container* container = tok->valueType()->container;
    if (!container)
        return false;
    return container->stdStringLike;
}

static std::pair<const Token*, const Library::Container*> getContainerFunction(const Token* tok, const Library& library)
{
    const Library::Container* cont{};
    if (!tok || !tok->valueType() || (!tok->valueType()->container && (!(cont = library.detectContainerOrIterator(tok->valueType()->smartPointerTypeToken)))))
        return {};
    const Token* parent = tok->astParent();
    if (Token::Match(parent, ". %name% [(<]") && astIsLHS(tok)) {
        return { parent->next(), cont ? cont : tok->valueType()->container };
    }
    return {};
}

Library::Container::Action astContainerAction(const Token* tok, const Library& library, const Token** ftok)
{
    const auto ftokCont = getContainerFunction(tok, library);
    if (ftok)
        *ftok = ftokCont.first;
    if (!ftokCont.first)
        return Library::Container::Action::NO_ACTION;
    return ftokCont.second->getAction(ftokCont.first->str());
}

Library::Container::Yield astContainerYield(const Token* tok, const Library& library, const Token** ftok)
{
    const auto ftokCont = getContainerFunction(tok, library);
    if (ftok)
        *ftok = ftokCont.first;
    if (!ftokCont.first)
        return Library::Container::Yield::NO_YIELD;
    return ftokCont.second->getYield(ftokCont.first->str());
}

Library::Container::Yield astFunctionYield(const Token* tok, const Settings& settings, const Token** ftok)
{
    if (!tok)
        return Library::Container::Yield::NO_YIELD;

    const auto* function = settings.library.getFunction(tok);
    if (!function)
        return Library::Container::Yield::NO_YIELD;

    if (ftok)
        *ftok = tok;
    return function->containerYield;
}

bool astIsRangeBasedForDecl(const Token* tok)
{
    return Token::simpleMatch(tok->astParent(), ":") && Token::simpleMatch(tok->astParent()->astParent(), "(");
}

std::string astCanonicalType(const Token *expr, bool pointedToType)
{
    if (!expr)
        return "";
    std::pair<const Token*, const Token*> decl = Token::typeDecl(expr, pointedToType);
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
    if (!tok->varId() && tok->hasKnownIntValue() && MathLib::toString(tok->getKnownIntValue()) == rhs)
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
    } else if (comp == "!=" && rhs == "0") {
        if (tok->str() == "!") {
            ret = tok->astOperand1();
            // handle (!(x==0)) as (x!=0)
            astIsVariableComparison(ret, "==", "0", &ret);
        } else
            ret = tok;
    } else if (comp == "==" && rhs == "0") {
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
    const Token * const varDeclEndToken = var->declEndToken();
    return Token::Match(varDeclEndToken, "; %var%") && varDeclEndToken->next() == tok;
}

bool isStlStringType(const Token* tok)
{
    return Token::Match(tok, "std :: string|wstring|u16string|u32string !!::") ||
           (Token::simpleMatch(tok, "std :: basic_string <") && !Token::simpleMatch(tok->linkAt(3), "> ::"));
}

bool isVoidCast(const Token* tok)
{
    return Token::simpleMatch(tok, "(") && tok->isCast() && tok->valueType() &&
           tok->valueType()->type == ValueType::Type::VOID && tok->valueType()->pointer == 0;
}

bool isTemporary(const Token* tok, const Library* library, bool unknown)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "."))
        return (tok->originalName() != "->" && isTemporary(tok->astOperand1(), library)) ||
               isTemporary(tok->astOperand2(), library);
    if (Token::Match(tok, ",|::"))
        return isTemporary(tok->astOperand2(), library);
    if (tok->isCast() || (tok->isCpp() && isCPPCast(tok)))
        return isTemporary(tok->astOperand2(), library);
    if (findLambdaEndToken(tok) != nullptr)
        return true;
    if (Token::Match(tok, ".|[|++|--|%name%|%assign%"))
        return false;
    if (tok->isUnaryOp("*"))
        return false;
    if (Token::Match(tok, "&|<<|>>") && isLikelyStream(tok->astOperand1()))
        return false;
    if (Token::simpleMatch(tok, "?")) {
        const Token* branchTok = tok->astOperand2();
        if (!branchTok->astOperand1() || !branchTok->astOperand1()->valueType())
            return false;
        if (!branchTok->astOperand2()->valueType())
            return false;
        return !branchTok->astOperand1()->valueType()->isTypeEqual(branchTok->astOperand2()->valueType());
    }
    if (Token::Match(tok, "(|{") && tok->astOperand1() &&
        (tok->astOperand2() || Token::simpleMatch(tok->next(), ")"))) {
        if (Token::simpleMatch(tok->astOperand1(), "typeid"))
            return false;
        if (tok->valueType()) {
            if (tok->valueType()->pointer > 0) {
                const Token* const parent = tok->astParent();
                if (Token::simpleMatch(parent, "&"))
                    return true;
                if (Token::simpleMatch(parent, "return") && parent->valueType()->reference != Reference::None &&
                    parent->valueType()->container && parent->valueType()->container->stdStringLike)
                    return true;
            }
            return tok->valueType()->reference == Reference::None && tok->valueType()->pointer == 0;
        }
        const Token* ftok = nullptr;
        if (Token::simpleMatch(tok->previous(), ">") && tok->linkAt(-1))
            ftok = tok->linkAt(-1)->previous();
        else
            ftok = tok->previous();
        if (!ftok)
            return false;
        if (const Function * f = ftok->function())
            return !Function::returnsReference(f, true);
        if (ftok->type())
            return true;
        if (library) {
            const std::string& returnType = library->returnValueType(ftok);
            return !returnType.empty() && returnType.back() != '&';
        }
        return unknown;
    }
    if (tok->isCast())
        return false;
    // Currying a function is unknown in cppcheck
    if (Token::simpleMatch(tok, "(") && Token::simpleMatch(tok->astOperand1(), "("))
        return unknown;
    return true;
}

static bool isFunctionCall(const Token* tok)
{
    if (Token::Match(tok, "%name% ("))
        return true;
    if (Token::Match(tok, "%name% <") && Token::simpleMatch(tok->linkAt(1), "> ("))
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
    T * rightmostLeaf = tok;
    if (!rightmostLeaf || !rightmostLeaf->astOperand1())
        return nullptr;
    do {
        if (T* lam = findLambdaEndToken(rightmostLeaf)) {
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
    while (Token::Match(rightmostLeaf->next(), "]|)") && !hasToken(rightmostLeaf->linkAt(1), rightmostLeaf->next(), tok))
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
        (Token::simpleMatch(parent->previous(), "> (") && parent->linkAt(-1)))
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
    const Token* parent = tok->astParent();
    while (Token::simpleMatch(parent->astParent(), "."))
        parent = parent->astParent();
    std::vector<const Token*> result;
    for (const Token* tok2 : astFlatten(parent, ".")) {
        if (Token::simpleMatch(tok2, "(") && Token::simpleMatch(tok2->astOperand1(), ".")) {
            std::vector<const Token*> sub = getParentMembers(tok2->astOperand1());
            result.insert(result.end(), sub.cbegin(), sub.cend());
        }
        result.push_back(tok2);
    }
    return result;
}

static const Token* getParentLifetimeObject(const Token* tok)
{
    while (Token::simpleMatch(tok, "["))
        tok = tok->astOperand1();
    return tok;
}

const Token* getParentLifetime(const Token* tok, const Library& library)
{
    std::vector<const Token*> members = getParentMembers(tok);
    if (members.size() < 2)
        return tok;
    // Find the first local variable, temporary, or array
    auto it = std::find_if(members.crbegin(), members.crend(), [&](const Token* tok2) {
        const Variable* var = tok2->variable();
        if (var)
            return var->isLocal() || var->isArgument();
        if (Token::simpleMatch(tok2, "["))
            return true;
        return isTemporary(tok2, &library);
    });
    if (it == members.rend())
        return tok;
    // If any of the submembers are borrowed types then stop
    if (std::any_of(it.base() - 1, members.cend() - 1, [&](const Token* tok2) {
        const Token* obj = getParentLifetimeObject(tok2);
        if (!obj)
            return false;
        const Variable* var = obj->variable();
        // Check for arrays first since astIsPointer will return true, but an array is not a borrowed type
        if (var && var->isArray())
            return false;
        if (astIsPointer(obj) || astIsContainerView(obj) || astIsIterator(obj))
            return true;
        if (!astIsUniqueSmartPointer(obj)) {
            if (astIsSmartPointer(obj))
                return true;
            const Token* dotTok = obj->next();
            if (!Token::simpleMatch(dotTok, ".")) {
                const Token* endTok = nextAfterAstRightmostLeaf(obj);
                if (!endTok)
                    dotTok = obj->next();
                else if (Token::simpleMatch(endTok, "."))
                    dotTok = endTok;
                else if (Token::simpleMatch(endTok->next(), "."))
                    dotTok = endTok->next();
            }
            // If we are dereferencing the member variable then treat it as borrowed
            if (Token::simpleMatch(dotTok, ".") && dotTok->originalName() == "->")
                return true;
        }
        return var && var->isReference();
    }))
        return nullptr;
    const Token* result = getParentLifetimeObject(*it);
    if (result != *it)
        return getParentLifetime(result);
    return result;
}

static bool isInConstructorList(const Token* tok)
{
    if (!tok)
        return false;
    if (!astIsRHS(tok))
        return false;
    const Token* parent = tok->astParent();
    if (!Token::Match(parent, "{|("))
        return false;
    if (!Token::Match(parent->previous(), "%var% {|("))
        return false;
    if (!parent->astOperand1() || !parent->astOperand2())
        return false;
    do {
        parent = parent->astParent();
    } while (Token::simpleMatch(parent, ","));
    return Token::simpleMatch(parent, ":") && !Token::simpleMatch(parent->astParent(), "?");
}

std::vector<ValueType> getParentValueTypes(const Token* tok, const Settings& settings, const Token** parent)
{
    if (!tok)
        return {};
    if (!tok->astParent())
        return {};
    if (isInConstructorList(tok)) {
        if (parent)
            *parent = tok->astParent()->astOperand1();
        if (tok->astParent()->astOperand1()->valueType())
            return {*tok->astParent()->astOperand1()->valueType()};
        return {};
    }
    const Token* ftok = nullptr;
    if (Token::Match(tok->astParent(), "(|{|,")) {
        int argn = -1;
        ftok = getTokenArgumentFunction(tok, argn);
        const Token* typeTok = nullptr;
        if (ftok && argn >= 0) {
            if (ftok->function()) {
                std::vector<ValueType> result;
                const Token* nameTok = nullptr;
                for (const Variable* var : getArgumentVars(ftok, argn)) {
                    if (!var)
                        continue;
                    if (!var->valueType())
                        continue;
                    nameTok = var->nameToken();
                    result.push_back(*var->valueType());
                    if (var->isArray())
                        result.back().pointer += var->dimensions().size();
                }
                if (result.size() == 1 && nameTok && parent) {
                    *parent = nameTok;
                }
                return result;
            }
            if (const Type* t = Token::typeOf(ftok, &typeTok)) {
                if (astIsPointer(typeTok))
                    return {*typeTok->valueType()};
                const Scope* scope = t->classScope;
                // Check for aggregate constructors
                if (scope && scope->numConstructors == 0 && t->derivedFrom.empty() &&
                    (t->isClassType() || t->isStructType()) && numberOfArguments(ftok) <= scope->varlist.size() &&
                    !scope->varlist.empty()) {
                    if (argn < scope->varlist.size()) {
                        auto it = std::next(scope->varlist.cbegin(), argn);
                        if (it->valueType())
                            return { *it->valueType() };
                    }
                }
            }
        }
    }
    if (Token::Match(tok->astParent()->tokAt(-2), ". push_back|push_front|insert|push (") &&
        astIsContainer(tok->astParent()->tokAt(-2)->astOperand1())) {
        const Token* contTok = tok->astParent()->tokAt(-2)->astOperand1();
        const ValueType* vtCont = contTok->valueType();
        if (!vtCont->containerTypeToken)
            return {};
        ValueType vtParent = ValueType::parseDecl(vtCont->containerTypeToken, settings);
        return {std::move(vtParent)};
    }
    // The return type of a function is not the parent valuetype
    if (Token::simpleMatch(tok->astParent(), "(") && ftok && !tok->astParent()->isCast() &&
        ftok->tokType() != Token::eType)
        return {};
    if (parent && Token::Match(tok->astParent(), "return|(|{|%assign%")) {
        *parent = tok->astParent();
    }
    if (tok->astParent()->valueType())
        return {*tok->astParent()->valueType()};
    return {};
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
    if (Token::simpleMatch(tok, "do {")) {
        T* endTok = tok->linkAt(1);
        if (Token::simpleMatch(endTok, "} while ("))
            return endTok->tokAt(2)->astOperand2();
    }
    if (Token::simpleMatch(tok, "for") && Token::simpleMatch(tok->next()->astOperand2(), ";") &&
        tok->next()->astOperand2()->astOperand2())
        return tok->next()->astOperand2()->astOperand2()->astOperand1();
    if (Token::simpleMatch(tok->next()->astOperand2(), ";"))
        return tok->next()->astOperand2()->astOperand1();
    if (tok->isName() && !tok->isControlFlowKeyword())
        return nullptr;
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
    if (Token::simpleMatch(startBlock->previous(), "do"))
        return getCondTok(startBlock->previous());
    if (Token::simpleMatch(startBlock->previous(), ")"))
        return getCondTok(startBlock->linkAt(-1));
    if (Token::simpleMatch(startBlock->tokAt(-2), "} else {"))
        return getCondTokFromEnd(startBlock->tokAt(-2));
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
        if (scope->isLoopScope() || scope->type == ScopeType::eSwitch) {
            if (scope->type == ScopeType::eDo && Token::simpleMatch(scope->bodyEnd, "} while ("))
                return scope->bodyEnd->linkAt(2)->next();
            return scope->bodyEnd;
        }
        scope = scope->nestedIn;
    }
    return nullptr;
}

bool extractForLoopValues(const Token *forToken,
                          nonneg int &varid,
                          bool &knownInitValue,
                          MathLib::bigint &initValue,
                          bool &partialCond,
                          MathLib::bigint &stepValue,
                          MathLib::bigint &lastValue)
{
    if (!Token::simpleMatch(forToken, "for (") || !Token::simpleMatch(forToken->next()->astOperand2(), ";"))
        return false;
    const Token *initExpr = forToken->next()->astOperand2()->astOperand1();
    const Token *condExpr = forToken->next()->astOperand2()->astOperand2()->astOperand1();
    const Token *incExpr  = forToken->next()->astOperand2()->astOperand2()->astOperand2();
    if (!initExpr || !initExpr->isBinaryOp() || initExpr->str() != "=" || !Token::Match(initExpr->astOperand1(), "%var%"))
        return false;
    std::vector<MathLib::bigint> minInitValue =
        getMinValue(makeIntegralInferModel(), initExpr->astOperand2()->values());
    if (minInitValue.empty()) {
        const ValueFlow::Value* v = initExpr->astOperand2()->getMinValue(true);
        if (v)
            minInitValue.push_back(v->intvalue);
    }
    if (minInitValue.empty())
        return false;
    varid = initExpr->astOperand1()->varId();
    knownInitValue = initExpr->astOperand2()->hasKnownIntValue();
    initValue = minInitValue.front();
    partialCond = Token::Match(condExpr, "%oror%|&&");
    visitAstNodes(condExpr, [varid, &condExpr](const Token *tok) {
        if (Token::Match(tok, "%oror%|&&"))
            return ChildrenToVisit::op1_and_op2;
        if (Token::Match(tok, "<|<=") && tok->isBinaryOp() && tok->astOperand1()->varId() == varid && tok->astOperand2()->hasKnownIntValue()) {
            if (Token::Match(condExpr, "%oror%|&&") || tok->astOperand2()->getKnownIntValue() < condExpr->astOperand2()->getKnownIntValue())
                condExpr = tok;
        }
        return ChildrenToVisit::none;
    });
    if (!Token::Match(condExpr, "<|<=") || !condExpr->isBinaryOp() || condExpr->astOperand1()->varId() != varid || !condExpr->astOperand2()->hasKnownIntValue())
        return false;
    if (!incExpr || !incExpr->isUnaryOp("++") || incExpr->astOperand1()->varId() != varid)
        return false;
    stepValue = 1;
    if (condExpr->str() == "<")
        lastValue = condExpr->astOperand2()->getKnownIntValue() - 1;
    else
        lastValue = condExpr->astOperand2()->getKnownIntValue();
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

const Token* isInLoopCondition(const Token* tok)
{
    const Token* top = tok->astTop();
    return Token::Match(top->previous(), "for|while (") ? top : nullptr;
}

bool isDesignatedInitializer(const Token* tok)
{
    return tok && tok->isUnaryOp(".");
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
    // NOLINTNEXTLINE(readability-use-anyofallof) - TODO: fix this / also Cppcheck false negative
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

bool isAliasOf(const Token* tok, const Token* expr, nonneg int* indirect)
{
    if (indirect)
        *indirect = 1;
    if (!tok)
        return false;
    const Token* r = nullptr;
    for (const ReferenceToken& ref : tok->refs()) {
        const bool pointer = astIsPointer(ref.token);
        r = findAstNode(expr, [&](const Token* childTok) {
            if (childTok->exprId() == 0)
                return false;
            if (ref.token != tok && expr->exprId() == childTok->exprId() && ref.token->isUnaryOp("*") && expr->exprId() == ref.token->astOperand1()->exprId()) {
                if (indirect)
                    *indirect = 0;
                return true;
            }
            for (const ValueFlow::Value& val : ref.token->values()) {
                if (val.isImpossible())
                    continue;
                if (val.isLocalLifetimeValue() || (pointer && val.isSymbolicValue() && val.intvalue == 0)) {
                    if (findAstNode(val.tokvalue,
                                    [&](const Token* aliasTok) {
                        return aliasTok != childTok && aliasTok->exprId() == childTok->exprId();
                    })) {
                        return true;
                    }
                }
            }
            return false;
        });
        if (r)
            break;
    }
    return r;
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
    if (Token::Match(expr, "%name% (")) {
        if (expr->function() && expr->function()->nestedIn && expr->function()->nestedIn->isClassOrStruct() && !expr->function()->isStatic()) {
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
        }
        if (expr->isOperatorKeyword() && !Token::simpleMatch(expr->next()->astParent(), "."))
            return true;
    }
    if (onVar && expr->variable()) {
        const Variable* var = expr->variable();
        return ((var->isPrivate() || var->isPublic() || var->isProtected()) && !var->isStatic());
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
    if (!var || var->isArray())
        return false;
    const Token* tok = var->nameToken();
    while (tok && Token::Match(tok->astParent(), "[|,|:"))
        tok = tok->astParent();
    return tok && (tok->str() == "[" || Token::simpleMatch(tok->previous(), "] :")); // TODO: remove workaround when #11105 is fixed
}

/// This takes a token that refers to a variable and it will return the token
/// to the expression that the variable is assigned to. If its not valid to
/// make such substitution then it will return the original token.
static const Token * followVariableExpression(const Settings& settings, const Token * tok, const Token * end = nullptr)
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
    if (astIsRangeBasedForDecl(var->nameToken()))
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
    const int indirect = var->isArray() ? var->dimensions().size() : 0;
    if (!var->isConst() && (!precedes(varTok, endToken) || isVariableChanged(varTok, endToken, indirect, tok->varId(), false, settings)))
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
    } else if (findExpressionChanged(varTok, startToken, endToken, settings)) {
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
    if (std::find(errors->cbegin(), errors->cend(), item) != errors->cend())
        return;
    errors->push_back(std::move(item));
}

static SmallVector<ReferenceToken> followAllReferencesInternal(const Token* tok,
                                                               bool temporary = true,
                                                               bool inconclusive = true,
                                                               ErrorPath errors = ErrorPath{},
                                                               int depth = 20)
{
    struct ReferenceTokenLess {
        bool operator()(const ReferenceToken& x, const ReferenceToken& y) const {
            return x.token < y.token;
        }
    };
    if (!tok)
        return {};
    if (depth < 0) {
        SmallVector<ReferenceToken> refs_result;
        refs_result.emplace_back(tok, std::move(errors));
        return refs_result;
    }
    const Variable *var = tok->variable();
    if (var && var->declarationId() == tok->varId()) {
        if (var->nameToken() == tok || isStructuredBindingVariable(var)) {
            SmallVector<ReferenceToken> refs_result;
            refs_result.emplace_back(tok, std::move(errors));
            return refs_result;
        }
        if (var->isReference() || var->isRValueReference()) {
            const Token * const varDeclEndToken = var->declEndToken();
            if (!varDeclEndToken) {
                SmallVector<ReferenceToken> refs_result;
                refs_result.emplace_back(tok, std::move(errors));
                return refs_result;
            }
            if (var->isArgument()) {
                errors.emplace_back(varDeclEndToken, "Passed to reference.");
                SmallVector<ReferenceToken> refs_result;
                refs_result.emplace_back(tok, std::move(errors));
                return refs_result;
            }
            if (Token::simpleMatch(varDeclEndToken, "=")) {
                if (astHasToken(varDeclEndToken, tok))
                    return {};
                errors.emplace_back(varDeclEndToken, "Assigned to reference.");
                const Token *vartok = varDeclEndToken->astOperand2();
                if (vartok == tok || (!temporary && isTemporary(vartok, nullptr, true) &&
                                      (var->isConst() || var->isRValueReference()))) {
                    SmallVector<ReferenceToken> refs_result;
                    refs_result.emplace_back(tok, std::move(errors));
                    return refs_result;
                }
                if (vartok)
                    return followAllReferencesInternal(vartok, temporary, inconclusive, std::move(errors), depth - 1);
            }
        }
    } else if (Token::simpleMatch(tok, "?") && Token::simpleMatch(tok->astOperand2(), ":")) {
        std::set<ReferenceToken, ReferenceTokenLess> result;
        const Token* tok2 = tok->astOperand2();

        auto refs = followAllReferencesInternal(tok2->astOperand1(), temporary, inconclusive, errors, depth - 1);
        result.insert(refs.cbegin(), refs.cend());
        refs = followAllReferencesInternal(tok2->astOperand2(), temporary, inconclusive, errors, depth - 1);
        result.insert(refs.cbegin(), refs.cend());

        if (!inconclusive && result.size() != 1) {
            SmallVector<ReferenceToken> refs_result;
            refs_result.emplace_back(tok, std::move(errors));
            return refs_result;
        }

        if (!result.empty()) {
            SmallVector<ReferenceToken> refs_result;
            refs_result.insert(refs_result.end(), result.cbegin(), result.cend());
            return refs_result;
        }

    } else if (tok->previous() && tok->previous()->function() && Token::Match(tok->previous(), "%name% (")) {
        const Function *f = tok->previous()->function();
        if (!Function::returnsReference(f)) {
            SmallVector<ReferenceToken> refs_result;
            refs_result.emplace_back(tok, std::move(errors));
            return refs_result;
        }
        std::set<ReferenceToken, ReferenceTokenLess> result;
        std::vector<const Token*> returns = Function::findReturns(f);
        for (const Token* returnTok : returns) {
            if (returnTok == tok)
                continue;
            for (const ReferenceToken& rt :
                 followAllReferencesInternal(returnTok, temporary, inconclusive, errors, depth - returns.size())) {
                const Variable* argvar = rt.token->variable();
                if (!argvar) {
                    SmallVector<ReferenceToken> refs_result;
                    refs_result.emplace_back(tok, std::move(errors));
                    return refs_result;
                }
                if (argvar->isArgument() && (argvar->isReference() || argvar->isRValueReference())) {
                    const int n = getArgumentPos(argvar, f);
                    if (n < 0) {
                        SmallVector<ReferenceToken> refs_result;
                        refs_result.emplace_back(tok, std::move(errors));
                        return refs_result;
                    }
                    std::vector<const Token*> args = getArguments(tok->previous());
                    if (n >= args.size()) {
                        SmallVector<ReferenceToken> refs_result;
                        refs_result.emplace_back(tok, std::move(errors));
                        return refs_result;
                    }
                    const Token* argTok = args[n];
                    ErrorPath er = errors;
                    er.emplace_back(returnTok, "Return reference.");
                    er.emplace_back(tok->previous(), "Called function passing '" + argTok->expressionString() + "'.");
                    auto refs =
                        followAllReferencesInternal(argTok, temporary, inconclusive, std::move(er), depth - returns.size());
                    result.insert(refs.cbegin(), refs.cend());
                    if (!inconclusive && result.size() > 1) {
                        SmallVector<ReferenceToken> refs_result;
                        refs_result.emplace_back(tok, std::move(errors));
                        return refs_result;
                    }
                }
            }
        }
        if (!result.empty()) {
            SmallVector<ReferenceToken> refs_result;
            refs_result.insert(refs_result.end(), result.cbegin(), result.cend());
            return refs_result;
        }
    }
    SmallVector<ReferenceToken> refs_result;
    refs_result.emplace_back(tok, std::move(errors));
    return refs_result;
}

SmallVector<ReferenceToken> followAllReferences(const Token* tok, bool temporary)
{
    return followAllReferencesInternal(tok, temporary);
}

const Token* followReferences(const Token* tok, ErrorPath* errors)
{
    if (!tok)
        return nullptr;
    auto refs = followAllReferencesInternal(tok, true, false);
    if (refs.size() == 1) {
        if (errors)
            *errors = std::move(refs.front().errors);
        return refs.front().token;
    }
    return nullptr;
}

static bool isSameLifetime(const Token * const tok1, const Token * const tok2)
{
    ValueFlow::Value v1 = ValueFlow::getLifetimeObjValue(tok1);
    if (!v1.isLifetimeValue())
        return false;
    ValueFlow::Value v2 = ValueFlow::getLifetimeObjValue(tok2);
    if (!v2.isLifetimeValue())
        return false;
    return v1.tokvalue == v2.tokvalue;
}

static bool compareKnownValue(const Token * const tok1, const Token * const tok2, const std::function<bool(const ValueFlow::Value&, const ValueFlow::Value&, bool)> &compare)
{
    static const auto isKnownFn = std::mem_fn(&ValueFlow::Value::isKnown);

    const auto v1 = std::find_if(tok1->values().cbegin(), tok1->values().cend(), isKnownFn);
    if (v1 == tok1->values().end()) {
        return false;
    }
    if (v1->isNonValue() || v1->isContainerSizeValue() || v1->isBufferSizeValue() || v1->isSymbolicValue())
        return false;
    const auto v2 = std::find_if(tok2->values().cbegin(), tok2->values().cend(), isKnownFn);
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
        if (tok->astOperand2() && Token::Match(tok->previous(), "%type% (|{") && tok->previous()->isStandardType())
            return tok->astOperand2();
        return tok;
    };

    tok1 = adjustForCast(tok1);
    if (!tok1->isNumber() && !tok1->enumerator())
        return false;
    tok2 = adjustForCast(tok2);
    if (!tok2->isNumber() && !tok2->enumerator())
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

static bool isForLoopIncrement(const Token* const tok)
{
    if (!tok)
        return false;
    const Token *const parent = tok->astParent();
    return Token::simpleMatch(parent, ";") && parent->astOperand2() == tok &&
           Token::simpleMatch(parent->astParent(), ";") &&
           Token::simpleMatch(parent->astParent()->astParent(), "(") &&
           parent->astParent()->astParent()->astOperand1()->str() == "for";
}

bool isUsedAsBool(const Token* const tok, const Settings& settings)
{
    if (!tok)
        return false;
    if (isForLoopIncrement(tok))
        return false;
    if (astIsBool(tok))
        return true;
    if (Token::Match(tok, "!|&&|%oror%|%comp%"))
        return true;
    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    if (Token::simpleMatch(parent, "["))
        return false;
    if (parent->isUnaryOp("*"))
        return false;
    if (Token::simpleMatch(parent, ".")) {
        if (astIsRHS(tok))
            return isUsedAsBool(parent, settings);
        return false;
    }
    if (Token::Match(parent, "&&|!|%oror%"))
        return true;
    if (parent->isCast())
        return !Token::simpleMatch(parent->astOperand1(), "dynamic_cast") && isUsedAsBool(parent, settings);
    if (parent->isUnaryOp("*"))
        return isUsedAsBool(parent, settings);
    if (Token::Match(parent, "==|!=") && tok->valueType() && tok->valueType()->pointer &&
        tok->astSibling()->hasKnownIntValue() && tok->astSibling()->getKnownIntValue() == 0)
        return true;
    if (parent->str() == "(" && astIsRHS(tok) && Token::Match(parent->astOperand1(), "if|while"))
        return true;
    if (Token::simpleMatch(parent, "?") && astIsLHS(tok))
        return true;
    if (isForLoopCondition(tok))
        return true;
    if (!Token::Match(parent, "%cop%") && !(parent->str() == "(" && tok == parent->astOperand1())) {
        if (parent->str() == "," && parent->isInitComma())
            return false;
        std::vector<ValueType> vtParents = getParentValueTypes(tok, settings);
        return std::any_of(vtParents.cbegin(), vtParents.cend(), [&](const ValueType& vt) {
            return vt.pointer == 0 && vt.type == ValueType::BOOL;
        });
    }
    return false;
}

bool compareTokenFlags(const Token* tok1, const Token* tok2, bool macro) {
    if (macro) {
        if (tok1->isExpandedMacro() != tok2->isExpandedMacro())
            return false;
        if (tok1->isExpandedMacro()) { // both are macros
            if (tok1->getMacroName() != tok2->getMacroName())
                return false;
            if (tok1->astParent() && tok2->astParent() && tok1->astParent()->isExpandedMacro() && tok1->astParent()->getMacroName() == tok2->astParent()->getMacroName())
                return false;
        }
        if (tok1->isTemplateArg() || tok2->isTemplateArg())
            return false;
    }
    if (tok1->isComplex() != tok2->isComplex())
        return false;
    if (tok1->isLong() != tok2->isLong())
        return false;
    if (tok1->isUnsigned() != tok2->isUnsigned())
        return false;
    if (tok1->isSigned() != tok2->isSigned())
        return false;
    return true;
}

static bool astIsBoolLike(const Token* tok, const Settings& settings)
{
    return astIsBool(tok) || isUsedAsBool(tok, settings);
}

bool isSameExpression(bool macro, const Token *tok1, const Token *tok2, const Settings& settings, bool pure, bool followVar, ErrorPath* errors)
{
    if (tok1 == tok2)
        return true;
    if (tok1 == nullptr || tok2 == nullptr)
        return false;
    // tokens needs to be from the same TokenList so no need check standard on both of them
    if (tok1->isCpp()) {
        if (tok1->str() == "." && tok1->astOperand1() && tok1->astOperand1()->str() == "this")
            tok1 = tok1->astOperand2();
        if (tok2->str() == "." && tok2->astOperand1() && tok2->astOperand1()->str() == "this")
            tok2 = tok2->astOperand2();
    }
    // Skip double not
    if (Token::simpleMatch(tok1, "!") && Token::simpleMatch(tok1->astOperand1(), "!") && !Token::simpleMatch(tok1->astParent(), "=") && astIsBoolLike(tok2, settings)) {
        return isSameExpression(macro, tok1->astOperand1()->astOperand1(), tok2, settings, pure, followVar, errors);
    }
    if (Token::simpleMatch(tok2, "!") && Token::simpleMatch(tok2->astOperand1(), "!") && !Token::simpleMatch(tok2->astParent(), "=") && astIsBoolLike(tok1, settings)) {
        return isSameExpression(macro, tok1, tok2->astOperand1()->astOperand1(), settings, pure, followVar, errors);
    }
    const bool tok_str_eq = tok1->str() == tok2->str();
    if (!tok_str_eq && isDifferentKnownValues(tok1, tok2))
        return false;

    const Token *followTok1 = tok1, *followTok2 = tok2;
    while (Token::simpleMatch(followTok1, "::"))
        followTok1 = followTok1->astOperand2() ? followTok1->astOperand2() : followTok1->astOperand1();
    if (!followTok1)
        followTok1 = tok1; // TODO: remove after #14235 has been fixed
    while (Token::simpleMatch(followTok2, "::"))
        followTok2 = followTok2->astOperand2() ? followTok2->astOperand2() : followTok2->astOperand1();
    if (!followTok2)
        followTok2 = tok2;
    if (isSameConstantValue(macro, followTok1, followTok2))
        return true;

    // Follow variable
    if (followVar && !tok_str_eq && (followTok1->varId() || followTok2->varId() || followTok1->enumerator() || followTok2->enumerator())) {
        const Token * varTok1 = followVariableExpression(settings, followTok1, followTok2);
        if ((varTok1->str() == followTok2->str()) || isSameConstantValue(macro, varTok1, followTok2)) {
            followVariableExpressionError(followTok1, varTok1, errors);
            return isSameExpression(macro, varTok1, followTok2, settings, true, followVar, errors);
        }
        const Token * varTok2 = followVariableExpression(settings, followTok2, followTok1);
        if ((followTok1->str() == varTok2->str()) || isSameConstantValue(macro, followTok1, varTok2)) {
            followVariableExpressionError(followTok2, varTok2, errors);
            return isSameExpression(macro, followTok1, varTok2, settings, true, followVar, errors);
        }
        if ((varTok1->str() == varTok2->str()) || isSameConstantValue(macro, varTok1, varTok2)) {
            followVariableExpressionError(tok1, varTok1, errors);
            followVariableExpressionError(tok2, varTok2, errors);
            return isSameExpression(macro, varTok1, varTok2, settings, true, followVar, errors);
        }
    }
    // Follow references
    if (!tok_str_eq) {
        const Token* refTok1 = followReferences(tok1, errors);
        const Token* refTok2 = followReferences(tok2, errors);
        if (refTok1 != tok1 || refTok2 != tok2) {
            if (refTok1 && !refTok1->varId() && refTok2 && !refTok2->varId()) { // complex reference expression
                const Token *start = refTok1, *end = refTok2;
                if (!precedes(start, end))
                    std::swap(start, end);
                if (findExpressionChanged(start, start, end, settings))
                    return false;
            }
            return isSameExpression(macro, refTok1, refTok2, settings, pure, followVar, errors);
        }
    }
    if (tok1->varId() != tok2->varId() || !tok_str_eq || tok1->originalName() != tok2->originalName()) {
        if ((Token::Match(tok1,"<|>") && Token::Match(tok2,"<|>")) ||
            (Token::Match(tok1,"<=|>=") && Token::Match(tok2,"<=|>="))) {
            return isSameExpression(macro, tok1->astOperand1(), tok2->astOperand2(), settings, pure, followVar, errors) &&
                   isSameExpression(macro, tok1->astOperand2(), tok2->astOperand1(), settings, pure, followVar, errors);
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
            if (const ValueFlow::Value* vi1 = condTok->astOperand1()->getKnownValue(ValueFlow::Value::ValueType::INT)) {
                value = vi1;
                varTok1 = condTok->astOperand2();
            } else if (const ValueFlow::Value* vi2 = condTok->astOperand2()->getKnownValue(ValueFlow::Value::ValueType::INT)) {
                value = vi2;
                varTok1 = condTok->astOperand1();
            }
            const bool exprIsNot = Token::simpleMatch(exprTok, "!");
            if (exprIsNot)
                varTok2 = exprTok->astOperand1();
            bool compare = false;
            if (value) {
                if (value->intvalue == 0 && exprIsNot && Token::simpleMatch(condTok, "==")) {
                    compare = true;
                } else if (value->intvalue == 0 && !exprIsNot && Token::simpleMatch(condTok, "!=")) {
                    compare = true;
                } else if (value->intvalue != 0 && exprIsNot && Token::simpleMatch(condTok, "!=")) {
                    compare = true;
                } else if (value->intvalue != 0 && !exprIsNot && Token::simpleMatch(condTok, "==")) {
                    compare = true;
                }
            }
            if (compare && astIsBoolLike(varTok1, settings) && astIsBoolLike(varTok2, settings))
                return isSameExpression(macro, varTok1, varTok2, settings, pure, followVar, errors);

        }
        return false;
    }

    if (!compareTokenFlags(tok1, tok2, macro))
        return false;

    if (pure && tok1->isName() && tok1->strAt(1) == "(" && tok1->str() != "sizeof" && !(tok1->variable() && tok1 == tok1->variable()->nameToken())) {
        if (!tok1->function()) {
            if (Token::simpleMatch(tok1->previous(), ".")) {
                const Token *lhs = tok1->previous();
                while (Token::Match(lhs, "(|.|["))
                    lhs = lhs->astOperand1();
                if (!lhs)
                    return false;
                const bool lhsIsConst = (lhs->variable() && lhs->variable()->isConst()) ||
                                        (lhs->valueType() && lhs->valueType()->constness > 0) ||
                                        (Token::Match(lhs, "%var% . %name% (") && settings.library.isFunctionConst(lhs->tokAt(2)));
                if (!lhsIsConst)
                    return false;
            } else {
                const Token * ftok = tok1;
                if (!settings.library.isFunctionConst(ftok) && !ftok->isAttributeConst() && !ftok->isAttributePure())
                    return false;
            }
        } else {
            if (!tok1->function()->isConst() && !tok1->function()->isAttributeConst() &&
                !tok1->function()->isAttributePure())
                return false;
        }
    }
    // templates/casts
    if ((tok1->next() && tok1->linkAt(1) && Token::Match(tok1, "%name% <")) ||
        (tok2->next() && tok2->linkAt(1) && Token::Match(tok2, "%name% <"))) {

        // non-const template function that is not a dynamic_cast => return false
        if (pure && Token::simpleMatch(tok1->linkAt(1), "> (") &&
            !(tok1->function() && tok1->function()->isConst()) &&
            tok1->str() != "dynamic_cast")
            return false;

        // some template/cast stuff.. check that the template arguments are same
        const Token *t1 = tok1->next();
        const Token *t2 = tok2->next();
        const Token *end1 = t1->link();
        const Token *end2 = t2->link();
        while (t1 && t2 && t1 != end1 && t2 != end2) {
            if (t1->str() != t2->str() || !compareTokenFlags(t1, t2, macro))
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
        !(tok1->strAt(-1) == ">" && tok1->linkAt(-1))) {
        const Token *t1 = tok1->next();
        const Token *t2 = tok2->next();
        while (t1 && t2 &&
               t1->str() == t2->str() &&
               compareTokenFlags(t1, t2, macro) &&
               (t1->isName() || t1->str() == "*")) {
            t1 = t1->next();
            t2 = t2->next();
        }
        if (!t1 || !t2 || t1->str() != ")" || t2->str() != ")")
            return false;
    }
    bool noncommutativeEquals =
        isSameExpression(macro, tok1->astOperand1(), tok2->astOperand1(), settings, pure, followVar, errors);
    noncommutativeEquals = noncommutativeEquals &&
                           isSameExpression(macro, tok1->astOperand2(), tok2->astOperand2(), settings, pure, followVar, errors);

    if (noncommutativeEquals)
        return true;

    // in c++, a+b might be different to b+a, depending on the type of a and b
    if (tok1->isCpp() && tok1->str() == "+" && tok1->isBinaryOp()) {
        const ValueType* vt1 = tok1->astOperand1()->valueType();
        const ValueType* vt2 = tok1->astOperand2()->valueType();
        if (!(vt1 && (vt1->type >= ValueType::VOID || vt1->pointer) && vt2 && (vt2->type >= ValueType::VOID || vt2->pointer)))
            return false;
    }

    const bool commutative = tok1->isBinaryOp() && Token::Match(tok1, "%or%|%oror%|+|*|&|&&|^|==|!=");
    bool commutativeEquals = commutative &&
                             isSameExpression(macro, tok1->astOperand2(), tok2->astOperand1(), settings, pure, followVar, errors);
    commutativeEquals = commutativeEquals &&
                        isSameExpression(macro, tok1->astOperand1(), tok2->astOperand2(), settings, pure, followVar, errors);


    return commutativeEquals;
}

static bool isZeroBoundCond(const Token * const cond, bool reverse)
{
    if (cond == nullptr || !cond->isBinaryOp())
        return false;

    const Token* op = reverse ? cond->astOperand1() : cond->astOperand2();
    if (!op->hasKnownIntValue())
        return false;

    // Assume unsigned
    const bool isZero = op->getKnownIntValue() == 0;
    std::string cmp = cond->str();
    if (reverse) {
        if (cmp[0] == '>')
            cmp[0] = '<';
        else if (cmp[0] == '<')
            cmp[0] = '>';
    }

    if (cmp == "==" || cmp == ">=")
        return isZero;
    if (cmp == "<=")
        return true;
    if (cmp == "<")
        return !isZero;
    return false;
}

bool isOppositeCond(bool isNot, const Token * const cond1, const Token * const cond2, const Settings& settings, bool pure, bool followVar, ErrorPath* errors)
{
    if (!cond1 || !cond2)
        return false;

    if (isSameExpression(true, cond1, cond2, settings, pure, followVar, errors))
        return false;

    if (!isNot && cond1->str() == "&&" && cond2->str() == "&&") {
        for (const Token* tok1: {
            cond1->astOperand1(), cond1->astOperand2()
        }) {
            for (const Token* tok2: {
                cond2->astOperand1(), cond2->astOperand2()
            }) {
                if (isSameExpression(true, tok1, tok2, settings, pure, followVar, errors)) {
                    if (isOppositeCond(isNot, tok1->astSibling(), tok2->astSibling(), settings, pure, followVar, errors))
                        return true;
                }
            }
        }
    }

    if (cond1->str() != cond2->str() && (cond1->str() == "||" || cond2->str() == "||")) {
        const Token* orCond = nullptr;
        const Token* otherCond = nullptr;
        if (cond1->str() == "||") {
            orCond = cond1;
            otherCond = cond2;
        }
        if (cond2->str() == "||") {
            orCond = cond2;
            otherCond = cond1;
        }
        return isOppositeCond(isNot, orCond->astOperand1(), otherCond, settings, pure, followVar, errors) &&
               isOppositeCond(isNot, orCond->astOperand2(), otherCond, settings, pure, followVar, errors);
    }

    if (cond1->str() == "!") {
        if (cond2->str() == "!=") {
            if (cond2->astOperand1() && cond2->astOperand1()->str() == "0")
                return isSameExpression(true, cond1->astOperand1(), cond2->astOperand2(), settings, pure, followVar, errors);
            if (cond2->astOperand2() && cond2->astOperand2()->str() == "0")
                return isSameExpression(true, cond1->astOperand1(), cond2->astOperand1(), settings, pure, followVar, errors);
        }
        if (!isUsedAsBool(cond2, settings))
            return false;
        return isSameExpression(true, cond1->astOperand1(), cond2, settings, pure, followVar, errors);
    }

    if (cond2->str() == "!")
        return isOppositeCond(isNot, cond2, cond1, settings, pure, followVar, errors);

    if (!isNot) {
        if (cond1->str() == "==" && cond2->str() == "==") {
            if (isSameExpression(true, cond1->astOperand1(), cond2->astOperand1(), settings, pure, followVar, errors))
                return isDifferentKnownValues(cond1->astOperand2(), cond2->astOperand2());
            if (isSameExpression(true, cond1->astOperand2(), cond2->astOperand2(), settings, pure, followVar, errors))
                return isDifferentKnownValues(cond1->astOperand1(), cond2->astOperand1());
        }

        if (Library::isContainerYield(cond1, Library::Container::Yield::EMPTY, "empty") &&
            Library::isContainerYield(cond2->astOperand1(), Library::Container::Yield::SIZE, "size") &&
            isSameExpression(true,
                             cond1->astOperand1()->astOperand1(),
                             cond2->astOperand1()->astOperand1()->astOperand1(),
                             settings,
                             pure,
                             followVar,
                             errors)) {
            return !isZeroBoundCond(cond2, false);
        }

        if (Library::isContainerYield(cond1, Library::Container::Yield::EMPTY, "empty") &&
            Library::isContainerYield(cond2->astOperand2(), Library::Container::Yield::SIZE, "size") &&
            isSameExpression(true,
                             cond1->astOperand1()->astOperand1(),
                             cond2->astOperand2()->astOperand1()->astOperand1(),
                             settings,
                             pure,
                             followVar,
                             errors)) {
            return !isZeroBoundCond(cond2, true);
        }

        if (Library::isContainerYield(cond2, Library::Container::Yield::EMPTY, "empty") &&
            Library::isContainerYield(cond1->astOperand1(), Library::Container::Yield::SIZE, "size") &&
            isSameExpression(true,
                             cond2->astOperand1()->astOperand1(),
                             cond1->astOperand1()->astOperand1()->astOperand1(),
                             settings,
                             pure,
                             followVar,
                             errors)) {
            return !isZeroBoundCond(cond1, false);
        }

        if (Library::isContainerYield(cond2, Library::Container::Yield::EMPTY, "empty") &&
            Library::isContainerYield(cond1->astOperand2(), Library::Container::Yield::SIZE, "size") &&
            isSameExpression(true,
                             cond2->astOperand1()->astOperand1(),
                             cond1->astOperand2()->astOperand1()->astOperand1(),
                             settings,
                             pure,
                             followVar,
                             errors)) {
            return !isZeroBoundCond(cond1, true);
        }
    }


    if (!cond1->isComparisonOp() || !cond2->isComparisonOp())
        return false;

    const std::string &comp1 = cond1->str();

    // condition found .. get comparator
    std::string comp2;
    if (isSameExpression(true, cond1->astOperand1(), cond2->astOperand1(), settings, pure, followVar, errors) &&
        isSameExpression(true, cond1->astOperand2(), cond2->astOperand2(), settings, pure, followVar, errors)) {
        comp2 = cond2->str();
    } else if (isSameExpression(true, cond1->astOperand1(), cond2->astOperand2(), settings, pure, followVar, errors) &&
               isSameExpression(true, cond1->astOperand2(), cond2->astOperand1(), settings, pure, followVar, errors)) {
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
        if (!isSameExpression(true, expr1, expr2, settings, pure, followVar, errors))
            return false;

        const ValueFlow::Value &rhsValue1 = value1->values().front();
        const ValueFlow::Value &rhsValue2 = value2->values().front();

        if (op1 == "<" || op1 == "<=")
            return (op2 == "==" || op2 == ">" || op2 == ">=") && (rhsValue1.intvalue < rhsValue2.intvalue);
        if (op1 == ">=" || op1 == ">")
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

bool isOppositeExpression(const Token * const tok1, const Token * const tok2, const Settings& settings, bool pure, bool followVar, ErrorPath* errors)
{
    if (!tok1 || !tok2)
        return false;
    if (isOppositeCond(true, tok1, tok2, settings, pure, followVar, errors))
        return true;
    if (tok1->isUnaryOp("-") && !(tok2->astParent() && tok2->astParent()->tokType() == Token::eBitOp))
        return isSameExpression(true, tok1->astOperand1(), tok2, settings, pure, followVar, errors);
    if (tok2->isUnaryOp("-") && !(tok2->astParent() && tok2->astParent()->tokType() == Token::eBitOp))
        return isSameExpression(true, tok2->astOperand1(), tok1, settings, pure, followVar, errors);
    return false;
}

static bool functionModifiesArguments(const Function* f)
{
    return std::any_of(f->argumentList.cbegin(), f->argumentList.cend(), [](const Variable& var) {
        return var.isReference() && !var.isConst();
    });
}

bool isConstFunctionCall(const Token* ftok, const Library& library)
{
    if (isUnevaluated(ftok))
        return true;
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
                if (std::any_of(fs.cbegin(), fs.cend(), [&](const Function* g) {
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
        }
        if (f->argumentList.empty())
            return f->isConstexpr();
    } else if (Token::Match(ftok->previous(), ". %name% (") && ftok->previous()->originalName() != "->" &&
               astIsSmartPointer(ftok->previous()->astOperand1())) {
        return Token::Match(ftok, "get|get_deleter ( )");
    } else if (Token::Match(ftok->previous(), ". %name% (") && astIsContainer(ftok->previous()->astOperand1())) {
        const Library::Container* container = ftok->previous()->astOperand1()->valueType()->container;
        if (!container)
            return false;
        if (container->getYield(ftok->str()) != Library::Container::Yield::NO_YIELD)
            return true;
        if (container->getAction(ftok->str()) == Library::Container::Action::FIND_CONST)
            return true;
        return false;
    } else if (const Library::Function* lf = library.getFunction(ftok)) {
        if (lf->ispure)
            return true;
        if (lf->containerYield != Library::Container::Yield::NO_YIELD)
            return true;
        if (lf->containerAction == Library::Container::Action::FIND_CONST)
            return true;
        return false;
    } else {
        const bool memberFunction = Token::Match(ftok->previous(), ". %name% (");
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
        return constMember && std::all_of(args.cbegin(), args.cend(), [](const Token* tok) {
            const Variable* var = tok->variable();
            if (var)
                return var->isConst();
            return false;
        });
    }
    return false;
}

bool isConstExpression(const Token *tok, const Library& library)
{
    if (!tok)
        return true;
    if (tok->variable() && tok->variable()->isVolatile())
        return false;
    if (tok->isName() && tok->strAt(1) == "(") {
        if (!isConstFunctionCall(tok, library))
            return false;
    }
    if (tok->tokType() == Token::eIncDecOp)
        return false;
    if (tok->isAssignmentOp())
        return false;
    if (isLikelyStreamRead(tok))
        return false;
    // bailout when we see ({..})
    if (tok->str() == "{")
        return false;
    return isConstExpression(tok->astOperand1(), library) && isConstExpression(tok->astOperand2(), library);
}

bool isWithoutSideEffects(const Token* tok, bool checkArrayAccess, bool checkReference)
{
    if (!tok)
        return true;
    if (!tok->isCpp())
        return true;

    while (tok && tok->astOperand2() && tok->astOperand2()->str() != "(")
        tok = tok->astOperand2();
    if (tok && tok->varId()) {
        const Variable* var = tok->variable();
        return var && ((!var->isClass() && (checkReference || !var->isReference())) || var->isPointer() ||
                       (checkArrayAccess ? var->isArray() || (var->isStlType() && !var->isStlType(CheckClass::stl_containers_not_const)) : var->isStlType()));
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
        if (!std::all_of(scope->functionList.begin(), scope->functionList.end(), [&](const Function& f) {
            if (f.type != FunctionType::eFunction)
                return true;

            const std::string freturnType = f.retType ? f.retType->name() : f.retDef->stringifyList(f.returnDefEnd());
            return f.argumentList.size() != fun->argumentList.size() || returnType != freturnType || f.name() == fun->name();
        }))
            return false;
    } else if (tok->variable()) {
        const Variable * var = tok->variable();
        const Scope * scope = var->scope();
        if (!scope)
            return true;
        const Type * varType = var->type();
        // Iterate over the variables in scope and the parameters of the function if possible
        const Function * fun = scope->function;

        auto pred = [=](const Variable& v) {
            if (varType)
                return v.type() && v.type()->name() == varType->name() && v.name() != var->name();
            return v.isFloatingType() == var->isFloatingType() &&
                   v.isEnumType() == var->isEnumType() &&
                   v.isClass() == var->isClass() &&
                   v.isArray() == var->isArray() &&
                   v.isPointer() == var->isPointer() &&
                   v.name() != var->name();
        };
        if (std::any_of(scope->varlist.cbegin(), scope->varlist.cend(), pred))
            return false;
        if (fun) {
            if (std::any_of(fun->argumentList.cbegin(), fun->argumentList.cend(), pred))
                return false;
        }
    } else if (!isUniqueExpression(tok->astOperand1())) {
        return false;
    }

    return isUniqueExpression(tok->astOperand2());
}

static bool isEscaped(const Token* tok, bool functionsScope, const Library& library)
{
    if (library.isnoreturn(tok))
        return true;
    if (functionsScope)
        return Token::simpleMatch(tok, "throw");
    return Token::Match(tok, "return|throw");
}

static bool isEscapedOrJump(const Token* tok, bool functionsScope, const Library& library)
{
    if (library.isnoreturn(tok))
        return true;
    if (functionsScope)
        return Token::simpleMatch(tok, "throw");
    return Token::Match(tok, "return|goto|throw|continue|break");
}

bool isEscapeFunction(const Token* ftok, const Library& library)
{
    if (!Token::Match(ftok, "%name% ("))
        return false;
    const Function* function = ftok->function();
    if (function) {
        if (function->isEscapeFunction())
            return true;
        if (function->isAttributeNoreturn())
            return true;
    } else {
        if (library.isnoreturn(ftok))
            return true;
    }
    return false;
}

static bool hasNoreturnFunction(const Token* tok, const Library& library, const Token** unknownFunc)
{
    if (!tok)
        return false;
    const Token* ftok = (tok->str() == "(" && !tok->isCast()) ? tok->previous() : nullptr;
    while (Token::simpleMatch(ftok, "("))
        ftok = ftok->astOperand1();
    if (ftok) {
        const Function * function = ftok->function();
        if (function) {
            if (function->isEscapeFunction())
                return true;
            if (function->isAttributeNoreturn())
                return true;
        } else if (library.isnoreturn(ftok)) {
            return true;
        } else if (Token::Match(ftok, "exit|abort")) {
            return true;
        }
        if (unknownFunc && !function && library.functions().count(library.getFunctionName(ftok)) == 0)
            *unknownFunc = ftok;
        return false;
    }
    if (tok->isConstOp()) {
        return hasNoreturnFunction(tok->astOperand1(), library, unknownFunc) || hasNoreturnFunction(tok->astOperand2(), library, unknownFunc);
    }

    return false;
}

bool isReturnScope(const Token* const endToken, const Library& library, const Token** unknownFunc, bool functionScope)
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
        if (Token::simpleMatch(prev->previous(), ") ;") && prev->linkAt(-1) &&
            isEscaped(prev->linkAt(-1)->astTop(), functionScope, library))
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

bool isWithinScope(const Token* tok, const Variable* var, ScopeType type)
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

bool isVariableChangedByFunctionCall(const Token *tok, int indirect, nonneg int varid, const Settings &settings, bool *inconclusive)
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
static T* getTokenArgumentFunctionImpl(T* tok, int& argn)
{
    argn = -1;
    {
        T* parent = tok->astParent();
        if (parent && (parent->isUnaryOp("&") || parent->isIncDecOp()))
            parent = parent->astParent();
        while (parent && parent->isCast())
            parent = parent->astParent();
        if (Token::Match(parent, "[+-]") && parent->valueType() && parent->valueType()->pointer)
            parent = parent->astParent();

        // passing variable to subfunction?
        if (Token::Match(parent, "[[(,{.]") || Token::Match(parent, "%oror%|&&") || (parent && parent->isUnaryOp("*")))
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
    if (Token::Match(tok, ". * %name%")) // bailout for pointer to member
        return tok->tokAt(2);
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
    if (!tok)
        return {};
    if (tok->function()) {
        const Variable* argvar = tok->function()->getArgumentVar(argnr);
        if (argvar)
            return {argvar};
        return {};
    }
    if (tok->variable() || Token::simpleMatch(tok, "{") || Token::Match(tok->previous(), "%type% (|{")) {
        const Type* type = Token::typeOf(tok);
        if (!type)
            return {};
        const Scope* typeScope = type->classScope;
        if (!typeScope)
            return {};
        const bool tokIsBrace = Token::simpleMatch(tok, "{");
        // Aggregate constructor
        if (tokIsBrace && typeScope->numConstructors == 0 && argnr < typeScope->varlist.size()) {
            auto it = std::next(typeScope->varlist.cbegin(), argnr);
            return {&*it};
        }
        std::vector<const Variable*> result;
        const int argCount = numberOfArguments(tok);
        const bool constructor = tokIsBrace || (tok->variable() && tok->variable()->nameToken() == tok);
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
        return result;
    }
    return {};
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

bool isMutableExpression(const Token* tok)
{
    if (!tok)
        return false;
    if (tok->isLiteral() || tok->isKeyword() || tok->isStandardType() || tok->isEnumerator())
        return false;
    if (Token::Match(tok, ",|;|:|]|)|}"))
        return false;
    if (Token::simpleMatch(tok, "[ ]"))
        return false;
    if (tok->previous() && tok->previous()->isKeyword() && Token::Match(tok->previous(), "%name% ("))
        return false;
    if (tok->link() && Token::Match(tok, "<|>"))
        return false;
    if (tok->astOperand1() && Token::simpleMatch(tok, "["))
        return isMutableExpression(tok->astOperand1());
    if (const Variable* var = tok->variable()) {
        if (var->isConst() && !var->isPointer() && (!var->isArray() || !var->isArgument()))
            return false;
    }
    return true;
}

bool isVariableChangedByFunctionCall(const Token *tok, int indirect, const Settings &settings, bool *inconclusive)
{
    if (!tok)
        return false;

    if (Token::simpleMatch(tok, ","))
        return false;

    const Token * const tok1 = tok;

    // address of variable
    const bool addressOf = tok->astParent() && tok->astParent()->isUnaryOp("&");
    if (addressOf)
        indirect++;

    const bool deref = tok->astParent() && tok->astParent()->isUnaryOp("*");
    if (deref && indirect > 0)
        indirect--;

    if (indirect == 1 && tok->isCpp() && tok->tokAt(-1) && Token::simpleMatch(tok->tokAt(-2), "new (")) // placement new TODO: fix AST
        return true;

    int argnr;
    tok = getTokenArgumentFunction(tok, argnr);
    if (!tok)
        return false; // not a function => variable not changed
    if (Token::simpleMatch(tok, "{") && isTrivialConstructor(tok))
        return false;
    if (tok->isKeyword() && !isCPPCastKeyword(tok) && !startsWith(tok->str(),"operator"))
        return false;
    // A functional cast won't modify the variable
    if (Token::Match(tok, "%type% (|{") && tok->tokType() == Token::eType && astIsPrimitive(tok->next()))
        return false;
    const Token * parenTok = tok->next();
    if (Token::simpleMatch(parenTok, "<") && parenTok->link())
        parenTok = parenTok->link()->next();
    const bool possiblyPassedByReference = (parenTok->next() == tok1 || Token::Match(tok1->previous(), ", %name% [,)}]"));

    if (!tok->function() && !tok->variable() && tok->isName()) {
        // Check if direction (in, out, inout) is specified in the library configuration and use that
        const Library::ArgumentChecks::Direction argDirection = settings.library.getArgDirection(tok, 1 + argnr, indirect);
        if (argDirection == Library::ArgumentChecks::Direction::DIR_IN)
            return false;
        if (argDirection == Library::ArgumentChecks::Direction::DIR_OUT || argDirection == Library::ArgumentChecks::Direction::DIR_INOUT)
            return true;

        const bool requireNonNull = settings.library.isnullargbad(tok, 1 + argnr);
        if (Token::simpleMatch(tok->tokAt(-2), "std :: tie"))
            return true;
        // if the library says 0 is invalid
        // => it is assumed that parameter is an in parameter (TODO: this is a bad heuristic)
        if (indirect == 0 && requireNonNull)
            return false;
        // possible pass-by-reference => inconclusive
        if (possiblyPassedByReference) {
            if (inconclusive != nullptr)
                *inconclusive = true;
            return false;
        }
        // Safe guess: Assume that parameter is changed by function call
        return true;
    }

    if (const Variable* var = tok->variable()) {
        if (tok == var->nameToken() && (!var->isReference() || (var->isConst() && var->type() == tok1->type())) && (!var->isClass() || (var->valueType() && var->valueType()->container))) // const ref or passed to (copy) ctor
            return false;
    }

    std::vector<const Variable*> args = getArgumentVars(tok, argnr);
    bool conclusive = false;
    for (const Variable *arg:args) {
        if (!arg)
            continue;
        conclusive = true;
        if (indirect > 0) {
            if (arg->isPointer() && !(arg->valueType() && arg->valueType()->isConst(indirect)))
                return true;
            if (indirect > 1 && addressOf && arg->isPointer() && (!arg->valueType() || !arg->valueType()->isConst(indirect-1)))
                return true;
            if (arg->isArray() || (!arg->isPointer() && (!arg->valueType() || arg->valueType()->type == ValueType::UNKNOWN_TYPE)))
                return true;
        }
        if (!arg->isConst() && arg->isReference())
            return true;
    }
    if (addressOf && tok1->astParent()->isUnaryOp("&")) {
        const Token* castToken = tok1->astParent();
        while (castToken->astParent()->isCast())
            castToken = castToken->astParent();
        if (Token::Match(castToken->astParent(), ",|(") &&
            castToken->valueType() &&
            castToken->valueType()->isIntegral() &&
            castToken->valueType()->pointer == 0)
            return true;
    }
    if (!conclusive && inconclusive) {
        *inconclusive = true;
    }
    return false;
}

static bool hasOverloadedMemberAccess(const Token* tok)
{
    if (!Token::simpleMatch(tok, "."))
        return false;
    const Token* varTok = tok->astOperand2();
    return !varTok || !varTok->variable() || !varTok->variable()->valueType() || varTok->variable()->valueType()->pointer == 0;
}

bool isVariableChanged(const Token *tok, int indirect, const Settings &settings, int depth)
{
    if (!tok)
        return false;
    if (!tok->isMutableExpr())
        return false;

    if (isConstVarExpression(tok))
        return false;

    const Token *tok2 = tok;
    int derefs = 0;
    while ((tok2->astParent() && tok2->astParent()->isUnaryOp("*")) ||
           (Token::simpleMatch(tok2->astParent(), ".") && !Token::Match(tok2->astParent()->astParent(), "[(,]")) ||
           (tok2->astParent() && tok2->astParent()->isUnaryOp("&") && Token::simpleMatch(tok2->astParent()->astParent(), ".") && tok2->astParent()->astParent()->originalName()=="->") ||
           (Token::simpleMatch(tok2->astParent(), "[") && tok2 == tok2->astParent()->astOperand1()) ||
           (Token::simpleMatch(tok2->astParent(), "(") && tok2->astParent()->isCast())) {
        if (tok2->astParent() && (tok2->astParent()->isUnaryOp("*") || (astIsLHS(tok2) && tok2->astParent()->originalName() == "->" && !hasOverloadedMemberAccess(tok2))))
            derefs++;
        if (derefs > indirect)
            break;
        if (tok2->astParent() && tok2->astParent()->isUnaryOp("&") && Token::simpleMatch(tok2->astParent()->astParent(), ".") && tok2->astParent()->astParent()->originalName()=="->")
            tok2 = tok2->astParent();
        tok2 = tok2->astParent();
    }

    if (tok2->astParent() && tok2->astParent()->isUnaryOp("&")) {
        const Token* parent = tok2->astParent();
        while (parent->astParent() && parent->astParent()->isCast())
            parent = parent->astParent();
        if (parent->astParent() && parent->astParent()->isUnaryOp("*"))
            tok2 = parent->astParent();
    }

    while ((Token::simpleMatch(tok2, ":") && Token::simpleMatch(tok2->astParent(), "?")) ||
           (Token::simpleMatch(tok2->astParent(), ":") && Token::simpleMatch(tok2->astParent()->astParent(), "?")))
        tok2 = tok2->astParent();

    if (indirect == 0 && tok2->astParent() && tok2->astParent()->tokType() == Token::eIncDecOp)
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
        if (astIsLHS(tok2))
            return true;
        // Check if assigning to a non-const lvalue
        const Variable * var = getLHSVariable(tok2->astParent());
        if (var && var->isReference() && !var->isConst() && var->nameToken() &&
            var->nameToken()->next() == tok2->astParent()) {
            if (!var->isLocal() || isVariableChanged(var, settings, depth - 1))
                return true;
        }
    }

    const ValueType* vt = tok->variable() ? tok->variable()->valueType() : tok->valueType();

    // Check addressof
    if (tok2->astParent() && tok2->astParent()->isUnaryOp("&")) {
        if (isVariableChanged(tok2->astParent(), indirect + 1, settings, depth - 1))
            return true;
    } else {
        // If its already const then it can't be modified
        if (vt && vt->isConst(indirect))
            return false;
    }

    if (tok2->isCpp() && Token::Match(tok2->astParent(), ">>|&") && astIsRHS(tok2) && isLikelyStreamRead(tok2->astParent()))
        return true;

    if (isLikelyStream(tok2))
        return true;

    // Member function call
    if (Token::Match(tok2->astParent(), ". %name%") && isFunctionCall(tok2->astParent()->next()) &&
        tok2->astParent()->astOperand1() == tok2) {
        // Member function cannot change what `this` points to
        if (indirect == 0 && astIsPointer(tok))
            return false;

        const Token *ftok = tok2->astParent()->astOperand2();
        const Token* const ctok = tok2->str() == "." ? tok2->astOperand2() : tok2;
        if (astIsContainer(ctok) && ctok->valueType() && ctok->valueType()->container) {
            const Library::Container* c = ctok->valueType()->container;
            const Library::Container::Action action = c->getAction(ftok->str());
            if (contains({Library::Container::Action::INSERT,
                          Library::Container::Action::ERASE,
                          Library::Container::Action::APPEND,
                          Library::Container::Action::CHANGE,
                          Library::Container::Action::CHANGE_CONTENT,
                          Library::Container::Action::CHANGE_INTERNAL,
                          Library::Container::Action::CLEAR,
                          Library::Container::Action::FIND,
                          Library::Container::Action::PUSH,
                          Library::Container::Action::POP,
                          Library::Container::Action::RESIZE},
                         action))
                return true;
            const Library::Container::Yield yield = c->getYield(ftok->str());
            // If accessing element check if the element is changed
            if (contains({Library::Container::Yield::ITEM, Library::Container::Yield::AT_INDEX}, yield))
                return isVariableChanged(ftok->next(), indirect, settings, depth - 1);

            if (contains({Library::Container::Yield::BUFFER,
                          Library::Container::Yield::BUFFER_NT,
                          Library::Container::Yield::START_ITERATOR,
                          Library::Container::Yield::ITERATOR},
                         yield)) {
                return isVariableChanged(ftok->next(), indirect + 1, settings, depth - 1);
            }
            if (contains({Library::Container::Yield::SIZE,
                          Library::Container::Yield::EMPTY,
                          Library::Container::Yield::END_ITERATOR},
                         yield)) {
                return false;
            }
        }
        if (settings.library.isFunctionConst(ftok) || (astIsSmartPointer(tok) && ftok->str() == "get")) // TODO: replace with action/yield?
            return false;

        const Function * fun = ftok->function();
        if (!fun)
            return true;
        return !fun->isConst();
    }

    // Member pointer
    if (Token::Match(tok2->astParent(), ". * ( & %name% ::")) {
        const Token* ftok = tok2->astParent()->linkAt(2)->previous();
        // TODO: Check for pointer to member variable
        if (!ftok->function() || !ftok->function()->isConst())
            return true;
    }
    if (Token::Match(tok2->astParent(), ". * %name%")) // bailout
        return true;

    if (Token::simpleMatch(tok2, "[") && astIsContainer(tok) && vt && vt->container && vt->container->stdAssociativeLike)
        return true;

    const Token *ftok = tok2;
    while (ftok && (!Token::Match(ftok, "[({]") || ftok->isCast()))
        ftok = ftok->astParent();

    if (ftok && Token::Match(ftok->link(), ")|} !!{")) {
        if (ftok->str() == "(" && Token::simpleMatch(ftok->astOperand1(), "[")) // operator() on array element, bail out
            return true;
        const Token * ptok = tok2;
        while (Token::Match(ptok->astParent(), ".|::|["))
            ptok = ptok->astParent();
        int pindirect = indirect;
        if (indirect == 0 && astIsLHS(tok2) && Token::Match(ptok, ". %var%") && astIsPointer(ptok->next()))
            pindirect = 1;
        bool inconclusive = false;
        bool isChanged = isVariableChangedByFunctionCall(ptok, pindirect, settings, &inconclusive);
        isChanged |= inconclusive;
        if (isChanged)
            return true;
    }

    const Token *parent = tok2->astParent();
    while (Token::Match(parent, ".|::"))
        parent = parent->astParent();
    if (parent && parent->tokType() == Token::eIncDecOp && (indirect == 0 || tok2 != tok))
        return true;

    // structured binding, nonconst reference variable in lhs
    if (Token::Match(tok2->astParent(), ":|=") && tok2 == tok2->astParent()->astOperand2() && Token::simpleMatch(tok2->astParent()->previous(), "]")) {
        const Token *typeStart = tok2->astParent()->linkAt(-1)->previous();
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
        if (!loopVar->isConst() && loopVar->isReference() && isVariableChanged(loopVar, settings, depth - 1))
            return true;
        return false;
    }

    if (indirect > 0) {
        // check for `*(ptr + 1) = new_value` case
        parent = tok2->astParent();
        while (parent && ((parent->isArithmeticalOp() && parent->isBinaryOp()) || parent->isIncDecOp())) {
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

bool isVariableChanged(const Token *start, const Token *end, const nonneg int exprid, bool globalvar, const Settings &settings, int depth)
{
    return findVariableChanged(start, end, 0, exprid, globalvar, settings, depth) != nullptr;
}

bool isVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings &settings, int depth)
{
    return findVariableChanged(start, end, indirect, exprid, globalvar, settings, depth) != nullptr;
}

const Token* findExpression(const Token* start, const nonneg int exprid)
{
    const Function* f = Scope::nestedInFunction(start->scope());
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

const Token* findEscapeStatement(const Scope* scope, const Library& library)
{
    if (!scope)
        return nullptr;
    for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
        const Scope* escapeScope = tok->scope();
        if (!escapeScope->isExecutable()) { // skip type definitions
            tok = escapeScope->bodyEnd;
            continue;
        }
        if (const Token* lambdaEnd = findLambdaEndToken(tok)) { // skip lambdas
            tok = lambdaEnd;
            continue;
        }
        if (!tok->isName())
            continue;
        if (isEscapeFunction(tok, library))
            return tok;
        if (!tok->isKeyword())
            continue;
        if (Token::Match(tok, "goto|return|throw")) // TODO: check try/catch, labels?
            return tok;
        if (!Token::Match(tok, "break|continue"))
            continue;
        const bool isBreak = tok->str()[0] == 'b';
        while (escapeScope && escapeScope != scope) {
            if (escapeScope->isLoopScope() || (isBreak && escapeScope->type == ScopeType::eSwitch))
                return nullptr;
            escapeScope = escapeScope->nestedIn;
        }
        return tok;
    }
    return nullptr;
}

template<class F,
         REQUIRES("F must be a function that returns a Token class",
                  std::is_convertible<decltype(std::declval<F>()()), const Token*> )>
static bool isExpressionChangedAt(const F& getExprTok,
                                  const Token* tok,
                                  int indirect,
                                  const nonneg int exprid,
                                  bool globalvar,
                                  const Settings& settings,
                                  int depth)
{
    if (depth < 0)
        return true;
    if (!tok)
        return false;
    if (!tok->isMutableExpr())
        return false;
    if (tok->exprId() != exprid || (!tok->varId() && !tok->isName())) {
        if (globalvar && Token::Match(tok, "%name% (") &&
            (!(tok->function() && (tok->function()->isAttributePure() || tok->function()->isAttributeConst())))) {
            if (!Token::simpleMatch(tok->astParent(), "."))
                return true;
            const auto yield = astContainerYield(tok->astParent()->astOperand1(), settings.library);
            if (yield != Library::Container::Yield::SIZE && yield != Library::Container::Yield::EMPTY &&
                yield != Library::Container::Yield::BUFFER && yield != Library::Container::Yield::BUFFER_NT)
                // TODO: Is global variable really changed by function call?
                return true;
        }
        nonneg int i = 1;
        bool aliased = false;
        // If we can't find the expression then assume it is an alias
        auto expr = getExprTok();
        if (!expr && !(tok->valueType() && tok->valueType()->pointer == 0 && tok->valueType()->reference == Reference::None))
            aliased = true;
        if (!aliased && expr && expr->varId() && tok->isCast() && tok->valueType() && tok->valueType()->reference != Reference::None &&
            Token::Match(tok->astOperand2() ? tok->astOperand2() : tok->astOperand1(), "%varid%", expr->varId()))
            aliased = true;
        if (!aliased)
            aliased = isAliasOf(tok, expr, &i);
        if (!aliased)
            return false;
        i += indirect;
        if (tok->valueType() && tok->valueType()->pointer)
            i = std::min(i, tok->valueType()->pointer);
        if (isVariableChanged(tok, i, settings, depth))
            return true;
        // TODO: Try to traverse the lambda function
        if (Token::Match(tok, "%var% ("))
            return true;
        return false;
    }
    return (isVariableChanged(tok, indirect, settings, depth));
}

bool isExpressionChangedAt(const Token* expr,
                           const Token* tok,
                           int indirect,
                           bool globalvar,
                           const Settings& settings,
                           int depth)
{
    return isExpressionChangedAt([&] {
        return expr;
    }, tok, indirect, expr->exprId(), globalvar, settings, depth);
}

Token* findVariableChanged(Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings &settings, int depth)
{
    if (!precedes(start, end))
        return nullptr;
    if (depth < 0)
        return start;
    auto getExprTok = utils::memoize([&] {
        return findExpression(start, exprid);
    });
    for (Token *tok = start; tok != end; tok = tok->next()) {
        if (isExpressionChangedAt(getExprTok, tok, indirect, exprid, globalvar, settings, depth))
            return tok;
    }
    return nullptr;
}

const Token* findVariableChanged(const Token *start, const Token *end, int indirect, const nonneg int exprid, bool globalvar, const Settings &settings, int depth)
{
    return findVariableChanged(const_cast<Token*>(start), end, indirect, exprid, globalvar, settings, depth);
}

bool isVariableChanged(const Variable * var, const Settings &settings, int depth)
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
    if (Token::simpleMatch(start, "=")) {
        const Token* next = nextAfterAstRightmostLeafGeneric(start);
        if (next)
            start = next;
    }
    return findExpressionChanged(var->nameToken(), start->next(), var->scope()->bodyEnd, settings, depth);
}

bool isVariablesChanged(const Token* start,
                        const Token* end,
                        int indirect,
                        const std::vector<const Variable*> &vars,
                        const Settings& settings)
{
    std::set<int> varids;
    std::transform(vars.cbegin(), vars.cend(), std::inserter(varids, varids.begin()), [](const Variable* var) {
        return var->declarationId();
    });
    const bool globalvar = std::any_of(vars.cbegin(), vars.cend(), [](const Variable* var) {
        return var->isGlobal();
    });
    for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
        if (tok->varId() == 0 || varids.count(tok->varId()) == 0) {
            if (globalvar && Token::Match(tok, "%name% ("))
                // TODO: Is global variable really changed by function call?
                return true;
            continue;
        }
        if (isVariableChanged(tok, indirect, settings))
            return true;
    }
    return false;
}

bool isThisChanged(const Token* tok, int indirect, const Settings& settings)
{
    if ((Token::Match(tok->previous(), "%name% (") && !Token::simpleMatch(tok->astOperand1(), ".")) ||
        Token::Match(tok->tokAt(-3), "this . %name% (")) {
        if (tok->previous()->function()) {
            return (!tok->previous()->function()->isConst() && !tok->previous()->function()->isStatic());
        }
        if (!tok->previous()->isKeyword() || tok->previous()->isOperatorKeyword()) {
            return true;
        }
    }
    if (isVariableChanged(tok, indirect, settings))
        return true;
    return false;
}

static const Token* findThisChanged(const Token* start, const Token* end, int indirect, const Settings& settings)
{
    if (!precedes(start, end))
        return nullptr;
    for (const Token* tok = start; tok != end; tok = tok->next()) {
        if (!exprDependsOnThis(tok))
            continue;
        if (isThisChanged(tok, indirect, settings))
            return tok;
    }
    return nullptr;
}

template<class Find>
static const Token* findExpressionChangedImpl(const Token* expr,
                                              const Token* start,
                                              const Token* end,
                                              const Settings& settings,
                                              int depth,
                                              Find find)
{
    if (depth < 0)
        return start;
    if (!precedes(start, end))
        return nullptr;
    const Token* result = nullptr;
    findAstNode(expr, [&](const Token* tok) {
        if (exprDependsOnThis(tok)) {
            result = findThisChanged(start, end, /*indirect*/ 0, settings);
            if (result)
                return true;
        }
        bool global = false;
        if (tok->variable()) {
            global = !tok->variable()->isLocal() && !tok->variable()->isArgument();
        } else if (tok->isIncompleteVar() && !tok->isIncompleteConstant()) {
            global = true;
        }

        if (tok->exprId() > 0 || global) {
            const Token* modifedTok = find(start, end, [&](const Token* tok2) {
                int indirect = 0;
                if (const ValueType* vt = tok->valueType()) {
                    indirect = vt->pointer;
                    if (vt->type == ValueType::ITERATOR)
                        ++indirect;
                }
                for (int i = 0; i <= indirect; ++i) {
                    if (isExpressionChangedAt(tok, tok2, i, global, settings, depth))
                        return true;
                }
                return false;
            });
            if (modifedTok) {
                result = modifedTok;
                return true;
            }
        }
        return false;
    });
    return result;
}

namespace {
    struct ExpressionChangedSimpleFind {
        template<class F>
        const Token* operator()(const Token* start, const Token* end, F f) const
        {
            return findToken(start, end, f);
        }
    };

    struct ExpressionChangedSkipDeadCode {
        const Library& library;
        const std::function<std::vector<MathLib::bigint>(const Token* tok)>* evaluate;
        ExpressionChangedSkipDeadCode(const Library& library,
                                      const std::function<std::vector<MathLib::bigint>(const Token* tok)>& evaluate)
            : library(library), evaluate(&evaluate)
        {}
        template<class F>
        const Token* operator()(const Token* start, const Token* end, F f) const
        {
            return findTokenSkipDeadCode(library, start, end, std::move(f), *evaluate);
        }
    };
}

const Token* findExpressionChanged(const Token* expr,
                                   const Token* start,
                                   const Token* end,
                                   const Settings& settings,
                                   int depth)
{
    return findExpressionChangedImpl(expr, start, end, settings, depth, ExpressionChangedSimpleFind{});
}

const Token* findExpressionChangedSkipDeadCode(const Token* expr,
                                               const Token* start,
                                               const Token* end,
                                               const Settings& settings,
                                               const std::function<std::vector<MathLib::bigint>(const Token* tok)>& evaluate,
                                               int depth)
{
    return findExpressionChangedImpl(
        expr, start, end, settings, depth, ExpressionChangedSkipDeadCode{settings.library, evaluate});
}

const Token* getArgumentStart(const Token* ftok)
{
    const Token* tok = ftok;
    if (Token::Match(tok, "%name% (|{|)"))
        tok = ftok->next();
    while (Token::simpleMatch(tok, ")"))
        tok = tok->next();
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
    int arguments = 0;
    const Token* openBracket = start->next();
    while (Token::simpleMatch(openBracket, ")"))
        openBracket = openBracket->next();
    if (openBracket && openBracket->str()=="(" && openBracket->next() && openBracket->strAt(1)!=")") {
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
    auto arg_it = std::find_if(f->argumentList.cbegin(), f->argumentList.cend(), [&](const Variable& v) {
        return v.nameToken() == var->nameToken();
    });
    if (arg_it == f->argumentList.end())
        return -1;
    return std::distance(f->argumentList.cbegin(), arg_it);
}

const Token* getIteratorExpression(const Token* tok)
{
    if (!tok)
        return nullptr;
    if (tok->isUnaryOp("*"))
        return nullptr;
    if (!tok->isName()) {
        const Token* iter1 = getIteratorExpression(tok->astOperand1());
        if (iter1)
            return iter1;
        if (tok->str() == "(")
            return nullptr;
        const Token* iter2 = getIteratorExpression(tok->astOperand2());
        if (iter2)
            return iter2;
    } else if (Token::Match(tok, "begin|cbegin|rbegin|crbegin|end|cend|rend|crend (")) {
        if (Token::Match(tok->previous(), ". %name% ( ) !!."))
            return tok->previous()->astOperand1();
        if (!Token::simpleMatch(tok->previous(), ".") && Token::Match(tok, "%name% ( !!)") &&
            !Token::simpleMatch(tok->linkAt(1), ") ."))
            return tok->next()->astOperand2();
    }
    return nullptr;
}

bool isIteratorPair(const std::vector<const Token*>& args)
{
    if (args.size() != 2)
        return false;
    if (astIsPointer(args[0]) && astIsPointer(args[1]))
        return true;
    // Check if iterator is from same container
    const Token* tok1 = nullptr;
    const Token* tok2 = nullptr;
    if (astIsIterator(args[0]) && astIsIterator(args[1])) {
        tok1 = ValueFlow::getLifetimeObjValue(args[0]).tokvalue;
        tok2 = ValueFlow::getLifetimeObjValue(args[1]).tokvalue;
        if (!tok1 || !tok2)
            return true;
    } else {
        tok1 = getIteratorExpression(args[0]);
        tok2 = getIteratorExpression(args[1]);
    }
    if (tok1 && tok2)
        return tok1->exprId() == tok2->exprId();
    return tok1 || tok2;
}

const Token *findLambdaStartToken(const Token *last)
{
    if (!last || !last->isCpp() || last->str() != "}")
        return nullptr;
    const Token* tok = last->link();
    if (Token::simpleMatch(tok->astParent(), "("))
        tok = tok->astParent();
    if (Token::simpleMatch(tok->astParent(), "["))
        return tok->astParent();
    return nullptr;
}

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static T* findLambdaEndTokenGeneric(T* first)
{
    auto maybeLambda = [](T* tok) -> bool {
        while (Token::Match(tok, "*|%name%|::|>")) {
            if (tok->link())
                tok = tok->link()->previous();
            else {
                if (tok->str() == ">")
                    return true;
                if (tok->str() == "new")
                    return false;
                tok = tok->previous();
            }
        }
        return true;
    };

    if (!first || !first->isCpp() || first->str() != "[")
        return nullptr;
    if (!maybeLambda(first->previous()))
        return nullptr;
    if (!Token::Match(first->link(), "] (|{|<"))
        return nullptr;
    const Token* roundOrCurly = first->link()->next();
    if (roundOrCurly->link() && roundOrCurly->str() == "<")
        roundOrCurly = roundOrCurly->link()->next();
    if (first->astOperand1() != roundOrCurly)
        return nullptr;
    T * tok = first;

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

bool isLikelyStream(const Token *stream)
{
    if (!stream)
        return false;

    if (!stream->isCpp())
        return false;

    if (!Token::Match(stream->astParent(), "&|<<|>>") || !stream->astParent()->isBinaryOp())
        return false;

    if (stream->astParent()->astOperand1() != stream)
        return false;

    return !astIsIntegral(stream, false);
}

bool isLikelyStreamRead(const Token *op)
{
    if (!op)
        return false;

    if (!op->isCpp())
        return false;

    if (!Token::Match(op, "&|>>") || !op->isBinaryOp())
        return false;

    if (!Token::Match(op->astOperand2(), "%name%|.|*|[") && op->str() != op->astOperand2()->str())
        return false;

    const Token *parent = op;
    while (parent->astParent() && parent->astParent()->str() == op->str())
        parent = parent->astParent();
    if (parent->astParent() && !Token::Match(parent->astParent(), "%oror%|&&|(|,|.|!|;|return"))
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

bool isConstVarExpression(const Token *tok, const std::function<bool(const Token*)>& skipPredicate)
{
    if (!tok)
        return false;
    if (tok->str() == "?" && tok->astOperand2() && tok->astOperand2()->str() == ":") // ternary operator
        return isConstVarExpression(tok->astOperand2()->astOperand1()) || isConstVarExpression(tok->astOperand2()->astOperand2()); // left and right of ":"
    if (skipPredicate && skipPredicate(tok))
        return false;
    if (Token::simpleMatch(tok->previous(), "sizeof ("))
        return true;
    if (Token::Match(tok->previous(), "%name% (")) {
        if (Token::simpleMatch(tok->astOperand1(), ".") && !isConstVarExpression(tok->astOperand1(), skipPredicate))
            return false;
        std::vector<const Token *> args = getArguments(tok);
        if (args.empty() && tok->previous()->function() && tok->previous()->function()->isConstexpr())
            return true;
        return !args.empty() && std::all_of(args.cbegin(), args.cend(), [&](const Token* t) {
            return isConstVarExpression(t, skipPredicate);
        });
    }
    if (isCPPCast(tok)) {
        return isConstVarExpression(tok->astOperand2(), skipPredicate);
    }
    if (Token::Match(tok, "( %type%"))
        return isConstVarExpression(tok->astOperand1(), skipPredicate);
    if (tok->str() == "::" && tok->hasKnownValue())
        return isConstVarExpression(tok->astOperand2(), skipPredicate);
    if (Token::Match(tok, "%cop%|[|.")) {
        if (tok->astOperand1() && !isConstVarExpression(tok->astOperand1(), skipPredicate))
            return false;
        if (tok->astOperand2() && !isConstVarExpression(tok->astOperand2(), skipPredicate))
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

static ExprUsage getFunctionUsage(const Token* tok, int indirect, const Settings& settings)
{
    const bool addressOf = tok->astParent() && tok->astParent()->isUnaryOp("&");

    int argnr;
    const Token* ftok = getTokenArgumentFunction(tok, argnr);
    if (!ftok)
        return ExprUsage::None;
    const Function* func = ftok->function();
    // variable init/constructor call?
    if (!func && ftok->variable() && ftok == ftok->variable()->nameToken()) {
        // STL types or containers don't initialize external variables
        if (indirect == 0 && (ftok->variable()->isStlType() || (ftok->variable()->valueType() && ftok->variable()->valueType()->container)))
            return ExprUsage::Used;
        // TODO: resolve multiple constructors
        if (ftok->variable()->type() && ftok->variable()->type()->classScope) {
            const int nCtor = ftok->variable()->type()->classScope->numConstructors;
            if (nCtor == 0) {
                if (indirect > 0) {
                    const std::vector<const Variable*> argvar = getArgumentVars(ftok->astParent(), argnr);
                    if (argvar.size() == 1 && argvar[0]->valueType() && argvar[0]->valueType()->pointer == indirect)
                        return ExprUsage::NotUsed;
                }
                return ExprUsage::Used;
            }
            if (nCtor == 1) {
                const Scope* scope = ftok->variable()->type()->classScope;
                auto it = std::find_if(scope->functionList.begin(), scope->functionList.end(), [](const Function& f) {
                    return f.isConstructor();
                });
                if (it != scope->functionList.end())
                    func = &*it;
            }
        }
    }
    if (func) {
        std::vector<const Variable*> args = getArgumentVars(ftok, argnr);
        for (const Variable* arg : args) {
            if (!arg)
                continue;
            if (arg->isReference() || (arg->isPointer() && indirect == 1)) {
                if (!func->hasBody())
                    return ExprUsage::PassedByReference;
                for (const Token* bodytok = func->functionScope->bodyStart; bodytok != func->functionScope->bodyEnd; bodytok = bodytok->next()) {
                    if (bodytok->variable() == arg) {
                        if (arg->isReference())
                            return ExprUsage::PassedByReference;
                        if (Token::Match(bodytok->astParent(), "%comp%|!"))
                            return ExprUsage::NotUsed;
                        return ExprUsage::PassedByReference;
                    }
                }
                return ExprUsage::NotUsed;
            }
        }
        if (!args.empty() && indirect == 0 && !addressOf)
            return ExprUsage::Used;
    } else if (ftok->isControlFlowKeyword()) {
        return ExprUsage::Used;
    } else if (ftok->str() == "{") {
        return indirect == 0 ? ExprUsage::Used : ExprUsage::Inconclusive;
    } else {
        const bool isnullbad = settings.library.isnullargbad(ftok, argnr + 1);
        if (indirect == 0 && astIsPointer(tok) && !addressOf && isnullbad)
            return ExprUsage::Used;
        bool hasIndirect = false;
        const bool isuninitbad = settings.library.isuninitargbad(ftok, argnr + 1, indirect, &hasIndirect);
        if (isuninitbad && (!addressOf || isnullbad))
            return ExprUsage::Used;
        const Library::ArgumentChecks::Direction argDirection = settings.library.getArgDirection(ftok, argnr + 1, indirect);
        if (argDirection == Library::ArgumentChecks::Direction::DIR_IN) // TODO: DIR_INOUT?
            return ExprUsage::Used;
        if (argDirection == Library::ArgumentChecks::Direction::DIR_OUT)
            return ExprUsage::NotUsed;
    }
    return ExprUsage::Inconclusive;
}

bool isLeafDot(const Token* tok)
{
    if (!tok)
        return false;
    const Token * parent = tok->astParent();
    if (!Token::simpleMatch(parent, "."))
        return false;
    if (parent->astOperand2() == tok && !Token::simpleMatch(parent->astParent(), "."))
        return true;
    return isLeafDot(parent);
}

ExprUsage getExprUsage(const Token* tok, int indirect, const Settings& settings)
{
    const Token* parent = tok->astParent();
    if (indirect > 0 && parent) {
        while (Token::simpleMatch(parent, "[") && parent->astParent())
            parent = parent->astParent();
        if (Token::Match(parent, "%assign%") && (astIsRHS(tok) || astIsLHS(parent->astOperand1())))
            return ExprUsage::NotUsed;
        if (Token::Match(parent, "++|--"))
            return ExprUsage::NotUsed;
        if (parent->isConstOp())
            return ExprUsage::NotUsed;
        if (parent->isCast())
            return ExprUsage::NotUsed;
        if (Token::simpleMatch(parent, ":") && Token::simpleMatch(parent->astParent(), "?"))
            return getExprUsage(parent->astParent(), indirect, settings);
        if (isUsedAsBool(tok, settings))
            return ExprUsage::NotUsed;
    }
    if (tok->isUnaryOp("&") && !parent)
        return ExprUsage::NotUsed;
    if (indirect == 0) {
        if (Token::Match(parent, "%cop%|%assign%|++|--") && parent->str() != "=" &&
            !parent->isUnaryOp("&") &&
            !(astIsRHS(tok) && isLikelyStreamRead(parent)))
            return ExprUsage::Used;
        if (isLeafDot(tok)) {
            const Token* op = parent->astParent();
            while (Token::simpleMatch(op, "."))
                op = op->astParent();
            if (Token::Match(op, "%assign%|++|--")) {
                if (op->str() == "=") {
                    if (precedes(tok, op))
                        return ExprUsage::NotUsed;
                } else
                    return ExprUsage::Used;
            }
        }
        if (Token::simpleMatch(parent, "=") && astIsRHS(tok)) {
            const Token* const lhs  = parent->astOperand1();
            if (lhs && lhs->variable() && lhs->variable()->isReference() && lhs == lhs->variable()->nameToken())
                return ExprUsage::NotUsed;
            return ExprUsage::Used;
        }
        // Function call or index
        if (((Token::simpleMatch(parent, "(") && !parent->isCast()) || (Token::simpleMatch(parent, "[") && tok->valueType())) &&
            (astIsLHS(tok) || Token::simpleMatch(parent, "( )")))
            return ExprUsage::Used;
    }
    return getFunctionUsage(tok, indirect, settings);
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
    if (!Token::Match(tok, "%assign%|(|{"))
        return {};
    if (!tok->astOperand1())
        return {};
    if (tok->astOperand1()->varId() > 0 && tok->astOperand1()->variable())
        return {tok->astOperand1()->variable()};
    std::vector<const Variable*> result;
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

bool isNullOperand(const Token *expr)
{
    if (!expr)
        return false;
    if (expr->isCpp() && Token::Match(expr, "static_cast|const_cast|dynamic_cast|reinterpret_cast <"))
        expr = expr->astParent();
    else if (!expr->isCast())
        return Token::Match(expr, "NULL|nullptr");
    if (expr->valueType() && expr->valueType()->pointer == 0)
        return false;
    const Token *castOp = expr->astOperand2() ? expr->astOperand2() : expr->astOperand1();
    return Token::Match(castOp, "NULL|nullptr") || (MathLib::isInt(castOp->str()) && MathLib::isNullValue(castOp->str()));
}

bool isGlobalData(const Token *expr)
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
                  [expr, &globalData, &var](const Token *tok) {
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
        }
        if (Token::Match(tok, "[*[]") && tok->astOperand1()) {
            // TODO check if pointer points at local data
            const Token *lhs = tok->astOperand1();
            if (lhs->isCast()) {
                lhs = lhs->astOperand2() ? lhs->astOperand2() : lhs->astOperand1();
            }
            if (lhs && lhs->variable()) {
                const Variable *lhsvar = lhs->variable();
                const ValueType *lhstype = lhs->valueType();
                if (lhsvar->isPointer() || !lhstype || lhstype->type == ValueType::Type::ITERATOR) {
                    globalData = true;
                    return ChildrenToVisit::none;
                }
                if (lhsvar->isArgument() && lhsvar->isArray()) {
                    globalData = true;
                    return ChildrenToVisit::none;
                }
                if (lhsvar->isArgument() && lhstype->type <= ValueType::Type::VOID && !lhstype->container) {
                    globalData = true;
                    return ChildrenToVisit::none;
                }
            }
        }
        if (tok->varId() == 0 && tok->isName() && tok->strAt(-1) != ".") {
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
            if (tok->strAt(-1) != "." && !tok->variable()->isLocal() && !tok->variable()->isArgument()) {
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
        if (tok->isCpp() && tok->str() == "." && tok->astOperand1() && tok->astOperand1()->variable() && !tok->astOperand1()->valueType()) {
            globalData = true;
            return ChildrenToVisit::none;
        }
        if (Token::Match(tok, ".|["))
            return ChildrenToVisit::op1;
        return ChildrenToVisit::op1_and_op2;
    });
    return globalData || !var;
}

bool isUnevaluated(const Token *tok)
{
    return Token::Match(tok, "alignof|_Alignof|_alignof|__alignof|__alignof__|decltype|offsetof|sizeof|typeid|typeof|__typeof__ (");
}

static std::set<MathLib::bigint> getSwitchValues(const Token *startbrace, bool &hasDefault)
{
    std::set<MathLib::bigint> values;
    const Token *endbrace = startbrace->link();
    if (!endbrace)
        return values;

    hasDefault = false;
    for (const Token *tok = startbrace->next(); tok && tok != endbrace; tok = tok->next()) {
        if (Token::simpleMatch(tok, "{") && tok->scope()->type == ScopeType::eSwitch) {
            tok = tok->link();
            continue;
        }
        if (Token::simpleMatch(tok, "default")) {
            hasDefault = true;
            break;
        }
        if (Token::simpleMatch(tok, "case")) {
            const Token *valueTok = tok->astOperand1();
            if (const ValueFlow::Value* v = valueTok->getKnownValue(ValueFlow::Value::ValueType::INT))
                values.insert(v->intvalue);
            continue;
        }
    }

    return values;
}

bool isExhaustiveSwitch(const Token *startbrace)
{
    if (!startbrace || !Token::simpleMatch(startbrace->previous(), ") {") || startbrace->scope()->type != ScopeType::eSwitch)
        return false;
    const Token *rpar = startbrace->previous();
    const Token *lpar = rpar->link();

    const Token *condition = lpar->astOperand2();
    if (!condition->valueType())
        return true;

    bool hasDefault = false;
    const std::set<MathLib::bigint> switchValues = getSwitchValues(startbrace, hasDefault);

    if (hasDefault)
        return true;

    if (condition->valueType()->type == ValueType::Type::BOOL)
        return switchValues.count(0) && switchValues.count(1);

    if (condition->valueType()->isEnum()) {
        const std::vector<Enumerator> &enumList = condition->valueType()->typeScope->enumeratorList;
        return std::all_of(enumList.cbegin(), enumList.cend(), [&](const Enumerator &e) {
            return !e.value_known || switchValues.count(e.value);
        });
    }

    return false;
}

bool isUnreachableOperand(const Token *tok)
{
    for (;;)
    {
        const Token *parent = tok->astParent();
        if (!parent)
            break;

        if (parent->isBinaryOp()) {
            const bool left = tok == parent->astOperand1();
            const Token *sibling = left ? parent->astOperand2() : parent->astOperand1();

            // logical and
            if (Token::simpleMatch(parent, "&&") && !left && sibling->hasKnownIntValue()
                && !sibling->getKnownIntValue())
                return true;

            // logical or
            if (Token::simpleMatch(parent, "||") && !left && sibling->hasKnownIntValue()
                && sibling->getKnownIntValue())
                return true;

            // ternary
            if (Token::simpleMatch(parent, ":") && Token::simpleMatch(parent->astParent(), "?")) {
                const Token *condTok = parent->astParent()->astOperand1();
                if (condTok->hasKnownIntValue() && static_cast<bool>(condTok->getKnownIntValue()) != left)
                    return true;
            }
        }

        tok = parent;
    }

    return false;
}

static bool unknownLeafValuesAreTemplateArgs(const Token *tok)
{
    if (!tok)
        return true;

    if (!tok->astOperand1() && !tok->astOperand2())
        return tok->isTemplateArg() || tok->hasKnownIntValue();

    return unknownLeafValuesAreTemplateArgs(tok->astOperand1())
           && unknownLeafValuesAreTemplateArgs(tok->astOperand2());
}

static const Token *skipUnreachableIfBranch(const Token *tok)
{
    const Token *condTok = tok->linkAt(-1);
    if (!condTok)
        return tok;

    if (!Token::simpleMatch(condTok->tokAt(-1), "if") && !Token::simpleMatch(condTok->tokAt(-2), "if constexpr"))
        return tok;

    condTok = condTok->astOperand2();
    if (!condTok)
        return tok;

    if ((condTok->hasKnownIntValue() && condTok->getKnownIntValue() == 0)
        || (unknownLeafValuesAreTemplateArgs(condTok) && condTok->getValue(0))) {
        tok = tok->link();
    }

    return tok;
}

static const Token *skipUnreachableElseBranch(const Token *tok)
{
    if (!Token::simpleMatch(tok->tokAt(-2), "} else {"))
        return tok;

    const Token *condTok = tok->linkAt(-2);
    if (!condTok)
        return tok;

    condTok = condTok->linkAt(-1);
    if (!condTok)
        return tok;

    if (!Token::simpleMatch(condTok->tokAt(-1), "if (") && !Token::simpleMatch(condTok->tokAt(-2), "if constexpr ("))
        return tok;

    condTok = condTok->astOperand2();

    if ((condTok->hasKnownIntValue() && condTok->getKnownIntValue() != 0)
        || (unknownLeafValuesAreTemplateArgs(condTok) && condTok->getValueNE(0))) {
        tok = tok->link();
    }

    return tok;
}

const Token *skipUnreachableBranch(const Token *tok)
{
    if (!Token::simpleMatch(tok, "{"))
        return tok;

    if (tok->scope()->type == ScopeType::eIf) {
        return skipUnreachableIfBranch(tok);
    }

    if (tok->scope()->type == ScopeType::eElse) {
        return skipUnreachableElseBranch(tok);
    }

    return tok;
}
