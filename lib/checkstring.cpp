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
#include "checkstring.h"

#include "astutils.h"
#include "errortypes.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"

#include <cstddef>
#include <list>
#include <vector>
#include <utility>

//---------------------------------------------------------------------------

// CWE ids used:
static const CWE CWE570(570U);   // Expression is Always False
static const CWE CWE571(571U);   // Expression is Always True
static const CWE CWE595(595U);   // Comparison of Object References Instead of Object Contents
static const CWE CWE628(628U);   // Function Call with Incorrectly Specified Arguments
static const CWE CWE665(665U);   // Improper Initialization
static const CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior

//---------------------------------------------------------------------------
// Writing string literal is UB
//---------------------------------------------------------------------------
void CheckStringImpl::stringLiteralWrite()
{
    logChecker("CheckString::stringLiteralWrite");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->variable() || !tok->variable()->isPointer())
                continue;
            const Token *str = tok->getValueTokenMinStrSize(mSettings);
            if (!str)
                continue;
            if (Token::Match(tok, "%var% [") && Token::simpleMatch(tok->linkAt(1), "] ="))
                stringLiteralWriteError(tok, str);
            else if (Token::Match(tok->previous(), "* %var% ="))
                stringLiteralWriteError(tok, str);
        }
    }
}

void CheckStringImpl::stringLiteralWriteError(const Token *tok, const Token *strValue)
{
    std::list<const Token*> callstack{ tok };
    if (strValue)
        callstack.push_back(strValue);

    std::string errmsg("Modifying string literal");
    if (strValue) {
        std::string s = strValue->str();
        // 20 is an arbitrary value, the max string length shown in a warning message
        if (s.size() > 20U)
            s.replace(17, std::string::npos, "..\"");
        errmsg += " " + s;
    }
    errmsg += " directly or indirectly is undefined behaviour.";

    reportError(callstack, Severity::error, "stringLiteralWrite", errmsg, CWE758, Certainty::normal);
}

//---------------------------------------------------------------------------
// Check for string comparison involving two static strings.
// if(strcmp("00FF00","00FF00")==0) // <- statement is always true
//---------------------------------------------------------------------------
void CheckStringImpl::checkAlwaysTrueOrFalseStringCompare()
{
    if (!mSettings.severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckString::checkAlwaysTrueOrFalseStringCompare"); // warning

    for (const Token* tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->isName() && tok->strAt(1) == "(" && Token::Match(tok, "memcmp|strncmp|strcmp|stricmp|strverscmp|bcmp|strcmpi|strcasecmp|strncasecmp|strncasecmp_l|strcasecmp_l|wcsncasecmp|wcscasecmp|wmemcmp|wcscmp|wcscasecmp_l|wcsncasecmp_l|wcsncmp|_mbscmp|_mbscmp_l|_memicmp|_memicmp_l|_stricmp|_wcsicmp|wcsicmp|_mbsicmp|_stricmp_l|_wcsicmp_l|_mbsicmp_l")) {
            if (Token::Match(tok->tokAt(2), "%str% , %str% ,|)")) {
                const std::string &str1 = tok->strAt(2);
                const std::string &str2 = tok->strAt(4);
                if (!tok->isExpandedMacro() && !tok->tokAt(2)->isExpandedMacro() && !tok->tokAt(4)->isExpandedMacro())
                    alwaysTrueFalseStringCompareError(tok, str1, str2);
                tok = tok->tokAt(5);
            } else if (Token::Match(tok->tokAt(2), "%name% , %name% ,|)")) {
                const std::string &str1 = tok->strAt(2);
                const std::string &str2 = tok->strAt(4);
                if (str1 == str2)
                    alwaysTrueStringVariableCompareError(tok, str1, str2);
                tok = tok->tokAt(5);
            } else if (Token::Match(tok->tokAt(2), "%name% . c_str ( ) , %name% . c_str ( ) ,|)")) {
                const std::string &str1 = tok->strAt(2);
                const std::string &str2 = tok->strAt(8);
                if (str1 == str2)
                    alwaysTrueStringVariableCompareError(tok, str1, str2);
                tok = tok->tokAt(13);
            }
        } else if (tok->isName() && Token::Match(tok, "QString :: compare ( %str% , %str% )")) {
            const std::string &str1 = tok->strAt(4);
            const std::string &str2 = tok->strAt(6);
            alwaysTrueFalseStringCompareError(tok, str1, str2);
            tok = tok->tokAt(7);
        } else if (Token::Match(tok, "!!+ %str% ==|!= %str% !!+")) {
            const std::string &str1 = tok->strAt(1);
            const std::string &str2 = tok->strAt(3);
            alwaysTrueFalseStringCompareError(tok, str1, str2);
            tok = tok->tokAt(5);
        }
        if (!tok)
            break;
    }
}

