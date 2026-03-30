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
// Auto variables checks
//---------------------------------------------------------------------------

#include "checkautovariables.h"

#include "astutils.h"
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm>
#include <list>
#include <unordered_set>
#include <utility>
#include <vector>

//---------------------------------------------------------------------------


// Register this check class into cppcheck by creating a static instance of it..
namespace {
    CheckAutoVariables instance;
}

static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE562(562U);  // Return of Stack Variable Address
static const CWE CWE590(590U);  // Free of Memory not on the Heap

static bool isPtrArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && var->isPointer());
}

static bool isArrayArg(const Token *tok, const Settings& settings)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && var->isArray() && (!var->scope() || !settings.library.isentrypoint(var->scope()->className)));
}

static bool isArrayVar(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArray() && !var->isArgument());
}

static bool isRefPtrArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && var->isReference() && var->isPointer());
}

static bool isNonReferenceArg(const Token *tok)
{
    const Variable *var = tok->variable();
    return (var && var->isArgument() && !var->isReference() && (var->isPointer() || (var->valueType() && var->valueType()->type >= ValueType::Type::CONTAINER) || var->type()));
}

static bool isAutoVar(const Token *tok)
{
    const Variable *var = tok->variable();

    if (!var || !var->isLocal() || var->isStatic())
        return false;

    if (var->isReference()) {
        // address of reference variable can be taken if the address
        // of the variable it points at is not a auto-var
        // TODO: check what the reference variable references.
        return false;
    }

    if (Token::Match(tok, "%name% .|::")) {
        do {
            tok = tok->tokAt(2);
        } while (Token::Match(tok, "%name% .|::"));
        if (Token::Match(tok, "%name% ("))
            return false;
        if (tok->variable() && tok->variable()->isPointer())
            return false;
    }
    return true;
}

static bool isAutoVarArray(const Token *tok)
{
    if (!tok)
        return false;

    // &x[..]
    if (tok->isUnaryOp("&") && Token::simpleMatch(tok->astOperand1(), "["))
        return isAutoVarArray(tok->astOperand1()->astOperand1());

    // x+y
    if (tok->str() == "+")
        return isAutoVarArray(tok->astOperand1()) || isAutoVarArray(tok->astOperand2());

    // x-intexpr
    if (tok->str() == "-")
        return isAutoVarArray(tok->astOperand1()) &&
               tok->astOperand2() &&
               tok->astOperand2()->valueType() &&
               tok->astOperand2()->valueType()->isIntegral();

    const Variable *var = tok->variable();
    if (!var)
        return false;

    // Variable
    if (var->isLocal() && !var->isStatic() && var->isArray() && !var->isPointer())
        return true;

    // ValueFlow
    if (var->isPointer() && !var->isArgument()) {
        for (auto it = tok->values().cbegin(); it != tok->values().cend(); ++it) {
            const ValueFlow::Value &val = *it;
            if (val.isTokValue() && isAutoVarArray(val.tokvalue))
                return true;
        }
    }

    return false;
}

static bool isLocalContainerBuffer(const Token* tok, const Settings& settings)
{
    if (!tok)
        return false;

    // x+y
    if (tok->str() == "+")
        return isLocalContainerBuffer(tok->astOperand1(), settings) || isLocalContainerBuffer(tok->astOperand2(), settings);

    if (tok->str() != "(" || !Token::simpleMatch(tok->astOperand1(), "."))
        return false;

    tok = tok->astOperand1()->astOperand1();

    const Variable* var = tok->variable();
    if (!var || !var->isLocal() || var->isStatic())
        return false;

    const Library::Container::Yield yield = astContainerYield(tok, settings.library);

    return yield == Library::Container::Yield::BUFFER || yield == Library::Container::Yield::BUFFER_NT;
}

static bool isAddressOfLocalVariable(const Token *expr)
{
    if (!expr)
        return false;
    if (Token::Match(expr, "+|-"))
        return isAddressOfLocalVariable(expr->astOperand1()) || isAddressOfLocalVariable(expr->astOperand2());
    if (expr->isCast())
        return isAddressOfLocalVariable(expr->astOperand2() ? expr->astOperand2() : expr->astOperand1());
    if (expr->isUnaryOp("&")) {
        const Token *op = expr->astOperand1();
        bool deref = false;
        while (Token::Match(op, ".|[")) {
            if (op->originalName() == "->")
                return false;
            if (op->str() == "[")
                deref = true;
            op = op->astOperand1();
        }
        return op && isAutoVar(op) && (!deref || !op->variable()->isPointer());
    }
    return false;
}

