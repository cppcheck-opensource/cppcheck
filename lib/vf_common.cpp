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
#include "infer.h"
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
    ValueType::Sign resolveCharSign(const ValueType& type, const Platform& platform)
    {
        if (type.type != ValueType::Type::CHAR || type.sign != ValueType::Sign::UNKNOWN_SIGN)
            return type.sign;
        if (platform.defaultSign == 's' || platform.defaultSign == 'S')
            return ValueType::Sign::SIGNED;
        if (platform.defaultSign == 'u' || platform.defaultSign == 'U')
            return ValueType::Sign::UNSIGNED;
        return ValueType::Sign::UNKNOWN_SIGN;
    }

    std::size_t getTypeBits(const ValueType& type, const Settings& settings)
    {
        return type.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee) *
               settings.platform.char_bit;
    }

    MathLib::biguint getUnsignedMaximum(std::size_t bits)
    {
        const std::size_t carrierBits = std::numeric_limits<MathLib::biguint>::digits;
        if (bits >= carrierBits)
            return std::numeric_limits<MathLib::biguint>::max();
        return (MathLib::biguint{1} << bits) - 1;
    }

    MathLib::biguint convertToUnsigned(MathLib::bigint value, std::size_t bits)
    {
        auto converted = static_cast<MathLib::biguint>(value);
        if (bits < std::numeric_limits<MathLib::biguint>::digits)
            converted &= (MathLib::biguint{1} << bits) - 1;
        return converted;
    }

    // The sizes of the constant and operand of a mixed-sign comparison whose common type is unsigned,
    // i.e. the comparison converts a signed operand to unsigned.
    static bool getUnsignedDomainSizes(const Token* constantTok,
                                       const Token* operandTok,
                                       const Settings& settings,
                                       std::size_t& constantSize,
                                       std::size_t& operandSize)
    {
        if (!constantTok || !operandTok)
            return false;
        const ValueType* const constantType = constantTok->valueType();
        const ValueType* const operandType = operandTok->valueType();
        if (!constantType || !operandType)
            return false;
        if (!constantType->isIntegral() || constantType->pointer || !operandType->isIntegral() || operandType->pointer)
            return false;
        // bool operands are compared without the usual integral promotion rules
        if (constantType->type == ValueType::Type::BOOL || operandType->type == ValueType::Type::BOOL)
            return false;
        // Only an unsigned constant compared with a signed operand needs the other interpretation
        if (constantType->sign != ValueType::Sign::UNSIGNED)
            return false;
        if (resolveCharSign(*operandType, settings.platform) != ValueType::Sign::SIGNED)
            return false;
        // The common type of the comparison is the one with the higher rank, so only a constant of at
        // least the operand's rank is converted away from its own type.
        constantSize = constantType->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
        operandSize = operandType->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
        return constantSize != 0 && operandSize != 0 && constantSize >= operandSize;
    }

    bool comparesSignedOperandAsUnsigned(const Token* constantTok, const Token* operandTok, const Settings& settings)
    {
        std::size_t constantSize = 0;
        std::size_t operandSize = 0;
        return getUnsignedDomainSizes(constantTok, operandTok, settings, constantSize, operandSize);
    }

    MathLib::bigint castConstantToOperandType(const Token* constantTok,
                                              const Token* operandTok,
                                              const MathLib::bigint value,
                                              const Settings& settings)
    {
        std::size_t constantSize = 0;
        std::size_t operandSize = 0;
        if (!getUnsignedDomainSizes(constantTok, operandTok, settings, constantSize, operandSize))
            return value;
        const ValueType* const operandType = operandTok->valueType();
        const MathLib::bigint converted = truncateIntValue(value, operandSize, operandType, settings.platform);
        // The comparison happens in the common type, which is the constant's own type here. The
        // operand is sign-extended to that type before the conversion, so only a value of the
        // operand's type that converts back to the constant's value can equal the constant. A
        // 'short' sign-extends to 0xFFFFFFFF, so it never equals 0xFFFFu.
        MathLib::bigint roundTrip = converted;
        const std::size_t constantBits = constantSize * settings.platform.char_bit;
        if (constantBits < static_cast<std::size_t>(MathLib::bigint_bits)) {
            const MathLib::biguint mask = (MathLib::biguint{1} << constantBits) - 1;
            roundTrip = static_cast<MathLib::bigint>(static_cast<MathLib::biguint>(converted) & mask);
        }
        if (converted == value || roundTrip != value)
            return value;
        return converted;
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


    template<class T>
    struct Bounds {
        bool valid;   // false: the fact leaves no value of the type
        T lower;
        T upper;
    };

    template<class T>
    static Bounds<T> narrowBounds(Bounds<T> bounds, const Value& fact, T endpoint)
    {
        if (fact.isImpossible()) {
            if (fact.bound == Value::Bound::Upper) {
                if (endpoint == std::numeric_limits<T>::max())
                    return Bounds<T>{false, T{}, T{}};
                bounds.lower = std::max<T>(bounds.lower, endpoint + 1);
            } else if (fact.bound == Value::Bound::Lower) {
                if (endpoint == std::numeric_limits<T>::min())
                    return Bounds<T>{false, T{}, T{}};
                bounds.upper = std::min<T>(bounds.upper, endpoint - 1);
            }
        } else if (fact.bound == Value::Bound::Lower) {
            bounds.lower = std::max<T>(bounds.lower, endpoint);
        } else if (fact.bound == Value::Bound::Upper) {
            bounds.upper = std::min<T>(bounds.upper, endpoint);
        }
        return bounds;
    }

    bool isValueRangePreservingConversion(const ValueType* source,
                                          const ValueType* destination,
                                          const Settings& settings,
                                          const Value& value)
    {
        if (!destination || !destination->isIntegral() || destination->pointer)
            return false;
        const bool impossiblePoint = value.isIntValue() && value.isImpossible() && value.bound == Value::Bound::Point;
        if (!source)
            return impossiblePoint;
        const std::size_t destinationBits = getTypeBits(*destination, settings);
        if (source->pointer > 0) {
            // A non-null pointer remains nonzero in bool, and in an integer type large enough to hold a pointer.
            if (!impossiblePoint || value.intvalue != 0)
                return false;
            if (destination->type == ValueType::Type::BOOL)
                return true;
            const std::size_t sourceBits = source->getSizeOf(settings,
                                                             ValueType::Accuracy::ExactOrZero,
                                                             ValueType::SizeOf::Pointer) * settings.platform.char_bit;
            return sourceBits > 0 && destinationBits >= sourceBits;
        }
        if (source->isFloat()) {
            if (!value.isFloatValue() || value.isImpossible() ||
                value.bound == Value::Bound::Point || destination->type == ValueType::Type::BOOL)
                return false;
            MathLib::bigint converted{};
            return destinationBits > 0 && destinationBits <= static_cast<std::size_t>(std::numeric_limits<int>::max()) &&
                   convertFloatToInteger(converted, value.floatValue, destination->sign, static_cast<int>(destinationBits));
        }
        if (!source->isIntegral())
            return false;
        const std::size_t sourceBits = getTypeBits(*source, settings);
        if (source->isTypeEqual(destination) && source->sign == destination->sign)
            return true;

        if (value.isIntValue() && value.bound != Value::Bound::Point && !value.isInconclusive()) {
            MathLib::bigint sourceMin{}, sourceMax{}, destinationMin{}, destinationMax{};
            if (getMinMaxValues(source, settings.platform, sourceMin, sourceMax) &&
                getMinMaxValues(destination, settings.platform, destinationMin, destinationMax)) {
                const bool upperBoundedByFact = value.isImpossible() ? value.bound == Value::Bound::Lower
                                                                     : value.bound == Value::Bound::Upper;
                const Bounds<MathLib::bigint> bounds = narrowBounds(
                    Bounds<MathLib::bigint>{true, sourceMin, sourceMax},
                    value,
                    value.intvalue);
                if (!bounds.valid)
                    return false;

                const bool sourceHasRepresentableMaximum = source->sign != ValueType::Sign::UNSIGNED ||
                                                           sourceBits <= std::numeric_limits<MathLib::bigint>::digits;
                if (bounds.lower <= bounds.upper && (sourceHasRepresentableMaximum || upperBoundedByFact) &&
                    bounds.lower >= destinationMin && bounds.upper <= destinationMax)
                    return true;
            }
        }

        if (value.isIntValue() && value.bound == Value::Bound::Point &&
            destination->type != ValueType::Type::BOOL && destination->sign != ValueType::Sign::UNKNOWN_SIGN) {
            if (destinationBits == 0) {
                const ValueType smallestType(destination->sign, ValueType::Type::CHAR, 0);
                MathLib::bigint minimum{}, maximum{};
                return getMinMaxValues(&smallestType, settings.platform, minimum, maximum) &&
                       value.intvalue >= minimum && value.intvalue <= maximum;
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

        if (sourceBits == 0 || destinationBits == 0)
            return false;
        if (source->sign == ValueType::Sign::UNKNOWN_SIGN)
            return destination->sign == ValueType::Sign::SIGNED && sourceBits < destinationBits;

        if (source->sign == destination->sign)
            return sourceBits <= destinationBits;
        if (impossiblePoint && source->sign == ValueType::Sign::SIGNED &&
            destination->sign == ValueType::Sign::UNSIGNED) {
            MathLib::bigint sourceMin{}, sourceMax{};
            if (!getMinMaxValues(source, settings.platform, sourceMin, sourceMax) || value.intvalue < sourceMin ||
                value.intvalue > sourceMax)
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
            bool valid{};   // false: the expression has no proven range
            MathLib::biguint lower{};
            MathLib::biguint upper{};
            // conditional symbolic bound: value <= symbolicUpper + symbolicUpperOffset
            const Token* symbolicUpper{};
            MathLib::bigint symbolicUpperOffset{};
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
            maximum = getUnsignedMaximum(bits);
            return true;
        }

        MathLib::biguint truncateUnsignedValue(MathLib::biguint value, std::size_t bits)
        {
            const std::size_t carrierBits = std::numeric_limits<MathLib::biguint>::digits;
            if (bits < carrierBits)
                value &= (MathLib::biguint{1} << bits) - 1;
            return value;
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

        const Token* getIncrementedToken(const Token* step)
        {
            return step->astOperand1() ? step->astOperand1() : step->astOperand2();
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
        // Which branch of its controlling condition the use sits in. `valid` is false when neither
        // branch is proven, so `{}` is an invalid result.
        struct ConditionBranch {
            bool valid;       // false: the use is not dominated by either branch
            bool branchIsTrue;
        };

        ConditionBranch conditionBranchAtUse(const Token* conditionTop, const Token* use)
        {
            if (!conditionTop || !use)
                return {};
            const Scope* const functionScope = enclosingFunction(conditionTop->scope());
            if (!functionScope || functionScope != enclosingFunction(use->scope()))
                return {};
            const Token* control = conditionTop->previous();
            if (!Token::Match(control, "if|while|for ("))
                return {};
            const Token* bodyStart = conditionTop->link() ? conditionTop->link()->next() : nullptr;
            const Token* bodyEnd = statementEnd(bodyStart);
            if (!bodyEnd)
                return {};
            if (containsToken(bodyStart, bodyEnd, use))
                return ConditionBranch{true, true};
            if (control->str() != "if")
                return {};
            const auto useAfterIfIsDominated = [&]() {
                for (const Scope* scope = bodyStart->scope(); scope; scope = scope->nestedIn) {
                    if (scope->type == ScopeType::eIf && scope->classDef == control)
                        return use->scope()->isNestedIn(scope->nestedIn);
                }
                return false;
            };

            const Token* elseToken = bodyEnd->next();
            if (Token::simpleMatch(elseToken, "else")) {
                const Token* elseStart = elseToken->next();
                const Token* elseEnd = statementEnd(elseStart);
                if (!elseEnd)
                    return {};
                if (containsToken(elseStart, elseEnd, use))
                    return ConditionBranch{true, false};
                if (!precedes(elseEnd, use) || !useAfterIfIsDominated())
                    return {};
                const bool thenExits = exitsUnconditionally(bodyStart);
                const bool elseExits = exitsUnconditionally(elseStart);
                if (thenExits == elseExits)
                    return {};
                return ConditionBranch{true, !thenExits};
            }

            if (precedes(bodyEnd, use) && exitsUnconditionally(bodyStart) && useAfterIfIsDominated())
                return ConditionBranch{true, false};
            return {};
        }

        // A use inside the condition expression is dominated by the operands evaluated before it: the
        // left operand of `&&` was true, the left operand of `||` was false, and a branch of a `?:`
        // requires its condition to be true or false. When the fact's comparison sits in that already
        // evaluated part, `branchIsTrue` reports which value the condition had at the use.
        ConditionBranch conditionHoldsAtUse(const Token* condition, const Token* conditionTop, const Token* use)
        {
            if (!condition || !conditionTop || !use || !astHasToken(conditionTop, use))
                return {};
            for (const Token* node = use; node && node != conditionTop;) {
                const Token* const parent = node->astParent();
                if (!parent)
                    return {};
                const Token* evaluatedBefore = nullptr;
                bool evaluatedValue = false;
                if (Token::simpleMatch(parent, "&&") && parent->astOperand2() == node) {
                    evaluatedBefore = parent->astOperand1();
                    evaluatedValue = true;
                } else if (Token::simpleMatch(parent, "||") && parent->astOperand2() == node) {
                    evaluatedBefore = parent->astOperand1();
                    evaluatedValue = false;
                } else if (parent->str() == ":" && parent->astParent() && parent->astParent()->str() == "?" &&
                           parent->astParent()->astOperand2() == parent) {
                    evaluatedBefore = parent->astParent()->astOperand1();
                    evaluatedValue = parent->astOperand1() == node;
                }
                if (evaluatedBefore && astHasToken(evaluatedBefore, condition))
                    return ConditionBranch{true, evaluatedValue};
                node = parent;
            }
            return {};
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
            // The expression that holds the comparison is the child of the top that contains it: the
            // condition of an `if`/`while` hangs off astOperand2, the expression of a `return` off
            // astOperand1.
            const Token* conditionRoot = nullptr;
            if (conditionTop) {
                if (conditionTop->astOperand2() && astHasToken(conditionTop->astOperand2(), condition))
                    conditionRoot = conditionTop->astOperand2();
                else if (conditionTop->astOperand1() && astHasToken(conditionTop->astOperand1(), condition))
                    conditionRoot = conditionTop->astOperand1();
            }
            if (!conditionTop || !conditionRoot)
                return false;
            ConditionBranch branch = conditionBranchAtUse(conditionTop, use);
            if (!branch.valid)
                branch = conditionHoldsAtUse(condition, conditionTop, use);
            if (!branch.valid)
                return false;

            for (const Token* node = condition; node != conditionRoot;) {
                const Token* const parent = node->astParent();
                if (!parent)
                    return false;
                // In a for-clause, stop at the semicolon that owns the condition subtree.
                if (Token::simpleMatch(conditionTop->previous(), "for (") && parent->str() == ";" &&
                    parent->astOperand1() == node)
                    break;
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
            return branch.branchIsTrue == requiredConditionValue;
        }

        bool isForLoopUpperBoundActiveAtUse(const Value& value, const Token* valueTok, const Token* useTok);

        UnsignedArithmeticRange getUnsignedArithmeticRange(const Token* tok,
                                                           MathLib::bigint path,
                                                           const ValueType* resultType,
                                                           MathLib::biguint resultMax,
                                                           const Settings& settings,
                                                           const Token* useTok,
                                                           bool contextual,
                                                           bool includeSymbolic,
                                                           int depth = 8)
        {
            UnsignedArithmeticRange range;
            if (depth < 0)
                return UnsignedArithmeticRange{};
            const ValueType* const valueType = tok ? tok->valueType() : nullptr;
            if (!valueType || !valueType->isIntegral() || valueType->pointer)
                return UnsignedArithmeticRange{};

            const std::size_t valueSize = valueType->getSizeOf(settings,
                                                               ValueType::Accuracy::ExactOrZero,
                                                               ValueType::SizeOf::Pointer);
            const std::size_t valueBits = valueSize * settings.platform.char_bit;
            const std::size_t resultSize = resultType->getSizeOf(settings,
                                                                 ValueType::Accuracy::ExactOrZero,
                                                                 ValueType::SizeOf::Pointer);
            const std::size_t resultBits = resultSize * settings.platform.char_bit;
            if (valueBits == 0 || valueBits > 64 || resultBits == 0 || resultBits > 64)
                return UnsignedArithmeticRange{};

            const bool unsignedValue = valueType->sign == ValueType::Sign::UNSIGNED ||
                                       valueType->type == ValueType::Type::BOOL;
            MathLib::bigint signedLower{};
            MathLib::bigint signedUpper{};
            if (unsignedValue) {
                if (!getUnsignedTypeMaximum(valueType, valueBits, range.upper))
                    return UnsignedArithmeticRange{};
                range.lower = 0;
            } else {
                if (valueType->sign != ValueType::Sign::SIGNED ||
                    !getMinMaxValues(valueType, settings.platform, signedLower, signedUpper))
                    return UnsignedArithmeticRange{};
            }

            const Token* symbolicSource = nullptr;
            bool symbolicSourceConflict = false;
            bool symbolicUpperConflict = false;
            bool hasKnownPoint = false;
            MathLib::bigint knownSignedPoint{};
            MathLib::biguint knownUnsignedPoint{};
            for (const Value& value : tok->values()) {
                if (value.path < 0)
                    return UnsignedArithmeticRange{};
                if (value.path != 0 && value.path != path)
                    continue;
                if (value.isSymbolicValue() ? !includeSymbolic : (!value.isIntValue() || value.isInconclusive()))
                    continue;
                if (!contextual && (value.condition || value.conditional))
                    continue;
                if (contextual && value.path == 0 && value.condition &&
                    !isConditionFactActiveAtUse(value, tok, useTok))
                    continue;
                if (contextual && value.conditional && !value.isSymbolicValue() &&
                    !isForLoopUpperBoundActiveAtUse(value, tok, useTok))
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
                            symbolicUpperConflict = true;
                        } else {
                            range.symbolicUpper = value.tokvalue;
                            range.symbolicUpperOffset = value.intvalue;
                        }
                    }
                    continue;
                }

                if (value.bound == Value::Bound::Point) {
                    if (!value.isKnown())
                        continue;
                    if (unsignedValue) {
                        const MathLib::biguint point = truncateUnsignedValue(static_cast<MathLib::biguint>(value.intvalue), valueBits);
                        if (hasKnownPoint && point != knownUnsignedPoint)
                            return UnsignedArithmeticRange{};
                        knownUnsignedPoint = point;
                    } else {
                        if (hasKnownPoint && value.intvalue != knownSignedPoint)
                            return UnsignedArithmeticRange{};
                        knownSignedPoint = value.intvalue;
                    }
                    hasKnownPoint = true;
                    continue;
                }

                if (unsignedValue) {
                    if (value.intvalue < 0) {
                        if (value.isImpossible()) {
                            if (value.bound == Value::Bound::Lower)
                                return UnsignedArithmeticRange{};
                        } else if (value.bound == Value::Bound::Upper) {
                            return UnsignedArithmeticRange{};
                        }
                        continue;
                    }
                    const Bounds<MathLib::biguint> bounds = narrowBounds(
                        Bounds<MathLib::biguint>{true, range.lower, range.upper},
                        value,
                        static_cast<MathLib::biguint>(value.intvalue));
                    if (!bounds.valid)
                        return UnsignedArithmeticRange{};
                    range.lower = bounds.lower;
                    range.upper = bounds.upper;
                } else {
                    const Bounds<MathLib::bigint> bounds = narrowBounds(
                        Bounds<MathLib::bigint>{true, signedLower, signedUpper},
                        value,
                        value.intvalue);
                    if (!bounds.valid)
                        return UnsignedArithmeticRange{};
                    signedLower = bounds.lower;
                    signedUpper = bounds.upper;
                }
            }

            if (unsignedValue) {
                if (range.lower > range.upper)
                    return UnsignedArithmeticRange{};
                if (hasKnownPoint) {
                    if (knownUnsignedPoint < range.lower || knownUnsignedPoint > range.upper)
                        return UnsignedArithmeticRange{};
                    range.lower = truncateUnsignedValue(knownUnsignedPoint, resultBits);
                    range.upper = range.lower;
                }
            } else {
                if (signedLower > signedUpper)
                    return UnsignedArithmeticRange{};
                if (hasKnownPoint) {
                    if (knownSignedPoint < signedLower || knownSignedPoint > signedUpper)
                        return UnsignedArithmeticRange{};
                    range.lower = convertToUnsigned(knownSignedPoint, resultBits);
                    range.upper = range.lower;
                } else {
                    if (signedLower < 0)
                        return UnsignedArithmeticRange{};
                    range.lower = static_cast<MathLib::biguint>(signedLower);
                    range.upper = static_cast<MathLib::biguint>(signedUpper);
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
                        const UnsignedArithmeticRange sourceRange =
                            getUnsignedArithmeticRange(symbolicSource,
                                                       path,
                                                       resultType,
                                                       resultMax,
                                                       settings,
                                                       useTok,
                                                       contextual,
                                                       includeSymbolic,
                                                       depth - 1);
                        if (sourceRange.valid) {
                            range.lower = std::max(range.lower, sourceRange.lower);
                            range.upper = std::min(range.upper, sourceRange.upper);
                            if (range.upper < range.lower)
                                return UnsignedArithmeticRange{};
                            if (sourceRange.symbolicUpper && !symbolicUpperConflict) {
                                if (range.symbolicUpper &&
                                    (range.symbolicUpper != sourceRange.symbolicUpper ||
                                     range.symbolicUpperOffset != sourceRange.symbolicUpperOffset)) {
                                    range.symbolicUpper = nullptr;
                                } else {
                                    range.symbolicUpper = sourceRange.symbolicUpper;
                                    range.symbolicUpperOffset = sourceRange.symbolicUpperOffset;
                                }
                            }
                        }
                    }
                }
            }

            range.valid = range.lower <= range.upper && range.upper <= resultMax;
            return range;
        }
        bool getKnownPointIntValue(const Token* tok, MathLib::bigint& value)
        {
            if (!tok || !tok->hasKnownIntValue())
                return false;
            const Value& knownValue = tok->values().front();
            if (knownValue.bound != Value::Bound::Point || knownValue.path != 0 || knownValue.condition ||
                knownValue.conditional)
                return false;
            value = knownValue.intvalue;

            return true;
        }

        Bounds<MathLib::bigint> getForLoopIndexRange(const Token* valueTok,
                                                     const Token* useTok,
                                                     const Settings& settings)
        {
            if (!valueTok || valueTok->varId() == 0 || !useTok)
                return Bounds<MathLib::bigint>{};
            const ValueType* const valueType = valueTok->valueType();
            MathLib::bigint typeMinimum{}, typeMaximum{};
            if (!valueType || valueType->sign != ValueType::Sign::SIGNED || valueType->pointer ||
                !getMinMaxValues(valueType, settings.platform, typeMinimum, typeMaximum))
                return Bounds<MathLib::bigint>{};

            for (const Scope* loop = valueTok->scope(); loop; loop = loop->nestedIn) {
                if (loop->type != ScopeType::eFor || !loop->classDef ||
                    !containsToken(loop->bodyStart, loop->bodyEnd, useTok))
                    continue;
                const Token* const bodyStart = loop->bodyStart;
                const Token* const init = getInitTok(loop->classDef);
                const Token* const condition = getCondTok(loop->classDef);
                const Token* const step = getStepTok(loop->classDef);
                if (!Token::simpleMatch(init, "=") ||
                    !sameValueExpression(init->astOperand1(), valueTok) ||
                    !Token::Match(condition, "<|<=") ||
                    !sameValueExpression(condition->astOperand1(), valueTok) ||
                    !Token::simpleMatch(step, "++") ||
                    !sameValueExpression(getIncrementedToken(step), valueTok))
                    continue;

                MathLib::bigint initial{};
                MathLib::bigint limit{};
                if (!getKnownPointIntValue(init->astOperand2(), initial) || initial < 0 ||
                    !getKnownPointIntValue(condition->astOperand2(), limit) || limit < initial)
                    continue;
                const bool strict = Token::simpleMatch(condition, "<");
                if (strict && limit == initial)
                    continue;
                const MathLib::bigint loopUpper = strict ? limit - 1 : limit;
                const MathLib::bigint lower = std::max(initial, typeMinimum);
                const MathLib::bigint upper = std::min(loopUpper, typeMaximum);
                if (upper < lower)
                    continue;

                for (const Token* scan = bodyStart; scan && scan != useTok; scan = scan->next()) {
                    const Token* const nextToken = scan->isName() ? scan->next() : nullptr;
                    if (nextToken && nextToken->str() == "(" &&
                        !Token::Match(scan, "if|while|for|switch|catch|sizeof|decltype"))
                        return Bounds<MathLib::bigint>{};
                    if (scan->varId() != valueTok->varId())
                        continue;
                    const Token* const parent = scan->astParent();
                    if ((parent && (parent->isAssignmentOp() || Token::Match(parent, "++|--"))) ||
                        Token::Match(scan->next(), "++|--") || Token::simpleMatch(scan->previous(), "&"))
                        return Bounds<MathLib::bigint>{};
                }
                return Bounds<MathLib::bigint>{true, lower, upper};
            }
            return Bounds<MathLib::bigint>{};
        }

        bool isForLoopUpperBoundActiveAtUse(const Value& value, const Token* valueTok, const Token* useTok)
        {
            if (!value.conditional || !value.isPossible() || value.bound != Value::Bound::Upper || value.condition ||
                !valueTok || valueTok->varId() == 0 || !useTok)
                return false;

            for (const Scope* loop = valueTok->scope(); loop; loop = loop->nestedIn) {
                if (loop->type != ScopeType::eFor || !loop->classDef ||
                    !containsToken(loop->bodyStart, loop->bodyEnd, useTok))
                    continue;
                const Token* const condition = getCondTok(loop->classDef);
                if (!condition || !condition->isComparisonOp() || !condition->astOperand1() || !condition->astOperand2())
                    continue;

                const Token* boundTok = nullptr;
                bool strict = false;
                if (sameValueExpression(condition->astOperand1(), valueTok) && Token::Match(condition, "<|<=")) {
                    boundTok = condition->astOperand2();
                    strict = Token::simpleMatch(condition, "<");
                } else if (sameValueExpression(condition->astOperand2(), valueTok) &&
                           Token::Match(condition, ">|>=")) {
                    boundTok = condition->astOperand1();
                    strict = Token::simpleMatch(condition, ">");
                } else {
                    continue;
                }

                MathLib::bigint limit{};
                if (!getKnownPointIntValue(boundTok, limit) ||
                    (strict && limit == std::numeric_limits<MathLib::bigint>::min()))
                    continue;
                const MathLib::bigint upper = strict ? limit - 1 : limit;
                if (value.intvalue == upper)
                    return true;
            }
            return false;
        }

        bool isUnsignedArithmeticValue(const Token* tok)
        {
            const ValueType* const valueType = tok ? tok->valueType() : nullptr;
            return valueType && valueType->isIntegral() && !valueType->pointer &&
                   (valueType->sign == ValueType::Sign::UNSIGNED || valueType->type == ValueType::Type::BOOL);
        }

        bool isSmallUnsignedConstant(const Token* tok, MathLib::biguint maximum)
        {
            MathLib::bigint value{};
            return getKnownPointIntValue(tok, value) && value >= 0 &&
                   static_cast<MathLib::biguint>(value) <= maximum;
        }

        bool isUnsignedAdditionOperand(const Token* tok, MathLib::biguint smallAddendLimit, unsigned int depth = 8)
        {
            if (!tok || depth == 0)
                return false;
            if (tok->isCast()) {
                if (!isUnsignedArithmeticValue(tok))
                    return false;
                const Token* const source = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
                const ValueType* const sourceType = source ? source->valueType() : nullptr;
                if (!sourceType || sourceType->pointer ||
                    (sourceType->sign != ValueType::Sign::UNSIGNED && sourceType->type != ValueType::Type::BOOL))
                    return false;
                return isUnsignedAdditionOperand(source, smallAddendLimit, depth - 1);
            }
            if (tok->str() == "(") {
                if (Token::Match(tok->previous(), "%name% (") ||
                    (tok->astOperand1() && tok->astOperand1()->function()))
                    return isUnsignedArithmeticValue(tok);
                const Token* const inner = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
                return isUnsignedAdditionOperand(inner, smallAddendLimit, depth - 1);
            }
            if (!isUnsignedArithmeticValue(tok))
                return false;
            if (tok->variable() || tok->varId() != 0 || tok->str() == "." || tok->str() == "->")
                return true;
            if (tok->str() != "+" || !isUnsignedArithmeticResult(tok) || !tok->astOperand1() || !tok->astOperand2())
                return false;
            if (isSmallUnsignedConstant(tok->astOperand1(), smallAddendLimit))
                return isUnsignedAdditionOperand(tok->astOperand2(), smallAddendLimit, depth - 1);
            if (isSmallUnsignedConstant(tok->astOperand2(), smallAddendLimit))
                return isUnsignedAdditionOperand(tok->astOperand1(), smallAddendLimit, depth - 1);
            return false;
        }

        bool isSourceFactCompatible(const Value& fact,
                                    MathLib::bigint path,
                                    const Token* sourceTok,
                                    const Token* useTok)
        {
            if (fact.path > 0 && (path == 0 || fact.path != path))
                return false;
            // A conditional fact is only usable when it carries the condition it depends on: the
            // branch at the use then decides whether the fact still holds.
            if (fact.conditional && !fact.condition)
                return false;
            return fact.path != 0 || !fact.condition || isConditionFactActiveAtUse(fact, sourceTok, useTok);
        }

        // Combine the excluded facts of a source expression into one interval.
        // After `if (x < 0 || x > 100) return;` the facts `!<=-1` and `!>=101` are separate values.
        // Neither one proves a bound that survives a narrowing conversion on its own, but together
        // they describe the interval [0,100]. Only excluded bounds prove universal bounds; possible
        // bounds can be witnesses from different merged paths and are used only to reject contradictions.
        Bounds<MathLib::bigint> combineSourceRangeFacts(const Token* sourceTok,
                                                        MathLib::bigint path,
                                                        const Token* useTok)
        {
            bool hasLower = false;
            bool hasUpper = false;
            MathLib::bigint lower{};
            MathLib::bigint upper{};
            const auto isCompatible = [&](const Value& fact) {
                return isSourceFactCompatible(fact, path, sourceTok, useTok);
            };
            const auto updateBounds = [&](const Value& fact) {
                if (fact.bound == Value::Bound::Point || !fact.isImpossible())
                    return true;
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
                return Bounds<MathLib::bigint>{};
            if (!hasLower || !hasUpper || lower > upper)
                return Bounds<MathLib::bigint>{};
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
                return Bounds<MathLib::bigint>{};
            return Bounds<MathLib::bigint>{true, lower, upper};
        }

        bool hasFittedSourceRange(const Token* sourceTok,
                                  MathLib::bigint path,
                                  const Token* useTok,
                                  const ValueType* destination,
                                  const Settings& settings)
        {
            if (!sourceTok || !useTok)
                return false;
            const Bounds<MathLib::bigint> bounds = combineSourceRangeFacts(sourceTok, path, useTok);
            if (!bounds.valid)
                return false;
            const ValueType::Sign destinationSign = resolveCharSign(*destination, settings.platform);
            if (destinationSign == ValueType::Sign::UNKNOWN_SIGN)
                return false;
            ValueType resolvedType = *destination;
            resolvedType.sign = destinationSign;
            MathLib::bigint destinationMinimum{};
            MathLib::bigint destinationMaximum{};
            return getMinMaxValues(&resolvedType, settings.platform, destinationMinimum, destinationMaximum) &&
                   bounds.lower >= destinationMinimum && bounds.upper <= destinationMaximum;
        }
    }

    std::vector<Value> getFittedSourceValues(const Token* sourceTok,
                                             const Value& value,
                                             const Token* useTok,
                                             const ValueType* destination,
                                             const Settings& settings)
    {
        std::vector<Value> values;
        if (!sourceTok || !useTok)
            return values;
        if (!hasFittedSourceRange(sourceTok, value.path, useTok, destination, settings))
            return values;
        // The destination represents the whole proven interval exactly, so the conversion leaves every
        // excluded bound unchanged. Collect them, keeping the error path of the fact they come from,
        // but without the path id and the condition: what they record holds at the use, unconditionally.
        // A bound that kept its path id would differ from the same bound derived by an earlier pass, so
        // each pass would add a new value and value flow would never settle.
        for (const Value& source : sourceTok->values()) {
            if (!source.isIntValue() || source.isInconclusive() || source.bound == Value::Bound::Point ||
                !source.isImpossible() || !isSourceFactCompatible(source, value.path, sourceTok, useTok))
                continue;
            Value fact = source;
            fact.path = 0;
            fact.conditional = false;
            values.push_back(std::move(fact));
        }
        return values;
    }

    ConvertedValue convertValueRange(Value value,
                                     const ValueType* source,
                                     const ValueType* destination,
                                     const Settings& settings,
                                     const Token* sourceTok,
                                     const Token* useTok)
    {
        if (!value.isIntValue() || value.bound == Value::Bound::Point || value.isInconclusive() ||
            !destination || !destination->isIntegral() || destination->pointer)
            return ConvertedValue{};

        if (source && source->isIntegral() && source->sign == ValueType::Sign::SIGNED &&
            destination->sign == ValueType::Sign::UNSIGNED && destination->type != ValueType::Type::BOOL &&
            value.isPossible() && value.bound == Value::Bound::Upper && sourceTok && useTok) {
            MathLib::bigint destinationMinimum{};
            MathLib::bigint destinationMaximum{};
            const Bounds<MathLib::bigint> loopRange = getForLoopIndexRange(sourceTok, useTok, settings);
            if (loopRange.valid && loopRange.lower >= 0 && loopRange.upper >= loopRange.lower &&
                value.intvalue >= loopRange.lower &&
                getMinMaxValues(destination, settings.platform, destinationMinimum, destinationMaximum)) {
                const MathLib::bigint upper = std::min(value.intvalue, loopRange.upper);
                if (loopRange.lower >= destinationMinimum && upper <= destinationMaximum) {
                    value.intvalue = upper;
                    return ConvertedValue(std::move(value));
                }
            }
        }

        if (!source) {
            if (!sourceTok || !useTok || sourceTok->valueType() || value.path < 0 || value.conditional ||
                (value.condition && value.path == 0 && !isConditionFactActiveAtUse(value, sourceTok, useTok)))
                return ConvertedValue{};
            const Variable* const sourceVariable = sourceTok->variable();
            if (sourceVariable && sourceVariable->isPointer())
                return ConvertedValue{};

            if (!hasFittedSourceRange(sourceTok, value.path, useTok, destination, settings))
                return ConvertedValue{};
            return ConvertedValue(std::move(value));
        }

        if (!source->isIntegral() || source->pointer)
            return ConvertedValue{};
        if (isValueRangePreservingConversion(source, destination, settings, value))
            return ConvertedValue(std::move(value));
        // Implicit conversions (assignments, arguments) reach this function directly, without the cast
        // path that re-emits the fitted facts, so accept the combined range here as well.
        if (hasFittedSourceRange(sourceTok, value.path, useTok, destination, settings))
            return ConvertedValue(std::move(value));
        if (value.isImpossible() ||
            destination->sign != ValueType::Sign::UNSIGNED || destination->type == ValueType::Type::BOOL ||
            (source->sign != ValueType::Sign::SIGNED && source->sign != ValueType::Sign::UNSIGNED))
            return ConvertedValue{};

        const std::size_t destinationBits = getTypeBits(*destination, settings);
        const std::size_t carrierBits = std::numeric_limits<MathLib::biguint>::digits;
        if (destinationBits == 0 || destinationBits > carrierBits)
            return ConvertedValue{};
        if (destinationBits == carrierBits) {
            if (source->sign != ValueType::Sign::SIGNED)
                return ConvertedValue{};
            MathLib::bigint sourceMinimum{};
            MathLib::bigint sourceMaximum{};
            if (!getMinMaxValues(source, settings.platform, sourceMinimum, sourceMaximum))
                return ConvertedValue{};
            const Bounds<MathLib::bigint> bounds = narrowBounds(
                Bounds<MathLib::bigint>{true, sourceMinimum, sourceMaximum},
                value,
                value.intvalue);
            if (!bounds.valid || bounds.lower > bounds.upper || bounds.upper >= 0)
                return ConvertedValue{};
            value.intvalue = std::numeric_limits<MathLib::bigint>::max();
            value.bound = Value::Bound::Lower;
            return ConvertedValue(std::move(value));
        }
        const MathLib::biguint destinationMaximum = (MathLib::biguint{1} << destinationBits) - 1;

        MathLib::biguint imageMinimum{};
        MathLib::biguint imageMaximum{};
        if (source->sign == ValueType::Sign::SIGNED) {
            MathLib::bigint sourceMinimum{};
            MathLib::bigint sourceMaximum{};
            if (!getMinMaxValues(source, settings.platform, sourceMinimum, sourceMaximum))
                return ConvertedValue{};
            const Bounds<MathLib::bigint> bounds = narrowBounds(
                Bounds<MathLib::bigint>{true, sourceMinimum, sourceMaximum},
                value,
                value.intvalue);
            if (!bounds.valid || bounds.lower > bounds.upper)
                return ConvertedValue{};
            const MathLib::biguint distance = static_cast<MathLib::biguint>(bounds.upper) -
                                              static_cast<MathLib::biguint>(bounds.lower);
            if (distance >= destinationMaximum)
                return ConvertedValue{};
            imageMinimum = static_cast<MathLib::biguint>(bounds.lower) & destinationMaximum;
            imageMaximum = static_cast<MathLib::biguint>(bounds.upper) & destinationMaximum;
        } else {
            const std::size_t sourceBits = getTypeBits(*source, settings);
            MathLib::biguint sourceMaximum{};
            if (!getUnsignedTypeMaximum(source, sourceBits, sourceMaximum) || value.intvalue < 0)
                return ConvertedValue{};
            const Bounds<MathLib::biguint> bounds = narrowBounds(
                Bounds<MathLib::biguint>{true, MathLib::biguint{0}, sourceMaximum},
                value,
                static_cast<MathLib::biguint>(value.intvalue));
            if (!bounds.valid || bounds.lower > bounds.upper)
                return ConvertedValue{};
            const MathLib::biguint distance = bounds.upper - bounds.lower;
            if (distance >= destinationMaximum)
                return ConvertedValue{};
            imageMinimum = bounds.lower & destinationMaximum;
            imageMaximum = bounds.upper & destinationMaximum;
        }
        if (imageMinimum > imageMaximum)
            return ConvertedValue{};

        if (imageMinimum > 0) {
            if (imageMinimum > static_cast<MathLib::biguint>(std::numeric_limits<MathLib::bigint>::max()))
                return ConvertedValue{};
            value.intvalue = static_cast<MathLib::bigint>(imageMinimum);
            value.bound = Value::Bound::Lower;
            return ConvertedValue(std::move(value));
        }
        if (imageMaximum < destinationMaximum) {
            value.intvalue = static_cast<MathLib::bigint>(imageMaximum);
            value.bound = Value::Bound::Upper;
            return ConvertedValue(std::move(value));
        }
        return ConvertedValue{};
    }

    // A known small non-negative addend whose other operand cannot reach the type's maximum.
    static bool canAssumeNoWrap(const Token* expr,
                                const Token* addend,
                                const Token* value,
                                MathLib::biguint smallAddendLimit,
                                MathLib::biguint resultMax,
                                const Settings& settings)
    {
        MathLib::bigint addendValue{};
        if (!getKnownPointIntValue(addend, addendValue) || addendValue < 0 ||
            static_cast<MathLib::biguint>(addendValue) > smallAddendLimit)
            return false;
        if (!isUnsignedAdditionOperand(value, smallAddendLimit)) {
            const Bounds<MathLib::bigint> loopRange = getForLoopIndexRange(value, expr, settings);
            if (!loopRange.valid || loopRange.lower < 0 ||
                loopRange.upper < loopRange.lower ||
                static_cast<MathLib::biguint>(loopRange.upper) >
                resultMax - static_cast<MathLib::biguint>(addendValue))
                return false;
        }

        const Token* expression = expr;
        const Token* parent = expression->astParent();
        while (parent && (parent->str() == "(" || parent->isCast())) {
            expression = parent;
            parent = parent->astParent();
        }
        if (parent && parent->isComparisonOp()) {
            const Token* compareValue = nullptr;
            if (parent->astOperand1() == expression)
                compareValue = parent->astOperand2();
            else if (parent->astOperand2() == expression)
                compareValue = parent->astOperand1();
            MathLib::bigint compareConstant{};
            if (getKnownPointIntValue(compareValue, compareConstant) &&
                (compareConstant < addendValue ||
                 (compareConstant == addendValue && Token::Match(parent, "<|<="))))
                return false;
        }
        return true;
    }

    // An addition of a relative offset that the enclosing for loop keeps below the type's maximum.
    static bool isSafeRelativeAddition(const UnsignedArithmeticRange& boundedRange,
                                       const UnsignedArithmeticRange& addendRange,
                                       const Token* boundedOperand,
                                       std::size_t resultBits,
                                       const Settings& settings)
    {
        if (!boundedRange.symbolicUpper ||
            !boundedOperand || !boundedOperand->variable() ||
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
            const Token* const init = getInitTok(loop->classDef);
            const Token* const condition = getCondTok(loop->classDef);
            const Token* const step = getStepTok(loop->classDef);
            if (!Token::simpleMatch(condition, "<") ||
                condition->astOperand1()->varId() != boundedOperand->varId() ||
                condition->astOperand2()->varId() != boundedRange.symbolicUpper->varId() ||
                !Token::simpleMatch(init, "=") ||
                init->astOperand1()->varId() != boundedOperand->varId() ||
                !Token::simpleMatch(step, "++"))
                continue;
            const Token* const stepVariable = getIncrementedToken(step);
            if (stepVariable && stepVariable->varId() == boundedOperand->varId())
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

        if (tok->str() == "+") {
            // Keep the historical small-increment assumption for simple unsigned values. The relative
            // cap avoids applying it to increments that are large for the result type; explicit wrap
            // checks such as x + 1 == 0 and x + c < c remain subject to the range proof below.
            const MathLib::biguint smallAddendLimit = std::min<MathLib::biguint>(0xFFFF, resultMax >> 1);
            if (canAssumeNoWrap(tok, tok->astOperand1(), tok->astOperand2(), smallAddendLimit, resultMax, settings) ||
                canAssumeNoWrap(tok, tok->astOperand2(), tok->astOperand1(), smallAddendLimit, resultMax, settings))
                return true;
        }

        const Token* const lhs = tok->astOperand1();
        const Token* const rhs = tok->astOperand2();
        const auto hasOnlyNonnegativePaths = [](const Token* operand) {
            return std::all_of(operand->values().cbegin(), operand->values().cend(), [](const Value& value) {
                return value.path >= 0;
            });
        };
        if (!hasOnlyNonnegativePaths(lhs) || !hasOnlyNonnegativePaths(rhs))
            return false;
        std::vector<MathLib::bigint> paths;
        if (requireGlobalContext)
            paths.push_back(0);
        else
            paths = getCommonPaths(lhs->values(), rhs->values());
        if (paths.empty())
            return false;

        const bool contextual = !requireGlobalContext;
        return std::all_of(paths.cbegin(), paths.cend(), [&](MathLib::bigint path) {
            if (tok->str() == "-") {
                const UnsignedArithmeticRange rhsPoint = getUnsignedArithmeticRange(rhs, path, resultType, resultMax,
                                                                                    settings, tok, contextual, false);
                if (rhsPoint.valid && rhsPoint.lower == rhsPoint.upper) {
                    if (rhsPoint.upper == 0)
                        return true;
                    const UnsignedArithmeticRange lhsRange = getUnsignedArithmeticRange(lhs, path, resultType, resultMax,
                                                                                        settings, tok, contextual, false);
                    if (lhsRange.valid && lhsRange.lower >= rhsPoint.upper)
                        return true;
                }
            }
            if (tok->str() == "+") {
                const auto isZeroOperand = [&](const Token* operand) {
                    const UnsignedArithmeticRange zeroRange = getUnsignedArithmeticRange(operand, path, resultType,
                                                                                         resultMax, settings, tok,
                                                                                         contextual, false);
                    return zeroRange.valid && zeroRange.lower == 0 && zeroRange.upper == 0;
                };
                if (isZeroOperand(lhs) || isZeroOperand(rhs))
                    return true;
            }
            const UnsignedArithmeticRange lhsRange = getUnsignedArithmeticRange(lhs, path, resultType, resultMax,
                                                                                settings, tok, contextual, true);
            const UnsignedArithmeticRange rhsRange = getUnsignedArithmeticRange(rhs, path, resultType, resultMax,
                                                                                settings, tok, contextual, true);
            if (!lhsRange.valid || !rhsRange.valid)
                return false;

            if (tok->str() == "-") {
                if (lhsRange.lower < rhsRange.upper)
                    return false;
            } else if (tok->str() == "+") {
                if (lhsRange.upper > resultMax - rhsRange.upper &&
                    !isSafeRelativeAddition(lhsRange, rhsRange, lhs, resultBits, settings) &&
                    !isSafeRelativeAddition(rhsRange, lhsRange, rhs, resultBits, settings))
                    return false;
            } else if (tok->str() == "*") {
                if (lhsRange.upper != 0 && rhsRange.upper > resultMax / lhsRange.upper)
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
        if (resolveCharSign(*dst, platform) == ValueType::Sign::SIGNED && (value & signBit))
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
                    !sz1->variable()->isDecayedArrayArgument() &&
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
                    const bool isPointer = var->isDecayedArrayArgument();
                    size_t size = 0;
                    if (isPointer) {
                        size = settings.platform.sizeof_pointer;
                    } else if (var->isEnumType()) {
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
                    for (size_t i = 0; !isPointer && i < var->dimensions().size(); ++i) {
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

    ConvertedValue castValue(Value value, const ValueType::Sign sign, nonneg int bit)
    {
        if (bit <= 0 || sign == ValueType::Sign::UNKNOWN_SIGN)
            return ConvertedValue{};

        if (value.isFloatValue()) {
            MathLib::bigint converted{};
            if (value.isImpossible() || !convertFloatToInteger(converted, value.floatValue, sign, bit))
                return ConvertedValue{};
            value.intvalue = converted;
            value.valueType = Value::ValueType::INT;
            return ConvertedValue(std::move(value));
        }

        if (value.bound != Value::Bound::Point || !value.isIntValue())
            return ConvertedValue{};
        if (bit < MathLib::bigint_bits) {
            const MathLib::biguint mask = (MathLib::biguint{1} << bit) - 1;
            const MathLib::biguint raw = static_cast<MathLib::biguint>(value.intvalue) & mask;
            if (sign == ValueType::Sign::SIGNED && (raw & (MathLib::biguint{1} << (bit - 1))))
                value.intvalue = -1 - static_cast<MathLib::bigint>((~raw) & mask);
            else
                value.intvalue = static_cast<MathLib::bigint>(raw);
        } else if (sign == ValueType::Sign::UNSIGNED && value.intvalue < 0) {
            return ConvertedValue{};
        }
        return ConvertedValue(std::move(value));
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
