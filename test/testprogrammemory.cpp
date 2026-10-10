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

#include "config.h"
#include "fixture.h"
#include "helpers.h"
#include "mathlib.h"
#include "settings.h"
#include "token.h"
#include "programmemory.h"
#include "utils.h"
#include "vfvalue.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

class TestProgramMemory : public TestFixture {
public:
    TestProgramMemory() : TestFixture("TestProgramMemory") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").build();

    void run() override {
        TEST_CASE(copyOnWrite);
        TEST_CASE(hasValue);
        TEST_CASE(getValue);
        TEST_CASE(at);
        TEST_CASE(setValueConstraints);
        TEST_CASE(setValueReplacesConstraints);
        TEST_CASE(containerEmpty);
        TEST_CASE(executeRange);
        TEST_CASE(executeScaledRange);
        TEST_CASE(executeSolvedRange);
        TEST_CASE(executeCompoundAssignment);
        TEST_CASE(executeRangeAtLimit);
        TEST_CASE(executeUnsignedRange);
        TEST_CASE(executeContainerAlias);
        TEST_CASE(executeContainerSizeRange);
    }

    static ValueFlow::Value impossible(MathLib::bigint x, ValueFlow::Value::Bound bound = ValueFlow::Value::Bound::Point) {
        ValueFlow::Value v{x, bound};
        v.setImpossible();
        return v;
    }

    // The constraint "x > lower": the values up to lower are impossible
    static ValueFlow::Value greaterThan(MathLib::bigint lower) {
        return impossible(lower, ValueFlow::Value::Bound::Upper);
    }

    // The constraint "x < upper": the values from upper on are impossible
    static ValueFlow::Value lessThan(MathLib::bigint upper) {
        return impossible(upper, ValueFlow::Value::Bound::Lower);
    }

    static ValueFlow::Value containerSize(ValueFlow::Value v) {
        v.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
        return v;
    }

    static bool hasValue(const ProgramMemory::Values& values, MathLib::bigint x, ValueFlow::Value::Bound bound) {
        return std::any_of(values.cbegin(), values.cend(), [&](const ValueFlow::Value& v) {
            return v.intvalue == x && v.bound == bound;
        });
    }

    void copyOnWrite() const {
        SimpleTokenList tokenlist("1+1;\n");
        Token* tok = tokenlist.front();
        const nonneg int id = 123;
        tok->exprId(id);

        ProgramMemory pm;
        const ValueFlow::Value* v = pm.getValue(id);
        ASSERT(!v);
        pm.setValue(tok, ValueFlow::Value{41});

        v = pm.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(41, v->intvalue);

        // create a copy
        ProgramMemory pm2 = pm;

        // make sure the value was copied
        v = pm2.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(41, v->intvalue);

        // set a value in the copy to trigger copy-on-write
        pm2.setValue(tok, ValueFlow::Value{42});

        // make another copy and set another value
        ProgramMemory pm3 = pm2;

        // set a value in the copy to trigger copy-on-write
        pm3.setValue(tok, ValueFlow::Value{43});

        // make sure the value was set
        v = pm2.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(42, v->intvalue);

        // make sure the value was set
        v = pm3.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(43, v->intvalue);

        // make sure the original value remains unchanged
        v = pm.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(41, v->intvalue);
    }

    void hasValue() const {
        ProgramMemory pm;
        ASSERT(!pm.hasValue(123));
    }

    void getValue() const {
        ProgramMemory pm;
        ASSERT(!pm.getValue(123));
        ASSERT(!pm.getValues(123));
    }

    void at() const {
        ProgramMemory pm;
        ASSERT_THROW_EQUALS(pm.at(123), std::out_of_range, "ProgramMemory::at");
        ASSERT_THROW_EQUALS(utils::as_const(pm).at(123), std::out_of_range, "ProgramMemory::at");
    }

