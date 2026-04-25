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
// Check for condition mismatches
//---------------------------------------------------------------------------

#include "checkcondition.h"

#include "astutils.h"
#include "library.h"
#include "platform.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"
#include "vfvalue.h"

#include "checkother.h" // comparisonNonZeroExpressionLessThanZero and testIfNonZeroExpressionIsPositive

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <list>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

// CWE ids used
static const CWE uncheckedErrorConditionCWE(391U);
static const CWE CWE398(398U);   // Indicator of Poor Code Quality
static const CWE CWE570(570U);   // Expression is Always False
static const CWE CWE571(571U);   // Expression is Always True

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckCondition instance;
}

bool CheckCondition::diag(const Token* tok, bool insert)
{
    if (!tok)
        return false;
    const Token* parent = tok->astParent();
    bool hasParent = false;
    while (Token::Match(parent, "!|&&|%oror%")) {
        if (mCondDiags.count(parent) != 0) {
            hasParent = true;
            break;
        }
        parent = parent->astParent();
    }
    if (mCondDiags.count(tok) == 0 && !hasParent) {
        if (insert)
            mCondDiags.insert(tok);
        return false;
    }
    return true;
}

bool CheckCondition::diag(const Token* tok1, const Token* tok2)
{
    const bool b1 = diag(tok1);
    const bool b2 = diag(tok2);
    return b1 && b2;
}

bool CheckCondition::isAliased(const std::set<int> &vars) const
{
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "= & %var% ;") && vars.find(tok->tokAt(2)->varId()) != vars.end())
            return true;
    }
    return false;
}

void CheckCondition::assignIf()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("assignIfError"))
        return;

    logChecker("CheckCondition::assignIf"); // style

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "=")
            continue;

        if (Token::Match(tok->tokAt(-2), "[;{}] %var% =")) {
            const Variable *var = tok->previous()->variable();
            if (var == nullptr)
                continue;

            char bitop = '\0';
            MathLib::bigint num = 0;

            if (Token::Match(tok->next(), "%num% [&|]")) {
                bitop = tok->strAt(2).at(0);
                num = MathLib::toBigNumber(tok->tokAt(1));
            } else {
                const Token *endToken = Token::findsimplematch(tok, ";");

                // Casting address
                if (endToken && Token::Match(endToken->tokAt(-4), "* ) & %any% ;"))
                    endToken = nullptr;

                if (endToken && Token::Match(endToken->tokAt(-2), "[&|] %num% ;")) {
                    bitop = endToken->strAt(-2).at(0);
                    num = MathLib::toBigNumber(endToken->tokAt(-1));
                }
            }

            if (bitop == '\0')
                continue;

            if (num < 0 && bitop == '|')
                continue;

            assignIfParseScope(tok, tok->tokAt(4), var->declarationId(), var->isLocal(), bitop, num);
        }
    }
}

static bool isParameterChanged(const Token *partok)
{
    bool addressOf = Token::Match(partok, "[(,] &");
    int argumentNumber = 0;
    const Token *ftok;
    for (ftok = partok; ftok && ftok->str() != "("; ftok = ftok->previous()) {
        if (ftok->str() == ")")
            ftok = ftok->link();
        else if (argumentNumber == 0U && ftok->str() == "&")
            addressOf = true;
        else if (ftok->str() == ",")
            argumentNumber++;
    }
    ftok = ftok ? ftok->previous() : nullptr;
    if (!(ftok && ftok->function()))
        return true;
    const Variable *par = ftok->function()->getArgumentVar(argumentNumber);
    if (!par)
        return true;
    if (par->isConst())
        return false;
    if (addressOf || par->isReference() || par->isPointer())
        return true;
    return false;
}

/** parse scopes recursively */
bool CheckCondition::assignIfParseScope(const Token * const assignTok,
                                        const Token * const startTok,
                                        const nonneg int varid,
                                        const bool islocal,
                                        const char bitop,
                                        const MathLib::bigint num)
{
    bool ret = false;

    for (const Token *tok2 = startTok; tok2; tok2 = tok2->next()) {
        if ((bitop == '&') && Token::Match(tok2->tokAt(2), "%varid% %cop% %num% ;", varid) && tok2->strAt(3) == std::string(1U, bitop)) {
            const MathLib::bigint num2 = MathLib::toBigNumber(tok2->tokAt(4));
            if (0 == (num & num2))
                mismatchingBitAndError(assignTok, num, tok2, num2);
        }
        if (Token::Match(tok2, "%varid% =", varid)) {
            return true;
        }
        if (bitop == '&' && Token::Match(tok2, "%varid% &= %num% ;", varid)) {
            const MathLib::bigint num2 = MathLib::toBigNumber(tok2->tokAt(2));
            if (0 == (num & num2))
                mismatchingBitAndError(assignTok, num, tok2, num2);
        }
        if (Token::Match(tok2, "++|-- %varid%", varid) || Token::Match(tok2, "%varid% ++|--", varid))
            return true;
        if (Token::Match(tok2, "[(,] &| %varid% [,)]", varid) && isParameterChanged(tok2))
            return true;
        if (tok2->str() == "}")
            return false;
        if (Token::Match(tok2, "break|continue|return"))
            ret = true;
        if (ret && tok2->str() == ";")
            return false;
        if (!islocal && Token::Match(tok2, "%name% (") && !Token::simpleMatch(tok2->linkAt(1), ") {"))
            return true;
        if (Token::Match(tok2, "if|while (")) {
            if (!islocal && tok2->str() == "while")
                continue;
            if (tok2->str() == "while") {
                // is variable changed in loop?
                const Token *bodyStart = tok2->linkAt(1)->next();
                const Token *bodyEnd   = bodyStart ? bodyStart->link() : nullptr;
                if (!bodyEnd || bodyEnd->str() != "}" || isVariableChanged(bodyStart, bodyEnd, varid, !islocal, *mSettings))
                    continue;
            }

            // parse condition
            const Token * const end = tok2->linkAt(1);
            for (; tok2 != end; tok2 = tok2->next()) {
                if (Token::Match(tok2, "[(,] &| %varid% [,)]", varid)) {
                    return true;
                }
                if (Token::Match(tok2,"&&|%oror%|( %varid% ==|!= %num% &&|%oror%|)", varid)) {
                    const Token *vartok = tok2->next();
                    const MathLib::bigint num2 = MathLib::toBigNumber(vartok->tokAt(2));
                    if ((num & num2) != ((bitop=='&') ? num2 : num)) {
                        const std::string& op(vartok->strAt(1));
                        const bool alwaysTrue = op == "!=";
                        const std::string condition(vartok->str() + op + vartok->strAt(2));
                        assignIfError(assignTok, tok2, condition, alwaysTrue);
                    }
                }
                if (Token::Match(tok2, "%varid% %op%", varid) && tok2->next()->isAssignmentOp()) {
                    return true;
                }
            }

            const bool ret1 = assignIfParseScope(assignTok, end->tokAt(2), varid, islocal, bitop, num);
            bool ret2 = false;
            if (Token::simpleMatch(end->linkAt(1), "} else {"))
                ret2 = assignIfParseScope(assignTok, end->linkAt(1)->tokAt(3), varid, islocal, bitop, num);
            if (ret1 || ret2)
                return true;
        }
    }
    return false;
}

void CheckCondition::assignIfError(const Token *tok1, const Token *tok2, const std::string &condition, bool result)
{
    if (tok2 && diag(tok2->tokAt(2)))
        return;
    std::list<const Token *> locations = { tok1, tok2 };
    reportError(locations,
                Severity::style,
                "assignIfError",
                "Mismatching assignment and comparison, comparison '" + condition + "' is always " + std::string(bool_to_string(result)) + ".", CWE398, Certainty::normal);
}


void CheckCondition::mismatchingBitAndError(const Token *tok1, const MathLib::bigint num1, const Token *tok2, const MathLib::bigint num2)
{
    std::list<const Token *> locations = { tok1, tok2 };

    std::ostringstream msg;
    msg << "Mismatching bitmasks. Result is always 0 ("
        << "X = Y & 0x" << std::hex << num1 << "; Z = X & 0x" << std::hex << num2 << "; => Z=0).";

    reportError(locations,
                Severity::style,
                "mismatchingBitAnd",
                msg.str(), CWE398, Certainty::normal);
}


static void getnumchildren(const Token *tok, std::list<MathLib::bigint> &numchildren)
{
    if (tok->astOperand1() && tok->astOperand1()->isNumber())
        numchildren.push_back(MathLib::toBigNumber(tok->astOperand1()));
    else if (tok->astOperand1() && tok->str() == tok->astOperand1()->str())
        getnumchildren(tok->astOperand1(), numchildren);
    if (tok->astOperand2() && tok->astOperand2()->isNumber())
        numchildren.push_back(MathLib::toBigNumber(tok->astOperand2()));
    else if (tok->astOperand2() && tok->str() == tok->astOperand2()->str())
        getnumchildren(tok->astOperand2(), numchildren);
}

/* Return whether tok is in the body for a function returning a boolean. */
static bool inBooleanFunction(const Token *tok)
{
    const Scope *scope = tok ? tok->scope() : nullptr;
    while (scope && scope->isLocal())
        scope = scope->nestedIn;
    if (scope && scope->type == ScopeType::eFunction) {
        const Function *func = scope->function;
        if (func) {
            const Token *ret = func->retDef;
            while (Token::Match(ret, "static|const"))
                ret = ret->next();
            return Token::Match(ret, "bool|_Bool");
        }
    }
    return false;
}

static bool isOperandExpanded(const Token *tok)
{
    if (tok->isExpandedMacro() || tok->isEnumerator())
        return true;
    if (tok->astOperand1() && isOperandExpanded(tok->astOperand1()))
        return true;
    if (tok->astOperand2() && isOperandExpanded(tok->astOperand2()))
        return true;
    return false;
}