static bool variableIsUsedInScope(const Token* start, nonneg int varId, const Scope *scope)
{
    if (!start) // Ticket #5024
        return false;

    for (const Token *tok = start; tok && tok != scope->bodyEnd; tok = tok->next()) {
        if (tok->varId() == varId)
            return true;
        const ScopeType scopeType = tok->scope()->type;
        if (scopeType == ScopeType::eFor || scopeType == ScopeType::eDo || scopeType == ScopeType::eWhile) // In case of loops, better checking would be necessary
            return true;
        if (Token::simpleMatch(tok, "asm ("))
            return true;
    }
    return false;
}

void CheckAutoVariables::assignFunctionArg()
{
    const bool printStyle = mSettings->severity.isEnabled(Severity::style);
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    if (!printStyle && !printWarning && !mSettings->isPremiumEnabled("uselessAssignmentPtrArg"))
        return;

    logChecker("CheckAutoVariables::assignFunctionArg"); // style,warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            // TODO: What happens if this is removed?
            if (tok->astParent())
                continue;
            if (!(tok->isAssignmentOp() || tok->tokType() == Token::eIncDecOp) || !Token::Match(tok->astOperand1(), "%var%"))
                continue;
            const Token* const vartok = tok->astOperand1();
            if (isNonReferenceArg(vartok) &&
                !Token::Match(vartok->next(), "= %varid% ;", vartok->varId()) &&
                !variableIsUsedInScope(Token::findsimplematch(vartok->next(), ";"), vartok->varId(), scope) &&
                !Token::findsimplematch(vartok, "goto", scope->bodyEnd)) {
                if (vartok->variable()->isPointer() && printWarning)
                    errorUselessAssignmentPtrArg(vartok);
                else if (printStyle)
                    errorUselessAssignmentArg(vartok);
            }
        }
    }
}

static bool isAutoVariableRHS(const Token* tok, const Settings& settings) {
    return isAddressOfLocalVariable(tok) || isAutoVarArray(tok) || isLocalContainerBuffer(tok, settings);
}

static bool hasOverloadedAssignment(const Token* tok, bool& inconclusive)
{
    inconclusive = false;
    if (tok->isC())
        return false;
    if (const ValueType* vt = tok->valueType()) {
        if (vt->pointer && !Token::simpleMatch(tok->astParent(), "*"))
            return false;
        if (vt->container && vt->container->stdStringLike)
            return true;
        if (vt->typeScope)
            return std::any_of(vt->typeScope->functionList.begin(), vt->typeScope->functionList.end(), [](const Function& f) { // TODO: compare argument type
                return f.name() == "operator=";
            });
        return false;
    }
    inconclusive = true;
    return true;
}

static bool isMemberAssignment(const Token* tok, const Token*& rhs, const Settings& settings)
{
    const Token *endBracket = nullptr;
    if (!Token::Match(tok, "[;{}] %var% . %var%")) {
        if (!Token::Match(tok, "[;{}] %var% ["))
            return false;
        endBracket = tok->linkAt(2);
        if (!Token::Match(endBracket, "] . %var%"))
            return false;
    }
    if (!isPtrArg(tok->next()))
        return false;
    const Token* assign = (endBracket ? endBracket->next() : tok->tokAt(2))->astParent();
    while (Token::simpleMatch(assign, "["))
        assign = assign->astParent();
    if (!Token::simpleMatch(assign, "="))
        return false;
    if (!isAutoVariableRHS(assign->astOperand2(), settings))
        return false;
    rhs = assign->astOperand2();
    return true;
}

