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
#include "checkother.h"

#include "astutils.h"
#include "fwdanalysis.h"
#include "library.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm> // find_if()
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckOther instance;
}

static const CWE CWE128(128U);   // Wrap-around Error
static const CWE CWE131(131U);   // Incorrect Calculation of Buffer Size
static const CWE CWE197(197U);   // Numeric Truncation Error
static const CWE CWE362(362U);   // Concurrent Execution using Shared Resource with Improper Synchronization ('Race Condition')
static const CWE CWE369(369U);   // Divide By Zero
static const CWE CWE398(398U);   // Indicator of Poor Code Quality
static const CWE CWE475(475U);   // Undefined Behavior for Input to API
static const CWE CWE561(561U);   // Dead Code
static const CWE CWE563(563U);   // Assignment to Variable without Use ('Unused Variable')
static const CWE CWE570(570U);   // Expression is Always False
static const CWE CWE571(571U);   // Expression is Always True
static const CWE CWE672(672U);   // Operation on a Resource after Expiration or Release
static const CWE CWE628(628U);   // Function Call with Incorrectly Specified Arguments
static const CWE CWE683(683U);   // Function Call With Incorrect Order of Arguments
static const CWE CWE704(704U);   // Incorrect Type Conversion or Cast
static const CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE768(768U);   // Incorrect Short Circuit Evaluation
static const CWE CWE783(783U);   // Operator Precedence Logic Error

//----------------------------------------------------------------------------------
// The return value of fgetc(), getc(), ungetc(), getchar() etc. is an integer value.
// If this return value is stored in a character variable and then compared
// to EOF, which is an integer, the comparison maybe be false.
//
// Reference:
// - Ticket #160
// - http://www.cplusplus.com/reference/cstdio/fgetc/
// - http://www.cplusplus.com/reference/cstdio/getc/
// - http://www.cplusplus.com/reference/cstdio/getchar/
// - http://www.cplusplus.com/reference/cstdio/ungetc/ ...
//----------------------------------------------------------------------------------
void CheckOther::checkCastIntToCharAndBack()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckOther::checkCastIntToCharAndBack"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        std::map<int, std::string> vars;
        for (const Token* tok = scope->bodyStart->next(); tok && tok != scope->bodyEnd; tok = tok->next()) {
            // Quick check to see if any of the matches below have any chances
            if (!Token::Match(tok, "%var%|EOF %comp%|="))
                continue;
            if (Token::Match(tok, "%var% = fclose|fflush|fputc|fputs|fscanf|getchar|getc|fgetc|putchar|putc|puts|scanf|sscanf|ungetc (")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    vars[tok->varId()] = tok->strAt(2);
                }
            } else if (Token::Match(tok, "EOF %comp% ( %var% = fclose|fflush|fputc|fputs|fscanf|getchar|getc|fgetc|putchar|putc|puts|scanf|sscanf|ungetc (")) {
                tok = tok->tokAt(3);
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    checkCastIntToCharAndBackError(tok, tok->strAt(2));
                }
            } else if (tok->isCpp() && (Token::Match(tok, "EOF %comp% ( %var% = std :: cin . get (") || Token::Match(tok, "EOF %comp% ( %var% = cin . get ("))) {
                tok = tok->tokAt(3);
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    checkCastIntToCharAndBackError(tok, "cin.get");
                }
            } else if (tok->isCpp() && (Token::Match(tok, "%var% = std :: cin . get (") || Token::Match(tok, "%var% = cin . get ("))) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "char" && !var->typeEndToken()->isSigned()) {
                    vars[tok->varId()] = "cin.get";
                }
            } else if (Token::Match(tok, "%var% %comp% EOF")) {
                if (vars.find(tok->varId()) != vars.end()) {
                    checkCastIntToCharAndBackError(tok, vars[tok->varId()]);
                }
            } else if (Token::Match(tok, "EOF %comp% %var%")) {
                tok = tok->tokAt(2);
                if (vars.find(tok->varId()) != vars.end()) {
                    checkCastIntToCharAndBackError(tok, vars[tok->varId()]);
                }
            }
        }
    }
}

void CheckOther::checkCastIntToCharAndBackError(const Token *tok, const std::string &strFunctionName)
{
    reportError(
        tok,
        Severity::warning,
        "checkCastIntToCharAndBack",
        "$symbol:" + strFunctionName + "\n"
        "Storing $symbol() return value in char variable and then comparing with EOF.\n"
        "When saving $symbol() return value in char variable there is loss of precision. "
        " When $symbol() returns EOF this value is truncated. Comparing the char "
        "variable with EOF can have unexpected results. For instance a loop \"while (EOF != (c = $symbol());\" "
        "loops forever on some compilers/platforms and on other compilers/platforms it will stop "
        "when the file contains a matching character.", CWE197, Certainty::normal
        );
}


//---------------------------------------------------------------------------
// Clarify calculation precedence for ternary operators.
//---------------------------------------------------------------------------
void CheckOther::clarifyCalculation()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("clarifyCalculation"))
        return;

    logChecker("CheckOther::clarifyCalculation"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            // ? operator where lhs is arithmetical expression
            if (tok->str() != "?" || !tok->astOperand1() || !tok->astOperand1()->isCalculation())
                continue;
            if (!tok->astOperand1()->isArithmeticalOp() && tok->astOperand1()->tokType() != Token::eBitOp)
                continue;

            // non-pointer calculation in lhs and pointer in rhs => no clarification is needed
            if (tok->astOperand1()->isBinaryOp() && Token::Match(tok->astOperand1(), "%or%|&|%|*|/") && tok->astOperand2()->valueType() && tok->astOperand2()->valueType()->pointer > 0)
                continue;

            // bit operation in lhs and char literals in rhs => probably no mistake
            if (tok->astOperand1()->tokType() == Token::eBitOp && Token::Match(tok->astOperand2()->astOperand1(), "%char%") && Token::Match(tok->astOperand2()->astOperand2(), "%char%"))
                continue;

            // 2nd operand in lhs has known integer value => probably no mistake
            if (tok->astOperand1()->isBinaryOp() && tok->astOperand1()->astOperand2()->hasKnownIntValue()) {
                const Token *op = tok->astOperand1()->astOperand2();
                if (op->isNumber())
                    continue;
                if (op->valueType() && op->valueType()->isEnum())
                    continue;
            }

            // Is code clarified by parentheses already?
            const Token *tok2 = tok->astOperand1();
            for (; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->str() == ")")
                    break;
                else if (tok2->str() == "?") {
                    clarifyCalculationError(tok, tok->astOperand1()->str());
                    break;
                }
            }
        }
    }
}

void CheckOther::clarifyCalculationError(const Token *tok, const std::string &op)
{
    // suspicious calculation
    const std::string calc("'a" + op + "b?c:d'");

    // recommended calculation #1
    const std::string s1("'(a" + op + "b)?c:d'");

    // recommended calculation #2
    const std::string s2("'a" + op + "(b?c:d)'");

    reportError(tok,
                Severity::style,
                "clarifyCalculation",
                "Clarify calculation precedence for '" + op + "' and '?'.\n"
                "Suspicious calculation. Please use parentheses to clarify the code. "
                "The code '" + calc + "' should be written as either '" + s1 + "' or '" + s2 + "'.", CWE783, Certainty::normal);
}

//---------------------------------------------------------------------------
// Clarify (meaningless) statements like *foo++; with parentheses.
//---------------------------------------------------------------------------
void CheckOther::clarifyStatement()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckOther::clarifyStatement"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->astOperand1() && Token::Match(tok, "* %name%")) {
                const Token *tok2 = tok->previous();

                while (tok2 && tok2->str() == "*")
                    tok2 = tok2->previous();

                if (tok2 && !tok2->astParent() && Token::Match(tok2, "[{};]")) {
                    tok2 = tok->astOperand1();
                    if (Token::Match(tok2, "++|-- [;,]"))
                        clarifyStatementError(tok2);
                }
            }
        }
    }
}

void CheckOther::clarifyStatementError(const Token *tok)
{
    reportError(tok, Severity::warning, "clarifyStatement", "In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'?\n"
                "A statement like '*A++;' might not do what you intended. Postfix 'operator++' is executed before 'operator*'. "
                "Thus, the dereference is meaningless. Did you intend to write '(*A)++;'?", CWE783, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check for suspicious occurrences of 'if(); {}'.
//---------------------------------------------------------------------------
void CheckOther::checkSuspiciousSemicolon()
{
    if (!mSettings->certainty.isEnabled(Certainty::inconclusive) || !mSettings->severity.isEnabled(Severity::warning))
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    logChecker("CheckOther::checkSuspiciousSemicolon"); // warning,inconclusive

    // Look for "if(); {}", "for(); {}" or "while(); {}"
    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type == ScopeType::eIf || scope.type == ScopeType::eElse || scope.type == ScopeType::eWhile || scope.type == ScopeType::eFor) {
            // Ensure the semicolon is at the same line number as the if/for/while statement
            // and the {..} block follows it without an extra empty line.
            if (Token::simpleMatch(scope.bodyStart, "{ ; } {") &&
                scope.bodyStart->previous()->linenr() == scope.bodyStart->tokAt(2)->linenr() &&
                scope.bodyStart->linenr()+1 >= scope.bodyStart->tokAt(3)->linenr() &&
                !scope.bodyStart->tokAt(3)->isExpandedMacro()) {
                suspiciousSemicolonError(scope.classDef);
            }
        }
    }
}

void CheckOther::suspiciousSemicolonError(const Token* tok)
{
    reportError(tok, Severity::warning, "suspiciousSemicolon",
                "Suspicious use of ; at the end of '" + (tok ? tok->str() : std::string()) + "' statement.", CWE398, Certainty::normal);
}

/** @brief would it make sense to use dynamic_cast instead of C style cast? */
static bool isDangerousTypeConversion(const Token* const tok)
{
    const Token* from = tok->astOperand1();
    if (!from)
        return false;
    if (!tok->valueType() || !from->valueType())
        return false;
    if (tok->valueType()->typeScope != nullptr &&
        tok->valueType()->typeScope == from->valueType()->typeScope)
        return false;
    if (tok->valueType()->type == from->valueType()->type &&
        tok->valueType()->isPrimitive())
        return false;
    // cast from derived object to base object is safe..
    if (tok->valueType()->typeScope && from->valueType()->typeScope) {
        const Type* fromType = from->valueType()->typeScope->definedType;
        const Type* toType = tok->valueType()->typeScope->definedType;
        if (fromType && toType && fromType->isDerivedFrom(toType->name()))
            return false;
    }
    const bool refcast = (tok->valueType()->reference != Reference::None);
    if (!refcast && tok->valueType()->pointer == 0)
        return false;
    if (!refcast && from->valueType()->pointer == 0)
        return false;

    if (tok->valueType()->type == ValueType::Type::VOID || from->valueType()->type == ValueType::Type::VOID)
        return false;
    if (tok->valueType()->pointer == 0 && tok->valueType()->isIntegral())
        // ok: (uintptr_t)ptr;
        return false;
    if (from->valueType()->pointer == 0 && from->valueType()->isIntegral())
        // ok: (int *)addr;
        return false;

    return true;
}

//---------------------------------------------------------------------------
// For C++ code, warn if C-style casts are used on pointer types
//---------------------------------------------------------------------------
void CheckOther::warningOldStylePointerCast()
{
    // Only valid on C++ code
    if (!mTokenizer->isCPP())
        return;

    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("cstyleCast"))
        return;

    logChecker("CheckOther::warningOldStylePointerCast"); // style,c++

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const Token* tok;
        if (scope->function && scope->function->isConstructor())
            tok = scope->classDef;
        else
            tok = scope->bodyStart;
        for (; tok && tok != scope->bodyEnd; tok = tok->next()) {
            // Old style pointer casting..
            if (!tok->isCast() || tok->isBinaryOp())
                continue;
            if (isDangerousTypeConversion(tok))
                continue;
            const Token* const errtok = tok;
            const Token* castTok = tok->next();
            while (Token::Match(castTok, "const|volatile|class|struct|union|%type%|::")) {
                castTok = castTok->next();
                if (Token::simpleMatch(castTok, "<") && castTok->link())
                    castTok = castTok->link()->next();
            }
            if (castTok == tok->next())
                continue;
            bool isPtr = false, isRef = false;
            while (Token::Match(castTok, "*|const|&")) {
                if (castTok->str() == "*")
                    isPtr = true;
                else if (castTok->str() == "&")
                    isRef = true;
                castTok = castTok->next();
            }
            if ((!isPtr && !isRef) || !Token::Match(castTok, ") (| %name%|%bool%|%char%|%str%|&"))
                continue;

            if (Token::Match(tok->previous(), "%type%"))
                continue;

            // skip first "const" in "const Type* const"
            while (Token::Match(tok->next(), "const|volatile|class|struct|union"))
                tok = tok->next();
            const Token* typeTok = tok->next();
            // skip second "const" in "const Type* const"
            if (tok->strAt(3) == "const")
                tok = tok->next();

            const Token *p = tok->tokAt(4);
            if (p->hasKnownIntValue() && p->getKnownIntValue()==0) // Casting nullpointers is safe
                continue;

            if (typeTok->tokType() == Token::eType || typeTok->tokType() == Token::eName)
                cstyleCastError(errtok, isPtr);
        }
    }
}

void CheckOther::cstyleCastError(const Token *tok, bool isPtr)
{
    const std::string type = isPtr ? "pointer" : "reference";
    reportError(tok, Severity::style, "cstyleCast",
                "C-style " + type + " casting\n"
                "C-style " + type + " casting detected. C++ offers four different kinds of casts as replacements: "
                "static_cast, const_cast, dynamic_cast and reinterpret_cast. A C-style cast could evaluate to "
                "any of those automatically, thus it is considered safer if the programmer explicitly states "
                "which kind of cast is expected.", CWE398, Certainty::normal);
}

void CheckOther::warningDangerousTypeCast()
{
    // Only valid on C++ code
    if (!mTokenizer->isCPP())
        return;
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("dangerousTypeCast"))
        return;

    logChecker("CheckOther::warningDangerousTypeCast"); // warning,c++

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const Token* tok;
        if (scope->function && scope->function->isConstructor())
            tok = scope->classDef;
        else
            tok = scope->bodyStart;
        for (; tok && tok != scope->bodyEnd; tok = tok->next()) {
            // Old style pointer casting..
            if (!tok->isCast() || tok->isBinaryOp())
                continue;

            if (isDangerousTypeConversion(tok))
                dangerousTypeCastError(tok, tok->valueType()->pointer > 0);
        }
    }
}

void CheckOther::dangerousTypeCastError(const Token *tok, bool isPtr)
{
    //const std::string type = isPtr ? "pointer" : "reference";
    (void)isPtr;
    reportError(tok, Severity::warning, "dangerousTypeCast",
                "Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast",
                CWE398, Certainty::normal);
}

void CheckOther::warningIntToPointerCast()
{
    if (!mSettings->severity.isEnabled(Severity::portability) && !mSettings->isPremiumEnabled("cstyleCast"))
        return;

    logChecker("CheckOther::warningIntToPointerCast"); // portability

    for (const Token* tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        // pointer casting..
        if (!tok->isCast())
            continue;
        const Token* from = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
        if (!from || !from->isNumber())
            continue;
        if (!tok->valueType() || tok->valueType()->pointer == 0)
            continue;
        if (!MathLib::isIntHex(from->str()) && from->hasKnownIntValue() && from->getKnownIntValue() != 0) {
            std::string format;
            if (MathLib::isDec(from->str()))
                format = "decimal";
            else if (MathLib::isOct(from->str()))
                format = "octal";
            else
                continue;
            intToPointerCastError(tok, format);
        }
    }
}

void CheckOther::intToPointerCastError(const Token *tok, const std::string& format)
{
    reportError(tok, Severity::portability, "intToPointerCast",
                "Casting non-zero " + format + " integer literal to pointer.",
                CWE398, Certainty::normal);
}

void CheckOther::suspiciousFloatingPointCast()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("suspiciousFloatingPointCast"))
        return;

    logChecker("CheckOther::suspiciousFloatingPointCast"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const Token* tok = scope->bodyStart;
        if (scope->function && scope->function->isConstructor())
            tok = scope->classDef;
        for (; tok && tok != scope->bodyEnd; tok = tok->next()) {

            if (!tok->isCast())
                continue;

            const ValueType* vt = tok->valueType();
            if (!vt || vt->pointer || vt->reference != Reference::None || (vt->type != ValueType::FLOAT && vt->type != ValueType::DOUBLE))
                continue;

            using VTT = std::vector<ValueType::Type>;
            const VTT sourceTypes = vt->type == ValueType::FLOAT ? VTT{ ValueType::DOUBLE, ValueType::LONGDOUBLE } : VTT{ ValueType::LONGDOUBLE };

            const Token* source = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
            if (!source || !source->valueType() || std::find(sourceTypes.begin(), sourceTypes.end(), source->valueType()->type) == sourceTypes.end())
                continue;

            const Token* parent = tok->astParent();
            if (!parent)
                continue;

            const ValueType* parentVt = parent->valueType();
            if (!parentVt || parent->str() == "(") {
                int argn{};
                if (const Token* ftok = getTokenArgumentFunction(tok, argn)) {
                    if (ftok->function()) {
                        if (const Variable* argVar = ftok->function()->getArgumentVar(argn))
                            parentVt = argVar->valueType();
                    }
                }
            }
            if (!parentVt || std::find(sourceTypes.begin(), sourceTypes.end(), parentVt->type) == sourceTypes.end())
                continue;

            suspiciousFloatingPointCastError(tok);
        }
    }
}

void CheckOther::suspiciousFloatingPointCastError(const Token* tok)
{
    reportError(tok, Severity::style, "suspiciousFloatingPointCast",
                "Floating-point cast causes loss of precision.\n"
                "If this cast is not intentional, remove it to avoid loss of precision", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// float* f; double* d = (double*)f; <-- Pointer cast to a type with an incompatible binary data representation
//---------------------------------------------------------------------------

void CheckOther::invalidPointerCast()
{
    if (!mSettings->severity.isEnabled(Severity::portability))
        return;

    logChecker("CheckOther::invalidPointerCast"); // portability

    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            const Token* toTok = nullptr;
            const Token* fromTok = nullptr;
            // Find cast
            if (Token::Match(tok, "( const|volatile| const|volatile| %type% %type%| const| * )")) {
                toTok = tok;
                fromTok = tok->astOperand1();
            } else if (Token::simpleMatch(tok, "reinterpret_cast <") && tok->linkAt(1)) {
                toTok = tok->linkAt(1)->next();
                fromTok = toTok->astOperand2();
            }
            if (!fromTok)
                continue;

            const ValueType* fromType = fromTok->valueType();
            const ValueType* toType = toTok->valueType();
            if (!fromType || !toType || !fromType->pointer || !toType->pointer)
                continue;

            if (fromType->type != toType->type && fromType->type >= ValueType::Type::BOOL && toType->type >= ValueType::Type::BOOL && (toType->type != ValueType::Type::CHAR || printInconclusive)) {
                if (toType->isIntegral() && fromType->isIntegral())
                    continue;

                invalidPointerCastError(tok, fromType->str(), toType->str(), toType->type == ValueType::Type::CHAR, toType->isIntegral());
            }
        }
    }
}


void CheckOther::invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive, bool toIsInt)
{
    if (toIsInt) { // If we cast something to int*, this can be useful to play with its binary data representation
        reportError(tok, Severity::portability, "invalidPointerCast", "Casting from " + from + " to " + to + " is not portable due to different binary data representations on different platforms.", CWE704, inconclusive ? Certainty::inconclusive : Certainty::normal);
    } else
        reportError(tok, Severity::portability, "invalidPointerCast", "Casting between " + from + " and " + to + " which have an incompatible binary data representation.", CWE704, Certainty::normal);
}


//---------------------------------------------------------------------------
// Detect redundant assignments: x = 0; x = 4;
//---------------------------------------------------------------------------

void CheckOther::checkRedundantAssignment()
{
    if (!mSettings->severity.isEnabled(Severity::style) &&
        !mSettings->isPremiumEnabled("redundantAssignment") &&
        !mSettings->isPremiumEnabled("redundantAssignInSwitch"))
        return;

    logChecker("CheckOther::checkRedundantAssignment"); // style

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        if (!scope->bodyStart)
            continue;
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::simpleMatch(tok, "] ("))
                // todo: handle lambdas
                break;
            if (Token::simpleMatch(tok, "try {"))
                // todo: check try blocks
                tok = tok->linkAt(1);
            if ((tok->isAssignmentOp() || tok->tokType() == Token::eIncDecOp) && tok->astOperand1()) {
                if (tok->astParent())
                    continue;

                // Do not warn about redundant initialization when rhs is trivial
                // TODO : do not simplify the variable declarations
                bool isInitialization = false;
                if (Token::Match(tok->tokAt(-2), "; %var% =") && tok->tokAt(-2)->isSplittedVarDeclEq()) {
                    isInitialization = true;
                    bool trivial = true;
                    visitAstNodes(tok->astOperand2(),
                                  [&](const Token *rhs) {
                        if (Token::simpleMatch(rhs, "{ 0 }"))
                            return ChildrenToVisit::none;
                        if (Token::Match(rhs, "%num%|%name%") && !rhs->varId())
                            return ChildrenToVisit::none;
                        if (Token::Match(rhs, ":: %name%") && rhs->hasKnownIntValue())
                            return ChildrenToVisit::none;
                        if (rhs->isCast())
                            return ChildrenToVisit::op2;
                        trivial = false;
                        return ChildrenToVisit::done;
                    });
                    if (trivial)
                        continue;
                }

                const Token* rhs = tok->astOperand2();
                // Do not warn about assignment with 0 / NULL
                if ((rhs && MathLib::isNullValue(rhs->str())) || isNullOperand(rhs))
                    continue;

                if (tok->astOperand1()->variable() && tok->astOperand1()->variable()->isReference())
                    // todo: check references
                    continue;

                if (tok->astOperand1()->variable() && tok->astOperand1()->variable()->isStatic())
                    // todo: check static variables
                    continue;

                bool inconclusive = false;
                if (tok->isCpp() && tok->astOperand1()->valueType()) {
                    // If there is a custom assignment operator => this is inconclusive
                    if (tok->astOperand1()->valueType()->typeScope) {
                        const std::string op = "operator" + tok->str();
                        const std::list<Function>& fList = tok->astOperand1()->valueType()->typeScope->functionList;
                        inconclusive = std::any_of(fList.cbegin(), fList.cend(), [&](const Function& f) {
                            return f.name() == op;
                        });
                    }
                    // assigning a smart pointer has side effects
                    if (tok->astOperand1()->valueType()->type == ValueType::SMART_POINTER)
                        break;
                }
                if (inconclusive && !mSettings->certainty.isEnabled(Certainty::inconclusive))
                    continue;

                FwdAnalysis fwdAnalysis(*mSettings);
                if (fwdAnalysis.hasOperand(tok->astOperand2(), tok->astOperand1()))
                    continue;

                // Is there a redundant assignment?
                const Token *start;
                if (tok->isAssignmentOp())
                    start = tok->astOperand2();
                else
                    start = tok->findExpressionStartEndTokens().second->next();

                const Token * tokenToCheck = tok->astOperand1();

                // Check if we are working with union
                for (const Token* tempToken = tokenToCheck; Token::simpleMatch(tempToken, ".");) {
                    tempToken = tempToken->astOperand1();
                    if (tempToken && tempToken->variable() && tempToken->variable()->type() && tempToken->variable()->type()->isUnionType())
                        tokenToCheck = tempToken;
                }

                if (start->hasKnownSymbolicValue(tokenToCheck) && Token::simpleMatch(start->astParent(), "=") && !diag(tok)) {
                    const ValueFlow::Value* val = start->getKnownValue(ValueFlow::Value::ValueType::SYMBOLIC);
                    if (val->intvalue == 0) // no offset
                        redundantAssignmentSameValueError(tokenToCheck, val, tok->astOperand1()->expressionString());
                }

                // Get next assignment..
                const Token *nextAssign = fwdAnalysis.reassign(tokenToCheck, start, scope->bodyEnd);
                // extra check for union
                if (nextAssign && tokenToCheck != tok->astOperand1())
                    nextAssign = fwdAnalysis.reassign(tok->astOperand1(), start, scope->bodyEnd);

                if (!nextAssign)
                    continue;

                // there is redundant assignment. Is there a case between the assignments?
                bool hasCase = false;
                for (const Token *tok2 = tok; tok2 != nextAssign; tok2 = tok2->next()) {
                    if (tok2->str() == "break" || tok2->str() == "return")
                        break;
                    if (tok2->str() == "case") {
                        hasCase = true;
                        break;
                    }
                }

                // warn
                if (hasCase)
                    redundantAssignmentInSwitchError(tok, nextAssign, tok->astOperand1()->expressionString());
                else if (isInitialization)
                    redundantInitializationError(tok, nextAssign, tok->astOperand1()->expressionString(), inconclusive);
                else {
                    diag(nextAssign);
                    redundantAssignmentError(tok, nextAssign, tok->astOperand1()->expressionString(), inconclusive);
                }
            }
        }
    }
}

