/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef vfAnalyzers
#define vfAnalyzers

#include "analyzer.h"
#include "valueptr.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Token;
class Variable;
class Settings;
namespace ValueFlow {
    class Value;
}

ValuePtr<Analyzer> makeMultiValueFlowAnalyzer(const std::unordered_map<const Variable*, ValueFlow::Value>& args, const Settings& settings);

ValuePtr<Analyzer> makeSameExpressionAnalyzer(const Token* e, ValueFlow::Value val, const Settings& s);

ValuePtr<Analyzer> makeOppositeExpressionAnalyzer(bool pIsNot, const Token* e, ValueFlow::Value val, const Settings& s);

using PartialReadContainer = std::vector<std::pair<Token *, ValueFlow::Value>>;
ValuePtr<Analyzer> makeMemberExpressionAnalyzer(std::string varname, const Token* e, ValueFlow::Value val, const std::shared_ptr<PartialReadContainer>& p, const Settings& s);

ValuePtr<Analyzer> makeAnalyzer(const Token* exprTok, ValueFlow::Value value, const Settings& settings);

ValuePtr<Analyzer> makeReverseAnalyzer(const Token* exprTok, ValueFlow::Value value, const Settings& settings);

#endif // vfAnalyzers
