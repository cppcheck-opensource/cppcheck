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
#include "checktype.h"

#include "astutils.h"
#include "errortypes.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <list>
#include <sstream>
#include <utility>
#include <vector>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckType instance;
}

//---------------------------------------------------------------------------
// Checking for shift by too many bits
//---------------------------------------------------------------------------
//

// CWE ids used:
static const CWE CWE195(195U);   // Signed to Unsigned Conversion Error
static const CWE CWE197(197U);   // Numeric Truncation Error
static const CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE190(190U);   // Integer Overflow or Wraparound


void CheckType::checkTooBigBitwiseShift()
{
    // unknown sizeof(int) => can't run this checker
    if (mSettings->platform.type == Platform::Type::Unspecified)
        return;

    logChecker("CheckType::checkTooBigBitwiseShift"); // platform

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        // C++ and macro: OUT(x<<y)
        if (tok->isCpp() && Token::Match(tok, "[;{}] %name% (") && Token::simpleMatch(tok->linkAt(2), ") ;") && tok->next()->isUpperCaseName() && !tok->next()->function())
            tok = tok->linkAt(2);

        tok = skipUnreachableBranch(tok);

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        if (!Token::Match(tok, "<<|>>|<<=|>>="))
            continue;

        // get number of bits of lhs
        const ValueType * const lhstype = tok->astOperand1()->valueType();
        if (!lhstype || !lhstype->isIntegral() || lhstype->pointer >= 1)
            continue;
        // C11 Standard, section 6.5.7 Bitwise shift operators, states:
        //   The integer promotions are performed on each of the operands.
        //   The type of the result is that of the promoted left operand.
        std::uint8_t lhsbits;
        if ((lhstype->type == ValueType::Type::CHAR) ||
            (lhstype->type == ValueType::Type::SHORT) ||
            (lhstype->type == ValueType::Type::WCHAR_T) ||
            (lhstype->type == ValueType::Type::BOOL) ||
            (lhstype->type == ValueType::Type::INT))
            lhsbits = mSettings->platform.int_bit;
        else if (lhstype->type == ValueType::Type::LONG)
            lhsbits = mSettings->platform.long_bit;
        else if (lhstype->type == ValueType::Type::LONGLONG)
            lhsbits = mSettings->platform.long_long_bit;
        else
            continue;

        // Get biggest rhs value. preferably a value which doesn't have 'condition'.
        const ValueFlow::Value * value = tok->astOperand2()->getValueGE(lhsbits, *mSettings);
        if (value && mSettings->isEnabled(value, false))
            tooBigBitwiseShiftError(tok, lhsbits, *value);
        else if (lhstype->sign == ValueType::Sign::SIGNED) {
            value = tok->astOperand2()->getValueGE(lhsbits-1, *mSettings);
            if (value && mSettings->isEnabled(value, false))
                tooBigSignedBitwiseShiftError(tok, lhsbits, *value);
        }
    }
}