    void setValueConstraints() const {
        SimpleTokenList tokenlist("1+1;\n");
        Token* tok = tokenlist.front();
        const nonneg int id = 123;
        tok->exprId(id);

        ProgramMemory pm;
        // x > 3 and x < 10 hold at the same time
        pm.setValue(tok, greaterThan(3));
        pm.setValue(tok, lessThan(10));
        const ProgramMemory::Values* values = pm.getValues(id);
        ASSERT(values);
        ASSERT_EQUALS(2U, values->size());
        ASSERT(hasValue(*values, 3, ValueFlow::Value::Bound::Upper));
        ASSERT(hasValue(*values, 10, ValueFlow::Value::Bound::Lower));

        // several constraints are not a single value
        ASSERT(!pm.getValue(id));
        ASSERT(!pm.getValue(id, true));
        MathLib::bigint i = 0;
        ASSERT(!pm.getIntValue(id, i));

        // a repeated constraint is not added again
        pm.setValue(tok, greaterThan(3));
        ASSERT_EQUALS(2U, pm.at(id).size());

        // a weaker bound is dropped
        pm.setValue(tok, greaterThan(1));
        ASSERT_EQUALS(2U, pm.at(id).size());
        ASSERT(hasValue(pm.at(id), 3, ValueFlow::Value::Bound::Upper));

        // a stronger bound replaces the bound
        pm.setValue(tok, greaterThan(5));
        ASSERT_EQUALS(2U, pm.at(id).size());
        ASSERT(hasValue(pm.at(id), 5, ValueFlow::Value::Bound::Upper));
        ASSERT(!hasValue(pm.at(id), 3, ValueFlow::Value::Bound::Upper));

        // an impossible value inside the range is kept
        pm.setValue(tok, impossible(7));
        ASSERT_EQUALS(3U, pm.at(id).size());
        ASSERT(hasValue(pm.at(id), 7, ValueFlow::Value::Bound::Point));

        // x > 5 and x != 6 is x > 6, and then x != 7 makes it x > 7
        pm.setValue(tok, impossible(6));
        ASSERT_EQUALS(2U, pm.at(id).size());
        ASSERT(hasValue(pm.at(id), 7, ValueFlow::Value::Bound::Upper));
        ASSERT(hasValue(pm.at(id), 10, ValueFlow::Value::Bound::Lower));
    }

    void setValueReplacesConstraints() const {
        SimpleTokenList tokenlist("1+1;\n");
        Token* tok = tokenlist.front();
        const nonneg int id = 123;
        tok->exprId(id);

        ProgramMemory pm;
        pm.setValue(tok, greaterThan(3));
        pm.setValue(tok, lessThan(10));

        // a value of the expression replaces its constraints
        pm.setValue(tok, ValueFlow::Value{5});
        MathLib::bigint i = 0;
        ASSERT(pm.getIntValue(id, i));
        ASSERT_EQUALS(5, i);
        ASSERT_EQUALS(1U, pm.at(id).size());

        // a constraint the value satisfies keeps the value
        pm.setValue(tok, greaterThan(3));
        pm.setValue(tok, impossible(7));
        ASSERT(pm.getIntValue(id, i));
        ASSERT_EQUALS(5, i);

        // a constraint the value violates replaces the value
        pm.setValue(tok, impossible(5));
        ASSERT(!pm.getIntValue(id, i));
        ASSERT_EQUALS(1U, pm.at(id).size());
        ASSERT(pm.at(id).front().isImpossible());

        // a possible value with a bound is a value of the expression
        pm.setValue(tok, ValueFlow::Value{4, ValueFlow::Value::Bound::Lower});
        ASSERT(pm.getIntValue(id, i));
        ASSERT_EQUALS(4, i);

        // a value of another type replaces the value
        pm.setValue(tok, containerSize(ValueFlow::Value{3}));
        ASSERT(!pm.getIntValue(id, i));
        ASSERT(pm.getContainerSizeValue(id, i));
        ASSERT_EQUALS(3, i);

        pm.setUnknown(tok);
        ASSERT(pm.hasValue(id));
        ASSERT(pm.getValue(id));
        ASSERT(pm.getValue(id)->isUninitValue());
    }