void CheckAutoVariables::autoVariables()
{
    logChecker("CheckAutoVariables::autoVariables");

    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            // Skip lambda..
            if (const Token *lambdaEndToken = findLambdaEndToken(tok)) {
                tok = lambdaEndToken;
                continue;
            }
            // Critical assignment
            const Token* rhs{};
            if (Token::Match(tok, "[;{}] %var% =") && isRefPtrArg(tok->next()) && isAutoVariableRHS(tok->tokAt(2)->astOperand2(), *mSettings)) {
                checkAutoVariableAssignment(tok->next(), false);
            } else if (Token::Match(tok, "[;{}] * %var% =") && isPtrArg(tok->tokAt(2)) && isAutoVariableRHS(tok->tokAt(3)->astOperand2(), *mSettings)) {
                const Token* lhs = tok->tokAt(2);
                bool inconclusive = false;
                if (!hasOverloadedAssignment(lhs, inconclusive) || (printInconclusive && inconclusive))
                    checkAutoVariableAssignment(tok->next(), inconclusive);
                tok = tok->tokAt(4);
            } else if (isMemberAssignment(tok, rhs, *mSettings)) {
                const Token* lhs = tok->tokAt(3);
                bool inconclusive = false;
                if (!hasOverloadedAssignment(lhs, inconclusive) || (printInconclusive && inconclusive))
                    checkAutoVariableAssignment(tok->next(), inconclusive);
                tok = rhs;
            } else if (Token::Match(tok, "[;{}] %var% [") && Token::simpleMatch(tok->linkAt(2), "] =") &&
                       (isPtrArg(tok->next()) || isArrayArg(tok->next(), *mSettings)) &&
                       isAutoVariableRHS(tok->linkAt(2)->next()->astOperand2(), *mSettings)) {
                errorAutoVariableAssignment(tok->next(), false);
            }
            // Invalid pointer deallocation
            else if ((Token::Match(tok, "%name% ( %var%|%str% ) ;") && mSettings->library.getDeallocFuncInfo(tok)) ||
                     (tok->isCpp() && Token::Match(tok, "delete [| ]| (| %var%|%str% !!["))) {
                tok = Token::findmatch(tok->next(), "%var%|%str%");
                if (Token::simpleMatch(tok->astParent(), "."))
                    continue;
                if (isArrayVar(tok) || tok->tokType() == Token::eString)
                    errorInvalidDeallocation(tok, nullptr);
                else if (tok->variable() && tok->variable()->isPointer()) {
                    for (const ValueFlow::Value &v : tok->values()) {
                        if (v.isImpossible())
                            continue;
                        if ((v.isTokValue() && (isArrayVar(v.tokvalue) || (v.tokvalue->tokType() == Token::eString))) ||
                            (v.isLocalLifetimeValue() && v.lifetimeKind == ValueFlow::Value::LifetimeKind::Address && !Token::simpleMatch(v.tokvalue, "("))) {
                            errorInvalidDeallocation(tok, &v);
                            break;
                        }
                    }
                }
            } else if ((Token::Match(tok, "%name% ( & %var% ) ;") && mSettings->library.getDeallocFuncInfo(tok)) ||
                       (tok->isCpp() && Token::Match(tok, "delete [| ]| (| & %var% !!["))) {
                tok = Token::findmatch(tok->next(), "%var%");
                if (isAutoVar(tok))
                    errorInvalidDeallocation(tok, nullptr);
            }
        }
    }
}

bool CheckAutoVariables::checkAutoVariableAssignment(const Token *expr, bool inconclusive, const Token *startToken)
{
    if (!startToken)
        startToken = Token::findsimplematch(expr, "=")->next();
    for (const Token *tok = startToken; tok; tok = tok->next()) {
        if (tok->str() == "}" && tok->scope()->type == ScopeType::eFunction) {
            errorAutoVariableAssignment(expr, inconclusive);
            return true;
        }

        if (Token::Match(tok, "return|throw|break|continue")) {
            errorAutoVariableAssignment(expr, inconclusive);
            return true;
        }
        if (Token::simpleMatch(tok, "=")) {
            const Token *lhs = tok;
            while (Token::Match(lhs->previous(), "%name%|.|*|]")) {
                if (lhs->linkAt(-1))
                    lhs = lhs->linkAt(-1);
                else
                    lhs = lhs->previous();
            }
            const Token *e = expr;
            while (e->str() != "=" && lhs->str() == e->str()) {
                e = e->next();
                lhs = lhs->next();
            }
            if (lhs->str() == "=")
                return false;
        }

        if (Token::simpleMatch(tok, "if (")) {
            const Token *ifStart = tok->linkAt(1)->next();
            return checkAutoVariableAssignment(expr, inconclusive, ifStart) || checkAutoVariableAssignment(expr, inconclusive, ifStart->link()->next());
        }
        if (Token::simpleMatch(tok, "} else {"))
            tok = tok->linkAt(2);
    }
    return false;
}