void CheckCondition::checkBadBitmaskCheck()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("badBitmaskCheck"))
        return;

    logChecker("CheckCondition::checkBadBitmaskCheck"); // style

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() == "|" && tok->astOperand1() && tok->astOperand2() && tok->astParent()) {
            const Token* parent = tok->astParent();
            const bool isBoolean = Token::Match(parent, "&&|%oror%") ||
                                   (parent->str() == "?" && parent->astOperand1() == tok) ||
                                   (parent->str() == "=" && parent->astOperand2() == tok && parent->astOperand1() && parent->astOperand1()->variable() && Token::Match(parent->astOperand1()->variable()->typeStartToken(), "bool|_Bool")) ||
                                   (parent->str() == "(" && Token::Match(parent->astOperand1(), "if|while")) ||
                                   (parent->str() == "return" && parent->astOperand1() == tok && inBooleanFunction(tok));

            const bool isTrue = (tok->astOperand1()->hasKnownIntValue() && tok->astOperand1()->getKnownIntValue() != 0) ||
                                (tok->astOperand2()->hasKnownIntValue() && tok->astOperand2()->getKnownIntValue() != 0);

            if (isBoolean && isTrue)
                badBitmaskCheckError(tok);

            // If there are #ifdef in the expression don't warn about redundant | to avoid FP
            const auto& startStop = tok->findExpressionStartEndTokens();
            if (mTokenizer->hasIfdef(startStop.first, startStop.second))
                continue;

            const bool isZero1 = (tok->astOperand1()->hasKnownIntValue() && tok->astOperand1()->getKnownIntValue() == 0);
            const bool isZero2 = (tok->astOperand2()->hasKnownIntValue() && tok->astOperand2()->getKnownIntValue() == 0);
            if (!isZero1 && !isZero2)
                continue;

            if (!tok->isExpandedMacro() &&
                !(isZero1 && isOperandExpanded(tok->astOperand1())) &&
                !(isZero2 && isOperandExpanded(tok->astOperand2())))
                badBitmaskCheckError(tok, /*isNoOp*/ true);
        }
    }
}

void CheckCondition::badBitmaskCheckError(const Token *tok, bool isNoOp)
{
    if (isNoOp)
        reportError(tok, Severity::style, "badBitmaskCheck", "Operator '|' with one operand equal to zero is redundant.", CWE571, Certainty::normal);
    else
        reportError(tok, Severity::warning, "badBitmaskCheck", "Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?", CWE571, Certainty::normal);
}

void CheckCondition::comparison()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("comparisonError"))
        return;

    logChecker("CheckCondition::comparison"); // style

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isComparisonOp())
            continue;

        const Token *expr1 = tok->astOperand1();
        const Token *expr2 = tok->astOperand2();
        if (!expr1 || !expr2)
            continue;
        if (expr1->hasKnownIntValue())
            std::swap(expr1,expr2);
        if (!expr2->hasKnownIntValue())
            continue;
        if (!compareTokenFlags(expr1, expr2, /*macro*/ true))
            continue;
        const MathLib::bigint num2 = expr2->getKnownIntValue();
        if (num2 < 0)
            continue;
        if (!Token::Match(expr1,"[&|]"))
            continue;
        std::list<MathLib::bigint> numbers;
        getnumchildren(expr1, numbers);
        for (const MathLib::bigint num1 : numbers) {
            if (num1 < 0)
                continue;
            if (Token::Match(tok, "==|!=")) {
                if ((expr1->str() == "&" && (num1 & num2) != num2) ||
                    (expr1->str() == "|" && (num1 | num2) != num2)) {
                    const std::string& op(tok->str());
                    comparisonError(expr1, expr1->str(), num1, op, num2, op != "==");
                }
            } else if (expr1->str() == "&") {
                const bool or_equal = Token::Match(tok, ">=|<=");
                const std::string& op(tok->str());
                if ((Token::Match(tok, ">=|<")) && (num1 < num2)) {
                    comparisonError(expr1, expr1->str(), num1, op, num2, !or_equal);
                } else if ((Token::Match(tok, "<=|>")) && (num1 <= num2)) {
                    comparisonError(expr1, expr1->str(), num1, op, num2, or_equal);
                }
            } else if (expr1->str() == "|") {
                if ((expr1->astOperand1()->valueType()) &&
                    (expr1->astOperand1()->valueType()->sign == ValueType::Sign::UNSIGNED)) {
                    const bool or_equal = Token::Match(tok, ">=|<=");
                    const std::string& op(tok->str());
                    if ((Token::Match(tok, ">=|<")) && (num1 >= num2)) {
                        //"(a | 0x07) >= 7U" is always true for unsigned a
                        //"(a | 0x07) < 7U" is always false for unsigned a
                        comparisonError(expr1, expr1->str(), num1, op, num2, or_equal);
                    } else if ((Token::Match(tok, "<=|>")) && (num1 > num2)) {
                        //"(a | 0x08) <= 7U" is always false for unsigned a
                        //"(a | 0x07) > 6U" is always true for unsigned a
                        comparisonError(expr1, expr1->str(), num1, op, num2, !or_equal);
                    }
                }
            }
        }
    }
}

void CheckCondition::comparisonError(const Token *tok, const std::string &bitop, MathLib::bigint value1, const std::string &op, MathLib::bigint value2, bool result)
{
    std::ostringstream expression;
    expression << std::hex << "(X " << bitop << " 0x" << value1 << ") " << op << " 0x" << value2;

    const std::string errmsg("Expression '" + expression.str() + "' is always " + bool_to_string(result) + ".\n"
                             "The expression '" + expression.str() + "' is always " + bool_to_string(result) +
                             ". Check carefully constants and operators used, these errors might be hard to "
                             "spot sometimes. In case of complex expression it might help to split it to "
                             "separate expressions.");

    reportError(tok, Severity::style, "comparisonError", errmsg, CWE398, Certainty::normal);
}

bool CheckCondition::isOverlappingCond(const Token * const cond1, const Token * const cond2, bool pure) const
{
    if (!cond1 || !cond2)
        return false;

    // same expressions
    if (isSameExpression(true, cond1, cond2, *mSettings, pure, false))
        return true;

    // bitwise overlap for example 'x&7' and 'x==1'
    if (cond1->str() == "&" && cond1->astOperand1() && cond2->astOperand2()) {
        const Token *expr1 = cond1->astOperand1();
        const Token *num1  = cond1->astOperand2();
        if (!num1) // unary operator&
            return false;
        if (!num1->isNumber())
            std::swap(expr1,num1);
        if (!num1->isNumber() || MathLib::isNegative(num1->str()))
            return false;

        if (!Token::Match(cond2, "&|==") || !cond2->astOperand1() || !cond2->astOperand2())
            return false;
        const Token *expr2 = cond2->astOperand1();
        const Token *num2  = cond2->astOperand2();
        if (!num2->isNumber())
            std::swap(expr2,num2);
        if (!num2->isNumber() || MathLib::isNegative(num2->str()))
            return false;

        if (!isSameExpression(true, expr1, expr2, *mSettings, pure, false))
            return false;

        const MathLib::bigint value1 = MathLib::toBigNumber(num1);
        const MathLib::bigint value2 = MathLib::toBigNumber(num2);
        if (cond2->str() == "&")
            return ((value1 & value2) == value2);
        return ((value1 & value2) > 0);
    }
    return false;
}

void CheckCondition::duplicateCondition()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("duplicateCondition"))
        return;

    logChecker("CheckCondition::duplicateCondition"); // style

    const SymbolDatabase *const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eIf)
            continue;

        const Token* tok2 = scope.classDef->next();
        if (!tok2)
            continue;
        const Token* cond1 = tok2->astOperand2();
        if (!cond1)
            continue;
        if (cond1->hasKnownIntValue())
            continue;

        tok2 = tok2->link();
        if (!Token::simpleMatch(tok2, ") {"))
            continue;
        tok2 = tok2->linkAt(1);
        if (!Token::simpleMatch(tok2, "} if ("))
            continue;
        const Token *cond2 = tok2->tokAt(2)->astOperand2();
        if (!cond2)
            continue;

        ErrorPath errorPath;
        if (!findExpressionChanged(cond1, scope.classDef->next(), cond2, *mSettings) &&
            isSameExpression(true, cond1, cond2, *mSettings, true, true, &errorPath))
            duplicateConditionError(cond1, cond2, std::move(errorPath));
    }
}

void CheckCondition::duplicateConditionError(const Token *tok1, const Token *tok2, ErrorPath errorPath)
{
    if (diag(tok1, tok2))
        return;
    errorPath.emplace_back(tok1, "First condition");
    errorPath.emplace_back(tok2, "Second condition");

    std::string msg = "The if condition is the same as the previous if condition";

    reportError(std::move(errorPath), Severity::style, "duplicateCondition", msg, CWE398, Certainty::normal);
}

void CheckCondition::multiCondition()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("multiCondition"))
        return;

    logChecker("CheckCondition::multiCondition"); // style

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eIf)
            continue;

        const Token * const cond1 = scope.classDef->next()->astOperand2();
        if (!cond1)
            continue;

        const Token * tok2 = scope.classDef->next();

        // Check each 'else if'
        for (;;) {
            tok2 = tok2->link();
            if (!Token::simpleMatch(tok2, ") {"))
                break;
            tok2 = tok2->linkAt(1);
            if (!Token::simpleMatch(tok2, "} else { if ("))
                break;
            tok2 = tok2->tokAt(4);

            if (tok2->astOperand2()) {
                ErrorPath errorPath;
                if (isOverlappingCond(cond1, tok2->astOperand2(), true) &&
                    !findExpressionChanged(cond1, cond1, tok2->astOperand2(), *mSettings))
                    overlappingElseIfConditionError(tok2->astOperand2(), cond1->linenr());
                else if (isOppositeCond(
                             true, cond1, tok2->astOperand2(), *mSettings, true, true, &errorPath) &&
                         !findExpressionChanged(cond1, cond1, tok2->astOperand2(), *mSettings))
                    oppositeElseIfConditionError(cond1, tok2->astOperand2(), std::move(errorPath));
            }
        }
    }
}

void CheckCondition::overlappingElseIfConditionError(const Token *tok, nonneg int line1)
{
    if (diag(tok))
        return;
    std::ostringstream errmsg;
    errmsg << "Expression is always false because 'else if' condition matches previous condition at line "
           << line1 << ".";

    reportError(tok, Severity::style, "multiCondition", errmsg.str(), CWE398, Certainty::normal);
}