void CheckStringImpl::alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2)
{
    constexpr std::size_t stringLen = 10;
    const std::string string1 = (str1.size() < stringLen) ? str1 : (str1.substr(0, stringLen-2) + "..");
    const std::string string2 = (str2.size() < stringLen) ? str2 : (str2.substr(0, stringLen-2) + "..");

    reportError(tok, Severity::warning, "staticStringCompare",
                "Unnecessary comparison of static strings.\n"
                "The compared strings, '" + string1 + "' and '" + string2 + "', are always " + (str1==str2?"identical":"unequal") + ". "
                "Therefore the comparison is unnecessary and looks suspicious.", (str1==str2)?CWE571:CWE570, Certainty::normal);
}

void CheckStringImpl::alwaysTrueStringVariableCompareError(const Token *tok, const std::string& str1, const std::string& str2)
{
    reportError(tok, Severity::warning, "stringCompare",
                "Comparison of identical string variables.\n"
                "The compared strings, '" + str1 + "' and '" + str2 + "', are identical. "
                "This could be a logic bug.", CWE571, Certainty::normal);
}


//-----------------------------------------------------------------------------
// Detect "str == '\0'" where "*str == '\0'" is correct.
// Comparing char* with each other instead of using strcmp()
//-----------------------------------------------------------------------------
void CheckStringImpl::checkSuspiciousStringCompare()
{
    if (!mSettings.severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckString::checkSuspiciousStringCompare"); // warning

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->isComparisonOp())
                continue;

            const Token* varTok = tok->astOperand1();
            const Token* litTok = tok->astOperand2();
            if (!varTok || !litTok)  // <- failed to create AST for comparison
                continue;
            if (Token::Match(varTok, "%char%|%num%|%str%"))
                std::swap(varTok, litTok);
            else if (!Token::Match(litTok, "%char%|%num%|%str%"))
                continue;

            if (varTok->isLiteral())
                continue;

            const ValueType* varType = varTok->valueType();
            if (varTok->isCpp() && (!varType || !varType->isIntegral()))
                continue;

            if (litTok->tokType() == Token::eString) {
                if (varTok->isC() || (varType && varType->pointer))
                    suspiciousStringCompareError(tok, varTok->expressionString(), litTok->isLong());
            } else if (litTok->tokType() == Token::eChar && varType && varType->pointer) {
                suspiciousStringCompareError_char(tok, varTok->expressionString());
            }
        }
    }
}

void CheckStringImpl::suspiciousStringCompareError(const Token* tok, const std::string& var, bool isLong)
{
    const std::string cmpFunc = isLong ? "wcscmp" : "strcmp";
    reportError(tok, Severity::warning, "literalWithCharPtrCompare",
                "$symbol:" + var + "\nString literal compared with variable '$symbol'. Did you intend to use " + cmpFunc + "() instead?", CWE595, Certainty::normal);
}

void CheckStringImpl::suspiciousStringCompareError_char(const Token* tok, const std::string& var)
{
    reportError(tok, Severity::warning, "charLiteralWithCharPtrCompare",
                "$symbol:" + var + "\nChar literal compared with pointer '$symbol'. Did you intend to dereference it?", CWE595, Certainty::normal);
}