//---------------------------------------------------------------------------

void CheckAutoVariables::errorAutoVariableAssignment(const Token *tok, bool inconclusive)
{
    if (!inconclusive) {
        reportError(tok, Severity::error, "autoVariables",
                    "Address of local auto-variable assigned to a function parameter.\n"
                    "Dangerous assignment - the function parameter is assigned the address of a local "
                    "auto-variable. Local auto-variables are reserved from the stack which "
                    "is freed when the function ends. So the pointer to a local variable "
                    "is invalid after the function ends.", CWE562, Certainty::normal);
    } else {
        reportError(tok, Severity::error, "autoVariables",
                    "Address of local auto-variable assigned to a function parameter.\n"
                    "Function parameter is assigned the address of a local auto-variable. "
                    "Local auto-variables are reserved from the stack which is freed when "
                    "the function ends. The address is invalid after the function ends and it "
                    "might 'leak' from the function through the parameter.",
                    CWE562,
                    Certainty::inconclusive);
    }
}

void CheckAutoVariables::errorUselessAssignmentArg(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "uselessAssignmentArg",
                "Assignment of function parameter has no effect outside the function.", CWE398, Certainty::normal);
}

void CheckAutoVariables::errorUselessAssignmentPtrArg(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "uselessAssignmentPtrArg",
                "Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?", CWE398, Certainty::normal);
}

bool CheckAutoVariables::diag(const Token* tokvalue)
{
    if (!tokvalue)
        return true;
    return !mDiagDanglingTemp.insert(tokvalue).second;
}

//---------------------------------------------------------------------------

static bool isInScope(const Token * tok, const Scope * scope)
{
    if (!tok)
        return false;
    if (!scope)
        return false;
    const Variable * var = tok->variable();
    if (var && (var->isGlobal() || var->isStatic() || var->isExtern()))
        return false;
    if (tok->scope() && !tok->scope()->isClassOrStructOrUnion() && tok->scope()->isNestedIn(scope))
        return true;
    if (!var)
        return false;
    if (var->isArgument() && !var->isReference()) {
        const Scope * tokScope = tok->scope();
        if (!tokScope)
            return false;
        if (std::any_of(tokScope->nestedList.cbegin(), tokScope->nestedList.cend(), [&](const Scope* argScope) {
            return argScope && argScope->isNestedIn(scope);
        }))
            return true;
    }
    return false;
}

static bool isDeadScope(const Token * tok, const Scope * scope)
{
    if (!tok)
        return false;
    if (!scope)
        return false;
    const Variable * var = tok->variable();
    if (var && (!var->isLocal() || var->isStatic() || var->isExtern()))
        return false;
    if (tok->scope() && tok->scope()->bodyEnd != scope->bodyEnd && precedes(tok->scope()->bodyEnd, scope->bodyEnd))
        return true;
    return false;
}

static int getPointerDepth(const Token *tok)
{
    if (!tok)
        return 0;
    if (tok->valueType())
        return tok->valueType()->pointer;
    int n = 0;
    std::pair<const Token*, const Token*> decl = Token::typeDecl(tok);
    for (const Token* tok2 = decl.first; tok2 != decl.second; tok2 = tok2->next())
        if (Token::simpleMatch(tok2, "*"))
            n++;
    return n;
}

static bool isDeadTemporary(const Token* tok, const Token* expr, const Library& library)
{
    if (!isTemporary(tok, &library))
        return false;
    if (expr) {
        if (!precedes(nextAfterAstRightmostLeaf(tok->astTop()), nextAfterAstRightmostLeaf(expr->astTop())))
            return false;
        const Token* parent = tok->astParent();
        if (Token::simpleMatch(parent, "{") && Token::simpleMatch(parent->astParent(), ":"))
            parent = parent->astParent();
        // Is in a for loop
        if (astIsRHS(tok) && Token::simpleMatch(parent, ":") && Token::simpleMatch(parent->astParent(), "(") && Token::simpleMatch(parent->astParent()->previous(), "for (")) {
            const Token* braces = parent->astParent()->link()->next();
            if (precedes(braces, expr) && precedes(expr, braces->link()))
                return false;
        }
    }
    return true;
}