void CheckCondition::oppositeElseIfConditionError(const Token *ifCond, const Token *elseIfCond, ErrorPath errorPath)
{
    if (diag(ifCond, elseIfCond))
        return;
    std::ostringstream errmsg;
    errmsg << "Expression is always true because 'else if' condition is opposite to previous condition at line "
           << ifCond->linenr() << ".";

    errorPath.emplace_back(ifCond, "first condition");
    errorPath.emplace_back(elseIfCond, "else if condition is opposite to first condition");

    reportError(std::move(errorPath), Severity::style, "multiCondition", errmsg.str(), CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// - Opposite inner conditions => always false
// - Same/Overlapping inner condition => always true
// - same condition after early exit => always false
//---------------------------------------------------------------------------

static bool isNonConstFunctionCall(const Token *ftok, const Library &library)
{
    if (library.isFunctionConst(ftok))
        return false;
    const Token *obj = ftok->next()->astOperand1();
    while (obj && obj->str() == ".")
        obj = obj->astOperand1();
    if (!obj)
        return true;
    if (obj->variable() && obj->variable()->isConst())
        return false;
    if (ftok->function() && ftok->function()->isConst())
        return false;
    if (ftok->isControlFlowKeyword())
        return false;
    return true;
}

static bool isNestedInLambda(const Scope* inner, const Scope* outer)
{
    while (inner && inner != outer) {
        if (inner->type == ScopeType::eLambda)
            return true;
        inner = inner->nestedIn;
    }
    return false;
}

void CheckCondition::multiCondition2()
{
    if (!mSettings->severity.isEnabled(Severity::warning) &&
        !mSettings->isPremiumEnabled("identicalConditionAfterEarlyExit") &&
        !mSettings->isPremiumEnabled("identicalInnerCondition"))
        return;

    logChecker("CheckCondition::multiCondition2"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        const Token *condTok = nullptr;
        if (scope.type == ScopeType::eIf || scope.type == ScopeType::eWhile)
            condTok = scope.classDef->next()->astOperand2();
        else if (scope.type == ScopeType::eFor) {
            condTok = scope.classDef->next()->astOperand2();
            if (!condTok || condTok->str() != ";")
                continue;
            condTok = condTok->astOperand2();
            if (!condTok || condTok->str() != ";")
                continue;
            condTok = condTok->astOperand1();
        }
        if (!condTok)
            continue;
        const Token * const cond1 = condTok;

        if (!Token::simpleMatch(scope.classDef->linkAt(1), ") {"))
            continue;

        bool functionCall = false;
        bool nonConstFunctionCall = false;
        bool nonlocal = false; // nonlocal variable used in condition
        std::set<int> vars; // variables used in condition
        visitAstNodes(condTok,
                      [&](const Token *cond) {
            if (Token::Match(cond, "%name% (")) {
                functionCall = true;
                nonConstFunctionCall = isNonConstFunctionCall(cond, mSettings->library);
                if (nonConstFunctionCall)
                    return ChildrenToVisit::done;
            }

            if (cond->varId()) {
                vars.insert(cond->varId());
                const Variable *var = cond->variable();
                if (!nonlocal && var) {
                    if (!(var->isLocal() || var->isArgument()))
                        nonlocal = true;
                    else if ((var->isPointer() || var->isReference()) && !Token::Match(cond->astParent(), "%oror%|&&|!"))
                        // TODO: if var is pointer check what it points at
                        nonlocal = true;
                }
            } else if (!nonlocal && cond->isName()) {
                // varid is 0. this is possibly a nonlocal variable..
                nonlocal = Token::Match(cond->astParent(), "%cop%|(|[") || Token::Match(cond, "%name% .") || (cond->isCpp() && cond->str() == "this");
            } else {
                return ChildrenToVisit::op1_and_op2;
            }
            return ChildrenToVisit::none;
        });

        if (nonConstFunctionCall)
            continue;

        std::vector<const Variable*> varsInCond;
        visitAstNodes(condTok,
                      [&varsInCond](const Token *cond) {
            if (cond->variable()) {
                const Variable *var = cond->variable();
                if (std::find(varsInCond.cbegin(), varsInCond.cend(), var) == varsInCond.cend())
                    varsInCond.push_back(var);
            }
            return ChildrenToVisit::op1_and_op2;
        });

        // parse until second condition is reached..
        enum MULTICONDITIONTYPE : std::uint8_t { INNER, AFTER };
        const Token *tok;

        // Parse inner condition first and then early return condition
        std::vector<MULTICONDITIONTYPE> types = {MULTICONDITIONTYPE::INNER};
        if (Token::Match(scope.bodyStart, "{ return|throw|continue|break"))
            types.push_back(MULTICONDITIONTYPE::AFTER);
        for (const MULTICONDITIONTYPE type:types) {
            if (type == MULTICONDITIONTYPE::AFTER) {
                tok = scope.bodyEnd->next();
            } else {
                tok = scope.bodyStart;
            }
            const Token * const endToken = tok->scope()->bodyEnd;

            for (; tok && tok != endToken; tok = tok->next()) {
                if (isNestedInLambda(tok->scope(), cond1->scope()))
                    continue;
                if (isExpressionChangedAt(cond1, tok, 0, false, *mSettings))
                    break;
                if (Token::Match(tok, "if|return")) {
                    const Token * condStartToken = tok->str() == "if" ? tok->next() : tok;
                    const Token * condEndToken = tok->str() == "if" ? condStartToken->link() : Token::findsimplematch(condStartToken, ";");
                    // Does condition modify tracked variables?
                    if (findExpressionChanged(cond1, condStartToken, condEndToken, *mSettings))
                        break;

                    // Condition..
                    const Token *cond2 = tok->str() == "if" ? condStartToken->astOperand2() : condStartToken->astOperand1();
                    const bool isReturnVar = (tok->str() == "return" && (!Token::Match(cond2, "%cop%") || (cond2 && cond2->isUnaryOp("!"))));

                    ErrorPath errorPath;

                    if (type == MULTICONDITIONTYPE::INNER) {
                        visitAstNodes(cond1, [&](const Token* firstCondition) {
                            if (!firstCondition)
                                return ChildrenToVisit::none;
                            if (firstCondition->str() == "&&") {
                                if (!isOppositeCond(false, firstCondition, cond2, *mSettings, true, true))
                                    return ChildrenToVisit::op1_and_op2;
                            }
                            if (!firstCondition->hasKnownIntValue()) {
                                if (!isReturnVar && isOppositeCond(false, firstCondition, cond2, *mSettings, true, true, &errorPath)) {
                                    if (!isAliased(vars))
                                        oppositeInnerConditionError(firstCondition, cond2, errorPath);
                                } else if (!isReturnVar && isSameExpression(true, firstCondition, cond2, *mSettings, true, true, &errorPath)) {
                                    identicalInnerConditionError(firstCondition, cond2, errorPath);
                                } else if (!isReturnVar && isOverlappingCond(cond2, firstCondition, true)) {
                                    overlappingInnerConditionError(firstCondition, cond2, errorPath);
                                }
                            }
                            return ChildrenToVisit::none;
                        });
                    } else {
                        visitAstNodes(cond2, [&](const Token *secondCondition) {
                            if (secondCondition->str() == "||" || secondCondition->str() == "&&")
                                return ChildrenToVisit::op1_and_op2;

                            if ((!cond1->hasKnownIntValue() || !secondCondition->hasKnownIntValue()) &&
                                isSameExpression(true, cond1, secondCondition, *mSettings, true, true, &errorPath)) {
                                if (!isAliased(vars) && !mTokenizer->hasIfdef(cond1, secondCondition)) {
                                    identicalConditionAfterEarlyExitError(cond1, secondCondition, errorPath);
                                    return ChildrenToVisit::done;
                                }
                            }
                            return ChildrenToVisit::none;
                        });
                    }
                }
                if (Token::Match(tok, "%name% (") &&
                    isVariablesChanged(tok, tok->linkAt(1), 0, varsInCond, *mSettings)) {
                    break;
                }
                if (Token::Match(tok, "%type% (") && nonlocal && isNonConstFunctionCall(tok, mSettings->library)) // non const function call -> bailout if there are nonlocal variables
                    break;
                if (Token::Match(tok, "case|break|continue|return|throw") && tok->scope() == endToken->scope())
                    break;
                if (Token::Match(tok, "[;{}] %name% :"))
                    break;
                // bailout if loop is seen.
                // TODO: handle loops better.
                if (Token::Match(tok, "for|while|do")) {
                    const Token *tok1 = tok->next();
                    const Token *tok2;
                    if (Token::simpleMatch(tok, "do {")) {
                        if (!Token::simpleMatch(tok->linkAt(1), "} while ("))
                            break;
                        tok2 = tok->linkAt(1)->linkAt(2);
                    } else if (Token::Match(tok, "if|while (")) {
                        tok2 = tok->linkAt(1);
                        if (Token::simpleMatch(tok2, ") {"))
                            tok2 = tok2->linkAt(1);
                        if (!tok2)
                            break;
                    } else {
                        // Incomplete code
                        break;
                    }
                    const bool changed = std::any_of(vars.cbegin(), vars.cend(), [&](int varid) {
                        return isVariableChanged(tok1, tok2, varid, nonlocal, *mSettings);
                    });
                    if (changed)
                        break;
                }
                if ((tok->varId() && vars.find(tok->varId()) != vars.end()) ||
                    (!tok->varId() && nonlocal) ||
                    (functionCall && tok->variable() && !tok->variable()->isLocal())) {
                    if (Token::Match(tok, "%name% %assign%|++|--"))
                        break;
                    if (Token::Match(tok->astParent(), "*|.|[")) {
                        const Token *parent = tok;
                        while (Token::Match(parent->astParent(), ".|[") || (parent->astParent() && parent->astParent()->isUnaryOp("*")))
                            parent = parent->astParent();
                        if (Token::Match(parent->astParent(), "%assign%|++|--"))
                            break;
                    }
                    if (tok->isCpp() && Token::Match(tok, "%name% <<") && (!tok->valueType() || !tok->valueType()->isIntegral()))
                        break;
                    if (isLikelyStreamRead(tok->next()) || isLikelyStreamRead(tok->previous()))
                        break;
                    if (Token::Match(tok, "%name% [")) {
                        const Token *tok2 = tok->linkAt(1);
                        while (Token::simpleMatch(tok2, "] ["))
                            tok2 = tok2->linkAt(1);
                        if (Token::Match(tok2, "] %assign%|++|--"))
                            break;
                    }
                    if (Token::Match(tok->previous(), "++|--|& %name%"))
                        break;
                    if (tok->variable() &&
                        !tok->variable()->isConst() &&
                        Token::Match(tok, "%name% . %name% (")) {
                        const Function* function = tok->tokAt(2)->function();
                        if (!function || !function->isConst())
                            break;
                    }
                    if (Token::Match(tok->previous(), "[(,] *|& %name% [,)]") && isParameterChanged(tok))
                        break;
                }
            }
        }
    }
}

static std::string innerSmtString(const Token * tok)
{
    if (!tok)
        return "if";
    const Token * top = tok->astTop();
    if (top->str() == "(" && top->astOperand1())
        return top->astOperand1()->str();
    return top->str();
}

void CheckCondition::oppositeInnerConditionError(const Token *tok1, const Token* tok2, ErrorPath errorPath)
{
    if (diag(tok1, tok2))
        return;
    const std::string s1(tok1 ? tok1->expressionString() : "x");
    const std::string s2(tok2 ? tok2->expressionString() : "!x");
    const std::string innerSmt = innerSmtString(tok2);
    errorPath.emplace_back(tok1, "outer condition: " + s1);
    errorPath.emplace_back(tok2, "opposite inner condition: " + s2);

    const std::string msg("Opposite inner '" + innerSmt + "' condition leads to a dead code block.\n"
                          "Opposite inner '" + innerSmt + "' condition leads to a dead code block (outer condition is '" + s1 + "' and inner condition is '" + s2 + "').");
    reportError(std::move(errorPath), Severity::warning, "oppositeInnerCondition", msg, CWE398, Certainty::normal);
}

void CheckCondition::overlappingInnerConditionError(const Token *tok1,  const Token* tok2, ErrorPath errorPath)
{
    if (diag(tok1, tok2))
        return;
    const std::string s1(tok1 ? tok1->expressionString() : "x");
    const std::string s2(tok2 ? tok2->expressionString() : "x");
    const std::string innerSmt = innerSmtString(tok2);
    errorPath.emplace_back(tok1, "outer condition: " + s1);
    errorPath.emplace_back(tok2, "overlapping inner condition: " + s2);

    const std::string msg("Overlapping inner '" + innerSmt + "' condition is always true.\n"
                          "Overlapping inner '" + innerSmt + "' condition is always true (outer condition is '" + s1 + "' and inner condition is '" + s2 + "').");
    reportError(std::move(errorPath), Severity::warning, "overlappingInnerCondition", msg, CWE398, Certainty::normal);
}

void CheckCondition::identicalInnerConditionError(const Token *tok1, const Token* tok2, ErrorPath errorPath)
{
    if (diag(tok1, tok2))
        return;
    const std::string s1(tok1 ? tok1->expressionString() : "x");
    const std::string s2(tok2 ? tok2->expressionString() : "x");
    const std::string innerSmt = innerSmtString(tok2);
    errorPath.emplace_back(tok1, "outer condition: " + s1);
    errorPath.emplace_back(tok2, "identical inner condition: " + s2);

    const std::string msg("Identical inner '" + innerSmt + "' condition is always true.\n"
                          "Identical inner '" + innerSmt + "' condition is always true (outer condition is '" + s1 + "' and inner condition is '" + s2 + "').");
    reportError(std::move(errorPath), Severity::warning, "identicalInnerCondition", msg, CWE398, Certainty::normal);
}

void CheckCondition::identicalConditionAfterEarlyExitError(const Token *cond1, const Token* cond2, ErrorPath errorPath)
{
    if (diag(cond1, cond2))
        return;

    const bool isReturnValue = cond2 && Token::simpleMatch(cond2->astParent(), "return");

    const std::string cond(cond1 ? cond1->expressionString() : "x");
    const std::string value = (cond2 && cond2->valueType() && cond2->valueType()->type == ValueType::Type::BOOL) ? "false" : "0";

    errorPath.emplace_back(cond1, "If condition '" + cond + "' is true, the function will return/exit");
    errorPath.emplace_back(cond2, (isReturnValue ? "Returning identical expression '" : "Testing identical condition '") + cond + "'");

    reportError(std::move(errorPath),
                Severity::warning,
                "identicalConditionAfterEarlyExit",
                isReturnValue
                ? ("Identical condition and return expression '" + cond + "', return value is always " + value)
                : ("Identical condition '" + cond + "', second condition is always false"),
                CWE398,
                Certainty::normal);
}

//---------------------------------------------------------------------------
//    if ((x != 1) || (x != 3))            // expression always true
//    if ((x == 1) && (x == 3))            // expression always false
//    if ((x < 1)  && (x > 3))             // expression always false
//    if ((x > 3)  || (x < 10))            // expression always true
//    if ((x > 5)  && (x != 1))            // second comparison always true
//
//    Check for suspect logic for an expression consisting of 2 comparison
//    expressions with a shared variable and constants and a logical operator
//    between them.
//
//    Suggest a different logical operator when the logical operator between
//    the comparisons is probably wrong.
//
//    Inform that second comparison is always true when first comparison is true.
//---------------------------------------------------------------------------

static std::string invertOperatorForOperandSwap(std::string s)
{
    if (s[0] == '<')
        s[0] = '>';
    else if (s[0] == '>')
        s[0] = '<';
    return s;
}

template<typename T>
static int sign(const T v) {
    return static_cast<int>(v > 0) - static_cast<int>(v < 0);
}

// returns 1 (-1) if the first (second) condition is sufficient, 0 if indeterminate
template<typename T>
static int sufficientCondition(std::string op1, const bool not1, const T value1, std::string op2, const bool not2, const T value2, const bool isAnd) {
    auto transformOp = [](std::string& op, const bool invert) {
        if (invert) {
            if (op == "==")
                op = "!=";
            else if (op == "!=")
                op = "==";
            else if (op == "<")
                op = ">=";
            else if (op == ">")
                op = "<=";
            else if (op == "<=")
                op = ">";
            else if (op == ">=")
                op = "<";
        }
    };
    transformOp(op1, not1);
    transformOp(op2, not2);
    int res = 0;
    bool equal = false;
    if (op1 == op2) {
        equal = true;
        if (op1 == ">" || op1 == ">=")
            res = sign(value1 - value2);
        else if (op1 == "<" || op1 == "<=")
            res = -sign(value1 - value2);
    } else { // not equal
        if (op1 == "!=")
            res = 1;
        else if (op2 == "!=")
            res = -1;
        else if (op1 == "==")
            res = -1;
        else if (op2 == "==")
            res = 1;
        else if (op1 == ">" && op2 == ">=")
            res = sign(value1 - (value2 - 1));
        else if (op1 == ">=" && op2 == ">")
            res = sign((value1 - 1) - value2);
        else if (op1 == "<" && op2 == "<=")
            res = -sign(value1 - (value2 + 1));
        else if (op1 == "<=" && op2 == "<")
            res = -sign((value1 + 1) - value2);
    }
    return res * (isAnd == equal ? 1 : -1);
}

template<typename T>
static bool checkIntRelation(const std::string &op, const T value1, const T value2)
{
    return (op == "==" && value1 == value2) ||
           (op == "!=" && value1 != value2) ||
           (op == ">" && value1 >  value2) ||
           (op == ">=" && value1 >= value2) ||
           (op == "<" && value1 <  value2) ||
           (op == "<=" && value1 <= value2);
}

static bool checkFloatRelation(const std::string &op, const double value1, const double value2)
{
    return (op == ">" && value1 >  value2) ||
           (op == ">=" && value1 >= value2) ||
           (op == "<" && value1 <  value2) ||
           (op == "<=" && value1 <= value2);
}

template<class T>
static T getvalue3(const T value1, const T value2)
{
    const T min = std::min(value1, value2);
    if (min== std::numeric_limits<T>::max())
        return min;
    return min + 1; // see #5895
}

template<>
double getvalue3(const double value1, const double value2)
{
    return (value1 + value2) / 2.0;
}


template<class T>
static inline T getvalue(const int test, const T value1, const T value2)
{
    // test:
    // 1 => return value that is less than both value1 and value2
    // 2 => return value1
    // 3 => return value that is between value1 and value2
    // 4 => return value2
    // 5 => return value that is larger than both value1 and value2
    switch (test) {
    case 1:
        return std::numeric_limits<T>::lowest();
    case 2:
        return value1;
    case 3:
        return getvalue3<T>(value1, value2);
    case 4:
        return value2;
    case 5:
        return std::numeric_limits<T>::max();
    }
    return 0;
}

static bool parseComparison(const Token *comp, bool &not1, std::string &op, std::string &value, const Token *&expr, bool &inconclusive)
{
    not1 = false;
    while (comp && comp->str() == "!") {
        not1 = !not1;
        comp = comp->astOperand1();
    }

    if (!comp)
        return false;

    const Token* op1 = comp->astOperand1();
    const Token* op2 = comp->astOperand2();
    if (!comp->isComparisonOp() || !op1 || !op2) {
        op = "!=";
        value = "0";
        expr = comp;
    } else if (op1->isLiteral()) {
        if (op1->isExpandedMacro())
            return false;
        op = invertOperatorForOperandSwap(comp->str());
        if (op1->enumerator() && op1->enumerator()->value_known)
            value = MathLib::toString(op1->enumerator()->value);
        else
            value = op1->str();
        expr = op2;
    } else if (comp->astOperand2()->isLiteral()) {
        if (op2->isExpandedMacro())
            return false;
        op = comp->str();
        if (op2->enumerator() && op2->enumerator()->value_known)
            value = MathLib::toString(op2->enumerator()->value);
        else
            value = op2->str();
        expr = op1;
    } else {
        op = "!=";
        value = "0";
        expr = comp;
    }

    inconclusive = inconclusive || (value[0] == '\'' && !(op == "!=" || op == "=="));

    // Only float and int values are currently handled
    return MathLib::isInt(value) || MathLib::isFloat(value) || (value[0] == '\'');
}

static std::string conditionString(bool not1, const Token *expr1, const std::string &op, const std::string &value1)
{
    if (expr1->astParent()->isComparisonOp())
        return std::string(not1 ? "!(" : "") + expr1->expressionString() +
               " " +
               op +
               " " +
               value1 +
               (not1 ? ")" : "");

    return std::string(not1 ? "!" : "") + expr1->expressionString();
}

static std::string conditionString(const Token * tok)
{
    if (!tok)
        return "";
    if (tok->isComparisonOp()) {
        bool inconclusive = false;
        bool not_;
        std::string op, value;
        const Token *expr;
        if (parseComparison(tok, not_, op, value, expr, inconclusive) && expr->isName()) {
            return conditionString(not_, expr, op, value);
        }
    }
    if (Token::Match(tok, "%cop%|&&|%oror%")) {
        if (tok->astOperand2())
            return conditionString(tok->astOperand1()) + " " + tok->str() + " " + conditionString(tok->astOperand2());
        return tok->str() + "(" + conditionString(tok->astOperand1()) + ")";

    }
    return tok->expressionString();
}

static bool isIfConstexpr(const Token* tok) {
    const Token* const top = tok->astTop();
    return Token::simpleMatch(top->astOperand1(), "if") && top->astOperand1()->isConstexpr();
}

void CheckCondition::checkIncorrectLogicOperator()
{
    const bool printStyle = mSettings->severity.isEnabled(Severity::style);
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    if (!printWarning && !printStyle && !mSettings->isPremiumEnabled("incorrectLogicOperator"))
        return;
    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);

    logChecker("CheckCondition::checkIncorrectLogicOperator"); // style,warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {

        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%oror%|&&") || !tok->astOperand1() || !tok->astOperand2())
                continue;


            // 'A && (!A || B)' is equivalent to 'A && B'
            // 'A || (!A && B)' is equivalent to 'A || B'
            // 'A && (A || B)' is equivalent to 'A'
            // 'A || (A && B)' is equivalent to 'A'
            if (printStyle &&
                ((tok->str() == "||" && tok->astOperand2()->str() == "&&") ||
                 (tok->str() == "&&" && tok->astOperand2()->str() == "||"))) {
                const Token* tok2 = tok->astOperand2()->astOperand1();
                if (isOppositeCond(true, tok->astOperand1(), tok2, *mSettings, true, false)) {
                    std::string expr1(tok->astOperand1()->expressionString());
                    std::string expr2(tok->astOperand2()->astOperand1()->expressionString());
                    std::string expr3(tok->astOperand2()->astOperand2()->expressionString());
                    // make copy for later because the original string might get overwritten
                    const std::string expr1VerboseMsg = expr1;
                    const std::string expr2VerboseMsg = expr2;
                    const std::string expr3VerboseMsg = expr3;

                    if (expr1.length() + expr2.length() + expr3.length() > 50U) {
                        if (expr1[0] == '!' && expr2[0] != '!') {
                            expr1 = "!A";
                            expr2 = "A";
                        } else {
                            expr1 = "A";
                            expr2 = "!A";
                        }

                        expr3 = "B";
                    }

                    const std::string cond1 = expr1 + " " + tok->str() + " (" + expr2 + " " + tok->astOperand2()->str() + " " + expr3 + ")";
                    const std::string cond2 = expr1 + " " + tok->str() + " " + expr3;

                    const std::string cond1VerboseMsg = expr1VerboseMsg + " " + tok->str() + " " + expr2VerboseMsg + " " + tok->astOperand2()->str() + " " + expr3VerboseMsg;
                    const std::string cond2VerboseMsg = expr1VerboseMsg + " " + tok->str() + " " + expr3VerboseMsg;
                    // for the --verbose message, transform the actual condition and print it
                    const std::string msg = tok2->expressionString() + ". '" + cond1 + "' is equivalent to '" + cond2 + "'\n"
                                            "The condition '" + cond1VerboseMsg + "' is equivalent to '" + cond2VerboseMsg + "'.";
                    redundantConditionError(tok, msg, false);
                    continue;
                }
                if (isSameExpression(false, tok->astOperand1(), tok2, *mSettings, true, true)) {
                    std::string expr1(tok->astOperand1()->expressionString());
                    std::string expr2(tok->astOperand2()->astOperand1()->expressionString());
                    std::string expr3(tok->astOperand2()->astOperand2()->expressionString());
                    // make copy for later because the original string might get overwritten
                    const std::string expr1VerboseMsg = expr1;
                    const std::string expr2VerboseMsg = expr2;
                    const std::string expr3VerboseMsg = expr3;

                    if (expr1.length() + expr2.length() + expr3.length() > 50U) {
                        expr1 = "A";
                        expr2 = "A";
                        expr3 = "B";
                    }

                    const std::string cond1 = expr1 + " " + tok->str() + " (" + expr2 + " " + tok->astOperand2()->str() + " " + expr3 + ")";
                    const std::string cond2 = std::move(expr1);

                    const std::string cond1VerboseMsg = expr1VerboseMsg + " " + tok->str() + " " + expr2VerboseMsg + " " + tok->astOperand2()->str() + " " + expr3VerboseMsg;
                    const std::string& cond2VerboseMsg = expr1VerboseMsg;
                    // for the --verbose message, transform the actual condition and print it
                    const std::string msg = tok2->expressionString() + ". '" + cond1 + "' is equivalent to '" + cond2 + "'\n"
                                            "The condition '" + cond1VerboseMsg + "' is equivalent to '" + cond2VerboseMsg + "'.";
                    redundantConditionError(tok, msg, false);
                    continue;
                }
            }

            // Comparison #1 (LHS)
            const Token *comp1 = tok->astOperand1();
            if (comp1->str() == tok->str())
                comp1 = comp1->astOperand2();

            // Comparison #2 (RHS)
            const Token *comp2 = tok->astOperand2();

            bool inconclusive = false;
            bool parseable = true;

            // Parse LHS
            bool not1;
            std::string op1, value1;
            const Token *expr1 = nullptr;
            parseable &= (parseComparison(comp1, not1, op1, value1, expr1, inconclusive));

            // Parse RHS
            bool not2;
            std::string op2, value2;
            const Token *expr2 = nullptr;
            parseable &= (parseComparison(comp2, not2, op2, value2, expr2, inconclusive));

            if (inconclusive && !printInconclusive)
                continue;

            const bool isUnknown = (expr1 && expr1->valueType() && expr1->valueType()->type == ValueType::UNKNOWN_TYPE) ||
                                   (expr2 && expr2->valueType() && expr2->valueType()->type == ValueType::UNKNOWN_TYPE);
            if (isUnknown)
                continue;

            const bool isfloat = astIsFloat(expr1, true) || MathLib::isFloat(value1) || astIsFloat(expr2, true) || MathLib::isFloat(value2);

            ErrorPath errorPath;

            // Opposite comparisons around || or && => always true or always false
            const bool isLogicalOr(tok->str() == "||");
            if (!isfloat && isOppositeCond(isLogicalOr, tok->astOperand1(), tok->astOperand2(), *mSettings, true, true, &errorPath)) {
                if (!isIfConstexpr(tok)) {
                    const bool alwaysTrue(isLogicalOr);
                    incorrectLogicOperatorError(tok, conditionString(tok), alwaysTrue, inconclusive, std::move(errorPath));
                }
                continue;
            }

            if (!parseable)
                continue;

            if (isSameExpression(true, comp1, comp2, *mSettings, true, true))
                continue; // same expressions => only report that there are same expressions
            if (!isSameExpression(true, expr1, expr2, *mSettings, true, true))
                continue;


            // don't check floating point equality comparisons. that is bad
            // and deserves different warnings.
            if (isfloat && (op1 == "==" || op1 == "!=" || op2 == "==" || op2 == "!="))
                continue;

            // the expr are not the token of the value but they provide better context
            const double d1 = isfloat ? MathLib::toDoubleNumber(value1) : 0;
            const double d2 = isfloat ? MathLib::toDoubleNumber(value2) : 0;
            const MathLib::bigint i1 = isfloat ? 0 : MathLib::toBigNumber(value1, expr1);
            const MathLib::bigint i2 = isfloat ? 0 : MathLib::toBigNumber(value2, expr2);
            const bool useUnsignedInt = (std::numeric_limits<MathLib::bigint>::max()==i1) || (std::numeric_limits<MathLib::bigint>::max()==i2);
            const MathLib::biguint u1 = useUnsignedInt ? MathLib::toBigUNumber(value1, expr1) : 0;
            const MathLib::biguint u2 = useUnsignedInt ? MathLib::toBigUNumber(value2, expr2) : 0;
            // evaluate if expression is always true/false
            bool alwaysTrue = true, alwaysFalse = true;
            bool firstTrue = true, secondTrue = true;
            const bool isAnd = tok->str() == "&&";
            for (int test = 1; test <= 5; ++test) {
                // test:
                // 1 => testvalue is less than both value1 and value2
                // 2 => testvalue is value1
                // 3 => testvalue is between value1 and value2
                // 4 => testvalue value2
                // 5 => testvalue is larger than both value1 and value2
                bool result1, result2;
                if (isfloat) {
                    const auto testvalue = getvalue<double>(test, d1, d2);
                    result1 = checkFloatRelation(op1, testvalue, d1);
                    result2 = checkFloatRelation(op2, testvalue, d2);
                } else if (useUnsignedInt) {
                    const auto testvalue = getvalue<MathLib::biguint>(test, u1, u2);
                    result1 = checkIntRelation(op1, testvalue, u1);
                    result2 = checkIntRelation(op2, testvalue, u2);
                } else {
                    const auto testvalue = getvalue<MathLib::bigint>(test, i1, i2);
                    result1 = checkIntRelation(op1, testvalue, i1);
                    result2 = checkIntRelation(op2, testvalue, i2);
                }
                if (not1)
                    result1 = !result1;
                if (not2)
                    result2 = !result2;
                if (isAnd) {
                    alwaysTrue &= (result1 && result2);
                    alwaysFalse &= !(result1 && result2);
                } else {
                    alwaysTrue &= (result1 || result2);
                    alwaysFalse &= !(result1 || result2);
                }
                firstTrue &= !(!result1 && result2);
                secondTrue &= !(result1 && !result2);
            }

            const std::string cond1str = conditionString(not1, expr1, op1, value1);
            const std::string cond2str = conditionString(not2, expr2, op2, value2);
            if (printWarning && (alwaysTrue || alwaysFalse)) {
                const std::string text = cond1str + " " + tok->str() + " " + cond2str;
                incorrectLogicOperatorError(tok, text, alwaysTrue, inconclusive, std::move(errorPath));
            } else if (printStyle && (firstTrue || secondTrue)) {
                const int which = isfloat ? sufficientCondition(std::move(op1), not1, d1, std::move(op2), not2, d2, isAnd) : sufficientCondition(std::move(op1), not1, i1, std::move(op2), not2, i2, isAnd);
                std::string text;
                if (which != 0) {
                    text = "The condition '" + (which == 1 ? cond2str : cond1str) + "' is redundant since '" + (which == 1 ? cond1str : cond2str) + "' is sufficient.";
                } else
                    text = "If '" + (secondTrue ? cond1str : cond2str) + "', the comparison '" + (secondTrue ? cond2str : cond1str) + "' is always true.";
                redundantConditionError(tok, text, inconclusive);
            }
        }
    }
}

