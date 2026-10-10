/* -*- C++ -*-
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

#ifndef GUARD_PROGRAMMEMORY_H
#define GUARD_PROGRAMMEMORY_H

#include "config.h"
#include "mathlib.h"
#include "vfvalue.h" // needed for alias

#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

class Scope;
class Token;
class Settings;

// Class used to handle heterogeneous lookup in unordered_map(since we can't use C++20 yet)
struct ExprIdToken {
    const Token* tok = nullptr;
    nonneg int exprid = 0;

    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    ExprIdToken(const Token* tok);

    nonneg int getExpressionId() const;

    bool operator==(const ExprIdToken& rhs) const {
        return getExpressionId() == rhs.getExpressionId();
    }

    bool operator<(const ExprIdToken& rhs) const {
        return getExpressionId() < rhs.getExpressionId();
    }

    template<class T, class U>
    friend bool operator!=(const T& lhs, const U& rhs)
    {
        return !(lhs == rhs);
    }

    template<class T, class U>
    friend bool operator<=(const T& lhs, const U& rhs)
    {
        return !(lhs > rhs);
    }

    template<class T, class U>
    friend bool operator>(const T& lhs, const U& rhs)
    {
        return rhs < lhs;
    }

    template<class T, class U>
    friend bool operator>=(const T& lhs, const U& rhs)
    {
        return !(lhs < rhs);
    }

    const Token& operator*() const noexcept {
        return *tok;
    }

    const Token* operator->() const noexcept {
        return tok;
    }

    struct Hash {
        std::size_t operator()(ExprIdToken etok) const;
    };

    /** create object for hashed lookups */
    static ExprIdToken create(nonneg int exprId) {
        return ExprIdToken(exprId);
    }

private:
    // for hashed lookups only
    explicit ExprIdToken(nonneg int exprId);
};

struct CPPCHECKLIB ProgramMemory {
    /**
     * The values recorded for one expression. Either a single value of the expression (a possible
     * value with a bound is still its value; the bound is extra information about the range it lies
     * in) or a set of constraints that hold at the same time: impossible values, where a bound makes
     * the value an impossible range, so that "x > 3" is recorded as "values <= 3 are impossible".
     * The constraints of one expression all have the same value type. A list, so that adding a
     * constraint does not move the values already recorded.
     */
    using Values = std::list<ValueFlow::Value>;
    using Map = std::map<ExprIdToken, Values>;

    ProgramMemory() : mValues(new Map()) {}

    explicit ProgramMemory(Map values) : mValues(new Map(std::move(values))) {}

    /**
     * Record a fact about the expression. A value of the expression replaces everything recorded so
     * far. A constraint (impossible value) is added to the constraints already recorded, keeping only
     * the strongest bound in each direction; it replaces a recorded value only if that value violates it.
     */
    void setValue(const Token* expr, const ValueFlow::Value& value);
    /** setValue() for each of the values */
    void setValues(const Token* expr, const Values& values);
    /**
     * The single value recorded for the expression, or nullptr if there is none or if several
     * constraints are recorded. Impossible values are skipped unless impossible is true.
     */
    const ValueFlow::Value* getValue(nonneg int exprid, bool impossible = false) const;
    /** All values recorded for the expression, or nullptr if there are none */
    const Values* getValues(nonneg int exprid) const;

    /** The int value of the expression, if it has one */
    bool getIntValue(nonneg int exprid, MathLib::bigint& result) const;
    void setIntValue(const Token* expr, MathLib::bigint value, bool impossible = false);

    /** The container size of the expression, if it has one */
    bool getContainerSizeValue(nonneg int exprid, MathLib::bigint& result) const;
    /** Is the container empty? Decided from the size or from the recorded size constraints. */
    bool getContainerEmptyValue(nonneg int exprid, MathLib::bigint& result) const;
    void setContainerSizeValue(const Token* expr, MathLib::bigint value, bool equal = true);

    void setUnknown(const Token* expr);

    /** The token value of the expression, if it has one */
    bool getTokValue(nonneg int exprid, const Token*& result) const;
    bool hasValue(nonneg int exprid) const;