static bool isEscapedReference(const Variable* var)
{
    if (!var)
        return false;
    if (!var->isReference())
        return false;
    const Token * const varDeclEndToken = var->declEndToken();
    if (!varDeclEndToken)
        return false;
    if (!Token::simpleMatch(varDeclEndToken, "="))
        return false;
    const Token* vartok = varDeclEndToken->astOperand2();
    return !isTemporary(vartok, nullptr, false);
}

static bool isDanglingSubFunction(const Token* tokvalue, const Token* tok)
{
    if (!tokvalue)
        return false;
    const Variable* var = tokvalue->variable();
    if (!var->isLocal())
        return false;
    const Function* f = Scope::nestedInFunction(tok->scope());
    if (!f)
        return false;
    const Token* parent = tokvalue->astParent();
    while (parent && !Token::Match(parent->previous(), "%name% (")) {
        parent = parent->astParent();
    }
    if (!Token::simpleMatch(parent, "("))
        return false;
    return exprDependsOnThis(parent);
}

static const Variable* getParentVar(const Token* tok)
{
    if (!tok)
        return nullptr;
    if (Token::simpleMatch(tok, "."))
        return getParentVar(tok->astOperand1());
    return tok->variable();
}

static bool isAssignedToNonLocal(const Token* tok)
{
    if (!Token::simpleMatch(tok->astParent(), "="))
        return false;
    if (!astIsRHS(tok))
        return false;
    const Variable* var = getParentVar(tok->astParent()->astOperand1());
    if (!var)
        return false;
    return !var->isLocal() || var->isStatic();
}