    void containerEmpty() const {
        SimpleTokenList tokenlist("1+1;\n");
        Token* tok = tokenlist.front();
        const nonneg int id = 123;
        tok->exprId(id);

        ProgramMemory pm;
        MathLib::bigint empty = -1;
        ASSERT(!pm.getContainerEmptyValue(id, empty));

        pm.setContainerSizeValue(tok, 0);
        ASSERT(pm.getContainerEmptyValue(id, empty));
        ASSERT_EQUALS(1, empty);

        pm.setContainerSizeValue(tok, 3);
        ASSERT(pm.getContainerEmptyValue(id, empty));
        ASSERT_EQUALS(0, empty);

        // size != 0
        pm.clear();
        pm.setContainerSizeValue(tok, 0, false);
        ASSERT(pm.getContainerEmptyValue(id, empty));
        ASSERT_EQUALS(0, empty);

        // size > 2
        pm.clear();
        pm.setValue(tok, containerSize(greaterThan(2)));
        ASSERT(pm.getContainerEmptyValue(id, empty));
        ASSERT_EQUALS(0, empty);

        // size < 1
        pm.clear();
        pm.setValue(tok, containerSize(lessThan(1)));
        ASSERT(pm.getContainerEmptyValue(id, empty));
        ASSERT_EQUALS(1, empty);

        // size < 5 does not decide it
        pm.clear();
        pm.setValue(tok, containerSize(lessThan(5)));
        ASSERT(!pm.getContainerEmptyValue(id, empty));
    }

    // Remove the values ValueFlow attached to the tokens, so that only the program memory decides.
    // Numbers keep their value, as they always have it.
    static void clearValues(SimpleTokenizer& tokenizer) {
        for (Token* tok = tokenizer.list.front(); tok; tok = tok->next()) {
            if (!tok->isNumber())
                tok->clearValueFlow();
        }
    }

    // The right hand sides of the assignments to the variable, in order
    static std::vector<const Token*> assignedExpressions(const Token* tokens, const std::string& var) {
        std::vector<const Token*> result;
        for (const Token* tok = tokens; tok; tok = tok->next()) {
            if (tok->str() == "=" && tok->astOperand1() && tok->astOperand1()->str() == var && tok->astOperand2())
                result.push_back(tok->astOperand2());
        }
        return result;
    }

    // Evaluate the expression with the program memory. The result as a string, empty if it is unknown.
    std::string evaluate(const Token* expr, ProgramMemory pm) const {
        MathLib::bigint result = 0;
        bool error = false;
        execute(expr, pm, &result, &error, settings);
        return error ? "" : std::to_string(result);
    }

    // Evaluate the expression with the program memory built from the conditions enclosing it
    std::string evaluate(const Token* expr) const {
        ProgramMemoryState pms(settings);
        pms.addState(expr, {});
        return evaluate(expr, pms.state);
    }

    // The results of the expressions assigned to y in the code, each evaluated at its position
    // Tokenize the code and return the expressions assigned to y, in order. The ValueFlow values are
    // removed so that only the program memory decides, unless they are kept.
    std::vector<const Token*> parseAssignments(SimpleTokenizer& tokenizer, const char code[], bool clear = true) const {
        ASSERT(tokenizer.tokenize(code));
        if (clear)
            clearValues(tokenizer);
        return assignedExpressions(tokenizer.tokens(), "y");
    }

    // Run the statement of the operator on the program memory
    void step(const Token* op, ProgramMemory& pm) const {
        execute(op, pm, nullptr, nullptr, settings);
    }

    std::vector<std::string> evaluateAssignments(const char code[]) {
        SimpleTokenizer tokenizer(settings, *this);
        const std::vector<const Token*> exprs = parseAssignments(tokenizer, code);
        std::vector<std::string> results;
        std::transform(exprs.cbegin(), exprs.cend(), std::back_inserter(results), [&](const Token* expr) {
            return evaluate(expr);
        });
        return results;
    }

