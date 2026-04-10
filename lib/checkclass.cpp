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
#include "checkclass.h"

#include "astutils.h"
#include "library.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "platform.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <utility>
#include <unordered_map>

#include "xml.h"

//---------------------------------------------------------------------------

// Register CheckClass..
namespace {
    CheckClass instance;
}

static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE404(404U);  // Improper Resource Shutdown or Release
static const CWE CWE665(665U);  // Improper Initialization
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE762(762U);  // Mismatched Memory Management Routines

static const CWE CWE_ONE_DEFINITION_RULE(758U);

static const char * getFunctionTypeName(FunctionType type)
{
    switch (type) {
    case FunctionType::eConstructor:
        return "constructor";
    case FunctionType::eCopyConstructor:
        return "copy constructor";
    case FunctionType::eMoveConstructor:
        return "move constructor";
    case FunctionType::eDestructor:
        return "destructor";
    case FunctionType::eFunction:
        return "function";
    case FunctionType::eOperatorEqual:
        return "operator=";
    case FunctionType::eLambda:
        return "lambda";
    }
    return "";
}

static bool isVariableCopyNeeded(const Variable &var, FunctionType type)
{
    bool isOpEqual = false;
    switch (type) {
    case FunctionType::eOperatorEqual:
        isOpEqual = true;
        break;
    case FunctionType::eCopyConstructor:
    case FunctionType::eMoveConstructor:
        break;
    default:
        return true;
    }

    return (!var.hasDefault() || isOpEqual) && // default init does not matter for operator=
           (var.isPointer() ||
            (var.type() && var.type()->needInitialization == Type::NeedInitialization::True) ||
            (var.valueType() && var.valueType()->type >= ValueType::Type::CHAR));
}

static bool isVclTypeInit(const Type *type)
{
    if (!type)
        return false;
    return std::any_of(type->derivedFrom.begin(), type->derivedFrom.end(), [&](const Type::BaseInfo& baseInfo) {
        if (!baseInfo.type)
            return true;
        if (isVclTypeInit(baseInfo.type))
            return true;
        return false;
    });
}
//---------------------------------------------------------------------------

CheckClass::CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : Check(myName(), tokenizer, settings, errorLogger),
    mSymbolDatabase(tokenizer?tokenizer->getSymbolDatabase():nullptr)
{}

bool CheckClass::isInitialized(const CheckClass::Usage& usage, FunctionType funcType) const
{
    const Variable& var = *usage.var;

    if (usage.assign || usage.init || var.isStatic())
        return true;

    if (!var.nameToken() || var.nameToken()->isAnonymous())
        return true;

    if (var.valueType() && var.valueType()->pointer == 0 && var.type() && var.type()->needInitialization == Type::NeedInitialization::False && var.type()->derivedFrom.empty())
        return true;

    if (var.isConst() && funcType == FunctionType::eOperatorEqual) // We can't set const members in assignment operator
        return true;

    // Check if this is a class constructor
    if (!var.isPointer() && !var.isPointerArray() && var.isClass() && funcType == FunctionType::eConstructor) {
        // Unknown type so assume it is initialized
        if (!var.type()) {
            if (var.isStlType() && var.valueType() && var.valueType()->containerTypeToken) {
                if (var.valueType()->type == ValueType::Type::ITERATOR)
                {
                    // needs initialization
                }
                else if (var.getTypeName() == "std::array") {
                    const Token* ctt = var.valueType()->containerTypeToken;
                    if (!ctt->isStandardType() &&
                        (!ctt->type() || ctt->type()->needInitialization != Type::NeedInitialization::True) &&
                        !mSettings->library.podtype(ctt->str())) // TODO: handle complex type expression
                        return true;
                }
                else
                    return true;
            }
            else
                return true;
        }

        // Known type that doesn't need initialization or
        // known type that has member variables of an unknown type
        else if (var.type()->needInitialization != Type::NeedInitialization::True)
            return true;
    }

    // Check if type can't be copied
    if (!var.isPointer() && !var.isPointerArray() && var.typeScope()) {
        if (funcType == FunctionType::eMoveConstructor) {
            if (canNotMove(var.typeScope()))
                return true;
        }
        else {
            if (canNotCopy(var.typeScope()))
                return true;
        }
    }
    return false;
}

void CheckClass::handleUnionMembers(std::vector<Usage>& usageList)
{
    // Assign 1 union member => assign all union members
    for (const Usage& usage : usageList) {
        const Variable& var = *usage.var;
        if (!usage.assign && !usage.init)
            continue;
        const Scope* varScope1 = var.nameToken()->scope();
        while (varScope1->type == ScopeType::eStruct)
            varScope1 = varScope1->nestedIn;
        if (varScope1->type == ScopeType::eUnion) {
            for (Usage& usage2 : usageList) {
                const Variable& var2 = *usage2.var;
                if (usage2.assign || usage2.init || var2.isStatic())
                    continue;
                const Scope* varScope2 = var2.nameToken()->scope();
                while (varScope2->type == ScopeType::eStruct)
                    varScope2 = varScope2->nestedIn;
                if (varScope1 == varScope2)
                    usage2.assign = true;
            }
        }
    }
}

//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    const bool printStyle = mSettings->severity.isEnabled(Severity::style);
    const bool printWarnings = mSettings->severity.isEnabled(Severity::warning);
    if (!printStyle && !printWarnings && !mSettings->isPremiumEnabled("uninitMemberVar"))
        return;

    logChecker("CheckClass::checkConstructors"); // style,warning

    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        if (mSettings->hasLib("vcl") && isVclTypeInit(scope->definedType))
            continue;

        const bool unusedTemplate = Token::simpleMatch(scope->classDef->previous(), ">");

        const bool usedInUnion = std::any_of(mSymbolDatabase->scopeList.cbegin(), mSymbolDatabase->scopeList.cend(), [&](const Scope& unionScope) {
            if (unionScope.type != ScopeType::eUnion)
                return false;
            return std::any_of(unionScope.varlist.cbegin(), unionScope.varlist.cend(), [&](const Variable& var) {
                return var.type() && var.type()->classScope == scope;
            });
        });

        // There are no constructors.
        std::set<const Variable*> diagVars;
        if (scope->numConstructors == 0 && printStyle && !usedInUnion) {
            // If there is a private variable, there should be a constructor..
            int needInit = 0, haveInit = 0;
            std::vector<const Variable*> uninitVars;
            for (const Variable &var : scope->varlist) {
                if (var.isPrivate() && !var.isStatic() &&
                    (!var.isClass() || (var.type() && var.type()->needInitialization == Type::NeedInitialization::True))) {
                    ++needInit;
                    if (!var.isInit() && !var.hasDefault() && var.nameToken()->scope() == scope) // don't warn for anonymous union members
                        uninitVars.emplace_back(&var);
                    else
                        ++haveInit;
                }
            }
            if (needInit > haveInit) {
                if (haveInit == 0)
                    noConstructorError(scope->classDef, scope->className, scope->classDef->str() == "struct");
                else
                    for (const Variable* uv : uninitVars) {
                        uninitVarError(uv->typeStartToken(), uv->scope()->className, uv->name());
                        diagVars.emplace(uv);
                    }
            }
        }

        if (!printWarnings)
            continue;

        std::vector<Usage> usageList = createUsageList(scope);

        for (const Function &func : scope->functionList) {
            if (!(func.isConstructor() && (func.hasBody() || (func.isDefault() && func.type == FunctionType::eConstructor))) &&
                !(func.type == FunctionType::eOperatorEqual && func.hasBody()))
                continue; // a defaulted constructor does not initialize primitive members

            // Bail: If initializer list is not recognized as a variable or type then skip since parsing is incomplete
            if (unusedTemplate && func.type == FunctionType::eConstructor) {
                const Token *initList = func.constructorMemberInitialization();
                if (Token::Match(initList, ": %name% (") && initList->next()->tokType() == Token::eName)
                    break;
            }

            // Mark all variables not used
            clearAllVar(usageList);

            // Variables with default initializers
            for (Usage &usage : usageList) {
                const Variable& var = *usage.var;

                // check for C++11 initializer
                if (var.hasDefault() && func.type != FunctionType::eOperatorEqual && func.type != FunctionType::eCopyConstructor) { // variable still needs to be copied
                    usage.init = true;
                }
            }

            std::list<const Function *> callstack;
            initializeVarList(func, callstack, scope, usageList);

            handleUnionMembers(usageList);

            // Check if any variables are uninitialized
            for (const Usage &usage : usageList) {
                if (isInitialized(usage, func.type))
                    continue;

                // Is there missing member copy in copy/move constructor or assignment operator?
                const Variable& var = *usage.var;
                bool missingCopy = false;

                // Don't warn about unknown types in copy constructors since we
                // don't know if they can be copied or not..
                if (!isVariableCopyNeeded(var, func.type)) {
                    if (!printInconclusive)
                        continue;

                    missingCopy = true;
                }

                // It's non-static and it's not initialized => error
                if (func.type == FunctionType::eOperatorEqual) {
                    const Token *operStart = func.arg;

                    bool classNameUsed = false;
                    for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next()) {
                        if (operTok->str() == scope->className) {
                            classNameUsed = true;
                            break;
                        }
                    }

                    if (classNameUsed && mSettings->library.getTypeCheck("operatorEqVarError", var.getTypeName()) != Library::TypeCheck::suppress)
                        operatorEqVarError(func.token, scope->className, var.name(), missingCopy);
                } else if (func.access != AccessControl::Private || mSettings->standards.cpp >= Standards::CPP11) {
                    // If constructor is not in scope then we maybe using a constructor from a different template specialization
                    if (!precedes(scope->bodyStart, func.tokenDef))
                        continue;

                    const bool derived = scope != var.scope();
                    if (func.type == FunctionType::eConstructor &&
                        func.nestedIn && (func.nestedIn->numConstructors - func.nestedIn->numCopyOrMoveConstructors) > 1 &&
                        func.argCount() == 0 && func.functionScope &&
                        func.arg && func.arg->link()->next() == func.functionScope->bodyStart &&
                        func.functionScope->bodyStart->link() == func.functionScope->bodyStart->next()) {
                        // don't warn about user defined default constructor when there are other constructors
                        if (printInconclusive)
                            uninitVarError(func.token, func.access == AccessControl::Private, func.type, var.scope()->className, var.name(), derived, true);
                    } else if (missingCopy)
                        missingMemberCopyError(func.token, func.type, var.scope()->className, var.name());
                    else
                        uninitVarError(func.token, func.access == AccessControl::Private, func.type, var.scope()->className, var.name(), derived, false);
                }
            }
        }

        if (scope->numConstructors == 0) {

            // Mark all variables not used
            clearAllVar(usageList);

            // Variables with default initializers
            bool hasAnyDefaultInit = false;
            for (Usage& usage : usageList) {
                const Variable& var = *usage.var;

                // check for C++11 initializer
                if (var.hasDefault()) {
                    usage.init = true;
                    hasAnyDefaultInit = true;
                }
            }
            if (!hasAnyDefaultInit)
                continue;

            handleUnionMembers(usageList);

            // Check if any variables are uninitialized
            for (const Usage& usage : usageList) {
                if (isInitialized(usage, FunctionType::eConstructor))
                    continue;

                const Variable& var = *usage.var;
                if (diagVars.count(&var) == 0)
                    uninitVarError(scope->bodyStart, false, FunctionType::eConstructor, var.scope()->className, var.name(), false, false, true);
            }
        }
    }
}

static bool isPermissibleConversion(const std::string& type)
{
    return type == "std::initializer_list" || type == "std::nullptr_t";
}

void CheckClass::checkExplicitConstructors()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("noExplicitConstructor"))
        return;

    logChecker("CheckClass::checkExplicitConstructors"); // style

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        // Do not perform check, if the class/struct has not any constructors
        if (scope->numConstructors == 0)
            continue;

        // Is class abstract? Maybe this test is over-simplification, but it will suffice for simple cases,
        // and it will avoid false positives.
        const bool isAbstractClass = std::any_of(scope->functionList.cbegin(), scope->functionList.cend(), [](const Function& func) {
            return func.isPure();
        });

        // Abstract classes can't be instantiated. But if there is C++11
        // "misuse" by derived classes then these constructors must be explicit.
        if (isAbstractClass && mSettings->standards.cpp >= Standards::CPP11)
            continue;

        for (const Function &func : scope->functionList) {

            // We are looking for constructors, which are meeting following criteria:
            //  1) Constructor is declared with a single parameter
            //  2) Constructor is not declared as explicit
            //  3) It is not a copy/move constructor of non-abstract class
            //  4) Constructor is not marked as delete (programmer can mark the default constructor as deleted, which is ok)
            if (!func.isConstructor() || func.isDelete() || (!func.hasBody() && func.access == AccessControl::Private))
                continue;

            if (!func.isExplicit() &&
                func.argCount() > 0 && func.minArgCount() < 2 &&
                func.type != FunctionType::eCopyConstructor &&
                func.type != FunctionType::eMoveConstructor &&
                !(func.templateDef && Token::simpleMatch(func.argumentList.front().typeEndToken(), "...")) &&
                !isPermissibleConversion(func.argumentList.front().getTypeName())) {
                noExplicitConstructorError(func.tokenDef, scope->className, scope->type == ScopeType::eStruct);
            }
        }
    }
}

static bool hasNonCopyableBase(const Scope *scope, bool *unknown)
{
    // check if there is base class that is not copyable
    for (const Type::BaseInfo &baseInfo : scope->definedType->derivedFrom) {
        if (!baseInfo.type || !baseInfo.type->classScope) {
            *unknown = true;
            continue;
        }

        if (hasNonCopyableBase(baseInfo.type->classScope, unknown))
            return true;

        for (const Function &func : baseInfo.type->classScope->functionList) {
            if (func.type != FunctionType::eCopyConstructor)
                continue;
            if (func.access == AccessControl::Private || func.isDelete()) {
                *unknown = false;
                return true;
            }
        }
    }
    return false;
}

void CheckClass::copyconstructors()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckClass::checkCopyConstructors"); // warning

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        std::map<int, const Token*> allocatedVars;
        std::map<int, const Token*> deallocatedVars;

        for (const Function &func : scope->functionList) {
            if (func.type == FunctionType::eConstructor && func.functionScope) {
                // Allocations in constructors
                const Token* tok = func.token->linkAt(1);
                for (const Token* const end = func.functionScope->bodyStart; tok != end; tok = tok->next()) {
                    if (Token::Match(tok, "%var% ( new") ||
                        (Token::Match(tok, "%var% ( %name% (") && mSettings->library.getAllocFuncInfo(tok->tokAt(2)))) {
                        const Variable* var = tok->variable();
                        if (var && var->isPointer() && var->scope() == scope)
                            allocatedVars[tok->varId()] = tok;
                    }
                }
                for (const Token* const end = func.functionScope->bodyEnd; tok != end; tok = tok->next()) {
                    if (Token::Match(tok, "%var% = new") ||
                        (Token::Match(tok, "%var% = %name% (") && mSettings->library.getAllocFuncInfo(tok->tokAt(2)))) {
                        const Variable* var = tok->variable();
                        if (var && var->isPointer() && var->scope() == scope && !var->isStatic())
                            allocatedVars[tok->varId()] = tok;
                    }
                }
            } else if (func.type == FunctionType::eDestructor && func.functionScope) {
                // Deallocations in destructors
                const Token* tok = func.functionScope->bodyStart;
                for (const Token* const end = func.functionScope->bodyEnd; tok != end; tok = tok->next()) {
                    if (Token::Match(tok, "delete %var%") ||
                        (Token::Match(tok, "%name% ( %var%") && mSettings->library.getDeallocFuncInfo(tok))) {
                        const Token *vartok = tok->str() == "delete" ? tok->next() : tok->tokAt(2);
                        const Variable* var = vartok->variable();
                        if (var && var->isPointer() && var->scope() == scope && !var->isStatic())
                            deallocatedVars[vartok->varId()] = vartok;
                    }
                }
            }
        }

        const bool hasAllocatedVars = !allocatedVars.empty();
        const bool hasDeallocatedVars = !deallocatedVars.empty();

        if (hasAllocatedVars || hasDeallocatedVars) {
            const Function *funcCopyCtor = nullptr;
            const Function *funcOperatorEq = nullptr;
            const Function *funcDestructor = nullptr;
            for (const Function &func : scope->functionList) {
                if (func.type == FunctionType::eCopyConstructor)
                    funcCopyCtor = &func;
                else if (func.type == FunctionType::eOperatorEqual)
                    funcOperatorEq = &func;
                else if (func.type == FunctionType::eDestructor)
                    funcDestructor = &func;
            }
            if (!funcCopyCtor || funcCopyCtor->isDefault()) {
                bool unknown = false;
                if (!hasNonCopyableBase(scope, &unknown) && !unknown) {
                    if (hasAllocatedVars)
                        noCopyConstructorError(scope, funcCopyCtor, allocatedVars.cbegin()->second, unknown);
                    else
                        noCopyConstructorError(scope, funcCopyCtor, deallocatedVars.cbegin()->second, unknown);
                }
            }
            if (!funcOperatorEq || funcOperatorEq->isDefault()) {
                bool unknown = false;
                if (!hasNonCopyableBase(scope, &unknown) && !unknown) {
                    if (hasAllocatedVars)
                        noOperatorEqError(scope, funcOperatorEq, allocatedVars.cbegin()->second, unknown);
                    else
                        noOperatorEqError(scope, funcOperatorEq, deallocatedVars.cbegin()->second, unknown);
                }
            }
            if (!funcDestructor || funcDestructor->isDefault()) {
                const Token * mustDealloc = nullptr;
                for (auto it = allocatedVars.cbegin(); it != allocatedVars.cend(); ++it) {
                    if (!Token::Match(it->second, "%var% [(=] new %type%")) {
                        mustDealloc = it->second;
                        break;
                    }
                    if (it->second->valueType() && it->second->valueType()->isIntegral()) {
                        mustDealloc = it->second;
                        break;
                    }
                    const Variable *var = it->second->variable();
                    if (var && var->typeScope() && var->typeScope()->functionList.empty() && var->type()->derivedFrom.empty()) {
                        mustDealloc = it->second;
                        break;
                    }
                }
                if (mustDealloc)
                    noDestructorError(scope, funcDestructor, mustDealloc);
            }
        }

        std::set<const Token*> copiedVars;
        const Token* copyCtor = nullptr;
        for (const Function &func : scope->functionList) {
            if (func.type != FunctionType::eCopyConstructor)
                continue;
            copyCtor = func.tokenDef;
            if (!func.functionScope) {
                allocatedVars.clear();
                break;
            }
            const Token* tok = func.tokenDef->linkAt(1)->next();
            if (tok->str()==":") {
                tok=tok->next();
                while (Token::Match(tok, "%name% (")) {
                    if (allocatedVars.find(tok->varId()) != allocatedVars.end()) {
                        if (tok->varId() && Token::Match(tok->tokAt(2), "%name% . %name% )"))
                            copiedVars.insert(tok);
                        else if (!Token::Match(tok->tokAt(2), "%any% )"))
                            allocatedVars.erase(tok->varId()); // Assume memory is allocated
                    }
                    tok = tok->linkAt(1)->tokAt(2);
                }
            }
            for (tok = func.functionScope->bodyStart; tok != func.functionScope->bodyEnd; tok = tok->next()) {
                if ((tok->isCpp() && Token::Match(tok, "%var% = new")) ||
                    (Token::Match(tok, "%var% = %name% (") && (mSettings->library.getAllocFuncInfo(tok->tokAt(2)) || mSettings->library.getReallocFuncInfo(tok->tokAt(2))))) {
                    allocatedVars.erase(tok->varId());
                } else if (Token::Match(tok, "%var% = %name% . %name% ;") && allocatedVars.find(tok->varId()) != allocatedVars.end()) {
                    copiedVars.insert(tok);
                }
            }
            break;
        }
        if (copyCtor && !copiedVars.empty()) {
            for (const Token *cv : copiedVars)
                copyConstructorShallowCopyError(cv, cv->str());
            // throw error if count mismatch
            /* FIXME: This doesn't work. See #4154
               for (auto i = allocatedVars.cbegin(); i != allocatedVars.end(); ++i) {
                copyConstructorMallocError(copyCtor, i->second, i->second->str());
               }
             */
        }
    }
}