void CheckCondition::incorrectLogicOperatorError(const Token *tok, const std::string &condition, bool always, bool inconclusive, ErrorPath errors)
{
    if (diag(tok))
        return;
    errors.emplace_back(tok, "");
    if (always)
        reportError(std::move(errors), Severity::warning, "incorrectLogicOperator",
                    "Logical disjunction always evaluates to true: " + condition + ".\n"
                    "Logical disjunction always evaluates to true: " + condition + ". "
                    "Are these conditions necessary? Did you intend to use && instead? Are the numbers correct? Are you comparing the correct variables?", CWE571, inconclusive ? Certainty::inconclusive : Certainty::normal);
    else
        reportError(std::move(errors), Severity::warning, "incorrectLogicOperator",
                    "Logical conjunction always evaluates to false: " + condition + ".\n"
                    "Logical conjunction always evaluates to false: " + condition + ". "
                    "Are these conditions necessary? Did you intend to use || instead? Are the numbers correct? Are you comparing the correct variables?", CWE570, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckCondition::redundantConditionError(const Token *tok, const std::string &text, bool inconclusive)
{
    if (diag(tok))
        return;
    reportError(tok, Severity::style, "redundantCondition", "Redundant condition: " + text, CWE398, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

//-----------------------------------------------------------------------------
// Detect "(var % val1) > val2" where val2 is >= val1.
//-----------------------------------------------------------------------------
void CheckCondition::checkModuloAlwaysTrueFalse()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckCondition::checkModuloAlwaysTrueFalse"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->isComparisonOp())
                continue;
            const Token *num, *modulo;
            if (Token::simpleMatch(tok->astOperand1(), "%") && Token::Match(tok->astOperand2(), "%num%")) {
                modulo = tok->astOperand1();
                num = tok->astOperand2();
            } else if (Token::Match(tok->astOperand1(), "%num%") && Token::simpleMatch(tok->astOperand2(), "%")) {
                num = tok->astOperand1();
                modulo = tok->astOperand2();
            } else {
                continue;
            }

            if (Token::Match(modulo->astOperand2(), "%num%") &&
                MathLib::isLessEqual(modulo->astOperand2()->str(), num->str()))
                moduloAlwaysTrueFalseError(tok, modulo->astOperand2()->str());
        }
    }
}

void CheckCondition::moduloAlwaysTrueFalseError(const Token* tok, const std::string& maxVal)
{
    if (diag(tok))
        return;
    reportError(tok, Severity::warning, "moduloAlwaysTrueFalse",
                "Comparison of modulo result is predetermined, because it is always less than " + maxVal + ".", CWE398, Certainty::normal);
}

static int countPar(const Token *tok1, const Token *tok2)
{
    int par = 0;
    for (const Token *tok = tok1; tok && tok != tok2; tok = tok->next()) {
        if (tok->str() == "(")
            ++par;
        else if (tok->str() == ")")
            --par;
        else if (tok->str() == ";")
            return -1;
    }
    return par;
}

//---------------------------------------------------------------------------
// Clarify condition '(x = a < 0)' into '((x = a) < 0)' or '(x = (a < 0))'
// Clarify condition '(a & b == c)' into '((a & b) == c)' or '(a & (b == c))'
//---------------------------------------------------------------------------
void CheckCondition::clarifyCondition()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("clarifyCondition"))
        return;

    logChecker("CheckCondition::clarifyCondition"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "( %name% [=&|^]")) {
                for (const Token *tok2 = tok->tokAt(3); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "[")
                        tok2 = tok2->link();
                    else if (tok2->isComparisonOp()) {
                        // This might be a template
                        if (!tok2->isC() && tok2->link())
                            break;
                        if (Token::simpleMatch(tok2->astParent(), "?"))
                            break;
                        clarifyConditionError(tok, tok->strAt(2) == "=", false);
                        break;
                    } else if (!tok2->isName() && !tok2->isNumber() && tok2->str() != ".")
                        break;
                }
            } else if (tok->tokType() == Token::eBitOp && !tok->isUnaryOp("&")) {
                if (tok->astOperand2() && tok->astOperand2()->variable() && tok->astOperand2()->variable()->nameToken() == tok->astOperand2())
                    continue;

                // using boolean result in bitwise operation ! x [&|^]
                const ValueType* vt1 = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
                const ValueType* vt2 = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;
                if (vt1 && vt1->type == ValueType::BOOL && !Token::Match(tok->astOperand1(), "%name%|(|[|::|.") && countPar(tok->astOperand1(), tok) == 0)
                    clarifyConditionError(tok, false, true);
                else if (vt2 && vt2->type == ValueType::BOOL && !Token::Match(tok->astOperand2(), "%name%|(|[|::|.") && countPar(tok, tok->astOperand2()) == 0)
                    clarifyConditionError(tok, false, true);
            }
        }
    }
}