    void executeRange() {
        const char code[] = "void f(int x, int y) {\n"
                            "    if (x > 3) {\n"
                            "        if (x < 10) {\n"
                            "            y = x == 15;\n"
                            "            y = x == 5;\n"
                            "            y = x < 20;\n"
                            "            y = x >= 4;\n"
                            "            y = x + 1 > 4;\n"
                            "            y = 10 - x < 7;\n"
                            "            y = -x < 0;\n"
                            "            y = x;\n"
                            "            y = x + 1 < 20;\n"
                            "            y = -x > -20;\n"
                            "            y = (long)x < 20;\n"
                            "            y = (x > 0 ? x : 0) < 20;\n"
                            "            y = 2 * x - 1 == 3;\n"
                            "        }\n"
                            "    }\n"
                            "}\n";
        const std::vector<std::string> results = evaluateAssignments(code);
        ASSERT_EQUALS(13U, results.size());
        // 3 < x < 10
        ASSERT_EQUALS("0", results[0]);
        ASSERT_EQUALS("", results[1]);
        ASSERT_EQUALS("1", results[2]);
        ASSERT_EQUALS("1", results[3]);
        // the range is shifted by arithmetic
        ASSERT_EQUALS("1", results[4]);
        ASSERT_EQUALS("1", results[5]);
        ASSERT_EQUALS("1", results[6]);
        // a range is not a value
        ASSERT_EQUALS("", results[7]);
        // both bounds follow the value through arithmetic, casts and conditionals
        ASSERT_EQUALS("1", results[8]);
        ASSERT_EQUALS("1", results[9]);
        ASSERT_EQUALS("1", results[10]);
        ASSERT_EQUALS("1", results[11]);
        ASSERT_EQUALS("0", results[12]);
    }

    void executeScaledRange() {
        const char code[] = "void f(int x, int y) {\n"
                            "    if (x > 3) {\n"
                            "        y = x * 2 > 6;\n"
                            "        y = x * 2 == 7;\n"
                            "        y = -2 * x < -6;\n"
                            "        y = x * 0 == 0;\n"
                            "        y = (x << 1) >= 8;\n"
                            "        y = x % 2 == 0;\n"
                            "        y = (x >> 1) == 1;\n"
                            "    }\n"
                            "    if (x > 6) {\n"
                            "        y = x / 2 > 2;\n"
                            "        y = x / -2 < -2;\n"
                            "    }\n"
                            "}\n";
        const std::vector<std::string> results = evaluateAssignments(code);
        ASSERT_EQUALS(9U, results.size());
        // x > 3: x * 2 >= 8
        ASSERT_EQUALS("1", results[0]);
        ASSERT_EQUALS("0", results[1]);
        ASSERT_EQUALS("1", results[2]);
        // x * 0 is not "not zero"
        ASSERT_EQUALS("", results[3]);
        ASSERT_EQUALS("1", results[4]);
        // the remainder does not keep the range; x >> 1 >= 2
        ASSERT_EQUALS("", results[5]);
        ASSERT_EQUALS("0", results[6]);
        // x > 6: x / 2 >= 3
        ASSERT_EQUALS("1", results[7]);
        ASSERT_EQUALS("1", results[8]);
    }

    void executeSolvedRange() {
        const char code[] = "void f(int x, int y) {\n"
                            "    if (x * 2 < 3) {\n"
                            "        y = x <= 1;\n"
                            "        y = x == 1;\n"
                            "        y = x == 2;\n"
                            "    }\n"
                            "    if (x * 3 >= 7) {\n"
                            "        y = x >= 3;\n"
                            "        y = x == 2;\n"
                            "    }\n"
                            "    if (-2 * x > 3) {\n"
                            "        y = x <= -2;\n"
                            "        y = x == -1;\n"
                            "    }\n"
                            "    if ((x ^ 4) > 3) {\n"
                            "        y = x == 0;\n"
                            "    }\n"
                            "}\n";
        const std::vector<std::string> results = evaluateAssignments(code);
        ASSERT_EQUALS(8U, results.size());
        // x * 2 < 3: x <= 1
        ASSERT_EQUALS("1", results[0]);
        ASSERT_EQUALS("", results[1]);
        ASSERT_EQUALS("0", results[2]);
        // x * 3 >= 7: x >= 3
        ASSERT_EQUALS("1", results[3]);
        ASSERT_EQUALS("0", results[4]);
        // -2 * x > 3: x <= -2
        ASSERT_EQUALS("1", results[5]);
        ASSERT_EQUALS("0", results[6]);
        // (x ^ 4) > 3 does not give a range for x
        ASSERT_EQUALS("", results[7]);
    }