void CheckAutoVariables::checkVarLifetimeScope(const Token * start, const Token * end)
{
    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    if (!start)
        return;
    const Scope * scope = start->scope();
    if (!scope)
        return;
    // If the scope is not set correctly then skip checking it
    if (scope->bodyStart != start)
        return;
    const bool returnRef = Function::returnsReference(scope->function);
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        // Return reference from function
        if (returnRef && Token::simpleMatch(tok->astParent(), "return")) {
            for (const ValueFlow::LifetimeToken& lt : ValueFlow::getLifetimeTokens(tok, *mSettings, true)) {
                if (!printInconclusive && lt.inconclusive)
                    continue;
                const Variable* var = lt.token->variable();
                if (var && !var->isGlobal() && !var->isStatic() && !var->isReference() && !var->isRValueReference() &&
                    isInScope(var->nameToken(), tok->scope())) {
                    errorReturnReference(tok, lt.errorPath, lt.inconclusive);
                    break;
                }
                if (isDeadTemporary(lt.token, nullptr, mSettings->library)) {
                    errorReturnTempReference(tok, lt.errorPath, lt.inconclusive);
                    break;
                }
            }
            // Assign reference to non-local variable
        } else if (Token::Match(tok->previous(), "&|&& %var% =") && tok->astParent() == tok->next() &&
                   tok->variable() && tok->variable()->nameToken() == tok &&
                   tok->variable()->declarationId() == tok->varId() && tok->variable()->isStatic() &&
                   !tok->variable()->isArgument()) {
            ErrorPath errorPath;
            const Variable *var = ValueFlow::getLifetimeVariable(tok, errorPath, *mSettings);
            if (var && isInScope(var->nameToken(), tok->scope())) {
                errorDanglingReference(tok, var, std::move(errorPath));
                continue;
            }
            // Reference to temporary
        } else if (tok->variable() && (tok->variable()->isReference() || tok->variable()->isRValueReference())) {
            for (const ValueFlow::LifetimeToken& lt : ValueFlow::getLifetimeTokens(getParentLifetime(tok), *mSettings)) {
                if (!printInconclusive && lt.inconclusive)
                    continue;
                const Token * tokvalue = lt.token;
                if (isDeadTemporary(tokvalue, tok, mSettings->library)) {
                    errorDanglingTempReference(tok, lt.errorPath, lt.inconclusive);
                    break;
                }
            }
        }
        const bool escape = Token::simpleMatch(tok->astParent(), "throw") ||
                            (Token::simpleMatch(tok->astParent(), "return") && !Function::returnsStandardType(scope->function));
        std::unordered_set<const Token*> exprs;
        for (const ValueFlow::Value& val:tok->values()) {
            if (!val.isLocalLifetimeValue() && !val.isSubFunctionLifetimeValue())
                continue;
            if (!printInconclusive && val.isInconclusive())
                continue;
            const Token* parent = getParentLifetime(val.tokvalue, mSettings->library);
            if (!exprs.insert(parent).second)
                continue;
            for (const ValueFlow::LifetimeToken& lt : ValueFlow::getLifetimeTokens(parent, *mSettings, escape || isAssignedToNonLocal(tok))) {
                const Token * tokvalue = lt.token;
                if (val.isLocalLifetimeValue()) {
                    if (escape) {
                        if (getPointerDepth(tok) < getPointerDepth(tokvalue))
                            continue;
                        if (!ValueFlow::isLifetimeBorrowed(tok, *mSettings))
                            continue;
                        if (tokvalue->exprId() == tok->exprId() && !(tok->variable() && tok->variable()->isArray()) &&
                            !astIsContainerView(tok->astParent()))
                            continue;
                        if ((tokvalue->variable() && !isEscapedReference(tokvalue->variable()) &&
                             isInScope(tokvalue->variable()->nameToken(), scope)) ||
                            isDeadTemporary(tokvalue, nullptr, mSettings->library)) {
                            if (!diag(tokvalue))
                                errorReturnDanglingLifetime(tok, &val);
                            break;
                        }
                    } else if (tokvalue->variable() && isDeadScope(tokvalue->variable()->nameToken(), tok->scope())) {
                        errorInvalidLifetime(tok, &val);
                        break;
                    } else if (!tokvalue->variable() &&
                               isDeadTemporary(tokvalue, tok, mSettings->library)) {
                        if (!diag(tokvalue))
                            errorDanglingTemporaryLifetime(tok, &val, tokvalue);
                        break;
                    }
                }
                if (tokvalue->variable() && (isInScope(tokvalue->variable()->nameToken(), tok->scope()) ||
                                             (val.isSubFunctionLifetimeValue() && isDanglingSubFunction(tokvalue, tok)))) {
                    const Variable * var = nullptr;
                    const Token * tok2 = tok;
                    if (Token::simpleMatch(tok->astParent(), "=")) {
                        if (astIsRHS(tok)) {
                            var = getParentVar(tok->astParent()->astOperand1());
                            tok2 = tok->astParent()->astOperand1();
                        }
                    } else if (tok->variable() && tok->variable()->declarationId() == tok->varId()) {
                        var = tok->variable();
                    }
                    if (!ValueFlow::isLifetimeBorrowed(tok, *mSettings))
                        continue;
                    const Token* nextTok = nextAfterAstRightmostLeaf(tok->astTop());
                    if (!nextTok)
                        nextTok = tok->next();
                    if (var && (!var->isLocal() || var->isStatic()) && !var->isArgument() && !(val.tokvalue && val.tokvalue->variable() && val.tokvalue->variable()->isStatic()) &&
                        !isVariableChanged(nextTok,
                                           tok->scope()->bodyEnd,
                                           var->valueType() ? var->valueType()->pointer : 0,
                                           var->declarationId(),
                                           var->isGlobal(),
                                           *mSettings)) {
                        errorDanglngLifetime(tok2, &val, var->isLocal());
                        break;
                    }
                }
            }
        }
        const Token *lambdaEndToken = findLambdaEndToken(tok);
        if (lambdaEndToken) {
            checkVarLifetimeScope(lambdaEndToken->link(), lambdaEndToken);
            tok = lambdaEndToken;
        }
        if (tok->str() == "{" && tok->scope()) {
            // Check functions in local classes
            if (tok->scope()->type == ScopeType::eClass ||
                tok->scope()->type == ScopeType::eStruct ||
                tok->scope()->type == ScopeType::eUnion) {
                for (const Function& f:tok->scope()->functionList) {
                    if (f.functionScope)
                        checkVarLifetimeScope(f.functionScope->bodyStart, f.functionScope->bodyEnd);
                }
                tok = tok->link();
            }
        }
    }
}

void CheckAutoVariables::checkVarLifetime()
{
    logChecker("CheckAutoVariables::checkVarLifetime");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (!scope->function)
            continue;
        checkVarLifetimeScope(scope->bodyStart, scope->bodyEnd);
    }
}