void CheckCondition::clarifyConditionError(const Token *tok, bool assign, bool boolop)
{
    std::string errmsg;

    if (assign)
        errmsg = "Suspicious condition (assignment + comparison); Clarify expression with parentheses.";

    else if (boolop)
        errmsg = "Boolean result is used in bitwise operation. Clarify expression with parentheses.\n"
                 "Suspicious expression. Boolean result is used in bitwise operation. The operator '!' "
                 "and the comparison operators have higher precedence than bitwise operators. "
                 "It is recommended that the expression is clarified with parentheses.";
    else
        errmsg = "Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses.\n"
                 "Suspicious condition. Comparison operators have higher precedence than bitwise operators. "
                 "Please clarify the condition with parentheses.";

    reportError(tok,
                Severity::style,
                "clarifyCondition",
                errmsg, CWE398, Certainty::normal);
}

void CheckCondition::alwaysTrueFalse()
{
    const bool pedantic = mSettings->isPremiumEnabled("alwaysTrue") ||
                          mSettings->isPremiumEnabled("alwaysFalse") ||
                          mSettings->isPremiumEnabled("knownConditionTrueFalse");

    if (!pedantic && !mSettings->severity.isEnabled(Severity::style))
        return;

    logChecker("CheckCondition::alwaysTrueFalse"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            // don't write false positives when templates are used or inside of asserts or non-evaluated contexts
            if (tok->link() && (Token::simpleMatch(tok, "<") ||
                                Token::Match(tok->previous(), "static_assert|assert|ASSERT|sizeof|decltype ("))) {
                tok = tok->link();
                continue;
            }
            if (!tok->hasKnownIntValue())
                continue;
            const Token* condition = nullptr;
            {
                // is this a condition..
                const Token *parent = tok->astParent();
                while (Token::Match(parent, "%oror%|&&"))
                    parent = parent->astParent();
                if (!parent)
                    continue;
                if (parent->str() == "?" && precedes(tok, parent))
                    condition = parent;
                else if (Token::Match(parent->previous(), "if|while ("))
                    condition = parent->previous();
                else if (Token::simpleMatch(parent, "return"))
                    condition = parent;
                else if (parent->str() == ";" && parent->astParent() && parent->astParent()->astParent() &&
                         Token::simpleMatch(parent->astParent()->astParent()->previous(), "for ("))
                    condition = parent->astParent()->astParent()->previous();
                else if (Token::Match(tok, "%comp%"))
                    condition = tok;
                else if (tok->str() == "(" && astIsBool(parent) && Token::Match(parent, "%assign%"))
                    condition = tok;
                else
                    continue;
            }
            // Skip already diagnosed values
            if (diag(tok, false))
                continue;
            if (condition->isConstexpr())
                continue;
            if (!isUsedAsBool(tok, *mSettings))
                continue;
            if (Token::simpleMatch(condition, "return") && Token::Match(tok, "%assign%"))
                continue;
            if (Token::simpleMatch(tok->astParent(), "return") && Token::Match(tok, ".|%var%"))
                continue;
            if (Token::Match(tok, "%num%|%bool%|%char%"))
                continue;
            if (Token::Match(tok, "! %num%|%bool%|%char%"))
                continue;
            if (Token::Match(tok, "%oror%|&&")) {
                bool bail = false;
                for (const Token* op : { tok->astOperand1(), tok->astOperand2() }) {
                    if (op->hasKnownIntValue() && (!op->isLiteral() || op->isBoolean())) {
                        bail = true;
                        break;
                    }
                }
                if (bail)
                    continue;
            }
            if (Token::simpleMatch(tok, ":"))
                continue;
            if (Token::Match(tok->astOperand1(), "%name% (") && Token::simpleMatch(tok->astParent(), "return"))
                continue;
            if (tok->isComparisonOp() && isWithoutSideEffects(tok->astOperand1()) &&
                isSameExpression(true,
                                 tok->astOperand1(),
                                 tok->astOperand2(),
                                 *mSettings,
                                 true,
                                 true))
                continue;

            if (!pedantic && isConstVarExpression(tok, [](const Token* tok) {
                return Token::Match(tok, "[|(|&|+|-|*|/|%|^|>>|<<") && !Token::simpleMatch(tok, "( )");
            }))
                continue;

            // there are specific warnings about nonzero expressions (pointer/unsigned)
            // do not write alwaysTrueFalse for these comparisons.
            {
                const ValueFlow::Value *zeroValue = nullptr;
                const Token *nonZeroExpr = nullptr;
                if (CheckOther::comparisonNonZeroExpressionLessThanZero(tok, zeroValue, nonZeroExpr, /*suppress*/ true) ||
                    CheckOther::testIfNonZeroExpressionIsPositive(tok, zeroValue, nonZeroExpr))
                    continue;
            }

            // Don't warn when there are expanded macros..
            bool isExpandedMacro = false;
            visitAstNodes(tok, [&](const Token * tok2) {
                if (!tok2)
                    return ChildrenToVisit::none;
                if (tok2->isExpandedMacro()) {
                    isExpandedMacro = true;
                    return ChildrenToVisit::done;
                }
                return ChildrenToVisit::op1_and_op2;
            });
            if (isExpandedMacro)
                continue;
            for (const Token *parent = tok; parent; parent = parent->astParent()) {
                if (parent->isExpandedMacro()) {
                    isExpandedMacro = true;
                    break;
                }
            }
            if (isExpandedMacro)
                continue;

            // don't warn when condition checks sizeof result
            bool hasSizeof = false;
            visitAstNodes(tok, [&](const Token * tok2) {
                if (!tok2)
                    return ChildrenToVisit::none;
                if (tok2->isNumber())
                    return ChildrenToVisit::none;
                if (Token::simpleMatch(tok2->previous(), "sizeof (")) {
                    hasSizeof = true;
                    return ChildrenToVisit::none;
                }
                if (tok2->isComparisonOp() || tok2->isArithmeticalOp()) {
                    return ChildrenToVisit::op1_and_op2;
                }
                return ChildrenToVisit::none;
            });
            if (hasSizeof)
                continue;

            alwaysTrueFalseError(tok, condition, &tok->values().front());
        }
    }
}