void CheckOther::redundantCopyError(const Token *tok1, const Token* tok2, const std::string& var)
{
    const std::list<const Token *> callstack = { tok1, tok2 };
    reportError(callstack, Severity::performance, "redundantCopy",
                "$symbol:" + var + "\n"
                "Buffer '$symbol' is being written before its old content has been used.", CWE563, Certainty::normal);
}

void CheckOther::redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive)
{
    ErrorPath errorPath = { ErrorPathItem(tok1, var + " is assigned"), ErrorPathItem(tok2, var + " is overwritten") };
    if (inconclusive)
        reportError(std::move(errorPath), Severity::style, "redundantAssignment",
                    "$symbol:" + var + "\n"
                    "Variable '$symbol' is reassigned a value before the old one has been used if variable is no semaphore variable.\n"
                    "Variable '$symbol' is reassigned a value before the old one has been used. Make sure that this variable is not used like a semaphore in a threading environment before simplifying this code.", CWE563, Certainty::inconclusive);
    else
        reportError(std::move(errorPath), Severity::style, "redundantAssignment",
                    "$symbol:" + var + "\n"
                    "Variable '$symbol' is reassigned a value before the old one has been used.", CWE563, Certainty::normal);
}

void CheckOther::redundantInitializationError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive)
{
    ErrorPath errorPath = { ErrorPathItem(tok1, var + " is initialized"), ErrorPathItem(tok2, var + " is overwritten") };
    reportError(std::move(errorPath), Severity::style, "redundantInitialization",
                "$symbol:" + var + "\nRedundant initialization for '$symbol'. The initialized value is overwritten before it is read.",
                CWE563,
                inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok1, const Token* tok2, const std::string &var)
{
    ErrorPath errorPath = { ErrorPathItem(tok1, "$symbol is assigned"), ErrorPathItem(tok2, "$symbol is overwritten") };
    reportError(std::move(errorPath), Severity::style, "redundantAssignInSwitch",
                "$symbol:" + var + "\n"
                "Variable '$symbol' is reassigned a value before the old one has been used. 'break;' missing?", CWE563, Certainty::normal);
}

void CheckOther::redundantAssignmentSameValueError(const Token *tok, const ValueFlow::Value* val, const std::string &var)
{
    auto errorPath = val->errorPath;
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::style, "redundantAssignment",
                "$symbol:" + var + "\n"
                "Variable '$symbol' is assigned an expression that holds the same value.", CWE563, Certainty::normal);
}


//---------------------------------------------------------------------------
//    switch (x)
//    {
//        case 2:
//            y = a;        // <- this assignment is redundant
//        case 3:
//            y = b;        // <- case 2 falls through and sets y twice
//    }
//---------------------------------------------------------------------------
static inline bool isFunctionOrBreakPattern(const Token *tok)
{
    return Token::Match(tok, "%name% (") || Token::Match(tok, "break|continue|return|exit|goto|throw");
}

void CheckOther::redundantBitwiseOperationInSwitchError()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckOther::redundantBitwiseOperationInSwitch"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // Find the beginning of a switch. E.g.:
    //   switch (var) { ...
    for (const Scope &switchScope : symbolDatabase->scopeList) {
        if (switchScope.type != ScopeType::eSwitch || !switchScope.bodyStart)
            continue;

        // Check the contents of the switch statement
        std::map<int, const Token*> varsWithBitsSet;
        std::map<int, std::string> bitOperations;

        for (const Token *tok2 = switchScope.bodyStart->next(); tok2 != switchScope.bodyEnd; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                // Inside a conditional or loop. Don't mark variable accesses as being redundant. E.g.:
                //   case 3: b = 1;
                //   case 4: if (a) { b = 2; }    // Doesn't make the b=1 redundant because it's conditional
                if (Token::Match(tok2->previous(), ")|else {") && tok2->link()) {
                    const Token* endOfConditional = tok2->link();
                    for (const Token* tok3 = tok2; tok3 != endOfConditional; tok3 = tok3->next()) {
                        if (tok3->varId() != 0) {
                            varsWithBitsSet.erase(tok3->varId());
                            bitOperations.erase(tok3->varId());
                        } else if (isFunctionOrBreakPattern(tok3)) {
                            varsWithBitsSet.clear();
                            bitOperations.clear();
                        }
                    }
                    tok2 = endOfConditional;
                }
            }

            // Variable assignment. Report an error if it's assigned to twice before a break. E.g.:
            //    case 3: b = 1;    // <== redundant
            //    case 4: b = 2;

            if (Token::Match(tok2->previous(), ";|{|}|: %var% = %any% ;")) {
                varsWithBitsSet.erase(tok2->varId());
                bitOperations.erase(tok2->varId());
            }

            // Bitwise operation. Report an error if it's performed twice before a break. E.g.:
            //    case 3: b |= 1;    // <== redundant
            //    case 4: b |= 1;
            else if (Token::Match(tok2->previous(), ";|{|}|: %var% %assign% %num% ;") &&
                     (tok2->strAt(1) == "|=" || tok2->strAt(1) == "&=") &&
                     Token::Match(tok2->next()->astOperand2(), "%num%")) {
                std::string bitOp = tok2->strAt(1)[0] + tok2->strAt(2);
                const auto i2 = utils::as_const(varsWithBitsSet).find(tok2->varId());

                // This variable has not had a bit operation performed on it yet, so just make a note of it
                if (i2 == varsWithBitsSet.end()) {
                    varsWithBitsSet[tok2->varId()] = tok2;
                    bitOperations[tok2->varId()] = std::move(bitOp);
                }

                // The same bit operation has been performed on the same variable twice, so report an error
                else if (bitOperations[tok2->varId()] == bitOp)
                    redundantBitwiseOperationInSwitchError(i2->second, i2->second->str());

                // A different bit operation was performed on the variable, so clear it
                else {
                    varsWithBitsSet.erase(tok2->varId());
                    bitOperations.erase(tok2->varId());
                }
            }

            // Bitwise operation. Report an error if it's performed twice before a break. E.g.:
            //    case 3: b = b | 1;    // <== redundant
            //    case 4: b = b | 1;
            else if (Token::Match(tok2->previous(), ";|{|}|: %var% = %name% %or%|& %num% ;") &&
                     tok2->varId() == tok2->tokAt(2)->varId()) {
                std::string bitOp = tok2->strAt(3) + tok2->strAt(4);
                const auto i2 = utils::as_const(varsWithBitsSet).find(tok2->varId());

                // This variable has not had a bit operation performed on it yet, so just make a note of it
                if (i2 == varsWithBitsSet.end()) {
                    varsWithBitsSet[tok2->varId()] = tok2;
                    bitOperations[tok2->varId()] = std::move(bitOp);
                }

                // The same bit operation has been performed on the same variable twice, so report an error
                else if (bitOperations[tok2->varId()] == bitOp)
                    redundantBitwiseOperationInSwitchError(i2->second, i2->second->str());

                // A different bit operation was performed on the variable, so clear it
                else {
                    varsWithBitsSet.erase(tok2->varId());
                    bitOperations.erase(tok2->varId());
                }
            }

            // Not a simple assignment so there may be good reason if this variable is assigned to twice. E.g.:
            //    case 3: b = 1;
            //    case 4: b++;
            else if (tok2->varId() != 0 && tok2->strAt(1) != "|" && tok2->strAt(1) != "&") {
                varsWithBitsSet.erase(tok2->varId());
                bitOperations.erase(tok2->varId());
            }

            // Reset our record of assignments if there is a break or function call. E.g.:
            //    case 3: b = 1; break;
            if (isFunctionOrBreakPattern(tok2)) {
                varsWithBitsSet.clear();
                bitOperations.clear();
            }
        }
    }
}

void CheckOther::redundantBitwiseOperationInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style,
                "redundantBitwiseOperationInSwitch",
                "$symbol:" + varname + "\n"
                "Redundant bitwise operation on '$symbol' in 'switch' statement. 'break;' missing?");
}


//---------------------------------------------------------------------------
// Check for statements like case A||B: in switch()
//---------------------------------------------------------------------------
void CheckOther::checkSuspiciousCaseInSwitch()
{
    if (!mSettings->certainty.isEnabled(Certainty::inconclusive) || !mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckOther::checkSuspiciousCaseInSwitch"); // warning,inconclusive

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope & scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eSwitch)
            continue;

        for (const Token* tok = scope.bodyStart->next(); tok != scope.bodyEnd; tok = tok->next()) {
            if (tok->str() == "case") {
                const Token* finding = nullptr;
                for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == ":")
                        break;
                    if (Token::Match(tok2, "[;}{]"))
                        break;

                    if (tok2->str() == "?")
                        finding = nullptr;
                    else if (Token::Match(tok2, "&&|%oror%"))
                        finding = tok2;
                }
                if (finding)
                    suspiciousCaseInSwitchError(finding, finding->str());
            }
        }
    }
}

void CheckOther::suspiciousCaseInSwitchError(const Token* tok, const std::string& operatorString)
{
    reportError(tok, Severity::warning, "suspiciousCase",
                "Found suspicious case label in switch(). Operator '" + operatorString + "' probably doesn't work as intended.\n"
                "Using an operator like '" + operatorString + "' in a case label is suspicious. Did you intend to use a bitwise operator, multiple case labels or if/else instead?", CWE398, Certainty::inconclusive);
}

static bool isNestedInSwitch(const Scope* scope)
{
    while (scope) {
        if (scope->type == ScopeType::eSwitch)
            return true;
        if (scope->type == ScopeType::eUnconditional) {
            scope = scope->nestedIn;
            continue;
        }
        break;
    }
    return false;
}

static bool isVardeclInSwitch(const Token* tok)
{
    if (!tok)
        return false;
    if (!isNestedInSwitch(tok->scope()))
        return false;
    if (const Token* end = Token::findsimplematch(tok, ";")) {
        for (const Token* tok2 = tok; tok2 != end; tok2 = tok2->next()) {
            if (tok2->isKeyword() && tok2->str() == "case")
                return false;
            if (tok2->variable() && tok2->variable()->nameToken() == tok2) {
                end = tok2->scope()->bodyEnd;
                for (const Token* tok3 = tok2; tok3 != end; tok3 = tok3->next()) {
                    if (tok3->isKeyword())
                        return tok3->str() == "case";
                }
                return false;
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------
//    Find consecutive return, break, continue, goto or throw statements. e.g.:
//        break; break;
//    Detect dead code, that follows such a statement. e.g.:
//        return(0); foo();
//---------------------------------------------------------------------------
void CheckOther::checkUnreachableCode()
{
    // misra-c-2012-2.1
    // misra-c-2023-2.1
    // misra-cpp-2008-0-1-1
    // autosar
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("duplicateBreak") && !mSettings->isPremiumEnabled("unreachableCode"))
        return;

    logChecker("CheckOther::checkUnreachableCode"); // style

    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (scope->hasInlineOrLambdaFunction(nullptr, /*onlyInline*/ true))
            continue;
        for (const Token* tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            const Token* secondBreak = nullptr;
            const Token* labelName = nullptr;
            if (tok->link() && Token::Match(tok, "(|[|<"))
                tok = tok->link();
            else if (Token::Match(tok, "break|continue ;"))
                secondBreak = tok->tokAt(2);
            else if (Token::Match(tok, "[;{}:] return|throw") && tok->next()->isKeyword()) {
                if (Token::simpleMatch(tok->astParent(), "?"))
                    continue;
                tok = tok->next(); // tok should point to return or throw
                for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "{")
                        tok2 = tok2->link();
                    if (tok2->str() == ";") {
                        secondBreak = tok2->next();
                        break;
                    }
                }
            } else if (Token::Match(tok, "goto %any% ;")) {
                secondBreak = tok->tokAt(3);
                labelName = tok->next();
            } else if (Token::Match(tok, "%name% (") && mSettings->library.isnoreturn(tok) && !Token::Match(tok->next()->astParent(), "?|:")) {
                if ((!tok->function() || (tok->function()->token != tok && tok->function()->tokenDef != tok)) && tok->linkAt(1)->strAt(1) != "{")
                    secondBreak = tok->linkAt(1)->tokAt(2);
                if (Token::simpleMatch(secondBreak, "return")) {
                    // clarification for tools that function returns
                    continue;
                }
            }
            while (Token::simpleMatch(secondBreak, "}") && secondBreak->scope()->type == ScopeType::eUnconditional)
                secondBreak = secondBreak->next();
            if (secondBreak && secondBreak->scope()->nestedIn && secondBreak->scope()->nestedIn->type == ScopeType::eSwitch &&
                tok->str() == "break") {
                while (Token::simpleMatch(secondBreak, "{") && secondBreak->scope()->type == ScopeType::eUnconditional)
                    secondBreak = secondBreak->next();
            }
            while (Token::simpleMatch(secondBreak, ";"))
                secondBreak = secondBreak->next();

            // Statements follow directly, no line between them. (#3383)
            // TODO: Try to find a better way to avoid false positives due to preprocessor configurations.
            const bool inconclusive = secondBreak && (secondBreak->linenr() - 1 > secondBreak->previous()->linenr());

            if (secondBreak && (printInconclusive || !inconclusive)) {
                if (Token::Match(secondBreak, "continue|goto|throw|return") && secondBreak->isKeyword()) {
                    duplicateBreakError(secondBreak, inconclusive);
                    tok = Token::findmatch(secondBreak, "[}:]");
                } else if (secondBreak->str() == "break") { // break inside switch as second break statement should not issue a warning
                    if (tok->str() == "break") // If the previous was a break, too: Issue warning
                        duplicateBreakError(secondBreak, inconclusive);
                    else {
                        if (tok->scope()->type != ScopeType::eSwitch) // Check, if the enclosing scope is a switch
                            duplicateBreakError(secondBreak, inconclusive);
                    }
                    tok = Token::findmatch(secondBreak, "[}:]");
                } else if (!Token::Match(secondBreak, "return|}|case|default") && secondBreak->strAt(1) != ":") { // TODO: No bailout for unconditional scopes
                    // If the goto label is followed by a loop construct in which the label is defined it's quite likely
                    // that the goto jump was intended to skip some code on the first loop iteration.
                    bool labelInFollowingLoop = false;
                    if (labelName && Token::Match(secondBreak, "while|do|for")) {
                        const Token *scope2 = Token::findsimplematch(secondBreak, "{");
                        if (scope2) {
                            for (const Token *tokIter = scope2; tokIter != scope2->link() && tokIter; tokIter = tokIter->next()) {
                                if (Token::Match(tokIter, "[;{}] %any% :") && labelName->str() == tokIter->strAt(1)) {
                                    labelInFollowingLoop = true;
                                    break;
                                }
                            }
                        }
                    }

                    // hide FP for statements that just hide compiler warnings about unused function arguments
                    bool silencedCompilerWarningOnly = false;
                    const Token *silencedWarning = secondBreak;
                    for (;;) {
                        if (Token::Match(silencedWarning, "( void ) %name% ;")) {
                            silencedWarning = silencedWarning->tokAt(5);
                            continue;
                        }
                        if (silencedWarning && silencedWarning == scope->bodyEnd)
                            silencedCompilerWarningOnly = true;
                        break;
                    }
                    if (silencedWarning)
                        secondBreak = silencedWarning;

                    if (!labelInFollowingLoop && !silencedCompilerWarningOnly && !isVardeclInSwitch(secondBreak))
                        unreachableCodeError(secondBreak, tok, inconclusive);
                    tok = Token::findmatch(secondBreak, "[}:]");
                } else if (secondBreak->scope() && secondBreak->scope()->isLoopScope() && secondBreak->str() == "}" && tok->str() == "continue") {
                    redundantContinueError(tok);
                    tok = secondBreak;
                } else
                    tok = secondBreak;

                if (!tok)
                    break;
                tok = tok->previous(); // Will be advanced again by for loop
            }
        }
    }
}

