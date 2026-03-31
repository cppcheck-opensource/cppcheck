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
// Check functions
//---------------------------------------------------------------------------

#include "checkfunctions.h"

#include "astutils.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <iomanip>
#include <list>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckFunctions instance;
}

static const CWE CWE252(252U);  // Unchecked Return Value
static const CWE CWE477(477U);  // Use of Obsolete Functions
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE628(628U);  // Function Call with Incorrectly Specified Arguments
static const CWE CWE686(686U);  // Function Call With Incorrect Argument Type
static const CWE CWE687(687U);  // Function Call With Incorrectly Specified Argument Value
static const CWE CWE688(688U);  // Function Call With Incorrect Variable or Reference as Argument

void CheckFunctions::checkProhibitedFunctions()
{
    const bool checkAlloca = mSettings->severity.isEnabled(Severity::warning) && ((mTokenizer->isC() && mSettings->standards.c >= Standards::C99) || mSettings->standards.cpp >= Standards::CPP11);

    logChecker("CheckFunctions::checkProhibitedFunctions");

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% (") && tok->varId() == 0)
                continue;
            // alloca() is special as it depends on the code being C or C++, so it is not in Library
            if (checkAlloca && Token::simpleMatch(tok, "alloca (") && (!tok->function() || tok->function()->nestedIn->type == ScopeType::eGlobal)) {
                if (tok->isC()) {
                    if (mSettings->standards.c > Standards::C89)
                        reportError(tok, Severity::warning, "allocaCalled",
                                    "$symbol:alloca\n"
                                    "Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.\n"
                                    "The obsolete function 'alloca' is called. In C99 and later it is recommended to use a variable length array or "
                                    "a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons "
                                    "(http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).");
                } else
                    reportError(tok, Severity::warning, "allocaCalled",
                                "$symbol:alloca\n"
                                "Obsolete function 'alloca' called.\n"
                                "The obsolete function 'alloca' is called. In C++11 and later it is recommended to use std::array<> or "
                                "a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons "
                                "(http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).");
            } else {
                if (tok->function() && tok->function()->hasBody())
                    continue;

                const Library::WarnInfo* wi = mSettings->library.getWarnInfo(tok);
                if (wi) {
                    if (mSettings->severity.isEnabled(wi->severity) && ((tok->isC() && mSettings->standards.c >= wi->standards.c) || (tok->isCpp() && mSettings->standards.cpp >= wi->standards.cpp))) {
                        const std::string daca = mSettings->daca ? "prohibited" : "";
                        reportError(tok, wi->severity, daca + tok->str() + "Called", wi->message, CWE477, Certainty::normal);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// Check <valid>, <strz> and <not-bool>
//---------------------------------------------------------------------------
void CheckFunctions::invalidFunctionUsage()
{
    logChecker("CheckFunctions::invalidFunctionUsage");
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            tok = skipUnreachableBranch(tok);
            if (!Token::Match(tok, "%name% ( !!)"))
                continue;
            const Token * const functionToken = tok;
            const std::vector<const Token *> arguments = getArguments(tok);
            for (size_t argnr = 1; argnr <= arguments.size(); ++argnr) {
                const Token * const argtok = arguments[argnr-1];

                // check <valid>...</valid>
                const ValueFlow::Value *invalidValue = argtok->getInvalidValue(functionToken,argnr,*mSettings);
                if (invalidValue) {
                    invalidFunctionArgError(argtok, functionToken->next()->astOperand1()->expressionString(), argnr, invalidValue, mSettings->library.validarg(functionToken, argnr));
                }

                if (astIsBool(argtok)) {
                    // check <not-bool>
                    if (mSettings->library.isboolargbad(functionToken, argnr))
                        invalidFunctionArgBoolError(argtok, functionToken->str(), argnr);

                    // Are the values 0 and 1 valid?
                    else if (!mSettings->library.isIntArgValid(functionToken, argnr, 0, *mSettings))
                        invalidFunctionArgError(argtok, functionToken->str(), argnr, nullptr, mSettings->library.validarg(functionToken, argnr));
                    else if (!mSettings->library.isIntArgValid(functionToken, argnr, 1, *mSettings))
                        invalidFunctionArgError(argtok, functionToken->str(), argnr, nullptr, mSettings->library.validarg(functionToken, argnr));
                }
                // check <strz>
                if (mSettings->library.isargstrz(functionToken, argnr)) {
                    if (Token::Match(argtok, "& %var% !![") && argtok->next() && argtok->next()->valueType()) {
                        const ValueType * valueType = argtok->next()->valueType();
                        const Variable * variable = argtok->next()->variable();
                        if ((valueType->type == ValueType::Type::CHAR || valueType->type == ValueType::Type::WCHAR_T || (valueType->type == ValueType::Type::RECORD && Token::Match(argtok, "& %var% . %var% ,|)"))) &&
                            !variable->isArray() &&
                            (variable->isConst() || !variable->isGlobal()) &&
                            (!argtok->next()->hasKnownValue() || argtok->next()->getValue(0) == nullptr)) {
                            invalidFunctionArgStrError(argtok, functionToken->str(), argnr);
                        }
                    }
                    const ValueType* const valueType = argtok->valueType();
                    const Variable* const variable = argtok->variable();
                    // Is non-null terminated local variable of type char (e.g. char buf[] = {'x'};) ?
                    if (variable && variable->isLocal()
                        && valueType && (valueType->type == ValueType::Type::CHAR || valueType->type == ValueType::Type::WCHAR_T)
                        && !isVariablesChanged(variable->declEndToken(), functionToken, 0 /*indirect*/, { variable }, *mSettings)) {
                        const Token* varTok = variable->declEndToken();
                        MathLib::bigint count = -1; // Find out explicitly set count, e.g.: char buf[3] = {...}. Variable 'count' is set to 3 then.
                        if (varTok && Token::simpleMatch(varTok->astOperand1(), "["))
                        {
                            const Token* const countTok = varTok->astOperand1()->astOperand2();
                            if (countTok && countTok->hasKnownIntValue())
                                count = countTok->getKnownIntValue();
                        }
                        if (Token::simpleMatch(varTok, "= {")) {
                            varTok = varTok->tokAt(1);
                            auto charsUntilFirstZero = 0;
                            bool search = true;
                            while (search && varTok && !Token::simpleMatch(varTok->next(), "}")) {
                                varTok = varTok->next();
                                if (!Token::simpleMatch(varTok, ",")) {
                                    if (Token::Match(varTok, "%op%")) {
                                        varTok = varTok->next();
                                        continue;
                                    }
                                    ++charsUntilFirstZero;
                                    if (varTok && varTok->hasKnownIntValue() && varTok->getKnownIntValue() == 0)
                                        search=false; // stop counting for cases like char buf[3] = {'x', '\0', 'y'};
                                }
                            }
                            if (varTok && varTok->hasKnownIntValue() && varTok->getKnownIntValue() != 0
                                && (count == -1 || (count > 0 && count <= charsUntilFirstZero))) {
                                invalidFunctionArgStrError(argtok, functionToken->str(), argnr);
                            }
                        } else if (count > -1 && Token::Match(varTok, "= %str%")) {
                            const Token* strTok = varTok->getValueTokenMinStrSize(*mSettings);
                            if (strTok) {
                                const int strSize = Token::getStrArraySize(strTok);
                                if (strSize > count && strTok->str().find('\0') == std::string::npos)
                                    invalidFunctionArgStrError(argtok, functionToken->str(), argnr);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CheckFunctions::invalidFunctionArgError(const Token *tok, const std::string &functionName, int argnr, const ValueFlow::Value *invalidValue, const std::string &validstr)
{
    std::ostringstream errmsg;
    errmsg << "$symbol:" << functionName << '\n';
    if (invalidValue && invalidValue->condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(invalidValue->condition)
               << " or $symbol() argument nr " << argnr << " can have invalid value.";
    else
        errmsg << "Invalid $symbol() argument nr " << argnr << '.';
    if (invalidValue)
        errmsg << " The value is " << std::setprecision(10) << (invalidValue->isIntValue() ? invalidValue->intvalue : invalidValue->floatValue) << " but the valid values are '" << validstr << "'.";
    else
        errmsg << " The value is 0 or 1 (boolean) but the valid values are '" << validstr << "'.";
    if (invalidValue)
        reportError(getErrorPath(tok, invalidValue, "Invalid argument"),
                    invalidValue->errorSeverity() && invalidValue->isKnown() ? Severity::error : Severity::warning,
                    "invalidFunctionArg",
                    errmsg.str(),
                    CWE628,
                    invalidValue->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    else
        reportError(tok,
                    Severity::error,
                    "invalidFunctionArg",
                    errmsg.str(),
                    CWE628,
                    Certainty::normal);
}

void CheckFunctions::invalidFunctionArgBoolError(const Token *tok, const std::string &functionName, int argnr)
{
    std::ostringstream errmsg;
    errmsg << "$symbol:" << functionName << '\n';
    errmsg << "Invalid $symbol() argument nr " << argnr << ". A non-boolean value is required.";
    reportError(tok, Severity::error, "invalidFunctionArgBool", errmsg.str(), CWE628, Certainty::normal);
}

void CheckFunctions::invalidFunctionArgStrError(const Token *tok, const std::string &functionName, nonneg int argnr)
{
    std::ostringstream errmsg;
    errmsg << "$symbol:" << functionName << '\n';
    errmsg << "Invalid $symbol() argument nr " << argnr << ". A nul-terminated string is required.";
    reportError(tok, Severity::error, "invalidFunctionArgStr", errmsg.str(), CWE628, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check for ignored return values.
//---------------------------------------------------------------------------
void CheckFunctions::checkIgnoredReturnValue()
{
    if (!mSettings->severity.isEnabled(Severity::warning) &&
        !mSettings->severity.isEnabled(Severity::style) &&
        !mSettings->isPremiumEnabled("ignoredReturnValue"))
        return;

    logChecker("CheckFunctions::checkIgnoredReturnValue"); // style,warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            // skip c++11 initialization, ({...})
            if (Token::Match(tok, "%var%|(|,|return {"))
                tok = tok->linkAt(1);
            else if (Token::Match(tok, "[(<]") && tok->link())
                tok = tok->link();

            if (tok->varId() || tok->isKeyword() || tok->isStandardType() || !Token::Match(tok, "%name% ("))
                continue;

            const Token *parent = tok->next()->astParent();
            while (Token::Match(parent, "%cop%")) {
                if (Token::Match(parent, "<<|>>|*") && !parent->astParent())
                    break;
                parent = parent->astParent();
            }
            if (parent)
                continue;

            if (!tok->scope()->isExecutable()) {
                tok = tok->scope()->bodyEnd;
                continue;
            }

            if ((!tok->function() || !Token::Match(tok->function()->retDef, "void %name%")) &&
                tok->next()->astOperand1()) {
                const Library::UseRetValType retvalTy = mSettings->library.getUseRetValType(tok);
                const bool warn = (tok->function() && (tok->function()->isAttributeNodiscard() || tok->function()->isAttributePure() || tok->function()->isAttributeConst())) ||
                                  // avoid duplicate warnings for resource-allocating functions
                                  (retvalTy == Library::UseRetValType::DEFAULT && mSettings->library.getAllocFuncInfo(tok) == nullptr);
                if (mSettings->severity.isEnabled(Severity::warning) && warn)
                    ignoredReturnValueError(tok, tok->next()->astOperand1()->expressionString());
                else if (mSettings->severity.isEnabled(Severity::style) &&
                         retvalTy == Library::UseRetValType::ERROR_CODE)
                    ignoredReturnErrorCode(tok, tok->next()->astOperand1()->expressionString());
            }
        }
    }
}

void CheckFunctions::ignoredReturnValueError(const Token* tok, const std::string& function)
{
    reportError(tok, Severity::warning, "ignoredReturnValue",
                "$symbol:" + function + "\nReturn value of function $symbol() is not used.", CWE252, Certainty::normal);
}

void CheckFunctions::ignoredReturnErrorCode(const Token* tok, const std::string& function)
{
    reportError(tok, Severity::style, "ignoredReturnErrorCode",
                "$symbol:" + function + "\nError code from the return value of function $symbol() is not used.", CWE252, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check for ignored return values.
//---------------------------------------------------------------------------
static const Token *checkMissingReturnScope(const Token *tok, const Library &library);

void CheckFunctions::checkMissingReturn()
{
    logChecker("CheckFunctions::checkMissingReturn");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        const Function *function = scope->function;
        if (!function || !function->hasBody())
            continue;
        if (function->name() == "main" && !(mTokenizer->isC() && mSettings->standards.c < Standards::C99))
            continue;
        if (function->type != FunctionType::eFunction && function->type != FunctionType::eOperatorEqual)
            continue;
        if (Token::Match(function->retDef, "%name% (") && function->retDef->isUpperCaseName())
            continue;
        if (Function::returnsVoid(function, true))
            continue;
        const Token *errorToken = checkMissingReturnScope(scope->bodyEnd, mSettings->library);
        if (errorToken)
            missingReturnError(errorToken);
    }
}

static bool isForwardJump(const Token *gotoToken)
{
    if (!Token::Match(gotoToken, "goto %name% ;"))
        return false;
    for (const Token *prev = gotoToken; gotoToken; gotoToken = gotoToken->previous()) {
        if (Token::Match(prev, "%name% :") && prev->str() == gotoToken->strAt(1))
            return true;
        if (prev->str() == "{" && prev->scope()->type == ScopeType::eFunction)
            return false;
    }
    return false;
}

static const Token *checkMissingReturnScope(const Token *tok, const Library &library)
{
    const Token *lastStatement = nullptr;
    while ((tok = tok->previous()) != nullptr) {
        if (tok->str() == ")")
            tok = tok->link();
        if (tok->str() == "{")
            return lastStatement ? lastStatement : tok->next();
        if (tok->str() == "}") {
            for (const Token *prev = tok->link()->previous(); prev && prev->scope() == tok->scope() && !Token::Match(prev, "[;{}]"); prev = prev->previous()) {
                if (prev->isKeyword() && Token::Match(prev, "return|throw"))
                    return nullptr;
                if (prev->str() == "goto" && !isForwardJump(prev))
                    return nullptr;
            }
            if (tok->scope()->type == ScopeType::eSwitch) {
                bool reachable = false;
                for (const Token *switchToken = tok->link()->next(); switchToken != tok; switchToken = switchToken->next()) {
                    if (reachable && Token::simpleMatch(switchToken, "break ;")) {
                        if (Token::simpleMatch(switchToken->previous(), "}") && !checkMissingReturnScope(switchToken->previous(), library))
                            reachable = false;
                        else
                            return switchToken;
                    }
                    if (switchToken->isKeyword() && Token::Match(switchToken, "return|throw"))
                        reachable = false;
                    if (Token::Match(switchToken, "%name% (") && library.isnoreturn(switchToken))
                        reachable = false;
                    if (Token::Match(switchToken, "case|default"))
                        reachable = true;
                    else if (switchToken->str() == "{" && (switchToken->scope()->isLoopScope() || switchToken->scope()->type == ScopeType::eSwitch))
                        switchToken = switchToken->link();
                }
                if (!isExhaustiveSwitch(tok->link()))
                    return tok->link();
            } else if (tok->scope()->type == ScopeType::eIf) {
                const Token *condition = tok->scope()->classDef->next()->astOperand2();
                if (condition && condition->hasKnownIntValue() && condition->getKnownIntValue() == 1)
                    return checkMissingReturnScope(tok, library);
                return tok;
            } else if (tok->scope()->type == ScopeType::eElse) {
                const Token *errorToken = checkMissingReturnScope(tok, library);
                if (errorToken)
                    return errorToken;
                tok = tok->link();
                if (Token::simpleMatch(tok->tokAt(-2), "} else {"))
                    return checkMissingReturnScope(tok->tokAt(-2), library);
                return tok;
            } else if (tok->scope()->type == ScopeType::eCatch) {
                while (tok->str() == "}") {
                    const Token *errorToken = checkMissingReturnScope(tok, library);
                    if (errorToken || tok->scope()->type == ScopeType::eTry)
                        return errorToken;
                    tok = tok->link();
                    if (Token::simpleMatch(tok->previous(), ") {") && Token::simpleMatch(tok->linkAt(-1)->tokAt(-2), "} catch ("))
                        tok = tok->linkAt(-1)->tokAt(-2);
                    else
                        break;
                }
                // FIXME this should not be reached
                return nullptr;
            }
            // FIXME
            return nullptr;
        }
        if (tok->isKeyword() && Token::Match(tok, "return|throw"))
            return nullptr;
        if (tok->str() == "goto" && !isForwardJump(tok))
            return nullptr;
        if (Token::Match(tok, "%name% (") && library.isnoreturn(tok)) {
            return nullptr;
        }
        if (Token::Match(tok->previous(), "[;{}] %name% (") && !library.isnotnoreturn(tok)) {
            return nullptr;
        }
        if (Token::Match(tok, "[;{}] %name% :"))
            return tok;
        if (Token::Match(tok, "; !!}") && !lastStatement)
            lastStatement = tok->next();
    }
    return nullptr;
}

void CheckFunctions::missingReturnError(const Token* tok)
{
    reportError(tok, Severity::error, "missingReturn",
                "Found an exit path from function with non-void return type that has missing return statement", CWE758, Certainty::normal);
}
//---------------------------------------------------------------------------
// Detect passing wrong values to <cmath> functions like atan(0, x);
//---------------------------------------------------------------------------
void CheckFunctions::checkMathFunctions()
{
    const bool styleC99 = mSettings->severity.isEnabled(Severity::style) && ((mTokenizer->isC() && mSettings->standards.c != Standards::C89) || (mTokenizer->isCPP() && mSettings->standards.cpp != Standards::CPP03));
    const bool printWarnings = mSettings->severity.isEnabled(Severity::warning);

    if (!styleC99 && !printWarnings && !mSettings->isPremiumEnabled("wrongmathcall"))
        return;

    logChecker("CheckFunctions::checkMathFunctions"); // style,warning,c99,c++11

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->varId())
                continue;
            if (printWarnings && Token::Match(tok, "%name% ( !!)")) {
                if (tok->strAt(-1) != "."
                    && Token::Match(tok, "log|logf|logl|log10|log10f|log10l|log2|log2f|log2l ( %num% )")) {
                    const std::string& number = tok->strAt(2);
                    if ((MathLib::isInt(number) && MathLib::toBigNumber(number) <= 0) ||
                        (MathLib::isFloat(number) && MathLib::toDoubleNumber(number) <= 0.))
                        mathfunctionCallWarning(tok);
                } else if (Token::Match(tok, "log1p|log1pf|log1pl ( %num% )")) {
                    const std::string& number = tok->strAt(2);
                    if ((MathLib::isInt(number) && MathLib::toBigNumber(number) <= -1) ||
                        (MathLib::isFloat(number) && MathLib::toDoubleNumber(number) <= -1.))
                        mathfunctionCallWarning(tok);
                }
                // atan2 ( x , y): x and y can not be zero, because this is mathematically not defined
                else if (Token::Match(tok, "atan2|atan2f|atan2l ( %num% , %num% )")) {
                    if (MathLib::isNullValue(tok->strAt(2)) && MathLib::isNullValue(tok->strAt(4)))
                        mathfunctionCallWarning(tok, 2);
                }
                // fmod ( x , y) If y is zero, then either a range error will occur or the function will return zero (implementation-defined).
                else if (Token::Match(tok, "fmod|fmodf|fmodl (")) {
                    const Token* nextArg = tok->tokAt(2)->nextArgument();
                    if (nextArg && MathLib::isNullValue(nextArg->str()))
                        mathfunctionCallWarning(tok, 2);
                }
                // pow ( x , y) If x is zero, and y is negative --> division by zero
                else if (Token::Match(tok, "pow|powf|powl ( %num% , %num% )")) {
                    if (MathLib::isNullValue(tok->strAt(2)) && MathLib::isNegative(tok->strAt(4)))
                        mathfunctionCallWarning(tok, 2);
                }
            }

            if (styleC99) {
                if (Token::Match(tok, "%num% - erf (") && Tokenizer::isOneNumber(tok->str()) && tok->next()->astOperand2() == tok->tokAt(3)) {
                    mathfunctionCallWarning(tok, "1 - erf(x)", "erfc(x)");
                } else if (Token::simpleMatch(tok, "exp (") && Token::Match(tok->linkAt(1), ") - %num%") && Tokenizer::isOneNumber(tok->linkAt(1)->strAt(2)) && tok->linkAt(1)->next()->astOperand1() == tok->next()) {
                    mathfunctionCallWarning(tok, "exp(x) - 1", "expm1(x)");
                } else if (Token::simpleMatch(tok, "log (") && tok->next()->astOperand2()) {
                    const Token* plus = tok->next()->astOperand2();
                    if (plus->str() == "+" && ((plus->astOperand1() && Tokenizer::isOneNumber(plus->astOperand1()->str())) || (plus->astOperand2() && Tokenizer::isOneNumber(plus->astOperand2()->str()))))
                        mathfunctionCallWarning(tok, "log(1 + x)", "log1p(x)");
                }
            }
        }
    }
}

void CheckFunctions::mathfunctionCallWarning(const Token *tok, const nonneg int numParam)
{
    if (tok) {
        if (numParam == 1)
            reportError(tok, Severity::warning, "wrongmathcall", "$symbol:" + tok->str() + "\nPassing value " + tok->strAt(2) + " to $symbol() leads to implementation-defined result.", CWE758, Certainty::normal);
        else if (numParam == 2)
            reportError(tok, Severity::warning, "wrongmathcall", "$symbol:" + tok->str() + "\nPassing values " + tok->strAt(2) + " and " + tok->strAt(4) + " to $symbol() leads to implementation-defined result.", CWE758, Certainty::normal);
    } else
        reportError(tok, Severity::warning, "wrongmathcall", "Passing value '#' to #() leads to implementation-defined result.", CWE758, Certainty::normal);
}

void CheckFunctions::mathfunctionCallWarning(const Token *tok, const std::string& oldexp, const std::string& newexp)
{
    reportError(tok, Severity::style, "unpreciseMathCall", "Expression '" + oldexp + "' can be replaced by '" + newexp + "' to avoid loss of precision.", CWE758, Certainty::normal);
}

//---------------------------------------------------------------------------
// memset(p, y, 0 /* bytes to fill */) <- 2nd and 3rd arguments inverted
//---------------------------------------------------------------------------
void CheckFunctions::memsetZeroBytes()
{
// FIXME:
//  Replace this with library configuration.
//  For instance:
//     <arg nr="3">
//       <warn knownIntValue="0" severity="warning" msg="..."/>
//     </arg>

    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckFunctions::memsetZeroBytes"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|wmemset (") && (numberOfArguments(tok)==3)) {
                const std::vector<const Token *> &arguments = getArguments(tok);
                if (WRONG_DATA(arguments.size() != 3U, tok))
                    continue;
                const Token* lastParamTok = arguments[2];
                if (MathLib::isNullValue(lastParamTok->str()))
                    memsetZeroBytesError(tok);
            }
        }
    }
}

void CheckFunctions::memsetZeroBytesError(const Token *tok)
{
    const std::string summary("memset() called to fill 0 bytes.");
    const std::string verbose(summary + " The second and third arguments might be inverted."
                              " The function memset ( void * ptr, int value, size_t num ) sets the"
                              " first num bytes of the block of memory pointed by ptr to the specified value.");
    reportError(tok, Severity::warning, "memsetZeroBytes", summary + "\n" + verbose, CWE687, Certainty::normal);
}

void CheckFunctions::memsetInvalid2ndParam()
{
// FIXME:
//  Replace this with library configuration.
//  For instance:
//     <arg nr="2">
//       <not-float/>
//       <warn possibleIntValue=":-129,256:" severity="warning" msg="..."/>
//     </arg>

    const bool printPortability = mSettings->severity.isEnabled(Severity::portability);
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    if (!printWarning && !printPortability)
        return;

    logChecker("CheckFunctions::memsetInvalid2ndParam"); // warning,portability

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok && (tok != scope->bodyEnd); tok = tok->next()) {
            if (!Token::simpleMatch(tok, "memset ("))
                continue;

            const std::vector<const Token *> args = getArguments(tok);
            if (args.size() != 3)
                continue;

            // Second parameter is zero literal, i.e. 0.0f
            const Token * const secondParamTok = args[1];
            if (Token::Match(secondParamTok, "%num% ,") && MathLib::isNullValue(secondParamTok->str()))
                continue;

            // Check if second parameter is a float variable or a float literal != 0.0f
            if (printPortability && astIsFloat(secondParamTok,false)) {
                memsetFloatError(secondParamTok, secondParamTok->expressionString());
            }

            if (printWarning && secondParamTok->isNumber()) { // Check if the second parameter is a literal and is out of range
                const MathLib::bigint value = MathLib::toBigNumber(secondParamTok);
                const long long sCharMin = mSettings->platform.signedCharMin();
                const long long uCharMax = mSettings->platform.unsignedCharMax();
                if (value < sCharMin || value > uCharMax)
                    memsetValueOutOfRangeError(secondParamTok, secondParamTok->str());
            }
        }
    }
}

void CheckFunctions::memsetFloatError(const Token *tok, const std::string &var_value)
{
    const std::string message("The 2nd memset() argument '" + var_value +
                              "' is a float, its representation is implementation defined.");
    const std::string verbose(message + " memset() is used to set each byte of a block of memory to a specific value and"
                              " the actual representation of a floating-point value is implementation defined.");
    reportError(tok, Severity::portability, "memsetFloat", message + "\n" + verbose, CWE688, Certainty::normal);
}

void CheckFunctions::memsetValueOutOfRangeError(const Token *tok, const std::string &value)
{
    const std::string message("The 2nd memset() argument '" + value + "' doesn't fit into an 'unsigned char'.");
    const std::string verbose(message + " The 2nd parameter is passed as an 'int', but the function fills the block of memory using the 'unsigned char' conversion of this value.");
    reportError(tok, Severity::warning, "memsetValueOutOfRange", message + "\n" + verbose, CWE686, Certainty::normal);
}

//---------------------------------------------------------------------------
// --check-library => warn for unconfigured functions
//---------------------------------------------------------------------------

void CheckFunctions::checkLibraryMatchFunctions()
{
    if (!mSettings->checkLibrary)
        return;

    bool insideNew = false;
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->scope() || !tok->scope()->isExecutable())
            continue;

        // skip uninstantiated templates
        if (tok == tok->scope()->bodyStart && tok->scope()->function && tok->scope()->function->templateDef) {
            tok = tok->link();
            continue;
        }

        if (tok->str() == "new")
            insideNew = true;
        else if (tok->str() == ";")
            insideNew = false;
        else if (insideNew)
            continue;

        if (tok->isKeyword() || !Token::Match(tok, "%name% ("))
            continue;

        if (tok->varId() != 0 || tok->type() || tok->isStandardType())
            continue;

        if (tok->linkAt(1)->strAt(1) == "(")
            continue;

        if (tok->function())
            continue;

        if (Token::simpleMatch(tok->astTop(), "throw"))
            continue;

        if (Token::simpleMatch(tok->astParent(), ".")) {
            const Token* contTok = tok->astParent()->astOperand1();
            if (astContainerAction(contTok, mSettings->library) != Library::Container::Action::NO_ACTION)
                continue;
            if (astContainerYield(contTok, mSettings->library) != Library::Container::Yield::NO_YIELD)
                continue;
        }

        if (!mSettings->library.isNotLibraryFunction(tok))
            continue;

        const std::string &functionName = mSettings->library.getFunctionName(tok);
        if (functionName.empty())
            continue;

        if (mSettings->library.functions().find(functionName) != mSettings->library.functions().end())
            continue;

        if (mSettings->library.podtype(tok->expressionString()))
            continue;

        if (mSettings->library.getTypeCheck("unusedvar", functionName) != Library::TypeCheck::def)
            continue;

        const Token* start = tok;
        while (Token::Match(start->tokAt(-2), "%name% ::") && !start->tokAt(-2)->isKeyword())
            start = start->tokAt(-2);
        if (mSettings->library.detectContainerOrIterator(start))
            continue;

        reportError(tok,
                    Severity::information,
                    "checkLibraryFunction",
                    "--check-library: There is no matching configuration for function " + functionName + "()");
    }
}

// Check for problems to compiler apply (Named) Return Value Optimization for local variable
// Technically we have different guarantees between standard versions
// details: https://en.cppreference.com/w/cpp/language/copy_elision
void CheckFunctions::returnLocalStdMove()
{
    if (!mTokenizer->isCPP() || mSettings->standards.cpp < Standards::CPP11)
        return;

    if (!mSettings->severity.isEnabled(Severity::performance))
        return;

    logChecker("CheckFunctions::returnLocalStdMove"); // performance,c++11

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        // Expect return by-value
        if (Function::returnsReference(scope->function, /*unknown*/ true, /*includeRValueRef*/ true))
            continue;
        const auto rets = Function::findReturns(scope->function);
        for (const Token* ret : rets) {
            if (!Token::simpleMatch(ret->tokAt(-3), "std :: move ("))
                continue;
            const Token* retval = ret->astOperand2();
            // NRVO
            if (retval->variable() && retval->variable()->isLocal() && !retval->variable()->isVolatile())
                copyElisionError(retval);
            // RVO
            if (Token::Match(retval, "(|{") && !retval->isCast() && !(retval->valueType() && retval->valueType()->reference != Reference::None))
                copyElisionError(retval);
        }
    }
}

void CheckFunctions::copyElisionError(const Token *tok)
{
    reportError(tok,
                Severity::performance,
                "returnStdMoveLocal",
                "Using std::move for returning object by-value from function will affect copy elision optimization."
                " More: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-return-move-local");
}

void CheckFunctions::useStandardLibrary()
{
    if (!mSettings->severity.isEnabled(Severity::style))
        return;

    logChecker("CheckFunctions::useStandardLibrary"); // style

    for (const Scope& scope: mTokenizer->getSymbolDatabase()->scopeList) {
        if (scope.type != ScopeType::eFor)
            continue;

        const Token *forToken = scope.classDef;
        // for ( initToken ; condToken ; stepToken )
        const Token* initToken = getInitTok(forToken);
        if (!initToken)
            continue;
        const Token* condToken = getCondTok(forToken);
        if (!condToken)
            continue;
        const Token* stepToken = getStepTok(forToken);
        if (!stepToken)
            continue;

        // 1. we expect that idx variable will be initialized with 0
        const Token* idxToken = initToken->astOperand1();
        const Token* initVal = initToken->astOperand2();
        if (!idxToken || !initVal || !initVal->hasKnownIntValue() || initVal->getKnownIntValue() != 0)
            continue;
        const auto idxVarId = idxToken->varId();
        if (0 == idxVarId)
            continue;

        // 2. we expect that idx will be less of some variable
        if (!condToken->isComparisonOp())
            continue;

        const auto& secondOp = condToken->str();
        const bool isLess = "<" == secondOp &&
                            isConstExpression(condToken->astOperand2(), mSettings->library) &&
                            condToken->astOperand1()->varId() == idxVarId;
        const bool isMore = ">" == secondOp &&
                            isConstExpression(condToken->astOperand1(), mSettings->library) &&
                            condToken->astOperand2()->varId() == idxVarId;

        if (!(isLess || isMore))
            continue;

        // 3. we expect idx incrementing by 1
        const bool inc = stepToken->str() == "++" && stepToken->astOperand1() && stepToken->astOperand1()->varId() == idxVarId;
        const bool plusOne = stepToken->isBinaryOp() && stepToken->str() == "+=" &&
                             stepToken->astOperand1()->varId() == idxVarId &&
                             stepToken->astOperand2()->str() == "1";
        if (!inc && !plusOne)
            continue;

        // technically using void* here is not correct but some compilers could allow it

        const Token *tok = scope.bodyStart;
        const std::string memcpyName = tok->isCpp() ? "std::memcpy" : "memcpy";
        // (reinterpret_cast<uint8_t*>(dest))[i] = (reinterpret_cast<const uint8_t*>(src))[i];
        if (Token::Match(tok, "{ (| reinterpret_cast < uint8_t|int8_t|char|void * > ( %var% ) )| [ %varid% ] = "
                         "(| reinterpret_cast < const| uint8_t|int8_t|char|void * > ( %var% ) )| [ %varid% ] ; }", idxVarId)) {
            useStandardLibraryError(tok->next(), memcpyName);
            continue;
        }

        // ((char*)dst)[i] = ((const char*)src)[i];
        if (Token::Match(tok, "{ ( ( uint8_t|int8_t|char|void * ) (| %var% ) )| [ %varid% ] = "
                         "( ( const| uint8_t|int8_t|char|void * ) (| %var% ) )| [ %varid% ] ; }", idxVarId)) {
            useStandardLibraryError(tok->next(), memcpyName);
            continue;
        }


        const static std::string memsetName = tok->isCpp() ? "std::memset" : "memset";
        // ((char*)dst)[i] = 0;
        if (Token::Match(tok, "{ ( ( uint8_t|int8_t|char|void * ) (| %var% ) )| [ %varid% ] = %char%|%num% ; }", idxVarId)) {
            useStandardLibraryError(tok->next(), memsetName);
            continue;
        }

        // ((char*)dst)[i] = (const char*)0;
        if (Token::Match(tok, "{ ( ( uint8_t|int8_t|char|void * ) (| %var% ) )| [ %varid% ] = "
                         "( const| uint8_t|int8_t|char ) (| %char%|%num% )| ; }", idxVarId)) {
            useStandardLibraryError(tok->next(), memsetName);
            continue;
        }

        // (reinterpret_cast<uint8_t*>(dest))[i] = static_cast<const uint8_t>(0);
        if (Token::Match(tok, "{ (| reinterpret_cast < uint8_t|int8_t|char|void * > ( %var% ) )| [ %varid% ] = "
                         "(| static_cast < const| uint8_t|int8_t|char > ( %char%|%num% ) )| ; }", idxVarId)) {
            useStandardLibraryError(tok->next(), memsetName);
            continue;
        }

        // (reinterpret_cast<int8_t*>(dest))[i] = 0;
        if (Token::Match(tok, "{ (| reinterpret_cast < uint8_t|int8_t|char|void * > ( %var% ) )| [ %varid% ] = "
                         "%char%|%num% ; }", idxVarId)) {
            useStandardLibraryError(tok->next(), memsetName);
            continue;
        }
    }
}

void CheckFunctions::useStandardLibraryError(const Token *tok, const std::string& expected)
{
    reportError(tok, Severity::style,
                "useStandardLibrary",
                "Consider using " + expected + " instead of loop.");
}

void CheckFunctions::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckFunctions checkFunctions(&tokenizer, &tokenizer.getSettings(), errorLogger);

    checkFunctions.checkIgnoredReturnValue();
    checkFunctions.checkMissingReturn();  // Missing "return" in exit path

    // --check-library : functions with nonmatching configuration
    checkFunctions.checkLibraryMatchFunctions();

    checkFunctions.checkProhibitedFunctions();
    checkFunctions.invalidFunctionUsage();
    checkFunctions.checkMathFunctions();
    checkFunctions.memsetZeroBytes();
    checkFunctions.memsetInvalid2ndParam();
    checkFunctions.returnLocalStdMove();
    checkFunctions.useStandardLibrary();
}

void CheckFunctions::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckFunctions c(nullptr, settings, errorLogger);

    for (auto i = settings->library.functionwarn().cbegin(); i != settings->library.functionwarn().cend(); ++i) {
        c.reportError(nullptr, Severity::style, i->first+"Called", i->second.message);
    }

    c.invalidFunctionArgError(nullptr, "func_name", 1, nullptr,"1:4");
    c.invalidFunctionArgBoolError(nullptr, "func_name", 1);
    c.invalidFunctionArgStrError(nullptr, "func_name", 1);
    c.ignoredReturnValueError(nullptr, "malloc");
    c.ignoredReturnErrorCode(nullptr, "func_name");
    c.mathfunctionCallWarning(nullptr);
    c.mathfunctionCallWarning(nullptr, "1 - erf(x)", "erfc(x)");
    c.memsetZeroBytesError(nullptr);
    c.memsetFloatError(nullptr,  "varname");
    c.memsetValueOutOfRangeError(nullptr,  "varname");
    c.missingReturnError(nullptr);
    c.copyElisionError(nullptr);
    c.useStandardLibraryError(nullptr, "memcpy");
    // TODO: allocaCalled, <funcname>Called
}