void CheckType::tooBigBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits)
{
    constexpr char id[] = "shiftTooManyBits";

    if (!tok) {
        reportError(tok, Severity::error, id, "Shifting 32-bit value by 40 bits is undefined behaviour", CWE758, Certainty::normal);
        return;
    }

    ErrorPath errorPath = getErrorPath(tok, &rhsbits, "Shift");

    std::ostringstream errmsg;
    errmsg << "Shifting " << lhsbits << "-bit value by " << rhsbits.intvalue << " bits is undefined behaviour";
    if (rhsbits.condition)
        errmsg << ". See condition at line " << rhsbits.condition->linenr() << ".";

    reportError(std::move(errorPath), rhsbits.errorSeverity() ? Severity::error : Severity::warning, id, errmsg.str(), CWE758, rhsbits.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

void CheckType::tooBigSignedBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits)
{
    constexpr char id[] = "shiftTooManyBitsSigned";

    const bool cpp14 = ((tok && tok->isCpp()) || (mTokenizer && mTokenizer->isCPP())) && (mSettings->standards.cpp >= Standards::CPP14);

    std::string behaviour = "undefined";
    if (cpp14)
        behaviour = "implementation-defined";
    if (!tok) {
        reportError(tok, Severity::error, id, "Shifting signed 32-bit value by 31 bits is " + behaviour + " behaviour", CWE758, Certainty::normal);
        return;
    }


    Severity severity = rhsbits.errorSeverity() ? Severity::error : Severity::warning;
    if (cpp14)
        severity = Severity::portability;

    if ((severity == Severity::portability) && !mSettings->severity.isEnabled(Severity::portability))
        return;
    ErrorPath errorPath = getErrorPath(tok, &rhsbits, "Shift");

    std::ostringstream errmsg;
    errmsg << "Shifting signed " << lhsbits << "-bit value by " << rhsbits.intvalue << " bits is " + behaviour + " behaviour";
    if (rhsbits.condition)
        errmsg << ". See condition at line " << rhsbits.condition->linenr() << ".";

    reportError(std::move(errorPath), severity, id, errmsg.str(), CWE758, rhsbits.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// Checking for integer overflow
//---------------------------------------------------------------------------

void CheckType::checkIntegerOverflow()
{
    // unknown sizeof(int) => can't run this checker
    if (mSettings->platform.type == Platform::Type::Unspecified || mSettings->platform.int_bit >= MathLib::bigint_bits)
        return;

    logChecker("CheckType::checkIntegerOverflow"); // platform

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isArithmeticalOp())
            continue;

        // is result signed integer?
        const ValueType *vt = tok->valueType();
        if (!vt || !vt->isIntegral() || vt->sign != ValueType::Sign::SIGNED)
            continue;

        unsigned int bits;
        if (vt->type == ValueType::Type::INT)
            bits = mSettings->platform.int_bit;
        else if (vt->type == ValueType::Type::LONG)
            bits = mSettings->platform.long_bit;
        else if (vt->type == ValueType::Type::LONGLONG)
            bits = mSettings->platform.long_long_bit;
        else
            continue;

        if (bits >= MathLib::bigint_bits)
            continue;

        // max value according to platform settings.
        const MathLib::bigint maxvalue = (static_cast<MathLib::biguint>(1) << (bits - 1)) - 1;

        // is there a overflow result value
        bool isOverflow = true;
        const ValueFlow::Value *value = tok->getValueGE(maxvalue + 1, *mSettings);
        if (!value) {
            value = tok->getValueLE(-maxvalue - 2, *mSettings);
            isOverflow = false;
        }
        if (!value || !mSettings->isEnabled(value,false))
            continue;

        // For left shift, it's common practice to shift into the sign bit
        if (tok->str() == "<<" && value->intvalue > 0 && value->intvalue < (static_cast<MathLib::biguint>(1) << bits))
            continue;

        integerOverflowError(tok, *value, isOverflow);
    }
}

void CheckType::integerOverflowError(const Token *tok, const ValueFlow::Value &value, bool isOverflow)
{
    const std::string expr(tok ? tok->expressionString() : "");
    const std::string type = isOverflow ? "overflow" : "underflow";

    std::string msg;
    if (value.condition)
        msg = ValueFlow::eitherTheConditionIsRedundant(value.condition) +
              " or there is signed integer " + type + " for expression '" + expr + "'.";
    else
        msg = "Signed integer " + type + " for expression '" + expr + "'.";

    if (value.safe)
        msg = "Safe checks: " + msg;

    reportError(getErrorPath(tok, &value, "Integer " + type),
                value.errorSeverity() ? Severity::error : Severity::warning,
                getMessageId(value, "integerOverflow").c_str(),
                msg,
                CWE190,
                value.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// Checking for sign conversion when operand can be negative
//---------------------------------------------------------------------------

void CheckType::checkSignConversion()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckType::checkSignConversion"); // warning

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isArithmeticalOp() || Token::Match(tok,"+|-"))
            continue;

        // Is result unsigned?
        if (!(tok->valueType() && tok->valueType()->sign == ValueType::Sign::UNSIGNED))
            continue;

        // Check if an operand can be negative..
        const Token * astOperands[] = { tok->astOperand1(), tok->astOperand2() };
        for (const Token * tok1 : astOperands) {
            if (!tok1)
                continue;
            const ValueFlow::Value* negativeValue =
                ValueFlow::findValue(tok1->values(), *mSettings, [&](const ValueFlow::Value& v) {
                return !v.isImpossible() && v.isIntValue() && (v.intvalue <= -1 || v.wideintvalue <= -1);
            });
            if (!negativeValue)
                continue;
            if (tok1->valueType() && tok1->valueType()->sign != ValueType::Sign::UNSIGNED)
                signConversionError(tok1, negativeValue, tok1->isNumber());
        }
    }
}

