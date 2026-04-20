/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "checkvaarg.h"

#include "astutils.h"
#include "errortypes.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

#include <cstddef>
#include <iterator>
#include <list>
#include <vector>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckVaarg instance;
}


//---------------------------------------------------------------------------
// Ensure that correct parameter is passed to va_start()
//---------------------------------------------------------------------------

// CWE ids used:
static const CWE CWE664(664U);   // Improper Control of a Resource Through its Lifetime
static const CWE CWE688(688U);   // Function Call With Incorrect Variable or Reference as Argument
static const CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior

void CheckVaarg::va_start_argument()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    const bool printWarnings = mSettings->severity.isEnabled(Severity::warning);

    logChecker("CheckVaarg::va_start_argument");

    for (std::size_t i = 0; i < functions; ++i) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        const Function* function = scope->function;
        if (!function)
            continue;
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->scope()->isExecutable())
                tok = tok->scope()->bodyEnd;
            else if (Token::simpleMatch(tok, "va_start (")) {
                const Token* param2 = tok->tokAt(2)->nextArgument();
                if (!param2)
                    continue;
                const Variable* var = param2->variable();
                if (var && var->isReference())
                    referenceAs_va_start_error(param2, var->name());
                if (var && var->index() + 2 < function->argCount() && printWarnings) {
                    auto it = function->argumentList.end();
                    std::advance(it, -2);
                    wrongParameterTo_va_start_error(tok, var->name(), it->name());
                }
                tok = tok->linkAt(1);
            }
        }
    }
}

void CheckVaarg::wrongParameterTo_va_start_error(const Token *tok, const std::string& paramIsName, const std::string& paramShouldName)
{
    reportError(tok, Severity::warning,
                "va_start_wrongParameter", "'" + paramIsName + "' given to va_start() is not last named argument of the function. Did you intend to pass '" + paramShouldName + "'?", CWE688, Certainty::normal);
}

void CheckVaarg::referenceAs_va_start_error(const Token *tok, const std::string& paramName)
{
    reportError(tok, Severity::error,
                "va_start_referencePassed", "Using reference '" + paramName + "' as parameter for va_start() results in undefined behaviour.", CWE758, Certainty::normal);
}

//---------------------------------------------------------------------------
// Detect missing va_end() if va_start() was used
// Detect va_list usage after va_end()
//---------------------------------------------------------------------------

void CheckVaarg::va_list_usage()
{
    if (mSettings->clang)
        return;

    logChecker("CheckVaarg::va_list_usage"); // notclang

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || var->isPointer() || var->isReference() || var->isArray() || !var->scope() || var->typeStartToken()->str() != "va_list")
            continue;
        if (!var->isLocal() && !var->isArgument()) // Check only local variables and arguments
            continue;

        bool open = var->isArgument(); // va_list passed as argument are opened
        bool exitOnEndOfStatement = false;

        const Token* tok = var->nameToken()->next();
        for (; tok && tok != var->scope()->bodyEnd; tok = tok->next()) {
            // Skip lambdas
            const Token* tok2 = findLambdaEndToken(tok);
            if (tok2)
                tok = tok2;
            if (Token::Match(tok, "va_start ( %varid%", var->declarationId())) {
                if (open)
                    va_start_subsequentCallsError(tok, var->name());
                open = true;
                tok = tok->linkAt(1);
            } else if (Token::Match(tok, "va_end ( %varid%", var->declarationId())) {
                if (!open)
                    va_list_usedBeforeStartedError(tok, var->name());
                open = false;
                tok = tok->linkAt(1);
            } else if (Token::simpleMatch(tok, "va_copy (")) {
                bool nopen = open;
                if (tok->linkAt(1)->previous()->varId() == var->declarationId()) { // Source
                    if (!open)
                        va_list_usedBeforeStartedError(tok, var->name());
                }
                if (tok->tokAt(2)->varId() == var->declarationId()) { // Destination
                    if (open)
                        va_start_subsequentCallsError(tok, var->name());
                    nopen = true;
                }
                open = nopen;
                tok = tok->linkAt(1);
            } else if (Token::Match(tok, "throw|return"))
                exitOnEndOfStatement = true;
            else if (tok->str() == "break") {
                tok = findNextTokenFromBreak(tok);
                if (!tok)
                    return;
            } else if (tok->str() == "goto" || (tok->isCpp() && tok->str() == "try")) {
                open = false;
                break;
            } else if (!open && tok->varId() == var->declarationId())
                va_list_usedBeforeStartedError(tok, var->name());
            else if (exitOnEndOfStatement && tok->str() == ";")
                break;
        }
        if (open && !var->isArgument())
            va_end_missingError(tok, var->name());
    }
}

void CheckVaarg::va_end_missingError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "va_end_missing", "va_list '" + varname + "' was opened but not closed by va_end().", CWE664, Certainty::normal);
}

void CheckVaarg::va_list_usedBeforeStartedError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "va_list_usedBeforeStarted", "va_list '" + varname + "' used before va_start() was called.", CWE664, Certainty::normal);
}

void CheckVaarg::va_start_subsequentCallsError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "va_start_subsequentCalls", "va_start() or va_copy() called subsequently on '" + varname + "' without va_end() in between.", CWE664, Certainty::normal);
}

void CheckVaarg::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckVaarg check(&tokenizer, &tokenizer.getSettings(), errorLogger);
    check.va_start_argument();
    check.va_list_usage();
}

void CheckVaarg::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckVaarg c(nullptr, settings, errorLogger);
    c.wrongParameterTo_va_start_error(nullptr, "arg1", "arg2");
    c.referenceAs_va_start_error(nullptr, "arg1");
    c.va_end_missingError(nullptr, "vl");
    c.va_list_usedBeforeStartedError(nullptr, "vl");
    c.va_start_subsequentCallsError(nullptr, "vl");
}