static std::string getConditionString(const Token* condition)
{
    if (Token::simpleMatch(condition, "return"))
        return "Return value";
    if (Token::simpleMatch(condition, "(") && Token::Match(condition->astParent(), "%assign%"))
        return "Assigned value";
    return "Condition";
}

void CheckCondition::alwaysTrueFalseError(const Token* tok, const Token* condition, const ValueFlow::Value* value)
{
    const bool alwaysTrue = value && (value->intvalue != 0 || value->isImpossible());
    const std::string expr = tok ? tok->expressionString() : std::string("x");
    const std::string conditionStr = getConditionString(condition);
    const std::string errmsg = conditionStr + " '" + expr + "' is always " + bool_to_string(alwaysTrue);
    ErrorPath errorPath = getErrorPath(tok, value, errmsg);
    reportError(std::move(errorPath),
                Severity::style,
                "knownConditionTrueFalse",
                errmsg,
                (alwaysTrue ? CWE571 : CWE570), Certainty::normal);
}

void CheckCondition::checkInvalidTestForOverflow()
{
    // Interesting blogs:
    // https://www.airs.com/blog/archives/120
    // https://kristerw.blogspot.com/2016/02/how-undefined-signed-overflow-enables.html
    // https://research.checkpoint.com/2020/optout-compiler-undefined-behavior-optimizations/

    // x + c < x       ->   false
    // x + c <= x      ->   false
    // x + c > x       ->   true
    // x + c >= x      ->   true

    // x + y < x       ->   y < 0


    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckCondition::checkInvalidTestForOverflow"); // warning

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "<|<=|>=|>") || !tok->isBinaryOp())
            continue;

        const Token *lhsTokens[2] = {tok->astOperand1(), tok->astOperand2()};
        for (const Token *lhs: lhsTokens) {
            std::string cmp = tok->str();
            if (lhs == tok->astOperand2())
                cmp[0] = (cmp[0] == '<') ? '>' : '<';

            if (!Token::Match(lhs, "[+-]") || !lhs->isBinaryOp())
                continue;

            const bool isSignedInteger = lhs->valueType() && lhs->valueType()->isIntegral() && lhs->valueType()->sign == ValueType::Sign::SIGNED;
            const bool isPointer = lhs->valueType() && lhs->valueType()->pointer > 0;
            if (!isSignedInteger && !isPointer)
                continue;

            const Token *exprTokens[2] = {lhs->astOperand1(), lhs->astOperand2()};
            for (const Token *expr: exprTokens) {
                if (lhs->str() == "-" && expr == lhs->astOperand2())
                    continue; // TODO?

                if (expr->hasKnownIntValue())
                    continue;

                if (!isSameExpression(true,
                                      expr,
                                      lhs->astSibling(),
                                      *mSettings,
                                      true,
                                      false))
                    continue;

                const Token * const other = expr->astSibling();

                // x [+-] c cmp x
                if ((other->isNumber() && other->hasKnownIntValue() && other->getKnownIntValue() > 0) ||
                    (!other->isNumber() && other->valueType() && other->valueType()->isIntegral() && other->valueType()->sign == ValueType::Sign::UNSIGNED)) {
                    bool result;
                    if (lhs->str() == "+")
                        result = (cmp == ">" || cmp == ">=");
                    else
                        result = (cmp == "<" || cmp == "<=");
                    invalidTestForOverflow(tok, lhs->valueType(), bool_to_string(result));
                    continue;
                }

                // x + y cmp x
                if (lhs->str() == "+" && other->varId() > 0) {
                    const std::string result = other->str() + cmp + "0";
                    invalidTestForOverflow(tok, lhs->valueType(), result);
                    continue;
                }

                // x - y cmp x
                if (lhs->str() == "-" && other->varId() > 0) {
                    std::string cmp2 = cmp;
                    cmp2[0] = (cmp[0] == '<') ? '>' : '<';
                    const std::string result = other->str() + cmp2 + "0";
                    invalidTestForOverflow(tok, lhs->valueType(), result);
                    continue;
                }
            }
        }
    }
}