/* This doesn't work. See #4154
   void CheckClass::copyConstructorMallocError(const Token *cctor, const Token *alloc, const std::string& varname)
   {
    std::list<const Token*> callstack;
    callstack.push_back(cctor);
    callstack.push_back(alloc);
    reportError(callstack, Severity::warning, "copyCtorNoAllocation", "Copy constructor does not allocate memory for member '" + varname + "' although memory has been allocated in other constructors.");
   }
 */

void CheckClass::copyConstructorShallowCopyError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::warning, "copyCtorPointerCopying",
                "$symbol:" + varname + "\nValue of pointer '$symbol', which points to allocated memory, is copied in copy constructor instead of allocating new memory.", CWE398, Certainty::normal);
}

static std::string noMemberErrorMessage(const Scope *scope, const char function[], bool isdefault)
{
    const std::string &classname = scope ? scope->className : "class";
    const std::string type = (scope && scope->type == ScopeType::eStruct) ? "Struct" : "Class";
    const bool isDestructor = (function[0] == 'd');
    std::string errmsg = "$symbol:" + classname + '\n';

    if (isdefault) {
        errmsg += type + " '$symbol' has dynamic memory/resource allocation(s). The " + function + " is explicitly defaulted but the default " + function + " does not work well.";
        if (isDestructor)
            errmsg += " It is recommended to define the " + std::string(function) + '.';
        else
            errmsg += " It is recommended to define or delete the " + std::string(function) + '.';
    } else {
        errmsg += type + " '$symbol' does not have a " + function + " which is recommended since it has dynamic memory/resource management.";
    }

    return errmsg;
}

