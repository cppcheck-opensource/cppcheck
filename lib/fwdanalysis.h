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

//---------------------------------------------------------------------------
#ifndef fwdanalysisH
#define fwdanalysisH
//---------------------------------------------------------------------------

class Token;
class Settings;

/**
 * Forward data flow analysis for checks
 *  - unused value
 *  - redundant assignment
 */
struct FwdAnalysis {
    static bool hasOperand(const Token *tok, const Token *lhs, const Settings& settings);

    /**
     * Check if "expr" is reassigned. The "expr" can be a tree (x.y[12]).
     * @param expr Symbolic expression to perform forward analysis for
     * @param startToken First token in forward analysis
     * @param endToken Last token in forward analysis
     * @param settings the settings to use
     * @return Token where expr is reassigned. If it's not reassigned then nullptr is returned.
     */
    static const Token *reassign(const Token *expr, const Token *startToken, const Token *endToken, const Settings& settings);

    /**
     * Check if "expr" is used. The "expr" can be a tree (x.y[12]).
     * @param expr Symbolic expression to perform forward analysis for
     * @param startToken First token in forward analysis
     * @param endToken Last token in forward analysis
     * @param settings the settings to use
     * @return true if expr is used.
     */
    static bool unusedValue(const Token *expr, const Token *startToken, const Token *endToken, const Settings& settings);
};

#endif // fwdanalysisH