void CheckCondition::invalidTestForOverflow(const Token* tok, const ValueType *valueType, const std::string &replace)
{
    const std::string expr = (tok ? tok->expressionString() : std::string("x + c < x"));
    const std::string overflow = (valueType && valueType->pointer) ? "pointer overflow" : "signed integer overflow";

    std::string errmsg =
        "Invalid test for overflow '" +  expr + "'; " + overflow + " is undefined behavior.";
    if (replace == "false" || replace == "true")
        errmsg += " Some mainstream compilers remove such overflow tests when optimising the code and assume it's always " + replace + ".";
    else
        errmsg += " Some mainstream compilers removes handling of overflows when optimising the code and change the code to '" + replace + "'.";
    reportError(tok, Severity::warning, "invalidTestForOverflow", errmsg, uncheckedErrorConditionCWE, Certainty::normal);
}


void CheckCondition::checkPointerAdditionResultNotNull()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckCondition::checkPointerAdditionResultNotNull"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {

        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->isComparisonOp() || !tok->astOperand1() || !tok->astOperand2())
                continue;

            // Macros might have pointless safety checks
            if (tok->isExpandedMacro())
                continue;

            const Token *calcToken, *exprToken;
            if (tok->astOperand1()->str() == "+") {
                calcToken = tok->astOperand1();
                exprToken = tok->astOperand2();
            } else if (tok->astOperand2()->str() == "+") {
                calcToken = tok->astOperand2();
                exprToken = tok->astOperand1();
            } else
                continue;

            // pointer comparison against NULL (ptr+12==0)
            if (calcToken->hasKnownIntValue())
                continue;
            if (!calcToken->valueType() || calcToken->valueType()->pointer==0)
                continue;
            if (!exprToken->hasKnownIntValue() || !exprToken->getValue(0))
                continue;

            pointerAdditionResultNotNullError(tok, calcToken);
        }
    }
}

void CheckCondition::pointerAdditionResultNotNullError(const Token *tok, const Token *calc)
{
    const std::string s = calc ? calc->expressionString() : "ptr+1";
    reportError(tok, Severity::warning, "pointerAdditionResultNotNull", "Comparison is wrong. Result of '" + s + "' can't be 0 unless there is pointer overflow, and pointer overflow is undefined behaviour.");
}

void CheckCondition::checkDuplicateConditionalAssign()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("duplicateConditionalAssign"))
        return;

    logChecker("CheckCondition::checkDuplicateConditionalAssign"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "if ("))
                continue;
            if (!Token::simpleMatch(tok->linkAt(1), ") {"))
                continue;
            const Token *blockTok = tok->linkAt(1)->next();
            const Token *condTok = tok->next()->astOperand2();
            const bool isBoolVar = Token::Match(condTok, "!| %var%");
            if (!isBoolVar && !Token::Match(condTok, "==|!="))
                continue;
            if ((isBoolVar || condTok->str() == "!=") && Token::simpleMatch(blockTok->link(), "} else {"))
                continue;
            if (!blockTok->next())
                continue;
            const Token *assignTok = blockTok->next()->astTop();
            if (!Token::simpleMatch(assignTok, "="))
                continue;
            if (nextAfterAstRightmostLeaf(assignTok) != blockTok->link()->previous())
                continue;
            bool isRedundant = false;
            if (isBoolVar) {
                const bool isNegation = condTok->str() == "!";
                const Token* const varTok = isNegation ? condTok->next() : condTok;
                const ValueType* vt = varTok->variable() ? varTok->variable()->valueType() : nullptr;
                if (!(vt && vt->type == ValueType::Type::BOOL && !vt->pointer))
                    continue;

                if (!(assignTok->astOperand1() && assignTok->astOperand1()->varId() == varTok->varId()))
                    continue;
                if (!(assignTok->astOperand2() && assignTok->astOperand2()->hasKnownIntValue()))
                    continue;
                const MathLib::bigint val = assignTok->astOperand2()->getKnownIntValue();
                if (val < 0 || val > 1)
                    continue;
                isRedundant = (isNegation && val == 0) || (!isNegation && val == 1);
            } else { // comparison
                if (!isSameExpression(
                        true, condTok->astOperand1(), assignTok->astOperand1(), *mSettings, true, true))
                    continue;
                if (!isSameExpression(
                        true, condTok->astOperand2(), assignTok->astOperand2(), *mSettings, true, true))
                    continue;
            }
            duplicateConditionalAssignError(condTok, assignTok, isRedundant);
        }
    }
}