void CheckOther::duplicateBreakError(const Token *tok, bool inconclusive)
{
    reportError(tok, Severity::style, "duplicateBreak",
                "Consecutive return, break, continue, goto or throw statements are unnecessary.\n"
                "Consecutive return, break, continue, goto or throw statements are unnecessary. "
                "The second statement can never be executed, and so should be removed.", CWE561, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckOther::unreachableCodeError(const Token *tok, const Token* noreturn, bool inconclusive)
{
    std::string msg = "Statements following ";
    if (noreturn && (noreturn->function() || mSettings->library.isnoreturn(noreturn)))
        msg += "noreturn function '" + noreturn->str() + "()'";
    else if (noreturn && noreturn->isKeyword())
        msg += "'" + noreturn->str() + "'";
    else
        msg += "return, break, continue, goto or throw";
    msg += " will never be executed.";
    reportError(tok, Severity::style, "unreachableCode",
                msg, CWE561, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckOther::redundantContinueError(const Token *tok)
{
    reportError(tok, Severity::style, "redundantContinue",
                "'continue' is redundant since it is the last statement in a loop.", CWE561, Certainty::normal);
}

static bool isSimpleExpr(const Token* tok, const Variable* var, const Settings& settings) {
    if (!tok)
        return false;
    if (tok->isNumber() || tok->tokType() == Token::eString || tok->tokType() == Token::eChar || tok->isBoolean())
        return true;
    if (isNullOperand(tok))
        return true;
    bool needsCheck = tok->varId() > 0;
    if (!needsCheck) {
        if (tok->isArithmeticalOp())
            return isSimpleExpr(tok->astOperand1(), var, settings) && (!tok->astOperand2() || isSimpleExpr(tok->astOperand2(), var, settings));
        const Token* ftok = tok->previous();
        if (Token::Match(ftok, "%name% (") &&
            ((ftok->function() && ftok->function()->isConst()) || settings.library.isFunctionConst(ftok->str(), /*pure*/ true)))
            needsCheck = true;
        else if (tok->str() == "[") {
            needsCheck = tok->astOperand1() && tok->astOperand1()->varId() > 0;
            tok = tok->astOperand1();
        }
        else if (isLeafDot(tok->astOperand2())) {
            needsCheck = tok->astOperand2()->varId() > 0;
            tok = tok->astOperand2();
        }
    }
    return (needsCheck && !findExpressionChanged(tok, tok->astParent(), var->scope()->bodyEnd, settings));
}

//---------------------------------------------------------------------------
// Check scope of variables..
//---------------------------------------------------------------------------
void CheckOther::checkVariableScope()
{
    if (mSettings->clang)
        return;

    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("variableScope"))
        return;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // In C it is common practice to declare local variables at the
    // start of functions.
    if (mSettings->daca && mTokenizer->isC())
        return;

    logChecker("CheckOther::checkVariableScope"); // style,notclang

    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->isLocal() || var->isConst())
            continue;

        if (var->nameToken()->isExpandedMacro())
            continue;
        if (isStructuredBindingVariable(var) && // warn for single decomposition
            !(Token::simpleMatch(var->nameToken()->astParent(), "[") && var->nameToken()->astParent()->astOperand2() == var->nameToken()))
            continue;

        const bool isPtrOrRef = var->isPointer() || var->isReference();
        const bool isSimpleType = var->typeStartToken()->isStandardType() || var->typeStartToken()->isEnumType() || var->typeStartToken()->isC() || symbolDatabase->isRecordTypeWithoutSideEffects(var->type());
        if (!isPtrOrRef && !isSimpleType && !astIsContainer(var->nameToken()))
            continue;

        if (mTokenizer->hasIfdef(var->nameToken(), var->scope()->bodyEnd))
            continue;

        // reference of range for loop variable..
        if (Token::Match(var->nameToken()->previous(), "& %var% = %var% .")) {
            const Token *otherVarToken = var->nameToken()->tokAt(2);
            const Variable *otherVar = otherVarToken->variable();
            if (otherVar && Token::Match(otherVar->nameToken(), "%var% :") &&
                otherVar->nameToken()->next()->astParent() &&
                Token::simpleMatch(otherVar->nameToken()->next()->astParent()->previous(), "for ("))
                continue;
        }

        bool forHead = false; // Don't check variables declared in header of a for loop
        for (const Token* tok = var->typeStartToken(); tok; tok = tok->previous()) {
            if (tok->str() == "(") {
                forHead = true;
                break;
            }
            if (Token::Match(tok, "[;{}]"))
                break;
        }
        if (forHead)
            continue;

        const Token* tok = var->nameToken()->next();
        bool isConstructor = false;
        if (Token::Match(tok, "; %varid% =", var->declarationId())) { // bailout for assignment
            tok = tok->tokAt(2)->astOperand2();
            if (!isSimpleExpr(tok, var, *mSettings))
                continue;
        }
        else if (Token::Match(tok, "{|(")) { // bailout for constructor
            isConstructor = true;
            const Token* argTok = tok->astOperand2();
            bool bail = false;
            while (argTok) {
                if (Token::simpleMatch(argTok, ",")) {
                    if (!isSimpleExpr(argTok->astOperand2(), var, *mSettings)) {
                        bail = true;
                        break;
                    }
                } else if (argTok->str() != "." && !isSimpleExpr(argTok, var, *mSettings)) {
                    bail = true;
                    break;
                }
                argTok = argTok->astOperand1();
            }
            if (bail)
                continue;
        }
        // bailout if initialized with function call that has possible side effects
        if (!isConstructor && Token::Match(tok, "[(=]") && Token::simpleMatch(tok->astOperand2(), "("))
            continue;
        bool reduce = true;
        bool used = false; // Don't warn about unused variables
        for (; tok && tok != var->scope()->bodyEnd; tok = tok->next()) {
            if (tok->str() == "{" && tok->scope() != tok->previous()->scope() && !tok->isExpandedMacro() && !isWithinScope(tok, var, ScopeType::eLambda)) {
                if (used) {
                    bool used2 = false;
                    if (!checkInnerScope(tok, var, used2) || used2) {
                        reduce = false;
                        break;
                    }
                } else if (!checkInnerScope(tok, var, used)) {
                    reduce = false;
                    break;
                }

                tok = tok->link();

                // parse else if blocks..
            } else if (Token::simpleMatch(tok, "else { if (") && tok->next()->isSimplifiedScope() && Token::simpleMatch(tok->linkAt(3), ") {")) {
                tok = tok->next();
            } else if (tok->varId() == var->declarationId() || tok->str() == "goto") {
                reduce = false;
                break;
            }
        }

        if (reduce && used)
            variableScopeError(var->nameToken(), var->name());
    }
}

// used to check if an argument to a function might depend on another argument
static bool mayDependOn(const ValueType *other, const ValueType *original)
{
    if (!other || !original)
        return false;

    // other must be pointer
    if (!other->pointer)
        return false;

    // must be same underlying type
    if (other->type != original->type)
        return false;

    const int otherPtr = other->pointer + (other->reference == Reference::LValue ? 1 : 0);
    const int originalPtr = original->pointer;

    if (otherPtr == originalPtr) {
        // if other is not const than original may be copied to other
        return !other->isConst(otherPtr);
    }

    // other may be reassigned to original
    return otherPtr > originalPtr;
}

static bool isOnlyUsedInCurrentScope(const Variable* var, const Token *tok, const Scope* scope)
{
    if (tok->scope() == scope)
        return true;
    if (tok->scope()->type == ScopeType::eSwitch)
        return false;
    return !Token::findmatch(tok->scope()->bodyEnd, "%varid%", var->scope()->bodyEnd, var->declarationId());
}

bool CheckOther::checkInnerScope(const Token *tok, const Variable* var, bool& used) const
{
    const Scope* scope = tok->next()->scope();
    bool loopVariable = scope->isLoopScope();
    bool noContinue = true;
    const Token* forHeadEnd = nullptr;
    const Token* end = tok->link();
    if (scope->type == ScopeType::eUnconditional && (tok->strAt(-1) == ")" || tok->previous()->isName())) // Might be an unknown macro like BOOST_FOREACH
        loopVariable = true;

    if (scope->type == ScopeType::eDo) {
        end = end->linkAt(2);
    } else if (loopVariable && tok->strAt(-1) == ")") {
        tok = tok->linkAt(-1); // Jump to opening ( of for/while statement
    } else if (scope->type == ScopeType::eSwitch) {
        for (const Scope* innerScope : scope->nestedList) {
            if (used) {
                bool used2 = false;
                if (!checkInnerScope(innerScope->bodyStart, var, used2) || used2) {
                    return false;
                }
            } else if (!checkInnerScope(innerScope->bodyStart, var, used)) {
                return false;
            }
        }
    }

    bool bFirstAssignment=false;
    for (; tok && tok != end; tok = tok->next()) {
        if (tok->str() == "goto")
            return false;
        if (tok->str() == "continue")
            noContinue = false;

        if (Token::simpleMatch(tok, "for ("))
            forHeadEnd = tok->linkAt(1);
        if (tok == forHeadEnd)
            forHeadEnd = nullptr;

        if (loopVariable && noContinue && !forHeadEnd && scope->type != ScopeType::eSwitch && Token::Match(tok, "%varid% =", var->declarationId()) &&
            isOnlyUsedInCurrentScope(var, tok, scope)) { // Assigned in outer scope.
            loopVariable = false;
            std::pair<const Token*, const Token*> range = tok->next()->findExpressionStartEndTokens();
            if (range.first)
                range.first = range.first->next();
            const Token* exprTok = findExpression(var->nameToken()->exprId(), range.first, range.second, [&](const Token* tok2) {
                return tok2->varId() == var->declarationId();
            });
            if (exprTok) {
                tok = exprTok;
                loopVariable = true;
            }
        }

        if (loopVariable && Token::Match(tok, "%varid% !!=", var->declarationId())) // Variable used in loop
            return false;

        if (Token::Match(tok, "& %varid%", var->declarationId())) // Taking address of variable
            return false;

        if (Token::Match(tok, "%varid% =", var->declarationId())) {
            if (!bFirstAssignment && var->isInit() && Token::findmatch(tok->tokAt(2), "%varid%", Token::findsimplematch(tok->tokAt(3), ";"), var->declarationId()))
                return false;
            bFirstAssignment = true;
        }

        if (!bFirstAssignment && Token::Match(tok, "* %varid%", var->declarationId())) // dereferencing means access to previous content
            return false;

        if (Token::Match(tok, "= %varid%", var->declarationId()) && (var->isArray() || var->isPointer() || (var->valueType() && var->valueType()->container))) // Create a copy of array/pointer. Bailout, because the memory it points to might be necessary in outer scope
            return false;

        if (tok->varId() == var->declarationId()) {
            used = true;
            if (scope == tok->scope()) {
                if (scope->type == ScopeType::eSwitch)
                    return false; // Used in outer switch scope - unsafe or impossible to reduce scope

                if (scope->bodyStart && scope->bodyStart->isSimplifiedScope())
                    return false; // simplified if/for/switch init statement
            }
            if (var->isArrayOrPointer()) {
                int argn{};
                if (const Token* ftok = getTokenArgumentFunction(tok, argn)) { // var passed to function?
                    if (ftok->next()->astParent()) { // return value used?
                        if (ftok->function() && Function::returnsPointer(ftok->function()))
                            return false;
                        const std::string ret = mSettings->library.returnValueType(ftok); // assume that var is returned
                        if (!ret.empty() && ret.back() == '*')
                            return false;
                    }
                    if (ftok->function()) {
                        const std::list<Variable> &argvars = ftok->function()->argumentList;
                        if (const Variable* argvar = ftok->function()->getArgumentVar(argn)) {
                            if (!std::all_of(argvars.cbegin(), argvars.cend(), [&](const Variable& other) {
                                return &other == argvar || !mayDependOn(other.valueType(), argvar->valueType());
                            }))
                                return false;
                        }
                    }
                }
            }
            const auto yield = astContainerYield(tok, mSettings->library);
            if (yield == Library::Container::Yield::BUFFER || yield == Library::Container::Yield::BUFFER_NT)
                return false;
        }
    }

    return true;
}

void CheckOther::variableScopeError(const Token *tok, const std::string &varname)
{
    reportError(tok,
                Severity::style,
                "variableScope",
                "$symbol:" + varname + "\n"
                "The scope of the variable '$symbol' can be reduced.\n"
                "The scope of the variable '$symbol' can be reduced. Warning: Be careful "
                "when fixing this message, especially when there are inner loops. Here is an "
                "example where cppcheck will write that the scope for 'i' can be reduced:\n"
                "void f(int x)\n"
                "{\n"
                "    int i = 0;\n"
                "    if (x) {\n"
                "        // it's safe to move 'int i = 0;' here\n"
                "        for (int n = 0; n < 10; ++n) {\n"
                "            // it is possible but not safe to move 'int i = 0;' here\n"
                "            do_something(&i);\n"
                "        }\n"
                "    }\n"
                "}\n"
                "When you see this message it is always safe to reduce the variable scope 1 level.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// Comma in return statement: return a+1, b++;. (experimental)
//---------------------------------------------------------------------------
void CheckOther::checkCommaSeparatedReturn()
{
    // TODO: This is experimental for now. See #5076
    if ((true) || !mSettings->severity.isEnabled(Severity::style)) // NOLINT(readability-simplify-boolean-expr,readability-redundant-parentheses)
        return;

    // logChecker

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "return") {
            tok = tok->next();
            while (tok && tok->str() != ";") {
                if (tok->link() && Token::Match(tok, "[([{<]"))
                    tok = tok->link();

                if (!tok->isExpandedMacro() && tok->str() == "," && tok->linenr() != tok->next()->linenr())
                    commaSeparatedReturnError(tok);

                tok = tok->next();
            }
            // bailout: missing semicolon (invalid code / bad tokenizer)
            if (!tok)
                break;
        }
    }
}

void CheckOther::commaSeparatedReturnError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "commaSeparatedReturn",
                "Comma is used in return statement. The comma can easily be misread as a ';'.\n"
                "Comma is used in return statement. When comma is used in a return statement it can "
                "easily be misread as a semicolon. For example in the code below the value "
                "of 'b' is returned if the condition is true, but it is easy to think that 'a+1' is "
                "returned:\n"
                "    if (x)\n"
                "        return a + 1,\n"
                "    b++;\n"
                "However it can be useful to use comma in macros. No warning is reported when such a "
                "macro is then used in a return statement, it is less likely such code is misunderstood.", CWE398, Certainty::normal);
}

static bool isLargeContainer(const Variable* var, const Settings& settings)
{
    const ValueType* vt = var->valueType();
    if (vt->container->startPattern == "std :: initializer_list <")
        return false;
    if (vt->container->size_templateArgNo < 0)
        return true;
    const std::size_t maxByValueSize = 2 * settings.platform.sizeof_pointer;
    if (var->dimensions().empty()) {
        if (vt->container->startPattern == "std :: bitset <") {
            if (const ValueFlow::Value* v = vt->containerTypeToken->getKnownValue(ValueFlow::Value::ValueType::INT))
                return v->intvalue / 8 > maxByValueSize;
        }
        return false;
    }
    const ValueType vtElem = ValueType::parseDecl(vt->containerTypeToken, settings);
    const auto elemSize = std::max<std::size_t>(vtElem.getSizeOf(settings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer), 1);
    const auto arraySize = var->dimension(0) * elemSize;
    return arraySize > maxByValueSize;
}

void CheckOther::checkPassByReference()
{
    if (!mSettings->severity.isEnabled(Severity::performance) || mTokenizer->isC())
        return;

    logChecker("CheckOther::checkPassByReference"); // performance,c++

    const SymbolDatabase * const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->isClass() || var->isPointer() || (var->isArray() && !var->isStlType()) || var->isReference() || var->isEnumType())
            continue;

        const bool isRangeBasedFor = astIsRangeBasedForDecl(var->nameToken());
        if (!var->isArgument() && !isRangeBasedFor)
            continue;

        if (!isRangeBasedFor && var->scope() && var->scope()->function->arg->link()->strAt(-1) == "...")
            continue; // references could not be used as va_start parameters (#5824)

        const Token * const varDeclEndToken = var->declEndToken();
        if ((varDeclEndToken && varDeclEndToken->isExternC()) ||
            (var->scope() && var->scope()->function && var->scope()->function->tokenDef && var->scope()->function->tokenDef->isExternC()))
            continue; // references cannot be used in functions in extern "C" blocks

        bool inconclusive = false;

        const bool isContainer = var->valueType() && var->valueType()->type == ValueType::Type::CONTAINER && var->valueType()->container && !var->valueType()->container->view;
        if (isContainer && !isLargeContainer(var, *mSettings))
            continue;
        if (!isContainer) {
            if (var->type() && !var->type()->isEnumType()) { // Check if type is a struct or class.
                // Ensure that it is a large object.
                if (!var->type()->classScope)
                    inconclusive = true;
                else if (!var->valueType() || var->valueType()->getSizeOf(*mSettings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer) <= 2 * mSettings->platform.sizeof_pointer)
                    continue;
            }
            else
                continue;
        }

        if (inconclusive && !mSettings->certainty.isEnabled(Certainty::inconclusive))
            continue;

        if (var->isArray() && (!var->isStlType() || Token::simpleMatch(var->nameToken()->next(), "[")))
            continue;

        const bool isConst = var->isConst();
        if (isConst) {
            passedByValueError(var, inconclusive, isRangeBasedFor);
            continue;
        }

        // Check if variable could be const
        if (!isRangeBasedFor && (!var->scope() || var->scope()->function->isImplicitlyVirtual()))
            continue;

        if (!isVariableChanged(var, *mSettings)) {
            passedByValueError(var, inconclusive, isRangeBasedFor);
        }
    }
}

void CheckOther::passedByValueError(const Variable* var, bool inconclusive, bool isRangeBasedFor)
{
    std::string id = isRangeBasedFor ? "iterateByValue" : "passedByValue";
    const std::string action = isRangeBasedFor ? "declared as": "passed by";
    const std::string type = isRangeBasedFor ? "Range variable" : "Function parameter";
    std::string msg = "$symbol:" + (var ? var->name() : "") + "\n" +
                      type + " '$symbol' should be " + action + " const reference.";
    ErrorPath errorPath;
    if (var && var->scope() && var->scope()->function && var->scope()->function->functionPointerUsage) {
        id += "Callback";
        errorPath.emplace_front(var->scope()->function->functionPointerUsage, "Function pointer used here.");
        msg += " However it seems that '" + var->scope()->function->name() + "' is a callback function.";
    }
    if (var)
        errorPath.emplace_back(var->nameToken(), msg);
    if (isRangeBasedFor)
        msg += "\nVariable '$symbol' is used to iterate by value. It could be declared as a const reference which is usually faster and recommended in C++.";
    else
        msg += "\nParameter '$symbol' is passed by value. It could be passed as a const reference which is usually faster and recommended in C++.";
    reportError(std::move(errorPath), Severity::performance, id.c_str(), msg, CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

static bool isVariableMutableInInitializer(const Token* start, const Token * end, nonneg int varid)
{
    if (!start)
        return false;
    if (!end)
        return false;
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() != varid)
            continue;
        if (tok->astParent()) {
            const Token * memberTok = tok->astParent()->previous();
            if (Token::Match(memberTok, "%var% (") && memberTok->variable()) {
                const Variable * memberVar = memberTok->variable();
                if (memberVar->isClass())
                    //TODO: check if the called constructor could live with a const variable
                    // pending that, assume the worst (that it can't)
                    return true;
                if (!memberVar->isReference())
                    continue;
                if (memberVar->isConst())
                    continue;
            }
        }
        return true;
    }
    return false;
}

static bool isCastToVoid(const Variable* var)
{
    if (!var)
        return false;
    if (!var->scope())
        return false;
    for (const Token* tok = var->scope()->bodyStart; tok != var->scope()->bodyEnd; tok = tok->next()) {
        if (tok->varId() != var->declarationId())
            continue;
        if (isVoidCast(tok->astParent()))
            return true;
    }
    return false;
}

void CheckOther::checkConstVariable()
{
    if ((!mSettings->severity.isEnabled(Severity::style) || mTokenizer->isC()) && !mSettings->isPremiumEnabled("constVariable"))
        return;

    logChecker("CheckOther::checkConstVariable"); // style,c++

    const SymbolDatabase *const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Variable *var : symbolDatabase->variableList()) {
        if (!var)
            continue;
        if (!var->isReference())
            continue;
        if (var->isRValueReference())
            continue;
        if (var->isPointer())
            continue;
        if (var->isConst())
            continue;
        const Scope* scope = var->scope();
        if (!scope)
            continue;
        const Function* function = scope->function;
        if (!function && !scope->isLocal())
            continue;
        if (function && var->isArgument()) {
            if (function->isImplicitlyVirtual() || function->templateDef)
                continue;
            if (function->isConstructor() && isVariableMutableInInitializer(function->constructorMemberInitialization(), scope->bodyStart, var->declarationId()))
                continue;
        }
        if (var->isGlobal())
            continue;
        if (var->isStatic())
            continue;
        if (var->isArray() && !var->isStlType())
            continue;
        if (var->isEnumType())
            continue;
        if (var->isVolatile())
            continue;
        if (var->isMaybeUnused())
            continue;
        if (var->nameToken()->isExpandedMacro())
            continue;
        if (isStructuredBindingVariable(var)) // TODO: check all bound variables
            continue;
        if (isCastToVoid(var))
            continue;
        if (isVariableChanged(var, *mSettings))
            continue;
        const bool hasFunction = function != nullptr;
        if (!hasFunction) {
            const Scope* functionScope = scope;
            do {
                functionScope = functionScope->nestedIn;
            } while (functionScope && !(function = functionScope->function));
        }
        if (function && (Function::returnsReference(function) || Function::returnsPointer(function)) && !Function::returnsConst(function)) {
            std::vector<const Token*> returns = Function::findReturns(function);
            if (std::any_of(returns.cbegin(), returns.cend(), [&](const Token* retTok) {
                if (retTok->varId() == var->declarationId())
                    return true;
                while (retTok && retTok->isCast())
                    retTok = retTok->astOperand2() ? retTok->astOperand2() : retTok->astOperand1();
                while (Token::simpleMatch(retTok, "."))
                    retTok = retTok->astOperand2();
                if (Token::simpleMatch(retTok, "&"))
                    retTok = retTok->astOperand1();
                ValueFlow::Value ltVal = ValueFlow::getLifetimeObjValue(retTok);
                if (ltVal.isLifetimeValue() && ltVal.tokvalue->varId() == var->declarationId())
                    return true;
                return ValueFlow::hasLifetimeToken(getParentLifetime(retTok), var->nameToken(), *mSettings);
            }))
                continue;
        }
        // Skip if another non-const variable is initialized with this variable
        {
            //Is it the right side of an initialization of a non-const reference
            bool usedInAssignment = false;
            for (const Token* tok = var->nameToken(); tok != scope->bodyEnd && tok != nullptr; tok = tok->next()) {
                if (Token::Match(tok, "& %var% = %varid%", var->declarationId())) {
                    const Variable* refvar = tok->next()->variable();
                    if (refvar && !refvar->isConst() && refvar->nameToken() == tok->next()) {
                        usedInAssignment = true;
                        break;
                    }
                }
                if (tok->isUnaryOp("&") && Token::Match(tok, "& %varid%", var->declarationId())) {
                    const Token* opTok = tok->astParent();
                    int argn = -1;
                    if (opTok && (opTok->isUnaryOp("!") || opTok->isComparisonOp()))
                        continue;
                    if (opTok && (opTok->isAssignmentOp() || opTok->isCalculation())) {
                        if (opTok->isCalculation()) {
                            if (opTok->astOperand1() != tok)
                                opTok = opTok->astOperand1();
                            else
                                opTok = opTok->astOperand2();
                        }
                        if (opTok && opTok->valueType() && var->valueType() && opTok->valueType()->isConst(var->valueType()->pointer))
                            continue;
                    } else if (const Token* ftok = getTokenArgumentFunction(tok, argn)) {
                        bool inconclusive{};
                        if (var->valueType() && !isVariableChangedByFunctionCall(ftok, var->valueType()->pointer, var->declarationId(), *mSettings, &inconclusive) && !inconclusive)
                            continue;
                    }
                    usedInAssignment = true;
                    break;
                }
                if (astIsRangeBasedForDecl(tok) && Token::Match(tok->astParent()->astOperand2(), "%varid%", var->declarationId())) {
                    const Variable* refvar = tok->astParent()->astOperand1()->variable();
                    if (refvar && refvar->isReference() && !refvar->isConst()) {
                        usedInAssignment = true;
                        break;
                    }
                }
            }
            if (usedInAssignment)
                continue;
        }

        constVariableError(var, hasFunction ? function : nullptr);
    }
}

static const Token* getVariableChangedStart(const Variable* p)
{
    if (p->isArgument())
        return p->scope()->bodyStart;
    const Token* start = p->nameToken()->next();
    if (start->isSplittedVarDeclEq())
        start = start->tokAt(3);
    return start;
}

static bool isConstPointerVariable(const Variable* p, const Settings& settings)
{
    const int indirect = p->isArray() ? p->dimensions().size() : 1;
    const Token* start = getVariableChangedStart(p);
    while (const Token* tok =
               findVariableChanged(start, p->scope()->bodyEnd, indirect, p->declarationId(), false, settings)) {
        if (p->isReference())
            return false;
        // Assigning a pointer through another pointer may still be const
        if (!Token::simpleMatch(tok->astParent(), "="))
            return false;
        if (!astIsLHS(tok))
            return false;
        start = tok->next();
    }
    return true;
}

namespace {
    struct CompareVariables {
        bool operator()(const Variable* a, const Variable* b) const {
            const int fileA = a->nameToken()->fileIndex();
            const int fileB = b->nameToken()->fileIndex();
            if (fileA != fileB)
                return fileA < fileB;
            const int lineA = a->nameToken()->linenr();
            const int lineB = b->nameToken()->linenr();
            if (lineA != lineB)
                return lineA < lineB;
            const int columnA = a->nameToken()->column();
            const int columnB = b->nameToken()->column();
            return columnA < columnB;
        }
    };
}

static bool isCastToInteger(const Token* tok)
{
    return tok && tok->isCast() && tok->valueType() && tok->valueType()->isIntegral() && tok->valueType()->pointer == 0;
}

static const Function* getEnclosingFunction(const Variable* var)
{
    if (var->isArgument())
        return var->scope()->function;
    const Scope* scope = var->scope();
    while (scope && scope->type != ScopeType::eFunction) {
        scope = scope->nestedIn;
    }
    return scope ? scope->function : nullptr;
}

void CheckOther::checkConstPointer()
{
    if (!mSettings->severity.isEnabled(Severity::style) &&
        !mSettings->isPremiumEnabled("constParameter") &&
        !mSettings->isPremiumEnabled("constParameterPointer") &&
        !mSettings->isPremiumEnabled("constParameterReference") &&
        !mSettings->isPremiumEnabled("constVariablePointer"))
        return;

    logChecker("CheckOther::checkConstPointer"); // style

    std::set<const Variable*, CompareVariables> pointers, nonConstPointers;
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        const Variable* const var = tok->variable();
        if (!var)
            continue;
        if (!var->isLocal() && !var->isArgument())
            continue;
        if (var->isArgument() && var->scope() && var->scope()->type == ScopeType::eLambda && !Token::simpleMatch(var->scope()->bodyEnd, "} ("))
            continue;
        const Token* const nameTok = var->nameToken();
        if (tok == nameTok && var->isLocal() && !astIsRangeBasedForDecl(nameTok)) {
            if (var->isReference() && var->isPointer()) {
                nonConstPointers.emplace(var);
                continue;
            }
            // declarations of (static) pointers are (not) split up, array declarations are never split up
            if ((!var->isStatic() || Token::simpleMatch(nameTok->next(), "[")))
                continue;
        }
        // Skip function pointers
        if (Token::Match(nameTok, "%name% ) ("))
            continue;
        const ValueType* const vt = tok->valueType();
        if (!vt)
            continue;
        if ((vt->pointer != 1 && !(vt->pointer == 2 && var->isArray())) || (vt->constness & 1))
            continue;
        if (var->typeStartToken()->isTemplateArg())
            continue;
        if (std::find(nonConstPointers.cbegin(), nonConstPointers.cend(), var) != nonConstPointers.cend())
            continue;
        pointers.emplace(var);
        const Token* parent = tok->astParent();
        enum Deref : std::uint8_t { NONE, DEREF, MEMBER } deref = NONE;
        bool hasIncDecPlus = false;
        if (parent && (parent->isUnaryOp("*") || (((hasIncDecPlus = parent->isIncDecOp()) || (hasIncDecPlus = (parent->str() == "+"))) &&
                                                  parent->astParent() && parent->astParent()->isUnaryOp("*"))))
            deref = DEREF;
        else if (Token::simpleMatch(parent, "[") && parent->astOperand1() == tok && tok != nameTok)
            deref = DEREF;
        else if (Token::Match(parent, "%op%") && Token::simpleMatch(parent->astParent(), "."))
            deref = MEMBER;
        else if (Token::simpleMatch(parent, "."))
            deref = MEMBER;
        else if (astIsRangeBasedForDecl(tok))
            continue;
        else if (isCastToInteger(parent))
            continue;
        if (deref != NONE) {
            const Token* gparent = parent->astParent();
            while (Token::simpleMatch(gparent, "[") && parent != gparent->astOperand2() && parent->str() == gparent->str())
                gparent = gparent->astParent();
            if (deref == MEMBER) {
                if (!gparent)
                    continue;
                if (parent->astOperand2()) {
                    if (parent->astOperand2()->function() && parent->astOperand2()->function()->isConst())
                        continue;
                    if (mSettings->library.isFunctionConst(parent->astOperand2()))
                        continue;
                }
            }
            if (hasIncDecPlus) {
                parent = gparent;
                gparent = gparent ? gparent->astParent() : nullptr;
            }
            if (Token::Match(gparent, "%cop%") && !gparent->isUnaryOp("&") && !gparent->isUnaryOp("*"))
                continue;
            int argn = -1;
            if (Token::simpleMatch(gparent, "return")) {
                const Function* function = getEnclosingFunction(var);
                if (function && (!Function::returnsReference(function) || Function::returnsConst(function)))
                    continue;
            }
            else if (Token::Match(gparent, "%assign%") && parent == gparent->astOperand2()) {
                bool takingRef = false, nonConstPtrAssignment = false;
                const Token *lhs = gparent->astOperand1();
                if (lhs && lhs->variable() && lhs->variable()->isReference() && lhs->variable()->nameToken() == lhs && !lhs->variable()->isConst())
                    takingRef = true;
                if (lhs && lhs->valueType() && lhs->valueType()->pointer && (lhs->valueType()->constness & 1) == 0 &&
                    parent->valueType() && parent->valueType()->pointer && lhs->variable() != var)
                    nonConstPtrAssignment = true;
                if (!takingRef && !nonConstPtrAssignment)
                    continue;
            } else if (Token::simpleMatch(gparent, "[") && gparent->astOperand2() == parent)
                continue;
            else if (gparent && gparent->isCast() && gparent->valueType() &&
                     ((gparent->valueType()->pointer == 0 && gparent->valueType()->reference == Reference::None) ||
                      (var->valueType() && gparent->valueType()->isConst(var->valueType()->pointer))))
                continue;
            else if (const Token* ftok = getTokenArgumentFunction(parent, argn)) {
                bool inconclusive{};
                if (!isVariableChangedByFunctionCall(ftok->next(), vt->pointer, var->declarationId(), *mSettings, &inconclusive) && !inconclusive)
                    continue;
            }
        } else {
            int argn = -1;
            if (Token::Match(parent, "%oror%|%comp%|&&|?|!|-|<<|;"))
                continue;
            if (hasIncDecPlus && (!parent->astParent() || parent->astParent()->str() == ";"))
                continue;
            if (Token::simpleMatch(parent, "(") && Token::Match(parent->astOperand1(), "if|while"))
                continue;
            if (Token::simpleMatch(parent, "=")) {
                const Token* lhs = parent->astOperand1();
                if (lhs == tok)
                    continue;
                if (lhs && lhs->valueType() && lhs->valueType()->isConst(vt->pointer))
                    continue;
            }
            if (const Token* ftok = getTokenArgumentFunction(tok, argn)) {
                if (ftok->function()) {
                    const bool isCastArg = parent->isCast() && !ftok->function()->getOverloadedFunctions().empty(); // assume that cast changes the called function
                    if (!isCastArg) {
                        const Variable* argVar = ftok->function()->getArgumentVar(argn);
                        if (argVar && argVar->valueType() && argVar->valueType()->isConst(vt->pointer)) {
                            bool inconclusive{};
                            if (!isVariableChangedByFunctionCall(ftok, vt->pointer, var->declarationId(), *mSettings, &inconclusive) && !inconclusive)
                                continue;
                        }
                    }
                } else {
                    const auto dir = mSettings->library.getArgDirection(ftok, argn + 1);
                    if (dir == Library::ArgumentChecks::Direction::DIR_IN)
                        continue;
                }
            }
            else if (Token::simpleMatch(parent, "(")) {
                if (parent->isCast() && parent->valueType() && var->valueType() && parent->valueType()->isConst(var->valueType()->pointer))
                    continue;
            }
        }
        if (tok != nameTok)
            nonConstPointers.emplace(var);
    }
    for (const Variable *p: pointers) {
        if (p->isArgument()) {
            if (!p->scope() || !p->scope()->function || p->scope()->function->isImplicitlyVirtual(true) || p->scope()->function->hasVirtualSpecifier())
                continue;
            if (p->isMaybeUnused())
                continue;
        }
        if (const Function* func = Scope::nestedInFunction(p->scope()))
            if (func->templateDef)
                continue;
        if (std::find(nonConstPointers.cbegin(), nonConstPointers.cend(), p) == nonConstPointers.cend()) {
            // const Token *start = getVariableChangedStart(p);
            // const int indirect = p->isArray() ? p->dimensions().size() : 1;
            // if (isVariableChanged(start, p->scope()->bodyEnd, indirect, p->declarationId(), false, *mSettings))
            //     continue;
            if (!isConstPointerVariable(p, *mSettings))
                continue;
            if (p->typeStartToken() && p->typeStartToken()->isSimplifiedTypedef() && !(Token::simpleMatch(p->typeEndToken(), "*") && !p->typeEndToken()->isSimplifiedTypedef()))
                continue;
            constVariableError(p, p->isArgument() ? p->scope()->function : nullptr);
        }
    }
}

