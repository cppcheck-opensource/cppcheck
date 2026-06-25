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

#ifndef analyzerH
#define analyzerH

#include "config.h"
#include "mathlib.h"

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

class Token;
template<class T>
class ValuePtr;

struct Analyzer {
    struct Action {

        Action() = default;
        Action(const Action&) = default;
        Action& operator=(const Action& rhs) & = default;

        template<class T,
                 REQUIRES("T must be convertible to unsigned int", std::is_convertible<T, unsigned int> ),
                 REQUIRES("T must not be a bool", !std::is_same<T, bool> )>
        // NOLINTNEXTLINE(google-explicit-constructor)
        Action(T f) : mFlag(f) // cppcheck-suppress noExplicitConstructor
        {}

        enum : std::uint16_t {
            None = 0,
            Read = (1 << 0),
            Write = (1 << 1),
            Invalid = (1 << 2),
            Inconclusive = (1 << 3),
            Match = (1 << 4),
            Idempotent = (1 << 5),
            Incremental = (1 << 6),
            SymbolicMatch = (1 << 7),
            Internal = (1 << 8),
        };

        void set(unsigned int f, bool state = true) {
            mFlag = state ? mFlag | f : mFlag & ~f;
        }

        bool get(unsigned int f) const {
            return ((mFlag & f) != 0);
        }

        bool isRead() const {
            return get(Read);
        }

        bool isWrite() const {
            return get(Write);
        }

        bool isInvalid() const {
            return get(Invalid);
        }

        bool isInconclusive() const {
            return get(Inconclusive);
        }

        bool isNone() const {
            return mFlag == None;
        }

        bool isModified() const {
            return isWrite() || isInvalid();
        }

        bool isIdempotent() const {
            return get(Idempotent);
        }

        bool isIncremental() const {
            return get(Incremental);
        }

        bool isSymbolicMatch() const {
            return get(SymbolicMatch);
        }

        bool isInternal() const {
            return get(Internal);
        }

        bool matches() const {
            return get(Match);
        }

        Action& operator|=(Action a) {
            set(a.mFlag);
            return *this;
        }

        friend Action operator|(Action a, Action b) {
            a |= b;
            return a;
        }

        friend bool operator==(Action a, Action b) {
            return a.mFlag == b.mFlag;
        }

        friend bool operator!=(Action a, Action b) {
            return a.mFlag != b.mFlag;
        }

    private:
        unsigned int mFlag{};
    };

    enum class Terminate : std::uint8_t { None, Bail, Escape, Modified, Inconclusive, Conditional };

    struct Result {
        explicit Result(Action action = Action::None, Terminate terminate = Terminate::None)
            : action(action), terminate(terminate)
        {}
        Action action;
        Terminate terminate;

        void update(Result rhs) {
            if (terminate == Terminate::None)
                terminate = rhs.terminate;
            action |= rhs.action;
        }
    };

    enum class Direction : std::uint8_t { Forward, Reverse };

    struct Assume {
        enum Flags : std::uint8_t {
            None = 0,
            Quiet = (1 << 0),
            Absolute = (1 << 1),
            ContainerEmpty = (1 << 2),
        };
    };

    enum class Evaluate : std::uint8_t { Integral, ContainerEmpty };

    /// Analyze a token
    virtual Action analyze(const Token* tok, Direction d) const = 0;
    /// Update the state of the value
    virtual void update(Token* tok, Action a, Direction d) = 0;
    /// Try to evaluate the value of a token(most likely a condition)
    virtual std::vector<MathLib::bigint> evaluate(Evaluate e, const Token* tok, const Token* ctx = nullptr) const = 0;
    std::vector<MathLib::bigint> evaluate(const Token* tok, const Token* ctx = nullptr) const
    {
        return evaluate(Evaluate::Integral, tok, ctx);
    }
    /// Lower any values to possible
    virtual bool lowerToPossible() = 0;
    /// Lower any values to inconclusive
    virtual bool lowerToInconclusive() = 0;
    /// If the analysis is unsure whether to update a scope, this will return true if the analysis should bifurcate the scope
    virtual bool updateScope(const Token* endBlock, bool modified) const = 0;
    /// If the value is conditional
    virtual bool isConditional() const = 0;
    /// If analysis should stop on the condition
    virtual bool stopOnCondition(const Token* condTok) const = 0;
    /// The condition that will be assumed during analysis
    virtual void assume(const Token* tok, bool state, unsigned int flags = 0) = 0;
    /// Update the state of the program at the token
    virtual void updateState(const Token* tok) = 0;
    /// Return analyzer for expression at token
    virtual ValuePtr<Analyzer> reanalyze(Token* tok, const std::string& msg = "") const = 0;
    virtual bool invalid() const {
        return false;
    }
    virtual ~Analyzer() = default;
    Analyzer(const Analyzer&) = default;
protected:
    Analyzer() = default;
};

#endif