void CheckType::signConversionError(const Token *tok, const ValueFlow::Value *negativeValue, const bool constvalue)
{
    const std::string expr(tok ? tok->expressionString() : "var");

    std::ostringstream msg;
    if (tok && tok->isName())
        msg << "$symbol:" << expr << "\n";
    if (constvalue)
        msg << "Expression '" << expr << "' has a negative value. That is converted to an unsigned value and used in an unsigned calculation.";
    else
        msg << "Expression '" << expr << "' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation.";

    if (!negativeValue)
        reportError(tok, Severity::warning, "signConversion", msg.str(), CWE195, Certainty::normal);
    else {
        ErrorPath errorPath = getErrorPath(tok,negativeValue,"Negative value is converted to an unsigned value");
        reportError(std::move(errorPath),
                    Severity::warning,
                    Check::getMessageId(*negativeValue, "signConversion").c_str(),
                    msg.str(),
                    CWE195,
                    negativeValue->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    }
}


//---------------------------------------------------------------------------
// Checking for long cast of int result   const long x = var1 * var2;
//---------------------------------------------------------------------------
static bool checkTypeCombination(ValueType src, ValueType tgt, const Settings& settings)
{
    static const std::pair<ValueType::Type, ValueType::Type> typeCombinations[] = {
        { ValueType::Type::INT, ValueType::Type::LONG },
        { ValueType::Type::INT, ValueType::Type::LONGLONG },
        { ValueType::Type::LONG, ValueType::Type::LONGLONG },
        { ValueType::Type::FLOAT, ValueType::Type::DOUBLE },
        { ValueType::Type::FLOAT, ValueType::Type::LONGDOUBLE },
        { ValueType::Type::DOUBLE, ValueType::Type::LONGDOUBLE },
    };

    src.reference = Reference::None;
    tgt.reference = Reference::None;

    const std::size_t sizeSrc = src.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
    const std::size_t sizeTgt = tgt.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
    if (!(sizeSrc > 0 && sizeTgt > 0 && sizeSrc < sizeTgt))
        return false;

    return std::any_of(std::begin(typeCombinations), std::end(typeCombinations), [&](const std::pair<ValueType::Type, ValueType::Type>& p) {
        return src.type == p.first && tgt.type == p.second;
    });
}

void CheckType::checkLongCast()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("truncLongCastAssignment"))
        return;

    logChecker("CheckType::checkLongCast"); // style

    // Assignments..
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "=" || !Token::Match(tok->astOperand2(), "*|<<") || tok->astOperand2()->isUnaryOp("*"))
            continue;

        if (const ValueFlow::Value* v = tok->astOperand2()->getKnownValue(ValueFlow::Value::ValueType::INT)) {
            if (mSettings->platform.isIntValue(v->intvalue))
                continue;
        }

        const ValueType *lhstype = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
        const ValueType *rhstype = tok->astOperand2()->valueType();

        if (!lhstype || !rhstype)
            continue;
        if (!checkTypeCombination(*rhstype, *lhstype, *mSettings))
            continue;

        // assign int result to long/longlong const nonpointer?
        if (rhstype->pointer == 0U &&
            rhstype->originalTypeName.empty() &&
            lhstype->pointer == 0U &&
            lhstype->originalTypeName.empty())
            longCastAssignError(tok, rhstype, lhstype);
    }

    // Return..
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {

        // function must return long data
        const Token * def = scope->classDef;
        if (!Token::Match(def, "%name% (") || !def->next()->valueType())
            continue;
        const ValueType* retVt = def->next()->valueType();

        // return statements
        const Token *ret = nullptr;
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->str() == "return") {
                if (Token::Match(tok->astOperand1(), "<<|*")) {
                    const ValueType *type = tok->astOperand1()->valueType();
                    if (type && checkTypeCombination(*type, *retVt, *mSettings) &&
                        type->pointer == 0U &&
                        type->originalTypeName.empty()) {
                        std::pair<MathLib::bigint, MathLib::bigint> opRange1, opRange2;
                        if (tok->astOperand1()->hasKnownIntValue()) {
                            if (!mSettings->platform.isIntValue(tok->astOperand1()->getKnownIntValue()))
                                ret = tok;
                        } else if (!getExpressionResultRange(tok->astOperand1()->astOperand1(), *mSettings, opRange1) || !getExpressionResultRange(tok->astOperand1()->astOperand2(), *mSettings, opRange2)) {
                            ret = tok;
                        } else if (!mSettings->platform.isIntValue(opRange1.first) || !mSettings->platform.isIntValue(opRange1.second) ||
                                   !mSettings->platform.isIntValue(opRange2.first) || !mSettings->platform.isIntValue(opRange2.second)) {
                            ret = tok;
                        } else if (!isOperationResultWithinIntRange(tok->astOperand1(), *mSettings, &opRange1, &opRange2)) {
                            ret = tok;
                        }
                    }
                }
                // All return statements must have problem otherwise no warning
                if (ret != tok) {
                    ret = nullptr;
                    break;
                }
            }
        }

        if (ret)
            longCastReturnError(ret, ret->astOperand1()->valueType(), retVt);
    }
}

