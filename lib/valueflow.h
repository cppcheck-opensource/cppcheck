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

//---------------------------------------------------------------------------
#ifndef valueflowH
#define valueflowH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"
#include "mathlib.h"

#include <functional>
#include <list>
#include <string>
#include <utility>
#include <vector>

class ErrorLogger;
class Settings;
class SymbolDatabase;
class TimerResultsIntf;
class Token;
class TokenList;
class Variable;
class Scope;

namespace ValueFlow
{
    class Value;
}

namespace ValueFlow {
    /// Constant folding of expression. This can be used before the full ValueFlow has been executed (ValueFlow::setValues).
    const Value * valueFlowConstantFoldAST(Token *expr, const Settings &settings);

    /// Perform valueflow analysis.
    void setValues(TokenList& tokenlist,
                   SymbolDatabase& symboldatabase,
                   ErrorLogger& errorLogger,
                   const Settings& settings,
                   TimerResultsIntf* timerResults);

    std::string eitherTheConditionIsRedundant(const Token *condition);

    const Value* findValue(const std::list<Value>& values,
                           const Settings& settings,
                           const std::function<bool(const Value&)> &pred);

    std::vector<Value> isOutOfBounds(const Value& size, const Token* indexTok, bool possible = true);

    Value asImpossible(Value v);

    bool isContainerSizeChanged(const Token* tok, int indirect, const Settings& settings, int depth = 20);

    struct LifetimeToken {
        const Token* token{};
        ErrorPath errorPath;
        bool addressOf{};
        bool inconclusive{};

        LifetimeToken() = default;

        LifetimeToken(const Token* token, ErrorPath errorPath)
            : token(token), errorPath(std::move(errorPath))
        {}

        LifetimeToken(const Token* token, bool addressOf, ErrorPath errorPath)
            : token(token), errorPath(std::move(errorPath)), addressOf(addressOf)
        {}

        static std::vector<LifetimeToken> setAddressOf(std::vector<LifetimeToken> v, bool b) {
            for (LifetimeToken& x : v)
                x.addressOf = b;
            return v;
        }

        static std::vector<LifetimeToken> setInconclusive(std::vector<LifetimeToken> v, bool b) {
            for (LifetimeToken& x : v)
                x.inconclusive = b;
            return v;
        }
    };

    const Token *parseCompareInt(const Token *tok, Value &true_value, Value &false_value, const std::function<std::vector<MathLib::bigint>(const Token*)>& evaluate);
    const Token *parseCompareInt(const Token *tok, Value &true_value, Value &false_value);

    const Token* solveExprValue(const Token* expr,
                                const std::function<std::vector<MathLib::bigint>(const Token*)>& eval,
                                Value& value);

    std::vector<LifetimeToken> getLifetimeTokens(const Token* tok,
                                                 const Settings& settings,
                                                 bool escape = false,
                                                 ErrorPath errorPath = ErrorPath{});

    bool hasLifetimeToken(const Token* tok, const Token* lifetime, const Settings& settings);

    const Variable* getLifetimeVariable(const Token* tok, ErrorPath& errorPath, const Settings& settings, bool* addressOf = nullptr);

    const Variable* getLifetimeVariable(const Token* tok, const Settings& settings);

    bool isLifetimeBorrowed(const Token *tok, const Settings &settings);

    std::string lifetimeMessage(const Token *tok, const Value *val, ErrorPath &errorPath);

    CPPCHECKLIB Value getLifetimeObjValue(const Token *tok, bool inconclusive = false);

    CPPCHECKLIB std::vector<Value> getLifetimeObjValues(const Token* tok,
                                                        bool inconclusive = false,
                                                        MathLib::bigint path = 0);

    const Token* getEndOfExprScope(const Token* tok, const Scope* defaultScope = nullptr, bool smallest = true);

    void combineValueProperties(const Value& value1, const Value& value2, Value& result);
}

#endif // valueflowH