void CheckClass::noCopyConstructorError(const Scope *scope, bool isdefault, const Token *alloc, bool inconclusive)
{
    reportError(alloc, Severity::warning, "noCopyConstructor", noMemberErrorMessage(scope, "copy constructor", isdefault), CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckClass::noOperatorEqError(const Scope *scope, bool isdefault, const Token *alloc, bool inconclusive)
{
    reportError(alloc, Severity::warning, "noOperatorEq", noMemberErrorMessage(scope, "operator=", isdefault), CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckClass::noDestructorError(const Scope *scope, bool isdefault, const Token *alloc)
{
    reportError(alloc, Severity::warning, "noDestructor", noMemberErrorMessage(scope, "destructor", isdefault), CWE398, Certainty::normal);
}

bool CheckClass::canNotCopy(const Scope *scope)
{
    bool constructor = false;
    bool publicAssign = false;
    bool publicCopy = false;

    for (const Function &func : scope->functionList) {
        if (func.isConstructor())
            constructor = true;
        if (func.access != AccessControl::Public)
            continue;
        if (func.type == FunctionType::eCopyConstructor) {
            publicCopy = true;
            break;
        }
        if (func.type == FunctionType::eOperatorEqual) {
            publicAssign = true;
            break;
        }
    }

    return constructor && !(publicAssign || publicCopy);
}

bool CheckClass::canNotMove(const Scope *scope)
{
    bool constructor = false;
    bool publicAssign = false;
    bool publicCopy = false;
    bool publicMove = false;

    for (const Function &func : scope->functionList) {
        if (func.isConstructor())
            constructor = true;
        if (func.access != AccessControl::Public)
            continue;
        if (func.type == FunctionType::eCopyConstructor) {
            publicCopy = true;
            break;
        }
        if (func.type == FunctionType::eMoveConstructor) {
            publicMove = true;
            break;
        }
        if (func.type == FunctionType::eOperatorEqual) {
            publicAssign = true;
            break;
        }
    }

    return constructor && !(publicAssign || publicCopy || publicMove);
}

static void getAllVariableMembers(const Scope *scope, std::vector<const Variable *>& varList)
{
    std::transform(scope->varlist.cbegin(), scope->varlist.cend(), std::back_inserter(varList), [](const Variable& var) {
        return &var;
    });
    if (scope->definedType) {
        for (const Type::BaseInfo& baseInfo: scope->definedType->derivedFrom) {
            if (scope->definedType == baseInfo.type)
                continue;
            const Scope *baseClass = baseInfo.type ? baseInfo.type->classScope : nullptr;
            if (baseClass && baseClass->isClassOrStruct() && baseClass->numConstructors == 0)
                getAllVariableMembers(baseClass, varList);
        }
    }
}

std::vector<CheckClass::Usage> CheckClass::createUsageList(const Scope *scope)
{
    std::vector<Usage> ret;
    std::vector<const Variable *> varlist;
    getAllVariableMembers(scope, varlist);
    ret.reserve(varlist.size());
    std::transform(varlist.cbegin(), varlist.cend(), std::back_inserter(ret), [](const Variable* var) {
        return Usage(var);
    });
    return ret;
}

void CheckClass::assignVar(std::vector<Usage> &usageList, nonneg int varid)
{
    auto it = std::find_if(usageList.begin(), usageList.end(), [varid](const Usage& usage) {
        return usage.var->declarationId() == varid;
    });
    if (it != usageList.end())
        it->assign = true;
}

void CheckClass::assignVar(std::vector<Usage> &usageList, const Token* vartok)
{
    if (vartok->varId() > 0) {
        assignVar(usageList, vartok->varId());
        return;
    }
    auto it = std::find_if(usageList.begin(), usageList.end(), [vartok](const Usage& usage) {
        // FIXME: This is a workaround when varid is not set for a derived member
        return usage.var->name() == vartok->str();
    });
    if (it != usageList.end())
        it->assign = true;
}

void CheckClass::initVar(std::vector<Usage> &usageList, nonneg int varid)
{
    auto it = std::find_if(usageList.begin(), usageList.end(), [varid](const Usage& usage) {
        return usage.var->declarationId() == varid;
    });
    if (it != usageList.end())
        it->init = true;
}

void CheckClass::assignAllVar(std::vector<Usage> &usageList)
{
    for (Usage & i : usageList)
        i.assign = true;
}

void CheckClass::assignAllVarsVisibleFromScope(std::vector<Usage>& usageList, const Scope* scope)
{
    for (Usage& usage : usageList) {
        if (usage.var->scope() == scope)
            usage.assign = true;
    }

    // Iterate through each base class...
    for (const Type::BaseInfo& i : scope->definedType->derivedFrom) {
        const Type *derivedFrom = i.type;

        if (derivedFrom && derivedFrom->classScope)
            assignAllVarsVisibleFromScope(usageList, derivedFrom->classScope);
    }
}

void CheckClass::clearAllVar(std::vector<Usage> &usageList)
{
    for (Usage & i : usageList) {
        i.assign = false;
        i.init = false;
    }
}

bool CheckClass::isBaseClassMutableMemberFunc(const Token *tok, const Scope *scope)
{
    // Iterate through each base class...
    for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
        const Type *derivedFrom = i.type;

        // Check if base class exists in database
        if (derivedFrom && derivedFrom->classScope) {
            const std::list<Function>& functionList = derivedFrom->classScope->functionList;

            if (std::any_of(functionList.cbegin(), functionList.cend(), [&](const Function& func) {
                return func.tokenDef->str() == tok->str() && !func.isStatic() && !func.isConst();
            }))
                return true;

            if (isBaseClassMutableMemberFunc(tok, derivedFrom->classScope))
                return true;
        }

        // Base class not found so assume it is in it.
        else
            return true;
    }

    return false;
}

void CheckClass::initializeVarList(const Function &func, std::list<const Function *> &callstack, const Scope *scope, std::vector<Usage> &usage) const
{
    if (!func.functionScope)
        return;

    bool initList = func.isConstructor();
    const Token *ftok = func.arg->link()->next();
    int level = 0;
    for (; ftok && ftok != func.functionScope->bodyEnd; ftok = ftok->next()) {
        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (initList) {
            if (level == 0 && Token::Match(ftok, "%name% {|(") && Token::Match(ftok->linkAt(1), "}|) ,|{")) {
                if (ftok->str() != func.name()) {
                    if (ftok->varId())
                        initVar(usage, ftok->varId());
                    else { // base class constructor
                        for (Usage& u : usage) {
                            if (u.var->scope() != scope) // assume that all variables are initialized in base class
                                u.init = true;
                        }
                    }
                } else { // c++11 delegate constructor
                    const Function *member = ftok->function();
                    // member function not found => assume it initializes all members
                    if (!member) {
                        assignAllVar(usage);
                        return;
                    }

                    // recursive call
                    // assume that all variables are initialized
                    if (std::find(callstack.cbegin(), callstack.cend(), member) != callstack.cend()) {
                        /** @todo false negative: just bail */
                        assignAllVar(usage);
                        return;
                    }

                    // member function has implementation
                    if (member->hasBody()) {
                        // initialize variable use list using member function
                        callstack.push_back(member);
                        initializeVarList(*member, callstack, scope, usage);
                        callstack.pop_back();
                    }

                    // there is a called member function, but it has no implementation, so we assume it initializes everything
                    else {
                        assignAllVar(usage);
                    }
                }
            } else if (level != 0 && Token::Match(ftok, "%name% =")) // assignment in the initializer: var(value = x)
                assignVar(usage, ftok->varId());

            // Level handling
            if (ftok->link() && Token::Match(ftok, "(|<"))
                level++;
            else if (ftok->str() == "{") {
                if (level != 0 ||
                    (Token::Match(ftok->previous(), "%name%|>") && Token::Match(ftok->link(), "} ,|{")))
                    level++;
                else
                    initList = false;
            } else if (ftok->link() && Token::Match(ftok, ")|>|}"))
                level--;
        }

        if (initList)
            continue;

        // Variable getting value from stream?
        if (Token::Match(ftok, ">>|& %name%") && isLikelyStreamRead(ftok)) {
            assignVar(usage, ftok->next()->varId());
        }

        // If assignment comes after an && or || this is really inconclusive because of short circuiting
        if (Token::Match(ftok, "%oror%|&&"))
            continue;

        if (Token::simpleMatch(ftok, "( !"))
            ftok = ftok->next();

        // Using the operator= function to initialize all variables..
        if (Token::Match(ftok->next(), "return| (| * this )| =")) {
            assignAllVar(usage);
            break;
        }

        // Using swap to assign all variables..
        if (func.type == FunctionType::eOperatorEqual && Token::Match(ftok, "[;{}] %name% (") && Token::Match(ftok->linkAt(2), ") . %name% ( *| this ) ;")) {
            assignAllVar(usage);
            break;
        }

        // Calling member variable function?
        if (Token::Match(ftok->next(), "%var% . %name% (") && !(ftok->next()->valueType() && ftok->next()->valueType()->pointer)) {
            if (std::any_of(scope->varlist.cbegin(), scope->varlist.cend(), [&](const Variable& var) {
                return var.declarationId() == ftok->next()->varId();
            }))
                /** @todo false negative: we assume function changes variable state */
                assignVar(usage, ftok->next()->varId());

            ftok = ftok->tokAt(2);
        }

        if (!Token::Match(ftok->next(), "::| %name%") &&
            !Token::Match(ftok->next(), "*| this . %name%") &&
            !Token::Match(ftok->next(), "* %name% =") &&
            !Token::Match(ftok->next(), "( * this ) . %name%") &&
            !Token::Match(ftok->next(), "( * %name% ) =") &&
            !Token::Match(ftok->next(), "* ( %name% ) ="))
            continue;

        // Goto the first token in this statement..
        ftok = ftok->next();

        // skip "return"
        if (ftok->str() == "return")
            ftok = ftok->next();

        // Skip "( * this )"
        if (Token::simpleMatch(ftok, "( * this ) .")) {
            ftok = ftok->tokAt(5);
        }

        // Skip "this->"
        if (Token::simpleMatch(ftok, "this ."))
            ftok = ftok->tokAt(2);

        // Skip "classname :: "
        if (Token::Match(ftok, ":: %name%"))
            ftok = ftok->next();
        while (Token::Match(ftok, "%name% ::"))
            ftok = ftok->tokAt(2);

        // Clearing all variables..
        if (Token::Match(ftok, "::| memset ( this ,")) {
            assignAllVar(usage);
            return;
        }

        // Ticket #7068
        if (Token::Match(ftok, "::| memset ( &| this . %name%")) {
            if (ftok->str() == "::")
                ftok = ftok->next();
            int offsetToMember = 4;
            if (ftok->strAt(2) == "&")
                ++offsetToMember;
            assignVar(usage, ftok->tokAt(offsetToMember)->varId());
            ftok = ftok->linkAt(1);
            continue;
        }

        // Clearing array..
        if (Token::Match(ftok, "::| memset ( %name% ,")) {
            if (ftok->str() == "::")
                ftok = ftok->next();
            assignVar(usage, ftok->tokAt(2)->varId());
            ftok = ftok->linkAt(1);
            continue;
        }

        // Calling member function?
        if (Token::simpleMatch(ftok, "operator= (")) {
            if (ftok->function()) {
                const Function *member = ftok->function();
                // recursive call
                // assume that all variables are initialized
                if (std::find(callstack.cbegin(), callstack.cend(), member) != callstack.cend()) {
                    /** @todo false negative: just bail */
                    assignAllVar(usage);
                    return;
                }

                // member function has implementation
                if (member->hasBody()) {
                    // initialize variable use list using member function
                    callstack.push_back(member);
                    initializeVarList(*member, callstack, scope, usage);
                    callstack.pop_back();
                }

                // assume that a base class call to operator= assigns all its base members (but not more)
                else if (func.tokenDef->str() == ftok->str() && isBaseClassMutableMemberFunc(ftok, scope)) {
                    if (member->nestedIn)
                        assignAllVarsVisibleFromScope(usage, member->nestedIn->definedType->classScope);
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything
                else {
                    assignAllVar(usage);
                }
            }

            // using default operator =, assume everything initialized
            else {
                assignAllVar(usage);
            }
        } else if (Token::Match(ftok, "::| %name% (") && !Token::Match(ftok, "if|while|for")) {
            if (ftok->str() == "::")
                ftok = ftok->next();

            // Passing "this" => assume that everything is initialized
            for (const Token *tok2 = ftok->linkAt(1); tok2 && tok2 != ftok; tok2 = tok2->previous()) {
                if (tok2->str() == "this") {
                    assignAllVar(usage);
                    return;
                }
            }

            // check if member function
            if (ftok->function() && ftok->function()->nestedIn == scope &&
                !ftok->function()->isConstructor()) {
                const Function *member = ftok->function();

                // recursive call
                // assume that all variables are initialized
                if (std::find(callstack.cbegin(), callstack.cend(), member) != callstack.cend()) {
                    assignAllVar(usage);
                    return;
                }

                // member function has implementation
                if (member->hasBody()) {
                    // initialize variable use list using member function
                    callstack.push_back(member);
                    initializeVarList(*member, callstack, scope, usage);
                    callstack.pop_back();

                    // Assume that variables that are passed to it are initialized..
                    for (const Token *tok2 = ftok; tok2; tok2 = tok2->next()) {
                        if (Token::Match(tok2, "[;{}]"))
                            break;
                        if (Token::Match(tok2, "[(,] &| %name% [,)]")) {
                            tok2 = tok2->next();
                            if (tok2->str() == "&")
                                tok2 = tok2->next();
                            if (isVariableChangedByFunctionCall(tok2, tok2->strAt(-1) == "&", tok2->varId(), *mSettings, nullptr))
                                assignVar(usage, tok2->varId());
                        }
                    }
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything (if it can mutate state)
                else if (!member->isConst() && !member->isStatic()) {
                    assignAllVar(usage);
                }

                // const method, assume it assigns all mutable members
                else if (member->isConst()) {
                    for (Usage& i: usage) {
                        if (i.var->isMutable())
                            i.assign = true;
                    }
                }
            }

            // not member function
            else {
                // could be a base class virtual function, so we assume it initializes everything
                if (!func.isConstructor() && isBaseClassMutableMemberFunc(ftok, scope)) {
                    /** @todo False Negative: we should look at the base class functions to see if they
                     *  call any derived class virtual functions that change the derived class state
                     */
                    assignAllVar(usage);
                }

                // has friends, so we assume it initializes everything
                if (!scope->definedType->friendList.empty())
                    assignAllVar(usage);

                // the function is external and it's neither friend nor inherited virtual function.
                // assume all variables that are passed to it are initialized..
                else {
                    for (const Token *tok = ftok->tokAt(2); tok && tok != ftok->linkAt(1); tok = tok->next()) {
                        if (tok->isName()) {
                            assignVar(usage, tok->varId());
                        }
                    }
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%name% =")) {
            assignVar(usage, ftok);
            bool bailout = ftok->variable() && ftok->variable()->isReference();
            const Token* tok2 = ftok->tokAt(2);
            if (tok2->str() == "&") {
                tok2 = tok2->next();
                bailout = true;
            }
            if (tok2->variable() && (bailout || tok2->variable()->isArray()) && tok2->strAt(1) != "[")
                assignVar(usage, tok2->varId());
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%name% [|.")) {
            const Token *tok2 = ftok;
            while (tok2) {
                if (tok2->strAt(1) == "[")
                    tok2 = tok2->linkAt(1);
                else if (Token::Match(tok2->next(), ". %name%"))
                    tok2 = tok2->tokAt(2);
                else
                    break;
            }
            if (tok2 && tok2->strAt(1) == "=")
                assignVar(usage, ftok->varId());
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "* %name% =") && (!func.isConstructor() || (!ftok->next()->variable() || ftok->next()->variable()->isArray()))) {
            assignVar(usage, ftok->next()->varId());
        } else if (Token::Match(ftok, "( * %name% ) =") && (!func.isConstructor() || (!ftok->tokAt(2)->variable() || ftok->tokAt(2)->variable()->isArray()))) {
            assignVar(usage, ftok->tokAt(2)->varId());
        } else if (Token::Match(ftok, "* this . %name% =") && (!func.isConstructor() || (!ftok->tokAt(3)->variable() || ftok->tokAt(3)->variable()->isArray()))) {
            assignVar(usage, ftok->tokAt(3)->varId());
        } else if (astIsRangeBasedForDecl(ftok)) {
            if (const Variable* rangeVar = ftok->astParent()->astOperand1()->variable()) {
                if (rangeVar->isReference() && !rangeVar->isConst())
                    assignVar(usage, ftok->varId());
            }
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%name% . clear|Clear (")) {
            assignVar(usage, ftok->varId());
        }
    }
}

void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    // For performance reasons the constructor might be intentionally missing. Therefore this is not a "warning"
    const std::string message {"The " + std::string(isStruct ? "struct" : "class") + " '$symbol' does not declare a constructor although it has private member variables which likely require initialization."};
    const std::string verbose {message + " Member variables of native types, pointers, or references are left uninitialized when the class is instantiated. That may cause bugs or undefined behavior."};
    reportError(tok, Severity::style, "noConstructor", "$symbol:" + classname + '\n' + message + '\n' + verbose, CWE398, Certainty::normal);
}

void CheckClass::noExplicitConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    const std::string message(std::string(isStruct ? "Struct" : "Class") + " '$symbol' has a constructor with 1 argument that is not explicit.");
    const std::string verbose(message + " Such, so called \"Converting constructors\", should in general be explicit for type safety reasons as that prevents unintended implicit conversions.");
    reportError(tok, Severity::style, "noExplicitConstructor", "$symbol:" + classname + '\n' + message + '\n' + verbose, CWE398, Certainty::normal);
}

void CheckClass::uninitVarError(const Token *tok, bool isprivate, FunctionType functionType, const std::string &classname, const std::string &varname, bool derived, bool inconclusive, bool noCtor)
{
    std::string message("Member variable '$symbol' ");
    if (noCtor)
        message += "has no initializer.";
    else {
        message += "is not initialized in the ";
        if (functionType == FunctionType::eCopyConstructor)
            message += "copy ";
        else if (functionType == FunctionType::eMoveConstructor)
            message += "move ";
        message += "constructor.";
    }
    if (derived)
        message += " Maybe it should be initialized directly in the class " + classname + "?";
    std::string id = std::string("uninit") + (derived ? "Derived" : "") + "MemberVar" + (isprivate ? "Private" : "") + (noCtor ? "NoCtor" : "");
    const std::string verbose {message + " Member variables of native types, pointers, or references are left uninitialized when the class is instantiated. That may cause bugs or undefined behavior."};
    reportError(tok, Severity::warning, id, "$symbol:" + classname + "::" + varname + '\n' + message + '\n' + verbose, CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    const std::string message("Member variable '$symbol' is not initialized."); // report missing in-class initializer
    const std::string verbose {message + " Member variables of native types, pointers, or references are left uninitialized when the class is instantiated. That may cause bugs or undefined behavior."};
    const std::string id = std::string("uninitMemberVarPrivate");
    reportError(tok, Severity::warning, id, "$symbol:" + classname + "::" + varname + '\n' + message + '\n' + verbose, CWE398, Certainty::normal);
}

void CheckClass::missingMemberCopyError(const Token *tok, FunctionType functionType, const std::string& classname, const std::string& varname)
{
    const std::string ctor(functionType == FunctionType::eCopyConstructor ? "copy" : "move");
    const std::string action(functionType == FunctionType::eCopyConstructor ? "copied?" : "moved?");
    const std::string message =
        "$symbol:" + classname + "::" + varname + "\n" +
        "Member variable '$symbol' is not assigned in the " + ctor + " constructor. Should it be " + action;
    reportError(tok, Severity::warning, "missingMemberCopy", message, CWE398, Certainty::inconclusive);
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive)
{
    reportError(tok, Severity::warning, "operatorEqVarError", "$symbol:" + classname + "::" + varname + "\nMember variable '$symbol' is not assigned a value in '" + classname + "::operator='.", CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// ClassCheck: Use initialization list instead of assignment
//---------------------------------------------------------------------------

void CheckClass::initializationListUsage()
{
    if (!mSettings->severity.isEnabled(Severity::performance))
        return;

    logChecker("CheckClass::initializationListUsage"); // performance

    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        // Check every constructor
        if (!scope->function || !scope->function->isConstructor())
            continue;

        // Do not warn when a delegate constructor is called
        if (const Token *initList = scope->function->constructorMemberInitialization()) {
            if (Token::Match(initList, ": %name% {|(") && initList->strAt(1) == scope->className)
                continue;
        }

        const Scope* owner = scope->functionOf;
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name% (")) // Assignments might depend on this function call or if/for/while/switch statement from now on.
                break;
            if (Token::Match(tok, "try|do {"))
                break;
            if (!Token::Match(tok, "%var% =") || tok->strAt(-1) == "*" || tok->strAt(-1) == ".")
                continue;

            const Variable* var = tok->variable();
            if (!var || var->scope() != owner || var->isStatic())
                continue;
            if (var->isPointer() || var->isReference() || var->isEnumType())
                continue;
            if (!WRONG_DATA(!var->valueType(), tok) && var->valueType()->type > ValueType::Type::ITERATOR)
                continue;

            // bailout: multi line lambda in rhs => do not warn
            if (findLambdaEndToken(tok->tokAt(2)) && tok->tokAt(2)->findExpressionStartEndTokens().second->linenr() > tok->tokAt(2)->linenr())
                continue;

            // Access local var member in rhs => do not warn
            bool localmember = false;
            visitAstNodes(tok->next()->astOperand2(),
                          [&](const Token *rhs) {
                if (rhs->str() == "." && rhs->astOperand1() && rhs->astOperand1()->variable() && rhs->astOperand1()->variable()->isLocal())
                    localmember = true;
                return ChildrenToVisit::op1_and_op2;
            });
            if (localmember)
                continue;

            bool allowed = true;
            visitAstNodes(tok->next()->astOperand2(),
                          [&](const Token *tok2) {
                const Variable* var2 = tok2->variable();
                if (var2) {
                    if (var2->scope() == owner && tok2->strAt(-1)!=".") { // Is there a dependency between two member variables?
                        allowed = false;
                        return ChildrenToVisit::done;
                    }
                    if (var2->isArray() && var2->isLocal()) { // Can't initialize with a local array
                        allowed = false;
                        return ChildrenToVisit::done;
                    }
                    if (var2->isLocal() && isVariableChanged(var2->nameToken(), previousBeforeAstLeftmostLeaf(tok), var2->declarationId(), /*globalvar*/ false, *mSettings)) {
                        allowed = false;
                        return ChildrenToVisit::done;
                    }
                } else if (tok2->str() == "this") { // 'this' instance is not completely constructed in initialization list
                    allowed = false;
                    return ChildrenToVisit::done;
                } else if (Token::Match(tok2, "%name% (") && tok2->strAt(-1) != "." && isMemberFunc(owner, tok2)) { // Member function called?
                    allowed = false;
                    return ChildrenToVisit::done;
                }
                return ChildrenToVisit::op1_and_op2;
            });
            if (!allowed)
                continue;

            suggestInitializationList(tok, tok->str());
        }
    }
}

void CheckClass::suggestInitializationList(const Token* tok, const std::string& varname)
{
    reportError(tok, Severity::performance, "useInitializationList", "$symbol:" + varname + "\nVariable '$symbol' is assigned in constructor body. Consider performing initialization in initialization list.\n"
                "When an object of a class is created, the constructors of all member variables are called consecutively "
                "in the order the variables are declared, even if you don't explicitly write them to the initialization list. You "
                "could avoid assigning '$symbol' a value by passing the value to the constructor in the initialization list.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

static bool checkFunctionUsage(const Function *privfunc, const Scope* scope)
{
    if (!scope)
        return true; // Assume it is used, if scope is not seen

    for (auto func = scope->functionList.cbegin(); func != scope->functionList.cend(); ++func) {
        if (func->functionScope) {
            if (Token::Match(func->tokenDef, "%name% (")) {
                for (const Token *ftok = func->tokenDef->tokAt(2); ftok && ftok->str() != ")"; ftok = ftok->next()) {
                    if (Token::Match(ftok, "= %name% [(,)]") && ftok->strAt(1) == privfunc->name())
                        return true;
                    if (ftok->str() == "(")
                        ftok = ftok->link();
                }
            }
            for (const Token *ftok = func->functionScope->classDef->linkAt(1); ftok != func->functionScope->bodyEnd; ftok = ftok->next()) {
                if (ftok->function() == privfunc)
                    return true;
                if (ftok->varId() == 0U && ftok->str() == privfunc->name()) // TODO: This condition should be redundant
                    return true;
            }
        } else if ((func->type != FunctionType::eCopyConstructor &&
                    func->type != FunctionType::eOperatorEqual &&
                    !func->isDefault() && !func->isDelete()) ||
                   func->access != AccessControl::Private) // Assume it is used, if a function implementation isn't seen, but empty private copy constructors and assignment operators are OK
            return true;
    }

    for (auto iter = scope->definedTypesMap.cbegin(); iter != scope->definedTypesMap.cend(); ++iter) {
        const Type *type = iter->second;
        if (type->enclosingScope == scope && checkFunctionUsage(privfunc, type->classScope))
            return true;
    }

    for (const Variable &var : scope->varlist) {
        if (var.isStatic()) {
            const Token* tok = Token::findmatch(scope->bodyStart, "%varid% =|(|{", var.declarationId());
            if (tok)
                tok = tok->tokAt(2);
            while (tok && tok->str() != ";") {
                if (tok->function() == privfunc)
                    return true;
                tok = tok->next();
            }
        }
    }

    return false; // Unused in this scope
}

void CheckClass::privateFunctions()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("unusedPrivateFunction"))
        return;

    logChecker("CheckClass::privateFunctions"); // style

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        // do not check borland classes with properties..
        if (Token::findsimplematch(scope->bodyStart, "; __property ;", scope->bodyEnd))
            continue;

        std::list<const Function*> privateFuncs;
        for (const Function &func : scope->functionList) {
            // Get private functions..
            if (func.type == FunctionType::eFunction && func.access == AccessControl::Private && !func.isOperator()) // TODO: There are smarter ways to check private operator usage
                privateFuncs.push_back(&func);
        }

        // Bailout for overridden virtual functions of base classes
        if (!scope->definedType->derivedFrom.empty()) {
            // Check virtual functions
            for (auto it = privateFuncs.cbegin(); it != privateFuncs.cend();) {
                if ((*it)->isImplicitlyVirtual(true)) // Give true as default value to be returned if we don't see all base classes
                    it = privateFuncs.erase(it);
                else
                    ++it;
            }
        }

        while (!privateFuncs.empty()) {
            const auto& pf = privateFuncs.front();
            if (pf->token->isAttributeMaybeUnused() || pf->token->isAttributeUnused()) {
                privateFuncs.pop_front();
                continue;
            }
            if (pf->tokenDef && (pf->tokenDef->isAttributeMaybeUnused() || pf->tokenDef->isAttributeUnused())) {
                privateFuncs.pop_front();
                continue;
            }
            // Check that all private functions are used
            bool used = checkFunctionUsage(pf, scope); // Usage in this class
            // Check in friend classes
            const std::vector<Type::FriendInfo>& friendList = scope->definedType->friendList;
            for (std::size_t i = 0; i < friendList.size() && !used; i++) {
                if (friendList[i].type)
                    used = checkFunctionUsage(pf, friendList[i].type->classScope);
                else
                    used = true; // Assume, it is used if we do not see friend class
            }

            if (!used)
                unusedPrivateFunctionError(pf->token, pf->tokenDef, scope->className, pf->name());

            privateFuncs.pop_front();
        }
    }
}

void CheckClass::unusedPrivateFunctionError(const Token* tok1, const Token *tok2, const std::string &classname, const std::string &funcname)
{
    std::list<const Token *> toks{ tok1 };
    if (tok2)
        toks.push_front(tok2);
    reportError(toks, Severity::style, "unusedPrivateFunction", "$symbol:" + classname + "::" + funcname + "\nUnused private function: '$symbol'", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// ClassCheck: Check that memset is not used on classes
//---------------------------------------------------------------------------

static const Scope* findFunctionOf(const Scope* scope)
{
    while (scope) {
        if (scope->type == ScopeType::eFunction)
            return scope->functionOf;
        scope = scope->nestedIn;
    }
    return nullptr;
}

void CheckClass::checkMemset()
{
    logChecker("CheckClass::checkMemset");
    const bool printWarnings = mSettings->severity.isEnabled(Severity::warning);
    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|memcpy|memmove (")) {
                const Token* arg1 = tok->tokAt(2);
                const Token* arg3 = arg1->nextArgument();
                if (arg3)
                    arg3 = arg3->nextArgument();
                if (!arg3)
                    // weird, shouldn't happen: memset etc should have
                    // 3 arguments.
                    continue;

                const Token *typeTok = nullptr;
                const Scope *type = nullptr;
                const Token* sizeofTok = arg3->previous()->astOperand2(); // try to find sizeof() in argument expression
                if (sizeofTok && sizeofTok->astOperand1() && Token::simpleMatch(sizeofTok->astOperand1()->previous(), "sizeof ("))
                    sizeofTok = sizeofTok->astOperand1();
                else if (sizeofTok && sizeofTok->astOperand2() && Token::simpleMatch(sizeofTok->astOperand2()->previous(), "sizeof ("))
                    sizeofTok = sizeofTok->astOperand2();
                if (Token::simpleMatch(sizeofTok, "("))
                    sizeofTok = sizeofTok->previous();
                if (Token::Match(sizeofTok, "sizeof ( %type% )"))
                    typeTok = sizeofTok->tokAt(2);
                else if (Token::Match(sizeofTok, "sizeof ( %type% :: %type% )"))
                    typeTok = sizeofTok->tokAt(4);
                else if (Token::Match(sizeofTok, "sizeof ( struct %type% )"))
                    typeTok = sizeofTok->tokAt(3);
                else if (Token::simpleMatch(sizeofTok, "sizeof ( * this )") || Token::simpleMatch(arg1, "this ,")) {
                    type = findFunctionOf(sizeofTok->scope());
                } else if (Token::Match(arg1, "&|*|%var%")) {
                    int numIndirToVariableType = 0; // Offset to the actual type in terms of dereference/addressof
                    for (;; arg1 = arg1->next()) {
                        if (arg1->str() == "&")
                            ++numIndirToVariableType;
                        else if (arg1->str() == "*")
                            --numIndirToVariableType;
                        else
                            break;
                    }

                    const Variable * const var = arg1->variable();
                    if (var && arg1->strAt(1) == ",") {
                        if (var->isArrayOrPointer()) {
                            const Token *endTok = var->typeEndToken();
                            while (Token::simpleMatch(endTok, "*")) {
                                ++numIndirToVariableType;
                                endTok = endTok->previous();
                            }
                        }

                        if (var->isArray())
                            numIndirToVariableType += int(var->dimensions().size());

                        if (numIndirToVariableType == 1)
                            type = var->typeScope();

                        if (!type && !var->isPointer() && !Token::simpleMatch(var->typeStartToken(), "std :: array") &&
                            mSettings->library.detectContainerOrIterator(var->typeStartToken())) {
                            memsetError(tok, tok->str(), var->getTypeName(), {}, /*isContainer*/ true);
                        }
                    }
                }

                // No type defined => The tokens didn't match
                if (!typeTok && !type)
                    continue;

                if (typeTok && typeTok->str() == "(")
                    typeTok = typeTok->next();

                if (!type && typeTok->type())
                    type = typeTok->type()->classScope;

                if (type) {
                    checkMemsetType(scope, tok, type, false, {});
                }
            } else if (tok->variable() && tok->variable()->isPointer() && tok->variable()->typeScope() && Token::Match(tok, "%var% = %name% (")) {
                const Library::AllocFunc* alloc = mSettings->library.getAllocFuncInfo(tok->tokAt(2));
                if (!alloc)
                    alloc = mSettings->library.getReallocFuncInfo(tok->tokAt(2));
                if (!alloc || alloc->bufferSize == Library::AllocFunc::BufferSize::none)
                    continue;
                std::set<const Scope *> parsedTypes;
                checkMemsetType(scope, tok->tokAt(2), tok->variable()->typeScope(), true, std::move(parsedTypes));

                if (printWarnings && tok->variable()->typeScope()->numConstructors > 0)
                    mallocOnClassWarning(tok, tok->strAt(2), tok->variable()->typeScope()->classDef);
            }
        }
    }
}

void CheckClass::checkMemsetType(const Scope *start, const Token *tok, const Scope *type, bool allocation, std::set<const Scope *> parsedTypes)
{
    // If type has been checked there is no need to check it again
    if (parsedTypes.find(type) != parsedTypes.end())
        return;
    parsedTypes.insert(type);

    const bool printPortability = mSettings->severity.isEnabled(Severity::portability);

    // recursively check all parent classes
    for (const Type::BaseInfo & i : type->definedType->derivedFrom) {
        const Type* derivedFrom = i.type;
        if (derivedFrom && derivedFrom->classScope)
            checkMemsetType(start, tok, derivedFrom->classScope, allocation, parsedTypes);
    }

    // Warn if type is a class that contains any virtual functions
    for (const Function &func : type->functionList) {
        if (func.hasVirtualSpecifier()) {
            if (allocation)
                mallocOnClassError(tok, tok->str(), type->classDef, "virtual function");
            else
                memsetError(tok, tok->str(), "virtual function", type->classDef->str());
        }
    }

    // Warn if type is a class or struct that contains any std::* variables
    for (const Variable &var : type->varlist) {
        if (var.isReference() && !var.isStatic()) {
            memsetErrorReference(tok, tok->str(), type->classDef->str());
            continue;
        }
        // don't warn if variable static or const, pointer or array of pointers
        if (!var.isStatic() && !var.isConst() && !var.isPointer() && (!var.isArray() || var.typeEndToken()->str() != "*")) {
            const Token *tok1 = var.typeStartToken();
            const Scope *typeScope = var.typeScope();

            std::string typeName;
            if (Token::Match(tok1, "%type% ::")) {
                const Token *typeTok = tok1;
                while (Token::Match(typeTok, "%type% ::")) {
                    typeName += typeTok->str() + "::";
                    typeTok = typeTok->tokAt(2);
                }
                typeName += typeTok->str();
            }

            // check for std:: type
            if (var.isStlType() && typeName != "std::array" && !mSettings->library.podtype(typeName)) {
                if (allocation)
                    mallocOnClassError(tok, tok->str(), type->classDef, "'" + typeName + "'");
                else
                    memsetError(tok, tok->str(), "'" + typeName + "'", type->classDef->str());
            }

            // check for known type
            else if (typeScope && typeScope != type)
                checkMemsetType(start, tok, typeScope, allocation, parsedTypes);

            // check for float
            else if (printPortability && var.isFloatingType() && tok->str() == "memset")
                memsetErrorFloat(tok, type->classDef->str());
        }
    }
}

void CheckClass::mallocOnClassWarning(const Token* tok, const std::string &memfunc, const Token* classTok)
{
    std::list<const Token *> toks = { tok, classTok };
    reportError(toks, Severity::warning, "mallocOnClassWarning",
                "$symbol:" + memfunc +"\n"
                "Memory for class instance allocated with $symbol(), but class provides constructors.\n"
                "Memory for class instance allocated with $symbol(), but class provides constructors. This is unsafe, "
                "since no constructor is called and class members remain uninitialized. Consider using 'new' instead.", CWE762, Certainty::normal);
}

void CheckClass::mallocOnClassError(const Token* tok, const std::string &memfunc, const Token* classTok, const std::string &classname)
{
    std::list<const Token *> toks = { tok, classTok };
    reportError(toks, Severity::error, "mallocOnClassError",
                "$symbol:" + memfunc +"\n"
                "$symbol:" + classname +"\n"
                "Memory for class instance allocated with " + memfunc + "(), but class contains a " + classname + ".\n"
                "Memory for class instance allocated with " + memfunc + "(), but class a " + classname + ". This is unsafe, "
                "since no constructor is called and class members remain uninitialized. Consider using 'new' instead.", CWE665, Certainty::normal);
}

void CheckClass::memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type, bool isContainer)
{
    const std::string typeStr = isContainer ? std::string() : (type + " that contains a ");
    const std::string msg = "$symbol:" + memfunc + "\n"
                            "$symbol:" + classname + "\n"
                            "Using '" + memfunc + "' on " + typeStr + classname + ".\n"
                            "Using '" + memfunc + "' on " + typeStr + classname + " is unsafe, because constructor, destructor "
                            "and copy operator calls are omitted. These are necessary for this non-POD type to ensure that a valid object "
                            "is created.";
    reportError(tok, Severity::error, "memsetClass", msg, CWE762, Certainty::normal);
}

void CheckClass::memsetErrorReference(const Token *tok, const std::string &memfunc, const std::string &type)
{
    reportError(tok, Severity::error, "memsetClassReference",
                "$symbol:" + memfunc +"\n"
                "Using '" + memfunc + "' on " + type + " that contains a reference.", CWE665, Certainty::normal);
}

void CheckClass::memsetErrorFloat(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::portability, "memsetClassFloat", "Using memset() on " + type + " which contains a floating point number.\n"
                "Using memset() on " + type + " which contains a floating point number."
                " This is not portable because memset() sets each byte of a block of memory to a specific value and"
                " the actual representation of a floating-point value is implementation defined."
                " Note: In case of an IEEE754-1985 compatible implementation setting all bits to zero results in the value 0.0.", CWE758, Certainty::normal);
}


//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

void CheckClass::operatorEqRetRefThis()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("operatorEqRetRefThis"))
        return;

    logChecker("CheckClass::operatorEqRetRefThis"); // style

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        for (auto func = scope->functionList.cbegin(); func != scope->functionList.cend(); ++func) {
            if (func->type == FunctionType::eOperatorEqual && func->hasBody()) {
                // make sure return signature is correct
                if (func->retType == func->nestedIn->definedType && func->tokenDef->strAt(-1) == "&") {
                    checkReturnPtrThis(scope, &(*func), func->functionScope->bodyStart, func->functionScope->bodyEnd);
                }
            }
        }
    }
}

void CheckClass::checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last)
{
    std::set<const Function*> analyzedFunctions;
    checkReturnPtrThis(scope, func, tok, last, analyzedFunctions);
}

void CheckClass::checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last, std::set<const Function*>& analyzedFunctions)
{
    bool foundReturn = false;

    const Token* const startTok = tok;

    for (; tok && tok != last; tok = tok->next()) {
        // check for return of reference to this

        if (const Token* lScope = isLambdaCaptureList(tok)) // skip lambda
            tok = lScope->link();

        if (tok->str() != "return")
            continue;

        foundReturn = true;

        const Token *retExpr = tok->astOperand1();
        if (retExpr && retExpr->str() == "=")
            retExpr = retExpr->astOperand1();
        if (retExpr && retExpr->isUnaryOp("*") && Token::simpleMatch(retExpr->astOperand1(), "this"))
            continue;

        std::string cast("( " + scope->className + " & )");
        if (Token::simpleMatch(tok->next(), cast.c_str(), cast.size()))
            tok = tok->tokAt(4);

        // check if a function is called
        if (tok->strAt(2) == "(" &&
            tok->linkAt(2)->strAt(1) == ";") {
            // check if it is a member function
            for (auto it = scope->functionList.cbegin(); it != scope->functionList.cend(); ++it) {
                // check for a regular function with the same name and a body
                if (it->type == FunctionType::eFunction && it->hasBody() &&
                    it->token->str() == tok->strAt(1)) {
                    // check for the proper return type
                    if (it->tokenDef->strAt(-1) == "&" &&
                        it->tokenDef->strAt(-2) == scope->className) {
                        // make sure it's not a const function
                        if (!it->isConst()) {
                            /** @todo make sure argument types match */
                            // avoid endless recursions
                            if (analyzedFunctions.find(&*it) == analyzedFunctions.end()) {
                                analyzedFunctions.insert(&*it);
                                checkReturnPtrThis(scope, &*it, it->arg->link()->next(), it->arg->link()->linkAt(1),
                                                   analyzedFunctions);
                            }
                            // just bail for now
                            else
                                return;
                        }
                    }
                }
            }
        }

        // check if *this is returned
        else if (!(Token::simpleMatch(tok->next(), "operator= (") ||
                   Token::simpleMatch(tok->next(), "this . operator= (") ||
                   (Token::Match(tok->next(), "%type% :: operator= (") &&
                    tok->strAt(1) == scope->className)))
            operatorEqRetRefThisError(func->token);
    }
    if (foundReturn) {
        return;
    }
    if (startTok->next() == last) {
        const std::string tmp("( const " + scope->className + " &");
        if (Token::simpleMatch(func->argDef, tmp.c_str(), tmp.size())) {
            // Typical wrong way to suppress default assignment operator by declaring it and leaving empty
            operatorEqMissingReturnStatementError(func->token, func->access == AccessControl::Public);
        } else {
            operatorEqMissingReturnStatementError(func->token, true);
        }
        return;
    }
    if (mSettings->library.isScopeNoReturn(last, nullptr)) {
        // Typical wrong way to prohibit default assignment operator
        // by always throwing an exception or calling a noreturn function
        operatorEqShouldBeLeftUnimplementedError(func->token);
        return;
    }

    operatorEqMissingReturnStatementError(func->token, func->access == AccessControl::Public);
}

