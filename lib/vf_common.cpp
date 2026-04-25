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

#include "vf_common.h"

#include "astutils.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <utility>
#include <vector>

namespace ValueFlow
{
    bool getMinMaxValues(const ValueType *vt, const Platform &platform, MathLib::bigint &minValue, MathLib::bigint &maxValue)
    {
        if (!vt || !vt->isIntegral() || vt->pointer)
            return false;

        std::uint8_t bits;
        switch (vt->type) {
        case ValueType::Type::BOOL:
            bits = 1;
            break;
        case ValueType::Type::CHAR:
            bits = platform.char_bit;
            break;
        case ValueType::Type::SHORT:
            bits = platform.short_bit;
            break;
        case ValueType::Type::INT:
            bits = platform.int_bit;
            break;
        case ValueType::Type::LONG:
            bits = platform.long_bit;
            break;
        case ValueType::Type::LONGLONG:
            bits = platform.long_long_bit;
            break;
        default:
            return false;
        }

        if (bits == 1) {
            minValue = 0;
            maxValue = 1;
        } else if (bits < 62) {
            if (vt->sign == ValueType::Sign::UNSIGNED) {
                minValue = 0;
                maxValue = (1LL << bits) - 1;
            } else {
                minValue = -(1LL << (bits - 1));
                maxValue = (1LL << (bits - 1)) - 1;
            }
        } else if (bits == 64) {
            if (vt->sign == ValueType::Sign::UNSIGNED) {
                minValue = 0;
                maxValue = LLONG_MAX; // todo max unsigned value
            } else {
                minValue = LLONG_MIN;
                maxValue = LLONG_MAX;
            }
        } else {
            return false;
        }

        return true;
    }

    MathLib::bigint truncateIntValue(MathLib::bigint value, size_t value_size, const ValueType::Sign dst_sign)
    {
        if (value_size == 0)
            return value;

        const MathLib::biguint unsignedMaxValue = std::numeric_limits<MathLib::biguint>::max() >> ((sizeof(unsignedMaxValue) - value_size) * 8);
        const MathLib::biguint signBit = 1ULL << ((value_size * 8) - 1);
        value &= unsignedMaxValue;
        if (dst_sign == ValueType::Sign::SIGNED && (value & signBit))
            value |= ~unsignedMaxValue;

        return value;
    }

    static nonneg int getSizeOfType(const Token *typeTok, const Settings &settings)
    {
        const ValueType &valueType = ValueType::parseDecl(typeTok, settings);

        return valueType.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
    }