static void makeBaseTypeString(std::string& typeStr)
{
    const auto pos = typeStr.find("signed");
    if (pos != std::string::npos)
        typeStr.erase(typeStr.begin(), typeStr.begin() + pos + 6 + 1);
}

void CheckType::longCastAssignError(const Token *tok, const ValueType* src, const ValueType* tgt)
{
    std::string srcStr = src ? src->str() : "int";
    makeBaseTypeString(srcStr);
    std::string tgtStr = tgt ? tgt->str() : "long";
    makeBaseTypeString(tgtStr);
    reportError(tok,
                Severity::style,
                "truncLongCastAssignment",
                srcStr + " result is assigned to " + tgtStr + " variable. If the variable is " + tgtStr + " to avoid loss of information, then you have loss of information.\n" +
                srcStr + " result is assigned to " + tgtStr + " variable. If the variable is " + tgtStr + " to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to " + tgtStr + ", for example 'l = a * b;' => 'l = (" + tgtStr + ")a * b;'.", CWE197, Certainty::normal);
}

void CheckType::longCastReturnError(const Token *tok, const ValueType* src, const ValueType* tgt)
{
    std::string srcStr = src ? src->str() : "int";
    makeBaseTypeString(srcStr);
    std::string tgtStr = tgt ? tgt->str() : "long";
    makeBaseTypeString(tgtStr);
    reportError(tok,
                Severity::style,
                "truncLongCastReturn",
                srcStr +" result is returned as " + tgtStr + " value. If the return value is " + tgtStr + " to avoid loss of information, then you have loss of information.\n" +
                srcStr +" result is returned as " + tgtStr + " value. If the return value is " + tgtStr + " to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'return a*b;' => 'return (long)a*b'.", CWE197, Certainty::normal);
}

//---------------------------------------------------------------------------
// Checking for float to integer overflow
//---------------------------------------------------------------------------