void CheckClass::operatorEqRetRefThisError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqRetRefThis", "'operator=' should return reference to 'this' instance.", CWE398, Certainty::normal);
}

void CheckClass::operatorEqShouldBeLeftUnimplementedError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqShouldBeLeftUnimplemented", "'operator=' should either return reference to 'this' instance or be declared private and left unimplemented.", CWE398, Certainty::normal);
}

void CheckClass::operatorEqMissingReturnStatementError(const Token *tok, bool error)
{
    if (error) {
        reportError(tok, Severity::error, "operatorEqMissingReturnStatement", "No 'return' statement in non-void function causes undefined behavior.", CWE398, Certainty::normal);
    } else {
        operatorEqRetRefThisError(tok);
    }
}

//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C& rhs) { if (this == &rhs) ... }"
// operator= should check for assignment to self
//
// For simple classes, an assignment to self check is only a potential optimization.
//
// For classes that allocate dynamic memory, assignment to self can be a real error
// if it is deallocated and allocated again without being checked for.
//
// This check is not valid for classes with multiple inheritance because a
// class can have multiple addresses so there is no trivial way to check for
// assignment to self.
//---------------------------------------------------------------------------

void CheckClass::operatorEqToSelf()
{
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("operatorEqToSelf"))
        return;

    logChecker("CheckClass::operatorEqToSelf"); // warning

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        // skip classes with multiple inheritance
        if (scope->definedType->derivedFrom.size() > 1)
            continue;

        for (const Function &func : scope->functionList) {
            if (func.type == FunctionType::eOperatorEqual && func.hasBody()) {
                // make sure that the operator takes an object of the same type as *this, otherwise we can't detect self-assignment checks
                if (func.argumentList.empty())
                    continue;
                const Token* typeTok = func.argumentList.front().typeEndToken();
                while (typeTok->str() == "const" || typeTok->str() == "&" || typeTok->str() == "*")
                    typeTok = typeTok->previous();
                if (typeTok->str() != scope->className)
                    continue;

                // make sure return signature is correct
                if (Token::Match(func.retDef, "%type% &") && func.retDef->str() == scope->className) {
                    // find the parameter name
                    const Token *rhs = func.argumentList.cbegin()->nameToken();
                    const Token* out_ifStatementScopeStart = nullptr;
                    if (!hasAssignSelf(&func, rhs, out_ifStatementScopeStart)) {
                        if (hasAllocation(&func, scope))
                            operatorEqToSelfError(func.token);
                    } else if (out_ifStatementScopeStart != nullptr) {
                        if (hasAllocationInIfScope(&func, scope, out_ifStatementScopeStart))
                            operatorEqToSelfError(func.token);
                    }
                }
            }
        }
    }
}

bool CheckClass::hasAllocationInIfScope(const Function *func, const Scope* scope, const Token *ifStatementScopeStart) const
{
    const Token *end;
    if (ifStatementScopeStart->str() == "{")
        end = ifStatementScopeStart->link();
    else
        end = func->functionScope->bodyEnd;
    return hasAllocation(func, scope, ifStatementScopeStart, end);
}

bool CheckClass::hasAllocation(const Function *func, const Scope* scope) const
{
    return hasAllocation(func, scope, func->functionScope->bodyStart, func->functionScope->bodyEnd);
}

bool CheckClass::hasAllocation(const Function *func, const Scope* scope, const Token *start, const Token *end) const
{
    if (!end)
        end = func->functionScope->bodyEnd;
    for (const Token *tok = start; tok && (tok != end); tok = tok->next()) {
        if (((tok->isCpp() && Token::Match(tok, "%var% = new")) ||
             (Token::Match(tok, "%var% = %name% (") && mSettings->library.getAllocFuncInfo(tok->tokAt(2)))) &&
            isMemberVar(scope, tok))
            return true;

        // check for deallocating memory
        const Token *var;
        if (Token::Match(tok, "%name% ( %var%") && mSettings->library.getDeallocFuncInfo(tok))
            var = tok->tokAt(2);
        else if (tok->isCpp() && Token::Match(tok, "delete [ ] %var%"))
            var = tok->tokAt(3);
        else if (tok->isCpp() && Token::Match(tok, "delete %var%"))
            var = tok->next();
        else
            continue;
        // Check for assignment to the deleted pointer (only if its a member of the class)
        if (isMemberVar(scope, var)) {
            for (const Token *tok1 = var->next(); tok1 && (tok1 != end); tok1 = tok1->next()) {
                if (Token::Match(tok1, "%varid% =", var->varId()))
                    return true;
            }
        }
    }

    return false;
}

static bool isTrueKeyword(const Token* tok)
{
    return tok->hasKnownIntValue() && tok->getKnownIntValue() == 1;
}

static bool isFalseKeyword(const Token* tok)
{
    return tok->hasKnownIntValue() && tok->getKnownIntValue() == 0;
}

/*
 * Checks if self-assignment test is inverse
 * For example 'if (this == &rhs)'
 */
CheckClass::Bool CheckClass::isInverted(const Token *tok, const Token *rhs)
{
    bool res = true;
    for (const Token *itr = tok; itr && itr->str()!="("; itr=itr->astParent()) {
        if (Token::simpleMatch(itr, "!=") && (isTrueKeyword(itr->astOperand1()) || isTrueKeyword(itr->astOperand2()))) {
            res = !res;
        } else if (Token::simpleMatch(itr, "!=") && ((Token::simpleMatch(itr->astOperand1(), "this") && Token::simpleMatch(itr->astOperand2(), "&") && Token::simpleMatch(itr->astOperand2()->next(), rhs->str().c_str(), rhs->str().size()))
                                                     || (Token::simpleMatch(itr->astOperand2(), "this") && Token::simpleMatch(itr->astOperand1(), "&") && Token::simpleMatch(itr->astOperand1()->next(), rhs->str().c_str(), rhs->str().size())))) {
            res = !res;
        } else if (Token::simpleMatch(itr, "!=") && (isFalseKeyword(itr->astOperand1()) || isFalseKeyword(itr->astOperand2()))) {
            //Do nothing
        } else if (Token::simpleMatch(itr, "!")) {
            res = !res;
        } else if (Token::simpleMatch(itr, "==") && (isFalseKeyword(itr->astOperand1()) || isFalseKeyword(itr->astOperand2()))) {
            res = !res;
        } else if (Token::simpleMatch(itr, "==") && (isTrueKeyword(itr->astOperand1()) || isTrueKeyword(itr->astOperand2()))) {
            //Do nothing
        } else if (Token::simpleMatch(itr, "==") && ((Token::simpleMatch(itr->astOperand1(), "this") && Token::simpleMatch(itr->astOperand2(), "&") && Token::simpleMatch(itr->astOperand2()->next(), rhs->str().c_str(), rhs->str().size()))
                                                     || (Token::simpleMatch(itr->astOperand2(), "this") && Token::simpleMatch(itr->astOperand1(), "&") && Token::simpleMatch(itr->astOperand1()->next(), rhs->str().c_str(), rhs->str().size())))) {
            //Do nothing
        } else {
            return Bool::Bailout;
        }
    }
    if (res)
        return Bool::True;
    return Bool::False;
}

const Token * CheckClass::getIfStmtBodyStart(const Token *tok, const Token *rhs)
{
    const Token *top = tok->astTop();
    if (Token::simpleMatch(top->link(), ") {")) {
        switch (isInverted(tok->astParent(), rhs)) {
        case Bool::Bailout:
            return nullptr;
        case Bool::True:
            return top->link()->next();
        case Bool::False:
            return top->link()->linkAt(1);
        }
    }
    return nullptr;
}

bool CheckClass::hasAssignSelf(const Function *func, const Token *rhs, const Token *&out_ifStatementScopeStart)
{
    if (!rhs)
        return false;
    const Token *last = func->functionScope->bodyEnd;
    for (const Token *tok = func->functionScope->bodyStart; tok && tok != last; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "if ("))
            continue;

        bool ret = false;
        visitAstNodes(tok->next()->astOperand2(),
                      [&](const Token *tok2) {
            if (!Token::Match(tok2, "==|!="))
                return ChildrenToVisit::op1_and_op2;
            if (Token::simpleMatch(tok2->astOperand1(), "this"))
                tok2 = tok2->astOperand2();
            else if (Token::simpleMatch(tok2->astOperand2(), "this"))
                tok2 = tok2->astOperand1();
            else
                return ChildrenToVisit::op1_and_op2;
            if (tok2 && tok2->isUnaryOp("&") && tok2->astOperand1()->str() == rhs->str())
                ret = true;
            if (ret) {
                out_ifStatementScopeStart = getIfStmtBodyStart(tok2, rhs);
            }
            return ret ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
        });
        if (ret)
            return ret;
    }

    return false;
}

void CheckClass::operatorEqToSelfError(const Token *tok)
{
    reportError(tok, Severity::warning, "operatorEqToSelf",
                "'operator=' should check for assignment to self to avoid problems with dynamic memory.\n"
                "'operator=' should check for assignment to self to ensure that each block of dynamically "
                "allocated memory is owned and managed by only one instance of the class.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// A destructor in a base class should be virtual
//---------------------------------------------------------------------------

void CheckClass::virtualDestructor()
{
    // This error should only be given if:
    // * base class doesn't have virtual destructor
    // * derived class has non-empty destructor (only c++03, in c++11 it's UB see paragraph 3 in [expr.delete])
    // * base class is deleted
    // unless inconclusive in which case:
    // * A class with any virtual functions should have a destructor that is either public and virtual or protected
    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);

    std::list<const Function *> inconclusiveErrors;

    logChecker("CheckClass::virtualDestructor");

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        // Skip base classes (unless inconclusive)
        if (scope->definedType->derivedFrom.empty()) {
            if (printInconclusive) {
                const Function *destructor = scope->getDestructor();
                if (destructor && !destructor->hasVirtualSpecifier() && destructor->access == AccessControl::Public) {
                    if (std::any_of(scope->functionList.cbegin(), scope->functionList.cend(), [](const Function& func) {
                        return func.hasVirtualSpecifier();
                    }))
                        inconclusiveErrors.push_back(destructor);
                }
            }
            continue;
        }

        // Check if destructor is empty and non-empty ..
        if (mSettings->standards.cpp <= Standards::CPP03) {
            // Find the destructor
            const Function *destructor = scope->getDestructor();

            // Check for destructor with implementation
            if (!destructor || !destructor->hasBody())
                continue;

            // Empty destructor
            if (destructor->token->linkAt(3) == destructor->token->tokAt(4))
                continue;
        }

        const Token *derived = scope->classDef;
        const Token *derivedClass = derived->next();

        // Iterate through each base class...
        for (const Type::BaseInfo & j : scope->definedType->derivedFrom) {
            // Check if base class is public and exists in database
            if (j.access != AccessControl::Private && j.type) {
                const Type *derivedFrom = j.type;
                const Scope *derivedFromScope = derivedFrom->classScope;
                if (!derivedFromScope)
                    continue;

                // Check for this pattern:
                // 1. Base class pointer is given the address of derived class instance
                // 2. Base class pointer is deleted
                //
                // If this pattern is not seen then bailout the checking of these base/derived classes
                {
                    // pointer variables of type 'Base *'
                    std::set<int> baseClassPointers;

                    for (const Variable* var : mSymbolDatabase->variableList()) {
                        if (var && var->isPointer() && var->type() == derivedFrom)
                            baseClassPointers.insert(var->declarationId());
                    }

                    // pointer variables of type 'Base *' that should not be deleted
                    std::set<int> dontDelete;

                    // No deletion of derived class instance through base class pointer found => the code is ok
                    bool ok = true;

                    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
                        if (Token::Match(tok, "[;{}] %var% =") &&
                            baseClassPointers.find(tok->next()->varId()) != baseClassPointers.end()) {
                            // new derived class..
                            const std::string tmp("new " + derivedClass->str());
                            if (Token::simpleMatch(tok->tokAt(3), tmp.c_str(), tmp.size())) {
                                dontDelete.insert(tok->next()->varId());
                            }
                        }

                        // Delete base class pointer that might point at derived class
                        else if (Token::Match(tok, "delete %var% ;") &&
                                 dontDelete.find(tok->next()->varId()) != dontDelete.end()) {
                            ok = false;
                            break;
                        }
                    }

                    // No base class pointer that points at a derived class is deleted
                    if (ok)
                        continue;
                }

                // Find the destructor declaration for the base class.
                const Function *baseDestructor = derivedFromScope->getDestructor();

                // Check that there is a destructor..
                if (!baseDestructor) {
                    if (derivedFrom->derivedFrom.empty()) {
                        virtualDestructorError(derivedFrom->classDef, derivedFrom->name(), derivedClass->str(), false);
                    }
                } else if (!baseDestructor->hasVirtualSpecifier()) {
                    // TODO: This is just a temporary fix, better solution is needed.
                    // Skip situations where base class has base classes of its own, because
                    // some of the base classes might have virtual destructor.
                    // Proper solution is to check all of the base classes. If base class is not
                    // found or if one of the base classes has virtual destructor, error should not
                    // be printed. See TODO test case "virtualDestructorInherited"
                    if (derivedFrom->derivedFrom.empty()) {
                        // Make sure that the destructor is public (protected or private
                        // would not compile if inheritance is used in a way that would
                        // cause the bug we are trying to find here.)
                        if (baseDestructor->access == AccessControl::Public) {
                            virtualDestructorError(baseDestructor->token, derivedFrom->name(), derivedClass->str(), false);
                            // check for duplicate error and remove it if found
                            const auto found = std::find(inconclusiveErrors.cbegin(), inconclusiveErrors.cend(), baseDestructor);
                            if (found != inconclusiveErrors.cend())
                                inconclusiveErrors.erase(found);
                        }
                    }
                }
            }
        }
    }

    for (const Function *func : inconclusiveErrors)
        virtualDestructorError(func->tokenDef, func->name(), "", true);
}