    // Handle various constants..
    Token * valueFlowSetConstantValue(Token *tok, const Settings &settings)
    {
        if ((tok->isNumber() && MathLib::isInt(tok->str())) || (tok->tokType() == Token::eChar)) {
            try {
                MathLib::bigint signedValue = MathLib::toBigNumber(tok);
                const ValueType* vt = tok->valueType();
                if (vt && vt->sign == ValueType::UNSIGNED && signedValue < 0
                    && vt->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer)
                    < sizeof(MathLib::bigint)) {
                    MathLib::bigint minValue{}, maxValue{};
                    if (getMinMaxValues(tok->valueType(), settings.platform, minValue, maxValue))
                        signedValue += maxValue + 1;
                }
                Value value(signedValue);
                if (!tok->isTemplateArg())
                    value.setKnown();
                setTokenValue(tok, std::move(value), settings);
            } catch (const std::exception & /*e*/) {
                // Bad character literal
            }
        } else if (tok->isNumber() && MathLib::isFloat(tok->str())) {
            Value value;
            value.valueType = Value::ValueType::FLOAT;
            value.floatValue = MathLib::toDoubleNumber(tok);
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, std::move(value), settings);
        } else if (tok->enumerator() && tok->enumerator()->value_known) {
            Value value(tok->enumerator()->value);
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, std::move(value), settings);
        } else if (tok->str() == "NULL" || ((tok->isCpp() || settings.standards.c >= Standards::C23) && tok->str() == "nullptr")) {
            Value value(0);
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, std::move(value), settings);
        } else if ((tok->isCpp() || settings.standards.c >= Standards::C23) && (tok->isName() && tok->varId() == 0 && Token::Match(tok, "%bool%"))) {
            Value value(tok->str() == "true");
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, std::move(value), settings);
        } else if (Token::simpleMatch(tok, "sizeof (")) {
            if (tok->next()->astOperand2() && !tok->next()->astOperand2()->isLiteral() && tok->next()->astOperand2()->valueType() &&
                (tok->next()->astOperand2()->valueType()->pointer == 0 || // <- TODO this is a bailout, abort when there are array->pointer conversions
                 (tok->next()->astOperand2()->variable() && !tok->next()->astOperand2()->variable()->isArray())) &&
                !tok->next()->astOperand2()->valueType()->isEnum()) { // <- TODO this is a bailout, handle enum with non-int types
                const size_t sz = tok->next()->astOperand2()->valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
                if (sz) {
                    Value value(sz);
                    value.setKnown();
                    setTokenValue(tok->next(), std::move(value), settings);
                    return tok->linkAt(1);
                }
            }

            const Token *tok2 = tok->tokAt(2);
            // skip over tokens to find variable or type
            while (tok2 && !tok2->isStandardType() && Token::Match(tok2, "%name% ::|.|[")) {
                if (tok2->strAt(1) == "[")
                    tok2 = tok2->linkAt(1)->next();
                else
                    tok2 = tok2->tokAt(2);
            }
            if (Token::simpleMatch(tok, "sizeof ( *")) {
                const ValueType *vt = tok->tokAt(2)->valueType();
                const size_t sz = vt ? vt->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer)
                                     : 0;
                if (sz > 0) {
                    Value value(sz);
                    if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                        value.setKnown();
                    setTokenValue(tok->next(), std::move(value), settings);
                }
            } else if (tok2->enumerator() && tok2->enumerator()->scope) {
                long long size = settings.platform.sizeof_int;
                const Token * type = tok2->enumerator()->scope->enumType;
                if (type) {
                    size = getSizeOfType(type, settings);
                    if (size == 0)
                        tok->linkAt(1);
                }
                Value value(size);
                if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                    value.setKnown();
                setTokenValue(tok, value, settings);
                setTokenValue(tok->next(), std::move(value), settings);
            } else if (tok2->type() && tok2->type()->isEnumType()) {
                long long size = settings.platform.sizeof_int;
                if (tok2->type()->classScope) {
                    const Token * type = tok2->type()->classScope->enumType;
                    if (type) {
                        size = getSizeOfType(type, settings);
                    }
                }
                Value value(size);
                if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                    value.setKnown();
                setTokenValue(tok, value, settings);
                setTokenValue(tok->next(), std::move(value), settings);
            } else if (Token::Match(tok, "sizeof ( %var% ) /") && tok->next()->astParent() == tok->tokAt(4) &&
                       tok->tokAt(4)->astOperand2() && Token::simpleMatch(tok->tokAt(4)->astOperand2()->previous(), "sizeof (")) {
                // Get number of elements in array
                const Token *sz1 = tok->tokAt(2);
                const Token *sz2 = tok->tokAt(4)->astOperand2(); // left parenthesis of sizeof on rhs
                const nonneg int varid1 = sz1->varId();
                if (varid1 &&
                    sz1->variable() &&
                    sz1->variable()->isArray() &&
                    !sz1->variable()->dimensions().empty() &&
                    sz1->variable()->dimensionKnown(0) &&
                    Token::Match(sz2->astOperand2(), "*|[") && Token::Match(sz2->astOperand2()->astOperand1(), "%varid%", varid1)) {
                    Value value(sz1->variable()->dimension(0));
                    if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                        value.setKnown();
                    setTokenValue(tok->tokAt(4), std::move(value), settings);
                }
            } else if (Token::Match(tok2, "%var% )")) {
                if (const Variable *var = tok2->variable()) {
                    // find the size of the type
                    size_t size = 0;
                    if (var->isEnumType()) {
                        size = settings.platform.sizeof_int;
                        if (var->type()->classScope && var->type()->classScope->enumType)
                            size = getSizeOfType(var->type()->classScope->enumType, settings);
                    } else if (var->valueType()) {
                        size = var->valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
                    } else if (!var->type()) {
                        size = getSizeOfType(var->typeStartToken(), settings);
                    }
                    // find the number of elements
                    size_t count = 1;
                    for (size_t i = 0; i < var->dimensions().size(); ++i) {
                        if (var->dimensionKnown(i))
                            count *= var->dimension(i);
                        else
                            count = 0;
                    }
                    if (size && count > 0) {
                        Value value(count * size);
                        if (settings.platform.type != Platform::Type::Unspecified)
                            value.setKnown();
                        setTokenValue(tok, value, settings);
                        setTokenValue(tok->next(), std::move(value), settings);
                    }
                }
            } else if (tok2->tokType() == Token::eString) {
                const size_t sz = Token::getStrSize(tok2, settings);
                if (sz > 0) {
                    Value value(sz);
                    value.setKnown();
                    setTokenValue(tok->next(), std::move(value), settings);
                }
            } else if (tok2->tokType() == Token::eChar) {
                nonneg int sz = 0;
                if (tok2->isCpp() && settings.standards.cpp >= Standards::CPP20 && tok2->isUtf8())
                    sz = 1;
                else if (tok2->isUtf16())
                    sz = 2;
                else if (tok2->isUtf32())
                    sz = 4;
                else if (tok2->isLong())
                    sz = settings.platform.sizeof_wchar_t;
                else if ((!tok2->isCpp() && tok2->isCChar()) || (tok2->isCMultiChar()))
                    sz = settings.platform.sizeof_int;
                else
                    sz = 1;

                if (sz > 0) {
                    Value value(sz);
                    value.setKnown();
                    setTokenValue(tok->next(), std::move(value), settings);
                }
            } else if (!tok2->type()) {
                const ValueType& vt = ValueType::parseDecl(tok2, settings);
                size_t sz = vt.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
                const Token* brac = tok2->astParent();
                while (Token::simpleMatch(brac, "[")) {
                    const Token* num = brac->astOperand2();
                    if (num && ((num->isNumber() && MathLib::isInt(num->str())) || num->tokType() == Token::eChar)) {
                        try {
                            const MathLib::biguint dim = MathLib::toBigUNumber(num);
                            sz *= dim;
                            brac = brac->astParent();
                            continue;
                        }
                        catch (const std::exception& /*e*/) {
                            // Bad integer literal
                        }
                    }
                    sz = 0;
                    break;
                }
                if (sz > 0) {
                    Value value(sz);
                    if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                        value.setKnown();
                    setTokenValue(tok->next(), std::move(value), settings);
                }
            }
            // skip over enum
            tok = tok->linkAt(1);
        } else if (Token::Match(tok, "%name% [{(] [)}]") && (tok->isStandardType() ||
                                                             (tok->variable() && tok->variable()->nameToken() == tok &&
                                                              (tok->variable()->isPointer() || (tok->variable()->valueType() && tok->variable()->valueType()->isIntegral()))))) {
            Value value(0);
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok->next(), std::move(value), settings);
        } else if (Token::simpleMatch(tok, "= { }")) {
            const Token* lhs = tok->astOperand1();
            if (lhs && lhs->valueType() && (lhs->valueType()->isIntegral() || lhs->valueType()->pointer > 0)) {
                Value value(0);
                value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
            }
        }
        return tok->next();
    }

    Value castValue(Value value, const ValueType::Sign sign, nonneg int bit)
    {
        if (value.isFloatValue()) {
            value.valueType = Value::ValueType::INT;
            if (value.floatValue >= std::numeric_limits<int>::min() && value.floatValue <= std::numeric_limits<int>::max()) {
                value.intvalue = static_cast<MathLib::bigint>(value.floatValue);
            } else { // don't perform UB
                value.intvalue = 0;
            }
        }
        if (bit < MathLib::bigint_bits) {
            constexpr MathLib::biguint one = 1;
            value.intvalue &= (one << bit) - 1;
            if (sign == ValueType::Sign::SIGNED && value.intvalue & (one << (bit - 1))) {
                value.intvalue |= ~((one << bit) - 1ULL);
            }
        }
        return value;
    }

    std::string debugString(const Value& v)
    {
        std::string kind;
        switch (v.valueKind) {

        case Value::ValueKind::Impossible:
        case Value::ValueKind::Known:
            kind = "always";
            break;
        case Value::ValueKind::Inconclusive:
            kind = "inconclusive";
            break;
        case Value::ValueKind::Possible:
            kind = "possible";
            break;
        }
        return kind + " " + v.toString();
    }

    void setSourceLocation(Value& v,
                           SourceLocation ctx,
                           const Token* tok,
                           SourceLocation local)
    {
        std::string file = ctx.file_name();
        if (file.empty())
            return;
        std::string s = Path::stripDirectoryPart(file) + ":" + std::to_string(ctx.line()) + ": " + ctx.function_name() +
                        " => " + local.function_name() + ": " + debugString(v);
        v.debugPath.emplace_back(tok, std::move(s));
    }

    MathLib::bigint valueFlowGetStrLength(const Token* tok)
    {
        if (tok->tokType() == Token::eString)
            return Token::getStrLength(tok);
        if (astIsGenericChar(tok) || tok->tokType() == Token::eChar)
            return 1;
        if (const Value* v = tok->getKnownValue(Value::ValueType::CONTAINER_SIZE))
            return v->intvalue;
        if (const Value* v = tok->getKnownValue(Value::ValueType::TOK)) {
            if (v->tokvalue != tok)
                return valueFlowGetStrLength(v->tokvalue);
        }
        return 0;
    }
}