    const Values& at(nonneg int exprid) const;
    Values& at(nonneg int exprid);

    void erase_if(const std::function<bool(const ExprIdToken&)>& pred);

    void swap(ProgramMemory &pm) noexcept;

    void clear();

    bool empty() const;

    void replace(ProgramMemory pm, bool skipUnknown = false);

    Map::const_iterator begin() const {
        return mValues->cbegin();
    }

    Map::const_iterator end() const {
        return mValues->cend();
    }

    friend bool operator==(const ProgramMemory& x, const ProgramMemory& y) {
        return x.mValues == y.mValues;
    }

    friend bool operator!=(const ProgramMemory& x, const ProgramMemory& y) {
        return x.mValues != y.mValues;
    }

private:
    /** Record the value for exactly this expression, without solving the expression */
    void record(const Token* expr, const ValueFlow::Value& value);
    void copyOnWrite();
    Map::const_iterator find(nonneg int exprid) const;
    Map::iterator find(nonneg int exprid);

    std::shared_ptr<Map> mValues;
};

struct CPPCHECKLIB ProgramMemoryState {
    struct ChangedKeyHash {
        std::size_t operator()(const std::tuple<const Token*, const Token*, const Token*>& t) const
        {
            const std::hash<const Token*> h;
            std::size_t seed = h(std::get<0>(t));
            seed ^= h(std::get<1>(t)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h(std::get<2>(t)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
    using ChangedCache =
        std::unordered_map<std::tuple<const Token*, const Token*, const Token*>, const Token*, ChangedKeyHash>;
    // The token modifying expr between start and end, or nullptr.
    using FindChangedFn = std::function<const Token*(const Token* expr, const Token* start, const Token* end)>;

    ProgramMemory state;
    std::map<nonneg int, const Token*> origins;
    const Settings& settings;
    // Memoized findExpressionChanged() pre-filter; structural, so never invalidated.
    std::shared_ptr<ChangedCache> changedCache;

    explicit ProgramMemoryState(const Settings& s);

    void replace(ProgramMemory pm, const Token* origin = nullptr);

    void addState(const Token* tok, const ProgramMemory::Map& vars);

    void assume(const Token* tok, bool b, bool isEmpty = false, const Token* origin = nullptr);

    void removeModifiedVars(const Token* tok);

    // A findExpressionChanged() closure memoized in changedCache
    FindChangedFn getCachedFindExpressionChanged(bool skipDeadCode) const;

    ProgramMemory get(const Token* tok, const Token* ctx, const ProgramMemory::Map& vars) const;
};

std::vector<ValueFlow::Value> execute(const Scope* scope, ProgramMemory& pm, const Settings& settings);

CPPCHECKLIB void execute(const Token* expr,
                         ProgramMemory& programMemory,
                         MathLib::bigint* result,
                         bool* error,
                         const Settings& settings,
                         const ProgramMemory::Map& vars = {});

/**
 * Is condition always false when variable has given value?
 * \param condition   top ast token in condition
 * \param pm   program memory
 * \param vars  optional tracked values that take precedence over the program memory
 */
bool conditionIsFalse(const Token* condition,
                      ProgramMemory pm,
                      const Settings& settings,
                      const ProgramMemory::Map& vars = {});

/**
 * Is condition always true when variable has given value?
 * \param condition   top ast token in condition
 * \param pm   program memory
 * \param vars  optional tracked values that take precedence over the program memory
 */
bool conditionIsTrue(const Token* condition,
                     ProgramMemory pm,
                     const Settings& settings,
                     const ProgramMemory::Map& vars = {});

/**
 * Get program memory by looking backwards from given token.
 */
ProgramMemory getProgramMemory(const Token* tok, const Token* expr, const ValueFlow::Value& value, const Settings& settings);

ValueFlow::Value evaluateLibraryFunction(const std::unordered_map<nonneg int, ValueFlow::Value>& args,
                                         const std::string& returnValue,
                                         const Settings& settings,
                                         bool cpp);

#endif