//---------------------------------------------------------------------------
// Adding C-string and char with operator+
//---------------------------------------------------------------------------

static bool isChar(const Variable* var)
{
    return (var && !var->isPointer() && !var->isArray() && (var->typeStartToken()->str() == "char" || var->typeStartToken()->str() == "wchar_t"));
}

void CheckStringImpl::strPlusChar()
{
    logChecker("CheckString::strPlusChar");
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->str() == "+") {
                if (tok->astOperand1() && (tok->astOperand1()->tokType() == Token::eString)) { // string literal...
                    if (tok->astOperand2() && (tok->astOperand2()->tokType() == Token::eChar || isChar(tok->astOperand2()->variable()))) // added to char variable or char constant
                        strPlusCharError(tok);
                }
            }
        }
    }
}

void CheckStringImpl::strPlusCharError(const Token *tok)
{
    std::string charType = "char";
    if (tok && tok->astOperand2() && tok->astOperand2()->variable())
        charType = tok->astOperand2()->variable()->typeStartToken()->str();
    else if (tok && tok->astOperand2() && tok->astOperand2()->tokType() == Token::eChar && tok->astOperand2()->isLong())
        charType = "wchar_t";
    reportError(tok, Severity::error, "strPlusChar", "Unusual pointer arithmetic. A value of type '" + charType +"' is added to a string literal.", CWE665, Certainty::normal);
}