void CheckClass::virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived, bool inconclusive)
{
    if (inconclusive) {
        if (mSettings->severity.isEnabled(Severity::warning))
            reportError(tok, Severity::warning, "virtualDestructor", "$symbol:" + Base + "\nClass '$symbol' which has virtual members does not have a virtual destructor.", CWE404, Certainty::inconclusive);
    } else {
        reportError(tok, Severity::error, "virtualDestructor",
                    "$symbol:" + Base +"\n"
                    "$symbol:" + Derived +"\n"
                    "Class '" + Base + "' which is inherited by class '" + Derived + "' does not have a virtual destructor.\n"
                    "Class '" + Base + "' which is inherited by class '" + Derived + "' does not have a virtual destructor. "
                    "If you destroy instances of the derived class by deleting a pointer that points to the base class, only "
                    "the destructor of the base class is executed. Thus, dynamic memory that is managed by the derived class "
                    "could leak. This can be avoided by adding a virtual destructor to the base class.", CWE404, Certainty::normal);
    }
}

//---------------------------------------------------------------------------
// warn for "this-x". The indented code may be "this->x"
//---------------------------------------------------------------------------

void CheckClass::thisSubtraction()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckClass::thisSubtraction"); // warning

    const Token *tok = mTokenizer->tokens();
    for (;;) {
        tok = Token::findmatch(tok, "this - %name%");
        if (!tok)
            break;

        if (tok->strAt(-1) != "*")
            thisSubtractionError(tok);

        tok = tok->next();
    }
}

void CheckClass::thisSubtractionError(const Token *tok)
{
    reportError(tok, Severity::warning, "thisSubtraction", "Suspicious pointer subtraction. Did you intend to write '->'?", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// can member function be const?
//---------------------------------------------------------------------------

void CheckClass::checkConst()
{
    if (!mSettings->severity.isEnabled(Severity::style) &&
        !mSettings->isPremiumEnabled("functionConst") &&
        !mSettings->isPremiumEnabled("functionStatic"))
        return;

    logChecker("CheckClass::checkConst"); // style,inconclusive

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {
        for (const Function &func : scope->functionList) {
            // does the function have a body?
            if (func.type != FunctionType::eFunction || !func.hasBody())
                continue;
            // don't warn for friend/static/virtual functions
            if (func.isFriend() || func.isStatic() || func.hasVirtualSpecifier())
                continue;
            if (func.functionPointerUsage)
                continue;
            if (func.hasRvalRefQualifier())
                continue;

            // don't suggest const when returning non-const pointer/reference, but still suggest static
            auto isPointerOrReference = [this](const Token* start, const Token* end) -> bool {
                bool inTemplArgList = false, isConstTemplArg = false;
                for (const Token* tok = start; tok != end; tok = tok->next()) {
                    if (tok->str() == "{") // end of trailing return type
                        return false;
                    if (tok->str() == "<") {
                        if (!tok->link())
                            mSymbolDatabase->debugMessage(tok, "debug", "CheckClass::checkConst found unlinked template argument list '" + tok->expressionString() + "'.");
                        inTemplArgList = true;
                    }
                    else if (tok->str() == ">") {
                        inTemplArgList = false;
                        isConstTemplArg = false;
                    }
                    else if (tok->str() == "const") {
                        if (!inTemplArgList)
                            return false;
                        isConstTemplArg = true;
                    }
                    else if (!isConstTemplArg && Token::Match(tok, "*|&"))
                        return true;
                }
                return false;
            };

            const bool returnsPtrOrRef = isPointerOrReference(func.retDef, func.tokenDef);

            if (Function::returnsPointer(&func, /*unknown*/ true) || Function::returnsReference(&func, /*unknown*/ true, /*includeRValueRef*/ true)) { // returns const/non-const depending on template arg
                bool isTemplateArg = false;
                for (const Token* tok2 = func.retDef; precedes(tok2, func.token); tok2 = tok2->next())
                    if (tok2->isTemplateArg() && tok2->str() == "const") {
                        isTemplateArg = true;
                        break;
                    }
                if (isTemplateArg)
                    continue;
            }

            if (func.isOperator()) { // Operator without return type: conversion operator
                const std::string& opName = func.tokenDef->str();
                if (opName.compare(8, 5, "const") != 0 && (endsWith(opName,'&') || endsWith(opName,'*')))
                    continue;
            } else if (mSettings->library.isSmartPointer(func.retDef)) {
                // Don't warn if a std::shared_ptr etc is returned
                continue;
            } else {
                // don't warn for unknown types..
                // LPVOID, HDC, etc
                if (func.retDef->str().size() > 2 && !func.retDef->type() && func.retDef->isUpperCaseName())
                    continue;
            }

            // check if base class function is virtual
            bool foundAllBaseClasses = true;
            if (!scope->definedType->derivedFrom.empty() && func.isImplicitlyVirtual(true, &foundAllBaseClasses) && foundAllBaseClasses)
                continue;

            MemberAccess memberAccessed = MemberAccess::NONE;
            // if nothing non-const was found. write error..
            if (!checkConstFunc(scope, &func, memberAccessed))
                continue;

            const bool suggestStatic = memberAccessed != MemberAccess::MEMBER && !func.isOperator();
            if ((returnsPtrOrRef || func.isConst() || func.hasLvalRefQualifier()) && !suggestStatic)
                continue;
            if (!suggestStatic && !mSettings->certainty.isEnabled(Certainty::inconclusive))
                // functionConst is inconclusive. False positives: #3322.
                continue;
            if (suggestStatic && func.isConst()) {
                const auto overloads = func.getOverloadedFunctions();
                if (overloads.size() > 1 && std::any_of(overloads.begin(), overloads.end(), [&](const Function* ovl) {
                    if (&func == ovl)
                        return false;
                    if (!ovl->functionScope)
                        return true;
                    return func.argCount() == ovl->argCount() && func.argsMatch(ovl->functionScope, ovl->argDef, func.argDef, "", 0);
                }))
                    continue;
            }

            std::string classname = scope->className;
            const Scope *nest = scope->nestedIn;
            while (nest && nest->type != ScopeType::eGlobal) {
                classname = nest->className + "::" + classname;
                nest = nest->nestedIn;
            }

            // get function name
            std::string functionName = (func.tokenDef->isName() ? "" : "operator") + func.tokenDef->str();

            if (func.tokenDef->str() == "(")
                functionName += ")";
            else if (func.tokenDef->str() == "[")
                functionName += "]";

            if (func.isInline())
                checkConstError(func.token, classname, functionName, suggestStatic, foundAllBaseClasses);
            else // not inline
                checkConstError2(func.token, func.tokenDef, classname, functionName, suggestStatic, foundAllBaseClasses);
        }
    }
}

// tok should point at "this"
static const Token* getFuncTokFromThis(const Token* tok) {
    if (!Token::simpleMatch(tok->next(), "."))
        return nullptr;
    tok = tok->tokAt(2);
    while (Token::Match(tok, "%name% ::"))
        tok = tok->tokAt(2);
    return Token::Match(tok, "%name% (") ? tok : nullptr;
}

bool CheckClass::isMemberVar(const Scope *scope, const Token *tok) const
{
    bool again = false;

    // try to find the member variable
    do {
        again = false;

        if (tok->str() == "this")
            return !getFuncTokFromThis(tok); // function calls are handled elsewhere
        if (Token::simpleMatch(tok->tokAt(-3), "( * this )"))
            return true;
        if (Token::Match(tok->tokAt(-3), "%name% ) . %name%")) {
            tok = tok->tokAt(-3);
            again = true;
        } else if (Token::Match(tok->tokAt(-2), "%name% . %name%")) {
            tok = tok->tokAt(-2);
            again = true;
        } else if (Token::Match(tok->tokAt(-2), "] . %name%")) {
            tok = tok->linkAt(-2)->previous();
            again = true;
        } else if (tok->str() == "]") {
            tok = tok->link()->previous();
            again = true;
        } else if (Token::Match(tok, "%name% ::")) {
            tok = tok->tokAt(2);
            again = true;
        }
    } while (again);

    if (tok->isKeyword() || tok->isStandardType())
        return false;
    if (tok->variable() && (tok->variable()->isArgument() || tok->variable()->isLocal()))
        return false;
    if (tok->function() || tok->type() || tok->enumerator())
        return false;

    for (const Variable& var : scope->varlist) {
        if (var.name() == tok->str()) {
            if (Token::Match(tok, "%name% ::"))
                continue;
            const Token* fqTok = tok;
            while (Token::Match(fqTok->tokAt(-2), "%name% ::"))
                fqTok = fqTok->tokAt(-2);
            if (fqTok->strAt(-1) == "::")
                fqTok = fqTok->previous();
            bool isMember = tok == fqTok;
            std::string scopeStr;
            const Scope* curScope = scope;
            while (!isMember && curScope && curScope->type != ScopeType::eGlobal) {
                scopeStr.insert(0, curScope->className + " :: ");
                isMember = Token::Match(fqTok, scopeStr.c_str());

                curScope = curScope->nestedIn;
            }
            if (isMember) {
                if (tok->varId() == 0)
                    mSymbolDatabase->debugMessage(tok, "varid0", "CheckClass::isMemberVar found used member variable \'" + tok->str() + "\' with varid 0");

                return !var.isStatic();
            }
        }
    }

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        bool foundAllBaseClasses = true;
        for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
            // find the base class
            const Type *derivedFrom = i.type;
            if (!derivedFrom || !derivedFrom->classScope)
                foundAllBaseClasses = false;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope && derivedFrom->classScope != scope) {
                if (isMemberVar(derivedFrom->classScope, tok))
                    return true;
            }
        }
        if (!foundAllBaseClasses)
            return true;
    }

    return false;
}