void CheckOther::constVariableError(const Variable *var, const Function *function)
{
    if (!var) {
        reportError(nullptr, Severity::style, "constParameter", "Parameter 'x' can be declared with const");
        reportError(nullptr, Severity::style, "constVariable",  "Variable 'x' can be declared with const");
        reportError(nullptr, Severity::style, "constParameterReference", "Parameter 'x' can be declared with const");
        reportError(nullptr, Severity::style, "constVariableReference", "Variable 'x' can be declared with const");
        reportError(nullptr, Severity::style, "constParameterPointer", "Parameter 'x' can be declared with const");
        reportError(nullptr, Severity::style, "constVariablePointer", "Variable 'x' can be declared with const");
        reportError(nullptr, Severity::style, "constParameterCallback", "Parameter 'x' can be declared with const, however it seems that 'f' is a callback function.");
        return;
    }

    const std::string vartype(var->isArgument() ? "Parameter" : "Variable");
    const std::string& varname(var->name());
    const std::string ptrRefArray = var->isArray() ? "const array" : (var->isPointer() ? "pointer to const" : "reference to const");

    ErrorPath errorPath;
    std::string id = "const" + vartype;
    std::string message = "$symbol:" + varname + "\n" + vartype + " '$symbol' can be declared as " + ptrRefArray;
    errorPath.emplace_back(var->nameToken(), message);
    if (var->isArgument() && function && function->functionPointerUsage) {
        errorPath.emplace_front(function->functionPointerUsage, "You might need to cast the function pointer here");
        id += "Callback";
        message += ". However it seems that '" + function->name() + "' is a callback function, if '$symbol' is declared with const you might also need to cast function pointer(s).";
    } else if (var->isReference()) {
        id += "Reference";
    } else if (var->isPointer() && !var->isArray()) {
        id += "Pointer";
    }

    reportError(std::move(errorPath), Severity::style, id.c_str(), message, CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckOther::checkCharVariable()
{
    const bool warning = mSettings->severity.isEnabled(Severity::warning);
    const bool portability = mSettings->severity.isEnabled(Severity::portability);
    if (!warning && !portability)
        return;

    logChecker("CheckOther::checkCharVariable"); // warning,portability

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% [")) {
                if (!tok->variable())
                    continue;
                if (!tok->variable()->isArray() && !tok->variable()->isPointer())
                    continue;
                const Token *index = tok->next()->astOperand2();
                if (warning && tok->variable()->isArray() && astIsSignedChar(index) && index->getValueGE(0x80, *mSettings))
                    signedCharArrayIndexError(tok);
                if (portability && astIsUnknownSignChar(index) && index->getValueGE(0x80, *mSettings))
                    unknownSignCharArrayIndexError(tok);
            } else if (warning && Token::Match(tok, "[&|^]") && tok->isBinaryOp()) {
                bool warn = false;
                if (astIsSignedChar(tok->astOperand1())) {
                    const ValueFlow::Value *v1 = tok->astOperand1()->getValueLE(-1, *mSettings);
                    const ValueFlow::Value *v2 = tok->astOperand2()->getMaxValue(false);
                    if (!v1)
                        v1 = tok->astOperand1()->getValueGE(0x80, *mSettings);
                    if (v1 && !(tok->str() == "&" && v2 && v2->isKnown() && v2->intvalue >= 0 && v2->intvalue < 0x100))
                        warn = true;
                } else if (astIsSignedChar(tok->astOperand2())) {
                    const ValueFlow::Value *v1 = tok->astOperand2()->getValueLE(-1, *mSettings);
                    const ValueFlow::Value *v2 = tok->astOperand1()->getMaxValue(false);
                    if (!v1)
                        v1 = tok->astOperand2()->getValueGE(0x80, *mSettings);
                    if (v1 && !(tok->str() == "&" && v2 && v2->isKnown() && v2->intvalue >= 0 && v2->intvalue < 0x100))
                        warn = true;
                }

                // is the result stored in a short|int|long?
                if (warn && Token::simpleMatch(tok->astParent(), "=")) {
                    const Token *lhs = tok->astParent()->astOperand1();
                    if (lhs && lhs->valueType() && lhs->valueType()->type >= ValueType::Type::SHORT)
                        charBitOpError(tok); // This is an error..
                }
            }
        }
    }
}

void CheckOther::signedCharArrayIndexError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "signedCharArrayIndex",
                "Signed 'char' type used as array index.\n"
                "Signed 'char' type used as array index. If the value "
                "can be greater than 127 there will be a buffer underflow "
                "because of sign extension.", CWE128, Certainty::normal);
}

void CheckOther::unknownSignCharArrayIndexError(const Token *tok)
{
    reportError(tok,
                Severity::portability,
                "unknownSignCharArrayIndex",
                "'char' type used as array index.\n"
                "'char' type used as array index. Values greater than 127 will be "
                "treated depending on whether 'char' is signed or unsigned on target platform.", CWE758, Certainty::normal);
}

void CheckOther::charBitOpError(const Token *tok)
{
    reportError(tok,
                Severity::warning,
                "charBitOp",
                "When using 'char' variables in bit operations, sign extension can generate unexpected results.\n"
                "When using 'char' variables in bit operations, sign extension can generate unexpected results. For example:\n"
                "    char c = 0x80;\n"
                "    int i = 0 | c;\n"
                "    if (i & 0x8000)\n"
                "        printf(\"not expected\");\n"
                "The \"not expected\" will be printed on the screen.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// Incomplete statement..
//---------------------------------------------------------------------------

static bool isType(const Token * tok, bool unknown)
{
    if (tok && (tok->isStandardType() || (!tok->isKeyword() && Token::Match(tok, "%type%")) || tok->str() == "auto"))
        return true;
    if (tok && tok->varId())
        return false;
    if (Token::simpleMatch(tok, "::"))
        return isType(tok->astOperand2(), unknown);
    if (Token::simpleMatch(tok, "<") && tok->link())
        return true;
    if (unknown && Token::Match(tok, "%name% !!("))
        return true;
    return false;
}

static bool isVarDeclOp(const Token* tok)
{
    if (!tok)
        return false;
    const Token * vartok = tok->astOperand2();
    if (vartok && vartok->variable() && vartok->variable()->nameToken() == vartok)
        return true;
    const Token * typetok = tok->astOperand1();
    return isType(typetok, vartok && vartok->varId() != 0);
}

static bool isBracketAccess(const Token* tok)
{
    if (!Token::simpleMatch(tok, "[") || !tok->astOperand1())
        return false;
    tok = tok->astOperand1();
    if (tok->str() == ".")
        tok = tok->astOperand2();
    while (Token::simpleMatch(tok, "["))
        tok = tok->astOperand1();
    if (!tok || !tok->variable())
        return false;
    return tok->variable()->nameToken() != tok;
}

static bool isConstant(const Token* tok) {
    return tok && (tok->isEnumerator() || Token::Match(tok, "%bool%|%num%|%str%|%char%|nullptr|NULL"));
}

static bool isConstStatement(const Token *tok, const Library& library, bool platformIndependent, bool isNestedBracket = false)
{
    if (!tok)
        return false;
    if (tok->isExpandedMacro())
        return false;
    if (tok->varId() != 0)
        return true;
    if (isConstant(tok))
        return true;
    if (Token::Match(tok, "*|&|&&") &&
        (Token::Match(tok->previous(), "::|.|const|volatile|restrict") || isVarDeclOp(tok)))
        return false;
    if (Token::Match(tok, "<<|>>") && !astIsIntegral(tok, false))
        return false;
    const Token* tok2 = tok;
    while (tok2) {
        if (Token::simpleMatch(tok2->astOperand1(), "delete"))
            return false;
        tok2 = tok2->astParent();
    }
    if (Token::Match(tok, "&&|%oror%"))
        return isConstStatement(tok->astOperand1(), library, platformIndependent) && isConstStatement(tok->astOperand2(), library, platformIndependent);
    if (Token::Match(tok, "!|~|%cop%") && (tok->astOperand1() || tok->astOperand2()))
        return true;
    if (Token::simpleMatch(tok->previous(), "sizeof ("))
        return !platformIndependent;
    if (Token::Match(tok->previous(), "alignof|noexcept|typeid (") && tok->previous()->isKeyword())
        return true;
    if (isCPPCast(tok)) {
        if (Token::simpleMatch(tok->astOperand1(), "dynamic_cast") && Token::simpleMatch(tok->astOperand1()->linkAt(1)->previous(), "& >"))
            return false;
        return isWithoutSideEffects(tok) && isConstStatement(tok->astOperand2(), library, platformIndependent);
    }
    if (tok->isCast() && tok->next() && tok->next()->isStandardType())
        return isWithoutSideEffects(tok->astOperand1()) && isConstStatement(tok->astOperand1(), library, platformIndependent);
    if (Token::simpleMatch(tok, "."))
        return isConstStatement(tok->astOperand2(), library, platformIndependent);
    if (Token::simpleMatch(tok, ",")) {
        if (tok->astParent()) // warn about const statement on rhs at the top level
            return isConstStatement(tok->astOperand1(), library, platformIndependent) &&
                   isConstStatement(tok->astOperand2(), library, platformIndependent);

        const Token* lml = previousBeforeAstLeftmostLeaf(tok); // don't warn about matrix/vector assignment (e.g. Eigen)
        if (lml)
            lml = lml->next();
        const Token* stream = lml;
        while (stream && Token::Match(stream->astParent(), ".|[|(|*"))
            stream = stream->astParent();
        return (!stream || !isLikelyStream(stream)) && isConstStatement(tok->astOperand2(), library, platformIndependent);
    }
    if (Token::simpleMatch(tok, "?") && Token::simpleMatch(tok->astOperand2(), ":")) // ternary operator
        return isConstStatement(tok->astOperand1(), library, platformIndependent) &&
               isConstStatement(tok->astOperand2()->astOperand1(), library, platformIndependent) &&
               isConstStatement(tok->astOperand2()->astOperand2(), library, platformIndependent);
    if (isBracketAccess(tok) && isWithoutSideEffects(tok->astOperand1(), /*checkArrayAccess*/ true, /*checkReference*/ false)) {
        const bool isChained = succeeds(tok->astParent(), tok);
        if (Token::simpleMatch(tok->astParent(), "[")) {
            if (isChained)
                return isConstStatement(tok->astOperand2(), library, platformIndependent) &&
                       isConstStatement(tok->astParent(), library, platformIndependent);
            return isNestedBracket && isConstStatement(tok->astOperand2(), library, platformIndependent);
        }
        return isConstStatement(tok->astOperand2(), library, platformIndependent, /*isNestedBracket*/ !isChained);
    }
    if (!tok->astParent() && findLambdaEndToken(tok))
        return true;

    tok2 = tok;
    if (tok2->str() == "::")
        tok2 = tok2->next();
    if (Token::Match(tok2, "%name% ;")) {
        if (tok2->function())
            return true;
        std::string funcStr = tok2->str();
        while (tok2->index() > 1 && Token::Match(tok2->tokAt(-2), "%name% ::")) {
            funcStr.insert(0, tok2->strAt(-2) + "::");
            tok2 = tok2->tokAt(-2);
        }
        if (library.functions().count(funcStr) > 0)
            return true;
    }
    return false;
}

static bool isVoidStmt(const Token *tok)
{
    if (Token::simpleMatch(tok, "( void"))
        return true;
    if (isCPPCast(tok) && tok->astOperand1() && Token::Match(tok->astOperand1()->next(), "< void *| >"))
        return true;
    const Token *tok2 = tok;
    while (tok2->astOperand1())
        tok2 = tok2->astOperand1();
    if (Token::simpleMatch(tok2->previous(), ")") && Token::simpleMatch(tok2->linkAt(-1), "( void"))
        return true;
    if (Token::simpleMatch(tok2, "( void"))
        return true;
    return Token::Match(tok2->previous(), "delete|throw|return");
}

static bool isConstTop(const Token *tok)
{
    if (!tok)
        return false;
    if (!tok->astParent())
        return true;
    if (Token::simpleMatch(tok->astParent(), ";") &&
        Token::Match(tok->astTop()->previous(), "for|if (") && Token::simpleMatch(tok->astTop()->astOperand2(), ";")) {
        if (Token::simpleMatch(tok->astParent()->astParent(), ";"))
            return tok->astParent()->astOperand2() == tok;
        return tok->astParent()->astOperand1() == tok;
    }
    if (Token::simpleMatch(tok, "[")) {
        const Token* bracTok = tok;
        while (Token::simpleMatch(bracTok->astParent(), "["))
            bracTok = bracTok->astParent();
        if (!bracTok->astParent())
            return true;
    }
    if (tok->str() == "," && tok->astParent()->isAssignmentOp())
        return true;
    return false;
}

void CheckOther::checkIncompleteStatement()
{
    if (!mSettings->severity.isEnabled(Severity::warning) &&
        !mSettings->isPremiumEnabled("constStatement"))
        return;

    logChecker("CheckOther::checkIncompleteStatement"); // warning

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        const Scope *scope = tok->scope();
        if (scope && !scope->isExecutable())
            continue;
        if (!isConstTop(tok))
            continue;
        if (tok->str() == "," && Token::simpleMatch(tok->astTop()->previous(), "for ("))
            continue;

        // Do not warn for statement when both lhs and rhs has side effects:
        //   dostuff() || x=213;
        if (Token::Match(tok, "%oror%|&&")) {
            bool warn = false;
            visitAstNodes(tok, [&warn](const Token *child) {
                if (Token::Match(child, "%oror%|&&"))
                    return ChildrenToVisit::op1_and_op2;
                if (child->isAssignmentOp())
                    return ChildrenToVisit::none;
                if (child->tokType() == Token::Type::eIncDecOp)
                    return ChildrenToVisit::none;
                if (Token::Match(child->previous(), "%name% ("))
                    return ChildrenToVisit::none;
                warn = true;
                return ChildrenToVisit::done;
            });
            if (!warn)
                continue;
        }

        const Token *rtok = nextAfterAstRightmostLeaf(tok);
        if (!Token::simpleMatch(tok->astParent(), ";") && !Token::simpleMatch(rtok, ";") &&
            !Token::Match(tok->previous(), ";|}|{ %any% ;") &&
            !(tok->isCpp() && tok->isCast() && !tok->astParent()) &&
            !(!tok->astParent() && tok->astOperand1() && Token::Match(tok->previous(), "%name% (") && tok->previous()->isKeyword()) &&
            !Token::simpleMatch(tok->tokAt(-2), "for (") &&
            !Token::Match(tok->tokAt(-1), "%var% [") &&
            !(tok->str() == "," && tok->astParent() && tok->astParent()->isAssignmentOp()))
            continue;
        // Skip statement expressions
        if (Token::simpleMatch(rtok, "; } )"))
            continue;
        if (!isConstStatement(tok, mSettings->library, false))
            continue;
        if (isVoidStmt(tok))
            continue;
        if (tok->isCpp() && tok->str() == "&" && !(tok->astOperand1() && tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->isIntegral()))
            // Possible archive
            continue;
        const bool inconclusive = tok->isConstOp() && !mSettings->isPremiumEnabled("constStatement");
        if (mSettings->certainty.isEnabled(Certainty::inconclusive) || !inconclusive)
            constStatementError(tok, tok->isNumber() ? "numeric" : "string", inconclusive);
    }
}

void CheckOther::constStatementError(const Token *tok, const std::string &type, bool inconclusive)
{
    const Token *valueTok = tok;
    while (valueTok && valueTok->isCast())
        valueTok = valueTok->astOperand2() ? valueTok->astOperand2() : valueTok->astOperand1();

    std::string msg;
    if (Token::simpleMatch(tok, "=="))
        msg = "Found suspicious equality comparison. Did you intend to assign a value instead?";
    else if (Token::Match(tok, ",|!|~|%cop%"))
        msg = "Found suspicious operator '" + tok->str() + "', result is not used.";
    else if (Token::Match(tok, "%var%"))
        msg = "Unused variable value '" + tok->str() + "'";
    else if (isConstant(valueTok)) {
        std::string typeStr("string");
        if (valueTok->isNumber())
            typeStr = "numeric";
        else if (valueTok->isBoolean())
            typeStr = "bool";
        else if (valueTok->tokType() == Token::eChar)
            typeStr = "character";
        else if (isNullOperand(valueTok))
            typeStr = "NULL";
        else if (valueTok->isEnumerator())
            typeStr = "enumerator";
        msg = "Redundant code: Found a statement that begins with " + typeStr + " constant.";
    }
    else if (!tok)
        msg = "Redundant code: Found a statement that begins with " + type + " constant.";
    else if (tok->isCast() && tok->tokType() == Token::Type::eExtendedOp) {
        msg = "Redundant code: Found unused cast ";
        msg += valueTok ? "of expression '" + valueTok->expressionString() + "'." : "expression.";
    }
    else if (tok->str() == "?" && tok->tokType() == Token::Type::eExtendedOp)
        msg = "Redundant code: Found unused result of ternary operator.";
    else if (tok->str() == "." && tok->tokType() == Token::Type::eOther)
        msg = "Redundant code: Found unused member access.";
    else if (tok->str() == "[" && tok->tokType() == Token::Type::eExtendedOp)
        msg = "Redundant code: Found unused array access.";
    else if (tok->str() == "[" && !tok->astParent())
        msg = "Redundant code: Found unused lambda.";
    else if (Token::Match(tok, "%name%|::"))
        msg = "Redundant code: Found unused function.";
    else if (Token::Match(tok->previous(), "%name% ("))
        msg = "Redundant code: Found unused '" + tok->strAt(-1) + "' expression.";
    else if (mSettings->debugwarnings) {
        reportError(tok, Severity::debug, "debug", "constStatementError not handled.");
        return;
    }
    reportError(tok, Severity::warning, "constStatement", msg, CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// Detect division by zero.
//---------------------------------------------------------------------------
void CheckOther::checkZeroDivision()
{
    logChecker("CheckOther::checkZeroDivision");

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->astOperand2() || !tok->astOperand1())
            continue;
        if (tok->str() != "%" && tok->str() != "/" && tok->str() != "%=" && tok->str() != "/=")
            continue;
        if (!tok->valueType() || !tok->valueType()->isIntegral())
            continue;
        if (tok->scope() && tok->scope()->type == ScopeType::eEnum) // don't warn for compile-time error
            continue;

        // Value flow..
        const ValueFlow::Value *value = tok->astOperand2()->getValue(0LL);
        if (value && mSettings->isEnabled(value, false))
            zerodivError(tok, value);
    }
}

void CheckOther::zerodivError(const Token *tok, const ValueFlow::Value *value)
{
    if (!tok && !value) {
        reportError(tok, Severity::error, "zerodiv", "Division by zero.", CWE369, Certainty::normal);
        reportError(tok, Severity::warning, "zerodivcond", ValueFlow::eitherTheConditionIsRedundant(nullptr) + " or there is division by zero.", CWE369, Certainty::normal);
        return;
    }

    ErrorPath errorPath = getErrorPath(tok, value, "Division by zero");

    std::ostringstream errmsg;
    if (value->condition) {
        const int line = tok ? tok->linenr() : 0;
        errmsg << ValueFlow::eitherTheConditionIsRedundant(value->condition)
               << " or there is division by zero at line " << line << ".";
    } else
        errmsg << "Division by zero.";

    reportError(std::move(errorPath),
                value->errorSeverity() ? Severity::error : Severity::warning,
                value->condition ? "zerodivcond" : "zerodiv",
                errmsg.str(), CWE369, value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// Check for NaN (not-a-number) in an arithmetic expression, e.g.
// double d = 1.0 / 0.0 + 100.0;
//---------------------------------------------------------------------------

void CheckOther::checkNanInArithmeticExpression()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("nanInArithmeticExpression"))
        return;
    logChecker("CheckOther::checkNanInArithmeticExpression"); // style
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "/")
            continue;
        if (!Token::Match(tok->astParent(), "[+-]"))
            continue;
        if (Token::simpleMatch(tok->astOperand2(), "0.0"))
            nanInArithmeticExpressionError(tok);
    }
}

void CheckOther::nanInArithmeticExpressionError(const Token *tok)
{
    reportError(tok, Severity::style, "nanInArithmeticExpression",
                "Using NaN/Inf in a computation.\n"
                "Using NaN/Inf in a computation. "
                "Although nothing bad really happens, it is suspicious.", CWE369, Certainty::normal);
}