void CheckType::checkFloatToIntegerOverflow()
{
    logChecker("CheckType::checkFloatToIntegerOverflow");

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        const ValueType *vtint, *vtfloat;

        // Explicit cast
        if (Token::Match(tok, "( %name%") && tok->astOperand1() && !tok->astOperand2()) {
            if (isUnreachableOperand(tok))
                continue;
            vtint = tok->valueType();
            vtfloat = tok->astOperand1()->valueType();
            checkFloatToIntegerOverflow(tok, vtint, vtfloat, tok->astOperand1()->values());
        }

        // Assignment
        else if (tok->str() == "=" && tok->astOperand1() && tok->astOperand2()) {
            if (isUnreachableOperand(tok))
                continue;
            vtint = tok->astOperand1()->valueType();
            vtfloat = tok->astOperand2()->valueType();
            checkFloatToIntegerOverflow(tok, vtint, vtfloat, tok->astOperand2()->values());
        }

        else if (tok->str() == "return" && tok->astOperand1() && tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->isFloat()) {
            if (isUnreachableOperand(tok))
                continue;
            const Scope *scope = tok->scope();
            while (scope && scope->type != ScopeType::eLambda && scope->type != ScopeType::eFunction)
                scope = scope->nestedIn;
            if (scope && scope->type == ScopeType::eFunction && scope->function && scope->function->retDef) {
                const ValueType &valueType = ValueType::parseDecl(scope->function->retDef, *mSettings);
                vtfloat = tok->astOperand1()->valueType();
                checkFloatToIntegerOverflow(tok, &valueType, vtfloat, tok->astOperand1()->values());
            }
        }
    }
}

void CheckType::checkFloatToIntegerOverflow(const Token *tok, const ValueType *vtint, const ValueType *vtfloat, const std::list<ValueFlow::Value> &floatValues)
{
    // Conversion of float to integer?
    if (!vtint || !vtint->isIntegral())
        return;
    if (!vtfloat || !vtfloat->isFloat())
        return;

    for (const ValueFlow::Value &f : floatValues) {
        if (f.valueType != ValueFlow::Value::ValueType::FLOAT)
            continue;
        if (!mSettings->isEnabled(&f, false))
            continue;
        if (f.floatValue >= std::exp2(mSettings->platform.long_long_bit))
            floatToIntegerOverflowError(tok, f);
        else if ((-f.floatValue) > std::exp2(mSettings->platform.long_long_bit - 1))
            floatToIntegerOverflowError(tok, f);
        else if (mSettings->platform.type != Platform::Type::Unspecified) {
            int bits = 0;
            if (vtint->type == ValueType::Type::CHAR)
                bits = mSettings->platform.char_bit;
            else if (vtint->type == ValueType::Type::SHORT)
                bits = mSettings->platform.short_bit;
            else if (vtint->type == ValueType::Type::INT)
                bits = mSettings->platform.int_bit;
            else if (vtint->type == ValueType::Type::LONG)
                bits = mSettings->platform.long_bit;
            else if (vtint->type == ValueType::Type::LONGLONG)
                bits = mSettings->platform.long_long_bit;
            else
                continue;
            if (bits < MathLib::bigint_bits && f.floatValue >= (1ULL << bits))
                floatToIntegerOverflowError(tok, f);
        }
    }
}

void CheckType::floatToIntegerOverflowError(const Token *tok, const ValueFlow::Value &value)
{
    std::ostringstream errmsg;
    errmsg << "Undefined behaviour: float (" << value.floatValue << ") to integer conversion overflow.";
    reportError(getErrorPath(tok, &value, "float to integer conversion"),
                value.errorSeverity() ? Severity::error : Severity::warning,
                "floatConversionOverflow",
                errmsg.str(), CWE190, value.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

void CheckType::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    // These are not "simplified" because casts can't be ignored
    CheckType checkType(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkType.checkTooBigBitwiseShift();
    checkType.checkIntegerOverflow();
    checkType.checkSignConversion();
    checkType.checkLongCast();
    checkType.checkFloatToIntegerOverflow();
}

void CheckType::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckType c(nullptr, settings, errorLogger);
    c.tooBigBitwiseShiftError(nullptr, 32, ValueFlow::Value(64));
    c.tooBigSignedBitwiseShiftError(nullptr, 31, ValueFlow::Value(31));
    c.integerOverflowError(nullptr, ValueFlow::Value(1LL<<32));
    // TODO: integerOverflowCond
    c.signConversionError(nullptr, nullptr, false);
    // TODO: signConversionCond
    c.longCastAssignError(nullptr);
    c.longCastReturnError(nullptr);
    ValueFlow::Value f;
    f.valueType = ValueFlow::Value::ValueType::FLOAT;
    f.floatValue = 1E100;
    c.floatToIntegerOverflowError(nullptr, f);
}