bool CheckClass::isMemberFunc(const Scope *scope, const Token *tok)
{
    if (!tok->function()) {
        if (Token::simpleMatch(tok->astParent(), ".") && Token::simpleMatch(tok->astParent()->astOperand1(), "this"))
            return true;
        for (const Function &func : scope->functionList) {
            if (func.name() == tok->str()) {
                const Token* tok2 = tok->tokAt(2);
                int argsPassed = tok2->str() == ")" ? 0 : 1;
                for (;;) {
                    tok2 = tok2->nextArgument();
                    if (tok2)
                        argsPassed++;
                    else
                        break;
                }
                if (argsPassed == func.argCount() ||
                    (func.isVariadic() && argsPassed >= (func.argCount() - 1)) ||
                    (argsPassed < func.argCount() && argsPassed >= func.minArgCount()))
                    return true;
            }
        }
    } else if (tok->function()->nestedIn == scope)
        return !tok->function()->isStatic();

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
            // find the base class
            const Type *derivedFrom = i.type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope && derivedFrom->classScope != scope) {
                if (isMemberFunc(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::isConstMemberFunc(const Scope *scope, const Token *tok)
{
    if (!tok->function())
        return false;
    if (tok->function()->nestedIn == scope)
        return tok->function()->isConst();

    // not found in this class
    if (!scope->definedType->derivedFrom.empty()) {
        // check each base class
        for (const Type::BaseInfo & i : scope->definedType->derivedFrom) {
            // find the base class
            const Type *derivedFrom = i.type;

            // find the function in the base class
            if (derivedFrom && derivedFrom->classScope) {
                if (isConstMemberFunc(derivedFrom->classScope, tok))
                    return true;
            }
        }
    }

    return false;
}

const std::set<std::string> CheckClass::stl_containers_not_const = { "map", "unordered_map", "std :: map|unordered_map <" }; // start pattern

static bool isNonConstPtrCast(const Token* tok)
{
    if (!tok || !tok->isCast())
        return false;
    const ValueType* vt = tok->valueType();
    return !vt || (vt->pointer > 0 && !vt->isConst(vt->pointer));
}

bool CheckClass::checkConstFunc(const Scope *scope, const Function *func, MemberAccess& memberAccessed) const
{
    if (mTokenizer->hasIfdef(func->functionScope->bodyStart, func->functionScope->bodyEnd))
        return false;

    auto getFuncTok = [](const Token* tok) -> const Token* {
        if (Token::simpleMatch(tok, "this"))
            tok = getFuncTokFromThis(tok);
        bool isReturn = false;
        if ((Token::Match(tok, "%name% (|{") || (isReturn = Token::simpleMatch(tok->astParent(), "return {"))) && !tok->isStandardType() && !tok->isKeyword()) {
            if (isReturn)
                tok = tok->astParent();
            return tok;
        }
        return nullptr;
    };

    auto checkFuncCall = [this, &memberAccessed](const Token* funcTok, const Scope* scope, const Function* func) {
        if (isMemberFunc(scope, funcTok) && (funcTok->strAt(-1) != "." || Token::simpleMatch(funcTok->tokAt(-2), "this ."))) {
            const bool isSelf = func == funcTok->function();
            if (!isConstMemberFunc(scope, funcTok) && !isSelf)
                return false;
            memberAccessed = (isSelf && memberAccessed != MemberAccess::MEMBER) ? MemberAccess::SELF : MemberAccess::MEMBER;
        }

        if (const Function* f = funcTok->function()) { // check known function
            const std::vector<const Token*> args = getArguments(funcTok);
            const auto argMax = std::min<nonneg int>(args.size(), f->argCount());

            for (nonneg int argIndex = 0; argIndex < argMax; ++argIndex) {
                const Variable* const argVar = f->getArgumentVar(argIndex);
                if (!argVar || ((argVar->isArrayOrPointer() || argVar->isReference()) &&
                                !(argVar->valueType() && argVar->valueType()->isConst(argVar->valueType()->pointer)))) { // argument might be modified
                    const Token* arg = args[argIndex];
                    // Member variable given as parameter
                    while (Token::simpleMatch(arg, "(") && arg->astOperand2())
                        arg = arg->astOperand2();
                    const Token* varTok = previousBeforeAstLeftmostLeaf(arg);
                    if (!varTok)
                        return false;
                    varTok = varTok->next();
                    if ((varTok->isName() && isMemberVar(scope, varTok)) || (varTok->isUnaryOp("&") && (varTok = varTok->astOperand1()) && isMemberVar(scope, varTok))) {
                        const Variable* var = varTok->variable();
                        if (!var || (!var->isMutable() && !var->isConst()))
                            return false;
                    }
                }
            }
            return true;
        }

        if (const Library::Function* fLib = mSettings->library.getFunction(funcTok))
            if (fLib->isconst || fLib->ispure)
                return true;

        // Member variable given as parameter to unknown function
        const Token *lpar = funcTok->next();
        if (Token::simpleMatch(lpar, "( ) ("))
            lpar = lpar->tokAt(2);
        for (const Token* tok = lpar->next(); tok && tok != funcTok->linkAt(1); tok = tok->next()) {
            if (tok->str() == "(")
                tok = tok->link();
            else if ((tok->isName() && isMemberVar(scope, tok)) || (tok->isUnaryOp("&") && (tok = tok->astOperand1()) && isMemberVar(scope, tok))) {
                const Variable* var = tok->variable();
                if (!var || (!var->isMutable() && !var->isConst()))
                    return false;
            }
        }
        return true;
    };

    // if the function doesn't have any assignment nor function call,
    // it can be a const function..
    for (const Token *tok1 = func->functionScope->bodyStart; tok1 && tok1 != func->functionScope->bodyEnd; tok1 = tok1->next()) {
        if (tok1->isName() && isMemberVar(scope, tok1)) {
            memberAccessed = MemberAccess::MEMBER;
            const Variable* v = tok1->variable();
            if (v && v->isMutable())
                continue;

            if (tok1->str() == "this") {
                if (tok1->previous()->isAssignmentOp())
                    return false;
                if (Token::Match(tok1->previous(), "( this . * %var% )")) // call using ptr to member function TODO: check constness
                    return false;
                if (Token::simpleMatch(tok1->astParent(), "*") && tok1->astParent()->astParent() && tok1->astParent()->astParent()->isIncDecOp())
                    return false;
            }

            // non const pointer cast
            if (tok1->valueType() && tok1->valueType()->pointer > 0 && isNonConstPtrCast(tok1->astParent()))
                return false;

            const Token* lhs = tok1->previous();
            if (lhs->str() == "(" && tok1->astParent() && tok1->astParent()->astParent())
                lhs = tok1->astParent()->astParent();
            else if (lhs->str() == "?" && lhs->astParent())
                lhs = lhs->astParent();
            else if (lhs->str() == ":" && lhs->astParent() && lhs->astParent()->astParent() && lhs->astParent()->str() == "?")
                lhs = lhs->astParent()->astParent();
            if (lhs->str() == "&") {
                const Token* parent = lhs->astParent();
                while (Token::Match(parent, "[+(]")) {
                    if (isNonConstPtrCast(parent))
                        return false;
                    parent = parent->astParent();
                }
                const Token* const top = lhs->astTop();
                if (top->isAssignmentOp()) {
                    if (Token::simpleMatch(top->astOperand2(), "{") && !top->astOperand2()->previous()->function()) // TODO: check usage in init list
                        return false;
                    if (top->previous()->variable()) {
                        if (top->previous()->variable()->typeStartToken()->strAt(-1) != "const" && top->previous()->variable()->isPointer())
                            return false;
                    }
                }
            } else if (lhs->str() == ":" && lhs->astParent() && lhs->astParent()->str() == "(") { // range-based for-loop (C++11)
                const Variable* loopVar = lhs->astOperand1()->variable();
                if (!loopVar || !loopVar->valueType())
                    return false;
                if (!loopVar->valueType()->isConst(loopVar->valueType()->pointer))
                    return false;
            } else {
                if (lhs->isAssignmentOp()) {
                    const Variable* lhsVar = lhs->previous()->variable();
                    if (lhsVar && !lhsVar->isConst() && lhsVar->isReference() && lhs == lhsVar->nameToken()->next())
                        return false;
                }
            }

            const Token* jumpBackToken = nullptr;
            const Token *lastVarTok = tok1;
            const Token *end = tok1;
            for (;;) {
                if (Token::Match(end->next(), ". %name%")) {
                    end = end->tokAt(2);
                    if (end->varId())
                        lastVarTok = end;
                } else if (end->strAt(1) == "[") {
                    if (end->varId()) {
                        const Variable *var = end->variable();
                        if (var && var->isStlType(stl_containers_not_const))
                            return false;
                        const Token* assignTok = end->next()->astParent();
                        if (var && assignTok && assignTok->isAssignmentOp() && assignTok->astOperand1() && assignTok->astOperand1()->variable()) {
                            // cppcheck-suppress shadowFunction - TODO: fix this
                            const Variable* assignVar = assignTok->astOperand1()->variable();
                            if (assignVar->isPointer() && !assignVar->isConst() && var->typeScope()) {
                                const auto& funcMap = var->typeScope()->functionMap;
                                // if there is no operator that is const and returns a non-const pointer, func cannot be const
                                if (std::none_of(funcMap.cbegin(), funcMap.cend(), [](const std::pair<std::string, const Function*>& fm) {
                                    return fm.second->isConst() && fm.first == "operator[]" && !Function::returnsConst(fm.second);
                                }))
                                    return false;
                            }
                        }
                    }
                    if (!jumpBackToken)
                        jumpBackToken = end->next(); // Check inside the [] brackets
                    end = end->linkAt(1);
                } else if (end->strAt(1) == ")")
                    end = end->next();
                else
                    break;
            }

            auto hasOverloadedMemberAccess = [](const Token* end, const Scope* scope) -> bool {
                if (!end || !scope || !Token::simpleMatch(end->astParent(), "."))
                    return false;
                const std::string op = "operator" + end->astParent()->originalName();
                auto it = std::find_if(scope->functionList.begin(), scope->functionList.end(), [&op](const Function& f) {
                    return f.isConst() && f.name() == op;
                });
                if (it == scope->functionList.end() || !it->retType || !it->retType->classScope)
                    return false;
                const Function* func = it->retType->classScope->findFunction(end, /*requireConst*/ true);
                return func && func->isConst();
            };

            auto isConstContainerUsage = [&](const ValueType* vt) -> bool {
                if (!vt || !vt->container)
                    return false;
                const auto yield = vt->container->getYield(end->str());
                const Token* parent = tok1->astParent();
                while (Token::Match(parent, "(|.|::"))
                    parent = parent->astParent();
                if (contains({Library::Container::Yield::START_ITERATOR, Library::Container::Yield::END_ITERATOR, Library::Container::Yield::ITERATOR}, yield)) {
                    if (parent && parent->isComparisonOp())
                        return true;
                    // TODO: use AST
                    if (parent && parent->isAssignmentOp() && tok1->tokAt(-2)->variable() && Token::Match(tok1->tokAt(-2)->variable()->typeEndToken(), "const_iterator|const_reverse_iterator"))
                        return true;
                }
                if ((yield == Library::Container::Yield::ITEM || yield == Library::Container::Yield::AT_INDEX) &&
                    ((parent && parent->isComparisonOp()) || lhs->isAssignmentOp() || (lhs->str() == "(" && Token::Match(lhs->astParent(), "%cop%"))))
                    return true; // assume that these functions have const overloads
                return false;
            };

            if (end->strAt(1) == "(") {
                const Variable *var = lastVarTok->variable();
                if (!var)
                    return false;
                if (var->isStlType() // assume all std::*::size() and std::*::empty() are const
                    && (Token::Match(end, "size|empty|cend|crend|cbegin|crbegin|max_size|length|count|capacity|get_allocator|c_str|str ( )") || Token::Match(end, "rfind|copy"))) {
                    // empty body
                }
                else if (isConstContainerUsage(lastVarTok->valueType())) {
                    // empty body
                }
                else if (var->smartPointerType() && var->smartPointerType()->classScope && isConstMemberFunc(var->smartPointerType()->classScope, end)) {
                    // empty body
                } else if (var->isSmartPointer() && Token::simpleMatch(tok1->next(), ".") && tok1->next()->originalName().empty() && mSettings->library.isFunctionConst(end)) {
                    // empty body
                } else if (hasOverloadedMemberAccess(end, var->typeScope())) {
                    // empty body
                } else if (!var->typeScope() || (end->function() != func && !isConstMemberFunc(var->typeScope(), end))) {
                    if (!mSettings->library.isFunctionConst(end))
                        return false;
                }
            }

            // Assignment
            else if (end->next()->isAssignmentOp())
                return false;

            // Streaming
            else if (end->strAt(1) == "<<" && tok1->strAt(-1) != "<<")
                return false;
            else if (isLikelyStreamRead(tok1->previous()))
                return false;

            // ++/--
            else if (end->next()->tokType() == Token::eIncDecOp || tok1->previous()->tokType() == Token::eIncDecOp)
                return false;


            const Token* start = tok1;
            while (tok1->strAt(-1) == ")")
                tok1 = tok1->linkAt(-1);

            if (start->strAt(-1) == "delete")
                return false;

            tok1 = jumpBackToken?jumpBackToken:end; // Jump back to first [ to check inside, or jump to end of expression
            if (tok1 == end && Token::Match(end->previous(), ". %name% ( !!)") && !checkFuncCall(tok1, scope, func)) // function call on member
                return false;
        }

        // streaming: <<
        else if (Token::simpleMatch(tok1->previous(), ") <<") &&
                 isMemberVar(scope, tok1->tokAt(-2))) {
            const Variable* var = tok1->tokAt(-2)->variable();
            if (!var || !var->isMutable())
                return false;
        }

        // streaming: >> *this
        else if (Token::simpleMatch(tok1, ">> * this") && isLikelyStreamRead(tok1)) {
            return false;
        }

        // function/constructor call, return init list
        else if (const Token* funcTok = getFuncTok(tok1)) {
            if (!checkFuncCall(funcTok, scope, func))
                return false;
        } else if (Token::simpleMatch(tok1, "> (") && (!tok1->link() || !Token::Match(tok1->link()->previous(), "static_cast|const_cast|dynamic_cast|reinterpret_cast"))) {
            return false;
        }
    }

    return true;
}

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname, bool suggestStatic, bool foundAllBaseClasses)
{
    checkConstError2(tok, nullptr, classname, funcname, suggestStatic, foundAllBaseClasses);
}

void CheckClass::checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname, bool suggestStatic, bool foundAllBaseClasses)
{
    std::list<const Token *> toks{ tok1 };
    if (tok2)
        toks.push_front(tok2);
    if (!suggestStatic) {
        const std::string msg = foundAllBaseClasses ?
                                "Technically the member function '$symbol' can be const.\nThe member function '$symbol' can be made a const " :
                                "Either there is a missing 'override', or the member function '$symbol' can be const.\nUnless it overrides a base class member, the member function '$symbol' can be made a const ";
        reportError(toks, Severity::style, "functionConst",
                    "$symbol:" + classname + "::" + funcname +"\n"
                    + msg +
                    "function. Making this function 'const' should not cause compiler errors. "
                    "Even though the function can be made const function technically it may not make "
                    "sense conceptually. Think about your design and the task of the function first - is "
                    "it a function that must not change object internal state?", CWE398, Certainty::inconclusive);
    }
    else {
        const std::string msg = foundAllBaseClasses ?
                                "The member function '$symbol' can be static." :
                                "Either there is a missing 'override', or the member function '$symbol' can be static.";
        reportError(toks, Severity::style, "functionStatic",
                    "$symbol:" + classname + "::" + funcname +"\n" + msg, CWE398, Certainty::normal);
    }
}

//---------------------------------------------------------------------------
// ClassCheck: Check that initializer list is in declared order.
//---------------------------------------------------------------------------

namespace { // avoid one-definition-rule violation
    struct VarInfo {
        VarInfo(const Variable *_var, const Token *_tok)
            : var(_var), tok(_tok) {}

        const Variable *var;
        const Token *tok;
        std::vector<const Variable*> initArgs;
    };
}

void CheckClass::initializerListOrder()
{
    if (!mSettings->isPremiumEnabled("initializerList")) {
        if (!mSettings->severity.isEnabled(Severity::style))
            return;

        // This check is not inconclusive.  However it only determines if the initialization
        // order is incorrect.  It does not determine if being out of order causes
        // a real error.  Out of order is not necessarily an error but you can never
        // have an error if the list is in order so this enforces defensive programming.
        if (!mSettings->certainty.isEnabled(Certainty::inconclusive))
            return;
    }

    logChecker("CheckClass::initializerListOrder"); // style,inconclusive

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        // iterate through all member functions looking for constructors
        for (auto func = scope->functionList.cbegin(); func != scope->functionList.cend(); ++func) {
            if (func->isConstructor() && func->hasBody()) {
                // check for initializer list
                const Token *tok = func->arg->link()->next();

                if (tok->str() == ":") {
                    std::vector<VarInfo> vars;
                    tok = tok->next();

                    // find all variable initializations in list
                    for (; tok && tok != func->functionScope->bodyStart; tok = tok->next()) {
                        if (Token::Match(tok, "%name% (|{")) {
                            const Token* const end = tok->linkAt(1);
                            const Variable *var = scope->getVariable(tok->str());
                            if (var)
                                vars.emplace_back(var, tok);
                            else
                                tok = end;

                            for (; tok != end; tok = tok->next()) {
                                if (Token::Match(tok->astParent(), ".|::"))
                                    continue;
                                if (const Variable* argVar = scope->getVariable(tok->str())) {
                                    if (scope != argVar->scope())
                                        continue;
                                    if (argVar->isStatic())
                                        continue;
                                    if (tok->variable() && tok->variable()->isArgument())
                                        continue;
                                    if (var->isPointer() && (argVar->isArray() || Token::simpleMatch(tok->astParent(), "&")))
                                        continue;
                                    if (var->isReference())
                                        continue;
                                    if (Token::simpleMatch(tok->astParent(), "="))
                                        continue;
                                    vars.back().initArgs.emplace_back(argVar);
                                }
                            }
                        }
                    }

                    for (std::size_t j = 0; j < vars.size(); j++) {
                        // check for use of uninitialized arguments
                        for (const auto& arg : vars[j].initArgs)
                            if (vars[j].var->index() < arg->index())
                                initializerListError(vars[j].tok, vars[j].var->nameToken(), scope->className, vars[j].var->name(), arg->name());

                        // need at least 2 members to have out of order initialization
                        if (j == 0)
                            continue;
                        // check for out of order initialization
                        if (vars[j].var->index() < vars[j - 1].var->index())
                            initializerListError(vars[j].tok,vars[j].var->nameToken(), scope->className, vars[j].var->name());
                    }
                }
            }
        }
    }
}

void CheckClass::initializerListError(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &varname, const std::string& argname)
{
    std::list<const Token *> toks = { tok1, tok2 };
    const std::string msg = argname.empty() ?
                            "Member variable '$symbol' is in the wrong place in the initializer list." :
                            "Member variable '$symbol' uses an uninitialized argument '" + argname + "' due to the order of declarations.";
    reportError(toks, Severity::style, "initializerList",
                "$symbol:" + classname + "::" + varname + '\n' +
                msg + '\n' +
                msg + ' ' +
                "Members are initialized in the order they are declared, not in the "
                "order they are in the initializer list. Keeping the initializer list "
                "in the same order that the members were declared prevents order dependent "
                "initialization errors.", CWE398, Certainty::inconclusive);
}


//---------------------------------------------------------------------------
// Check for self initialization in initialization list
//---------------------------------------------------------------------------

void CheckClass::checkSelfInitialization()
{
    logChecker("CheckClass::checkSelfInitialization");

    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        const Function* function = scope->function;
        if (!function || !function->isConstructor())
            continue;

        const Token* tok = function->arg->link()->next();
        if (tok->str() != ":")
            continue;

        for (; tok != scope->bodyStart; tok = tok->next()) {
            if (Token::Match(tok, "[:,] %var% (|{")) {
                const Token* varTok = tok->next();
                if (Token::Match(varTok->astParent(), "(|{")) {
                    if (const Token* initTok = varTok->astParent()->astOperand2()) {
                        if (initTok->varId() == varTok->varId())
                            selfInitializationError(tok, varTok->str());
                        else if (initTok->isCast() && ((initTok->astOperand1() && initTok->astOperand1()->varId() == varTok->varId()) || (initTok->astOperand2() && initTok->astOperand2()->varId() == varTok->varId())))
                            selfInitializationError(tok, varTok->str());
                    }
                }
            }
        }
    }
}

void CheckClass::selfInitializationError(const Token* tok, const std::string& varname)
{
    reportError(tok, Severity::error, "selfInitialization", "$symbol:" + varname + "\nMember variable '$symbol' is initialized by itself.", CWE665, Certainty::normal);
}


//---------------------------------------------------------------------------
// Check for virtual function calls in constructor/destructor
//---------------------------------------------------------------------------

void CheckClass::checkVirtualFunctionCallInConstructor()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;
    logChecker("CheckClass::checkVirtualFunctionCallInConstructor"); // warning
    std::map<const Function *, std::list<const Token *>> virtualFunctionCallsMap;
    for (const Scope *scope : mSymbolDatabase->functionScopes) {
        if (scope->function == nullptr || !scope->function->hasBody() ||
            !(scope->function->isConstructor() ||
              scope->function->isDestructor()))
            continue;

        const std::list<const Token *> & virtualFunctionCalls = getVirtualFunctionCalls(*scope->function, virtualFunctionCallsMap);
        for (const Token *callToken : virtualFunctionCalls) {
            std::list<const Token *> callstack(1, callToken);
            getFirstVirtualFunctionCallStack(virtualFunctionCallsMap, callToken, callstack);
            if (callstack.empty())
                continue;
            const Function* const func = callstack.back()->function();
            if (!(func->hasVirtualSpecifier() || func->hasOverrideSpecifier()))
                continue;
            if (func->isPure())
                pureVirtualFunctionCallInConstructorError(scope->function, callstack, callstack.back()->str());
            else if (!func->hasFinalSpecifier() &&
                     !(func->nestedIn && func->nestedIn->classDef && func->nestedIn->classDef->isFinalType()))
                virtualFunctionCallInConstructorError(scope->function, callstack, callstack.back()->str());
        }
    }
}

const std::list<const Token *> & CheckClass::getVirtualFunctionCalls(const Function & function,
                                                                     std::map<const Function *, std::list<const Token *>> & virtualFunctionCallsMap)
{
    const auto found = utils::as_const(virtualFunctionCallsMap).find(&function);
    if (found != virtualFunctionCallsMap.end())
        return found->second;

    virtualFunctionCallsMap[&function] = std::list<const Token *>();
    std::list<const Token *> & virtualFunctionCalls = virtualFunctionCallsMap.find(&function)->second;

    if (!function.hasBody() || !function.functionScope)
        return virtualFunctionCalls;

    for (const Token *tok = function.arg->link(); tok != function.functionScope->bodyEnd; tok = tok->next()) {
        if (function.type != FunctionType::eConstructor &&
            function.type != FunctionType::eCopyConstructor &&
            function.type != FunctionType::eMoveConstructor &&
            function.type != FunctionType::eDestructor) {
            if ((Token::simpleMatch(tok, ") {") && tok->link() && Token::Match(tok->link()->previous(), "if|switch")) ||
                Token::simpleMatch(tok, "else {")) {
                // Assume pure virtual function call is prevented by "if|else|switch" condition
                tok = tok->linkAt(1);
                continue;
            }
        }
        if (tok->scope()->type == ScopeType::eLambda)
            tok = tok->scope()->bodyEnd->next();

        const Function * callFunction = tok->function();
        if (!callFunction ||
            function.nestedIn != callFunction->nestedIn ||
            Token::simpleMatch(tok->previous(), ".") ||
            !(tok->astParent() && (tok->astParent()->str() == "(" || (tok->astParent()->str() == "::" && Token::simpleMatch(tok->astParent()->astParent(), "(")))))
            continue;

        if (tok->previous() &&
            tok->strAt(-1) == "(") {
            const Token * prev = tok->previous();
            if (prev->previous() &&
                (mSettings->library.ignorefunction(tok->str())
                 || mSettings->library.ignorefunction(prev->strAt(-1))))
                continue;
        }

        if (callFunction->isImplicitlyVirtual()) {
            if (!callFunction->isPure() && Token::simpleMatch(tok->previous(), "::"))
                continue;
            virtualFunctionCalls.push_back(tok);
            continue;
        }

        const std::list<const Token *> & virtualFunctionCallsOfTok = getVirtualFunctionCalls(*callFunction, virtualFunctionCallsMap);
        if (!virtualFunctionCallsOfTok.empty())
            virtualFunctionCalls.push_back(tok);
    }
    return virtualFunctionCalls;
}

void CheckClass::getFirstVirtualFunctionCallStack(
    std::map<const Function *, std::list<const Token *>> & virtualFunctionCallsMap,
    const Token * callToken,
    std::list<const Token *> & pureFuncStack)
{
    const Function *callFunction = callToken->function();
    if (callFunction->isImplicitlyVirtual() && (!callFunction->isPure() || !callFunction->hasBody())) {
        pureFuncStack.push_back(callFunction->tokenDef);
        return;
    }
    auto found = utils::as_const(virtualFunctionCallsMap).find(callFunction);
    if (found == virtualFunctionCallsMap.cend() || found->second.empty()) {
        pureFuncStack.clear();
        return;
    }
    const Token * firstCall = *found->second.cbegin();
    pureFuncStack.push_back(firstCall);
    getFirstVirtualFunctionCallStack(virtualFunctionCallsMap, firstCall, pureFuncStack);
}