    void executeCompoundAssignment() {
        const char code[] = "void f(int x, unsigned u, int y) {\n"
                            "    x *= -1;\n"
                            "    y = x < -3;\n"
                            "    u--;\n"
                            "    y = u > 100;\n"
                            "}\n";
        SimpleTokenizer tokenizer(settings, *this);
        const std::vector<const Token*> exprs = parseAssignments(tokenizer, code);
        ASSERT_EQUALS(2U, exprs.size());
        const Token* xtok = Token::findsimplematch(tokenizer.tokens(), "x *=");
        const Token* utok = Token::findsimplematch(tokenizer.tokens(), "u --");
        ASSERT(xtok && utok);

        ProgramMemory pm;
        // x > 3, then x *= -1: x < -3
        pm.setValue(xtok, greaterThan(3));
        step(xtok->next(), pm);
        ASSERT_EQUALS("1", evaluate(exprs[0], pm));

        // u < 1, then u--: the value wraps around, nothing is known
        pm.setValue(utok, lessThan(1));
        step(utok->next(), pm);
        ASSERT_EQUALS("", evaluate(exprs[1], pm));

        // u > 3, then u--: u > 2
        pm.setValue(utok, greaterThan(3));
        step(utok->next(), pm);
        ASSERT_EQUALS("", evaluate(exprs[1], pm));
        const ProgramMemory::Values* values = pm.getValues(utok->exprId());
        ASSERT(values);
        ASSERT_EQUALS(1U, values->size());
        ASSERT(hasValue(*values, 2, ValueFlow::Value::Bound::Upper));
    }

    // Arithmetic on a range at the limit of the type does not overflow in the analyzer
    void executeRangeAtLimit() {
        const char code[] = "void f(long long x, long long y) {\n"
                            "    y = x + 100 < 0;\n"
                            "    y = x - 100 > 0;\n"
                            "    y = 100 - x > 0;\n"
                            "    y = x * 2 > 0;\n"
                            "    y = (x << 1) > 0;\n"
                            "    x++;\n"
                            "    y = x > 0;\n"
                            "}\n";
        SimpleTokenizer tokenizer(settings, *this);
        const std::vector<const Token*> exprs = parseAssignments(tokenizer, code);
        ASSERT_EQUALS(6U, exprs.size());
        const Token* xtok = Token::findsimplematch(tokenizer.tokens(), "x +");
        const Token* inc = Token::findsimplematch(tokenizer.tokens(), "x ++");
        ASSERT(xtok && inc);
        const MathLib::bigint max = std::numeric_limits<MathLib::bigint>::max();
        const MathLib::bigint min = std::numeric_limits<MathLib::bigint>::min();

        // x > max - 10
        ProgramMemory pm;
        pm.setValue(xtok, greaterThan(max - 10));
        ASSERT_EQUALS("", evaluate(exprs[0], pm));
        ASSERT_EQUALS("1", evaluate(exprs[1], pm));
        ASSERT_EQUALS("0", evaluate(exprs[2], pm));
        ASSERT_EQUALS("", evaluate(exprs[3], pm));
        ASSERT_EQUALS("", evaluate(exprs[4], pm));

        // x < min + 10
        pm.clear();
        pm.setValue(xtok, lessThan(min + 10));
        ASSERT_EQUALS("1", evaluate(exprs[0], pm));
        ASSERT_EQUALS("", evaluate(exprs[1], pm));
        ASSERT_EQUALS("", evaluate(exprs[2], pm));
        ASSERT_EQUALS("", evaluate(exprs[3], pm));
        ASSERT_EQUALS("", evaluate(exprs[4], pm));

        // x is the largest value: incrementing it is unknown
        pm.clear();
        pm.setValue(xtok, ValueFlow::Value{max});
        step(inc->next(), pm);
        ASSERT_EQUALS("", evaluate(exprs[5], pm));

        // x > max - 2: incrementing moves the bound to the limit, which is still not known to overflow
        pm.setValue(xtok, greaterThan(max - 2));
        step(inc->next(), pm);
        ASSERT_EQUALS("1", evaluate(exprs[5], pm));
    }

