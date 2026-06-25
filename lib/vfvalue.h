/* -*- C++ -*-
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

//---------------------------------------------------------------------------
#ifndef vfvalueH
#define vfvalueH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"
#include "mathlib.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

FORCE_WARNING_CLANG_PUSH("-Wpadded")

class Token;

namespace ValueFlow
{
    class CPPCHECKLIB Value {
    public:
        enum class Bound : std::uint8_t { Upper, Lower, Point };

        explicit Value(MathLib::bigint val = 0, Bound b = Bound::Point) :
            bound(b),
            safe(false),
            conditional(false),
            macro(false),
            defaultArg(false),
            intvalue(val),
            varvalue(val),
            wideintvalue(val)
        {}
        Value(const Token* c, MathLib::bigint val, Bound b = Bound::Point);

        static Value unknown() {
            Value v;
            v.valueType = ValueType::UNINIT;
            return v;
        }

        bool equalValue(const ValueFlow::Value& rhs) const {
            if (valueType != rhs.valueType)
                return false;
            switch (valueType) {
            case ValueType::INT:
            case ValueType::CONTAINER_SIZE:
            case ValueType::BUFFER_SIZE:
            case ValueType::ITERATOR_START:
            case ValueType::ITERATOR_END:
                if (intvalue != rhs.intvalue)
                    return false;
                break;
            case ValueType::TOK:
                if (tokvalue != rhs.tokvalue)
                    return false;
                break;
            case ValueType::FLOAT:
                if (floatValue > rhs.floatValue || floatValue < rhs.floatValue || std::signbit(floatValue) != std::signbit(rhs.floatValue))
                    return false;
                break;
            case ValueType::MOVED:
                if (moveKind != rhs.moveKind)
                    return false;
                break;
            case ValueType::UNINIT:
                break;
            case ValueType::LIFETIME:
                if (tokvalue != rhs.tokvalue)
                    return false;
                break;
            case ValueType::SYMBOLIC:
                if (!sameToken(tokvalue, rhs.tokvalue))
                    return false;
                if (intvalue != rhs.intvalue)
                    return false;
                break;
            }
            return true;
        }

        template<class T, class F>
        static void visitValue(T& self, F f) {
            switch (self.valueType) {
            case ValueType::INT:
            case ValueType::SYMBOLIC:
            case ValueType::BUFFER_SIZE:
            case ValueType::CONTAINER_SIZE:
            case ValueType::ITERATOR_START:
            case ValueType::ITERATOR_END: {
                f(self.intvalue);
                break;
            }
            case ValueType::FLOAT: {
                f(self.floatValue);
                break;
            }
            case ValueType::UNINIT:
            case ValueType::TOK:
            case ValueType::LIFETIME:
            case ValueType::MOVED:
                break;
            }
        }

        struct compareVisitor {
            struct innerVisitor {
                template<class Compare, class T, class U>
                void operator()(bool& result, Compare compare, T x, U y) const {
                    result = compare(x, y);
                }
            };
            template<class Compare, class T>
            void operator()(bool& result, const Value& rhs, Compare compare, T x) const {
                visitValue(rhs,
                           std::bind(innerVisitor{}, std::ref(result), std::move(compare), x, std::placeholders::_1));
            }
        };

        template<class Compare>
        bool compareValue(const Value& rhs, Compare compare) const {
            assert((!this->isSymbolicValue() && !rhs.isSymbolicValue()) ||
                   (this->valueType == rhs.valueType && sameToken(this->tokvalue, rhs.tokvalue)));
            bool result = false;
            visitValue(
                *this,
                std::bind(compareVisitor{}, std::ref(result), std::ref(rhs), std::move(compare), std::placeholders::_1));
            return result;
        }

        bool operator==(const Value &rhs) const {
            if (!equalValue(rhs))
                return false;

            return varvalue == rhs.varvalue &&
                   condition == rhs.condition &&
                   varId == rhs.varId &&
                   conditional == rhs.conditional &&
                   defaultArg == rhs.defaultArg &&
                   indirect == rhs.indirect &&
                   valueKind == rhs.valueKind;
        }

        bool operator!=(const Value &rhs) const {
            return !(*this == rhs);
        }

        template<class T, REQUIRES("T must be an arithmetic type", std::is_arithmetic<T> )>
        bool equalTo(const T& x) const {
            bool result = false;
            visitValue(*this, std::bind(equalVisitor{}, std::ref(result), x, std::placeholders::_1));
            return result;
        }

        void decreaseRange() {
            if (bound == Bound::Lower)
                visitValue(*this, increment{});
            else if (bound == Bound::Upper)
                visitValue(*this, decrement{});
        }

        void invertBound() {
            if (bound == Bound::Lower)
                bound = Bound::Upper;
            else if (bound == Bound::Upper)
                bound = Bound::Lower;
        }

        void invertRange() {
            invertBound();
            decreaseRange();
        }

        void assumeCondition(const Token* tok);

        std::string infoString() const;

        std::string toString() const;

        enum class ValueType : std::uint8_t {
            INT,
            TOK,
            FLOAT,
            MOVED,
            UNINIT,
            CONTAINER_SIZE,
            LIFETIME,
            BUFFER_SIZE,
            ITERATOR_START,
            ITERATOR_END,
            SYMBOLIC
        } valueType = ValueType::INT;
        bool isIntValue() const {
            return valueType == ValueType::INT;
        }
        bool isTokValue() const {
            return valueType == ValueType::TOK;
        }
        bool isFloatValue() const {
            return valueType == ValueType::FLOAT;
        }
        bool isMovedValue() const {
            return valueType == ValueType::MOVED;
        }
        bool isUninitValue() const {
            return valueType == ValueType::UNINIT;
        }
        bool isContainerSizeValue() const {
            return valueType == ValueType::CONTAINER_SIZE;
        }
        bool isLifetimeValue() const {
            return valueType == ValueType::LIFETIME;
        }
        bool isBufferSizeValue() const {
            return valueType == ValueType::BUFFER_SIZE;
        }
        bool isIteratorValue() const {
            return valueType == ValueType::ITERATOR_START || valueType == ValueType::ITERATOR_END;
        }
        bool isIteratorStartValue() const {
            return valueType == ValueType::ITERATOR_START;
        }
        bool isIteratorEndValue() const {
            return valueType == ValueType::ITERATOR_END;
        }
        bool isSymbolicValue() const {
            return valueType == ValueType::SYMBOLIC;
        }

        bool isLocalLifetimeValue() const {
            return valueType == ValueType::LIFETIME && lifetimeScope == LifetimeScope::Local;
        }

        bool isArgumentLifetimeValue() const {
            return valueType == ValueType::LIFETIME && lifetimeScope == LifetimeScope::Argument;
        }

        bool isSubFunctionLifetimeValue() const {
            return valueType == ValueType::LIFETIME && lifetimeScope == LifetimeScope::SubFunction;
        }

        bool isNonValue() const {
            return isMovedValue() || isUninitValue() || isLifetimeValue();
        }

        /** The value bound  */
        Bound bound = Bound::Point;

        /** value relies on safe checking */
        // cppcheck-suppress premium-misra-cpp-2023-12.2.1
        bool safe : 1;

        /** Conditional value */
        bool conditional : 1;

        /** Value is is from an expanded macro */
        bool macro : 1;

        /** Is this value passed as default parameter to the function? */
        bool defaultArg : 1;

        long long : 4; // padding

        /** kind of moved  */
        enum class MoveKind : std::uint8_t { NonMovedVariable, MovedVariable, ForwardedVariable } moveKind = MoveKind::NonMovedVariable;

        enum class LifetimeScope : std::uint8_t { Local, Argument, SubFunction, ThisPointer, ThisValue } lifetimeScope = LifetimeScope::Local;

        enum class LifetimeKind : std::uint8_t {
            // Pointer points to a member of lifetime
            Object,
            // A member of object points to the lifetime
            SubObject,
            // Lambda has captured lifetime(similar to SubObject)
            Lambda,
            // Iterator points to the lifetime of a container(similar to Object)
            Iterator,
            // A pointer that holds the address of the lifetime
            Address
        } lifetimeKind = LifetimeKind::Object;

        /** How known is this value */
        enum class ValueKind : std::uint8_t {
            /** This value is possible, other unlisted values may also be possible */
            Possible,
            /** Only listed values are possible */
            Known,
            /** Inconclusive */
            Inconclusive,
            /** Listed values are impossible */
            Impossible
        } valueKind = ValueKind::Possible;

        std::int8_t indirect{}; // TODO: can we reduce the size?

        /** int value (or sometimes bool value?) */
        MathLib::bigint intvalue{};

        /** token value - the token that has the value. this is used for pointer aliases, strings, etc. */
        const Token* tokvalue{};

        /** float value */
        double floatValue{};

        /** For calculated values - variable value that calculated value depends on */
        MathLib::bigint varvalue{};

        /** Condition that this value depends on */
        const Token* condition{};

        ErrorPath errorPath;

        ErrorPath debugPath; // TODO: make lighter by default

        /** For calculated values - varId that calculated value depends on */
        nonneg int varId{};

        enum class UnknownFunctionReturn : std::uint8_t {
            no,             // not unknown function return
            outOfMemory,    // out of memory
            outOfResources, // out of resource
            other           // other
        };
        UnknownFunctionReturn unknownFunctionReturn{UnknownFunctionReturn::no};

        long long : 24; // padding

        /** Path id */
        MathLib::bigint path{};

        /** int value before implicit truncation */
        MathLib::bigint wideintvalue{};

        std::vector<std::string> subexpressions;

        // Set to where a lifetime is captured by value
        const Token* capturetok{};

        RET_NONNULL static const char* toString(MoveKind moveKind);
        RET_NONNULL static const char* toString(LifetimeKind lifetimeKind);
        RET_NONNULL static const char* toString(LifetimeScope lifetimeScope);
        RET_NONNULL static const char* toString(Bound bound);

        void setKnown() {
            valueKind = ValueKind::Known;
        }

        bool isKnown() const {
            return valueKind == ValueKind::Known;
        }

        void setPossible() {
            valueKind = ValueKind::Possible;
        }

        bool isPossible() const {
            return valueKind == ValueKind::Possible;
        }

        bool isImpossible() const {
            return valueKind == ValueKind::Impossible;
        }

        void setImpossible() {
            valueKind = ValueKind::Impossible;
        }

        void setInconclusive(bool inconclusive = true) {
            if (inconclusive)
                valueKind = ValueKind::Inconclusive;
        }

        bool isInconclusive() const {
            return valueKind == ValueKind::Inconclusive;
        }

        void changeKnownToPossible() {
            if (isKnown())
                valueKind = ValueKind::Possible;
        }

        bool errorSeverity() const {
            return !condition && !defaultArg;
        }

        static bool sameToken(const Token* tok1, const Token* tok2);

    private:
        struct equalVisitor {
            template<class T, class U>
            void operator()(bool& result, T x, U y) const {
                result = !(x > y || x < y);
            }
        };

        struct increment {
            template<class T>
            void operator()(T& x) const {
                x++;
            }
        };
        struct decrement {
            template<class T>
            void operator()(T& x) const {
                x--;
            }
        };
    };
}

FORCE_WARNING_CLANG_POP

#endif // vfvalueH