//---------------------------------------------------------------------------
// Creating instance of classes which are destroyed immediately
//---------------------------------------------------------------------------
void CheckOther::checkMisusedScopedObject()
{
    // Skip this check for .c files
    if (mTokenizer->isC())
        return;

    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("unusedScopedObject"))
        return;

    logChecker("CheckOther::checkMisusedScopedObject"); // style,c++

    auto getConstructorTok = [](const Token* tok, std::string& typeStr) -> const Token* {
        if (!Token::Match(tok, "[;{}] %name%") || tok->next()->isKeyword())
            return nullptr;
        tok = tok->next();
        typeStr.clear();
        while (Token::Match(tok, "%name% ::")) {
            typeStr += tok->str();
            typeStr += "::";
            tok = tok->tokAt(2);
        }
        typeStr += tok->str();
        const Token* endTok = tok;
        if (Token::Match(endTok, "%name% <"))
            endTok = endTok->linkAt(1);
        if (Token::Match(endTok, "%name%|> (|{") && Token::Match(endTok->linkAt(1), ")|} ;") &&
            !Token::simpleMatch(endTok->next()->astParent(), ";")) { // for loop condition
            return tok;
        }
        return nullptr;
    };

    auto isLibraryConstructor = [&](const Token* tok, const std::string& typeStr) -> bool {
        const Library::TypeCheck typeCheck = mSettings->library.getTypeCheck("unusedvar", typeStr);
        if (typeCheck == Library::TypeCheck::check || typeCheck == Library::TypeCheck::checkFiniteLifetime)
            return true;
        return mSettings->library.detectContainerOrIterator(tok);
    };

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    std::string typeStr;
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            const Token* ctorTok = getConstructorTok(tok, typeStr);
            if (ctorTok && (((ctorTok->type() || ctorTok->isStandardType() || (ctorTok->function() && ctorTok->function()->isConstructor())) // TODO: The rhs of || should be removed; It is a workaround for a symboldatabase bug
                             && (!ctorTok->function() || ctorTok->function()->isConstructor()) // // is not a function on this scope or is function in this scope and it's a ctor
                             && ctorTok->str() != "void") || isLibraryConstructor(tok->next(), typeStr))) {
                const Token* parTok = ctorTok->next();
                if (Token::simpleMatch(parTok, "<") && parTok->link())
                    parTok = parTok->link()->next();
                if (const Token* arg = parTok->astOperand2()) {
                    if (!isConstStatement(arg, mSettings->library, false))
                        continue;
                    if (parTok->str() == "(") {
                        if (arg->varId() && !(arg->variable() && arg->variable()->nameToken() != arg))
                            continue;
                        const Token* rml = nextAfterAstRightmostLeaf(arg);
                        if (rml && rml->previous() && rml->previous()->varId())
                            continue;
                    }
                }
                tok = tok->next();
                misusedScopeObjectError(ctorTok, typeStr);
                tok = tok->next();
            }
            if (tok->isAssignmentOp() && Token::simpleMatch(tok->astOperand1(), "(") && tok->astOperand1()->astOperand1()) {
                if (const Function* ftok = tok->astOperand1()->astOperand1()->function()) {
                    if (ftok->retType && Token::Match(ftok->retType->classDef, "class|struct|union") && !Function::returnsReference(ftok, /*unknown*/ false, /*includeRValueRef*/ true))
                        misusedScopeObjectError(tok->next(), ftok->retType->name(), /*isAssignment*/ true);
                }
            }
        }
    }
}

void CheckOther::misusedScopeObjectError(const Token *tok, const std::string& varname, bool isAssignment)
{
    std::string msg = "Instance of '$symbol' object is destroyed immediately";
    msg += isAssignment ? ", assignment has no effect." : ".";
    reportError(tok, Severity::style,
                "unusedScopedObject",
                "$symbol:" + varname + "\n" +
                msg, CWE563, Certainty::normal);
}

static const Token * getSingleExpressionInBlock(const Token * tok)
{
    if (!tok)
        return nullptr;
    const Token * top = tok->astTop();
    const Token * nextExpression = nextAfterAstRightmostLeaf(top);
    if (!Token::simpleMatch(nextExpression, "; }"))
        return nullptr;
    return top;
}

//-----------------------------------------------------------------------------
// check for duplicate code in if and else branches
// if (a) { b = true; } else { b = true; }
//-----------------------------------------------------------------------------
void CheckOther::checkDuplicateBranch()
{
    // This is inconclusive since in practice most warnings are noise:
    // * There can be unfixed low-priority todos. The code is fine as it
    //   is but it could be possible to enhance it. Writing a warning
    //   here is noise since the code is fine (see cppcheck, abiword, ..)
    // * There can be overspecified code so some conditions can't be true
    //   and their conditional code is a duplicate of the condition that
    //   is always true just in case it would be false. See for instance
    //   abiword.
    if (!mSettings->severity.isEnabled(Severity::style) || !mSettings->certainty.isEnabled(Certainty::inconclusive))
        return;

    logChecker("CheckOther::checkDuplicateBranch"); // style,inconclusive

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope & scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eIf)
            continue;

        // check all the code in the function for if (..) else
        if (Token::simpleMatch(scope.bodyEnd, "} else {")) {
            // Make sure there are no macros (different macros might be expanded
            // to the same code)
            bool macro = false;
            for (const Token *tok = scope.bodyStart; tok != scope.bodyEnd->linkAt(2); tok = tok->next()) {
                if (tok->isExpandedMacro()) {
                    macro = true;
                    break;
                }
            }
            if (macro)
                continue;

            const Token * const tokIf = scope.bodyStart->next();
            const Token * const tokElse = scope.bodyEnd->tokAt(3);

            // compare first tok before stringifying the whole blocks
            const std::string tokIfStr = tokIf->stringify(false, true, false);
            if (tokIfStr.empty())
                continue;

            const std::string tokElseStr = tokElse->stringify(false, true, false);

            if (tokIfStr == tokElseStr) {
                // save if branch code
                const std::string branch1 = tokIf->stringifyList(scope.bodyEnd);

                if (branch1.empty())
                    continue;

                // save else branch code
                const std::string branch2 = tokElse->stringifyList(scope.bodyEnd->linkAt(2));

                // check for duplicates
                if (branch1 == branch2) {
                    duplicateBranchError(scope.classDef, scope.bodyEnd->next(), ErrorPath{});
                    continue;
                }
            }

            // check for duplicates using isSameExpression
            const Token * branchTop1 = getSingleExpressionInBlock(tokIf);
            if (!branchTop1)
                continue;
            const Token * branchTop2 = getSingleExpressionInBlock(tokElse);
            if (!branchTop2)
                continue;
            if (branchTop1->str() != branchTop2->str())
                continue;
            ErrorPath errorPath;
            if (isSameExpression(false, branchTop1->astOperand1(), branchTop2->astOperand1(), *mSettings, true, true, &errorPath) &&
                isSameExpression(false, branchTop1->astOperand2(), branchTop2->astOperand2(), *mSettings, true, true, &errorPath))
                duplicateBranchError(scope.classDef, scope.bodyEnd->next(), std::move(errorPath));
        }
    }
}

void CheckOther::duplicateBranchError(const Token *tok1, const Token *tok2, ErrorPath errors)
{
    errors.emplace_back(tok2, "");
    errors.emplace_back(tok1, "");

    reportError(std::move(errors), Severity::style, "duplicateBranch", "Found duplicate branches for 'if' and 'else'.\n"
                "Finding the same code in an 'if' and related 'else' branch is suspicious and "
                "might indicate a cut and paste or logic error. Please examine this code "
                "carefully to determine if it is correct.", CWE398, Certainty::inconclusive);
}


//-----------------------------------------------------------------------------
// Check for a free() of an invalid address
// char* p = malloc(100);
// free(p + 10);
//-----------------------------------------------------------------------------
void CheckOther::checkInvalidFree()
{
    std::map<int, bool> inconclusive;
    std::map<int, std::string> allocation;

    logChecker("CheckOther::checkInvalidFree");

    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok && tok != scope->bodyEnd; tok = tok->next()) {

            // Keep track of which variables were assigned addresses to newly-allocated memory
            if ((tok->isCpp() && Token::Match(tok, "%var% = new")) ||
                (Token::Match(tok, "%var% = %name% (") && mSettings->library.getAllocFuncInfo(tok->tokAt(2)))) {
                allocation.emplace(tok->varId(), tok->strAt(2));
                inconclusive.emplace(tok->varId(), false);
            }

            // If a previously-allocated pointer is incremented or decremented, any subsequent
            // free involving pointer arithmetic may or may not be invalid, so we should only
            // report an inconclusive result.
            else if (Token::Match(tok, "%var% = %name% +|-") &&
                     tok->varId() == tok->tokAt(2)->varId() &&
                     allocation.find(tok->varId()) != allocation.end()) {
                if (printInconclusive)
                    inconclusive[tok->varId()] = true;
                else {
                    allocation.erase(tok->varId());
                    inconclusive.erase(tok->varId());
                }
            }

            // If a previously-allocated pointer is assigned a completely new value,
            // we can't know if any subsequent free() on that pointer is valid or not.
            else if (Token::Match(tok, "%var% =")) {
                allocation.erase(tok->varId());
                inconclusive.erase(tok->varId());
            }

            // If a variable that was previously assigned a newly-allocated memory location is
            // added or subtracted from when used to free the memory, report an error.
            else if ((Token::Match(tok, "%name% ( %any% +|-") && mSettings->library.getDeallocFuncInfo(tok)) ||
                     Token::Match(tok, "delete [ ] ( %any% +|-") ||
                     Token::Match(tok, "delete %any% +|- %any%")) {

                const int varIndex = tok->strAt(1) == "(" ? 2 :
                                     tok->strAt(3) == "(" ? 4 : 1;
                const int var1 = tok->tokAt(varIndex)->varId();
                const int var2 = tok->tokAt(varIndex + 2)->varId();
                const auto alloc1 = utils::as_const(inconclusive).find(var1);
                const auto alloc2 = utils::as_const(inconclusive).find(var2);
                if (alloc1 != inconclusive.end()) {
                    invalidFreeError(tok, allocation[var1], alloc1->second);
                } else if (alloc2 != inconclusive.end()) {
                    invalidFreeError(tok, allocation[var2], alloc2->second);
                }
            }

            // If the previously-allocated variable is passed in to another function
            // as a parameter, it might be modified, so we shouldn't report an error
            // if it is later used to free memory
            else if (Token::Match(tok, "%name% (") && !mSettings->library.isFunctionConst(tok->str(), true)) {
                const Token* tok2 = Token::findmatch(tok->next(), "%var%", tok->linkAt(1));
                while (tok2 != nullptr) {
                    allocation.erase(tok->varId());
                    inconclusive.erase(tok2->varId());
                    tok2 = Token::findmatch(tok2->next(), "%var%", tok->linkAt(1));
                }
            }
        }
    }
}

void CheckOther::invalidFreeError(const Token *tok, const std::string &allocation, bool inconclusive)
{
    std::string alloc = allocation;
    if (alloc != "new")
        alloc += "()";
    std::string deallocated = (alloc == "new") ? "deleted" : "freed";
    reportError(tok, Severity::error, "invalidFree", "Mismatching address is " + deallocated + ". The address you get from " + alloc + " must be " + deallocated + " without offset.", CWE(0U), inconclusive ? Certainty::inconclusive : Certainty::normal);
}


//---------------------------------------------------------------------------
// check for the same expression on both sides of an operator
// (x == x), (x && x), (x || x)
// (x.y == x.y), (x.y && x.y), (x.y || x.y)
//---------------------------------------------------------------------------

namespace {
    bool notconst(const Function* func)
    {
        return !func->isConst();
    }

    void getConstFunctions(const SymbolDatabase *symbolDatabase, std::list<const Function*> &constFunctions)
    {
        for (const Scope &scope : symbolDatabase->scopeList) {
            // only add const functions that do not have a non-const overloaded version
            // since it is pretty much impossible to tell which is being called.
            using StringFunctionMap = std::map<std::string, std::list<const Function*>>;
            StringFunctionMap functionsByName;
            for (const Function &func : scope.functionList) {
                functionsByName[func.tokenDef->str()].push_back(&func);
            }
            for (std::pair<const std::string, std::list<const Function*>>& it : functionsByName) {
                const auto nc = std::find_if(it.second.cbegin(), it.second.cend(), notconst);
                if (nc == it.second.cend()) {
                    // ok to add all of them
                    constFunctions.splice(constFunctions.end(), it.second);
                }
            }
        }
    }
}

static bool
isStaticAssert(const Settings &settings, const Token *tok)
{
    if (tok->isCpp() && settings.standards.cpp >= Standards::CPP11 &&
        Token::simpleMatch(tok, "static_assert")) {
        return true;
    }

    if (tok->isC() && settings.standards.c >= Standards::C11 &&
        Token::simpleMatch(tok, "_Static_assert")) {
        return true;
    }

    return false;
}

void CheckOther::checkDuplicateExpression()
{
    {
        const bool styleEnabled = mSettings->severity.isEnabled(Severity::style);
        const bool premiumEnabled = mSettings->isPremiumEnabled("oppositeExpression") ||
                                    mSettings->isPremiumEnabled("duplicateExpression") ||
                                    mSettings->isPremiumEnabled("duplicateAssignExpression") ||
                                    mSettings->isPremiumEnabled("duplicateExpressionTernary") ||
                                    mSettings->isPremiumEnabled("duplicateValueTernary") ||
                                    mSettings->isPremiumEnabled("selfAssignment") ||
                                    mSettings->isPremiumEnabled("knownConditionTrueFalse");

        if (!styleEnabled && !premiumEnabled)
            return;
    }

    logChecker("CheckOther::checkDuplicateExpression"); // style,warning

    // Parse all executing scopes..
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    std::list<const Function*> constFunctions;
    getConstFunctions(symbolDatabase, constFunctions);

    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->str() == "=" && Token::Match(tok->astOperand1(), "%var%")) {
                const Token * endStatement = Token::findsimplematch(tok, ";");
                if (Token::Match(endStatement, "; %type% %var% ;")) {
                    endStatement = endStatement->tokAt(4);
                }
                if (Token::Match(endStatement, "%var% %assign%")) {
                    const Token * nextAssign = endStatement->tokAt(1);
                    const Token * var1 = tok->astOperand1();
                    const Token * var2 = nextAssign->astOperand1();
                    if (var1 && var2 &&
                        Token::Match(var1->previous(), ";|{|} %var%") &&
                        Token::Match(var2->previous(), ";|{|} %var%") &&
                        var2->valueType() && var1->valueType() &&
                        var2->valueType()->originalTypeName == var1->valueType()->originalTypeName &&
                        var2->valueType()->pointer == var1->valueType()->pointer &&
                        var2->valueType()->constness == var1->valueType()->constness &&
                        var2->varId() != var1->varId() && (
                            tok->astOperand2()->isArithmeticalOp() ||
                            tok->astOperand2()->str() == "." ||
                            Token::Match(tok->astOperand2()->previous(), "%name% (")
                            ) &&
                        tok->next()->tokType() != Token::eType &&
                        isSameExpression(true, tok->next(), nextAssign->next(), *mSettings, true, false) &&
                        isSameExpression(true, tok->astOperand2(), nextAssign->astOperand2(), *mSettings, true, false) &&
                        tok->astOperand2()->expressionString() == nextAssign->astOperand2()->expressionString()) {
                        bool differentDomain = false;
                        const Scope * varScope = var1->scope() ? var1->scope() : scope;
                        const Token* assignTok = Token::findsimplematch(var2, ";");
                        for (; assignTok && assignTok != varScope->bodyEnd; assignTok = assignTok->next()) {
                            if (!Token::Match(assignTok, "%assign%|%comp%"))
                                continue;
                            if (!assignTok->astOperand1())
                                continue;
                            if (!assignTok->astOperand2())
                                continue;

                            if (assignTok->astOperand1()->varId() != var1->varId() &&
                                assignTok->astOperand1()->varId() != var2->varId() &&
                                !isSameExpression(true,
                                                  tok->astOperand2(),
                                                  assignTok->astOperand1(),
                                                  *mSettings,
                                                  true,
                                                  true))
                                continue;
                            if (assignTok->astOperand2()->varId() != var1->varId() &&
                                assignTok->astOperand2()->varId() != var2->varId() &&
                                !isSameExpression(true,
                                                  tok->astOperand2(),
                                                  assignTok->astOperand2(),
                                                  *mSettings,
                                                  true,
                                                  true))
                                continue;
                            differentDomain = true;
                            break;
                        }
                        if (!differentDomain && !isUniqueExpression(tok->astOperand2()))
                            duplicateAssignExpressionError(var1, var2, false);
                        else if (mSettings->certainty.isEnabled(Certainty::inconclusive)) {
                            diag(assignTok);
                            duplicateAssignExpressionError(var1, var2, true);
                        }
                    }
                }
            }
            auto isInsideLambdaCaptureList = [](const Token* tok) {
                const Token* parent = tok->astParent();
                while (Token::simpleMatch(parent, ","))
                    parent = parent->astParent();
                return isLambdaCaptureList(parent);
            };
            ErrorPath errorPath;
            if (tok->isOp() && tok->astOperand1() && !Token::Match(tok, "+|*|<<|>>|+=|*=|<<=|>>=") && !isInsideLambdaCaptureList(tok)) {
                if (Token::Match(tok, "==|!=|-") && astIsFloat(tok->astOperand1(), true))
                    continue;
                if (tok->isArithmeticalOp() &&
                    (tok->astOperand1()->isEnumerator() || (tok->astOperand2() && tok->astOperand2()->isEnumerator())))
                    continue;
                const bool pointerDereference = (tok->astOperand1() && tok->astOperand1()->isUnaryOp("*")) ||
                                                (tok->astOperand2() && tok->astOperand2()->isUnaryOp("*"));
                const bool followVar = (!isConstVarExpression(tok) || Token::Match(tok, "%comp%|%oror%|&&")) && !pointerDereference;
                if (isSameExpression(true,
                                     tok->astOperand1(),
                                     tok->astOperand2(),
                                     *mSettings,
                                     true,
                                     followVar,
                                     &errorPath)) {
                    if (isWithoutSideEffects(tok->astOperand1())) {
                        const Token* loopTok = isInLoopCondition(tok);
                        if (!loopTok ||
                            !findExpressionChanged(tok, tok, loopTok->link()->linkAt(1), *mSettings)) {
                            const bool isEnum = tok->scope()->type == ScopeType::eEnum;
                            const bool assignment = !isEnum && tok->str() == "=";
                            if (assignment)
                                selfAssignmentError(tok, tok->astOperand1()->expressionString());
                            else if (!isEnum) {
                                if (tok->str() == "==") {
                                    const Token* parent = tok->astParent();
                                    while (parent && parent->astParent()) {
                                        parent = parent->astParent();
                                    }
                                    if (parent && parent->previous() &&
                                        (isStaticAssert(*mSettings, parent->previous()) ||
                                         Token::simpleMatch(parent->previous(), "assert"))) {
                                        continue;
                                    }
                                }
                                duplicateExpressionError(tok->astOperand1(), tok->astOperand2(), tok, std::move(errorPath));
                            }
                        }
                    }
                } else if (tok->str() == "=" && Token::simpleMatch(tok->astOperand2(), "=") &&
                           isSameExpression(false,
                                            tok->astOperand1(),
                                            tok->astOperand2()->astOperand1(),
                                            *mSettings,
                                            true,
                                            false)) {
                    if (isWithoutSideEffects(tok->astOperand1())) {
                        selfAssignmentError(tok, tok->astOperand1()->expressionString());
                    }
                } else if (isOppositeExpression(tok->astOperand1(),
                                                tok->astOperand2(),
                                                *mSettings,
                                                false,
                                                true,
                                                &errorPath) &&
                           !Token::Match(tok, "=|-|-=|/|/=") &&
                           isWithoutSideEffects(tok->astOperand1())) {
                    oppositeExpressionError(tok, std::move(errorPath));
                } else if (!Token::Match(tok, "[-/%]")) { // These operators are not associative
                    if (tok->astOperand2() && tok->str() == tok->astOperand1()->str() &&
                        isSameExpression(true,
                                         tok->astOperand2(),
                                         tok->astOperand1()->astOperand2(),
                                         *mSettings,
                                         true,
                                         followVar,
                                         &errorPath) &&
                        isWithoutSideEffects(tok->astOperand2()))
                        duplicateExpressionError(tok->astOperand2(), tok->astOperand1()->astOperand2(), tok, std::move(errorPath));
                    else if (tok->astOperand2() && isConstExpression(tok->astOperand1(), mSettings->library)) {
                        auto checkDuplicate = [&](const Token* exp1, const Token* exp2, const Token* ast1) {
                            if (isSameExpression(true, exp1, exp2, *mSettings, true, true, &errorPath) &&
                                isWithoutSideEffects(exp1) &&
                                isWithoutSideEffects(ast1->astOperand2()))
                                duplicateExpressionError(exp1, exp2, tok, errorPath, /*hasMultipleExpr*/ true);
                        };
                        const Token *ast1 = tok->astOperand1();
                        while (ast1 && tok->str() == ast1->str()) { // chain of identical operators
                            checkDuplicate(ast1->astOperand2(), tok->astOperand2(), ast1);
                            if (ast1->astOperand1() && ast1->astOperand1()->str() != tok->str()) // check first condition in the chain
                                checkDuplicate(ast1->astOperand1(), tok->astOperand2(), ast1);
                            ast1 = ast1->astOperand1();
                        }
                    }
                }
            } else if (tok->astOperand1() && tok->astOperand2() && tok->str() == ":" && tok->astParent() && tok->astParent()->str() == "?") {
                if (isSameExpression(true, tok->astOperand1(), tok->astOperand2(), *mSettings, false, true, &errorPath))
                    duplicateExpressionTernaryError(tok, std::move(errorPath));

                else if (!tok->astOperand1()->values().empty() && !tok->astOperand2()->values().empty() && isEqualKnownValue(tok->astOperand1(), tok->astOperand2()) &&
                         !isVariableChanged(tok->astParent(), /*indirect*/ 0, *mSettings) &&
                         isConstStatement(tok->astOperand1(), mSettings->library, true) && isConstStatement(tok->astOperand2(), mSettings->library, true))
                    duplicateValueTernaryError(tok);
            }
        }
    }
}

void CheckOther::oppositeExpressionError(const Token *opTok, ErrorPath errors)
{
    errors.emplace_back(opTok, "");

    const std::string& op = opTok ? opTok->str() : "&&";

    reportError(std::move(errors), Severity::style, "oppositeExpression", "Opposite expression on both sides of \'" + op + "\'.\n"
                "Finding the opposite expression on both sides of an operator is suspicious and might "
                "indicate a cut and paste or logic error. Please examine this code carefully to "
                "determine if it is correct.", CWE398, Certainty::normal);
}

void CheckOther::duplicateExpressionError(const Token *tok1, const Token *tok2, const Token *opTok, ErrorPath errors, bool hasMultipleExpr)
{
    errors.emplace_back(opTok, "");

    const std::string& expr1 = tok1 ? tok1->expressionString() : "x";
    const std::string& expr2 = tok2 ? tok2->expressionString() : "x";

    const std::string& op = opTok ? opTok->str() : "&&";
    std::string msg = "Same expression " + (hasMultipleExpr ? "\'" + expr1 + "\'" + " found multiple times in chain of \'" + op + "\' operators" : "on both sides of \'" + op + "\'");
    const char *id = "duplicateExpression";
    if (expr1 != expr2 && (!opTok || Token::Match(opTok, "%oror%|%comp%|&&|?|!"))) {
        id = "knownConditionTrueFalse";
        std::string exprMsg = "The comparison \'" + expr1 + " " + op +  " " + expr2 + "\' is always ";
        if (Token::Match(opTok, "==|>=|<="))
            msg = exprMsg + "true";
        else if (Token::Match(opTok, "!=|>|<"))
            msg = exprMsg + "false";
    }

    if (expr1 != expr2 && !Token::Match(tok1, "%num%|NULL|nullptr") && !Token::Match(tok2, "%num%|NULL|nullptr"))
        msg += " because '" + expr1 + "' and '" + expr2 + "' represent the same value";

    reportError(std::move(errors), Severity::style, id, msg +
                (std::string(".\nFinding the same expression ") + (hasMultipleExpr ? "more than once in a condition" : "on both sides of an operator")) +
                " is suspicious and might indicate a cut and paste or logic error. Please examine this code carefully to "
                "determine if it is correct.", CWE398, Certainty::normal);
}

