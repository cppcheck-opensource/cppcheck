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
// 64-bit portability
//---------------------------------------------------------------------------

#include "check64bit.h"

#include "errortypes.h"
#include "platform.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

#include <vector>

//---------------------------------------------------------------------------

// CWE ids used
static const CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior

// Register this check class (by creating a static instance of it)
namespace {
    Check64BitPortability instance;
}

static bool is32BitIntegerReturn(const Function* func, const Settings* settings)
{
    if (settings->platform.sizeof_pointer != 8)
        return false;
    const ValueType* vt = func->arg->valueType();
    return vt && vt->pointer == 0 && vt->isIntegral() && vt->getSizeOf(*settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer) == 4;
}

static bool isFunctionPointer(const Token* tok)
{
    if (!tok || !tok->variable())
        return false;
    return Tokenizer::isFunctionPointer(tok->variable()->nameToken());
}

void Check64BitPortability::pointerassignment()
{
    if (!mSettings->severity.isEnabled(Severity::portability))
        return;

    logChecker("Check64BitPortability::pointerassignment"); // portability

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // Check return values
    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (scope->function == nullptr || !scope->function->hasBody()) // We only look for functions with a body
            continue;

        bool retPointer = false;
        if (scope->function->token->strAt(-1) == "*") // Function returns a pointer
            retPointer = true;
        else if (is32BitIntegerReturn(scope->function, mSettings))
            ;
        else
            continue;

        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            // skip nested functions
            if (tok->str() == "{") {
                if (tok->scope()->type == ScopeType::eFunction || tok->scope()->type == ScopeType::eLambda)
                    tok = tok->link();
            }

            if (tok->str() != "return")
                continue;

            if (!tok->astOperand1() || tok->astOperand1()->isNumber())
                continue;

            const ValueType * const returnType = tok->astOperand1()->valueType();
            if (!returnType)
                continue;

            if (retPointer && !returnType->typeScope && returnType->pointer == 0U)
                returnIntegerError(tok);

            if (!retPointer) {
                bool warn = returnType->pointer >= 1U;
                if (!warn) {
                    const Token* tok2 = tok->astOperand1();
                    while (tok2 && tok2->isCast())
                        tok2 = tok2->astOperand2() ? tok2->astOperand2() : tok2->astOperand1();
                    warn = tok2 && tok2->valueType() && tok2->valueType()->pointer;
                }
                if (warn)
                    returnPointerError(tok);
            }
        }
    }

    // Check assignments
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->str() != "=")
                continue;

            const ValueType *lhstype = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
            const ValueType *rhstype = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;
            if (!lhstype || !rhstype)
                continue;

            // Assign integer to pointer..
            if (lhstype->pointer >= 1U &&
                !tok->astOperand2()->isNumber() &&
                rhstype->pointer == 0U &&
                rhstype->originalTypeName.empty() &&
                rhstype->type == ValueType::Type::INT &&
                !isFunctionPointer(tok->astOperand1()))
                assignmentIntegerToAddressError(tok);

            // Assign pointer to integer..
            if (rhstype->pointer >= 1U &&
                lhstype->pointer == 0U &&
                lhstype->originalTypeName.empty() &&
                lhstype->isIntegral() &&
                lhstype->type >= ValueType::Type::CHAR &&
                lhstype->type <= ValueType::Type::INT)
                assignmentAddressToIntegerError(tok);
        }
    }
}

void Check64BitPortability::assignmentAddressToIntegerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentAddressToInteger",
                "Assigning a pointer to an integer is not portable.\n"
                "Assigning a pointer to an integer (int/long/etc) is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit address to 32-bit integer. The safe "
                "way is to store addresses only in pointer types (or typedefs like uintptr_t).", CWE758, Certainty::normal);
}

void Check64BitPortability::assignmentIntegerToAddressError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentIntegerToAddress",
                "Assigning an integer to a pointer is not portable.\n"
                "Assigning an integer (int/long/etc) to a pointer is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit integer to 32-bit pointer. The safe "
                "way is to store addresses only in pointer types (or typedefs like uintptr_t).", CWE758, Certainty::normal);
}

void Check64BitPortability::returnPointerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "CastAddressToIntegerAtReturn",
                "Returning an address value in a function with integer return type is not portable.\n"
                "Returning an address value in a function with integer (int/long/etc) return type is not portable across "
                "different platforms and compilers. For example in 32-bit Windows and Linux they are same width, but in "
                "64-bit Windows and Linux they are of different width. In worst case you end up casting 64-bit address down "
                "to 32-bit integer. The safe way is to return a type such as intptr_t.", CWE758, Certainty::normal);
}

void Check64BitPortability::returnIntegerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "CastIntegerToAddressAtReturn",
                "Returning an integer in a function with pointer return type is not portable.\n"
                "Returning an integer (int/long/etc) in a function with pointer return type is not portable across different "
                "platforms and compilers. For example in 32-bit Windows and Linux they are same width, but in 64-bit Windows "
                "and Linux they are of different width. In worst case you end up casting 64-bit integer down to 32-bit pointer. "
                "The safe way is to always return a pointer.", CWE758, Certainty::normal);
}

void Check64BitPortability::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    Check64BitPortability check64BitPortability(&tokenizer, &tokenizer.getSettings(), errorLogger);
    check64BitPortability.pointerassignment();
}

void Check64BitPortability::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    Check64BitPortability c(nullptr, settings, errorLogger);
    c.assignmentAddressToIntegerError(nullptr);
    c.assignmentIntegerToAddressError(nullptr);
    c.returnIntegerError(nullptr);
    c.returnPointerError(nullptr);
}