static bool isMacroUsage(const Token* tok)
{
    if (const Token* parent = tok->astParent()) {
        while (parent && (parent->isCast() || parent->str() == "&&")) {
            if (parent->isExpandedMacro())
                return true;
            parent = parent->astParent();
        }
        if (!parent)
            return false;
        if (parent->isExpandedMacro())
            return true;
        if (parent->isUnaryOp("!") || parent->isComparisonOp()) {
            int argn{};
            const Token* ftok = getTokenArgumentFunction(parent, argn);
            if (ftok && !ftok->function())
                return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
// Implicit casts of string literals to bool
// Comparing string literal with strlen() with wrong length
//---------------------------------------------------------------------------
void CheckStringImpl::checkIncorrectStringCompare()
{
    if (!mSettings.severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckString::checkIncorrectStringCompare"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            // skip "assert(str && ..)" and "assert(.. && str)"
            if ((endsWith(tok->str(), "assert") || endsWith(tok->str(), "ASSERT")) &&
                Token::Match(tok, "%name% (") &&
                (Token::Match(tok->tokAt(2), "%str% &&") || Token::Match(tok->linkAt(1)->tokAt(-2), "&& %str% )")))
                tok = tok->linkAt(1);

            if (Token::simpleMatch(tok, ". substr (") && Token::Match(tok->tokAt(3)->nextArgument(), "%num% )")) {
                const MathLib::biguint clen = MathLib::toBigUNumber(tok->linkAt(2)->tokAt(-1));
                const Token* begin = tok->previous();
                for (;;) { // Find start of statement
                    while (begin->link() && Token::Match(begin, "]|)|>"))
                        begin = begin->link()->previous();
                    if (Token::Match(begin->previous(), ".|::"))
                        begin = begin->tokAt(-2);
                    else
                        break;
                }
                begin = begin->previous();
                const Token* end = tok->linkAt(2)->next();
                if (Token::Match(begin->previous(), "%str% ==|!=") && begin->strAt(-2) != "+") {
                    const std::size_t slen = Token::getStrLength(begin->previous());
                    if (clen != slen) {
                        incorrectStringCompareError(tok->next(), "substr", begin->strAt(-1));
                    }
                } else if (Token::Match(end, "==|!= %str% !!+")) {
                    const std::size_t slen = Token::getStrLength(end->next());
                    if (clen != slen) {
                        incorrectStringCompareError(tok->next(), "substr", end->strAt(1));
                    }
                }
            } else if (Token::Match(tok, "%str%|%char%") &&
                       !Token::Match(tok->next(), "%name%") &&
                       isUsedAsBool(tok, mSettings) &&
                       !isMacroUsage(tok))
                incorrectStringBooleanError(tok, tok->str());
        }
    }
}

void CheckStringImpl::incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string)
{
    reportError(tok, Severity::warning, "incorrectStringCompare", "$symbol:" + func + "\nString literal " + string + " doesn't match length argument for $symbol().", CWE570, Certainty::normal);
}

void CheckStringImpl::incorrectStringBooleanError(const Token *tok, const std::string& string)
{
    const bool charLiteral = isCharLiteral(string);
    const std::string literalType = charLiteral ? "char" : "string";
    const std::string result = bool_to_string(getCharLiteral(string) != "\\0");
    reportError(tok,
                Severity::warning,
                charLiteral ? "incorrectCharBooleanError" : "incorrectStringBooleanError",
                "Conversion of " + literalType + " literal " + string + " to bool always evaluates to " + result + '.', CWE571, Certainty::normal);
}

//---------------------------------------------------------------------------
// always true: strcmp(str,"a")==0 || strcmp(str,"b")
// TODO: Library configuration for string comparison functions
//---------------------------------------------------------------------------
void CheckStringImpl::overlappingStrcmp()
{
    if (!mSettings.severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckString::overlappingStrcmp"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->str() != "||")
                continue;
            std::list<const Token *> equals0;
            std::list<const Token *> notEquals0;
            visitAstNodes(tok, [&](const Token * t) {
                if (!t)
                    return ChildrenToVisit::none;
                if (t->str() == "||") {
                    return ChildrenToVisit::op1_and_op2;
                }
                if (t->str() == "==") {
                    if (Token::simpleMatch(t->astOperand1(), "(") && Token::simpleMatch(t->astOperand2(), "0"))
                        equals0.push_back(t->astOperand1());
                    else if (Token::simpleMatch(t->astOperand2(), "(") && Token::simpleMatch(t->astOperand1(), "0"))
                        equals0.push_back(t->astOperand2());
                    return ChildrenToVisit::none;
                }
                if (t->str() == "!=") {
                    if (Token::simpleMatch(t->astOperand1(), "(") && Token::simpleMatch(t->astOperand2(), "0"))
                        notEquals0.push_back(t->astOperand1());
                    else if (Token::simpleMatch(t->astOperand2(), "(") && Token::simpleMatch(t->astOperand1(), "0"))
                        notEquals0.push_back(t->astOperand2());
                    return ChildrenToVisit::none;
                }
                if (t->str() == "!" && Token::simpleMatch(t->astOperand1(), "("))
                    equals0.push_back(t->astOperand1());
                else if (t->str() == "(")
                    notEquals0.push_back(t);
                return ChildrenToVisit::none;
            });

            for (const Token *eq0 : equals0) {
                for (const Token * ne0 : notEquals0) {
                    if (!Token::Match(eq0->previous(), "strcmp|wcscmp ("))
                        continue;
                    if (!Token::Match(ne0->previous(), "strcmp|wcscmp ("))
                        continue;
                    const std::vector<const Token *> args1 = getArguments(eq0->previous());
                    const std::vector<const Token *> args2 = getArguments(ne0->previous());
                    if (args1.size() != 2 || args2.size() != 2)
                        continue;
                    if (args1[1]->isLiteral() &&
                        args2[1]->isLiteral() &&
                        args1[1]->str() != args2[1]->str() &&
                        isSameExpression(true, args1[0], args2[0], mSettings, true, false))
                        overlappingStrcmpError(eq0, ne0);
                }
            }
        }
    }
}