void CheckOther::duplicateAssignExpressionError(const Token *tok1, const Token *tok2, bool inconclusive)
{
    const std::list<const Token *> toks = { tok2, tok1 };

    const std::string& var1 = tok1 ? tok1->str() : "x";
    const std::string& var2 = tok2 ? tok2->str() : "x";

    reportError(toks, Severity::style, "duplicateAssignExpression",
                "Same expression used in consecutive assignments of '" + var1 + "' and '" + var2 + "'.\n"
                "Finding variables '" + var1 + "' and '" + var2 + "' that are assigned the same expression "
                "is suspicious and might indicate a cut and paste or logic error. Please examine this code carefully to "
                "determine if it is correct.", CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckOther::duplicateExpressionTernaryError(const Token *tok, ErrorPath errors)
{
    errors.emplace_back(tok, "");
    reportError(std::move(errors), Severity::style, "duplicateExpressionTernary", "Same expression in both branches of ternary operator.\n"
                "Finding the same expression in both branches of ternary operator is suspicious as "
                "the same code is executed regardless of the condition.", CWE398, Certainty::normal);
}

void CheckOther::duplicateValueTernaryError(const Token *tok)
{
    reportError(tok, Severity::style, "duplicateValueTernary", "Same value in both branches of ternary operator.\n"
                "Finding the same value in both branches of ternary operator is suspicious as "
                "the same code is executed regardless of the condition.", CWE398, Certainty::normal);
}

void CheckOther::selfAssignmentError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style,
                "selfAssignment",
                "$symbol:" + varname + "\n"
                "Redundant assignment of '$symbol' to itself.", CWE398, Certainty::normal);
}

//-----------------------------------------------------------------------------
// Check is a comparison of two variables leads to condition, which is
// always true or false.
// For instance: int a = 1; if(isless(a,a)){...}
// In this case isless(a,a) always evaluates to false.
//
// Reference:
// - http://www.cplusplus.com/reference/cmath/
//-----------------------------------------------------------------------------
void CheckOther::checkComparisonFunctionIsAlwaysTrueOrFalse()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckOther::checkComparisonFunctionIsAlwaysTrueOrFalse"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->isName() && Token::Match(tok, "isgreater|isless|islessgreater|isgreaterequal|islessequal ( %var% , %var% )")) {
                const int varidLeft = tok->tokAt(2)->varId();// get the left varid
                const int varidRight = tok->tokAt(4)->varId();// get the right varid
                // compare varids: if they are not zero but equal
                // --> the comparison function is called with the same variables
                if (varidLeft == varidRight) {
                    const std::string& functionName = tok->str(); // store function name
                    const std::string& varNameLeft = tok->strAt(2); // get the left variable name
                    if (functionName == "isgreater" || functionName == "isless" || functionName == "islessgreater") {
                        // e.g.: isgreater(x,x) --> (x)>(x) --> false
                        checkComparisonFunctionIsAlwaysTrueOrFalseError(tok, functionName, varNameLeft, false);
                    } else { // functionName == "isgreaterequal" || functionName == "islessequal"
                        // e.g.: isgreaterequal(x,x) --> (x)>=(x) --> true
                        checkComparisonFunctionIsAlwaysTrueOrFalseError(tok, functionName, varNameLeft, true);
                    }
                }
            }
        }
    }
}
void CheckOther::checkComparisonFunctionIsAlwaysTrueOrFalseError(const Token* tok, const std::string &functionName, const std::string &varName, const bool result)
{
    const std::string strResult = bool_to_string(result);
    const CWE cweResult = result ? CWE571 : CWE570;

    reportError(tok, Severity::warning, "comparisonFunctionIsAlwaysTrueOrFalse",
                "$symbol:" + functionName + "\n"
                "Comparison of two identical variables with $symbol(" + varName + "," + varName + ") always evaluates to " + strResult + ".\n"
                "The function $symbol is designed to compare two variables. Calling this function with one variable (" + varName + ") "
                "for both parameters leads to a statement which is always " + strResult + ".", cweResult, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check testing sign of unsigned variables and pointers.
//---------------------------------------------------------------------------
void CheckOther::checkSignOfUnsignedVariable()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("unsignedLessThanZero"))
        return;

    logChecker("CheckOther::checkSignOfUnsignedVariable"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
        // check all the code in the function
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            const ValueFlow::Value *zeroValue = nullptr;
            const Token *nonZeroExpr = nullptr;
            if (comparisonNonZeroExpressionLessThanZero(tok, zeroValue, nonZeroExpr)) {
                const ValueType* vt = nonZeroExpr->valueType();
                if (vt->pointer)
                    pointerLessThanZeroError(tok, zeroValue);
                else
                    unsignedLessThanZeroError(tok, zeroValue, nonZeroExpr->expressionString());
            } else if (testIfNonZeroExpressionIsPositive(tok, zeroValue, nonZeroExpr)) {
                const ValueType* vt = nonZeroExpr->valueType();
                if (vt->pointer)
                    pointerPositiveError(tok, zeroValue);
                else
                    unsignedPositiveError(tok, zeroValue, nonZeroExpr->expressionString());
            }
        }
    }
}

bool CheckOther::comparisonNonZeroExpressionLessThanZero(const Token *tok, const ValueFlow::Value *&zeroValue, const Token *&nonZeroExpr, bool suppress)
{
    if (!tok->isComparisonOp() || !tok->astOperand1() || !tok->astOperand2())
        return false;

    const ValueFlow::Value *v1 = tok->astOperand1()->getValue(0);
    const ValueFlow::Value *v2 = tok->astOperand2()->getValue(0);

    if (Token::Match(tok, "<|<=") && v2 && v2->isKnown()) {
        zeroValue = v2;
        nonZeroExpr = tok->astOperand1();
    } else if (Token::Match(tok, ">|>=") && v1 && v1->isKnown()) {
        zeroValue = v1;
        nonZeroExpr = tok->astOperand2();
    } else {
        return false;
    }

    if (const Variable* var = nonZeroExpr->variable())
        if (var->typeStartToken()->isTemplateArg())
            return suppress;

    const ValueType* vt = nonZeroExpr->valueType();
    return vt && (vt->pointer || vt->sign == ValueType::UNSIGNED);
}

bool CheckOther::testIfNonZeroExpressionIsPositive(const Token *tok, const ValueFlow::Value *&zeroValue, const Token *&nonZeroExpr)
{
    if (!tok->isComparisonOp() || !tok->astOperand1() || !tok->astOperand2())
        return false;

    const ValueFlow::Value *v1 = tok->astOperand1()->getValue(0);
    const ValueFlow::Value *v2 = tok->astOperand2()->getValue(0);

    if (Token::simpleMatch(tok, ">=") && v2 && v2->isKnown()) {
        zeroValue = v2;
        nonZeroExpr = tok->astOperand1();
    } else if (Token::simpleMatch(tok, "<=") && v1 && v1->isKnown()) {
        zeroValue = v1;
        nonZeroExpr = tok->astOperand2();
    } else {
        return false;
    }

    const ValueType* vt = nonZeroExpr->valueType();
    return vt && (vt->pointer || vt->sign == ValueType::UNSIGNED);
}

void CheckOther::unsignedLessThanZeroError(const Token *tok, const ValueFlow::Value * v, const std::string &varname)
{
    reportError(getErrorPath(tok, v, "Unsigned less than zero"), Severity::style, "unsignedLessThanZero",
                "$symbol:" + varname + "\n"
                "Checking if unsigned expression '$symbol' is less than zero.\n"
                "The unsigned expression '$symbol' will never be negative so it "
                "is either pointless or an error to check if it is.", CWE570, Certainty::normal);
}

void CheckOther::pointerLessThanZeroError(const Token *tok, const ValueFlow::Value *v)
{
    reportError(getErrorPath(tok, v, "Pointer less than zero"), Severity::style, "pointerLessThanZero",
                "A pointer can not be negative so it is either pointless or an error to check if it is.", CWE570, Certainty::normal);
}

void CheckOther::unsignedPositiveError(const Token *tok, const ValueFlow::Value * v, const std::string &varname)
{
    reportError(getErrorPath(tok, v, "Unsigned positive"), Severity::style, "unsignedPositive",
                "$symbol:" + varname + "\n"
                "Unsigned expression '$symbol' can't be negative so it is unnecessary to test it.", CWE570, Certainty::normal);
}

void CheckOther::pointerPositiveError(const Token *tok, const ValueFlow::Value * v)
{
    reportError(getErrorPath(tok, v, "Pointer positive"), Severity::style, "pointerPositive",
                "A pointer can not be negative so it is either pointless or an error to check if it is not.", CWE570, Certainty::normal);
}

/* check if a constructor in given class scope takes a reference */
static bool constructorTakesReference(const Scope * const classScope)
{
    return std::any_of(classScope->functionList.begin(), classScope->functionList.end(), [&](const Function& constructor) {
        if (constructor.isConstructor()) {
            for (int argnr = 0U; argnr < constructor.argCount(); argnr++) {
                const Variable * const argVar = constructor.getArgumentVar(argnr);
                if (argVar && argVar->isReference()) {
                    return true;
                }
            }
        }
        return false;
    });
}

static bool isLargeObject(const Variable* var, const Settings& settings)
{
    if (!var || var->isGlobal() || !var->valueType())
        return false;
    ValueType vt = *var->valueType();
    vt.reference = Reference::None;
    return vt.getSizeOf(settings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer) > 2 * settings.platform.sizeof_pointer;
}

//---------------------------------------------------------------------------
// This check rule works for checking the "const A a = getA()" usage when getA() returns "const A &" or "A &".
// In most scenarios, "const A & a = getA()" will be more efficient.
//---------------------------------------------------------------------------
static bool checkFunctionReturnsRef(const Token* tok, const Settings& settings)
{
    if (!Token::Match(tok->previous(), "%name% ("))
        return false;
    if (!Token::Match(tok->link(), ") )|}| ;")) // bailout for usage like "const A a = getA()+3"
        return false;
    const Token* dot = tok->astOperand1();
    if (Token::simpleMatch(dot, ".")) {
        const Token* varTok = dot->astOperand1();
        const int indirect = varTok->valueType() ? varTok->valueType()->pointer : 0;
        if (isVariableChanged(tok, tok->scope()->bodyEnd, indirect, varTok->varId(), /*globalvar*/ true, settings))
            return false;
        if (isTemporary(dot, &settings.library, /*unknown*/ true))
            return false;
    }
    if (exprDependsOnThis(tok->previous()))
        return false;
    const Function* func = tok->previous()->function();
    if (func && func->tokenDef->strAt(-1) == "&") {
        const Scope* fScope = func->functionScope;
        if (fScope && fScope->bodyEnd && Token::Match(fScope->bodyEnd->tokAt(-3), "return %var% ;")) {
            const Token* varTok = fScope->bodyEnd->tokAt(-2);
            if (isLargeObject(varTok->variable(), settings))
                return true;
        }
    }
    return false;
}

static bool checkVariableAssignment(const Token* tok, const ValueType* vtLhs, const Settings& settings)
{
    if (!Token::Match(tok, "%var% ;"))
        return false;
    const Variable* var = tok->variable();
    if (!var || !isLargeObject(var, settings))
        return false;
    if (!vtLhs || !vtLhs->isTypeEqual(var->valueType()))
        return false;
    if (findVariableChanged(tok->tokAt(2), tok->scope()->bodyEnd, /*indirect*/ 0, var->declarationId(), /*globalvar*/ false, settings))
        return false;
    if (var->isLocal() || (var->isArgument() && !var->isReference()))
        return true;
    const Scope* scope = tok->scope();
    while (scope && scope->type != ScopeType::eFunction)
        scope = scope->nestedIn;
    return scope && scope->function && (!scope->functionOf || scope->function->isConst() || scope->function->isStatic());
}

void CheckOther::checkRedundantCopy()
{
    if (!mSettings->severity.isEnabled(Severity::performance) || mTokenizer->isC() || !mSettings->certainty.isEnabled(Certainty::inconclusive))
        return;

    logChecker("CheckOther::checkRedundantCopy"); // c++,performance,inconclusive

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || var->isReference() || var->isPointer() ||
            (!var->type() && !var->isStlType() && !(var->valueType() && var->valueType()->container)) || // bailout if var is of standard type, if it is a pointer or non-const
            (!var->isConst() && isVariableChanged(var, *mSettings)))
            continue;

        const Token* startTok = var->nameToken();
        if (startTok->strAt(1) == "=") // %type% %name% = ... ;
            ;
        else if (Token::Match(startTok->next(), "(|{") && var->isClass()) {
            if (!var->typeScope() && !(var->valueType() && var->valueType()->container))
                continue;
            // Object is instantiated. Warn if constructor takes arguments by value.
            if (var->typeScope() && constructorTakesReference(var->typeScope()))
                continue;
        } else if (Token::simpleMatch(startTok->next(), ";") && startTok->next()->isSplittedVarDeclEq()) {
            startTok = startTok->tokAt(2);
        } else
            continue;

        const Token* tok = startTok->next()->astOperand2();
        if (!tok)
            continue;
        if (!checkFunctionReturnsRef(tok, *mSettings) && !checkVariableAssignment(tok, var->valueType(), *mSettings))
            continue;
        redundantCopyError(startTok, startTok->str());
    }
}

void CheckOther::redundantCopyError(const Token *tok,const std::string& varname)
{
    reportError(tok, Severity::performance, "redundantCopyLocalConst",
                "$symbol:" + varname + "\n"
                "Use const reference for '$symbol' to avoid unnecessary data copying.\n"
                "The const variable '$symbol' is assigned a copy of the data. You can avoid "
                "the unnecessary data copying by converting '$symbol' to const reference.",
                CWE398,
                Certainty::inconclusive); // since #5618 that check became inconclusive
}

//---------------------------------------------------------------------------
// Checking for shift by negative values
//---------------------------------------------------------------------------

static bool isNegative(const Token *tok, const Settings &settings)
{
    return tok->valueType() && tok->valueType()->sign == ValueType::SIGNED && tok->getValueLE(-1LL, settings);
}

void CheckOther::checkNegativeBitwiseShift()
{
    const bool portability = mSettings->severity.isEnabled(Severity::portability);

    logChecker("CheckOther::checkNegativeBitwiseShift");

    for (const Token* tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        tok = skipUnreachableBranch(tok);

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        if (!Token::Match(tok, "<<|>>|<<=|>>="))
            continue;

        // don't warn if lhs is a class. this is an overloaded operator then
        if (tok->isCpp()) {
            const ValueType * lhsType = tok->astOperand1()->valueType();
            if (!lhsType || !lhsType->isIntegral() || lhsType->pointer)
                continue;
        }

        // bailout if operation is protected by ?:
        bool ternary = false;
        for (const Token *parent = tok; parent; parent = parent->astParent()) {
            if (Token::Match(parent, "?|:")) {
                ternary = true;
                break;
            }
        }
        if (ternary)
            continue;

        // Get negative rhs value. preferably a value which doesn't have 'condition'.
        if (portability && isNegative(tok->astOperand1(), *mSettings))
            negativeBitwiseShiftError(tok, 1);
        else if (isNegative(tok->astOperand2(), *mSettings))
            negativeBitwiseShiftError(tok, 2);
    }
}


void CheckOther::negativeBitwiseShiftError(const Token *tok, int op)
{
    if (op == 1)
        // LHS - this is used by intention in various software, if it
        // is used often in a project and works as expected then this is
        // a portability issue
        reportError(tok, Severity::portability, "shiftNegativeLHS", "Shifting a negative value is technically undefined behaviour", CWE758, Certainty::normal);
    else // RHS
        reportError(tok, Severity::error, "shiftNegative", "Shifting by a negative value is undefined behaviour", CWE758, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check for incompletely filled buffers.
//---------------------------------------------------------------------------
void CheckOther::checkIncompleteArrayFill()
{
    if (!mSettings->certainty.isEnabled(Certainty::inconclusive))
        return;
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    const bool printPortability = mSettings->severity.isEnabled(Severity::portability);
    if (!printPortability && !printWarning)
        return;

    logChecker("CheckOther::checkIncompleteArrayFill"); // warning,portability,inconclusive

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|memcpy|memmove (")) {
                std::vector<const Token*> args = getArguments(tok);
                if (args.size() != 3)
                    continue;
                const Token* tok2 = args[0];
                if (tok2->str() == "::")
                    tok2 = tok2->next();
                while (Token::Match(tok2, "%name% ::|."))
                    tok2 = tok2->tokAt(2);
                if (!Token::Match(tok2, "%var% ,"))
                    continue;

                const Variable *var = tok2->variable();
                if (!var || !var->isArray() || var->dimensions().empty() || !var->dimension(0))
                    continue;

                if (!args[2]->hasKnownIntValue() || args[2]->getKnownIntValue() != var->dimension(0))
                    continue;
                int size = mTokenizer->sizeOfType(var->typeStartToken());
                if (size == 0 && var->valueType()->pointer)
                    size = mSettings->platform.sizeof_pointer;
                else if (size == 0 && var->valueType())
                    size = var->valueType()->getSizeOf(*mSettings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer);
                const Token* tok3 = tok->next()->astOperand2()->astOperand1()->astOperand1();
                if ((size != 1 && size != 100 && size != 0) || var->isPointer()) {
                    if (printWarning)
                        incompleteArrayFillError(tok, tok3->expressionString(), tok->str(), false);
                } else if (var->valueType()->type == ValueType::Type::BOOL && printPortability) // sizeof(bool) is not 1 on all platforms
                    incompleteArrayFillError(tok, tok3->expressionString(), tok->str(), true);
            }
        }
    }
}

void CheckOther::incompleteArrayFillError(const Token* tok, const std::string& buffer, const std::string& function, bool boolean)
{
    if (boolean)
        reportError(tok, Severity::portability, "incompleteArrayFill",
                    "$symbol:" + buffer + "\n"
                    "$symbol:" + function + "\n"
                    "Array '" + buffer + "' might be filled incompletely. Did you forget to multiply the size given to '" + function + "()' with 'sizeof(*" + buffer + ")'?\n"
                    "The array '" + buffer + "' is filled incompletely. The function '" + function + "()' needs the size given in bytes, but the type 'bool' is larger than 1 on some platforms. Did you forget to multiply the size with 'sizeof(*" + buffer + ")'?", CWE131, Certainty::inconclusive);
    else
        reportError(tok, Severity::warning, "incompleteArrayFill",
                    "$symbol:" + buffer + "\n"
                    "$symbol:" + function + "\n"
                    "Array '" + buffer + "' is filled incompletely. Did you forget to multiply the size given to '" + function + "()' with 'sizeof(*" + buffer + ")'?\n"
                    "The array '" + buffer + "' is filled incompletely. The function '" + function + "()' needs the size given in bytes, but an element of the given array is larger than one byte. Did you forget to multiply the size with 'sizeof(*" + buffer + ")'?", CWE131, Certainty::inconclusive);
}

//---------------------------------------------------------------------------
// Detect NULL being passed to variadic function.
//---------------------------------------------------------------------------

void CheckOther::checkVarFuncNullUB()
{
    if (!mSettings->severity.isEnabled(Severity::portability))
        return;

    logChecker("CheckOther::checkVarFuncNullUB"); // portability

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            // Is NULL passed to a function?
            if (Token::Match(tok,"[(,] NULL )")) {
                // Locate function name in this function call.
                const Token *ftok = tok;
                int argnr = 1;
                while (ftok && ftok->str() != "(") {
                    if (ftok->str() == ")")
                        ftok = ftok->link();
                    else if (ftok->str() == ",")
                        ++argnr;
                    ftok = ftok->previous();
                }
                ftok = ftok ? ftok->previous() : nullptr;
                if (ftok && ftok->isName()) {
                    // If this is a variadic function then report error
                    const Function *f = ftok->function();
                    if (f && f->argCount() <= argnr) {
                        const Token *tok2 = f->argDef;
                        tok2 = tok2 ? tok2->link() : nullptr; // goto ')'
                        if (tok2 && Token::simpleMatch(tok2->tokAt(-1), "..."))
                            varFuncNullUBError(tok);
                    }
                }
            }
        }
    }
}

void CheckOther::varFuncNullUBError(const Token *tok)
{
    reportError(tok,
                Severity::portability,
                "varFuncNullUB",
                "Passing NULL after the last typed argument to a variadic function leads to undefined behaviour.\n"
                "Passing NULL after the last typed argument to a variadic function leads to undefined behaviour.\n"
                "The C99 standard, in section 7.15.1.1, states that if the type used by va_arg() is not compatible with the type of the actual next argument (as promoted according to the default argument promotions), the behavior is undefined.\n"
                "The value of the NULL macro is an implementation-defined null pointer constant (7.17), which can be any integer constant expression with the value 0, or such an expression casted to (void*) (6.3.2.3). This includes values like 0, 0L, or even 0LL.\n"
                "In practice on common architectures, this will cause real crashes if sizeof(int) != sizeof(void*), and NULL is defined to 0 or any other null pointer constant that promotes to int.\n"
                "To reproduce you might be able to use this little code example on 64bit platforms. If the output includes \"ERROR\", the sentinel had only 4 out of 8 bytes initialized to zero and was not detected as the final argument to stop argument processing via va_arg(). Changing the 0 to (void*)0 or 0L will make the \"ERROR\" output go away.\n"
                "#include <stdarg.h>\n"
                "#include <stdio.h>\n"
                "\n"
                "void f(char *s, ...) {\n"
                "    va_list ap;\n"
                "    va_start(ap,s);\n"
                "    for (;;) {\n"
                "        char *p = va_arg(ap,char*);\n"
                "        printf(\"%018p, %s\\n\", p, (long)p & 255 ? p : \"\");\n"
                "        if(!p) break;\n"
                "    }\n"
                "    va_end(ap);\n"
                "}\n"
                "\n"
                "void g() {\n"
                "    char *s2 = \"x\";\n"
                "    char *s3 = \"ERROR\";\n"
                "\n"
                "    // changing 0 to 0L for the 7th argument (which is intended to act as sentinel) makes the error go away on x86_64\n"
                "    f(\"first\", s2, s2, s2, s2, s2, 0, s3, (char*)0);\n"
                "}\n"
                "\n"
                "void h() {\n"
                "    int i;\n"
                "    volatile unsigned char a[1000];\n"
                "    for (i = 0; i<sizeof(a); i++)\n"
                "        a[i] = -1;\n"
                "}\n"
                "\n"
                "int main() {\n"
                "    h();\n"
                "    g();\n"
                "    return 0;\n"
                "}", CWE475, Certainty::normal);
}

void CheckOther::checkRedundantPointerOp()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("redundantPointerOp"))
        return;

    logChecker("CheckOther::checkRedundantPointerOp"); // style

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->isExpandedMacro() && tok->str() == "(")
            tok = tok->link();

        bool addressOfDeref{};
        if (tok->isUnaryOp("&") && tok->astOperand1()->isUnaryOp("*"))
            addressOfDeref = true;
        else if (tok->isUnaryOp("*") && tok->astOperand1()->isUnaryOp("&"))
            addressOfDeref = false;
        else
            continue;

        // variable
        const Token *varTok = tok->astOperand1()->astOperand1();
        if (!varTok || varTok->isExpandedMacro())
            continue;

        if (!addressOfDeref) { // dereference of address
            if (tok->isExpandedMacro())
                continue;
            if (varTok->valueType() && varTok->valueType()->pointer && varTok->valueType()->reference == Reference::LValue)
                continue;
        }

        const Variable *var = varTok->variable();
        if (!var || (addressOfDeref && !var->isPointer()))
            continue;

        redundantPointerOpError(tok, var->name(), false, addressOfDeref);
    }
}

