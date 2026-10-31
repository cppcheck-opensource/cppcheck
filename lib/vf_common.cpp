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
#include "library.h"
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
#include <cmath>
#include <exception>
#include <limits>
#include <algorithm>
#include <utility>
#include <vector>

namespace ValueFlow
{
    bool getMinMaxValues(const ValueType *vt, const Platform &platform, MathLib::bigint &minValue, MathLib::bigint &maxValue)
    {
        if (!vt || !vt->isIntegral() || vt->pointer)
            return false;

        std::size_t bits;
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
        case ValueType::Type::WCHAR_T:
            bits = platform.sizeof_wchar_t * platform.char_bit;
            break;
        default:
            return false;
        }

        if (bits == 0) {
            return false;
        }
        if (bits == 1) {
            minValue = 0;
            maxValue = 1;
        } else if (bits < 62) {
            if (vt->sign == ValueType::Sign::UNSIGNED) {
                minValue = 0;
                maxValue = (1LL << bits) - 1;
            } else if (vt->sign == ValueType::Sign::SIGNED) {
                minValue = -(1LL << (bits - 1));
                maxValue = (1LL << (bits - 1)) - 1;
            } else
                return false;
        } else if (bits == 64) {
            if (vt->sign == ValueType::Sign::UNSIGNED) {
                minValue = 0;
                maxValue = LLONG_MAX; // MathLib::bigint cannot represent ULLONG_MAX; conservative max for unsigned 64-bit
            } else if (vt->sign == ValueType::Sign::SIGNED) {
                minValue = LLONG_MIN;
                maxValue = LLONG_MAX;
            } else
                return false;
        } else {
            return false;
        }