    // The range of an unsigned expression is kept only when it cannot wrap around
    void executeUnsignedRange() {
        const char code[] = "void f(unsigned u, unsigned y) {\n"
                            "    if (u > 3) {\n"
                            "        y = u + 1 > 4;\n"
                            "        y = u - 1 > 2;\n"
                            "        y = u * 2 > 6;\n"
                            "    }\n"
                            "    if (u < 10) {\n"
                            "        y = u - 1 < 9;\n"
                            "        y = 10 - u > 0;\n"
                            "        y = u + 1 < 11;\n"
                            "    }\n"
                            "    if (u > 3 && u < 10) {\n"
                            "        y = u + 1 > 4;\n"
                            "        y = u * 2 < 20;\n"
                            "    }\n"
                            "}\n";
        const std::vector<std::string> results = evaluateAssignments(code);
        ASSERT_EQUALS(8U, results.size());
        // u > 3: adding and multiplying may wrap around, subtracting may not
        ASSERT_EQUALS("", results[0]);
        ASSERT_EQUALS("1", results[1]);
        ASSERT_EQUALS("", results[2]);
        // u < 10: subtracting may wrap around, the others may not
        ASSERT_EQUALS("", results[3]);
        ASSERT_EQUALS("1", results[4]);
        ASSERT_EQUALS("1", results[5]);
        // 3 < u < 10
        ASSERT_EQUALS("1", results[6]);
        ASSERT_EQUALS("1", results[7]);

        const char code2[] = "void f(unsigned u, unsigned y) {\n"
                             "    u++;\n"
                             "    y = u > 4;\n"
                             "}\n";
        SimpleTokenizer tokenizer(settings, *this);
        const std::vector<const Token*> exprs = parseAssignments(tokenizer, code2);
        ASSERT_EQUALS(1U, exprs.size());
        const Token* utok = Token::findsimplematch(tokenizer.tokens(), "u ++");
        ASSERT(utok);
        // u > 3, then u++: u may have wrapped around
        ProgramMemory pm;
        pm.setValue(utok, greaterThan(3));
        step(utok->next(), pm);
        ASSERT_EQUALS("", evaluate(exprs[0], pm));
        // 3 < u < 10, then u++: u > 4
        pm.clear();
        pm.setValue(utok, greaterThan(3));
        pm.setValue(utok, lessThan(10));
        step(utok->next(), pm);
        ASSERT_EQUALS("1", evaluate(exprs[0], pm));
    }

    // The size constraints of a container reach a container that is symbolically equal to it
    void executeContainerAlias() {
        const char code[] = "void f(std::string s, bool y) {\n"
                            "    std::string t = s;\n"
                            "    y = t.size() == 15;\n"
                            "    y = t.size() < 20;\n"
                            "}\n";
        // the symbolic value of t is kept
        SimpleTokenizer tokenizer(settings, *this);
        const std::vector<const Token*> exprs = parseAssignments(tokenizer, code, /*clear*/ false);
        ASSERT_EQUALS(2U, exprs.size());
        const Token* stok = Token::findsimplematch(tokenizer.tokens(), "s ;");
        ASSERT(stok);
        // 3 < s.size() < 10
        ProgramMemory pm;
        pm.setValue(stok, containerSize(greaterThan(3)));
        pm.setValue(stok, containerSize(lessThan(10)));
        ASSERT_EQUALS("0", evaluate(exprs[0], pm));
        ASSERT_EQUALS("1", evaluate(exprs[1], pm));
    }

    void executeContainerSizeRange() {
        const char code[] = "void f(std::string s, bool y) {\n"
                            "    if (s.size() > 3) {\n"
                            "        if (s.size() < 10) {\n"
                            "            y = s.size() == 15;\n"
                            "            y = s.size() < 20;\n"
                            "            y = s.empty();\n"
                            "            y = s.size() == 5;\n"
                            "        }\n"
                            "    }\n"
                            "}\n";
        const std::vector<std::string> results = evaluateAssignments(code);
        ASSERT_EQUALS(4U, results.size());
        // 3 < s.size() < 10
        ASSERT_EQUALS("0", results[0]);
        ASSERT_EQUALS("1", results[1]);
        ASSERT_EQUALS("0", results[2]);
        ASSERT_EQUALS("", results[3]);
    }
};

REGISTER_TEST(TestProgramMemory)