void CheckOther::redundantPointerOpError(const Token* tok, const std::string &varname, bool inconclusive, bool addressOfDeref)
{
    std::string msg = "$symbol:" + varname + "\nRedundant pointer operation on '$symbol' - it's already a ";
    msg += addressOfDeref ? "pointer." : "variable.";
    reportError(tok, Severity::style, "redundantPointerOp", msg, CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckOther::checkInterlockedDecrement()
{
    if (!mSettings->platform.isWindows()) {
        return;
    }

    logChecker("CheckOther::checkInterlockedDecrement"); // windows-platform

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->isName() && Token::Match(tok, "InterlockedDecrement ( & %name% ) ; if ( %name%|!|0")) {
            const Token* interlockedVarTok = tok->tokAt(3);
            const Token* checkStartTok =  interlockedVarTok->tokAt(5);
            if ((Token::Match(checkStartTok, "0 %comp% %name% )") && checkStartTok->strAt(2) == interlockedVarTok->str()) ||
                (Token::Match(checkStartTok, "! %name% )") && checkStartTok->strAt(1) == interlockedVarTok->str()) ||
                (Token::Match(checkStartTok, "%name% )") && checkStartTok->str() == interlockedVarTok->str()) ||
                (Token::Match(checkStartTok, "%name% %comp% 0 )") && checkStartTok->str() == interlockedVarTok->str())) {
                raceAfterInterlockedDecrementError(checkStartTok);
            }
        } else if (Token::Match(tok, "if ( ::| InterlockedDecrement ( & %name%")) {
            const Token* condEnd = tok->linkAt(1);
            const Token* funcTok = tok->tokAt(2);
            const Token* firstAccessTok = funcTok->str() == "::" ? funcTok->tokAt(4) : funcTok->tokAt(3);
            if (condEnd && condEnd->next() && condEnd->linkAt(1)) {
                const Token* ifEndTok = condEnd->linkAt(1);
                if (Token::Match(ifEndTok, "} return %name%")) {
                    const Token* secondAccessTok = ifEndTok->tokAt(2);
                    if (secondAccessTok->str() == firstAccessTok->str()) {
                        raceAfterInterlockedDecrementError(secondAccessTok);
                    }
                } else if (Token::Match(ifEndTok, "} else { return %name%")) {
                    const Token* secondAccessTok = ifEndTok->tokAt(4);
                    if (secondAccessTok->str() == firstAccessTok->str()) {
                        raceAfterInterlockedDecrementError(secondAccessTok);
                    }
                }
            }
        }
    }
}

void CheckOther::raceAfterInterlockedDecrementError(const Token* tok)
{
    reportError(tok, Severity::error, "raceAfterInterlockedDecrement",
                "Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.", CWE362, Certainty::normal);
}

void CheckOther::checkUnusedLabel()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("unusedLabel"))
        return;

    logChecker("CheckOther::checkUnusedLabel"); // style,warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const bool hasIfdef = mTokenizer->hasIfdef(scope->bodyStart, scope->bodyEnd);
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->scope()->isExecutable())
                tok = tok->scope()->bodyEnd;

            if (Token::Match(tok, "{|}|; %name% :") && !tok->tokAt(1)->isKeyword()) {
                const std::string tmp("goto " + tok->strAt(1));
                if (!Token::findsimplematch(scope->bodyStart->next(), tmp.c_str(), tmp.size(), scope->bodyEnd->previous()) && !tok->next()->isExpandedMacro())
                    unusedLabelError(tok->next(), tok->next()->scope()->type == ScopeType::eSwitch, hasIfdef);
            }
        }
    }
}

void CheckOther::unusedLabelError(const Token* tok, bool inSwitch, bool hasIfdef)
{
    if (tok && !mSettings->severity.isEnabled(inSwitch ? Severity::warning : Severity::style) && !mSettings->isPremiumEnabled("unusedLabel"))
        return;

    std::string id = "unusedLabel";
    if (inSwitch)
        id += "Switch";
    if (hasIfdef)
        id += "Configuration";

    std::string msg = "$symbol:" + (tok ? tok->str() : "") + "\nLabel '$symbol' is not used.";
    if (hasIfdef)
        msg += " There is #if in function body so the label might be used in code that is removed by the preprocessor.";
    if (inSwitch)
        msg += " Should this be a 'case' of the enclosing switch()?";

    reportError(tok,
                inSwitch ? Severity::warning : Severity::style,
                id,
                msg,
                CWE398,
                Certainty::normal);
}

static bool checkEvaluationOrderC(const Token * tok, const Token * tok2, const Token * parent, const Settings & settings, bool & selfAssignmentError)
{
    // self assignment..
    if (tok2 == tok && tok->str() == "=" && parent->str() == "=" && isSameExpression(false, tok->astOperand1(), parent->astOperand1(), settings, true, false)) {
        if (settings.severity.isEnabled(Severity::warning) && isSameExpression(true, tok->astOperand1(), parent->astOperand1(), settings, true, false))
            selfAssignmentError = true;
        return false;
    }
    // Is expression used?
    bool foundError = false;
    visitAstNodes((parent->astOperand1() != tok2) ? parent->astOperand1() : parent->astOperand2(), [&](const Token *tok3) {
        if (tok3->str() == "&" && !tok3->astOperand2())
            return ChildrenToVisit::none; // don't handle address-of for now
        if (tok3->str() == "(" && Token::simpleMatch(tok3->previous(), "sizeof"))
            return ChildrenToVisit::none; // don't care about sizeof usage
        if (isSameExpression(false, tok->astOperand1(), tok3, settings, true, false))
            foundError = true;
        return foundError ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
    });

    return foundError;
}

static bool checkEvaluationOrderCpp11(const Token * tok, const Token * tok2, const Token * parent, const Settings & settings)
{
    if (tok->isAssignmentOp()) // TODO check assignment
        return false;
    if (tok->previous() == tok->astOperand1() && parent->isArithmeticalOp() && parent->isBinaryOp()) {
        if (parent->astParent() && parent->astParent()->isAssignmentOp() && isSameExpression(false, tok->astOperand1(), parent->astParent()->astOperand1(), settings, true, false))
            return true;
    }
    bool foundUndefined{false};
    visitAstNodes((parent->astOperand1() != tok2) ? parent->astOperand1() : parent->astOperand2(), [&](const Token *tok3) {
        if (tok3->str() == "&" && !tok3->astOperand2())
            return ChildrenToVisit::none; // don't handle address-of for now
        if (tok3->str() == "(" && Token::simpleMatch(tok3->previous(), "sizeof"))
            return ChildrenToVisit::none; // don't care about sizeof usage
        if (isSameExpression(false, tok->astOperand1(), tok3, settings, true, false))
            foundUndefined = true;
        return foundUndefined ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
    });

    return foundUndefined;
}

static bool checkEvaluationOrderCpp17(const Token * tok, const Token * tok2, const Token * parent, const Settings & settings, bool & foundUnspecified)
{
    if (tok->isAssignmentOp())
        return false;
    bool foundUndefined{false};
    visitAstNodes((parent->astOperand1() != tok2) ? parent->astOperand1() : parent->astOperand2(), [&](const Token *tok3) {
        if (tok3->str() == "&" && !tok3->astOperand2())
            return ChildrenToVisit::none; // don't handle address-of for now
        if (tok3->str() == "(" && Token::simpleMatch(tok3->previous(), "sizeof"))
            return ChildrenToVisit::none; // don't care about sizeof usage
        if (isSameExpression(false, tok->astOperand1(), tok3, settings, true, false) && parent->isArithmeticalOp() && parent->isBinaryOp())
            foundUndefined = true;
        if (tok3->tokType() == Token::eIncDecOp && isSameExpression(false, tok->astOperand1(), tok3->astOperand1(), settings, true, false)) {
            if (parent->isArithmeticalOp() && parent->isBinaryOp())
                foundUndefined = true;
            else
                foundUnspecified = true;
        }
        return (foundUndefined || foundUnspecified) ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
    });

    return foundUndefined || foundUnspecified;
}

void CheckOther::checkEvaluationOrder()
{
    logChecker("CheckOther::checkEvaluationOrder");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * functionScope : symbolDatabase->functionScopes) {
        for (const Token* tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!tok->isIncDecOp() && !tok->isAssignmentOp())
                continue;
            if (!tok->astOperand1())
                continue;
            if (isDesignatedInitializer(tok->astOperand1()))
                continue;
            for (const Token *tok2 = tok;; tok2 = tok2->astParent()) {
                // If ast parent is a sequence point then break
                const Token * const parent = tok2->astParent();
                if (!parent)
                    break;
                if (Token::Match(parent, "%oror%|&&|?|:|;"))
                    break;
                if (parent->str() == ",") {
                    const Token *par = parent;
                    while (Token::simpleMatch(par,","))
                        par = par->astParent();
                    // not function or in a while clause => break
                    if (!(par && par->str() == "(" && par->astOperand2() && par->strAt(-1) != "while"))
                        break;
                    // control flow (if|while|etc) => break
                    if (Token::simpleMatch(par->link(),") {"))
                        break;
                    // sequence point in function argument: dostuff((1,2),3) => break
                    par = par->next();
                    while (par && (par->previous() != parent))
                        par = par->nextArgument();
                    if (!par)
                        break;
                }
                if (parent->str() == "(" && parent->astOperand2())
                    break;

                bool foundError{false}, foundUnspecified{false}, bSelfAssignmentError{false};
                if (mTokenizer->isCPP() && mSettings->standards.cpp >= Standards::CPP11) {
                    if (mSettings->standards.cpp >= Standards::CPP17)
                        foundError = checkEvaluationOrderCpp17(tok, tok2, parent, *mSettings, foundUnspecified);
                    else
                        foundError = checkEvaluationOrderCpp11(tok, tok2, parent, *mSettings);
                }
                else
                    foundError = checkEvaluationOrderC(tok, tok2, parent, *mSettings, bSelfAssignmentError);

                if (foundError) {
                    unknownEvaluationOrder(parent, foundUnspecified);
                    break;
                }
                if (bSelfAssignmentError) {
                    selfAssignmentError(parent, tok->astOperand1()->expressionString());
                    break;
                }
            }
        }
    }
}

void CheckOther::unknownEvaluationOrder(const Token* tok, bool isUnspecifiedBehavior)
{
    isUnspecifiedBehavior ?
    reportError(tok, Severity::portability, "unknownEvaluationOrder",
                "Expression '" + (tok ? tok->expressionString() : std::string("x++, x++")) + "' depends on order of evaluation of side effects. Behavior is Unspecified according to c++17", CWE768, Certainty::normal)
    :   reportError(tok, Severity::error, "unknownEvaluationOrder",
                    "Expression '" + (tok ? tok->expressionString() : std::string("x = x++;")) + "' depends on order of evaluation of side effects", CWE768, Certainty::normal);
}

void CheckOther::checkAccessOfMovedVariable()
{
    if (!mTokenizer->isCPP() || mSettings->standards.cpp < Standards::CPP11)
        return;
    if (!mSettings->isPremiumEnabled("accessMoved") && !mSettings->severity.isEnabled(Severity::warning))
        return;
    logChecker("CheckOther::checkAccessOfMovedVariable"); // c++11,warning
    const bool reportInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const Token * scopeStart = scope->bodyStart;
        if (scope->function) {
            const Token * memberInitializationStart = scope->function->constructorMemberInitialization();
            if (memberInitializationStart)
                scopeStart = memberInitializationStart;
        }
        for (const Token* tok = scopeStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->astParent())
                continue;
            const ValueFlow::Value * movedValue = tok->getMovedValue();
            if (!movedValue || movedValue->moveKind == ValueFlow::Value::MoveKind::NonMovedVariable)
                continue;
            if (movedValue->isInconclusive() && !reportInconclusive)
                continue;

            bool inconclusive = false;
            bool accessOfMoved = false;
            if (tok->strAt(1) == ".") {
                if (tok->next()->originalName() == "->")
                    accessOfMoved = true;
                else
                    inconclusive = true;
            } else {
                const ExprUsage usage = getExprUsage(tok, 0, *mSettings);
                if (usage == ExprUsage::Used)
                    accessOfMoved = true;
                if (usage == ExprUsage::PassedByReference)
                    accessOfMoved = !isVariableChangedByFunctionCall(tok, 0, *mSettings, &inconclusive);
                else if (usage == ExprUsage::Inconclusive)
                    inconclusive = true;
            }
            if (accessOfMoved || (inconclusive && reportInconclusive))
                accessMovedError(tok, tok->str(), movedValue, inconclusive || movedValue->isInconclusive());
        }
    }
}

void CheckOther::accessMovedError(const Token *tok, const std::string &varname, const ValueFlow::Value *value, bool inconclusive)
{
    if (!tok) {
        reportError(tok, Severity::warning, "accessMoved", "Access of moved variable 'v'.", CWE672, Certainty::normal);
        reportError(tok, Severity::warning, "accessForwarded", "Access of forwarded variable 'v'.", CWE672, Certainty::normal);
        return;
    }

    const char * errorId = nullptr;
    std::string kindString;
    switch (value->moveKind) {
    case ValueFlow::Value::MoveKind::MovedVariable:
        errorId = "accessMoved";
        kindString = "moved";
        break;
    case ValueFlow::Value::MoveKind::ForwardedVariable:
        errorId = "accessForwarded";
        kindString = "forwarded";
        break;
    default:
        return;
    }
    const std::string errmsg("$symbol:" + varname + "\nAccess of " + kindString + " variable '$symbol'.");
    ErrorPath errorPath = getErrorPath(tok, value, errmsg);
    reportError(std::move(errorPath), Severity::warning, errorId, errmsg, CWE672, inconclusive ? Certainty::inconclusive : Certainty::normal);
}



void CheckOther::checkFuncArgNamesDifferent()
{
    const bool style = mSettings->severity.isEnabled(Severity::style);
    const bool inconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const bool warning = mSettings->severity.isEnabled(Severity::warning);

    if (!(warning || (style && inconclusive)) && !mSettings->isPremiumEnabled("funcArgNamesDifferent"))
        return;

    logChecker("CheckOther::checkFuncArgNamesDifferent"); // style,warning,inconclusive

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    // check every function
    for (const Scope *scope : symbolDatabase->functionScopes) {
        const Function * function = scope->function;
        // only check functions with arguments
        if (!function || function->argCount() == 0)
            continue;

        // only check functions with separate declarations and definitions
        if (function->argDef == function->arg)
            continue;

        // get the function argument name tokens
        std::vector<const Token *>  declarations(function->argCount());
        std::vector<const Token *>  definitions(function->argCount());
        const Token * decl = function->argDef->next();
        for (int j = 0; j < function->argCount(); ++j) {
            // get the definition
            const Variable * variable = function->getArgumentVar(j);
            if (variable) {
                definitions[j] = variable->nameToken();
            }
            // get the declaration (search for first token with varId)
            while (decl && !Token::Match(decl, ",|)|;")) {
                // skip everything after the assignment because
                // it could also have a varId or be the first
                // token with a varId if there is no name token
                if (decl->str() == "=") {
                    decl = decl->nextArgument();
                    break;
                }
                // skip over template
                if (decl->link() && decl->str() == "<")
                    decl = decl->link();
                else if (decl->varId())
                    declarations[j] = decl;
                decl = decl->next();
            }
            if (Token::simpleMatch(decl, ","))
                decl = decl->next();
        }
        // check for different argument order
        if (warning) {
            bool order_different = false;
            for (int j = 0; j < function->argCount(); ++j) {
                if (!declarations[j] || !definitions[j] || declarations[j]->str() == definitions[j]->str())
                    continue;

                for (int k = 0; k < function->argCount(); ++k) {
                    if (j != k && definitions[k] && declarations[j]->str() == definitions[k]->str()) {
                        order_different = true;
                        break;
                    }
                }
            }
            if (order_different) {
                funcArgOrderDifferent(function->name(), function->argDef->next(), function->arg->next(), declarations, definitions);
                continue;
            }
        }
        // check for different argument names
        if (style && inconclusive) {
            for (int j = 0; j < function->argCount(); ++j) {
                if (declarations[j] && definitions[j] && declarations[j]->str() != definitions[j]->str())
                    funcArgNamesDifferent(function->name(), j, declarations[j], definitions[j]);
            }
        }
    }
}

void CheckOther::funcArgNamesDifferent(const std::string & functionName, nonneg int index,
                                       const Token* declaration, const Token* definition)
{
    std::list<const Token *> tokens = { declaration,definition };
    reportError(tokens, Severity::style, "funcArgNamesDifferent",
                "$symbol:" + functionName + "\n"
                "Function '$symbol' argument " + std::to_string(index + 1) + " names different: declaration '" +
                (declaration ? declaration->str() : std::string("A")) + "' definition '" +
                (definition ? definition->str() : std::string("B")) + "'.", CWE628, Certainty::inconclusive);
}

void CheckOther::funcArgOrderDifferent(const std::string & functionName,
                                       const Token* declaration, const Token* definition,
                                       const std::vector<const Token *> & declarations,
                                       const std::vector<const Token *> & definitions)
{
    std::list<const Token *> tokens = {
        !declarations.empty() ? declarations[0] ? declarations[0] : declaration : nullptr,
        !definitions.empty() ? definitions[0] ? definitions[0] : definition : nullptr
    };
    std::string msg = "$symbol:" + functionName + "\nFunction '$symbol' argument order different: declaration '";
    for (std::size_t i = 0; i < declarations.size(); ++i) {
        if (i != 0)
            msg += ", ";
        if (declarations[i])
            msg += declarations[i]->str();
    }
    msg += "' definition '";
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        if (i != 0)
            msg += ", ";
        if (definitions[i])
            msg += definitions[i]->str();
    }
    msg += "'";
    reportError(tokens, Severity::warning, "funcArgOrderDifferent", msg, CWE683, Certainty::normal);
}

static const Token *findShadowed(const Scope *scope, const Variable& var, int linenr)
{
    if (!scope)
        return nullptr;
    for (const Variable &v : scope->varlist) {
        if (scope->isExecutable() && v.nameToken()->linenr() > linenr)
            continue;
        if (v.name() == var.name())
            return v.nameToken();
    }
    auto it = std::find_if(scope->functionList.cbegin(), scope->functionList.cend(), [&](const Function& f) {
        return f.type == FunctionType::eFunction && f.name() == var.name() && precedes(f.tokenDef, var.nameToken());
    });
    if (it != scope->functionList.end())
        return it->tokenDef;

    if (scope->type == ScopeType::eLambda)
        return nullptr;
    const Token* shadowed = findShadowed(scope->nestedIn, var, linenr);
    if (!shadowed)
        shadowed = findShadowed(scope->functionOf, var, linenr);
    return shadowed;
}

void CheckOther::checkShadowVariables()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("shadowVariable"))
        return;
    logChecker("CheckOther::checkShadowVariables"); // style
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope & scope : symbolDatabase->scopeList) {
        if (!scope.isExecutable() || scope.type == ScopeType::eLambda)
            continue;
        const Scope *functionScope = &scope;
        while (functionScope && functionScope->type != ScopeType::eFunction && functionScope->type != ScopeType::eLambda)
            functionScope = functionScope->nestedIn;
        for (const Variable &var : scope.varlist) {
            if (var.nameToken() && var.nameToken()->isExpandedMacro()) // #8903
                continue;

            if (functionScope && functionScope->type == ScopeType::eFunction && functionScope->function) {
                const auto & argList = functionScope->function->argumentList;
                auto it = std::find_if(argList.cbegin(), argList.cend(), [&](const Variable& arg) {
                    return arg.nameToken() && var.name() == arg.name();
                });
                if (it != argList.end()) {
                    shadowError(var.nameToken(), it->nameToken(), "argument");
                    continue;
                }
            }

            const Token *shadowed = findShadowed(scope.nestedIn, var, var.nameToken()->linenr());
            if (!shadowed)
                shadowed = findShadowed(scope.functionOf, var, var.nameToken()->linenr());
            if (!shadowed)
                continue;
            if (scope.type == ScopeType::eFunction && scope.className == var.name())
                continue;
            if (functionScope->functionOf && functionScope->functionOf->isClassOrStructOrUnion() && functionScope->function &&
                (functionScope->function->isStatic() || functionScope->function->isFriend()) &&
                shadowed->variable() && !shadowed->variable()->isLocal())
                continue;
            shadowError(var.nameToken(), shadowed, (shadowed->varId() != 0) ? "variable" : "function");
        }
    }
}

void CheckOther::shadowError(const Token *var, const Token *shadowed, const std::string& type)
{
    ErrorPath errorPath;
    errorPath.emplace_back(shadowed, "Shadowed declaration");
    errorPath.emplace_back(var, "Shadow variable");
    const std::string &varname = var ? var->str() : type;
    const std::string Type = char(std::toupper(type[0])) + type.substr(1);
    const std::string id = "shadow" + Type;
    const std::string message = "$symbol:" + varname + "\nLocal variable \'$symbol\' shadows outer " + type;
    reportError(std::move(errorPath), Severity::style, id.c_str(), message, CWE398, Certainty::normal);
}

static bool isVariableExpression(const Token* tok)
{
    if (tok->varId() != 0)
        return true;
    if (Token::simpleMatch(tok, "."))
        return isVariableExpression(tok->astOperand1()) &&
               isVariableExpression(tok->astOperand2());
    if (Token::simpleMatch(tok, "["))
        return isVariableExpression(tok->astOperand1());
    return false;
}

static bool isVariableExprHidden(const Token* tok)
{
    if (!tok)
        return false;
    if (!tok->astParent())
        return false;
    if (Token::simpleMatch(tok->astParent(), "*") && Token::simpleMatch(tok->astSibling(), "0"))
        return true;
    if (Token::simpleMatch(tok->astParent(), "&&") && Token::simpleMatch(tok->astSibling(), "false"))
        return true;
    if (Token::simpleMatch(tok->astParent(), "||") && Token::simpleMatch(tok->astSibling(), "true"))
        return true;
    return false;
}

void CheckOther::checkKnownArgument()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("knownArgument"))
        return;
    logChecker("CheckOther::checkKnownArgument"); // style
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!tok->hasKnownIntValue() || tok->isExpandedMacro())
                continue;
            if (Token::Match(tok, "++|--|%assign%"))
                continue;
            if (!Token::Match(tok->astParent(), "(|{|,"))
                continue;
            if (tok->astParent()->isCast() || (tok->isCast() && Token::Match(tok->astOperand2(), "++|--|%assign%")))
                continue;
            int argn = -1;
            const Token* ftok = getTokenArgumentFunction(tok, argn);
            if (!ftok)
                continue;
            if (ftok->isCast())
                continue;
            if (Token::Match(ftok, "if|while|switch|sizeof"))
                continue;
            if (tok == tok->astParent()->previous())
                continue;
            if (isConstVarExpression(tok))
                continue;
            if (Token::Match(tok->astOperand1(), "%name% ("))
                continue;
            const Token * tok2 = tok;
            if (isCPPCast(tok2))
                tok2 = tok2->astOperand2();
            if (isVariableExpression(tok2))
                continue;
            if (tok->isComparisonOp() &&
                isSameExpression(
                    true, tok->astOperand1(), tok->astOperand2(), *mSettings, true, true))
                continue;
            // ensure that there is a integer variable in expression with unknown value
            const Token* vartok = nullptr;
            visitAstNodes(tok, [&](const Token* child) {
                if (Token::Match(child, "%var%|.|[")) {
                    if (child->hasKnownIntValue())
                        return ChildrenToVisit::none;
                    if (astIsIntegral(child, false) && !astIsPointer(child) && child->values().empty()) {
                        vartok = child;
                        return ChildrenToVisit::done;
                    }
                }
                return ChildrenToVisit::op1_and_op2;
            });
            if (!vartok)
                continue;
            if (vartok->astSibling() &&
                findAstNode(vartok->astSibling(), [](const Token* child) {
                return Token::simpleMatch(child, "sizeof");
            }))
                continue;
            // ensure that function name does not contain "assert"
            std::string funcname = ftok->str();
            strTolower(funcname);
            if (funcname.find("assert") != std::string::npos)
                continue;
            knownArgumentError(tok, ftok, &tok->values().front(), vartok->expressionString(), isVariableExprHidden(vartok));
        }
    }
}