void CheckClass::virtualFunctionCallInConstructorError(
    const Function * scopeFunction,
    const std::list<const Token *> & tokStack,
    const std::string &funcname)
{
    if (scopeFunction && !mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("virtualCallInConstructor"))
        return;

    const char * scopeFunctionTypeName = scopeFunction ? getFunctionTypeName(scopeFunction->type) : "constructor";

    ErrorPath errorPath;
    std::transform(tokStack.cbegin(), tokStack.cend(), std::back_inserter(errorPath), [](const Token* tok) {
        return ErrorPathItem(tok, "Calling " + tok->str());
    });
    int lineNumber = 1;
    if (!errorPath.empty()) {
        lineNumber = errorPath.front().first->linenr();
        errorPath.back().second = funcname + " is a virtual function";
    }

    std::string constructorName;
    if (scopeFunction) {
        const Token *endToken = scopeFunction->argDef->link()->next();
        if (scopeFunction->type == FunctionType::eDestructor)
            constructorName = "~";
        for (const Token *tok = scopeFunction->tokenDef; tok != endToken; tok = tok->next()) {
            if (!constructorName.empty() && Token::Match(tok->previous(), "%name%|%num% %name%|%num%"))
                constructorName += ' ';
            constructorName += tok->str();
            if (tok->str() == ")")
                break;
        }
    }

    reportError(std::move(errorPath), Severity::style, "virtualCallInConstructor",
                "Virtual function '" + funcname + "' is called from " + scopeFunctionTypeName + " '" + constructorName + "' at line " + std::to_string(lineNumber) + ". Dynamic binding is not used.", CWE(0U), Certainty::normal);
}

void CheckClass::pureVirtualFunctionCallInConstructorError(
    const Function * scopeFunction,
    const std::list<const Token *> & tokStack,
    const std::string &purefuncname)
{
    const char * scopeFunctionTypeName = scopeFunction ? getFunctionTypeName(scopeFunction->type) : "constructor";

    ErrorPath errorPath;
    std::transform(tokStack.cbegin(), tokStack.cend(), std::back_inserter(errorPath), [](const Token* tok) {
        return ErrorPathItem(tok, "Calling " + tok->str());
    });
    if (!errorPath.empty())
        errorPath.back().second = purefuncname + " is a pure virtual function without body";

    reportError(std::move(errorPath), Severity::warning, "pureVirtualCall",
                "$symbol:" + purefuncname +"\n"
                "Call of pure virtual function '$symbol' in " + scopeFunctionTypeName + ".\n"
                "Call of pure virtual function '$symbol' in " + scopeFunctionTypeName + ". The call will fail during runtime.", CWE(0U), Certainty::normal);
}


//---------------------------------------------------------------------------
// Check for members hiding inherited members with the same name
//---------------------------------------------------------------------------

void CheckClass::checkDuplInheritedMembers()
{
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("duplInheritedMember"))
        return;

    logChecker("CheckClass::checkDuplInheritedMembers"); // warning

    // Iterate over all classes
    for (const Type &classIt : mSymbolDatabase->typeList) {
        // Iterate over the parent classes
        checkDuplInheritedMembersRecursive(&classIt, &classIt);
    }
}

namespace {
    struct DuplMemberInfo {
        DuplMemberInfo(const Variable* cv, const Variable* pcv, const Type::BaseInfo* pc) : classVar(cv), parentClassVar(pcv), parentClass(pc) {}
        const Variable* classVar;
        const Variable* parentClassVar;
        const Type::BaseInfo* parentClass;
    };
    struct DuplMemberFuncInfo {
        DuplMemberFuncInfo(const Function* cf, const Function* pcf, const Type::BaseInfo* pc) : classFunc(cf), parentClassFunc(pcf), parentClass(pc) {}
        const Function* classFunc;
        const Function* parentClassFunc;
        const Type::BaseInfo* parentClass;
    };
}

static std::vector<DuplMemberInfo> getDuplInheritedMembersRecursive(const Type* typeCurrent, const Type* typeBase, bool skipPrivate = true)
{
    std::vector<DuplMemberInfo> results;
    for (const Type::BaseInfo &parentClassIt : typeBase->derivedFrom) {
        // Check if there is info about the 'Base' class
        if (!parentClassIt.type || !parentClassIt.type->classScope)
            continue;
        // Don't crash on recursive templates
        if (parentClassIt.type == typeBase)
            continue;
        // Check if they have a member variable in common
        for (const Variable &classVarIt : typeCurrent->classScope->varlist) {
            for (const Variable &parentClassVarIt : parentClassIt.type->classScope->varlist) {
                if (classVarIt.name() == parentClassVarIt.name() && (!parentClassVarIt.isPrivate() || !skipPrivate)) // Check if the class and its parent have a common variable
                    results.emplace_back(&classVarIt, &parentClassVarIt, &parentClassIt);
            }
        }
        if (typeCurrent != parentClassIt.type) {
            const auto recursive = getDuplInheritedMembersRecursive(typeCurrent, parentClassIt.type, skipPrivate);
            results.insert(results.end(), recursive.begin(), recursive.end());
        }
    }
    return results;
}

static std::vector<DuplMemberFuncInfo> getDuplInheritedMemberFunctionsRecursive(const Type* typeCurrent, const Type* typeBase, bool skipPrivate = true)
{
    std::vector<DuplMemberFuncInfo> results;
    for (const Type::BaseInfo &parentClassIt : typeBase->derivedFrom) {
        // Check if there is info about the 'Base' class
        if (!parentClassIt.type || !parentClassIt.type->classScope)
            continue;
        // Don't crash on recursive templates
        if (parentClassIt.type == typeBase)
            continue;
        for (const Function& classFuncIt : typeCurrent->classScope->functionList) {
            if (classFuncIt.isImplicitlyVirtual())
                continue;
            if (classFuncIt.tokenDef->isExpandedMacro())
                continue;
            if (classFuncIt.templateDef)
                continue;
            for (const Function& parentClassFuncIt : parentClassIt.type->classScope->functionList) {
                if (classFuncIt.name() == parentClassFuncIt.name() &&
                    (parentClassFuncIt.access != AccessControl::Private || !skipPrivate) &&
                    !classFuncIt.isConstructor() && !classFuncIt.isDestructor() &&
                    classFuncIt.argsMatch(parentClassIt.type->classScope, parentClassFuncIt.argDef, classFuncIt.argDef, "", 0) &&
                    (classFuncIt.isConst() == parentClassFuncIt.isConst() || Function::returnsConst(&classFuncIt) == Function::returnsConst(&parentClassFuncIt)) &&
                    !(classFuncIt.isDelete() || parentClassFuncIt.isDelete()))
                    results.emplace_back(&classFuncIt, &parentClassFuncIt, &parentClassIt);
            }
        }
        if (typeCurrent != parentClassIt.type) {
            const auto recursive = getDuplInheritedMemberFunctionsRecursive(typeCurrent, parentClassIt.type);
            results.insert(results.end(), recursive.begin(), recursive.end());
        }
    }
    return results;
}

void CheckClass::checkDuplInheritedMembersRecursive(const Type* typeCurrent, const Type* typeBase)
{
    const auto resultsVar = getDuplInheritedMembersRecursive(typeCurrent, typeBase);
    for (const auto& r : resultsVar) {
        duplInheritedMembersError(r.classVar->nameToken(), r.parentClassVar->nameToken(),
                                  typeCurrent->name(), r.parentClass->type->name(), r.classVar->name(),
                                  typeCurrent->classScope->type == ScopeType::eStruct,
                                  r.parentClass->type->classScope->type == ScopeType::eStruct);
    }

    const auto resultsFunc = getDuplInheritedMemberFunctionsRecursive(typeCurrent, typeBase);
    for (const auto& r : resultsFunc) {
        duplInheritedMembersError(r.classFunc->token, r.parentClassFunc->token,
                                  typeCurrent->name(), r.parentClass->type->name(), r.classFunc->name(),
                                  typeCurrent->classScope->type == ScopeType::eStruct,
                                  r.parentClass->type->classScope->type == ScopeType::eStruct, /*isFunction*/ true);
    }
}

void CheckClass::duplInheritedMembersError(const Token *tok1, const Token* tok2,
                                           const std::string &derivedName, const std::string &baseName,
                                           const std::string &memberName, bool derivedIsStruct, bool baseIsStruct, bool isFunction)
{
    ErrorPath errorPath;
    const std::string member = isFunction ? "function" : "variable";
    errorPath.emplace_back(tok2, "Parent " + member + " '" + baseName + "::" + memberName + "'");
    errorPath.emplace_back(tok1, "Derived " + member + " '" + derivedName + "::" + memberName + "'");

    const std::string symbols = "$symbol:" + derivedName + "\n$symbol:" + memberName + "\n$symbol:" + baseName;

    const std::string message = "The " + std::string(derivedIsStruct ? "struct" : "class") + " '" + derivedName +
                                "' defines member " + member + " with name '" + memberName + "' also defined in its parent " +
                                std::string(baseIsStruct ? "struct" : "class") + " '" + baseName + "'.";
    reportError(std::move(errorPath), Severity::warning, "duplInheritedMember", symbols + '\n' + message, CWE398, Certainty::normal);
}


//---------------------------------------------------------------------------
// Check that copy constructor and operator defined together
//---------------------------------------------------------------------------

enum class CtorType : std::uint8_t {
    NO,
    WITHOUT_BODY,
    WITH_BODY
};

void CheckClass::checkCopyCtorAndEqOperator()
{
    // TODO: This is disabled because of #8388
    // The message must be clarified. How is the behaviour different?
    if ((true) || !mSettings->severity.isEnabled(Severity::warning)) // NOLINT(readability-simplify-boolean-expr,readability-redundant-parentheses)
        return;

    // logChecker

    for (const Scope * scope : mSymbolDatabase->classAndStructScopes) {

        const bool hasNonStaticVars = std::any_of(scope->varlist.begin(), scope->varlist.end(), [](const Variable& var) {
            return !var.isStatic();
        });
        if (!hasNonStaticVars)
            continue;

        CtorType copyCtors = CtorType::NO;
        bool moveCtor = false;
        CtorType assignmentOperators = CtorType::NO;

        for (const Function &func : scope->functionList) {
            if (copyCtors == CtorType::NO && func.type == FunctionType::eCopyConstructor) {
                copyCtors = func.hasBody() ? CtorType::WITH_BODY : CtorType::WITHOUT_BODY;
            }
            if (assignmentOperators == CtorType::NO && func.type == FunctionType::eOperatorEqual) {
                const Variable * variable = func.getArgumentVar(0);
                if (variable && variable->type() && variable->type()->classScope == scope) {
                    assignmentOperators = func.hasBody() ? CtorType::WITH_BODY : CtorType::WITHOUT_BODY;
                }
            }
            if (func.type == FunctionType::eMoveConstructor) {
                moveCtor = true;
                break;
            }
        }

        if (moveCtor)
            continue;

        // No method defined
        if (copyCtors != CtorType::WITH_BODY && assignmentOperators != CtorType::WITH_BODY)
            continue;

        // both methods are defined
        if (copyCtors != CtorType::NO && assignmentOperators != CtorType::NO)
            continue;

        copyCtorAndEqOperatorError(scope->classDef, scope->className, scope->type == ScopeType::eStruct, copyCtors == CtorType::WITH_BODY);
    }
}

void CheckClass::copyCtorAndEqOperatorError(const Token *tok, const std::string &classname, bool isStruct, bool hasCopyCtor)
{
    const std::string message = "$symbol:" + classname + "\n"
                                "The " + std::string(isStruct ? "struct" : "class") + " '$symbol' has '" +
                                getFunctionTypeName(hasCopyCtor ? FunctionType::eCopyConstructor : FunctionType::eOperatorEqual) +
                                "' but lack of '" + getFunctionTypeName(hasCopyCtor ? FunctionType::eOperatorEqual : FunctionType::eCopyConstructor) +
                                "'.";
    reportError(tok, Severity::warning, "copyCtorAndEqOperator", message);
}

void CheckClass::checkOverride()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("missingOverride"))
        return;
    if (mSettings->standards.cpp < Standards::CPP11)
        return;
    logChecker("CheckClass::checkMissingOverride"); // style,c++03
    for (const Scope * classScope : mSymbolDatabase->classAndStructScopes) {
        if (!classScope->definedType || classScope->definedType->derivedFrom.empty())
            continue;
        for (const Function &func : classScope->functionList) {
            if (func.hasOverrideSpecifier() || func.hasFinalSpecifier())
                continue;
            if (func.tokenDef->isExpandedMacro())
                continue;
            if (func.templateDef)
                continue;
            const Function *baseFunc = func.getOverriddenFunction();
            if (baseFunc)
                overrideError(baseFunc, &func);
        }
    }
}

void CheckClass::overrideError(const Function *funcInBase, const Function *funcInDerived)
{
    const std::string functionName = funcInDerived ? ((funcInDerived->isDestructor() ? "~" : "") + funcInDerived->name()) : "";
    const std::string funcType = (funcInDerived && funcInDerived->isDestructor()) ? "destructor" : "function";

    ErrorPath errorPath;
    if (funcInBase && funcInDerived) {
        errorPath.emplace_back(funcInBase->tokenDef, "Virtual " + funcType + " in base class");
        errorPath.emplace_back(funcInDerived->tokenDef, char(std::toupper(funcType[0])) + funcType.substr(1) + " in derived class");
    }

    reportError(std::move(errorPath), Severity::style, "missingOverride",
                "$symbol:" + functionName + "\n"
                "The " + funcType + " '$symbol' overrides a " + funcType + " in a base class but is not marked with a 'override' specifier.",
                CWE(0U) /* Unknown CWE! */,
                Certainty::normal);
}

void CheckClass::uselessOverrideError(const Function *funcInBase, const Function *funcInDerived, bool isSameCode)
{
    const std::string functionName = funcInDerived ? ((funcInDerived->isDestructor() ? "~" : "") + funcInDerived->name()) : "";
    const std::string funcType = (funcInDerived && funcInDerived->isDestructor()) ? "destructor" : "function";

    ErrorPath errorPath;
    if (funcInBase && funcInDerived) {
        errorPath.emplace_back(funcInBase->tokenDef, "Virtual " + funcType + " in base class");
        errorPath.emplace_back(funcInDerived->tokenDef, char(std::toupper(funcType[0])) + funcType.substr(1) + " in derived class");
    }

    std::string errStr = "\nThe " + funcType + " '$symbol' overrides a " + funcType + " in a base class but ";
    if (isSameCode) {
        errStr += "is identical to the overridden function";
    }
    else
        errStr += "just delegates back to the base class.";
    reportError(std::move(errorPath), Severity::style, "uselessOverride",
                "$symbol:" + functionName +
                errStr,
                CWE(0U) /* Unknown CWE! */,
                Certainty::normal);
}

static const Token* getSingleFunctionCall(const Scope* scope) {
    const Token* const start = scope->bodyStart->next();
    const Token* const end = Token::findsimplematch(start, ";", 1, scope->bodyEnd);
    if (!end || end->next() != scope->bodyEnd)
        return nullptr;
    const Token* ftok = start;
    if (ftok->str() == "return")
        ftok = ftok->astOperand1();
    else {
        while (Token::Match(ftok, "%name%|::"))
            ftok = ftok->next();
    }
    if (Token::simpleMatch(ftok, "(") && ftok->previous()->function())
        return ftok->previous();
    return nullptr;
}

static bool compareTokenRanges(const Token* start1, const Token* end1, const Token* start2, const Token* end2) {
    const Token* tok1 = start1;
    const Token* tok2 = start2;
    bool isEqual = false;
    while (tok1 && tok2) {
        if (tok1->function() != tok2->function())
            break;
        if (tok1->str() != tok2->str())
            break;
        if (tok1->str() == "this")
            break;
        if (tok1->isExpandedMacro() || tok2->isExpandedMacro())
            break;
        if (tok1 == end1 && tok2 == end2) {
            isEqual = true;
            break;
        }
        tok1 = tok1->next();
        tok2 = tok2->next();
    }
    return isEqual;
}

void CheckClass::checkUselessOverride()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("uselessOverride"))
        return;

    logChecker("CheckClass::checkUselessOverride"); // style

    for (const Scope* classScope : mSymbolDatabase->classAndStructScopes) {
        if (!classScope->definedType || classScope->definedType->derivedFrom.size() != 1)
            continue;
        for (const Function& func : classScope->functionList) {
            if (!func.functionScope)
                continue;
            if (func.hasFinalSpecifier())
                continue;
            const Function* baseFunc = func.getOverriddenFunction();
            if (!baseFunc || baseFunc->isPure() || baseFunc->access != func.access)
                continue;
            if (std::any_of(classScope->functionList.begin(), classScope->functionList.end(), [&func](const Function& f) { // check for overloads
                if (&f == &func)
                    return false;
                return f.name() == func.name();
            }))
                continue;
            if (func.token->isExpandedMacro() || baseFunc->token->isExpandedMacro())
                continue;
            if (baseFunc->functionScope) {
                bool isSameCode = compareTokenRanges(baseFunc->argDef, baseFunc->argDef->link(), func.argDef, func.argDef->link()); // function arguments
                if (isSameCode) {
                    isSameCode = compareTokenRanges(baseFunc->functionScope->bodyStart, baseFunc->functionScope->bodyEnd, // function body
                                                    func.functionScope->bodyStart, func.functionScope->bodyEnd);

                    if (isSameCode) {
                        // bailout for shadowed members
                        if (!classScope->definedType ||
                            !getDuplInheritedMembersRecursive(classScope->definedType, classScope->definedType, /*skipPrivate*/ false).empty() ||
                            !getDuplInheritedMemberFunctionsRecursive(classScope->definedType, classScope->definedType, /*skipPrivate*/ false).empty())
                            continue;
                        uselessOverrideError(baseFunc, &func, true);
                        continue;
                    }
                }
            }
            if (const Token* const call = getSingleFunctionCall(func.functionScope)) {
                if (call->function() != baseFunc)
                    continue;
                if (Token::simpleMatch(call->astParent(), "."))
                    continue;
                std::vector<const Token*> callArgs = getArguments(call);
                if (func.argumentList.size() != callArgs.size() ||
                    !std::equal(func.argumentList.begin(), func.argumentList.end(), callArgs.begin(), [](const Variable& v, const Token* t) {
                    return v.nameToken() && v.nameToken()->str() == t->str();
                }))
                    continue;
                uselessOverrideError(baseFunc, &func);
            }
        }
    }
}

static const Variable* getSingleReturnVar(const Scope* scope) {
    if (!scope || !scope->bodyStart)
        return nullptr;
    const Token* const start = scope->bodyStart->next();
    const Token* const end = Token::findsimplematch(start, ";", 1, scope->bodyEnd);
    if (!end || end->next() != scope->bodyEnd)
        return nullptr;
    if (!start->astOperand1() || start->str() != "return")
        return nullptr;
    const Token* tok = start->astOperand1();
    if (tok->str() == ".") {
        const Token* top = tok->astOperand1();
        while (Token::Match(top, "[[.]"))
            top = top->astOperand1();
        if (!Token::Match(top, "%var%"))
            return nullptr;
        tok = tok->astOperand2();
    }
    return tok->variable();
}