        return true;
    }
    static bool convertFloatToInteger(MathLib::bigint& result,
                                      double value,
                                      ValueType::Sign sign,
                                      nonneg int bit)
    {
        if (!std::isfinite(value) || bit <= 0 || sign == ValueType::Sign::UNKNOWN_SIGN)
            return false;
        const double truncatedValue = std::trunc(value);
        const int destinationBits = sign == ValueType::Sign::SIGNED ? bit - 1 : bit;
        const int carrierBits = std::numeric_limits<MathLib::bigint>::digits;
        const double upperExclusive = std::ldexp(1.0, std::min(destinationBits, carrierBits));
        if (sign == ValueType::Sign::SIGNED) {
            if (truncatedValue < -upperExclusive || truncatedValue >= upperExclusive)
                return false;
        } else if (truncatedValue < 0.0 || truncatedValue >= upperExclusive) {
            return false;
        }
        result = static_cast<MathLib::bigint>(truncatedValue);
        return true;
    }


    bool isValueRangePreservingConversion(const ValueType* source,
                                          const ValueType* destination,
                                          const Settings& settings,
                                          const Value* range)
    {
        // Non-null pointers convert to true; other pointer-to-integer mappings are not injective.
        if (source && source->pointer > 0 && destination && destination->type == ValueType::Type::BOOL &&
            !destination->pointer && range && range->isIntValue() && range->isImpossible() &&
            range->bound == Value::Bound::Point && range->intvalue == 0)
            return true;
        if (!source || !destination || !destination->isIntegral() || source->pointer || destination->pointer)
            return false;
        if (source->isFloat()) {
            if (!range || !range->isFloatValue() || range->isImpossible() ||
                range->bound == Value::Bound::Point || destination->type == ValueType::Type::BOOL)
                return false;
            const std::size_t destinationBits = destination->getSizeOf(settings,
                                                                       ValueType::Accuracy::ExactOrZero,
                                                                       ValueType::SizeOf::Pointee) * settings.platform.char_bit;
            MathLib::bigint converted{};
            return destinationBits > 0 && destinationBits <= static_cast<std::size_t>(std::numeric_limits<int>::max()) &&
                   convertFloatToInteger(converted, range->floatValue, destination->sign, static_cast<int>(destinationBits));
        }
        if (!source->isIntegral())
            return false;
        if (source->isTypeEqual(destination) && source->sign == destination->sign)
            return true;

        if (range && range->isIntValue() && range->bound != Value::Bound::Point && !range->isInconclusive()) {
            MathLib::bigint sourceMin{}, sourceMax{}, destinationMin{}, destinationMax{};
            if (getMinMaxValues(source, settings.platform, sourceMin, sourceMax) &&
                getMinMaxValues(destination, settings.platform, destinationMin, destinationMax)) {
                MathLib::bigint lower = sourceMin;
                MathLib::bigint upper = sourceMax;
                bool upperBoundedByFact = false;
                if (range->isImpossible()) {
                    if (range->bound == Value::Bound::Upper) {
                        if (range->intvalue == std::numeric_limits<MathLib::bigint>::max())
                            return false;
                        lower = std::max(lower, range->intvalue + 1);
                    } else if (range->bound == Value::Bound::Lower) {
                        if (range->intvalue == std::numeric_limits<MathLib::bigint>::min())
                            return false;
                        upper = std::min(upper, range->intvalue - 1);
                        upperBoundedByFact = true;
                    }
                } else if (range->bound == Value::Bound::Lower) {
                    lower = std::max(lower, range->intvalue);
                } else if (range->bound == Value::Bound::Upper) {
                    upper = std::min(upper, range->intvalue);
                    upperBoundedByFact = true;
                }

                const std::size_t sourceBits = source->getSizeOf(settings,
                                                                 ValueType::Accuracy::ExactOrZero,
                                                                 ValueType::SizeOf::Pointee) * settings.platform.char_bit;
                const bool sourceHasRepresentableMaximum = source->sign != ValueType::Sign::UNSIGNED ||
                                                           sourceBits <= std::numeric_limits<MathLib::bigint>::digits;
                if (lower <= upper && (sourceHasRepresentableMaximum || upperBoundedByFact) &&
                    lower >= destinationMin && upper <= destinationMax)
                    return true;
            }
        }

        if (source->type == ValueType::Type::BOOL || destination->type == ValueType::Type::BOOL) {
            MathLib::bigint sourceMin{}, sourceMax{}, destinationMin{}, destinationMax{};
            return getMinMaxValues(source, settings.platform, sourceMin, sourceMax) &&
                   getMinMaxValues(destination, settings.platform, destinationMin, destinationMax) &&
                   sourceMin >= destinationMin && sourceMax <= destinationMax;
        }

        if (destination->sign == ValueType::Sign::UNKNOWN_SIGN)
            return false;

        const std::size_t sourceBits = source->getSizeOf(settings,
                                                         ValueType::Accuracy::ExactOrZero,
                                                         ValueType::SizeOf::Pointee) * settings.platform.char_bit;
        const std::size_t destinationBits = destination->getSizeOf(settings,
                                                                   ValueType::Accuracy::ExactOrZero,
                                                                   ValueType::SizeOf::Pointee) * settings.platform.char_bit;
        if (sourceBits == 0 || destinationBits == 0)
            return false;
        if (source->sign == ValueType::Sign::UNKNOWN_SIGN)
            return destination->sign == ValueType::Sign::SIGNED && sourceBits < destinationBits;

        if (source->sign == destination->sign)
            return sourceBits <= destinationBits;
        if (range && range->isIntValue() && range->isImpossible() && range->bound == Value::Bound::Point &&
            source->sign == ValueType::Sign::SIGNED && destination->sign == ValueType::Sign::UNSIGNED) {
            MathLib::bigint sourceMin{}, sourceMax{};
            if (!getMinMaxValues(source, settings.platform, sourceMin, sourceMax) || range->intvalue < sourceMin ||
                range->intvalue > sourceMax)
                return false;
            return sourceBits <= destinationBits;
        }
        return source->sign == ValueType::Sign::UNSIGNED && destination->sign == ValueType::Sign::SIGNED &&
               sourceBits < destinationBits;
    }


    bool isUnsignedArithmeticResult(const Token* tok)
    {
        if (!Token::Match(tok, "+|-|*"))
            return false;
        const ValueType* vt = tok->valueType();
        return vt && vt->isIntegral() && vt->pointer == 0 && vt->sign == ValueType::Sign::UNSIGNED &&
               (vt->type == ValueType::Type::INT ||
                vt->type == ValueType::Type::LONG ||
                vt->type == ValueType::Type::LONGLONG ||
                vt->type == ValueType::Type::UNKNOWN_INT);
    }

    namespace {
        struct UnsignedArithmeticRange {
            MathLib::biguint lower{};
            MathLib::biguint upper{};
            bool hasUpper{};
            const Token* symbolicUpper{};
            MathLib::bigint symbolicUpperOffset{};
            bool symbolicUpperConditional{};
        };

        bool getUnsignedTypeMaximum(const ValueType* valueType,
                                    std::size_t bits,
                                    MathLib::biguint& maximum)
        {
            if (valueType->type == ValueType::Type::BOOL) {
                maximum = 1;
                return true;
            }
            const std::size_t carrierBits = std::numeric_limits<MathLib::biguint>::digits;
            if (valueType->sign != ValueType::Sign::UNSIGNED || bits == 0 || bits > carrierBits)
                return false;
            maximum = bits == carrierBits ? std::numeric_limits<MathLib::biguint>::max()
                                          : (MathLib::biguint{1} << bits) - 1;
            return true;
        }

        MathLib::biguint truncateUnsignedValue(MathLib::biguint value, std::size_t bits)
        {
            const std::size_t carrierBits = std::numeric_limits<MathLib::biguint>::digits;
            if (bits < carrierBits)
                value &= (MathLib::biguint{1} << bits) - 1;
            return value;
        }

        MathLib::biguint convertToUnsigned(MathLib::bigint value, std::size_t bits)
        {
            return truncateUnsignedValue(static_cast<MathLib::biguint>(value), bits);
        }

        const Token* statementEnd(const Token* start)
        {
            if (!start)
                return nullptr;
            if (start->str() == "{")
                return start->link();
            if (Token::Match(start, "if|for|while|switch|do|try|catch"))
                return nullptr;
            for (const Token* tok = start; tok; tok = tok->next()) {
                if (Token::Match(tok, "(|[|{")) {
                    if (!tok->link())
                        return nullptr;
                    tok = tok->link();
                    continue;
                }
                if (tok->str() == ";")
                    return tok;
                if (tok->str() == "}")
                    return nullptr;
            }
            return nullptr;
        }

        bool containsToken(const Token* start, const Token* end, const Token* target)
        {
            return start && end && target && start->index() <= target->index() && target->index() <= end->index();
        }

        bool followsToken(const Token* end, const Token* target)
        {
            return end && target && end->index() < target->index();
        }

        bool exitsUnconditionally(const Token* start)
        {
            if (Token::simpleMatch(start, "{"))
                start = start->next();
            while (start && start->str() != "}") {
                if (Token::Match(start, "return|throw|break|continue"))
                    return true;
                if (start->str() == "goto" || start->str() == "case" || start->str() == "default" ||
                    start->str() == "{" || Token::Match(start, "%name% :"))
                    return false;
                const Token* const end = statementEnd(start);
                if (!end || end->str() != ";")
                    return false;
                start = end->next();
            }
            return false;
        }

        const Scope* enclosingFunction(const Scope* scope)
        {
            while (scope && scope->type != ScopeType::eFunction && scope->type != ScopeType::eLambda)
                scope = scope->nestedIn;
            return scope;
        }
        bool isInScope(const Scope* scope, const Scope* ancestor)
        {
            for (; scope; scope = scope->nestedIn) {
                if (scope == ancestor)
                    return true;
            }
            return false;
        }

        bool conditionBranchAtUse(const Token* conditionTop, const Token* use, bool& branchIsTrue)
        {
            if (!conditionTop || !use)
                return false;
            const Scope* const functionScope = enclosingFunction(conditionTop->scope());
            if (!functionScope || functionScope != enclosingFunction(use->scope()))
                return false;
            const Token* control = conditionTop->previous();
            if (!Token::Match(control, "if|while|for ("))
                return false;
            const Token* bodyStart = conditionTop->link() ? conditionTop->link()->next() : nullptr;
            const Token* bodyEnd = statementEnd(bodyStart);
            if (!bodyEnd)
                return false;
            if (containsToken(bodyStart, bodyEnd, use)) {
                branchIsTrue = true;
                return true;
            }
            if (control->str() != "if")
                return false;
            const auto useAfterIfIsDominated = [&]() {
                for (const Scope* scope = bodyStart->scope(); scope; scope = scope->nestedIn) {
                    if (scope->type == ScopeType::eIf && scope->classDef == control)
                        return isInScope(use->scope(), scope->nestedIn);
                }
                return false;
            };

            const Token* elseToken = bodyEnd->next();
            if (Token::simpleMatch(elseToken, "else")) {
                const Token* elseStart = elseToken->next();
                const Token* elseEnd = statementEnd(elseStart);
                if (!elseEnd)
                    return false;
                if (containsToken(elseStart, elseEnd, use)) {
                    branchIsTrue = false;
                    return true;
                }
                if (!followsToken(elseEnd, use) || !useAfterIfIsDominated())
                    return false;
                const bool thenExits = exitsUnconditionally(bodyStart);
                const bool elseExits = exitsUnconditionally(elseStart);
                if (thenExits == elseExits)
                    return false;
                branchIsTrue = !thenExits;
                return true;
            }

            if (followsToken(bodyEnd, use) && exitsUnconditionally(bodyStart) && useAfterIfIsDominated()) {
                branchIsTrue = false;
                return true;
            }
            return false;
        }

        bool sameValueExpression(const Token* lhs, const Token* rhs)
        {
            if (!lhs || !rhs)
                return false;
            if (lhs == rhs || (lhs->exprId() != 0 && lhs->exprId() == rhs->exprId()))
                return true;
            return lhs->varId() != 0 && lhs->varId() == rhs->varId();
        }

        bool isConditionFactActiveAtUse(const Value& value, const Token* valueTok, const Token* use)
        {
            if (!value.condition)
                return true;
            if (value.path > 0)
                return true;
            if (value.bound == Value::Bound::Point || !Token::Match(value.condition, "<|<=|>|>="))
                return false;

            const Token* const condition = value.condition;
            const bool valueIsLhs = sameValueExpression(condition->astOperand1(), valueTok);
            const bool valueIsRhs = sameValueExpression(condition->astOperand2(), valueTok);
            if (valueIsLhs == valueIsRhs)
                return false;
            const bool conditionTrueMeansGreater = valueIsLhs ? Token::Match(condition, ">|>=")
                                                              : Token::Match(condition, "<|<=");
            const bool factMeansGreater = (value.bound == Value::Bound::Lower) != value.isImpossible();
            bool requiredConditionValue = factMeansGreater == conditionTrueMeansGreater;

            const Token* const conditionTop = condition->astTop();
            const Token* const conditionRoot = conditionTop ? conditionTop->astOperand2() : nullptr;
            if (!conditionTop || !conditionRoot)
                return false;
            bool branchIsTrue = false;
            if (!conditionBranchAtUse(conditionTop, use, branchIsTrue))
                return false;

            for (const Token* node = condition; node != conditionRoot;) {
                const Token* const parent = node->astParent();
                if (!parent)
                    return false;
                if (parent->str() == "(") {
                    node = parent;
                    continue;
                }
                if (parent->str() == "!") {
                    requiredConditionValue = !requiredConditionValue;
                } else if (Token::simpleMatch(parent, "&&")) {
                    if (!requiredConditionValue)
                        return false;
                    requiredConditionValue = true;
                } else if (Token::simpleMatch(parent, "||")) {
                    if (requiredConditionValue)
                        return false;
                    requiredConditionValue = false;
                } else if (!(parent->isCast() && astIsBool(parent))) {
                    return false;
                }
                node = parent;
            }
            return branchIsTrue == requiredConditionValue;
        }

        bool getUnsignedArithmeticRange(const Token* tok,
                                        MathLib::bigint path,
                                        const ValueType* resultType,
                                        MathLib::biguint resultMax,
                                        const Settings& settings,
                                        UnsignedArithmeticRange& range,
                                        const Token* useTok,
                                        bool contextual,
                                        bool includeSymbolic,
                                        int depth = 8)
        {
            if (depth < 0)
                return false;
            const ValueType* const valueType = tok ? tok->valueType() : nullptr;
            if (!valueType || !valueType->isIntegral() || valueType->pointer)
                return false;

            const std::size_t valueSize = valueType->getSizeOf(settings,
                                                               ValueType::Accuracy::ExactOrZero,
                                                               ValueType::SizeOf::Pointer);
            const std::size_t valueBits = valueSize * settings.platform.char_bit;
            const std::size_t resultSize = resultType->getSizeOf(settings,
                                                                 ValueType::Accuracy::ExactOrZero,
                                                                 ValueType::SizeOf::Pointer);
            const std::size_t resultBits = resultSize * settings.platform.char_bit;
            if (valueBits == 0 || valueBits > 64 || resultBits == 0 || resultBits > 64)
                return false;

            const bool unsignedValue = valueType->sign == ValueType::Sign::UNSIGNED ||
                                       valueType->type == ValueType::Type::BOOL;
            MathLib::bigint signedLower{};
            MathLib::bigint signedUpper{};
            if (unsignedValue) {
                if (!getUnsignedTypeMaximum(valueType, valueBits, range.upper))
                    return false;
                range.lower = 0;
                range.hasUpper = true;
            } else {
                if (valueType->sign != ValueType::Sign::SIGNED ||
                    !getMinMaxValues(valueType, settings.platform, signedLower, signedUpper))
                    return false;
            }

            const Token* symbolicSource = nullptr;
            bool symbolicSourceConflict = false;
            bool symbolicUpperConflict = false;
            bool hasKnownPoint = false;
            MathLib::bigint knownSignedPoint{};
            MathLib::biguint knownUnsignedPoint{};
            for (const Value& value : tok->values()) {
                if (value.path < 0)
                    return false;
                if (value.path != 0 && value.path != path)
                    continue;
                if (!contextual && (value.condition || value.conditional))
                    continue;
                if (contextual && value.path == 0 && value.condition &&
                    !isConditionFactActiveAtUse(value, tok, useTok))
                    continue;
                if (contextual && value.conditional && !value.isSymbolicValue())
                    continue;
                if (value.isSymbolicValue() && !includeSymbolic)
                    continue;
                if (value.isSymbolicValue()) {
                    if (value.isImpossible())
                        continue;
                    const bool isConditionalUpper = value.isPossible() && value.conditional &&
                                                    value.bound == Value::Bound::Upper;
                    if (!value.tokvalue || value.tokvalue == tok)
                        continue;
                    if (value.bound == Value::Bound::Point && value.isKnown() && value.intvalue == 0) {
                        if (symbolicSource && symbolicSource != value.tokvalue) {
                            symbolicSource = nullptr;
                            symbolicSourceConflict = true;
                        } else if (!symbolicSourceConflict) {
                            symbolicSource = value.tokvalue;
                        }
                    } else if (isConditionalUpper && !symbolicUpperConflict) {
                        if (range.symbolicUpper &&
                            (range.symbolicUpper != value.tokvalue || range.symbolicUpperOffset != value.intvalue)) {
                            range.symbolicUpper = nullptr;
                            range.symbolicUpperConditional = false;
                            symbolicUpperConflict = true;
                        } else {
                            range.symbolicUpperConditional = range.symbolicUpper
                                ? range.symbolicUpperConditional && value.conditional
                                : value.conditional;
                            range.symbolicUpper = value.tokvalue;
                            range.symbolicUpperOffset = value.intvalue;
                        }
                    }
                    continue;
                }

                if (!value.isIntValue() || value.isInconclusive())
                    continue;
                if (value.bound == Value::Bound::Point) {
                    if (!value.isKnown())
                        continue;
                    if (unsignedValue) {
                        const MathLib::biguint point = truncateUnsignedValue(static_cast<MathLib::biguint>(value.intvalue), valueBits);
                        if (hasKnownPoint && point != knownUnsignedPoint)
                            return false;
                        knownUnsignedPoint = point;
                    } else {
                        if (hasKnownPoint && value.intvalue != knownSignedPoint)
                            return false;
                        knownSignedPoint = value.intvalue;
                    }
                    hasKnownPoint = true;
                    continue;
                }

                if (unsignedValue) {
                    if (value.intvalue < 0) {
                        if (value.isImpossible()) {
                            if (value.bound == Value::Bound::Lower)
                                return false;
                        } else if (value.bound == Value::Bound::Upper) {
                            return false;
                        }
                        continue;
                    }
                    const auto endpoint = static_cast<MathLib::biguint>(value.intvalue);
                    if (value.isImpossible()) {
                        if (value.bound == Value::Bound::Upper) {
                            if (endpoint == std::numeric_limits<MathLib::biguint>::max())
                                return false;
                            range.lower = std::max(range.lower, endpoint + 1);
                        } else if (value.bound == Value::Bound::Lower) {
                            if (endpoint == 0)
                                return false;
                            range.upper = std::min(range.upper, endpoint - 1);
                        }
                    } else if (value.bound == Value::Bound::Lower) {
                        range.lower = std::max(range.lower, endpoint);
                    } else if (value.bound == Value::Bound::Upper) {
                        range.upper = std::min(range.upper, endpoint);
                    }
                } else if (value.isImpossible()) {
                    if (value.bound == Value::Bound::Upper) {
                        if (value.intvalue == std::numeric_limits<MathLib::bigint>::max())
                            return false;
                        signedLower = std::max(signedLower, value.intvalue + 1);
                    } else if (value.bound == Value::Bound::Lower) {
                        if (value.intvalue == std::numeric_limits<MathLib::bigint>::min())
                            return false;
                        signedUpper = std::min(signedUpper, value.intvalue - 1);
                    }
                } else if (value.bound == Value::Bound::Lower) {
                    signedLower = std::max(signedLower, value.intvalue);
                } else if (value.bound == Value::Bound::Upper) {
                    signedUpper = std::min(signedUpper, value.intvalue);
                }
            }

            if (unsignedValue) {
                if (range.lower > range.upper)
                    return false;
                if (hasKnownPoint) {
                    if (knownUnsignedPoint < range.lower || knownUnsignedPoint > range.upper)
                        return false;
                    range.lower = truncateUnsignedValue(knownUnsignedPoint, resultBits);
                    range.upper = range.lower;
                }
            } else {
                if (signedLower > signedUpper)
                    return false;
                if (hasKnownPoint) {
                    if (knownSignedPoint < signedLower || knownSignedPoint > signedUpper)
                        return false;
                    range.lower = convertToUnsigned(knownSignedPoint, resultBits);
                    range.upper = range.lower;
                    range.hasUpper = true;
                } else {
                    if (signedLower < 0)
                        return false;
                    range.lower = static_cast<MathLib::biguint>(signedLower);
                    range.upper = static_cast<MathLib::biguint>(signedUpper);
                    range.hasUpper = true;
                }
            }

            if (symbolicSource && !hasKnownPoint) {
                const ValueType* const sourceType = symbolicSource->valueType();
                if (sourceType && sourceType->isIntegral() && !sourceType->pointer &&
                    sourceType->sign == valueType->sign) {
                    const std::size_t sourceSize = sourceType->getSizeOf(settings,
                                                                         ValueType::Accuracy::ExactOrZero,
                                                                         ValueType::SizeOf::Pointer);
                    if (sourceSize > 0 && sourceSize <= valueSize) {
                        UnsignedArithmeticRange sourceRange;
                        if (getUnsignedArithmeticRange(symbolicSource,
                                                       path,
                                                       resultType,
                                                       resultMax,
                                                       settings,
                                                       sourceRange,
                                                       useTok,
                                                       contextual,
                                                       includeSymbolic,
                                                       depth - 1)) {
                            range.lower = std::max(range.lower, sourceRange.lower);
                            range.upper = std::min(range.upper, sourceRange.upper);
                            if (range.upper < range.lower)
                                return false;
                            if (sourceRange.symbolicUpper && !symbolicUpperConflict) {
                                if (range.symbolicUpper &&
                                    (range.symbolicUpper != sourceRange.symbolicUpper ||
                                     range.symbolicUpperOffset != sourceRange.symbolicUpperOffset)) {
                                    range.symbolicUpper = nullptr;
                                    range.symbolicUpperConditional = false;
                                } else {
                                    range.symbolicUpperConditional = range.symbolicUpper
                                        ? range.symbolicUpperConditional && sourceRange.symbolicUpperConditional
                                        : sourceRange.symbolicUpperConditional;
                                    range.symbolicUpper = sourceRange.symbolicUpper;
                                    range.symbolicUpperOffset = sourceRange.symbolicUpperOffset;
                                }
                            }
                        }
                    }
                }
            }

            return range.lower <= range.upper && range.upper <= resultMax;
        }
    }
    bool convertValueRange(Value& value,
                           const ValueType* source,
                           const ValueType* destination,
                           const Settings& settings,
                           const Token* sourceTok,
                           const Token* useTok)
    {
        if (!value.isIntValue() || value.bound == Value::Bound::Point || value.isInconclusive() ||
            !destination || !destination->isIntegral() || destination->pointer)
            return false;

        if (!source) {
            if (!sourceTok || !useTok || sourceTok->valueType() || value.path < 0 || value.conditional ||
                (value.condition && value.path == 0 && !isConditionFactActiveAtUse(value, sourceTok, useTok)))
                return false;
            const Variable* const sourceVariable = sourceTok->variable();
            if (sourceVariable && sourceVariable->isPointer())
                return false;

            MathLib::bigint lower{};
            MathLib::bigint upper{};
            bool hasLower = false;
            bool hasUpper = false;
            const auto isCompatible = [&](const Value& fact) {
                if (fact.conditional || (fact.path > 0 && (value.path == 0 || fact.path != value.path)))
                    return false;
                return fact.path != 0 || !fact.condition || isConditionFactActiveAtUse(fact, sourceTok, useTok);
            };
            const auto updateBounds = [&](const Value& fact) {
                if (fact.bound == Value::Bound::Point || !fact.isImpossible())
                    return true;
                // Only excluded ranges prove universal bounds without a known source type.
                // Possible bounds can be witnesses from different merged paths.
                if (fact.bound == Value::Bound::Upper) {
                    if (fact.intvalue == std::numeric_limits<MathLib::bigint>::max())
                        return false;
                    const MathLib::bigint minimum = fact.intvalue + 1;
                    lower = hasLower ? std::max(lower, minimum) : minimum;
                    hasLower = true;
                } else {
                    if (fact.intvalue == std::numeric_limits<MathLib::bigint>::min())
                        return false;
                    const MathLib::bigint maximum = fact.intvalue - 1;
                    upper = hasUpper ? std::min(upper, maximum) : maximum;
                    hasUpper = true;
                }
                return !hasLower || !hasUpper || lower <= upper;
            };
            if (!std::all_of(sourceTok->values().cbegin(), sourceTok->values().cend(), [&](const Value& fact) {
                return fact.path >= 0 &&
                       (!isCompatible(fact) || (fact.isIntValue() && !fact.isInconclusive() && updateBounds(fact)));
            }))
                return false;
            if (!hasLower || !hasUpper || lower > upper)
                return false;

            MathLib::bigint destinationMinimum{};
            MathLib::bigint destinationMaximum{};
            if (!getMinMaxValues(destination, settings.platform, destinationMinimum, destinationMaximum) ||
                lower < destinationMinimum || upper > destinationMaximum)
                return false;

            // Possible bounds can reject contradictions but must not tighten this proof interval.
            if (!std::all_of(sourceTok->values().cbegin(), sourceTok->values().cend(), [&](const Value& fact) {
                if (!isCompatible(fact) || fact.isImpossible())
                    return true;
                if (fact.bound == Value::Bound::Point)
                    return !(fact.isPossible() || fact.isKnown()) ||
                           (fact.intvalue >= lower && fact.intvalue <= upper);
                return !((fact.bound == Value::Bound::Lower && fact.intvalue > upper) ||
                         (fact.bound == Value::Bound::Upper && fact.intvalue < lower));
            }))
                return false;
            return true;
        }

        if (!source->isIntegral() || source->pointer)
            return false;
        if (isValueRangePreservingConversion(source, destination, settings, &value))
            return true;
        if ((!value.isPossible() && !value.isKnown()) || value.isImpossible() ||
            destination->sign != ValueType::Sign::UNSIGNED || destination->type == ValueType::Type::BOOL ||
            (source->sign != ValueType::Sign::SIGNED && source->sign != ValueType::Sign::UNSIGNED))
            return false;

        const std::size_t destinationBits = destination->getSizeOf(settings,
                                                                   ValueType::Accuracy::ExactOrZero,
                                                                   ValueType::SizeOf::Pointee) * settings.platform.char_bit;
        const std::size_t carrierBits = std::numeric_limits<MathLib::biguint>::digits;
        if (destinationBits == 0 || destinationBits > carrierBits)
            return false;
        if (destinationBits == carrierBits) {
            if (source->sign != ValueType::Sign::SIGNED)
                return false;
            MathLib::bigint sourceMinimum{};
            MathLib::bigint sourceMaximum{};
            if (!getMinMaxValues(source, settings.platform, sourceMinimum, sourceMaximum))
                return false;
            MathLib::bigint lower = sourceMinimum;
            MathLib::bigint upper = sourceMaximum;
            if (value.bound == Value::Bound::Lower)
                lower = std::max(lower, value.intvalue);
            else
                upper = std::min(upper, value.intvalue);
            if (lower > upper || upper >= 0)
                return false;
            value.intvalue = std::numeric_limits<MathLib::bigint>::max();
            value.bound = Value::Bound::Lower;
            return true;
        }
        const MathLib::biguint destinationMaximum = (MathLib::biguint{1} << destinationBits) - 1;

        MathLib::biguint imageMinimum{};
        MathLib::biguint imageMaximum{};
        if (source->sign == ValueType::Sign::SIGNED) {
            MathLib::bigint sourceMinimum{};
            MathLib::bigint sourceMaximum{};
            if (!getMinMaxValues(source, settings.platform, sourceMinimum, sourceMaximum))
                return false;
            MathLib::bigint lower = sourceMinimum;
            MathLib::bigint upper = sourceMaximum;
            if (value.bound == Value::Bound::Lower)
                lower = std::max(lower, value.intvalue);
            else
                upper = std::min(upper, value.intvalue);
            if (lower > upper)
                return false;
            const MathLib::biguint distance = static_cast<MathLib::biguint>(upper) -
                                              static_cast<MathLib::biguint>(lower);
            if (distance >= destinationMaximum)
                return false;
            imageMinimum = static_cast<MathLib::biguint>(lower) & destinationMaximum;
            imageMaximum = static_cast<MathLib::biguint>(upper) & destinationMaximum;
        } else {
            const std::size_t sourceBits = source->getSizeOf(settings,
                                                             ValueType::Accuracy::ExactOrZero,
                                                             ValueType::SizeOf::Pointee) * settings.platform.char_bit;
            MathLib::biguint sourceMaximum{};
            if (!getUnsignedTypeMaximum(source, sourceBits, sourceMaximum) || value.intvalue < 0)
                return false;
            MathLib::biguint lower = 0;
            MathLib::biguint upper = sourceMaximum;
            if (value.bound == Value::Bound::Lower)
                lower = std::max(lower, static_cast<MathLib::biguint>(value.intvalue));
            else
                upper = std::min(upper, static_cast<MathLib::biguint>(value.intvalue));
            if (lower > upper)
                return false;
            const MathLib::biguint distance = upper - lower;
            if (distance >= destinationMaximum)
                return false;
            imageMinimum = lower & destinationMaximum;
            imageMaximum = upper & destinationMaximum;
        }
        if (imageMinimum > imageMaximum)
            return false;

        if (imageMinimum > 0) {
            if (imageMinimum > static_cast<MathLib::biguint>(std::numeric_limits<MathLib::bigint>::max()))
                return false;
            value.intvalue = static_cast<MathLib::bigint>(imageMinimum);
            value.bound = Value::Bound::Lower;
            return true;
        }
        if (imageMaximum < destinationMaximum) {
            value.intvalue = static_cast<MathLib::bigint>(imageMaximum);
            value.bound = Value::Bound::Upper;
            return true;
        }
        return false;
    }

    bool isUnsignedArithmeticNoWrap(const Token* tok, const Settings& settings, bool requireGlobalContext)
    {
        if (!isUnsignedArithmeticResult(tok) || !tok->astOperand1() || !tok->astOperand2())
            return false;

        const ValueType* const resultType = tok->valueType();
        const std::size_t resultSize = resultType->getSizeOf(settings,
                                                             ValueType::Accuracy::ExactOrZero,
                                                             ValueType::SizeOf::Pointer);
        const std::size_t resultBits = resultSize * settings.platform.char_bit;
        if (resultBits == 0 || resultBits > 64)
            return false;
        MathLib::biguint resultMax{};
        if (!getUnsignedTypeMaximum(resultType, resultBits, resultMax))
            return false;

        std::vector<MathLib::bigint> paths;
        const Token* const lhs = tok->astOperand1();
        const Token* const rhs = tok->astOperand2();
        const auto hasPath = [](const Token* operand, MathLib::bigint path) {
            if (operand->values().empty())
                return true;
            return std::any_of(operand->values().cbegin(), operand->values().cend(), [path](const Value& value) {
                return value.path == 0 || value.path == path;
            });
        };
        const auto addPath = [&](MathLib::bigint path) {
            if (std::find(paths.cbegin(), paths.cend(), path) == paths.cend())
                paths.push_back(path);
        };
        const auto hasUnconditionalPath = [](const Token* operand) {
            return operand->values().empty() ||
                   std::any_of(operand->values().cbegin(), operand->values().cend(), [](const Value& value) {
                return value.path == 0;
            });
        };
        const auto hasOnlyNonnegativePaths = [](const Token* operand) {
            return std::all_of(operand->values().cbegin(), operand->values().cend(), [](const Value& value) {
                return value.path >= 0;
            });
        };
        if (!hasOnlyNonnegativePaths(lhs) || !hasOnlyNonnegativePaths(rhs))
            return false;
        if (requireGlobalContext) {
            addPath(0);
        } else {
            if (hasUnconditionalPath(lhs) && hasUnconditionalPath(rhs))
                addPath(0);
            for (const Value& value : lhs->values()) {
                if (value.path > 0 && hasPath(rhs, value.path))
                    addPath(value.path);
            }
            for (const Value& value : rhs->values()) {
                if (value.path > 0 && hasPath(lhs, value.path))
                    addPath(value.path);
            }
        }
        if (paths.empty())
            return false;

        const bool contextual = !requireGlobalContext;
        return std::all_of(paths.cbegin(), paths.cend(), [&](MathLib::bigint path) {
            UnsignedArithmeticRange lhsRange;
            UnsignedArithmeticRange rhsRange;
            if (tok->str() == "-") {
                UnsignedArithmeticRange rhsPoint;
                if (getUnsignedArithmeticRange(rhs, path, resultType, resultMax, settings,
                                               rhsPoint, tok, contextual, false) &&
                    rhsPoint.hasUpper && rhsPoint.lower == rhsPoint.upper) {
                    if (rhsPoint.upper == 0)
                        return true;
                    if (getUnsignedArithmeticRange(lhs, path, resultType, resultMax, settings,
                                                   lhsRange, tok, contextual, false) &&
                        lhsRange.lower >= rhsPoint.upper)
                        return true;
                }
            }
            if (tok->str() == "+") {
                const auto isZeroOperand = [&](const Token* operand) {
                    UnsignedArithmeticRange zeroRange;
                    return getUnsignedArithmeticRange(operand, path, resultType, resultMax, settings,
                                                      zeroRange, tok, contextual, false) &&
                           zeroRange.hasUpper && zeroRange.lower == 0 && zeroRange.upper == 0;
                };
                if (isZeroOperand(lhs) || isZeroOperand(rhs))
                    return true;
            }
            if (!getUnsignedArithmeticRange(lhs, path, resultType, resultMax, settings, lhsRange, tok, contextual, true) ||
                !getUnsignedArithmeticRange(rhs, path, resultType, resultMax, settings, rhsRange, tok, contextual, true))
                return false;

            if (tok->str() == "-") {
                if (!rhsRange.hasUpper || lhsRange.lower < rhsRange.upper)
                    return false;
            } else if (tok->str() == "+") {
                const auto isSafeRelativeAddition = [&](const UnsignedArithmeticRange& boundedRange,
                                                        const UnsignedArithmeticRange& addendRange,
                                                        const Token* boundedOperand) {
                    if (!boundedRange.symbolicUpper || !boundedRange.symbolicUpperConditional ||
                        !addendRange.hasUpper || !boundedOperand || !boundedOperand->variable() ||
                        !boundedRange.symbolicUpper->variable() ||
                        addendRange.upper > static_cast<MathLib::biguint>(std::numeric_limits<MathLib::bigint>::max()))
                        return false;
                    const auto addendUpper = static_cast<MathLib::bigint>(addendRange.upper);
                    if (boundedRange.symbolicUpperOffset > -addendUpper)
                        return false;
                    const ValueType* const upperType = boundedRange.symbolicUpper->valueType();
                    if (!upperType || !upperType->isIntegral() || upperType->pointer ||
                        upperType->sign != ValueType::Sign::UNSIGNED)
                        return false;
                    const std::size_t upperSize = upperType->getSizeOf(settings,
                                                                       ValueType::Accuracy::ExactOrZero,
                                                                       ValueType::SizeOf::Pointer);
                    if (upperSize == 0 || upperSize * settings.platform.char_bit > resultBits)
                        return false;
                    for (const Scope* loop = boundedOperand->scope(); loop; loop = loop->nestedIn) {
                        if (loop->type != ScopeType::eFor || !loop->classDef)
                            continue;
                        const Token* const forExpressions = loop->classDef->next();
                        if (!forExpressions || !forExpressions->astOperand2())
                            continue;
                        const Token* const clauses = forExpressions->astOperand2();
                        const Token* const init = clauses->astOperand1();
                        const Token* const remainingClauses = clauses->astOperand2();
                        if (!init || !remainingClauses || !remainingClauses->astOperand1() ||
                            !remainingClauses->astOperand2())
                            continue;
                        const Token* const condition = remainingClauses->astOperand1();
                        const Token* const step = remainingClauses->astOperand2();
                        if (!Token::simpleMatch(condition, "<") || !condition->astOperand1() ||
                            !condition->astOperand2() || condition->astOperand1()->varId() != boundedOperand->varId() ||
                            condition->astOperand2()->varId() != boundedRange.symbolicUpper->varId() ||
                            !Token::simpleMatch(init, "=") || !init->astOperand1() ||
                            init->astOperand1()->varId() != boundedOperand->varId() || !Token::simpleMatch(step, "++"))
                            continue;
                        const Token* stepVariable = step->astOperand1();
                        if (!stepVariable)
                            stepVariable = step->astOperand2();
                        if (stepVariable && stepVariable->varId() == boundedOperand->varId())
                            return true;
                    }
                    return false;
                };
                const bool numericSafe = lhsRange.hasUpper && rhsRange.hasUpper &&
                                         lhsRange.upper <= resultMax - rhsRange.upper;
                if (!numericSafe && !isSafeRelativeAddition(lhsRange, rhsRange, lhs) &&
                    !isSafeRelativeAddition(rhsRange, lhsRange, rhs))
                    return false;
            } else if (tok->str() == "*") {
                if (!lhsRange.hasUpper || !rhsRange.hasUpper ||
                    (lhsRange.upper != 0 && rhsRange.upper > resultMax / lhsRange.upper))
                    return false;
            }
            return true;
        });
    }

    MathLib::bigint truncateIntValue(MathLib::bigint value, size_t value_size, const ValueType* dst, const Platform& platform)
    {
        if (value_size == 0)
            return value;

        const MathLib::biguint unsignedMaxValue = std::numeric_limits<MathLib::biguint>::max() >> ((sizeof(unsignedMaxValue) - value_size) * 8);
        const MathLib::biguint signBit = 1ULL << ((value_size * 8) - 1);
        value &= unsignedMaxValue;
        if ((dst->sign == ValueType::Sign::SIGNED || (dst->sign == ValueType::Sign::UNKNOWN_SIGN && dst->type == ValueType::Type::CHAR && platform.defaultSign == 's')) && (value & signBit))
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
            // A long double literal out of the range of double (1e400L) has no
            // double value: infinity would make different literals compare equal.
            if (!MathLib::isLongDoubleOutOfDoubleRange(tok->str())) {
                Value value;
                value.valueType = Value::ValueType::FLOAT;
                value.floatValue = MathLib::toDoubleNumber(tok);
                if (!tok->isTemplateArg())
                    value.setKnown();
                setTokenValue(tok, std::move(value), settings);
            }
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
            const Token* const obj = tok->next()->astOperand2();
            if (obj && !obj->isLiteral() && obj->valueType() &&
                (obj->valueType()->pointer == 0 || // <- TODO this is a bailout, abort when there are array->pointer conversions
                 (obj->variable() && !obj->variable()->isArray())) &&
                !obj->valueType()->isEnum() && // <- TODO this is a bailout, handle enum with non-int types
                !(obj->valueType()->container && obj->valueType()->container->startPattern == "std :: array <")) {
                const auto ptrPointee = obj->valueType()->pointer > 0 ? ValueType::SizeOf::Pointer : ValueType::SizeOf::Pointee;
                const size_t sz = obj->valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ptrPointee);
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
                        if (var->valueType()->container && var->valueType()->container->startPattern == "std :: array <") {
                            const ValueType vtElement = ValueType::parseDecl(var->valueType()->containerTypeToken, settings);
                            size = vtElement.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
                        }
                        else
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

    bool castValue(Value& value, const ValueType::Sign sign, nonneg int bit)
    {
        if (bit <= 0 || sign == ValueType::Sign::UNKNOWN_SIGN)
            return false;

        if (value.isFloatValue()) {
            if (value.bound != Value::Bound::Point) {
                if (value.isImpossible())
                    return false;
                MathLib::bigint converted{};
                if (!convertFloatToInteger(converted, value.floatValue, sign, bit))
                    return false;
                value.intvalue = converted;
                value.valueType = Value::ValueType::INT;
                return true;
            }
            if (value.isImpossible())
                return false;
            MathLib::bigint converted{};
            if (!convertFloatToInteger(converted, value.floatValue, sign, bit))
                return false;
            value.intvalue = converted;
            value.valueType = Value::ValueType::INT;
            return true;
        }

        if (value.bound != Value::Bound::Point || (value.isImpossible() && !value.isIntValue()))
            return false;

        if (!value.isIntValue())
            return false;
        if (bit < MathLib::bigint_bits) {
            const MathLib::biguint mask = (MathLib::biguint{1} << bit) - 1;
            const MathLib::biguint raw = static_cast<MathLib::biguint>(value.intvalue) & mask;
            if (sign == ValueType::Sign::SIGNED && (raw & (MathLib::biguint{1} << (bit - 1))))
                value.intvalue = -1 - static_cast<MathLib::bigint>((~raw) & mask);
            else
                value.intvalue = static_cast<MathLib::bigint>(raw);
        } else if (sign == ValueType::Sign::UNSIGNED && value.intvalue < 0) {
            return false;
        }
        return true;
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

    MathLib::bigint valueFlowGetStrLength(const Token* tok, const Library& library)
    {
        if (tok->tokType() == Token::eString)
            return Token::getStrLength(tok);
        if (astIsGenericChar(tok) || tok->tokType() == Token::eChar)
            return 1;
        if (const Value* v = tok->getKnownValue(Value::ValueType::CONTAINER_SIZE)) {
            // on a pointer the size is the number of elements in the buffer (possibly including
            // a null terminator), not the length of the string
            if (!astIsPointer(tok))
                return v->intvalue;
        }
        if (const Value* v = tok->getKnownValue(Value::ValueType::TOK)) {
            if (v->tokvalue != tok)
                return valueFlowGetStrLength(v->tokvalue, library);
        }
        if (const Token* cont = library.getContainerFromYield(tok, Library::Container::Yield::BUFFER_NT))
            return valueFlowGetStrLength(cont, library);
        return 0;
    }
}