void CheckCondition::duplicateConditionalAssignError(const Token *condTok, const Token* assignTok, bool isRedundant)
{
    ErrorPath errors;
    std::string msg = "Duplicate expression for the condition and assignment.";
    if (condTok && assignTok) {
        if (condTok->str() == "==") {
            msg = "Assignment '" + assignTok->expressionString() + "' is redundant with condition '" + condTok->expressionString() + "'.";
            errors.emplace_back(condTok, "Condition '" + condTok->expressionString() + "'");
            errors.emplace_back(assignTok, "Assignment '" + assignTok->expressionString() + "' is redundant");
        } else {
            msg = "The statement 'if (" + condTok->expressionString() + ") " + assignTok->expressionString();
            msg += isRedundant ? "' is redundant." : "' is logically equivalent to '" + assignTok->expressionString() + "'.";
            errors.emplace_back(assignTok, "Assignment '" + assignTok->expressionString() + "'");
            errors.emplace_back(condTok, "Condition '" + condTok->expressionString() + "' is redundant");
        }
    }

    reportError(
        std::move(errors), Severity::style, "duplicateConditionalAssign", msg, CWE398, Certainty::normal);
}


void CheckCondition::checkAssignmentInCondition()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("assignmentInCondition"))
        return;

    logChecker("CheckCondition::checkAssignmentInCondition"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->str() != "=")
                continue;
            if (!tok->astParent())
                continue;

            // Is this assignment of container/iterator?
            if (!tok->valueType())
                continue;
            if (tok->valueType()->pointer > 0)
                continue;
            if (tok->valueType()->type != ValueType::Type::CONTAINER && tok->valueType()->type != ValueType::Type::ITERATOR)
                continue;

            // warn if this is a conditional expression..
            if (Token::Match(tok->astParent()->previous(), "if|while ("))
                assignmentInCondition(tok);
            else if (Token::Match(tok->astParent(), "%oror%|&&"))
                assignmentInCondition(tok);
            else if (Token::simpleMatch(tok->astParent(), "?") && tok == tok->astParent()->astOperand1())
                assignmentInCondition(tok);
        }
    }
}

void CheckCondition::assignmentInCondition(const Token *eq)
{
    std::string expr = eq ? eq->expressionString() : "x=y";

    reportError(
        eq,
        Severity::style,
        "assignmentInCondition",
        "Suspicious assignment in condition. Condition '" + expr + "' is always true.",
        CWE571,
        Certainty::normal);
}

void CheckCondition::checkCompareValueOutOfTypeRange()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("compareValueOutOfTypeRangeError"))
        return;

    if (mSettings->platform.type == Platform::Type::Unspecified)
        return;

    logChecker("CheckCondition::checkCompareValueOutOfTypeRange"); // style,platform

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->isComparisonOp() || !tok->isBinaryOp())
                continue;

            for (int i = 0; i < 2; ++i) {
                const Token * const valueTok = (i == 0) ? tok->astOperand1() : tok->astOperand2();
                const Token * const typeTok = valueTok->astSibling();
                if (!valueTok->hasKnownIntValue() || !typeTok->valueType() || typeTok->valueType()->pointer)
                    continue;
                if (valueTok->getKnownIntValue() < 0 && valueTok->valueType() && valueTok->valueType()->sign != ValueType::Sign::SIGNED)
                    continue;
                if (typeTok->isLiteral())
                    continue;
                std::uint8_t bits = 0;
                switch (typeTok->valueType()->type) {
                case ValueType::Type::BOOL:
                    bits = 1;
                    break;
                case ValueType::Type::CHAR:
                    bits = mSettings->platform.char_bit;
                    break;
                case ValueType::Type::SHORT:
                    bits = mSettings->platform.short_bit;
                    break;
                case ValueType::Type::INT:
                    bits = mSettings->platform.int_bit;
                    break;
                case ValueType::Type::LONG:
                    bits = mSettings->platform.long_bit;
                    break;
                case ValueType::Type::LONGLONG:
                    bits = mSettings->platform.long_long_bit;
                    break;
                default:
                    break;
                }
                if (bits == 0 || bits >= 63)
                    continue;

                const auto typeMinValue = (typeTok->valueType()->sign == ValueType::Sign::UNSIGNED) ? 0 : (-(1LL << (bits-1)));
                const auto unsignedTypeMaxValue = (1LL << bits) - 1LL;
                long long typeMaxValue;
                if (typeTok->valueType()->sign != ValueType::Sign::SIGNED)
                    typeMaxValue = unsignedTypeMaxValue;
                else if (bits >= mSettings->platform.int_bit && (!valueTok->valueType() || valueTok->valueType()->sign != ValueType::Sign::SIGNED))
                    typeMaxValue = unsignedTypeMaxValue;
                else
                    typeMaxValue = unsignedTypeMaxValue / 2;

                bool result{};
                const auto kiv = valueTok->getKnownIntValue();
                if (kiv == 0)
                    continue; // prevent overlap with TestOther::unsignedPositive/unsignedLessThanZero
                if (tok->str() == "==")
                    result = false;
                else if (tok->str() == "!=")
                    result = true;
                else if (tok->str()[0] == '>' && i == 0)
                    // num > var
                    result = (kiv > 0);
                else if (tok->str()[0] == '>' && i == 1)
                    // var > num
                    result = (kiv < 0);
                else if (tok->str()[0] == '<' && i == 0)
                    // num < var
                    result = (kiv < 0);
                else if (tok->str()[0] == '<' && i == 1)
                    // var < num
                    result = (kiv > 0);

                bool error = false;
                if (kiv < typeMinValue || kiv > typeMaxValue) {
                    error = true;
                } else {
                    switch (i) {
                    case 0: // num cmp var
                        if (kiv == typeMinValue) {
                            if (tok->str() == "<=") {
                                result = true;
                                error = true;
                            } else if (tok->str() == ">")
                                error = true;
                        }
                        else if (kiv == typeMaxValue && (tok->str() == ">=" || tok->str() == "<")) {
                            error = true;
                        }
                        break;
                    case 1: // var cmp num
                        if (kiv == typeMinValue) {
                            if (tok->str() == ">=") {
                                result = true;
                                error = true;
                            } else if (tok->str() == "<")
                                error = true;
                        }
                        else if (kiv == typeMaxValue && (tok->str() == "<=" || tok->str() == ">")) {
                            error = true;
                        }
                        break;
                    }
                }
                if (!error || diag(tok))
                    continue;
                compareValueOutOfTypeRangeError(valueTok, typeTok->valueType()->str(), kiv, result);
            }
        }
    }
}

void CheckCondition::compareValueOutOfTypeRangeError(const Token *comparison, const std::string &type, MathLib::bigint value, bool result)
{
    reportError(
        comparison,
        Severity::style,
        "compareValueOutOfTypeRangeError",
        "Comparing expression of type '" + type + "' against value " + MathLib::toString(value) + ". Condition is always " + bool_to_string(result) + ".",
        CWE398,
        Certainty::normal);
}

void CheckCondition::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckCondition checkCondition(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkCondition.multiCondition();
    checkCondition.clarifyCondition();   // not simplified because ifAssign
    checkCondition.multiCondition2();
    checkCondition.checkIncorrectLogicOperator();
    checkCondition.checkInvalidTestForOverflow();
    checkCondition.duplicateCondition();
    checkCondition.checkPointerAdditionResultNotNull();
    checkCondition.checkDuplicateConditionalAssign();
    checkCondition.assignIf();
    checkCondition.checkBadBitmaskCheck();
    checkCondition.comparison();
    checkCondition.checkModuloAlwaysTrueFalse();
    checkCondition.checkAssignmentInCondition();
    checkCondition.checkCompareValueOutOfTypeRange();
    checkCondition.alwaysTrueFalse();
}

void CheckCondition::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckCondition c(nullptr, settings, errorLogger);

    c.assignIfError(nullptr, nullptr, "", false);
    c.badBitmaskCheckError(nullptr);
    c.comparisonError(nullptr, "&", 6, "==", 1, false);
    c.duplicateConditionError(nullptr, nullptr, ErrorPath{});
    c.overlappingElseIfConditionError(nullptr, 1);
    c.overlappingInnerConditionError(nullptr, nullptr, ErrorPath());
    c.mismatchingBitAndError(nullptr, 0xf0, nullptr, 1);
    c.oppositeInnerConditionError(nullptr, nullptr, ErrorPath{});
    c.identicalInnerConditionError(nullptr, nullptr, ErrorPath{});
    c.identicalConditionAfterEarlyExitError(nullptr, nullptr, ErrorPath{});
    c.incorrectLogicOperatorError(nullptr, "foo > 3 && foo < 4", true, false, ErrorPath{});
    c.redundantConditionError(nullptr, "If x > 11 the condition x > 10 is always true.", false);
    c.moduloAlwaysTrueFalseError(nullptr, "1");
    c.clarifyConditionError(nullptr, true, false);
    c.alwaysTrueFalseError(nullptr, nullptr, nullptr);
    c.invalidTestForOverflow(nullptr, nullptr, "false");
    c.pointerAdditionResultNotNullError(nullptr, nullptr);
    c.duplicateConditionalAssignError(nullptr, nullptr);
    c.assignmentInCondition(nullptr);
    c.compareValueOutOfTypeRangeError(nullptr, "unsigned char", 256, true);
}