void CheckOther::knownArgumentError(const Token *tok, const Token *ftok, const ValueFlow::Value *value, const std::string &varexpr, bool isVariableExpressionHidden)
{
    if (!tok) {
        reportError(tok, Severity::style, "knownArgument", "Argument 'x-x' to function 'func' is always 0. It does not matter what value 'x' has.");
        reportError(tok, Severity::style, "knownArgumentHiddenVariableExpression", "Argument 'x*0' to function 'func' is always 0. Constant literal calculation disable/hide variable expression 'x'.");
        return;
    }

    const MathLib::bigint intvalue = value->intvalue;
    const std::string &expr = tok->expressionString();
    const std::string &fun = ftok->str();

    std::string ftype = "function ";
    if (ftok->type())
        ftype = "constructor ";
    else if (fun == "{")
        ftype = "init list ";

    const char *id;
    std::string errmsg = "Argument '" + expr + "' to " + ftype + fun + " is always " + MathLib::toString(intvalue) + ". ";
    if (!isVariableExpressionHidden) {
        id = "knownArgument";
        errmsg += "It does not matter what value '" + varexpr + "' has.";
    } else {
        id = "knownArgumentHiddenVariableExpression";
        errmsg += "Constant literal calculation disable/hide variable expression '" + varexpr + "'.";
    }

    ErrorPath errorPath = getErrorPath(tok, value, errmsg);
    reportError(std::move(errorPath), Severity::style, id, errmsg, CWE570, Certainty::normal);
}

void CheckOther::checkKnownPointerToBool()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("knownPointerToBool"))
        return;
    logChecker("CheckOther::checkKnownPointerToBool"); // style
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope* functionScope : symbolDatabase->functionScopes) {
        for (const Token* tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!tok->hasKnownIntValue())
                continue;
            if (!astIsPointer(tok))
                continue;
            if (Token::Match(tok->astParent(), "?|!|&&|%oror%|%comp%"))
                continue;
            if (tok->astParent() && Token::Match(tok->astParent()->previous(), "if|while|switch|sizeof ("))
                continue;
            if (tok->isExpandedMacro())
                continue;
            if (findParent(tok, [](const Token* parent) {
                return parent->isExpandedMacro();
            }))
                continue;
            if (!isUsedAsBool(tok, *mSettings))
                continue;
            const ValueFlow::Value& value = tok->values().front();
            knownPointerToBoolError(tok, &value);
        }
    }
}

void CheckOther::knownPointerToBoolError(const Token* tok, const ValueFlow::Value* value)
{
    if (!tok) {
        reportError(tok, Severity::style, "knownPointerToBool", "Pointer expression 'p' converted to bool is always true.");
        return;
    }
    std::string cond = bool_to_string(!!value->intvalue);
    const std::string& expr = tok->expressionString();
    std::string errmsg = "Pointer expression '" + expr + "' converted to bool is always " + cond + ".";
    ErrorPath errorPath = getErrorPath(tok, value, errmsg);
    reportError(std::move(errorPath), Severity::style, "knownPointerToBool", errmsg, CWE570, Certainty::normal);
}

void CheckOther::checkComparePointers()
{
    logChecker("CheckOther::checkComparePointers");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "<|>|<=|>=|-"))
                continue;
            const Token *tok1 = tok->astOperand1();
            if (!astIsPointer(tok1))
                continue;
            const Token *tok2 = tok->astOperand2();
            if (!astIsPointer(tok2))
                continue;
            ValueFlow::Value v1 = ValueFlow::getLifetimeObjValue(tok1);
            if (!v1.isLocalLifetimeValue())
                continue;
            ValueFlow::Value v2 = ValueFlow::getLifetimeObjValue(tok2);
            if (!v2.isLocalLifetimeValue())
                continue;
            const Variable *var1 = v1.tokvalue->variable();
            const Variable *var2 = v2.tokvalue->variable();
            if (!var1 || !var2)
                continue;
            if (v1.tokvalue->varId() == v2.tokvalue->varId())
                continue;
            if (var1->isReference() || var2->isReference())
                continue;
            if (var1->isRValueReference() || var2->isRValueReference())
                continue;
            if (const Token* parent2 = getParentLifetime(v2.tokvalue, mSettings->library))
                if (var1 == parent2->variable())
                    continue;
            if (const Token* parent1 = getParentLifetime(v1.tokvalue, mSettings->library))
                if (var2 == parent1->variable())
                    continue;
            comparePointersError(tok, &v1, &v2);
        }
    }
}

void CheckOther::comparePointersError(const Token *tok, const ValueFlow::Value *v1, const ValueFlow::Value *v2)
{
    ErrorPath errorPath;
    std::string verb = "Comparing";
    if (Token::simpleMatch(tok, "-"))
        verb = "Subtracting";
    const char * const id = (verb[0] == 'C') ? "comparePointers" : "subtractPointers";
    if (v1) {
        errorPath.emplace_back(v1->tokvalue->variable()->nameToken(), "Variable declared here.");
        errorPath.insert(errorPath.end(), v1->errorPath.cbegin(), v1->errorPath.cend());
    }
    if (v2) {
        errorPath.emplace_back(v2->tokvalue->variable()->nameToken(), "Variable declared here.");
        errorPath.insert(errorPath.end(), v2->errorPath.cbegin(), v2->errorPath.cend());
    }
    errorPath.emplace_back(tok, "");
    reportError(
        std::move(errorPath), Severity::error, id, verb + " pointers that point to different objects", CWE758, Certainty::normal);
}

void CheckOther::checkModuloOfOne()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("moduloofone"))
        return;

    logChecker("CheckOther::checkModuloOfOne"); // style

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->astOperand2() || !tok->astOperand1())
            continue;
        if (tok->str() != "%" && tok->str() != "%=")
            continue;
        if (!tok->valueType() || !tok->valueType()->isIntegral())
            continue;

        // Value flow..
        const ValueFlow::Value *value = tok->astOperand2()->getValue(1LL);
        if (value && value->isKnown())
            checkModuloOfOneError(tok);
    }
}

void CheckOther::checkModuloOfOneError(const Token *tok)
{
    reportError(tok, Severity::style, "moduloofone", "Modulo of one is always equal to zero");
}

static const std::string noname;

struct UnionMember {
    UnionMember()
        : name(noname)
        , size(0) {}

    UnionMember(const std::string &name, size_t size)
        : name(name)
        , size(size) {}

    const std::string &name;
    size_t size;
};

struct Union {
    explicit Union(const Scope &scope)
        : scope(&scope)
        , name(scope.className) {}

    const Scope *scope;
    const std::string &name;
    std::vector<UnionMember> members;

    const UnionMember *largestMember() const {
        const UnionMember *largest = nullptr;
        for (const UnionMember &m : members) {
            if (m.size == 0)
                return nullptr;
            if (largest == nullptr || m.size > largest->size)
                largest = &m;
        }
        return largest;
    }

    bool isLargestMemberFirst() const {
        const UnionMember *largest = largestMember();
        return largest == nullptr || largest == &members[0];
    }
};

static UnionMember parseUnionMember(const Variable &var,
                                    const Settings &settings)
{
    const Token *nameToken = var.nameToken();
    if (nameToken == nullptr)
        return UnionMember();

    const ValueType *vt = nameToken->valueType();
    size_t size = 0;
    if (var.isArray()) {
        size = var.dimension(0);
    } else if (vt != nullptr) {
        size = vt->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
    }
    return UnionMember(nameToken->str(), size);
}

static std::vector<Union> parseUnions(const SymbolDatabase &symbolDatabase,
                                      const Settings &settings)
{
    std::vector<Union> unions;

    for (const Scope &scope : symbolDatabase.scopeList) {
        if (scope.type != ScopeType::eUnion)
            continue;

        Union u(scope);
        for (const Variable &var : scope.varlist) {
            u.members.push_back(parseUnionMember(var, settings));
        }
        unions.push_back(std::move(u));
    }

    return unions;
}

static bool isZeroInitializer(const Token *tok)
{
    return Token::Match(tok, "= { 0| } ;");
}


void CheckOther::checkUnionZeroInit()
{
    if (!mSettings->severity.isEnabled(Severity::portability))
        return;

    logChecker("CheckOther::checkUnionZeroInit"); // portability

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    std::unordered_map<const Scope *, Union> unionsByScopeId;
    const std::vector<Union> unions = parseUnions(*symbolDatabase, *mSettings);
    for (const Union &u : unions) {
        unionsByScopeId.emplace(u.scope, u);
    }

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isName() || !isZeroInitializer(tok->next()))
            continue;

        const ValueType *vt = tok->valueType();
        if (vt == nullptr || vt->typeScope == nullptr)
            continue;
        auto it = unionsByScopeId.find(vt->typeScope);
        if (it == unionsByScopeId.end())
            continue;
        const Union &u = it->second;
        if (!u.isLargestMemberFirst()) {
            const UnionMember *largestMember = u.largestMember();
            if (largestMember == nullptr) {
                throw InternalError(tok, "Largest union member not found",
                                    InternalError::INTERNAL);
            }
            assert(largestMember != nullptr);
            unionZeroInitError(tok, *largestMember);
        }
    }
}

void CheckOther::unionZeroInitError(const Token *tok,
                                    const UnionMember& largestMember)
{
    reportError(tok, Severity::portability, "UnionZeroInit",
                (tok != nullptr ? "$symbol:" + tok->str() + "\n" : "") +
                "Zero initializing union '$symbol' does not guarantee " +
                "its complete storage to be zero initialized as its largest member " +
                "is not declared as the first member. Consider making " +
                largestMember.name + " the first member or favor memset().");
}

//-----------------------------------------------------------------------------
// Overlapping write (undefined behavior)
//-----------------------------------------------------------------------------
static bool getBufAndOffset(const Token *expr, const Token *&buf, MathLib::bigint *offset, const Settings& settings, MathLib::bigint* sizeValue = nullptr)
{
    if (!expr)
        return false;
    const Token *bufToken, *offsetToken;
    MathLib::bigint elementSize = 0;
    if (expr->isUnaryOp("&") && Token::simpleMatch(expr->astOperand1(), "[")) {
        bufToken = expr->astOperand1()->astOperand1();
        offsetToken = expr->astOperand1()->astOperand2();
        if (expr->astOperand1()->valueType())
            elementSize = expr->astOperand1()->valueType()->getSizeOf(settings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer);
    } else if (Token::Match(expr, "+|-") && expr->isBinaryOp()) {
        const bool pointer1 = (expr->astOperand1()->valueType() && expr->astOperand1()->valueType()->pointer > 0);
        const bool pointer2 = (expr->astOperand2()->valueType() && expr->astOperand2()->valueType()->pointer > 0);
        if (pointer1 && !pointer2) {
            bufToken = expr->astOperand1();
            offsetToken = expr->astOperand2();
            auto vt = *expr->astOperand1()->valueType();
            --vt.pointer;
            elementSize = vt.getSizeOf(settings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer);
        } else if (!pointer1 && pointer2) {
            bufToken = expr->astOperand2();
            offsetToken = expr->astOperand1();
            auto vt = *expr->astOperand2()->valueType();
            --vt.pointer;
            elementSize = vt.getSizeOf(settings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer);
        } else {
            return false;
        }
    } else if (expr->valueType() && expr->valueType()->pointer > 0) {
        buf = expr;
        *offset = 0;
        auto vt = *expr->valueType();
        --vt.pointer;
        elementSize = vt.getSizeOf(settings, ValueType::Accuracy::LowerBound, ValueType::SizeOf::Pointer);
        if (elementSize > 0) {
            *offset *= elementSize;
            if (sizeValue)
                *sizeValue *= elementSize;
        }
        return true;
    } else {
        return false;
    }
    if (!bufToken->valueType() || !bufToken->valueType()->pointer)
        return false;
    if (!offsetToken->hasKnownIntValue())
        return false;
    buf = bufToken;
    *offset = offsetToken->getKnownIntValue();
    if (elementSize > 0) {
        *offset *= elementSize;
        if (sizeValue)
            *sizeValue *= elementSize;
    }
    return true;
}

void CheckOther::checkOverlappingWrite()
{
    logChecker("CheckOther::checkOverlappingWrite");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (tok->isAssignmentOp()) {
                // check if LHS is a union member..
                const Token * const lhs = tok->astOperand1();
                if (!Token::simpleMatch(lhs, ".") || !lhs->isBinaryOp())
                    continue;
                const Variable * const lhsvar = lhs->astOperand1()->variable();
                if (!lhsvar || !lhsvar->typeScope() || lhsvar->typeScope()->type != ScopeType::eUnion)
                    continue;
                const Token* const lhsmember = lhs->astOperand2();
                if (!lhsmember)
                    continue;

                // Is other union member used in RHS?
                const Token *errorToken = nullptr;
                visitAstNodes(tok->astOperand2(), [lhsvar, lhsmember, &errorToken](const Token *rhs) {
                    if (!Token::simpleMatch(rhs, "."))
                        return ChildrenToVisit::op1_and_op2;
                    if (!rhs->isBinaryOp() || rhs->astOperand1()->variable() != lhsvar)
                        return ChildrenToVisit::none;
                    if (lhsmember->str() == rhs->astOperand2()->str())
                        return ChildrenToVisit::none;
                    const Variable* rhsmembervar = rhs->astOperand2()->variable();
                    const Scope* varscope1 = lhsmember->variable() ? lhsmember->variable()->typeStartToken()->scope() : nullptr;
                    const Scope* varscope2 = rhsmembervar ? rhsmembervar->typeStartToken()->scope() : nullptr;
                    if (varscope1 && varscope1 == varscope2 && varscope1 != lhsvar->typeScope())
                        // lhsmember and rhsmember are declared in same anonymous scope inside union
                        return ChildrenToVisit::none;
                    errorToken = rhs->astOperand2();
                    return ChildrenToVisit::done;
                });
                if (errorToken)
                    overlappingWriteUnion(tok);
            } else if (Token::Match(tok, "%name% (")) {
                const Library::NonOverlappingData *nonOverlappingData = mSettings->library.getNonOverlappingData(tok);
                if (!nonOverlappingData)
                    continue;
                const std::vector<const Token *> args = getArguments(tok);
                if (nonOverlappingData->ptr1Arg <= 0 || nonOverlappingData->ptr1Arg > args.size())
                    continue;
                if (nonOverlappingData->ptr2Arg <= 0 || nonOverlappingData->ptr2Arg > args.size())
                    continue;

                const Token *ptr1 = args[nonOverlappingData->ptr1Arg - 1];
                if (ptr1->hasKnownIntValue() && ptr1->getKnownIntValue() == 0)
                    continue;

                const Token *ptr2 = args[nonOverlappingData->ptr2Arg - 1];
                if (ptr2->hasKnownIntValue() && ptr2->getKnownIntValue() == 0)
                    continue;

                // TODO: nonOverlappingData->strlenArg
                const int sizeArg = std::max(nonOverlappingData->sizeArg, nonOverlappingData->countArg);
                if (sizeArg <= 0 || sizeArg > args.size()) {
                    if (nonOverlappingData->sizeArg == -1) {
                        ErrorPath errorPath;
                        constexpr bool macro = true;
                        constexpr bool pure = true;
                        constexpr bool follow = true;
                        if (!isSameExpression(macro, ptr1, ptr2, *mSettings, pure, follow, &errorPath))
                            continue;
                        overlappingWriteFunction(tok, tok->str());
                    }
                    continue;
                }
                const bool isCountArg = nonOverlappingData->countArg > 0;
                if (!args[sizeArg-1]->hasKnownIntValue())
                    continue;
                MathLib::bigint sizeValue = args[sizeArg-1]->getKnownIntValue();
                const Token *buf1, *buf2;
                MathLib::bigint offset1, offset2;
                if (!getBufAndOffset(ptr1, buf1, &offset1, *mSettings, isCountArg ? &sizeValue : nullptr))
                    continue;
                if (!getBufAndOffset(ptr2, buf2, &offset2, *mSettings))
                    continue;

                if (offset1 < offset2 && offset1 + sizeValue <= offset2)
                    continue;
                if (offset2 < offset1 && offset2 + sizeValue <= offset1)
                    continue;

                ErrorPath errorPath;
                constexpr bool macro = true;
                constexpr bool pure = true;
                constexpr bool follow = true;
                if (!isSameExpression(macro, buf1, buf2, *mSettings, pure, follow, &errorPath))
                    continue;
                overlappingWriteFunction(tok, tok->str());
            }
        }
    }
}

void CheckOther::overlappingWriteUnion(const Token *tok)
{
    reportError(tok, Severity::error, "overlappingWriteUnion", "Overlapping read/write of union is undefined behavior");
}

void CheckOther::overlappingWriteFunction(const Token *tok, const std::string& funcname)
{
    reportError(tok, Severity::error, "overlappingWriteFunction", "Overlapping read/write in " + funcname + "() is undefined behavior");
}

void CheckOther::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckOther checkOther(&tokenizer, &tokenizer.getSettings(), errorLogger);

    // Checks
    checkOther.warningOldStylePointerCast();
    checkOther.warningDangerousTypeCast();
    checkOther.warningIntToPointerCast();
    checkOther.suspiciousFloatingPointCast();
    checkOther.invalidPointerCast();
    checkOther.checkCharVariable();
    checkOther.redundantBitwiseOperationInSwitchError();
    checkOther.checkSuspiciousCaseInSwitch();
    checkOther.checkDuplicateBranch();
    checkOther.checkDuplicateExpression();
    checkOther.checkRedundantAssignment();
    checkOther.checkUnreachableCode();
    checkOther.checkSuspiciousSemicolon();
    checkOther.checkVariableScope();
    checkOther.checkSignOfUnsignedVariable();  // don't ignore casts (#3574)
    checkOther.checkIncompleteArrayFill();
    checkOther.checkVarFuncNullUB();
    checkOther.checkNanInArithmeticExpression();
    checkOther.checkCommaSeparatedReturn();
    checkOther.checkRedundantPointerOp();
    checkOther.checkZeroDivision();
    checkOther.checkNegativeBitwiseShift();
    checkOther.checkInterlockedDecrement();
    checkOther.checkUnusedLabel();
    checkOther.checkEvaluationOrder();
    checkOther.checkFuncArgNamesDifferent();
    checkOther.checkShadowVariables();
    checkOther.checkKnownArgument();
    checkOther.checkKnownPointerToBool();
    checkOther.checkComparePointers();
    checkOther.checkIncompleteStatement();
    checkOther.checkRedundantCopy();
    checkOther.clarifyCalculation();
    checkOther.checkPassByReference();
    checkOther.checkConstVariable();
    checkOther.checkConstPointer();
    checkOther.checkComparisonFunctionIsAlwaysTrueOrFalse();
    checkOther.checkInvalidFree();
    checkOther.clarifyStatement();
    checkOther.checkCastIntToCharAndBack();
    checkOther.checkMisusedScopedObject();
    checkOther.checkAccessOfMovedVariable();
    checkOther.checkModuloOfOne();
    checkOther.checkOverlappingWrite();
    checkOther.checkUnionZeroInit();
}

void CheckOther::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckOther c(nullptr, settings, errorLogger);

    // error
    c.zerodivError(nullptr, nullptr);
    c.misusedScopeObjectError(nullptr, "varname");
    c.invalidPointerCastError(nullptr,  "float *", "double *", false, false);
    c.negativeBitwiseShiftError(nullptr, 1);
    c.negativeBitwiseShiftError(nullptr, 2);
    c.raceAfterInterlockedDecrementError(nullptr);
    c.invalidFreeError(nullptr, "malloc", false);
    c.overlappingWriteUnion(nullptr);
    c.overlappingWriteFunction(nullptr, "funcname");

    //performance
    c.redundantCopyError(nullptr,  "varname");
    c.redundantCopyError(nullptr, nullptr, "var");

    // style/warning
    c.checkComparisonFunctionIsAlwaysTrueOrFalseError(nullptr, "isless","varName",false);
    c.checkCastIntToCharAndBackError(nullptr, "func_name");
    c.cstyleCastError(nullptr);
    c.dangerousTypeCastError(nullptr, true);
    c.intToPointerCastError(nullptr, "decimal");
    c.suspiciousFloatingPointCastError(nullptr);
    c.passedByValueError(nullptr, false);
    // TODO: iterateByValue
    // TODO: passedByValueCallback
    c.constVariableError(nullptr, nullptr);
    c.constStatementError(nullptr, "type", false);
    c.signedCharArrayIndexError(nullptr);
    c.unknownSignCharArrayIndexError(nullptr);
    c.charBitOpError(nullptr);
    c.variableScopeError(nullptr,  "varname");
    c.redundantAssignmentInSwitchError(nullptr, nullptr, "var");
    c.suspiciousCaseInSwitchError(nullptr,  "||");
    c.selfAssignmentError(nullptr,  "varname");
    c.clarifyCalculationError(nullptr,  "+");
    c.clarifyStatementError(nullptr);
    c.duplicateBranchError(nullptr, nullptr, ErrorPath{});
    c.duplicateAssignExpressionError(nullptr, nullptr, true);
    c.oppositeExpressionError(nullptr, ErrorPath{});
    c.duplicateExpressionError(nullptr, nullptr, nullptr, ErrorPath{});
    c.duplicateValueTernaryError(nullptr);
    c.duplicateExpressionTernaryError(nullptr, ErrorPath{});
    c.duplicateBreakError(nullptr,  false);
    c.unreachableCodeError(nullptr, nullptr,  false);
    c.unsignedLessThanZeroError(nullptr, nullptr, "varname");
    c.unsignedPositiveError(nullptr, nullptr, "varname");
    c.pointerLessThanZeroError(nullptr, nullptr);
    c.pointerPositiveError(nullptr, nullptr);
    c.suspiciousSemicolonError(nullptr);
    c.incompleteArrayFillError(nullptr,  "buffer", "memset", false);
    c.varFuncNullUBError(nullptr);
    c.nanInArithmeticExpressionError(nullptr);
    c.commaSeparatedReturnError(nullptr);
    c.redundantPointerOpError(nullptr,  "varname", false, /*addressOfDeref*/ true);
    c.unusedLabelError(nullptr, false, false);
    c.unusedLabelError(nullptr, false, true);
    c.unusedLabelError(nullptr, true, false);
    c.unusedLabelError(nullptr, true, true);
    c.unknownEvaluationOrder(nullptr);
    c.accessMovedError(nullptr, "v", nullptr, false);
    c.funcArgNamesDifferent("function", 1, nullptr, nullptr);
    c.redundantBitwiseOperationInSwitchError(nullptr, "varname");
    c.shadowError(nullptr, nullptr, "variable");
    c.shadowError(nullptr, nullptr, "function");
    c.shadowError(nullptr, nullptr, "argument");
    c.knownArgumentError(nullptr, nullptr, nullptr, "x", false);
    c.knownPointerToBoolError(nullptr, nullptr);
    c.comparePointersError(nullptr, nullptr, nullptr);
    // TODO: subtractPointers
    c.redundantAssignmentError(nullptr, nullptr, "var", false);
    c.redundantInitializationError(nullptr, nullptr, "var", false);
    c.redundantContinueError(nullptr);

    const std::vector<const Token *> nullvec;
    c.funcArgOrderDifferent("function", nullptr, nullptr, nullvec, nullvec);
    c.checkModuloOfOneError(nullptr);
    c.unionZeroInitError(nullptr, UnionMember());
}