void CheckClass::checkReturnByReference()
{
    if (!mSettings->severity.isEnabled(Severity::performance) && !mSettings->isPremiumEnabled("returnByReference"))
        return;

    logChecker("CheckClass::checkReturnByReference"); // performance

    for (const Scope* classScope : mSymbolDatabase->classAndStructScopes) {
        for (const Function& func : classScope->functionList) {
            if (Function::returnsPointer(&func) || Function::returnsReference(&func) || Function::returnsStandardType(&func))
                continue;
            if (func.isImplicitlyVirtual())
                continue;
            if (func.isOperator())
                continue;
            if (func.functionPointerUsage)
                continue;
            if (const Library::Container* container = mSettings->library.detectContainer(func.retDef))
                if (container->view)
                    continue;
            if (!func.isConst() && func.hasRvalRefQualifier())
                // this method could be used by temporary objects, return by value can be dangerous
                continue;
            if (const Variable* var = getSingleReturnVar(func.functionScope)) {
                if (!var->valueType())
                    continue;
                if (var->isArgument())
                    continue;
                const bool isContainer = var->valueType()->type == ValueType::Type::CONTAINER && var->valueType()->container;
                const bool isView = isContainer && var->valueType()->container->view;
                bool warn = isContainer && !isView;
                if (!warn && !isView) {
                    const std::size_t size = var->valueType()->getSizeOf(*mSettings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer);
                    if (size > 2 * mSettings->platform.sizeof_pointer)
                        warn = true;
                }
                if (warn)
                    returnByReferenceError(&func, var);
            }
        }
    }
}

void CheckClass::returnByReferenceError(const Function* func, const Variable* var)
{
    const Token* tok = func ? func->tokenDef : nullptr;
    const std::string message = "Function '" + (func ? func->name() : "func") + "()' should return member '" + (var ? var->name() : "var") + "' by const reference.";
    reportError(tok, Severity::performance, "returnByReference", message);
}

void CheckClass::checkThisUseAfterFree()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckClass::checkThisUseAfterFree"); // warning

    for (const Scope * classScope : mSymbolDatabase->classAndStructScopes) {

        for (const Variable &var : classScope->varlist) {
            // Find possible "self pointer".. pointer/smartpointer member variable of "self" type.
            if (var.valueType() && var.valueType()->smartPointerType != classScope->definedType && var.valueType()->typeScope != classScope) {
                const ValueType valueType = ValueType::parseDecl(var.typeStartToken(), *mSettings);
                if (valueType.smartPointerType != classScope->definedType)
                    continue;
            }

            // If variable is not static, check that "this" is assigned
            if (!var.isStatic()) {
                bool hasAssign = false;
                for (const Function &func : classScope->functionList) {
                    if (func.type != FunctionType::eFunction || !func.hasBody())
                        continue;
                    for (const Token *tok = func.functionScope->bodyStart; tok != func.functionScope->bodyEnd; tok = tok->next()) {
                        if (Token::Match(tok, "%varid% = this|shared_from_this", var.declarationId())) {
                            hasAssign = true;
                            break;
                        }
                    }
                    if (hasAssign)
                        break;
                }
                if (!hasAssign)
                    continue;
            }

            // Check usage of self pointer..
            for (const Function &func : classScope->functionList) {
                if (func.type != FunctionType::eFunction || !func.hasBody())
                    continue;

                const Token * freeToken = nullptr;
                std::set<const Function *> callstack;
                checkThisUseAfterFreeRecursive(classScope, &func, &var, std::move(callstack), freeToken);
            }
        }
    }
}

bool CheckClass::checkThisUseAfterFreeRecursive(const Scope *classScope, const Function *func, const Variable *selfPointer, std::set<const Function *> callstack, const Token *&freeToken)
{
    if (!func || !func->functionScope)
        return false;

    // avoid recursion
    if (callstack.count(func))
        return false;
    callstack.insert(func);

    const Token * const bodyStart = func->functionScope->bodyStart;
    const Token * const bodyEnd = func->functionScope->bodyEnd;
    for (const Token *tok = bodyStart; tok != bodyEnd; tok = tok->next()) {
        const bool isDestroyed = freeToken != nullptr && !func->isStatic();
        if (Token::Match(tok, "delete %var% ;") && selfPointer == tok->next()->variable()) {
            freeToken = tok;
            tok = tok->tokAt(2);
        } else if (Token::Match(tok, "%var% . reset ( )") && selfPointer == tok->variable())
            freeToken = tok;
        else if (Token::Match(tok->previous(), "!!. %name% (") && tok->function() && tok->function()->nestedIn == classScope && tok->function()->type == FunctionType::eFunction) {
            if (isDestroyed) {
                thisUseAfterFree(selfPointer->nameToken(), freeToken, tok);
                return true;
            }
            if (checkThisUseAfterFreeRecursive(classScope, tok->function(), selfPointer, callstack, freeToken))
                return true;
        } else if (isDestroyed && Token::Match(tok->previous(), "!!. %name%") && tok->variable() && tok->variable()->scope() == classScope && !tok->variable()->isStatic() && !tok->variable()->isArgument()) {
            thisUseAfterFree(selfPointer->nameToken(), freeToken, tok);
            return true;
        } else if (freeToken && Token::Match(tok, "return|throw")) {
            // TODO
            return tok->str() == "throw";
        } else if (tok->str() == "{" && tok->scope()->type == ScopeType::eLambda) {
            tok = tok->link();
        }
    }
    return false;
}

void CheckClass::thisUseAfterFree(const Token *self, const Token *free, const Token *use)
{
    std::string selfPointer = self ? self->str() : "ptr";
    ErrorPath errorPath = { ErrorPathItem(self, "Assuming '" + selfPointer + "' is used as 'this'"), ErrorPathItem(free, "Delete '" + selfPointer + "', invalidating 'this'"), ErrorPathItem(use, "Call method when 'this' is invalid") };
    const std::string usestr = use ? use->str() : "x";
    const std::string usemsg = use && use->function() ? ("Calling method '" + usestr + "()'") : ("Using member '" + usestr + "'");
    reportError(std::move(errorPath), Severity::warning, "thisUseAfterFree",
                "$symbol:" + selfPointer + "\n" +
                usemsg + " when 'this' might be invalid",
                CWE(0), Certainty::normal);
}

void CheckClass::checkUnsafeClassRefMember()
{
    if (!mSettings->safeChecks.classes || !mSettings->severity.isEnabled(Severity::warning))
        return;
    logChecker("CheckClass::checkUnsafeClassRefMember"); // warning,safeChecks
    for (const Scope * classScope : mSymbolDatabase->classAndStructScopes) {
        for (const Function &func : classScope->functionList) {
            if (!func.hasBody() || !func.isConstructor())
                continue;

            const Token *initList = func.constructorMemberInitialization();
            while (Token::Match(initList, "[:,] %name% (")) {
                if (Token::Match(initList->tokAt(2), "( %var% )")) {
                    const Variable * const memberVar = initList->next()->variable();
                    const Variable * const argVar = initList->tokAt(3)->variable();
                    if (memberVar && argVar && memberVar->isConst() && memberVar->isReference() && argVar->isArgument() && argVar->isConst() && argVar->isReference())
                        unsafeClassRefMemberError(initList->next(), classScope->className + "::" + memberVar->name());
                }
                initList = initList->linkAt(2)->next();
            }
        }
    }
}

void CheckClass::unsafeClassRefMemberError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "unsafeClassRefMember",
                "$symbol:" + varname + "\n"
                "Unsafe class: The const reference member '$symbol' is initialized by a const reference constructor argument. You need to be careful about lifetime issues.\n"
                "Unsafe class checking: The const reference member '$symbol' is initialized by a const reference constructor argument. You need to be careful about lifetime issues. If you pass a local variable or temporary value in this constructor argument, be extra careful. If the argument is always some global object that is never destroyed then this is safe usage. However it would be defensive to make the member '$symbol' a non-reference variable or a smart pointer.",
                CWE(0), Certainty::normal);
}

// a Clang-built executable will crash when using the anonymous MyFileInfo later on - so put it in a unique namespace for now
// see https://trac.cppcheck.net/ticket/12108 for more details
#ifdef __clang__
inline namespace CheckClass_internal
#else
namespace
#endif
{
    /* multifile checking; one definition rule violations */
    class MyFileInfo : public Check::FileInfo {
    public:
        using Check::FileInfo::FileInfo;
        struct NameLoc {
            std::string className;
            std::string fileName;
            std::string configuration;
            int lineNumber;
            int column;
            std::size_t hash;

            bool isSameLocation(const NameLoc& other) const {
                return fileName == other.fileName &&
                       lineNumber == other.lineNumber &&
                       column == other.column;
            }
        };
        std::vector<NameLoc> classDefinitions;

        /** Convert data into xml string */
        std::string toString() const override
        {
            std::string ret;
            for (const NameLoc &nameLoc: classDefinitions) {
                ret += "<class name=\"" + ErrorLogger::toxml(nameLoc.className) +
                       "\" file=\"" + ErrorLogger::toxml(nameLoc.fileName) +
                       "\" configuration=\"" + ErrorLogger::toxml(nameLoc.configuration) +
                       "\" line=\"" + std::to_string(nameLoc.lineNumber) +
                       "\" col=\"" + std::to_string(nameLoc.column) +
                       "\" hash=\"" + std::to_string(nameLoc.hash) +
                       "\"/>\n";
            }
            return ret;
        }
    };
}

Check::FileInfo *CheckClass::getFileInfo(const Tokenizer &tokenizer, const Settings& /*settings*/, const std::string& currentConfig) const
{
    if (!tokenizer.isCPP())
        return nullptr;

    // One definition rule
    std::vector<MyFileInfo::NameLoc> classDefinitions;
    for (const Scope * classScope : tokenizer.getSymbolDatabase()->classAndStructScopes) {
        if (classScope->isAnonymous())
            continue;

        if (classScope->classDef && Token::simpleMatch(classScope->classDef->previous(), ">"))
            continue;

        // the full definition must be compared
        const bool fullDefinition = std::all_of(classScope->functionList.cbegin(),
                                                classScope->functionList.cend(),
                                                [](const Function& f) {
            return f.hasBody();
        });
        if (!fullDefinition)
            continue;

        std::string name;
        const Scope *scope = classScope;
        while (scope->isClassOrStruct() && !classScope->className.empty()) {
            if (Token::Match(scope->classDef, "struct|class %name% :: %name%")) {
                // TODO handle such classnames
                name.clear();
                break;
            }
            name = scope->className + "::" + name;
            scope = scope->nestedIn;
        }
        if (name.empty())
            continue;
        name.erase(name.size() - 2);
        if (scope->type != ScopeType::eGlobal)
            continue;

        MyFileInfo::NameLoc nameLoc;
        nameLoc.className = std::move(name);
        nameLoc.fileName = tokenizer.list.file(classScope->classDef);
        nameLoc.lineNumber = classScope->classDef->linenr();
        nameLoc.column = classScope->classDef->column();

        // Calculate hash from the full class/struct definition
        std::string def;
        for (const Token *tok = classScope->classDef; tok != classScope->bodyEnd; tok = tok->next())
            def += tok->str();
        for (const Function &f: classScope->functionList) {
            if (f.functionScope && f.functionScope->nestedIn != classScope) {
                for (const Token *tok = f.functionScope->bodyStart; tok != f.functionScope->bodyEnd; tok = tok->next())
                    def += tok->str();
            }
        }
        nameLoc.hash = std::hash<std::string> {}(def);
        nameLoc.configuration = currentConfig;

        classDefinitions.push_back(std::move(nameLoc));
    }

    if (classDefinitions.empty())
        return nullptr;

    auto *fileInfo = new MyFileInfo(tokenizer.list.getFiles()[0]);
    fileInfo->classDefinitions.swap(classDefinitions);
    return fileInfo;
}

Check::FileInfo * CheckClass::loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const
{
    auto *fileInfo = new MyFileInfo;
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "class") != 0)
            continue;
        const char *name = e->Attribute("name");
        const char *file = e->Attribute("file");
        const char *configuration = e->Attribute("configuration");
        const char *line = e->Attribute("line");
        const char *col = e->Attribute("col");
        const char *hash = e->Attribute("hash");
        if (name && file && configuration && line && col && hash) {
            MyFileInfo::NameLoc nameLoc;
            nameLoc.className = name;
            nameLoc.fileName = file;
            nameLoc.configuration = configuration;
            nameLoc.lineNumber = strToInt<int>(line);
            nameLoc.column = strToInt<int>(col);
            nameLoc.hash = strToInt<std::size_t>(hash);
            fileInfo->classDefinitions.push_back(std::move(nameLoc));
        }
    }
    if (fileInfo->classDefinitions.empty()) {
        delete fileInfo;
        fileInfo = nullptr;
    }
    return fileInfo;
}

bool CheckClass::analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    (void)ctu;
    (void)settings;

    CheckClass dummy(nullptr, &settings, &errorLogger);
    dummy.
    logChecker("CheckClass::analyseWholeProgram");

    if (fileInfo.empty())
        return false;

    bool foundErrors = false;

    std::unordered_map<std::string, MyFileInfo::NameLoc> all;

    for (const Check::FileInfo* fi1 : fileInfo) {
        const auto *fi = dynamic_cast<const MyFileInfo*>(fi1);
        if (!fi)
            continue;
        for (const MyFileInfo::NameLoc &nameLoc : fi->classDefinitions) {
            auto it = all.find(nameLoc.className);
            if (it == all.end()) {
                all[nameLoc.className] = nameLoc;
                continue;
            }
            if (it->second.hash == nameLoc.hash)
                continue;
            if (it->second.fileName == nameLoc.fileName && it->second.configuration != nameLoc.configuration)
                continue;
            // Same location, sometimes the hash is different wrongly (possibly because of different token simplifications).
            if (it->second.isSameLocation(nameLoc))
                continue;

            std::list<ErrorMessage::FileLocation> locationList;
            locationList.emplace_back(nameLoc.fileName, nameLoc.lineNumber, nameLoc.column);
            locationList.emplace_back(it->second.fileName, it->second.lineNumber, it->second.column);

            const ErrorMessage errmsg(std::move(locationList),
                                      fi->file0,
                                      Severity::error,
                                      "$symbol:" + nameLoc.className +
                                      "\nThe one definition rule is violated, different classes/structs have the same name '$symbol'",
                                      "ctuOneDefinitionRuleViolation",
                                      CWE_ONE_DEFINITION_RULE,
                                      Certainty::normal);
            errorLogger.reportErr(errmsg);

            foundErrors = true;
        }
    }
    return foundErrors;
}

void CheckClass::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    if (tokenizer.isC())
        return;

    CheckClass checkClass(&tokenizer, &tokenizer.getSettings(), errorLogger);

    // can't be a simplified check .. the 'sizeof' is used.
    checkClass.checkMemset();
    checkClass.constructors();
    checkClass.privateFunctions();
    checkClass.operatorEqRetRefThis();
    checkClass.thisSubtraction();
    checkClass.operatorEqToSelf();
    checkClass.initializerListOrder();
    checkClass.initializationListUsage();
    checkClass.checkSelfInitialization();
    checkClass.virtualDestructor();
    checkClass.checkConst();
    checkClass.copyconstructors();
    checkClass.checkVirtualFunctionCallInConstructor();
    checkClass.checkDuplInheritedMembers();
    checkClass.checkExplicitConstructors();
    checkClass.checkCopyCtorAndEqOperator();
    checkClass.checkOverride();
    checkClass.checkUselessOverride();
    checkClass.checkReturnByReference();
    checkClass.checkThisUseAfterFree();
    checkClass.checkUnsafeClassRefMember();
}

void CheckClass::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckClass c(nullptr, settings, errorLogger);
    c.noConstructorError(nullptr, "classname", false);
    c.noExplicitConstructorError(nullptr, "classname", false);
    //c.copyConstructorMallocError(nullptr, 0, "var");
    c.copyConstructorShallowCopyError(nullptr, "var");
    c.noCopyConstructorError(nullptr, false, nullptr, false);
    c.noOperatorEqError(nullptr, false, nullptr, false);
    c.noDestructorError(nullptr, false, nullptr);
    c.uninitVarError(nullptr, false, FunctionType::eConstructor, "classname", "varname", false, false);
    c.uninitVarError(nullptr, true, FunctionType::eConstructor, "classname", "varnamepriv", false, false);
    c.uninitVarError(nullptr, false, FunctionType::eConstructor, "classname", "varname", true, false);
    c.uninitVarError(nullptr, true, FunctionType::eConstructor, "classname", "varnamepriv", true, false);
    c.missingMemberCopyError(nullptr, FunctionType::eConstructor, "classname", "varnamepriv");
    c.operatorEqVarError(nullptr, "classname", "", false);
    c.unusedPrivateFunctionError(nullptr, nullptr, "classname", "funcname");
    c.memsetError(nullptr, "memfunc", "classname", "class");
    c.memsetErrorReference(nullptr, "memfunc", "class");
    c.memsetErrorFloat(nullptr, "class");
    c.mallocOnClassWarning(nullptr, "malloc", nullptr);
    c.mallocOnClassError(nullptr, "malloc", nullptr, "std::string");
    c.virtualDestructorError(nullptr, "Base", "Derived", false);
    c.thisSubtractionError(nullptr);
    c.operatorEqRetRefThisError(nullptr);
    c.operatorEqMissingReturnStatementError(nullptr, true);
    c.operatorEqShouldBeLeftUnimplementedError(nullptr);
    c.operatorEqToSelfError(nullptr);
    c.checkConstError(nullptr, "class", "function", false);
    c.checkConstError(nullptr, "class", "function", true);
    c.initializerListError(nullptr, nullptr, "class", "variable");
    c.suggestInitializationList(nullptr, "variable");
    c.selfInitializationError(nullptr, "var");
    c.duplInheritedMembersError(nullptr, nullptr, "class", "class", "variable", false, false);
    c.copyCtorAndEqOperatorError(nullptr, "class", false, false);
    c.overrideError(nullptr, nullptr);
    c.uselessOverrideError(nullptr, nullptr);
    c.returnByReferenceError(nullptr, nullptr);
    c.pureVirtualFunctionCallInConstructorError(nullptr, std::list<const Token *>(), "f");
    c.virtualFunctionCallInConstructorError(nullptr, std::list<const Token *>(), "f");
    c.thisUseAfterFree(nullptr, nullptr, nullptr);
    c.unsafeClassRefMemberError(nullptr, "UnsafeClass::var");
    // TODO: ctuOneDefinitionRuleViolation
}