void CheckAutoVariables::errorReturnDanglingLifetime(const Token *tok, const ValueFlow::Value *val)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Returning " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "returnDanglingLifetime", msg + " that will be invalid when returning.", CWE562, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorInvalidLifetime(const Token *tok, const ValueFlow::Value* val)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "invalidLifetime", msg + " that is out of scope.", CWE562, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorDanglingTemporaryLifetime(const Token* tok, const ValueFlow::Value* val, const Token* tempTok)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tempTok, "Temporary created here.");
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath),
                Severity::error,
                "danglingTemporaryLifetime",
                msg + " that is a temporary.",
                CWE562,
                inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorDanglngLifetime(const Token *tok, const ValueFlow::Value *val, bool isStatic)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    ErrorPath errorPath = val ? val->errorPath : ErrorPath();
    std::string tokName = tok ? tok->expressionString() : "x";
    std::string msg = isStatic ? "Static" : "Non-local";
    msg += " variable '" + tokName + "' will use " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "danglingLifetime", msg + ".", CWE562, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorDanglingTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive)
{
    errorPath.emplace_back(tok, "");
    reportError(
        std::move(errorPath), Severity::error, "danglingTempReference", "Using reference to dangling temporary.", CWE562, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorReturnReference(const Token* tok, ErrorPath errorPath, bool inconclusive)
{
    errorPath.emplace_back(tok, "");
    reportError(
        std::move(errorPath), Severity::error, "returnReference", "Reference to local variable returned.", CWE562, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorDanglingReference(const Token *tok, const Variable *var, ErrorPath errorPath)
{
    std::string tokName = tok ? tok->str() : "x";
    std::string varName = var ? var->name() : "y";
    std::string msg = "Non-local reference variable '" + tokName + "' to local variable '" + varName + "'";
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "danglingReference", msg, CWE562, Certainty::normal);
}

void CheckAutoVariables::errorReturnTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive)
{
    errorPath.emplace_back(tok, "");
    reportError(
        std::move(errorPath), Severity::error, "returnTempReference", "Reference to temporary returned.", CWE562, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckAutoVariables::errorInvalidDeallocation(const Token *tok, const ValueFlow::Value *val)
{
    const Variable *var = val ? val->tokvalue->variable() : (tok ? tok->variable() : nullptr);

    std::string type = "an auto-variable";
    if (tok && tok->tokType() == Token::eString)
        type = "a string literal";
    else if (val && val->tokvalue->tokType() == Token::eString)
        type = "a pointer pointing to a string literal";
    else if (var) {
        if (var->isGlobal())
            type = "a global variable";
        else if (var->isStatic())
            type = "a static variable";
    }

    if (val)
        type += " (" + val->tokvalue->str() + ")";

    reportError(getErrorPath(tok, val, "Deallocating memory that was not dynamically allocated"),
                Severity::error,
                "autovarInvalidDeallocation",
                "Deallocation of " + type + " results in undefined behaviour.\n"
                "The deallocation of " + type + " results in undefined behaviour. You should only free memory "
                "that has been allocated dynamically.", CWE590, Certainty::normal);
}

void CheckAutoVariables::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckAutoVariables checkAutoVariables(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkAutoVariables.assignFunctionArg();
    checkAutoVariables.checkVarLifetime();
    checkAutoVariables.autoVariables();
}

void CheckAutoVariables::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckAutoVariables c(nullptr,settings,errorLogger);
    c.errorAutoVariableAssignment(nullptr, false);
    c.errorReturnReference(nullptr, ErrorPath{}, false);
    c.errorDanglingReference(nullptr, nullptr, ErrorPath{});
    c.errorReturnTempReference(nullptr, ErrorPath{}, false);
    c.errorDanglingTempReference(nullptr, ErrorPath{}, false);
    c.errorInvalidDeallocation(nullptr, nullptr);
    c.errorUselessAssignmentArg(nullptr);
    c.errorUselessAssignmentPtrArg(nullptr);
    c.errorReturnDanglingLifetime(nullptr, nullptr);
    c.errorInvalidLifetime(nullptr, nullptr);
    c.errorDanglngLifetime(nullptr, nullptr);
    c.errorDanglingTemporaryLifetime(nullptr, nullptr, nullptr);
}