void CheckStringImpl::overlappingStrcmpError(const Token *eq0, const Token *ne0)
{
    std::string eq0Expr(eq0 ? eq0->expressionString() : std::string("strcmp(x,\"abc\")"));
    if (eq0 && eq0->astParent()->str() == "!")
        eq0Expr = "!" + eq0Expr;
    else
        eq0Expr += " == 0";

    const std::string ne0Expr = (ne0 ? ne0->expressionString() : std::string("strcmp(x,\"def\")")) + " != 0";

    reportError(ne0, Severity::warning, "overlappingStrcmp", "The expression '" + ne0Expr + "' is suspicious. It overlaps '" + eq0Expr + "'.");
}

//---------------------------------------------------------------------------
// Overlapping source and destination passed to sprintf().
// TODO: Library configuration for overlapping arguments
//---------------------------------------------------------------------------
void CheckStringImpl::sprintfOverlappingData()
{
    logChecker("CheckString::sprintfOverlappingData");

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "sprintf|snprintf|swprintf ("))
                continue;

            const std::vector<const Token *> args = getArguments(tok);

            const int formatString = Token::simpleMatch(tok, "sprintf") ? 1 : 2;
            for (unsigned int argnr = formatString + 1; argnr < args.size(); ++argnr) {
                const Token *dest = args[0];
                while (dest->isCast())
                    dest = dest->astOperand2() ? dest->astOperand2() : dest->astOperand1();
                const Token *arg = args[argnr];
                if (!arg->valueType() || arg->valueType()->pointer != 1)
                    continue;
                while (arg->isCast())
                    arg = arg->astOperand2() ? arg->astOperand2() : arg->astOperand1();

                const bool same = isSameExpression(false,
                                                   dest,
                                                   arg,
                                                   mSettings,
                                                   true,
                                                   false);
                if (same) {
                    sprintfOverlappingDataError(tok, args[argnr], arg->expressionString());
                }
            }
        }
    }
}

void CheckStringImpl::sprintfOverlappingDataError(const Token *funcTok, const Token *tok, const std::string &varname)
{
    const std::string func = funcTok ? funcTok->str() : "s[n]printf";

    reportError(tok, Severity::error, "sprintfOverlappingData",
                "$symbol:" + varname + "\n"
                "Undefined behavior: Variable '$symbol' is used as parameter and destination in " + func + "().\n" +
                "The variable '$symbol' is used both as a parameter and as destination in " +
                func + "(). The origin and destination buffers overlap. Quote from glibc (C-library) "
                "documentation (http://www.gnu.org/software/libc/manual/html_mono/libc.html#Formatted-Output-Functions): "
                "\"If copying takes place between objects that overlap as a result of a call "
                "to sprintf() or snprintf(), the results are undefined.\"", CWE628, Certainty::normal);
}

void CheckString::runChecks(const Tokenizer &tokenizer, ErrorLogger& errorLogger)
{
    CheckStringImpl checkString(&tokenizer, tokenizer.getSettings(), errorLogger);

    // Checks
    checkString.strPlusChar();
    checkString.checkSuspiciousStringCompare();
    checkString.stringLiteralWrite();
    checkString.overlappingStrcmp();
    checkString.checkIncorrectStringCompare();
    checkString.sprintfOverlappingData();
    checkString.checkAlwaysTrueOrFalseStringCompare();
}

void CheckString::getErrorMessages(ErrorLogger& errorLogger, const Settings &settings) const
{
    CheckStringImpl c(nullptr, settings, errorLogger);
    c.stringLiteralWriteError(nullptr, nullptr);
    c.sprintfOverlappingDataError(nullptr, nullptr, "varname");
    c.strPlusCharError(nullptr);
    c.incorrectStringCompareError(nullptr, "substr", "\"Hello World\"");
    c.suspiciousStringCompareError(nullptr, "foo", false);
    c.suspiciousStringCompareError_char(nullptr, "foo");
    c.incorrectStringBooleanError(nullptr, "\"Hello World\"");
    c.incorrectStringBooleanError(nullptr, "\'x\'");
    c.alwaysTrueFalseStringCompareError(nullptr, "str1", "str2");
    c.alwaysTrueStringVariableCompareError(nullptr, "varname1", "varname2");
    c.overlappingStrcmpError(nullptr, nullptr);
}
