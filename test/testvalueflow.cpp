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

#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "token.h"
#include "vfvalue.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class TestValueFlow : public TestFixture {
public:
    TestValueFlow() : TestFixture("TestValueFlow") {}

private:
    /*const*/ Settings settings = settingsBuilder().library("std.cfg").build();

    void run() override {
        // strcpy, abort cfg
        constexpr char cfg[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"strcpy\"> <arg nr=\"1\"><not-null/></arg> </function>\n"
                               "  <function name=\"abort\"> <noreturn>true</noreturn> </function>\n" // abort is a noreturn function
                               "</def>";
        settings = settingsBuilder(settings).libraryxml(cfg).build();

        mNewTemplate = true;
        TEST_CASE(valueFlowNumber);
        TEST_CASE(valueFlowString);
        TEST_CASE(valueFlowTypeTraits);
        TEST_CASE(valueFlowPointerAlias);
        TEST_CASE(valueFlowLifetime);
        TEST_CASE(valueFlowArrayElement);
        TEST_CASE(valueFlowMove);

        TEST_CASE(valueFlowBitAnd);
        TEST_CASE(valueFlowRightShift);

        TEST_CASE(valueFlowCalculations);
        TEST_CASE(valueFlowSizeof);
        TEST_CASE(valueFlowComma);

        TEST_CASE(valueFlowErrorPath);

        TEST_CASE(valueFlowBeforeCondition);
        TEST_CASE(valueFlowBeforeConditionAndAndOrOrGuard);
        TEST_CASE(valueFlowBeforeConditionAssignIncDec);
        TEST_CASE(valueFlowBeforeConditionFunctionCall);
        TEST_CASE(valueFlowBeforeConditionGlobalVariables);
        mNewTemplate = false;
        TEST_CASE(valueFlowBeforeConditionGoto);
        mNewTemplate = true;
        TEST_CASE(valueFlowBeforeConditionIfElse);
        TEST_CASE(valueFlowBeforeConditionLoop);
        mNewTemplate = false;
        TEST_CASE(valueFlowBeforeConditionMacro);
        mNewTemplate = true;
        TEST_CASE(valueFlowBeforeConditionSizeof);
        TEST_CASE(valueFlowBeforeConditionSwitch);
        TEST_CASE(valueFlowBeforeConditionTernaryOp);
        TEST_CASE(valueFlowBeforeConditionForward);
        TEST_CASE(valueFlowBeforeConditionConstructor);

        TEST_CASE(valueFlowAfterAssign);
        TEST_CASE(valueFlowAfterSwap);
        TEST_CASE(valueFlowAfterCondition);
        TEST_CASE(valueFlowAfterConditionTernary);
        TEST_CASE(valueFlowAfterConditionExpr);
        TEST_CASE(valueFlowAfterConditionSeveralNot);
        TEST_CASE(valueFlowForwardCompoundAssign);
        TEST_CASE(valueFlowForwardCorrelatedVariables);
        TEST_CASE(valueFlowForwardModifiedVariables);
        TEST_CASE(valueFlowForwardFunction);
        TEST_CASE(valueFlowForwardTernary);
        TEST_CASE(valueFlowForwardLambda);
        TEST_CASE(valueFlowForwardTryCatch);
        TEST_CASE(valueFlowForwardInconclusiveImpossible);
        TEST_CASE(valueFlowForwardConst);
        TEST_CASE(valueFlowForwardAfterCondition);

        TEST_CASE(valueFlowFwdAnalysis);

        TEST_CASE(valueFlowSwitchVariable);

        TEST_CASE(valueFlowForLoop);
        TEST_CASE(valueFlowSubFunction);
        TEST_CASE(valueFlowFunctionReturn);

        TEST_CASE(valueFlowFunctionDefaultParameter);

        TEST_CASE(knownValue);

        TEST_CASE(valueFlowSizeofForwardDeclaredEnum);

        TEST_CASE(valueFlowGlobalVar);

        TEST_CASE(valueFlowGlobalConstVar);

        TEST_CASE(valueFlowGlobalStaticVar);

        TEST_CASE(valueFlowInlineAssembly);

        TEST_CASE(valueFlowSameExpression);

        TEST_CASE(valueFlowUninit);

        TEST_CASE(valueFlowConditionExpressions);

        TEST_CASE(valueFlowContainerSize);
        TEST_CASE(valueFlowContainerElement);

        TEST_CASE(valueFlowDynamicBufferSize);

        TEST_CASE(valueFlowSafeFunctionParameterValues);
        TEST_CASE(valueFlowUnknownFunctionReturn);
        TEST_CASE(valueFlowUnknownFunctionReturnRand);
        TEST_CASE(valueFlowUnknownFunctionReturnMalloc);

        TEST_CASE(valueFlowPointerAliasDeref);

        TEST_CASE(valueFlowCrashIncompleteCode);

        TEST_CASE(valueFlowCrash);
        TEST_CASE(valueFlowHang);
        TEST_CASE(valueFlowCrashConstructorInitialization);

        TEST_CASE(valueFlowUnknownMixedOperators);
        TEST_CASE(valueFlowSolveExpr);
        TEST_CASE(valueFlowIdempotent);
        TEST_CASE(valueFlowUnsigned);
        TEST_CASE(valueFlowMod);
        TEST_CASE(valueFlowIncDec);
        TEST_CASE(valueFlowNotNull);
        TEST_CASE(valueFlowSymbolic);
        TEST_CASE(valueFlowSymbolicIdentity);
        TEST_CASE(valueFlowSymbolicStrlen);
        TEST_CASE(valueFlowSmartPointer);
        TEST_CASE(valueFlowImpossibleMinMax);
        TEST_CASE(valueFlowImpossibleIncDec);
        TEST_CASE(valueFlowImpossibleUnknownConstant);
        TEST_CASE(valueFlowContainerEqual);

        mNewTemplate = false;
        TEST_CASE(valueFlowBailoutIncompleteVar);
        TEST_CASE(valueFlowBailoutNoreturn);
        mNewTemplate = true;

        TEST_CASE(performanceIfCount);
        TEST_CASE(bitfields);

        TEST_CASE(bitfieldsHang);
    }

    static bool isNotTokValue(const ValueFlow::Value &val) {
        return !val.isTokValue();
    }

    // cppcheck-suppress unusedPrivateFunction
    static bool isNotLifetimeValue(const ValueFlow::Value& val) {
        return !val.isLifetimeValue();
    }

    static bool isNotUninitValue(const ValueFlow::Value& val) {
        return !val.isUninitValue();
    }

    static bool isNotPossible(const ValueFlow::Value& val) {
        return !val.isPossible();
    }

    static bool isNotKnown(const ValueFlow::Value& val) {
        return !val.isKnown();
    }

    static bool isNotInconclusive(const ValueFlow::Value& val) {
        return !val.isInconclusive();
    }

    static bool isNotImpossible(const ValueFlow::Value& val) {
        return !val.isImpossible();
    }

#define testValueOfXKnown(...) testValueOfXKnown_(__FILE__, __LINE__, __VA_ARGS__)
    bool testValueOfXKnown_(const char* file, int line, const char code[], unsigned int linenr, int value) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& val) {
                    if (val.isSymbolicValue())
                        return false;
                    if (val.isKnown() && val.intvalue == value)
                        return true;
                    return false;
                }))
                    return true;
            }
        }

        return false;
    }

    bool testValueOfXKnown_(const char* file, int line, const char code[], unsigned int linenr, const std::string& expr, int value) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token* tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& val) {
                    if (!val.isSymbolicValue())
                        return false;
                    if (val.isKnown() && val.intvalue == value && val.tokvalue->expressionString() == expr)
                        return true;
                    return false;
                }))
                    return true;
            }
        }

        return false;
    }

#define testValueOfXImpossible(...) testValueOfXImpossible_(__FILE__, __LINE__, __VA_ARGS__)
    bool testValueOfXImpossible_(const char* file, int line, const char code[], unsigned int linenr, int value) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& val) {
                    if (val.isSymbolicValue())
                        return false;
                    if (val.isImpossible() && val.intvalue == value)
                        return true;
                    return false;
                }))
                    return true;
            }
        }

        return false;
    }

    bool testValueOfXImpossible_(const char* file, int line, const char code[], unsigned int linenr, const std::string& expr, int value)
    {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token* tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& val) {
                    if (!val.isSymbolicValue())
                        return false;
                    if (val.isImpossible() && val.intvalue == value && val.tokvalue->expressionString() == expr)
                        return true;
                    return false;
                }))
                    return true;
            }
        }

        return false;
    }

#define testValueOfXInconclusive(...) testValueOfXInconclusive_(__FILE__, __LINE__, __VA_ARGS__)
    bool testValueOfXInconclusive_(const char* file, int line, const char code[], unsigned int linenr, int value) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& val) {
                    if (val.isSymbolicValue())
                        return false;
                    if (val.isInconclusive() && val.intvalue == value)
                        return true;
                    return false;
                }))
                    return true;
            }
        }

        return false;
    }

#define testValueOfX(...) testValueOfX_(__FILE__, __LINE__, __VA_ARGS__)
    bool testValueOfX_(const char* file, int line, const char code[], unsigned int linenr, MathLib::bigint value, const Settings *s = nullptr) {
        const Settings *settings1 = s ? s : &settings;

        // Tokenize..
        SimpleTokenizer tokenizer(*settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.isIntValue() && !v.isImpossible() && v.intvalue == value;
                }))
                    return true;
            }
        }

        return false;
    }

    bool testValueOfX_(const char* file, int line, const char code[], unsigned int linenr, const std::string& expr, int value)
    {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token* tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.isSymbolicValue() && !v.isImpossible() && v.intvalue == value && v.tokvalue->expressionString() == expr;
                }))
                    return true;
            }
        }

        return false;
    }

    bool testValueOfX_(const char* file, int line, const char code[], unsigned int linenr, double value, double diff) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.isFloatValue() && !v.isImpossible() && v.floatValue >= value - diff && v.floatValue <= value + diff;
                }))
                    return true;
            }
        }

        return false;
    }

#define getErrorPathForX(...) getErrorPathForX_(__FILE__, __LINE__, __VA_ARGS__)
    std::string getErrorPathForX_(const char* file, int line, const char code[], unsigned int linenr) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() != "x" || tok->linenr() != linenr)
                continue;

            std::ostringstream ostr;
            for (const ValueFlow::Value &v : tok->values()) {
                for (const ErrorPathItem &ep : v.errorPath) {
                    const Token *eptok = ep.first;
                    const std::string &msg = ep.second;
                    ostr << eptok->linenr() << ',' << msg << '\n';
                }
            }
            return ostr.str();
        }

        return "";
    }

    bool testValueOfX_(const char* file, int line, const char code[], unsigned int linenr, const char value[], ValueFlow::Value::ValueType type) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        const std::size_t len = strlen(value);
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.valueType == type && Token::simpleMatch(v.tokvalue, value, len);
                }))
                    return true;
            }
        }

        return false;
    }

#define testLifetimeOfX(...) testLifetimeOfX_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool testLifetimeOfX_(const char* file, int line, const char (&code)[size], unsigned int linenr, const char value[]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        const std::size_t len = strlen(value);
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.isLifetimeValue() && v.lifetimeScope == ValueFlow::Value::LifetimeScope::Local && Token::simpleMatch(v.tokvalue, value, len);
                }))
                    return true;
            }
        }

        return false;
    }

    bool testValueOfX_(const char* file, int line, const char code[], unsigned int linenr, int value, ValueFlow::Value::ValueType type) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.valueType == type && v.intvalue == value;
                }))
                    return true;
            }
        }

        return false;
    }

    bool testValueOfX_(const char* file, int line, const char code[], unsigned int linenr, ValueFlow::Value::MoveKind moveKind) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.isMovedValue() && v.moveKind == moveKind;
                }))
                    return true;
            }
        }

        return false;
    }

#define testConditionalValueOfX(...) testConditionalValueOfX_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool testConditionalValueOfX_(const char* file, int line, const char (&code)[size], unsigned int linenr, int value) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == "x" && tok->linenr() == linenr) {
                if (std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
                    return v.isIntValue() && v.intvalue == value && v.condition;
                }))
                    return true;
            }
        }

        return false;
    }

#define bailout(...) bailout_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void bailout_(const char* file, int line, const char (&code)[size]) {
        const Settings s = settingsBuilder().debugwarnings().build();

        SimpleTokenizer2 tokenizer(s, *this, code, "test.cpp");

        // Tokenize..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);
    }

#define tokenValues(...) tokenValues_(__FILE__, __LINE__, __VA_ARGS__)
    std::list<ValueFlow::Value> tokenValues_(const char* file, int line, const char code[], const char tokstr[], const Settings *s = nullptr, bool cpp = true) {
        SimpleTokenizer tokenizer(s ? *s : settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token *tok = Token::findmatch(tokenizer.tokens(), tokstr);
        return tok ? tok->values() : std::list<ValueFlow::Value>();
    }

    std::list<ValueFlow::Value> tokenValues_(const char* file, int line, const char code[], const char tokstr[], ValueFlow::Value::ValueType vt) {
        std::list<ValueFlow::Value> values = tokenValues_(file, line, code, tokstr);
        values.remove_if([&](const ValueFlow::Value& v) {
            return v.valueType != vt;
        });
        return values;
    }

#define lifetimeValues(...) lifetimeValues_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::vector<std::string> lifetimeValues_(const char* file, int line, const char (&code)[size], const char tokstr[]) {
        std::vector<std::string> result;
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token *tok = Token::findmatch(tokenizer.tokens(), tokstr);
        if (!tok)
            return result;
        for (const ValueFlow::Value& value:tok->values()) {
            if (!value.isLifetimeValue())
                continue;
            if (!value.tokvalue)
                continue;
            result.push_back(value.tokvalue->expressionString());
        }
        return result;
    }

#define valueOfTok(...) valueOfTok_(__FILE__, __LINE__, __VA_ARGS__)
    ValueFlow::Value valueOfTok_(const char* file, int line, const char code[], const char tokstr[], const Settings *s = nullptr, bool cpp = true) {
        std::list<ValueFlow::Value> values = removeImpossible(tokenValues_(file, line, code, tokstr, s, cpp));
        return values.size() == 1U && !values.front().isTokValue() ? values.front() : ValueFlow::Value();
    }

#define testKnownValueOfTok(...) testKnownValueOfTok_(__FILE__, __LINE__, __VA_ARGS__)
    bool testKnownValueOfTok_(const char* file,
                              int line,
                              const char code[],
                              const char tokstr[],
                              int value)
    {
        std::list<ValueFlow::Value> values = removeImpossible(tokenValues_(file, line, code, tokstr));
        return std::any_of(values.begin(), values.end(), [&](const ValueFlow::Value& v) {
            return v.isKnown() && v.isIntValue() && v.intvalue == value;
        });
    }

    static std::list<ValueFlow::Value> removeSymbolicTok(std::list<ValueFlow::Value> values)
    {
        values.remove_if([](const ValueFlow::Value& v) {
            return v.isSymbolicValue() || v.isTokValue();
        });
        return values;
    }

    static std::list<ValueFlow::Value> removeImpossible(std::list<ValueFlow::Value> values)
    {
        values.remove_if(std::mem_fn(&ValueFlow::Value::isImpossible));
        return values;
    }

    void valueFlowNumber() {
        ASSERT_EQUALS(123, valueOfTok("x=123;", "123").intvalue);
        ASSERT_EQUALS_DOUBLE(192.0, valueOfTok("x=0x0.3p10;", "0x0.3p10").floatValue, 1e-5); // 3 * 16^-1 * 2^10 = 192
        ASSERT(std::fabs(valueOfTok("x=0.5;", "0.5").floatValue - 0.5) < 0.1);
        ASSERT_EQUALS(10, valueOfTok("enum {A=10,B=15}; x=A+0;", "+").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x=false;", "false").intvalue);
        ASSERT_EQUALS(1, valueOfTok("x=true;", "true").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x(NULL);", "NULL").intvalue);
        ASSERT_EQUALS(static_cast<int>('a'), valueOfTok("x='a';", "'a'").intvalue);
        ASSERT_EQUALS(static_cast<int>('\n'), valueOfTok("x='\\n';", "'\\n'").intvalue);
        TODO_ASSERT_EQUALS(0xFFFFFFFF00000000, 0, valueOfTok("x=0xFFFFFFFF00000000;", "0xFFFFFFFF00000000").intvalue); // #7701
        ASSERT_EQUALS_DOUBLE(16, valueOfTok("x=(double)16;", "(").floatValue, 1e-5);
        ASSERT_EQUALS_DOUBLE(0.0625, valueOfTok("x=1/(double)16;", "/").floatValue, 1e-5);

        const Settings settingsC23 = settingsBuilder().c(Standards::C23).build();
        ASSERT_EQUALS(1, valueOfTok("x=true;", "true", &settingsC23, false).intvalue);
        ASSERT_EQUALS(0, valueOfTok("x=false;", "false", &settingsC23, false).intvalue);

        const Settings settingsC17 = settingsBuilder().c(Standards::C17).build();
        ASSERT(!valueOfTok("x=true;", "true", &settingsC17, false).isKnown());
        ASSERT(!valueOfTok("x=false;", "false", &settingsC17, false).isKnown());

        // scope
        {
            const char code[] = "namespace N { enum E {e0,e1}; }\n"
                                "void foo() { x = N::e1; }";
            ASSERT_EQUALS(1, valueOfTok(code, "::").intvalue);
        }

        ASSERT_EQUALS(63, valueOfTok("x = 3 * uint32_t{21};", "*").intvalue);
    }

    void valueFlowString() {
        const char *code;

        // valueFlowAfterAssign
        code  = "const char * f() {\n"
                "    static const char *x;\n"
                "    if (a) x = \"123\";\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "\"123\"", ValueFlow::Value::ValueType::TOK));

        // valueFlowSubFunction
        code  = "void dostuff(const char *x) {\n"
                "  f(x);\n"
                "}\n"
                "\n"
                "void test() { dostuff(\"abc\"); }";
        ASSERT_EQUALS(true, testValueOfX(code, 2, "\"abc\"", ValueFlow::Value::ValueType::TOK));
    }

    void valueFlowTypeTraits()
    {
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void<void>{};", "{", 1));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void<void>::value;", ":: value", 1));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void_v<void>;", "::", 1));

        // is_void
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void<int>{};", "{", 0));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void<void*>{};", "{", 0));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void<const void>{};", "{", 1));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_void<volatile void>{};", "{", 1));

        // is_lvalue_reference
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_lvalue_reference<int>{};", "{", 0));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_lvalue_reference<int&>{};", "{", 1));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_lvalue_reference<int&&>{};", "{", 0));

        // is_rvalue_reference
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_rvalue_reference<int>{};", "{", 0));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_rvalue_reference<int&>{};", "{", 0));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_rvalue_reference<int&&>{};", "{", 1));

        // is_reference
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_reference<int>{};", "{", 0));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_reference<int&>{};", "{", 1));
        ASSERT_EQUALS(true, testKnownValueOfTok("std::is_reference<int&&>{};", "{", 1));

        {
            const char* code;
            code = "void bar();\n"
                   "void foo() { std::is_void<decltype(bar())>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "int bar();\n"
                   "void foo() { std::is_void<decltype(bar())>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void bar();\n"
                   "void foo() { std::is_void<decltype(bar)>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "class A;\n"
                   "void foo() { std::is_lvalue_reference<A>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "class A;\n"
                   "void foo() { std::is_lvalue_reference<A&>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "class A;\n"
                   "void foo() { std::is_lvalue_reference<A&&>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "class A;\n"
                   "void foo() { std::is_rvalue_reference<A>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "class A;\n"
                   "void foo() { std::is_rvalue_reference<A&>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "class A;\n"
                   "void foo() { std::is_rvalue_reference<A&&>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "class A;\n"
                   "void foo() { std::is_reference<A>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "class A;\n"
                   "void foo() { std::is_reference<A&>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "class A;\n"
                   "void foo() { std::is_reference<A&&>::value; }";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "void foo() {\n"
                   "    int bar;\n"
                   "    std::is_void<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void foo(int bar) {\n"
                   "    std::is_lvalue_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void foo(int& bar) {\n"
                   "    std::is_lvalue_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "void foo(int&& bar) {\n"
                   "    std::is_lvalue_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void foo(int bar) {\n"
                   "    std::is_rvalue_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void foo(int& bar) {\n"
                   "    std::is_rvalue_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void foo(int&& bar) {\n"
                   "    std::is_rvalue_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "void foo(int bar) {\n"
                   "    std::is_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 0));

            code = "void foo(int& bar) {\n"
                   "    std::is_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));

            code = "void foo(int&& bar) {\n"
                   "    std::is_reference<decltype(bar)>::value;\n"
                   "}\n";
            ASSERT_EQUALS(true, testKnownValueOfTok(code, ":: value", 1));
        }
    }

    void valueFlowPointerAlias() {
        const char *code;
        std::list<ValueFlow::Value> values;

        code  = "const char * f() {\n"
                "    static const char *x;\n"
                "    static char ret[10];\n"
                "    if (a) x = &ret[0];\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5, "& ret [ 0 ]", ValueFlow::Value::ValueType::TOK));

        // dead pointer
        code  = "void f() {\n"
                "  int *x;\n"
                "  if (cond) { int i; x = &i; }\n"
                "  *x = 0;\n"  // <- x can point at i
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "& i", ValueFlow::Value::ValueType::TOK));

        code  = "void f() {\n"
                "  struct X *x;\n"
                "  x = &x[1];\n"
                "}";
        values = tokenValues(code, "&");
        values.remove_if(&isNotTokValue);
        ASSERT_EQUALS(true, values.empty());

        values = tokenValues(code, "x [");
        values.remove_if(&isNotTokValue);
        ASSERT_EQUALS(true, values.empty());
    }

    void valueFlowLifetime() {
        std::vector<std::string> lifetimes;

        {
            const char code[] = "void f() {\n"
                                "    int a = 1;\n"
                                "    auto x = [&]() { return a + 1; };\n"
                                "    auto b = x;\n"
                                "}\n";
            ASSERT_EQUALS(true, testLifetimeOfX(code, 4, "a + 1"));
        }

        {
            const char code[] = "void f() {\n"
                                "    int a = 1;\n"
                                "    auto x = [=]() { return a + 1; };\n"
                                "    auto b = x;\n"
                                "}\n";
            ASSERT_EQUALS(false, testLifetimeOfX(code, 4, "a ;"));
        }

        {
            const char code[] = "void f(int v) {\n"
                                "    int a = v;\n"
                                "    int * p = &a;\n"
                                "    auto x = [=]() { return p + 1; };\n"
                                "    auto b = x;\n"
                                "}\n";
            ASSERT_EQUALS(true, testLifetimeOfX(code, 5, "a ;"));
        }

        {
            const char code[] = "void f() {\n"
                                "    std::vector<int> v;\n"
                                "    auto x = v.begin();\n"
                                "    auto it = x;\n"
                                "}\n";
            ASSERT_EQUALS(true, testLifetimeOfX(code, 4, "v . begin"));
        }

        {
            const char code[] = "void f() {\n"
                                "    std::vector<int> v;\n"
                                "    auto x = v.begin() + 1;\n"
                                "    auto it = x;\n"
                                "}\n";
            ASSERT_EQUALS(true, testLifetimeOfX(code, 4, "v . begin"));
        }

        {
            const char code[] = "int* f() {\n"
                                "    std::vector<int> v;\n"
                                "    int * x = v.data();\n"
                                "    return x;\n"
                                "}\n";
            ASSERT_EQUALS(true, testLifetimeOfX(code, 4, "v . data"));
        }

        {
            const char code[] = "int* f() {\n"
                                "    std::vector<int> v;\n"
                                "    int * x = v.data() + 1;\n"
                                "    return x;\n"
                                "}\n";
            ASSERT_EQUALS(true, testLifetimeOfX(code, 4, "v . data"));
        }

        {
            const char code[] = "int f(int* a) {\n"
                                "    int **p = &a;\n"
                                "    int * x = *p;\n"
                                "    return x; \n"
                                "}\n";
            ASSERT_EQUALS(false, testLifetimeOfX(code, 4, "a"));
        }

        {
            const char code[] = "void f() {\n"
                                "    int i = 0;\n"
                                "    void* x = (void*)&i;\n"
                                "}\n";
            lifetimes = lifetimeValues(code, "( void * )");
            ASSERT_EQUALS(true, lifetimes.size() == 1);
            ASSERT_EQUALS(true, lifetimes.front() == "i");
        }

        {
            const char code[] = "struct T {\n" // #10810
                                "    static int g() { return 0; }\n"
                                "};\n"
                                "T t;\n"
                                "struct S { int i; };\n"
                                "S f() {\n"
                                "    S s = { decltype(t)::g() };\n"
                                "    return s;\n"
                                "};\n";
            lifetimes = lifetimeValues(code, "=");
            ASSERT_EQUALS(true, lifetimes.empty());
        }

        {
            const char code[] = "struct T {\n" // #10838
                                "     void f();\n"
                                "     double d[4][4];\n"
                                "};\n"
                                "void T::f() {\n"
                                "    auto g = [this]() -> double(&)[4] {\n"
                                "        double(&q)[4] = d[0];\n"
                                "        return q;\n"
                                "    };\n"
                                "}\n";
            lifetimes = lifetimeValues(code, "return"); // don't crash
            ASSERT_EQUALS(true, lifetimes.empty());
        }

        {
            const char code[] = "void f() {\n" // #13076
                                "    char a[10];\n"
                                "    struct S s = { sizeof(a), 0 };\n"
                                "    s.p = a;\n"
                                "}\n";
            lifetimes = lifetimeValues(code, "= a");
            ASSERT_EQUALS(true, lifetimes.size() == 1);
            ASSERT_EQUALS(true, lifetimes.front() == "a");
        }
    }

    void valueFlowArrayElement() {
        const char *code;

        code  = "void f() {\n"
                "    const int x[] = {43,23,12};\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "{ 43 , 23 , 12 }", ValueFlow::Value::ValueType::TOK));

        code  = "void f() {\n"
                "    const char x[] = \"abcd\";\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "\"abcd\"", ValueFlow::Value::ValueType::TOK));

        code  = "void f() {\n"
                "    char x[32] = \"abcd\";\n"
                "    return x;\n"
                "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, "\"abcd\"", ValueFlow::Value::ValueType::TOK));

        code = "void f() {\n"
               "  int a[10];\n"
               "  int *x = a;\n" // <- a value is a
               "  *x = 0;\n"     // .. => x value is a
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4, "a", ValueFlow::Value::ValueType::TOK));

        code  = "char f() {\n"
                "    const char *x = \"abcd\";\n"
                "    return x[0];\n"
                "}";
        ASSERT_EQUALS(static_cast<int>('a'), valueOfTok(code, "[").intvalue);

        code  = "char f() {\n"
                "    const char *x = \"\";\n"
                "    return x[0];\n"
                "}";
        ASSERT_EQUALS(0, valueOfTok(code, "[").intvalue);

        code = "int g() { return 3; }\n"
               "void f() {\n"
               "    const int x[2] = { g(), g() };\n"
               "    return x[0];\n"
               "}\n";
        ASSERT_EQUALS(3, valueOfTok(code, "[ 0").intvalue);

        code = "int g() { return 3; }\n"
               "void f() {\n"
               "    const int x[2] = { g(), g() };\n"
               "    return x[1];\n"
               "}\n";
        ASSERT_EQUALS(3, valueOfTok(code, "[ 1").intvalue);
    }

    void valueFlowMove() {
        const char *code;

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::forward<X>(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::ForwardedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x).getA());\n"   // Only parts of x might be moved out
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::forward<X>(x).getA());\n" // Only parts of x might be moved out
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::ForwardedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   x.clear();\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f() {\n"
               "   X x;\n"
               "   g(std::move(x));\n"
               "   y=x->y;\n"
               "   z=x->z;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    z = g(std::move(x));\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    y = g(std::move(x),\n"
               "          x.size());\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int i) {\n"
               "    X x;\n"
               "    x = g(std::move(x));\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "A f(int i) {\n"
               "    X x;\n"
               "    if (i)"
               "        return g(std::move(x));\n"
               "    return h(std::move(x));\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "struct X {\n"
               "};\n"
               "struct Data {\n"
               "  template<typename Fun>\n"
               "  void foo(Fun f) {}\n"
               "};\n"
               "Data g(X value) { return Data(); }\n"
               "void f() {\n"
               "   X x;\n"
               "   g(std::move(x)).foo([=](int value) mutable {;});\n"
               "   X y=x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 11U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void f(int x) {\n"
               "   g(std::move(x));\n"
               "   y=x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "void g(std::string);\n"
               "void foo(std::string x) {\n"
               "  g(std::move(x));\n"
               "  int counter = 0;\n"
               "\n"
               "  for (int i = 0; i < 5; i++) {\n"
               "    if (i % 2 == 0) {\n"
               "      x = std::to_string(i);\n"
               "      counter++;\n"
               "    }\n"
               "  }\n"
               "  for (int i = 0; i < counter; i++) {\n"
               "      if(x > 5) {}\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 13U, ValueFlow::Value::MoveKind::MovedVariable));
    }

    void valueFlowCalculations() {
        const char *code;

        // Different operators
        ASSERT_EQUALS(5, valueOfTok("3 +  (a ? b : 2);", "+").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 -  (a ? b : 2);", "-").intvalue);
        ASSERT_EQUALS(6, valueOfTok("3 *  (a ? b : 2);", "*").intvalue);
        ASSERT_EQUALS(6, valueOfTok("13 / (a ? b : 2);", "/").intvalue);
        ASSERT_EQUALS(1, valueOfTok("13 % (a ? b : 2);", "%").intvalue);
        ASSERT_EQUALS(0, valueOfTok("3 == (a ? b : 2);", "==").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 != (a ? b : 2);", "!=").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 >  (a ? b : 2);", ">").intvalue);
        ASSERT_EQUALS(1, valueOfTok("3 >= (a ? b : 2);", ">=").intvalue);
        ASSERT_EQUALS(0, valueOfTok("3 <  (a ? b : 2);", "<").intvalue);
        ASSERT_EQUALS(0, valueOfTok("3 <= (a ? b : 2);", "<=").intvalue);

        ASSERT_EQUALS(1, valueOfTok("(UNKNOWN_TYPE)1;","(").intvalue);
        ASSERT(tokenValues("(UNKNOWN_TYPE)1000;","(").empty()); // don't know if there is truncation, sign extension
        ASSERT_EQUALS(255, valueOfTok("(unsigned char)~0;", "(").intvalue);
        ASSERT_EQUALS(0, valueOfTok("(int)0;", "(").intvalue);
        ASSERT_EQUALS(3, valueOfTok("(int)(1+2);", "(").intvalue);
        ASSERT_EQUALS(0, valueOfTok("(UNKNOWN_TYPE*)0;","(").intvalue);
        ASSERT_EQUALS(100, valueOfTok("(int)100.0;", "(").intvalue);
        ASSERT_EQUALS(10, valueOfTok("x = static_cast<int>(10);", "( 10 )").intvalue);
        ASSERT_EQUALS(0, valueOfTok("x = sizeof (struct {int a;}) * 0;", "*").intvalue);

        // Don't calculate or crash if there is UB or invalid operations
        ASSERT(tokenValues(";-1<<10;","<<").empty());
        ASSERT(tokenValues(";10<<-1;","<<").empty());
        ASSERT(tokenValues(";10<<64;","<<").empty());
        ASSERT(tokenValues(";-1>>10;",">>").empty());
        ASSERT(tokenValues(";10>>-1;",">>").empty());
        ASSERT(tokenValues(";10>>64;",">>").empty());
        ASSERT_EQUALS(tokenValues(";1%-1;","%").size(), 1);
        ASSERT_EQUALS(tokenValues(";1%-10;","%").size(), 1);
        ASSERT_EQUALS(tokenValues(";1.5%-1;","%").size(), 1);
        ASSERT_EQUALS(tokenValues(";1.5%-10;","%").size(), 1);
        ASSERT(tokenValues(";1%-1.5;","%").empty());
        ASSERT(tokenValues(";1%-10.5;","%").empty());
        ASSERT(tokenValues(";1.5%-1.5;","%").empty());
        ASSERT(tokenValues(";1.5%-10.5;","%").empty());
        ASSERT(tokenValues(";1/-1;","/").empty());
        ASSERT(tokenValues(";1/-10;","/").empty());
        ASSERT(tokenValues(";1.5/-1;","/").empty());
        ASSERT(tokenValues(";1.5/-10;","/").empty());
        ASSERT(tokenValues(";1/-1.5;","/").empty());
        ASSERT(tokenValues(";1/-10.5;","/").empty());
        ASSERT(tokenValues(";1.5/-1.5;","/").empty());
        ASSERT(tokenValues(";1.5/-10.5;","/").empty());
        ASSERT(tokenValues(";1/0;","/").empty());
        ASSERT(tokenValues(";1/0;","/").empty());
        ASSERT(tokenValues(";1.5/0;","/").empty());
        ASSERT(tokenValues(";1.5/0;","/").empty());
        ASSERT(tokenValues(";((-1) * 9223372036854775807LL - 1) / (-1);", "/").empty()); // #12109
        ASSERT_EQUALS(tokenValues(";((-1) * 9223372036854775807LL - 1) % (-1);", "%").size(), 1); // #12109

        code = "float f(const uint16_t& value) {\n"
               "    const uint16_t uVal = value; \n"
               "    return static_cast<float>(uVal) / 2;\n"
               "}\n";
        ASSERT_EQUALS(true, tokenValues(code, "/").empty());

        // calculation using 1,2 variables/values
        code  = "void f(int x) {\n"
                "    a = x+456;\n"
                "    if (x==123) {}"
                "}";
        ASSERT_EQUALS(579, valueOfTok(code, "+").intvalue);

        code  = "void f(int x, int y) {\n"
                "    a = x+y;\n"
                "    if (x==123 || y==456) {}"
                "}";
        ASSERT_EQUALS(0, valueOfTok(code, "+").intvalue);

        code  = "void f(int x) {\n"
                "    a = x+x;\n"
                "    if (x==123) {}"
                "}";
        ASSERT_EQUALS(246, valueOfTok(code, "+").intvalue);

        code  = "void f(int x, int y) {\n"
                "    a = x*x;\n"
                "    if (x==2) {}\n"
                "    if (x==4) {}\n"
                "}";
        std::list<ValueFlow::Value> values = tokenValues(code,"*");
        ASSERT_EQUALS(2U, values.size());
        ASSERT_EQUALS(4, values.front().intvalue);
        ASSERT_EQUALS(16, values.back().intvalue);

        code  = "void f(int x) {\n"
                "    if (x == 3) {}\n"
                "    a = x * (1 - x - 1);\n"
                "}";
        ASSERT_EQUALS(-9, valueOfTok(code, "*").intvalue);

        // addition of different variables with known values
        code = "int f(int x) {\n"
               "  int a = 1;\n"
               "  while (x!=3) { x+=a; }\n"
               "  return x/a;\n"
               "}\n";
        ASSERT_EQUALS(3, valueOfTok(code, "/").intvalue);

        // ? :
        code = "x = y ? 2 : 3;\n";
        values = tokenValues(code,"?");
        ASSERT_EQUALS(2U, values.size());
        ASSERT_EQUALS(2, values.front().intvalue);
        ASSERT_EQUALS(3, values.back().intvalue);

        code = "void f(int a) { x = a ? 2 : 3; }\n";
        values = tokenValues(code,"?");
        ASSERT_EQUALS(2U, values.size());
        ASSERT_EQUALS(2, values.front().intvalue);
        ASSERT_EQUALS(3, values.back().intvalue);

        code = "x = (2<5) ? 2 : 3;\n";
        values = tokenValues(code, "?");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(2, values.front().intvalue);

        code = "x = 123 ? : 456;\n";
        values = tokenValues(code, "?");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(123, values.empty() ? 0 : values.front().intvalue);

        code = "x = true ? 2 : 3;\n"; // #14369
        values = tokenValues(code, "?");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(2, values.empty() ? 0 : values.front().intvalue);

        code = "x = false ? 2 : 3;\n"; // #14369
        values = tokenValues(code, "?");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(3, values.empty() ? 0 : values.front().intvalue);

        code  = "int f() {\n"
                "    const int i = 1;\n"
                "    int x = i < 0 ? 0 : 1;\n"
                "    return x;\n"
                "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 1));

        // ~
        code  = "x = ~0U;";
        values = tokenValues(code,"~");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(~0U, values.back().intvalue);

        // !
        code  = "void f(int x) {\n"
                "    a = !x;\n"
                "    if (x==0) {}\n"
                "}";
        values = removeImpossible(tokenValues(code, "!"));
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);

        // unary minus
        code  = "void f(int x) {\n"
                "    a = -x;\n"
                "    if (x==10) {}\n"
                "}";
        values = tokenValues(code,"-");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(-10, values.back().intvalue);

        // Logical and
        code = "void f(bool b) {\n"
               "   bool x = false && b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(bool b) {\n"
               "   bool x = b && false;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(bool b) {\n"
               "   bool x = true && b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));

        code = "void f(bool b) {\n"
               "   bool x = b && true;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));

        // Logical or
        code = "void f(bool b) {\n"
               "   bool x = true || b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f(bool b) {\n"
               "   bool x = b || true;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f(bool b) {\n"
               "   bool x = false || b;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(bool b) {\n"
               "   bool x = b || false;\n"
               "   bool a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int i) {\n"
               "    int * p = &i;\n"
               "    bool x = !p || i;\n"
               "    bool a = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 1));

        code = "bool f(const uint16_t * const p) {\n"
               "    const uint8_t x = (uint8_t)(*p & 0x01E0U) >> 5U;\n"
               "    return x != 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));

        code = "bool f() {\n"
               "    bool a = (4 == 3);\n"
               "    bool b = (3 == 3);\n"
               "    return a || b;\n"
               "}\n";
        values = tokenValues(code, "%oror%");
        ASSERT_EQUALS(1, values.size());
        if (!values.empty()) {
            ASSERT_EQUALS(true, values.front().isIntValue());
            ASSERT_EQUALS(true, values.front().isKnown());
            ASSERT_EQUALS(1, values.front().intvalue);
        }

        // function call => calculation
        code  = "void f(int x, int y) {\n"
                "    a = x + y;\n"
                "}\n"
                "void callf() {\n"
                "    f(1,1);\n"
                "    f(10,10);\n"
                "}";
        values = tokenValues(code, "+");
        ASSERT_EQUALS(true, values.empty());
        if (!values.empty()) {
            /* todo.. */
            ASSERT_EQUALS(2U, values.size());
            ASSERT_EQUALS(2, values.front().intvalue);
            ASSERT_EQUALS(22, values.back().intvalue);
        }

        // #10251 starship operator
        code  = "struct X {};\n"
                "auto operator<=>(const X & a, const X & b) -> decltype(1 <=> 2) {\n"
                "    return std::strong_ordering::less;\n"
                "}\n";
        ASSERT_NO_THROW(tokenValues(code, "<=>"));

        // Comparison of string
        values = removeImpossible(tokenValues("f(\"xyz\" == \"xyz\");", "==")); // implementation defined
        ASSERT_EQUALS(0U, values.size()); // <- no value

        values = removeImpossible(tokenValues("f(\"xyz\" == 0);", "=="));
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);

        values = removeImpossible(tokenValues("f(0 == \"xyz\");", "=="));
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);

        values = removeImpossible(tokenValues("f(\"xyz\" != 0);", "!="));
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.front().intvalue);

        values = removeImpossible(tokenValues("f(0 != \"xyz\");", "!="));
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.front().intvalue);
    }

    void valueFlowSizeof() {
        const char *code;
        std::list<ValueFlow::Value> values;

        // array size
        code  = "void f() {\n"
                "    char a[10];"
                "    x = sizeof(*a);\n"
                "}";
        values = tokenValues(code,"( *");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);

        code  = "void f() {\n"
                "    char a[10];"
                "    x = sizeof(a[0]);\n"
                "}";
        values = tokenValues(code,"( a");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);

        code = "enum testEnum : uint32_t { a };\n"
               "sizeof(testEnum);";
        values = tokenValues(code,"( testEnum");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(4, values.back().intvalue);

#define CHECK3(A, B, C)                          \
    do {                                     \
        code = "void f() {\n"                    \
               "    x = sizeof(" A ");\n"        \
               "}";                              \
        values = tokenValues(code,"( " C " )");  \
        ASSERT_EQUALS(1U, values.size());        \
        ASSERT_EQUALS(B, values.back().intvalue); \
    } while (false)
#define CHECK(A, B) CHECK3(A, B, A)

        // standard types
        CHECK("void *", settings.platform.sizeof_pointer);
        CHECK("char", 1U);
        CHECK("short", settings.platform.sizeof_short);
        CHECK("int", settings.platform.sizeof_int);
        CHECK("long", settings.platform.sizeof_long);
        CHECK3("long long", settings.platform.sizeof_long_long, "long");
        CHECK("wchar_t", settings.platform.sizeof_wchar_t);
        CHECK("float", settings.platform.sizeof_float);
        CHECK("double", settings.platform.sizeof_double);
        CHECK3("long double", settings.platform.sizeof_long_double, "double");

        // string/char literals
        CHECK("\"asdf\"", 5);
        CHECK("L\"asdf\"", 5LL * settings.platform.sizeof_wchar_t);
        CHECK("u8\"asdf\"", 5); // char8_t
        CHECK("u\"asdf\"", 5LL * 2); // char16_t
        CHECK("U\"asdf\"", 5LL * 4); // char32_t
        CHECK("'a'", 1U);
        CHECK("'ab'", settings.platform.sizeof_int);
        CHECK("L'a'", settings.platform.sizeof_wchar_t);
        CHECK("u8'a'", 1U); // char8_t
        CHECK("u'a'", 2U); // char16_t
        CHECK("U'a'", 4U); // char32_t
#undef CHECK
#undef CHECK3

        // array size
        code  = "void f() {\n"
                "    struct S *a[10];"
                "    x = sizeof(a) / sizeof(a[0]);\n"
                "}";
        values = tokenValues(code,"/");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(10, values.back().intvalue);

        code  = "void f() {\n" // #11294
                "    struct S { int i; };\n"
                "    const S a[] = { 1, 2 };\n"
                "    x = sizeof(a) / ( sizeof(a[0]) );\n"
                "}";
        values = tokenValues(code, "/");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(2, values.back().intvalue);

#define CHECK(A, B, C, D)                         \
    do {                                      \
        code = "enum " A " E " B " { E0, E1 };\n" \
               "void f() {\n"                     \
               "    x = sizeof(" C ");\n"         \
               "}";                               \
        values = tokenValues(code,"( " C " )");   \
        ASSERT_EQUALS(1U, values.size());         \
        ASSERT_EQUALS(D, values.back().intvalue); \
    } while (false)

        // enums
        CHECK("", "", "E", settings.platform.sizeof_int);

        // typed enums
        CHECK("", ": char", "E", 1U);
        CHECK("", ": signed char", "E", 1U);
        CHECK("", ": unsigned char", "E", 1U);
        CHECK("", ": short", "E", settings.platform.sizeof_short);
        CHECK("", ": signed short", "E", settings.platform.sizeof_short);
        CHECK("", ": unsigned short", "E", settings.platform.sizeof_short);
        CHECK("", ": int", "E", settings.platform.sizeof_int);
        CHECK("", ": signed int", "E", settings.platform.sizeof_int);
        CHECK("", ": unsigned int", "E", settings.platform.sizeof_int);
        CHECK("", ": long", "E", settings.platform.sizeof_long);
        CHECK("", ": signed long", "E", settings.platform.sizeof_long);
        CHECK("", ": unsigned long", "E", settings.platform.sizeof_long);
        CHECK("", ": long long", "E", settings.platform.sizeof_long_long);
        CHECK("", ": signed long long", "E", settings.platform.sizeof_long_long);
        CHECK("", ": unsigned long long", "E", settings.platform.sizeof_long_long);
        CHECK("", ": wchar_t", "E", settings.platform.sizeof_wchar_t);
        CHECK("", ": size_t", "E", settings.platform.sizeof_size_t);

        // enumerators
        CHECK("", "", "E0", settings.platform.sizeof_int);

        // typed enumerators
        CHECK("", ": char", "E0", 1U);
        CHECK("", ": signed char", "E0", 1U);
        CHECK("", ": unsigned char", "E0", 1U);
        CHECK("", ": short", "E0", settings.platform.sizeof_short);
        CHECK("", ": signed short", "E0", settings.platform.sizeof_short);
        CHECK("", ": unsigned short", "E0", settings.platform.sizeof_short);
        CHECK("", ": int", "E0", settings.platform.sizeof_int);
        CHECK("", ": signed int", "E0", settings.platform.sizeof_int);
        CHECK("", ": unsigned int", "E0", settings.platform.sizeof_int);
        CHECK("", ": long", "E0", settings.platform.sizeof_long);
        CHECK("", ": signed long", "E0", settings.platform.sizeof_long);
        CHECK("", ": unsigned long", "E0", settings.platform.sizeof_long);
        CHECK("", ": long long", "E0", settings.platform.sizeof_long_long);
        CHECK("", ": signed long long", "E0", settings.platform.sizeof_long_long);
        CHECK("", ": unsigned long long", "E0", settings.platform.sizeof_long_long);
        CHECK("", ": wchar_t", "E0", settings.platform.sizeof_wchar_t);
        CHECK("", ": size_t", "E0", settings.platform.sizeof_size_t);

        // class typed enumerators
        CHECK("class", ": char", "E :: E0", 1U);
        CHECK("class", ": signed char", "E :: E0", 1U);
        CHECK("class", ": unsigned char", "E :: E0", 1U);
        CHECK("class", ": short", "E :: E0", settings.platform.sizeof_short);
        CHECK("class", ": signed short", "E :: E0", settings.platform.sizeof_short);
        CHECK("class", ": unsigned short", "E :: E0", settings.platform.sizeof_short);
        CHECK("class", ": int", "E :: E0", settings.platform.sizeof_int);
        CHECK("class", ": signed int", "E :: E0", settings.platform.sizeof_int);
        CHECK("class", ": unsigned int", "E :: E0", settings.platform.sizeof_int);
        CHECK("class", ": long", "E :: E0", settings.platform.sizeof_long);
        CHECK("class", ": signed long", "E :: E0", settings.platform.sizeof_long);
        CHECK("class", ": unsigned long", "E :: E0", settings.platform.sizeof_long);
        CHECK("class", ": long long", "E :: E0", settings.platform.sizeof_long_long);
        CHECK("class", ": signed long long", "E :: E0", settings.platform.sizeof_long_long);
        CHECK("class", ": unsigned long long", "E :: E0", settings.platform.sizeof_long_long);
        CHECK("class", ": wchar_t", "E :: E0", settings.platform.sizeof_wchar_t);
        CHECK("class", ": size_t", "E :: E0", settings.platform.sizeof_size_t);
#undef CHECK

#define CHECK(A, B)                                   \
    do {                                          \
        code = "enum E " A " { E0, E1 };\n"           \
               "void f() {\n"                         \
               "    E arrE[] = { E0, E1 };\n"         \
               "    x = sizeof(arrE);\n"              \
               "}";                                   \
        values = tokenValues(code,"( arrE )");        \
        ASSERT_EQUALS(1U, values.size());             \
        ASSERT_EQUALS(B * 2ULL, values.back().intvalue); \
    } while (false)

        // enum array
        CHECK("", settings.platform.sizeof_int);

        // typed enum array
        CHECK(": char", 1U);
        CHECK(": signed char", 1U);
        CHECK(": unsigned char", 1U);
        CHECK(": short", settings.platform.sizeof_short);
        CHECK(": signed short", settings.platform.sizeof_short);
        CHECK(": unsigned short", settings.platform.sizeof_short);
        CHECK(": int", settings.platform.sizeof_int);
        CHECK(": signed int", settings.platform.sizeof_int);
        CHECK(": unsigned int", settings.platform.sizeof_int);
        CHECK(": long", settings.platform.sizeof_long);
        CHECK(": signed long", settings.platform.sizeof_long);
        CHECK(": unsigned long", settings.platform.sizeof_long);
        CHECK(": long long", settings.platform.sizeof_long_long);
        CHECK(": signed long long", settings.platform.sizeof_long_long);
        CHECK(": unsigned long long", settings.platform.sizeof_long_long);
        CHECK(": wchar_t", settings.platform.sizeof_wchar_t);
        CHECK(": size_t", settings.platform.sizeof_size_t);
#undef CHECK

#define CHECK(A, B)                                   \
    do {                                          \
        code = "enum class E " A " { E0, E1 };\n"     \
               "void f() {\n"                         \
               "    E arrE[] = { E::E0, E::E1 };\n"   \
               "    x = sizeof(arrE);\n"              \
               "}";                                   \
        values = tokenValues(code,"( arrE )");        \
        ASSERT_EQUALS(1U, values.size());             \
        ASSERT_EQUALS(B * 2ULL, values.back().intvalue); \
    } while (false)

        // enum array
        CHECK("", settings.platform.sizeof_int);

        // typed enum array
        CHECK(": char", 1U);
        CHECK(": signed char", 1U);
        CHECK(": unsigned char", 1U);
        CHECK(": short", settings.platform.sizeof_short);
        CHECK(": signed short", settings.platform.sizeof_short);
        CHECK(": unsigned short", settings.platform.sizeof_short);
        CHECK(": int", settings.platform.sizeof_int);
        CHECK(": signed int", settings.platform.sizeof_int);
        CHECK(": unsigned int", settings.platform.sizeof_int);
        CHECK(": long", settings.platform.sizeof_long);
        CHECK(": signed long", settings.platform.sizeof_long);
        CHECK(": unsigned long", settings.platform.sizeof_long);
        CHECK(": long long", settings.platform.sizeof_long_long);
        CHECK(": signed long long", settings.platform.sizeof_long_long);
        CHECK(": unsigned long long", settings.platform.sizeof_long_long);
        CHECK(": wchar_t", settings.platform.sizeof_wchar_t);
        CHECK(": size_t", settings.platform.sizeof_size_t);
#undef CHECK

        code = "uint16_t arr[10];\n"
               "x = sizeof(arr);";
        values = tokenValues(code,"( arr )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(10 * sizeof(std::uint16_t), values.back().intvalue);

        code = "int sz = sizeof(int32_t[10][20]);";
        values = tokenValues(code, "=");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(sizeof(std::int32_t) * 10 * 20, values.back().intvalue);

        code = "struct X { float a; float b; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(8, values.back().intvalue);

        code = "struct X { char a; char b; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(2, values.back().intvalue);

        code = "struct X { char a; float b; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(8, values.back().intvalue);

        code = "struct X { char a; char b; float c; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(8, values.back().intvalue);

        code = "struct X { float a; char b; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(8, values.back().intvalue);

        code = "struct X { float a; char b; char c; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(8, values.back().intvalue);

        code = "struct A { float a; char b; };\n"
               "struct X { A a; char b; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(12, values.back().intvalue);

        code = "struct X { A a; int b; A c; };\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(-1, values.back().intvalue);

        code = "struct X {};\n"
               "struct SubX : X {};\n"
               "void f() {\n"
               "    x = sizeof(X);\n"
               "    subx = sizeof(SubX);\n"
               "}";
        values = tokenValues(code, "( X )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);
        values = tokenValues(code, "( SubX )");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1, values.back().intvalue);

        code = "struct T;\n"
               "struct S { T& r; };\n"
               "struct T { S s{ *this }; };\n"
               "void f() {\n"
               "    sizeof(T) == sizeof(void*);\n"
               "}\n";
        values = tokenValues(code, "==");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(1LL, values.back().intvalue);

        code = "struct S { int32_t a[2][10] };\n" // #12479
               "void f() {\n"
               "    x = sizeof(S);\n"
               "}\n";
        values = tokenValues(code, "=");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(4LL * 2 * 10, values.back().intvalue);

        code = "struct S {\n"
               "    int a[10];\n"
               "    int b[20];\n"
               "    int c[30];\n"
               "};\n"
               "void f() {\n"
               "    x = sizeof(S);\n"
               "}\n";
        values = tokenValues(code, "=");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(4LL * (10 + 20 + 30), values.back().intvalue);

        code = "struct S {\n"
               "    char c;\n"
               "    int a[4];\n"
               "};\n"
               "void f() {\n"
               "    x = sizeof(S);\n"
               "}\n";
        values = tokenValues(code, "=");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(4LL * 5, values.back().intvalue);

        // #13734
        code = "void f() {\n"
               "    int a[N + 1];"
               "    x = sizeof(a) / sizeof(a[0]);\n"
               "}";
        values = tokenValues(code,"/");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(-1, values.back().intvalue);
        ASSERT_EQUALS_ENUM(ValueFlow::Value::ValueKind::Impossible, values.back().valueKind);

        code = "struct E;\n"
               "struct B {\n"
               "    E* e;\n"
               "    B* b;\n"
               "};\n"
               "struct D : B {};\n"
               "struct E : B {\n"
               "	B* be;\n"
               "};\n"
               "int f() {\n"
               "    return sizeof(D);\n"
               "}";
        values = tokenValues(code, "( D )");
        ASSERT_EQUALS(1U, values.size());
        TODO_ASSERT_EQUALS(2 * settings.platform.sizeof_pointer, 1, values.back().intvalue);

        code = "int f() {\n" // #11335
               "    int* a[2];"
               "    return sizeof(a);\n"
               "}";
        values = tokenValues(code, "( a");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(2 * settings.platform.sizeof_pointer, values.back().intvalue);
        ASSERT_EQUALS_ENUM(ValueFlow::Value::ValueKind::Known, values.back().valueKind);
    }

    void valueFlowComma()
    {
        const char* code;
        std::list<ValueFlow::Value> values;

        code = "void f(int i) {\n"
               "    int x = (i, 4);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 4));

        code = "void f(int i) {\n"
               "    int x = (4, i);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 4));

        code = "void f() {\n"
               "    int x = g(3, 4);\n"
               "    return x;\n"
               "}\n";
        values = tokenValues(code, ",");
        ASSERT_EQUALS(0U, values.size());
    }

    void valueFlowErrorPath() {
        const char *code;

        code = "void f() {\n"
               "  int x = 53;\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS("2,Assignment 'x=53', assigned value is 53\n",
                      getErrorPathForX(code, 3U));

        code = "void f(int y) {\n"
               "  int x = y;\n"
               "  a = x;\n"
               "  y += 12;\n"
               "  if (y == 32) {}"
               "}\n";
        ASSERT_EQUALS("2,x is assigned 'y' here.\n"
                      "5,Assuming that condition 'y==32' is not redundant\n"
                      "4,Compound assignment '+=', assigned value is 20\n"
                      "2,x is assigned 'y' here.\n",
                      getErrorPathForX(code, 3U));

        code = "void f1(int x) {\n"
               "  a = x;\n"
               "}\n"
               "void f2() {\n"
               "  int x = 3;\n"
               "  f1(x+1);\n"
               "}\n";
        ASSERT_EQUALS("5,Assignment 'x=3', assigned value is 3\n"
                      "6,Calling function 'f1', 1st argument 'x+1' value is 4\n",
                      getErrorPathForX(code, 2U));

        code = "void f(int a) {\n"
               "  int x;\n"
               "  for (x = a; x < 50; x++) {}\n"
               "  b = x;\n"
               "}\n";
        ASSERT_EQUALS("3,Assuming that condition 'x<50' is not redundant\n"
                      "3,Assuming that condition 'x<50' is not redundant\n",
                      getErrorPathForX(code, 4U));
    }

    void valueFlowBeforeCondition() {
        const char *code;

        code = "void f(int x) {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x >= 1) {}\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 2U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x > 0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(unsigned int x) {\n"
               "    int a = x;\n"
               "    if (x > 1) {}\n" // not zero => don't consider > condition
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 1));

        code = "void f(int x) {\n" // not unsigned => don't consider > condition
               "    int a = x;\n"
               "    if (x > 0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));

        code = "void f(int *x) {\n"
               "    *x = 100;\n"
               "    if (x) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "extern const int x;\n"
               "void f() {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        // after loop
        code = "void f(struct X *x) {\n"
               "  do {\n"
               "    if (!x)\n"
               "      break;\n"
               "  } while (x->a);\n"
               "  if (x) {}\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 1));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 6U, 0));
    }

    void valueFlowBeforeConditionAssignIncDec() {  // assignment / increment
        const char *code;

        code = "void f(int x) {\n"
               "   x = 2 + x;\n"
               "   if (x == 65);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 65));

        code = "void f(int x) {\n"
               "   x = y = 2 + x;\n"
               "   if (x == 65);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 65));

        code = "void f(int x) {\n"
               "   a[x++] = 0;\n"
               "   if (x == 5);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 5));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x++;\n"
               "   if (x == 4);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 3));

        // compound assignment += , -= , ...
        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x += 2;\n"
               "   if (x == 4);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 2));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x -= 2;\n"
               "   if (x == 4);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 6));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 6));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x *= 2;\n"
               "   if (x == 42);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 21));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 21));

        code = "void f(int x) {\n"
               "   a = x;\n"
               "   x /= 5;\n"
               "   if (x == 42);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 210));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 210));

        // bailout: assignment
        bailout("void f(int x) {\n"
                "    x = y;\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS(
            "[test.cpp:2:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable y [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void valueFlowBeforeConditionAndAndOrOrGuard() { // guarding by &&
        const char *code;

        code = "void f(int x) {\n"
               "    if (!x || \n"  // <- x can be 0
               "        a/x) {}\n" // <- x can't be 0
               "    if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int *x) {\n"
               "  ((x=ret())&&\n"
               "   (*x==0));\n"  // <- x is not 0
               "  if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int *x) {\n"
               "  int a = (x && *x == '1');\n"
               "  int b = a ? atoi(x) : 0;\n"  // <- x is not 0
               "  if (x==0) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
    }

    void valueFlowBeforeConditionFunctionCall() { // function calls
        const char *code;

        code = "void f(int x) {\n"
               "  a = x;\n"
               "  setx(x);\n"
               "  if (x == 1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX((std::string("void setx(int x);")+code).c_str(), 2U, 1));
        ASSERT_EQUALS(false, testValueOfX((std::string("void setx(int &x);")+code).c_str(), 2U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 1));

        code = "void f(char* x) {\n"
               "  strcpy(x,\"abc\");\n"
               "  if (x) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void addNewFunction(Scope**scope, const Token**tok);\n"
               "void f(Scope *x) {\n"
               "  x->functionList.back();\n"
               "  addNewFunction(&x,&tok);\n" // address-of, x can be changed by subfunction
               "  if (x) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void g(int&);"
               "void f(int x) {\n"
               "   g(x);\n"
               "   if (x == 5);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 5));
    }

    void valueFlowBeforeConditionLoop() { // while, for, do-while
        const char *code;

        code = "void f(int x) {\n" // loop condition, x is not assigned inside loop => use condition
               "  a = x;\n"  // x can be 37
               "  while (x == 37) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 37));

        code = "void f(int x) {\n" // loop condition, x is assigned inside loop => don't use condition
               "  a = x;\n"  // don't assume that x can be 37
               "  while (x != 37) { x++; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 37));

        code = "void f(int x) {\n"
               "  a = x;\n"
               "  for (; x!=1; x++) { }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 1));

        code = "void f(menu *x) {\n"
               "  a = x->parent;\n"
               "  for (i=0;(i<10) && (x!=0); i++) { x = x->next; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));

        code = "void f(int x) {\n"  // condition inside loop, x is NOT assigned inside loop => use condition
               "    a = x;\n"
               "    do {\n"
               "        if (x==76) {}\n"
               "    } while (1);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 76));

        code = "void f(int x) {\n"  // conditions inside loop, x is assigned inside do-while => don't use condition
               "    a = x;\n"
               "    do {\n"
               "        if (x!=76) { x=do_something(); }\n"
               "    } while (1);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 76));

        code = "void f(X x) {\n"  // conditions inside loop, x is assigned inside do-while => don't use condition
               "    a = x;\n"
               "    for (i=1;i<=count;i++) {\n"
               "        BUGON(x==0)\n"
               "        x = x.next;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));

        code = "struct S {\n" // #12848
               "    S* next;\n"
               "    int a;\n"
               "};\n"
               "void f(S* x, int i) {\n"
               "    while (x) {\n"
               "        if (x->a == 0) {\n"
               "            x = x->next;\n"
               "            continue;\n"
               "        }\n"
               "        if (i == 0)\n"
               "            break;\n"
               "        x->a = i--;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 13U, 0));
    }

    void valueFlowBeforeConditionTernaryOp() { // bailout: ?:
        const char *code;

        bailout("void f(int x) {\n"
                "    y = ((x<0) ? x : ((x==2)?3:4));\n"
                "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable y [valueFlowBailoutIncompleteVar]\n",
            errout_str());

        bailout("int f(int x) {\n"
                "  int r = x ? 1 / x : 0;\n"
                "  if (x == 0) {}\n"
                "}");

        code = "void f(int x) {\n"
               "    int a =v x;\n"
               "    a = b ? x/2 : 20/x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));

        code = "void f(const s *x) {\n"
               "  x->a = 0;\n"
               "  if (x ? x->a : 0) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(int x, int y) {\n"
               "    a = x;\n"
               "    if (y){}\n"
               "    if (x==123){}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 123));
    }

    void valueFlowBeforeConditionSizeof() { // skip sizeof
        const char *code;

        code = "void f(int *x) {\n"
               "    sizeof(x[0]);\n"
               "    if (x==63){}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 63));

        code = "void f(int *x) {\n"
               "    char a[sizeof x.y];\n"
               "    if (x==0){}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));
    }

    void valueFlowBeforeConditionIfElse() { // bailout: if/else/etc
        const char *code;

        code = "void f(X * x) {\n"
               "  a = x;\n"
               "  if ((x != NULL) &&\n"
               "      (a(x->name, html)) &&\n"
               "      (a(x->name, body))) {}\n"
               "  if (x != NULL) { }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));

        bailout("void f(int x) {\n"
                "    if (x != 123) { b = x; }\n"
                "    if (x == 123) {}\n"
                "}");
        ASSERT_EQUALS(
            "[test.cpp:2:21]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable b [valueFlowBailoutIncompleteVar]\n",
            errout_str());

        code = "void f(int x, bool abc) {\n"
               "  a = x;\n"
               "  if (abc) { x = 1; }\n" // <- condition must be false if x is 7 in next line
               "  if (x == 7) { }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 7));

        code = "void f(int x, bool abc) {\n"
               "  a = x;\n"
               "  if (abc) { x = 7; }\n" // <- condition is probably true
               "  if (x == 7) { }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 7));
    }

    void valueFlowBeforeConditionGlobalVariables() {
        const char *code;

        // handle global variables
        code = "int x;\n"
               "void f() {\n"
               "    int a = x;\n"
               "    if (x == 123) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3,123));

        // bailout when there is function call
        code = "class Fred { int x; void clear(); void f(); };\n"
               "void Fred::f() {\n"
               "    int a = x;\n"
               "    clear();\n"  // <- x might be assigned
               "    if (x == 234) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code,3,234));
    }

    void valueFlowBeforeConditionSwitch() {
        // bailout: switch
        // TODO : handle switch/goto more intelligently
        bailout("void f(int x, int y) {\n"
                "    switch (y) {\n"
                "    case 1: a=x; break;\n"
                "    case 2: if (x==5) {} break;\n"
                "    };\n"
                "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            errout_str());

        bailout("void f(int x, int y) {\n"
                "    switch (y) {\n"
                "    case 1: a=x; return 1;\n"
                "    case 2: if (x==5) {} break;\n"
                "    };\n"
                "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void valueFlowBeforeConditionMacro() {
        // bailout: condition is a expanded macro
        bailout("#define M  if (x==123) {}\n"
                "void f(int x) {\n"
                "    a = x;\n"
                "    M;\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a\n"
            "[test.cpp:4]: (debug) valueflow.cpp:1260:(valueFlow) bailout: variable 'x', condition is defined in macro\n"
            "[test.cpp:4]: (debug) valueflow.cpp:1260:(valueFlow) bailout: variable 'x', condition is defined in macro\n", // duplicate
            errout_str());

        bailout("#define FREE(obj) ((obj) ? (free((char *) (obj)), (obj) = 0) : 0)\n" // #8349
                "void f(int *x) {\n"
                "    a = x;\n"
                "    FREE(x);\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a\n"
            "[test.cpp:4]: (debug) valueflow.cpp:1260:(valueFlow) bailout: variable 'x', condition is defined in macro\n"
            "[test.cpp:4]: (debug) valueflow.cpp:1260:(valueFlow) bailout: variable 'x', condition is defined in macro\n", // duplicate
            errout_str());
    }

    void valueFlowBeforeConditionGoto() {
        // bailout: goto label (TODO: handle gotos more intelligently)
        bailout("void f(int x) {\n"
                "    if (x == 123) { goto out; }\n"
                "    a=x;\n"   // <- x is not 123
                "out:"
                "    if (x==123){}\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a\n"
            "[test.cpp:2]: (debug) valueflow.cpp::(valueFlow) bailout: valueFlowAfterCondition: bailing in conditional block\n"
            "[test.cpp:2]: (debug) valueflow.cpp::(valueFlow) bailout: valueFlowAfterCondition: bailing in conditional block\n", // duplicate
            errout_str());

        // #5721 - FP
        bailout("static void f(int rc) {\n"
                "    ABC* abc = getabc();\n"
                "    if (!abc) { goto out };\n"
                "\n"
                "    abc->majortype = 0;\n"
                "    if (FAILED(rc)) {}\n"
                "\n"
                "out:\n"
                "    if (abc) {}\n"
                "}");
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:3]: (debug) valueflow.cpp:6730:(valueFlow) bailout: valueFlowAfterCondition: bailing in conditional block\n"
            "[test.cpp:3]: (debug) valueflow.cpp:6730:(valueFlow) bailout: valueFlowAfterCondition: bailing in conditional block\n", // duplicate
            errout_str());
    }

    void valueFlowBeforeConditionForward() {
        const char* code;

        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    if (a == 123) {}\n"
               "    int b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    if (a != 123) {}\n"
               "    int b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));
    }

    void valueFlowBeforeConditionConstructor()
    {
        const char* code;

        code = "struct Fred {\n"
               "    Fred(int *x)\n"
               "      : i(*x) {\n"    // <- dereference x
               "        if (!x) {}\n" // <- check x
               "    }\n"
               "    int i;\n"
               "};\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "struct Fred {\n"
               "    Fred(int *x)\n"
               "      : i(*x), j(0) {\n" // <- dereference x
               "        if (!x) {}\n"    // <- check x
               "    }\n"
               "    int i;\n"
               "    int j;\n"
               "};\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
    }

    void valueFlowAfterAssign() {
        const char *code;

        code = "void f() {\n"
               "    int x = 123;\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f() {\n"
               "    bool x = 32;\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    a = sizeof(x);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f() {\n"
               "    const int x(321);\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 321));

        code = "void f() {\n"
               "    int x = 9;\n"
               "    --x;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 8));
        ASSERT_EQUALS("2,Assignment 'x=9', assigned value is 9\n"
                      "3,x is decremented', new value is 8\n",
                      getErrorPathForX(code, 4U));

        code = "void x() {\n"
               "    int x = value ? 6 : 0;\n"
               "    x =\n"
               "        1 + x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 7));

        code = "void f() {\n"
               "    int x = 0;\n"
               "    y = x += z;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    static int x = 2;\n"
               "    x++;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));

        code = "void f() {\n"
               "    static int x = 2;\n"
               "    a >> x;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));

        code = "void f() {\n"
               "    static int x = 0;\n"
               "    if (x==0) x = getX();\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // truncation
        code = "int f() {\n"
               "  int x = 1.5;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "int f() {\n"
               "  unsigned char x = 0x123;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0x23));

        code = "int f() {\n"
               "  signed char x = 0xfe;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, -2));

        // function
        code = "void f() {\n"
               "    char *x = 0;\n"
               "    int success = getx((char**)&x);\n"
               "    if (success) x[0] = 0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    char *x = 0;\n"
               "    getx(reinterpret_cast<void **>(&x));\n"
               "    *x = 0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // lambda
        code = "void f() {\n"
               "    int x = 0;\n"
               "    Q q = [&]() {\n"
               "        if (x > 0) {}\n"
               "        x++;\n"
               "    };\n"
               "    dosomething(q);\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x = 0;\n"
               "    dostuff([&]() {\n"
               "        if (x > 0) {}\n"
               "        x++;\n"
               "    });\n"
               "    dosomething(q);\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "int f() {\n"
               "    int x = 1;\n"
               "    dostuff([&]() {\n"
               "        x = y;\n"
               "    });\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 1));

        // ?:
        code = "void f() {\n"
               "    int x = 8;\n"
               "    a = ((x > 10) ?\n"
               "        x : 0);\n" // <- x is not 8
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 8));

        code = "void f() {\n" // #6973
               "    char *x = \"\";\n"
               "    a = ((x[0] == 'U') ?\n"
               "        x[1] : 0);\n" // <- x is not ""
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, "\"\"", ValueFlow::Value::ValueType::TOK));

        code = "void f() {\n" // #6973
               "    char *x = getenv (\"LC_ALL\");\n"
               "    if (x == NULL)\n"
               "        x = \"\";\n"
               "\n"
               "    if ( (x[0] == 'U') &&\n"  // x can be ""
               "         (x[1] ?\n"           // x can't be ""
               "          x[3] :\n"           // x can't be ""
               "          x[2] ))\n"          // x can't be ""
               "    {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, "\"\"", ValueFlow::Value::ValueType::TOK));
        ASSERT_EQUALS(false, testValueOfX(code, 7U, "\"\"", ValueFlow::Value::ValueType::TOK));
        ASSERT_EQUALS(false, testValueOfX(code, 8U, "\"\"", ValueFlow::Value::ValueType::TOK));
        ASSERT_EQUALS(false, testValueOfX(code, 9U, "\"\"", ValueFlow::Value::ValueType::TOK));

        code = "void f() {\n" // #7599
               "  t *x = 0;\n"
               "  y = (a ? 1 : x\n" // <- x is 0
               "       && x->y ? 1 : 2);" // <- x is not 0
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #7599
               "  t *x = 0;\n"
               "  y = (a ? 1 : !x\n" // <- x is 0
               "       || x->y ? 1 : 2);" // <- x is not 0
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // if/else
        code = "void f() {\n"
               "    int x = 123;\n"
               "    if (condition) return;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f() {\n"
               "    int x = 1;\n"
               "    if (condition) x = 2;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 2));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    if (condition1) x = 456;\n"
               "    if (condition2) x = 789;\n"
               "    a = 2 + x;\n" // <- either assignment "x=123" is redundant or x can be 123 here.
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 5U, 123));

        code = "void f(int a) {\n"
               "    int x = 123;\n"
               "    if (a > 1)\n"
               "        ++x;\n"
               "    else\n"
               "        ++x;\n"
               "    return 2 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f() {\n"
               "    int x = 1;\n"
               "    if (condition1) x = 2;\n"
               "    else return;\n"
               "    a = 2 + x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 1));

        code = "void f(){\n"
               "    int x = 0;\n"
               "    if (a>=0) { x = getx(); }\n"
               "    if (x==0) { return; }\n"
               "    return 123 / x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));

        code = "void f() {\n"
               "  X *x = getx();\n"
               "  if(0) { x = 0; }\n"
               "  else { x->y = 1; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #6239
               "  int x = 4;\n"
               "  if(1) { x = 0; }\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 4));

        code = "void f() {\n"
               "    int x = 32;\n"
               "    if (x>=32) return;\n"
               "    a[x]=0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 32));

        code = "void f() {\n"
               "    int x = 32;\n"
               "    if (x>=32) {\n"
               "        a[x] = 0;\n"  // <- should have possible value 32
               "        return;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 32));

        code = "void f() {\n"
               "    int x = 33;\n"
               "    if (x==33) goto fail;\n"
               "    a[x]=0;\n"
               "fail:\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 33));

        code = "void f() {\n"
               "    int x = 32;\n"
               "    if (a==1) { z=x+12; }\n"
               "    if (a==2) { z=x+32; }\n"
               "    z = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 32));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 32));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 32));

        code = "void f() {\n" // #5656 - FP
               "    int x = 0;\n"
               "    if (!x) {\n"
               "        x = getx();\n"
               "    }\n"
               "    y = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 0));

        code = "void f(int y) {\n" // alias
               "  int x = y;\n"
               "  if (y == 54) {}\n"
               "  else { a = x; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 54));

        code = "void f () {\n"
               "    ST * x =  g_pST;\n"
               "    if (x->y == 0) {\n"
               "        x = NULL;\n"
               "        return 1;\n"
               "    }\n"
               "    a = x->y;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        code = "void f () {\n"
               "    ST * x =  g_pST;\n"
               "    if (x->y == 0) {\n"
               "        x = NULL;\n"
               "        goto label;\n"
               "    }\n"
               "    a = x->y;\n"
               "label:\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        code = "void f() {\n" // #5752 - FP
               "    int *x = 0;\n"
               "    if (x && *x == 123) {\n"
               "        getx(*x);\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x = 0;\n"
               "    if (!x) {}\n"
               "    else { y = x; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #6118 - FP
               "    int x = 0;\n"
               "    x = x & 0x1;\n"
               "    if (x == 0) { x = 2; }\n"
               "    y = 42 / x;\n" // <- x is 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "void f() {\n" // #6118 - FN
               "    int x = 0;\n"
               "    x = x & 0x1;\n"
               "    if (x == 0) { x += 2; }\n"
               "    y = 42 / x;\n" // <- x is 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "void f(int mode) {\n"
               "    struct ABC *x;\n"
               "\n"
               "    if (mode) { x = &y; }\n"
               "    else { x = NULL; }\n"
               "\n"
               "    if (!x) exit(1);\n"
               "\n"
               "    a = x->a;\n" // <- x can't be 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 0));

        code = "void f(int i) {\n"
               "    bool x = false;\n"
               "    if (i == 0) { x = true; }\n"
               "    else if (x && i == 1) {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void f(int i) {\n"
               "    bool x = false;\n"
               "    while(i > 0) {\n"
               "        i++;\n"
               "        if (i == 0) { x = true; }\n"
               "        else if (x && i == 1) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 0));

        // multivariables
        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    if (a!=132) { b = x; }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 132));

        code = "void f(int a) {\n"
               "    int x = a;\n"
               "    b = x;\n" // <- line 3
               "    if (a!=132) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 132));

        code = "void f() {\n"
               "    int a;\n"
               "    if (n) { a = n; }\n"
               "    else { a = 0; }\n"
               "    int x = a;\n"
               "    if (a > 0) { a = b / x; }\n" // <- line 6
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 0)); // x is not 0 at line 6

        code = "void f(int x1) {\n" // #6086
               "  int x = x1;\n"
               "  if (x1 >= 3) {\n"
               "    return;\n"
               "  }\n"
               "  a = x;\n"  // <- x is not 3
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 3));

        code = "int f(int *x) {\n" // #5980
               "  if (!x) {\n"
               "    switch (i) {\n"
               "      default:\n"
               "        throw std::runtime_error(msg);\n"
               "    };\n"
               "  }\n"
               "  return *x;\n"  // <- x is not 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 0));

        code = "void f(int a) {\n" // #6826
               "    int x = a ? a : 87;\n"
               "    if (a && x) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 87));

        code = "void f() {\n"
               "  int first=-1, x=0;\n"
               "  do {\n"
               "    if (first >= 0) { a = x; }\n" // <- x is not 0
               "    first++; x=3;\n"
               "  } while (1);\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 3));

        // pointer/reference to x
        code = "int f(void) {\n"
               "  int x = 2;\n"
               "  int *px = &x;\n"
               "  for (int i = 0; i < 1; i++) {\n"
               "    *px = 1;\n"
               "  }\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 2));

        code = "int f(void) {\n"
               "  int x = 5;\n"
               "  int &rx = x;\n"
               "  for (int i = 0; i < 1; i++) {\n"
               "    rx = 1;\n"
               "  }\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 5));

        // break
        code = "void f() {\n"
               "  for (;;) {\n"
               "    int x = 1;\n"
               "    if (!abc()) {\n"
               "      x = 2;\n"
               "      break;\n"
               "    }\n"
               "    a = x;\n" // <- line 8
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 2)); // x is not 2 at line 8

        code = "void f() {\n"
               "  int x;\n"
               "  switch (ab) {\n"
               "    case A: x = 12; break;\n"
               "    case B: x = 34; break;\n"
               "  }\n"
               "  v = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 12));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 34));

        code = "void f() {\n" // #5981
               "  int x;\n"
               "  switch (ab) {\n"
               "    case A: x = 12; break;\n"
               "    case B: x = 34; break;\n"
               "  }\n"
               "  switch (ab) {\n"
               "    case A: v = x; break;\n" // <- x is not 34
               "  }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 34));

        // while/for
        code = "void f() {\n" // #6138
               "  ENTRY *x = 0;\n"
               "  while (x = get()) {\n"
               "    set(x->value);\n"  // <- x is not 0
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can be 0
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0)); // x can be 0 at line 9

        code = "void f(const int *buf) {\n"
               "  int x = 111;\n"
               "  bool found = false;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      found = true;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  if (found)\n"
               "    a = x;\n" // <- x can't be 111
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 12U, 111)); // x can not be 111 at line 9

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      ;\n" // <- no break
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0));       // x can be 0 at line 9
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 0)); // x can't be known at line 9

        code = "void f(const int *buf) {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      ;\n" // <- no break
               "    } else {\n"
               "      x = 1;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 0)); // x can't be 0 at line 11

        code = "void f(const int *buf) {\n"
               "  int i = 0;\n"
               "  int x = 0;\n"
               "  while (++i < 10) {\n"
               "    if (buf[i] == 123) {\n"
               "      x = i;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can be 0
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 10U, 0)); // x can be 0 at line 9

        code = "bool maybe();\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  bool found = false;\n"
               "  while(!found) {\n"
               "    if (maybe()) {\n"
               "      x = i;\n"
               "      found = true;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 0));

        code = "bool maybe();\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  bool found = false;\n"
               "  while(!found) {\n"
               "    if (maybe()) {\n"
               "      x = i;\n"
               "      found = true;\n"
               "    } else {\n"
               "      found = false;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 13U, 0));

        code = "bool maybe();\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  bool found = false;\n"
               "  while(!found) {\n"
               "    if (maybe()) {\n"
               "      x = i;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 0));

        code = "bool maybe();\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  bool found = false;\n"
               "  while(!found) {\n"
               "    if (maybe()) {\n"
               "      x = i;\n"
               "      found = true;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 12U, 0));

        code = "void f(const int a[]) {\n" // #6616
               "  const int *x = 0;\n"
               "  for (int i = 0; i < 10; i = *x) {\n" // <- x is not 0
               "    x = a[i];\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // alias
        code = "void f() {\n" // #7778
               "  int x = 0;\n"
               "  int *p = &x;\n"
               "  x = 3;\n"
               "  *p = 2;\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 3));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 6U, 2));

        code = "struct Fred {\n"
               "    static void Create(std::unique_ptr<Wilma> wilma);\n"
               "    Fred(std::unique_ptr<Wilma> wilma);\n"
               "    std::unique_ptr<Wilma> mWilma;\n"
               "};\n"
               "void Fred::Create(std::unique_ptr<Wilma> wilma) {\n"
               "    auto fred = std::make_shared<Fred>(std::move(wilma));\n"
               "    fred->mWilma.reset();\n"
               "}\n"
               "Fred::Fred(std::unique_ptr<Wilma> wilma)\n"
               "    : mWilma(std::move(wilma)) {}\n";
        ASSERT_EQUALS(0, tokenValues(code, "mWilma (").size());

        code = "void g(unknown*);\n"
               "int f() {\n"
               "    int a = 1;\n"
               "    unknown c[] = {{&a}};\n"
               "    g(c);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 1));
        ASSERT_EQUALS(true, testValueOfXInconclusive(code, 7U, 1));

        code = "void g(unknown&);\n"
               "int f() {\n"
               "    int a = 1;\n"
               "    unknown c{&a};\n"
               "    g(c);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 1));
        ASSERT_EQUALS(true, testValueOfXInconclusive(code, 7U, 1));

        code = "long foo();\n"
               "long bar();\n"
               "int test() {\n"
               "  bool b = true;\n"
               "  long a = foo();\n"
               "  if (a != 0)\n"
               "    return 1;\n"
               "  a = bar();\n"
               "  if (a != 0)\n"
               "    b = false;\n"
               "  int x = b;\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 12U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 12U, 0));

        code = "bool f(unsigned char uc) {\n"
               "  const bool x = uc;\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 1));

        code = "struct A {\n"
               "    int i, j;\n"
               "    int foo() {\n"
               "        i = 1;\n"
               "        j = 2;\n"
               "        int x = i;\n"
               "        return x;\n"
               "    }\n"
               "};\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 1));

        // global variable
        code = "int x;\n"
               "int foo(int y) {\n"
               "  if (y)\n"
               "    x = 10;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 10));

        code = "namespace A { int x; }\n"
               "int foo(int y) {\n"
               "  if (y)\n"
               "    A::x = 10;\n"
               "  return A::x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 10));

        // member variable
        code = "struct Fred {\n"
               "  int x;\n"
               "  int foo(int y) {\n"
               "    if (y)\n"
               "      x = 10;\n"
               "    return x;\n"
               "  }\n"
               "};";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 10));

        code = "void f(int i) {\n"
               "    if (i == 3) {}\n"
               "    for(int x = i;\n"
               "        x;\n"
               "        x++) {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 3));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 3));

        code = "void f() {\n"
               "    for(int x = 3;\n"
               "        x;\n"
               "        x++) {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 3));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 3));

        code = "void f() {\n"
               "    constexpr int x(123);\n"
               "    constexpr int y(x*x);\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 123));

        code = "void f() {\n"
               "    static const int x(123);\n"
               "    static const int y(x*x);\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 123));

        code = "void f() {\n"
               "    static int x(123);\n"
               "    static int y(x*x);\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "bool f() {\n" // #13208
               "    struct S {\n"
               "        bool b = true;\n"
               "        bool get() const { return b; }\n"
               "    };\n"
               "    S s;\n"
               "    s.b = false;\n"
               "    bool x = s.get();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 1));

        code = "int32_t f() {\n"
               "    const int32_t x = 0xffffffff;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, -1));
    }

    void valueFlowAfterSwap()
    {
        const char* code;

        code = "int f() {\n"
               "    int a = 1;\n"
               "    int b = 2;\n"
               "    std::swap(a, b);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 2));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 1));

        code = "int f() {\n"
               "    int a = 1;\n"
               "    int b = 2;\n"
               "    std::swap(a, b);\n"
               "    int x = b;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 2));

        code = "int f() {\n"
               "    std::string a;\n"
               "    std::string b=\"42\";\n"
               "    std::swap(b, a);\n"
               "    int x = b.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 2));

        code = "int f() {\n"
               "    std::string a;\n"
               "    std::string b=\"42\";\n"
               "    std::swap(b, a);\n"
               "    int x = a.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 2));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));
    }

    void valueFlowAfterCondition() {
        const char *code;
        // in if
        code = "void f(int x) {\n"
               "    if (x == 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x != 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 122));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        // ----

        code = "void f(int x) {\n"
               "    if (123 < x) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (123 > x) {\n"
               "        a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 122));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        // in else
        code = "void f(int x) {\n"
               "    if (x == 123) {}\n"
               "    else a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x != 123) {}\n"
               "    else a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        // after if
        code = "void f(int x) {\n"
               "    if (x == 10) {\n"
               "        x++;\n"
               "    }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 10));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 5U, 11));

        // !
        code = "void f(int x) {\n"
               "    if (!x) { a = x; }\n"
               "    else a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 0));

        code = "void f(int x, int y) {\n"
               "    if (!(x&&y)) { return; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int x) {\n"
               "    if (!x) { { throw new string(); }; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "void f(int x) {\n"
               "    if (x != 123) { throw " "; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x != 123) { }\n"
               "    else { throw " "; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));
        code = "void f(int x) {\n"
               "    if (x == 123) { }\n"
               "    else { throw " "; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));


        code = "void f(int x) {\n"
               "    if (x < 123) { }\n"
               "    else { a = x; }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) { }\n"
               "    else { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 122));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) { }\n"
               "    else { a = x; }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 123));

        code = "void f(int x) {\n"
               "    if (x > 123) { }\n"
               "    else { throw \"\"; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "void f(int x) {\n"
               "    if (x < 123) { return; }\n"
               "    else { return; }\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        // if (var)
        code = "void f(int x) {\n"
               "    if (x) { a = x; }\n"  // <- x is not 0
               "    else { b = x; }\n"    // <- x is 0
               "    c = x;\n"             // <- x might be 0
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 2U, 0));
        ASSERT_EQUALS(true,  testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 0));

        // After while
        code = "void f(int x) {\n"
               "    while (x != 3) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));

        code = "void f(int x) {\n"
               "    while (11 != (x = dostuff())) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 11));

        code = "void f(int x) {\n"
               "    while (11 != (x = dostuff()) && y) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 11));

        code = "void f(int x) {\n"
               "    while (x = dostuff()) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(const Token *x) {\n" // #5866
               "    x = x->next();\n"
               "    while (x) { x = x->next(); }\n"
               "    if (x->str()) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void f(const Token *x) {\n"
               "  while (0 != (x = x->next)) {}\n"
               "  x->ab = 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(const Token* x) {\n"
               "  while (0 != (x = x->next)) {}\n"
               "  if (x->str) {\n" // <- possible value 0
               "    x = y;\n" // <- this caused some problem
               "  }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        // conditional code after if/else/while
        code = "void f(int x) {\n"
               "  if (x == 2) {}\n"
               "  if (x > 0)\n"
               "    a = x;\n"  // <- TODO, x can be 2
               "  else\n"
               "    b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 2));
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 2));

        // condition with 2nd variable
        code = "void f(int x) {\n"
               "  int y = 0;\n"
               "  if (x == 7) { y = 1; }\n"
               "  if (!y)\n"
               "    a = x;\n" // <- x can not be 7 here
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 7));

        code = "void f(struct X *x) {\n"
               "  bool b = TRUE;\n"
               "  if(x) { }\n"
               "  else\n"
               "    b = FALSE;\n"
               "  if (b)\n"
               "    abc(x->value);\n" // <- x is not 0
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        // In condition, after && and ||
        code = "void f(int x) {\n"
               "  a = (x != 3 ||\n"
               "       x);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));

        code = "void f(int x) {\n"
               "  a = (x == 4 &&\n"
               "       x);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));

        // protected usage with &&
        code = "void f(const Token* x) {\n"
               "    if (x) {}\n"
               "    for (; x &&\n"
               "         x->str() != y; x = x->next()) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f(const Token* x) {\n"
               "    if (x) {}\n"
               "    if (x &&\n"
               "        x->str() != y) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        // return
        code = "void f(int x) {\n" // #6024
               "  if (x == 5) {\n"
               "    if (z) return; else return;\n"
               "  }\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 5));

        code = "void f(int x) {\n" // #6730
               "  if (x == 5) {\n"
               "    if (z) continue; else throw e;\n"
               "  }\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 5));

        // TODO: float
        code = "void f(float x) {\n"
               "  if (x == 0.5) {}\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // aliased variable
        code = "void f() {\n"
               "  int x = 1;\n"
               "  int *data = &x;\n"
               "  if (!x) {\n"
               "    calc(data);\n"
               "    a = x;\n"  // <- x might be changed by calc
               "  }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 0));

        code = "int* g();\n"
               "int f() {\n"
               "    int * x;\n"
               "    x = g();\n"
               "    if (x) { printf(\"\"); }\n"
               "    return *x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 0));

        // volatile variable
        code = "void foo(const volatile int &x) {\n"
               "    if (x==1) {\n"
               "        return x;\n"
               "    }"
               "}";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 1));

        code = "void foo(const std::atomic<int> &x) {\n"
               "    if (x==2) {\n"
               "        return x;\n"
               "    }"
               "}";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 2));

        code = "int f(int i, int j) {\n"
               "    if (i == 0) {\n"
               "        if (j < 0)\n"
               "            return 0;\n"
               "        i = j+1;\n"
               "    }\n"
               "    int x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 0));

        code = "int f(int i, int j) {\n"
               "    if (i == 0) {\n"
               "        if (j < 0)\n"
               "            return 0;\n"
               "        if (j < 0)\n"
               "            i = j+1;\n"
               "    }\n"
               "    int x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0));

        code = "void g(long& a);\n"
               "void f(long a) {\n"
               "    if (a == 0)\n"
               "        return;\n"
               "    if (a > 1)\n"
               "         g(a);\n"
               "    int x = a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 8U, 0));

        code = "int foo(int n) {\n"
               "    if( n>= 8 ) {\n"
               "        while(true) {\n"
               "            n -= 8;\n"
               "            if( n < 8 )\n"
               "                break;\n"
               "        }\n"
               "        int x = n == 0;\n"
               "        return x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 0));

        code = "bool c();\n"
               "long f() {\n"
               "    bool stop = false;\n"
               "    while (!stop) {\n"
               "        if (c())\n"
               "            stop = true;\n"
               "        break;\n"
               "    }\n"
               "    int x = !stop;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 10U, 1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 0));

        code = "int f(int a, int b) {\n"
               "  if (!a && !b)\n"
               "    return;\n"
               "  if ((!a && b) || (a && !b))\n"
               "    return;\n"
               "  int x = a;\n" // <- a is _not_ 0
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 0));

        code = "void f(int x, int y) {\n"
               "    if (x && y)\n"
               "        return;\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 4U, 1));

        code = "int f(std::vector<int> a, std::vector<int> b) {\n"
               "    if (a.empty() && b.empty())\n"
               "        return 0;\n"
               "    bool x = a.empty() && !b.empty();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 1));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 5U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 5U, 1));

        code = "auto f(int i) {\n"
               "    if (i == 0) return;\n"
               "    auto x = !i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "auto f(int i) {\n"
               "    if (i == 1) return;\n"
               "    auto x = !i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 4U, 0));

        code = "int g(int x) {\n"
               "    switch (x) {\n"
               "    case 1:\n"
               "        return 1;\n"
               "    default:\n"
               "        return 2;\n"
               "    }\n"
               "}\n"
               "void f(int x) {\n"
               "    if (x == 3)\n"
               "        x = g(0);\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 12U, 3));

        code = "long long f(const long long& x, const long long& y) {\n"
               "    switch (s) {\n"
               "    case 0:\n"
               "        if (x >= 64)\n"
               "            return 0;\n"
               "        return (long long)y << (long long)x;\n"
               "    case 1:\n"
               "        if (x >= 64) {\n"
               "        }\n"
               "    }\n"
               "    return 0; \n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 63));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 6U, 64));

        code = "long long f(const long long& x, const long long& y) {\n"
               "    switch (s) {\n"
               "    case 0:\n"
               "        if (x >= 64)\n"
               "            return 0;\n"
               "        return long long(y) << long long(x);\n"
               "    case 1:\n"
               "        if (x >= 64) {\n"
               "        }\n"
               "    }\n"
               "    return 0; \n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 63));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 6U, 64));

        code = "long long f(const long long& x, const long long& y) {\n"
               "    switch (s) {\n"
               "    case 0:\n"
               "        if (x >= 64)\n"
               "            return 0;\n"
               "        return long long{y} << long long{x};\n"
               "    case 1:\n"
               "        if (x >= 64) {\n"
               "        }\n"
               "    }\n"
               "    return 0; \n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 63));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 6U, 64));

        code = "int g(int x) { throw 0; }\n"
               "void f(int x) {\n"
               "    if (x == 3)\n"
               "        x = g(0);\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 5U, 3));

        code = "struct a {\n"
               "  a *b() const;\n"
               "  void c();\n"
               "};\n"
               "void e(a *x) {\n"
               "  while (x && x->b())\n"
               "    x = x->b();\n"
               "  x->c();\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 0));

        code = "struct a {\n"
               "  a *b();\n"
               "  void c();\n"
               "};\n"
               "void e(a *x) {\n"
               "  while (x && x->b())\n"
               "    x = x->b();\n"
               "  x->c();\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 0));

        code = "constexpr int f();\n"
               "int g() {\n"
               "    if (f() == 1) {\n"
               "        int x = f();\n"
               "        return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "int f(int x) {\n"
               "    if (x == 1) {\n"
               "        for(int i=0;i<1;i++) {\n"
               "            if (x == 1)\n"
               "                continue;\n"
               "        }\n"
               "    }\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 1));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 8U, 1));

        code = "void g(int i) {\n"
               "    if (i == 1)\n"
               "        return;\n"
               "    abort();\n"
               "}\n"
               "int f(int x) {\n"
               "    if (x != 0)\n"
               "        g(x);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 9U, 0));

        code = "int f(int a, int b) {\n"
               "    if (a > 0 && b > 0) {}\n"
               "    int x = a * b;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));
    }

    void valueFlowAfterConditionTernary()
    {
        const char* code;

        code = "auto f(int x) {\n"
               "    return x == 3 ?\n"
               "        x :\n"
               "        0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));

        code = "auto f(int x) {\n"
               "    return x != 3 ?\n"
               "        0 :\n"
               "        x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 3));

        code = "auto f(int x) {\n"
               "    return !(x == 3) ?\n"
               "        0 :\n"
               "        x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 3));

        code = "auto f(int* x) {\n"
               "    return x ?\n"
               "        x :\n"
               "        0;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "auto f(int* x) {\n"
               "    return x ?\n"
               "        0 :\n"
               "        x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "bool g(int);\n"
               "auto f(int* x) {\n"
               "    if (!g(x ?\n"
               "        *x :\n"
               "        0)) {}\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
    }

    void valueFlowAfterConditionExpr() {
        const char* code;

        code = "void f(int* p) {\n"
               "    if (p[0] == 123) {\n"
               "        int x = p[0];\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int y) {\n"
               "    if (y+1 == 123) {\n"
               "        int x = y+1;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int y) {\n"
               "    if (y+1 == 123) {\n"
               "        int x = y+2;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 124));

        code = "void f(int y, int z) {\n"
               "    if (y+z == 123) {\n"
               "        int x = y+z;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123));

        code = "void f(int y, int z) {\n"
               "    if (y+z == 123) {\n"
               "        y++;\n"
               "        int x = y+z;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 123));

        code = "void f(int y) {\n"
               "    if (y++ == 123) {\n"
               "        int x = y++;\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 124));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 125));

        code = "struct A {\n"
               "    bool g() const;\n"
               "};\n"
               "void f(A a) {\n"
               "    if (a.g()) {\n"
               "        bool x = a.g();\n"
               "        bool a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 0));

        code = "struct A {\n"
               "    bool g() const;\n"
               "};\n"
               "void f(A a) {\n"
               "    if (a.g()) {\n"
               "        bool x = !a.g();\n"
               "        bool a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 0));

        code = "struct A {\n"
               "    bool g() const;\n"
               "};\n"
               "void f(A a) {\n"
               "    if (!a.g()) {\n"
               "        bool x = a.g();\n"
               "        bool a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 0));

        code = "void f(std::vector<int> v) {\n"
               "    if (v.size() == 3) {\n"
               "        if (v.size() == 1) {\n"
               "            int x = 1;\n"
               "            int a = x;\n"
               "        }\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));
    }

    void valueFlowAfterConditionSeveralNot() {
        const char *code;

        code = "int f(int x, int y) {\n"
               "    if (x!=0) {}\n"
               "      return y/x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int x, int y) {\n"
               "    if (!!(x != 0)) {\n"
               "      return y/x;\n"
               "}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        code = "int f(int x, int y) {\n"
               "    if (!!!(x != 0)) {\n"
               "      return y/x;\n"
               "}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int x, int y) {\n"
               "    if (!!!!(x != 0)) {\n"
               "      return y/x;\n"
               "}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));
    }

    void valueFlowForwardCompoundAssign() {
        const char *code;

        code = "int f() {\n"
               "    int x = 123;\n"
               "    x += 43;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 166));
        ASSERT_EQUALS("2,Assignment 'x=123', assigned value is 123\n"
                      "3,Compound assignment '+=', assigned value is 166\n",
                      getErrorPathForX(code, 4U));

        code = "int f() {\n"
               "    int x = 123;\n"
               "    x /= 0;\n" // don't crash when evaluating x/=0
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 123));

        code = "float f() {\n"
               "    float x = 123.45f;\n"
               "    x += 67;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, static_cast<double>(123.45f) + 67, 0.01));

        code = "double f() {\n"
               "    double x = 123.45;\n"
               "    x += 67;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 123.45 + 67, 0.01));

        code = "void f() {\n"
               "    int x = 123;\n"
               "    x >>= 1;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 61));

        code = "int f() {\n"
               "    int x = 123;\n"
               "    x <<= 1;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 246));

        code = "int64_t f() {\n"
               "    const int64_t val = 1LL << 33;\n"
               "    int64_t x = val;\n"
               "    x += val;\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 1LL << 34));
    }

    void valueFlowForwardCorrelatedVariables() {
        const char *code;

        code = "void f(int x = 0) {\n"
               "  bool zero(x==0);\n"
               "  if (zero) a = x;\n"  // <- x is 0
               "  else b = x;\n"  // <- x is not 0
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "int g();\n"
               "int f(bool i, bool j) {\n"
               "    if (i && j) {}\n"
               "    else {\n"
               "        int x = 0;\n"
               "        if (i)\n"
               "            x = g();\n"
               "        return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 8U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 0));
    }

    void valueFlowForwardModifiedVariables() {
        const char *code;

        code = "void f(bool b) {\n"
               "  int x = 0;\n"
               "  if (b) x = 1;\n"
               "  else b = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(int i) {\n"
               "    int x = 0;\n"
               "    if (i == 0)\n"
               "        x = 1;\n"
               "    else if (!x && i == 1)\n"
               "        int b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 0));
    }

    void valueFlowForwardFunction() {
        const char *code;

        code = "class C {\n"
               "public:\n"
               "  C(int &i);\n" // non-const argument => might be changed
               "};\n"
               "int f() {\n"
               "  int x=1;\n"
               "  C c(x);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 1));

        code = "class C {\n"
               "public:\n"
               "  C(const int &i);\n" // const argument => is not changed
               "};\n"
               "int f() {\n"
               "  int x=1;\n"
               "  C c(x);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 1));

        code = "int f(int *);\n"
               "int g() {\n"
               "  const int a = 1;\n"
               "  int x = 11;\n"
               "  c = (a && f(&x));\n"
               "  if (x == 42) {}\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 11));

        code = "void f() {\n"
               "    int x = 1;\n"
               "    exit(x);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 1));

        code = "void f(jmp_buf env) {\n"
               "    int x = 1;\n"
               "    longjmp(env, x);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 1));
    }

    void valueFlowForwardTernary() {
        const char *code;

        code = "int f() {\n"
               "  int x=5;\n"
               "  a = b ? init1(&x) : init2(&x);\n"
               "  return 1 + x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 5));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 5));

        code = "int f(int *p) {\n" // #9008 - gcc ternary ?:
               "  if (p) return;\n"
               "  x = *p ? : 1;\n" // <- no explicit expr0
               "}";
        (void)testValueOfX(code, 1U, 0); // do not crash

        code = "void f(int a) {\n" // #8784
               "    int x = 13;\n"
               "    if (a == 1) x = 26;\n"
               "    return a == 1 ? x : 0;\n"  // <- x is 26
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 13));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 26));

        code = "void f(int* i) {\n"
               "    if (!i) return;\n"
               "    int * x = *i == 1 ? i : nullptr;\n"
               "    int* a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 4U, 0));
    }

    void valueFlowForwardLambda() {
        const char *code;

        code = "void f() {\n"
               "  int x=1;\n"
               "  auto f = [&](){ a=x; }\n"  // x is not 1
               "  x = 2;\n"
               "  f();\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, 2));

        code = "void f() {\n"
               "  int x=3;\n"
               "  auto f = [&](){ a=x; }\n"  // todo: x is 3
               "  f();\n"
               "}";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 3U, 3));

        code = "void f() {\n"
               "  int x=3;\n"
               "  auto f = [&](){ x++; }\n"
               "  x = 1;\n"
               "  f();\n"
               "  int a = x;\n" // x is actually 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 1));
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 3));
    }

    void valueFlowForwardTryCatch() {
        const char *code;

        code = "void g1();\n"
               "void g2();\n"
               "void f()\n {"
               "    bool x = false;\n"
               "    try {\n"
               "        g1();\n"
               "        x = true;\n"
               "        g2();\n"
               "    }\n"
               "    catch (...) {\n"
               "        if (x) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 11U, 1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 11U, 1));

        code = "void g1();\n"
               "void g2();\n"
               "void f()\n {"
               "    bool x = true;\n"
               "    try {\n"
               "        g1();\n"
               "        g2();\n"
               "    }\n"
               "    catch (...) {\n"
               "        if (x) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 10U, 1));
    }

    void valueFlowBitAnd() {
        const char *code;

        code = "int f(int a) {\n"
               "  int x = a & 0x80;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));
        ASSERT_EQUALS(true, testValueOfX(code,3U,0x80));

        code = "int f(int a) {\n"
               "  int x = a & 0x80 ? 1 : 2;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code,3U,0));
        ASSERT_EQUALS(false, testValueOfX(code,3U,0x80));

        code = "int f() {\n"
               "  int x = (19 - 3) & 15;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0));
        ASSERT_EQUALS(false, testValueOfX(code,3U,16));
    }

    void valueFlowForwardInconclusiveImpossible() {
        const char *code;

        code = "void foo() {\n"
               "    bool valid = f1();\n"
               "    if (!valid) return;\n"
               "    std::tie(endVal, valid) = f2();\n"
               "    bool x = !valid;"
               "    bool b = x;" // <- not always true
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 1));
    }

    void valueFlowForwardConst()
    {
        const char* code;

        code = "int f() {\n"
               "    const int i = 2;\n"
               "    const int x = i+1;\n"
               "    goto end;\n"
               "end:\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 3));

        code = "int f() {\n"
               "    int i = 2;\n"
               "    const int& x = i;\n"
               "    i++;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 2));

        code = "int f(int a, int b, int c) {\n"
               "    const int i = 2;\n"
               "    const int x = i+1;\n"
               "    if (a == x) { return 0; }\n"
               "    if (b == x) { return 0; }\n"
               "    if (c == x) { return 0; }\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 3));

        code = "int f(int a, int b, int c) {\n"
               "    const int i = 2;\n"
               "    const int y = i+1;\n"
               "    const int& x = y;\n"
               "    if (a == x) { return 0; }\n"
               "    if (b == x) { return 0; }\n"
               "    if (c == x) { return 0; }\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 8U, 3));

        code = "int f(int a, int b, int c, int x) {\n"
               "    const int i = 2;\n"
               "    const int y = i+1;\n"
               "    if (a == y) { return 0; }\n"
               "    if (b == y) { return 0; }\n"
               "    if (c == y) { return 0; }\n"
               "    if (x == y)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 8U, 3));
    }

    void valueFlowForwardAfterCondition()
    {
        const char* code;

        code = "int g();\n"
               "void f() {\n"
               "    int x = 3;\n"
               "    int kk = 11;\n"
               "    for (;;) {\n"
               "        if (kk > 10) {\n"
               "            kk = 0;\n"
               "            x = g();\n"
               "        }\n"
               "        kk++;\n"
               "        int a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 3));

        code = "int g();\n"
               "void f() {\n"
               "    int x = 3;\n"
               "    int kk = 11;\n"
               "    while (true) {\n"
               "        if (kk > 10) {\n"
               "            kk = 0;\n"
               "            x = g();\n"
               "        }\n"
               "        kk++;\n"
               "        int a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 3));

        code = "int g();\n"
               "void f() {\n"
               "    int x = 3;\n"
               "    int kk = 11;\n"
               "    if (true) {\n"
               "        if (kk > 10) {\n"
               "            kk = 0;\n"
               "            x = g();\n"
               "        }\n"
               "        kk++;\n"
               "        int a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 11U, 3));
    }

    void valueFlowRightShift() {
        const char *code;
        /* Set some temporary fixed values to simplify testing */
        /*const*/ Settings s = settings;
        s.platform.int_bit = 32;
        s.platform.long_bit = 64;
        s.platform.long_long_bit = MathLib::bigint_bits * 2;

        code = "int f(int a) {\n"
               "  int x = (a & 0xff) >> 16;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0,&s));

        code = "int f(unsigned int a) {\n"
               "  int x = (a % 123) >> 16;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code,3U,0,&s));

        code = "int f(int y) {\n"
               "  int x = (y & 0xFFFFFFF) >> 31;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0));

        code = "int f(int y) {\n"
               "  int x = (y & 0xFFFFFFF) >> 32;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0,&s));

        code = "int f(short y) {\n"
               "  int x = (y & 0xFFFFFF) >> 31;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0,&s));

        code = "int f(short y) {\n"
               "  int x = (y & 0xFFFFFF) >> 32;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0,&s));

        code = "int f(long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 63;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0,&s));

        code = "int f(long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 64;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0,&s));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 63;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3u, 0,&s));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 64;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0,&s));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 121;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0,&s));

        code = "int f(long long y) {\n"
               "  int x = (y & 0xFFFFFF) >> 128;\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 3u, 0,&s));
    }

    void valueFlowFwdAnalysis() {
        const char *code;
        std::list<ValueFlow::Value> values;

        code = "void f() {\n"
               "  struct Foo foo;\n"
               "  foo.x = 1;\n"
               "  x = 0 + foo.x;\n" // <- foo.x is 1
               "}";
        values = removeSymbolicTok(tokenValues(code, "+"));
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(true, values.front().isKnown());
        ASSERT_EQUALS(true, values.front().isIntValue());
        ASSERT_EQUALS(1, values.front().intvalue);

        code = "void f() {\n"
               "  S s;\n"
               "  s.x = 1;\n"
               "  int y = 10;\n"
               "  while (s.x < y)\n" // s.x does not have known value
               "    s.x++;\n"
               "}";
        values = removeImpossible(tokenValues(code, "<"));
        ASSERT_EQUALS(1, values.size());
        ASSERT(values.front().isPossible());
        ASSERT_EQUALS(1, values.front().intvalue);

        code = "void f() {\n"
               "  S s;\n"
               "  s.x = 37;\n"
               "  int y = 10;\n"
               "  while (s.x < y)\n" // s.x has a known value
               "    y--;\n"
               "}";
        values = tokenValues(code, ". x <");
        ASSERT(values.size() == 1 &&
               values.front().isKnown() &&
               values.front().isIntValue() &&
               values.front().intvalue == 37);

        code = "void f() {\n"
               "  Hints hints;\n"
               "  hints.x = 1;\n"
               "  if (foo)\n"
               "    hints.x = 2;\n"
               "  x = 0 + foo.x;\n" // <- foo.x is possible 1, possible 2
               "}";
        values = removeSymbolicTok(tokenValues(code, "+"));
        TODO_ASSERT_EQUALS(2U, 0U, values.size()); // should be 2

        // FP: Condition '*b>0' is always true
        code = "bool dostuff(const char *x, const char *y);\n"
               "void fun(char *s, int *b) {\n"
               "  for (int i = 0; i < 42; ++i) {\n"
               "    if (dostuff(s, \"1\")) {\n"
               "      *b = 1;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  if (*b > 0) {\n" // *b does not have known value
               "  }\n"
               "}";
        values = removeImpossible(tokenValues(code, ">"));
        ASSERT_EQUALS(1, values.size());
        ASSERT(values.front().isPossible());
        ASSERT_EQUALS(1, values.front().intvalue);

        code = "void foo() {\n"
               "    struct ISO_PVD_s pvd;\n"
               "    pvd.descr_type = 0xff;\n"
               "    do {\n"
               "        if (pvd.descr_type == 0xff) {}\n"
               "        dostuff(&pvd);\n"
               "    } while (condition);\n"
               "}";
        values = removeImpossible(tokenValues(code, "=="));
        ASSERT_EQUALS(1, values.size());
        ASSERT(values.front().isPossible());
        ASSERT_EQUALS(1, values.front().intvalue);

        // for loops
        code = "struct S { int x; };\n" // #9036
               "void foo(struct S s) {\n"
               "    for (s.x = 0; s.x < 127; s.x++) {}\n"
               "}";
        values = removeImpossible(tokenValues(code, "<"));
        values.remove_if([&](const ValueFlow::Value& v) {
            return !v.isKnown();
        });
        ASSERT_EQUALS(true, values.empty());
    }

    void valueFlowSwitchVariable() {
        const char code[] = "void f(int x) {\n"
                            "    a = x - 1;\n"  // <- x can be 14
                            "    switch (x) {\n"
                            "    case 14: a=x+2; break;\n"  // <- x is 14
                            "    };\n"
                            "    a = x;\n"  // <- x can be 14
                            "}";
        ASSERT_EQUALS(true, testConditionalValueOfX(code, 2U, 14));
        TODO_ASSERT_EQUALS(true, false, testConditionalValueOfX(code, 4U, 14));
        TODO_ASSERT_EQUALS(true, false, testConditionalValueOfX(code, 6U, 14));

        ValueFlow::Value value1 = valueOfTok(code, "-");
        ASSERT_EQUALS(13, value1.intvalue);
        ASSERT(!value1.isKnown());

        ValueFlow::Value value2 = valueOfTok(code, "+");
        TODO_ASSERT_EQUALS(16, 0, value2.intvalue);
        TODO_ASSERT_EQUALS(true, false, value2.isKnown());
    }

    void valueFlowForLoop() {
        const char *code;
        ValueFlow::Value value;

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 10));

        code = "void f() {\n"
               "    int x;\n"
               "    for (x = 2; x < 1; x++)\n"
               "        a[x] = 0;\n" // <- not 2
               "    b = x;\n" // 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "void f() {\n"
               "    int x;\n"
               "    for (x = 2; x < 1; ++x)\n"
               "        a[x] = 0;\n" // <- not 2
               "    b = x;\n" // 2
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 2));

        code = "enum AB {A,B};\n" // enum => handled by valueForLoop2
               "void f() {\n"
               "    int x;\n"
               "    for (x = 1; x < B; ++x)\n"
               "        a[x] = 0;\n" // <- not 1
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 1));

        code = "void f(int a) {\n"
               "    for (int x = a; x < 10; x++)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));

        code = "void f() {\n"
               "    for (int x = 0; x < 5; x += 2)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x = x + 2)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 8));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 10));

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x = x / 0)\n"
               "        a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0)); // don't crash

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        x<4 ?\n"
               "        a[x] : 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));

        code = "void f() {\n"
               "    for (int x = 0; x < 10; x++)\n"
               "        x==0 ?\n"
               "        0 : a[x];\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n" // #5223
               "    for (int x = 0; x < 300 && x < 18; x++)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 17));
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 299));

        code = "void f() {\n"
               "    int x;\n"
               "    for (int i = 0; x = bar[i]; i++)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    const char abc[] = \"abc\";\n"
               "    int x;\n"
               "    for (x = 0; abc[x] != '\\0'; x++) {}\n"
               "    a[x] = 0;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 3));

        code = "void f() {\n" // #5939
               "    int x;\n"
               "    for (int x = 0; (x = do_something()) != 0;)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x;\n"
               "    for (int x = 0; x < 10 && y = do_something();)\n"
               "        x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void f() {\n"
               "    int x,y;\n"
               "    for (x = 0, y = 0; x < 10, y < 10; x++, y++)\n" // usage of ,
               "        x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));

        code = "void foo(double recoveredX) {\n"
               "  for (double x = 1e-18; x < 1e40; x *= 1.9) {\n"
               "    double relativeError = (x - recoveredX) / x;\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 0));

        // Ticket #7139
        // "<<" in third expression of for
        code = "void f(void) {\n"
               "    int bit, x;\n"
               "    for (bit = 1, x = 0; bit < 128; bit = bit << 1, x++) {\n"
               "        z = x;\n"       // <- known value [0..6]
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 6));
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 7));

        // &&
        code = "void foo() {\n"
               "  for (int x = 0; x < 10; x++) {\n"
               "    if (x > 1\n"
               "        && x) {}" // <- x is not 0
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        // Known to be true, but it could also be 9
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "void foo() {\n"
               "  for (int x = 0; x < 10; x++) {\n"
               "    if (x < value\n"
               "        && x) {}" // <- maybe x is not 9
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 9));

        // ||
        code = "void foo() {\n"
               "  for (int x = 0; x < 10; x++) {\n"
               "    if (x == 0\n"
               "        || x) {}" // <- x is not 0
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));
        // Known to be true, but it could also be 9
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        // After loop
        code = "void foo() {\n"
               "  int x;\n"
               "  for (x = 0; x < 10; x++) {}\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 10));

        code = "void foo() {\n"
               "  int x;\n"
               "  for (x = 0; 2 * x < 20; x++) {}\n"
               "  a = x;\n"
               "}\n";
        ASSERT_EQUALS(true,  testValueOfX(code, 4U, 10));

        code = "void foo() {\n" // related with #887
               "  int x;\n"
               "  for (x = 0; x < 20; x++) {}\n"
               "  a = x++;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 20));

        code = "void f() {\n"
               "  int x;\n"
               "  for (x = 0; x < 5; x++) {}\n"
               "  if (x == 5) {\n"
               "    abort();\n"
               "  }\n"
               "  a = x;\n" // <- x can't be 5
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 7U, 5));

        code = "void f() {\n"
               "  int x;\n"
               "  for (x = 0; x < 5; x++) {}\n"
               "  if (x < 5) {}\n"
               "  else return;\n"
               "  a = x;\n" // <- x can't be 5
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 5));

        // assert after for loop..
        code = "static void f() {\n"
               "  int x;\n"
               "  int ctls[10];\n"
               "  for (x = 0; x <= 10; x++) {\n"
               "    if (cond)\n"
               "      break;\n"
               "  }\n"
               "  assert(x <= 10);\n"
               "  ctls[x] = 123;\n" // <- x can't be 11
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 11));

        // hang
        code = "void f() {\n"
               "  for(int i = 0; i < 20; i++)\n"
               "    n = (int)(i < 10 || abs(negWander) < abs(negTravel));\n"
               "}";
        (void)testValueOfX(code,0,0); // <- don't hang

        // crash (daca@home)
        code = "void foo(char *z, int n) {\n"
               "    int i;\n"
               "    if (fPScript) {\n"
               "        i = 1;\n"
               "    } else if (strncmp(&z[n], \"<!--\", 4) == 0) {\n"
               "        for (i = 4;;) {\n"
               "            if (z[n] && strncmp(&z[n+i], \"-->\", 3) == 0) ;\n"
               "        }\n"
               "    }\n"
               "}";
        (void)testValueOfX(code,0,0); // <- don't crash

        // conditional code in loop
        code = "void f(int mask) {\n" // #6000
               "  for (int x = 10; x < 14; x++) {\n"
               "    int bit = mask & (1 << i);\n"
               "    if (bit) {\n"
               "      if (bit == (1 << 10)) {}\n"
               "      else { a = x; }\n" // <- x is not 10
               "    }\n"
               "  }\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, 10));

        // #7886 - valueFlowForLoop must be called after valueFlowAfterAssign
        code = "void f() {\n"
               "  int sz = 4;\n"
               "  int x,y;\n"
               "  for(x=0,y=0; x < sz && y < 10; x++)\n"
               "    a = x;\n" // <- max value is 3
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 3));

        code = "void f() {\n"
               "    int x;\n"
               "    for (x = 0; x < 10; x++)\n"
               "        x;\n"
               "}";
        std::list<ValueFlow::Value> values = tokenValues(code, "x <");
        ASSERT(std::none_of(values.cbegin(), values.cend(), std::mem_fn(&ValueFlow::Value::isUninitValue)));

        // #9637
        code = "void f() {\n"
               "    unsigned int x = 0;\n"
               "    for (x = 0; x < 2; x++) {}\n"
               "}\n";
        value = valueOfTok(code, "x <");
        ASSERT(value.isPossible());
        ASSERT_EQUALS(0, value.intvalue);

        code = "void f() {\n"
               "    unsigned int x = 0;\n"
               "    for (;x < 2; x++) {}\n"
               "}\n";
        value = valueOfTok(code, "x <");
        ASSERT(value.isPossible());
        ASSERT_EQUALS(0, value.intvalue);

        code = "void f() {\n"
               "    unsigned int x = 1;\n"
               "    for (x = 0; x < 2; x++) {}\n"
               "}\n";
        value = valueOfTok(code, "x <");
        ASSERT(!value.isKnown());

        code = "void b(int* a) {\n" // #10795
               "    for (*a = 1;;)\n"
               "        if (0) {}\n"
               "}\n"
               "struct S { int* a; }\n"
               "void b(S& s) {\n"
               "    for (*s.a = 1;;)\n"
               "        if (0) {}\n"
               "}\n"
               "struct T { S s; };\n"
               "void b(T& t) {\n"
               "    for (*&t.s.a[0] = 1;;)\n"
               "        if (0) {}\n"
               "}\n";
        ASSERT_NO_THROW(testValueOfX(code, 0, 0));

        code = "void f() {\n"
               "    int p[2];\n"
               "    for (p[0] = 0; p[0] <= 2; p[0]++) {\n"
               "        for (p[1] = 0; p[1] <= 2 - p[0]; p[1]++) {}\n"
               "    }\n"
               "}\n";
        ASSERT_NO_THROW(testValueOfX(code, 0, 0));

        code = "struct C {\n" // #10828
               "    int& v() { return i; }\n"
               "    int& w() { return j; }\n"
               "    int i{}, j{};\n"
               "};\n"
               "void f() {\n"
               "    C c;\n"
               "    for (c.w() = 0; c.w() < 2; c.w()++) {\n"
               "        for (c.v() = 0; c.v() < 24; c.v()++) {}\n"
               "    }\n"
               "}\n";
        ASSERT_NO_THROW(testValueOfX(code, 0, 0));

        // #11072
        code = "struct a {\n"
               "    long b;\n"
               "    long c[6];\n"
               "    long d;\n"
               "};\n"
               "void e(long) {\n"
               "    a f = {0};\n"
               "    for (f.d = 0; 2; f.d++)\n"
               "        e(f.c[f.b]);\n"
               "}\n";
        values = tokenValues(code, ". c");
        ASSERT_EQUALS(true, values.empty());
        values = tokenValues(code, "[ f . b");
        ASSERT_EQUALS(true, values.empty());

        code = "void f() {\n" // #13109
               "    const int a[10] = {};\n"
               "    for (int n = 0; 1; ++n) {\n"
               "        (void)a[n];\n"
               "        break;\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "n ]");
        ASSERT_EQUALS(2, values.size());
        auto it = values.begin();
        ASSERT_EQUALS(-1, it->intvalue);
        ASSERT(it->isImpossible());
        ++it;
        ASSERT_EQUALS(0, it->intvalue);
        ASSERT(it->isPossible());

        code = "void f() {\n"
               "    const int a[10] = {};\n"
               "    for (int n = 0; 1; ++n) {\n"
               "        if (a[n] < 1)\n"
               "            break;\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "n ]");
        ASSERT_EQUALS(2, values.size());
        it = values.begin();
        ASSERT_EQUALS(-1, it->intvalue);
        ASSERT(it->isImpossible());
        ++it;
        ASSERT_EQUALS(0, it->intvalue);
        ASSERT(it->isPossible());

        code = "void f() {\n"
               "    const int a[10] = {};\n"
               "    for (int n = 0; 1; ++n) {\n"
               "        if (a[n] < 1)\n"
               "            throw 0;\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "n ]");
        ASSERT_EQUALS(2, values.size());
        it = values.begin();
        ASSERT_EQUALS(-1, it->intvalue);
        ASSERT(it->isImpossible());
        ++it;
        ASSERT_EQUALS(0, it->intvalue);
        ASSERT(it->isPossible());

        code = "void f() {\n"
               "    const int a[10] = {};\n"
               "    for (int n = 0; 1; ++n) {\n"
               "        (void)a[n];\n"
               "        exit(1);\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "n ]");
        ASSERT_EQUALS(2, values.size());
        it = values.begin();
        ASSERT_EQUALS(-1, it->intvalue);
        ASSERT(it->isImpossible());
        ++it;
        ASSERT_EQUALS(0, it->intvalue);
        ASSERT(it->isPossible());

        code = "void f() {\n"
               "    const int a[10] = {};\n"
               "    for (int n = 0; 1; ++n) {\n"
               "        if (a[n] < 1)\n"
               "            exit(1);\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "n ]");
        ASSERT_EQUALS(2, values.size());
        it = values.begin();
        ASSERT_EQUALS(-1, it->intvalue);
        ASSERT(it->isImpossible());
        ++it;
        ASSERT_EQUALS(0, it->intvalue);
        ASSERT(it->isPossible());
    }

    void valueFlowSubFunction() {
        const char *code;

        code = "int f(int size) {\n"
               "    int x = 0;\n"
               "    if(size>16) {\n"
               "        x = size;\n"
               "        int a = x;\n"
               "    }\n"
               "    return x;\n"
               "}\n"
               "void g(){\n"
               "    f(42);\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 17));
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 42));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 17));
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 42));

        code = "void g(int, int) {}\n"
               "void f(int x, int y) {\n"
               "    g(x, y);\n"
               "}\n"
               "void h() {\n"
               "    f(0, 0);\n"
               "    f(1, 1);\n"
               "    f(2, 2);\n"
               "    f(3, 3);\n"
               "    f(4, 4);\n"
               "    f(5, 5);\n"
               "    f(6, 6);\n"
               "    f(7, 7);\n"
               "    f(8, 8);\n"
               "    f(9, 9);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 2));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 3));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 4));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 5));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 6));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 7));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 8));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 9));

        code = "int f(int i, int j) {\n"
               "    if (i == j) {\n"
               "        int x = i;\n"
               "        return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n"
               "int g(int x) {\n"
               "    f(x, -1);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, -1));

        code = "void g() {\n"
               "    const std::vector<int> v;\n"
               "    f(v);\n"
               "}\n"
               "void f(const std::vector<int>& w) {\n"
               "    for (int i = 0; i < w.size(); ++i) {\n"
               "        int x = i != 0;\n"
               "        int a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 8U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 8U, 1));

        code = "void g() {\n"
               "    const std::vector<int> v;\n"
               "    f(v);\n"
               "}\n"
               "void f(const std::vector<int>& w) {\n"
               "    for (int i = 0; i < w.size(); ++i) {\n"
               "        int x = i;\n"
               "        int a = x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, -1));

        code = "typedef enum {\n"
               "  K0, K1\n"
               "} K;\n"
               "bool valid(Object *obj, K x) {\n"
               "  if (!obj || obj->kind != x)\n"
               "    return false;\n"
               "  return x == K0;\n"
               "}\n"
               "void f(Object *obj) {\n"
               "  if (valid(obj, K0)) {}\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 7U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, 0));

        code = "int f(int i) {\n"
               "    int x = abs(i);\n"
               "    return x;\n"
               "}\n"
               "void g() {\n"
               "    f(1);\n"
               "    f(0);\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void foo(int* p, int* x) {\n"
               "    bool b1 = (p != NULL);\n"
               "    bool b2 = b1 && (x != NULL);\n"
               "    if (b2) {\n"
               "        *x = 3;\n"
               "    }\n"
               "}\n"
               "\n"
               "void bar() {\n"
               "    foo(NULL, NULL);\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
    }

    void valueFlowFunctionReturn() {
        const char *code;

        code = "int f1(int x) {\n"
               "  return x+1;\n"
               "}\n"
               "void f2() {\n"
               "    x = 10 - f1(2);\n"
               "}";
        ASSERT_EQUALS(7, valueOfTok(code, "-").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "-").isKnown());

        code = "int add(int x, int y) {\n"
               "  return x+y;\n"
               "}\n"
               "void f2() {\n"
               "    x = 2 * add(10+1,4);\n"
               "}";
        ASSERT_EQUALS(30, valueOfTok(code, "*").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "*").isKnown());

        code = "int one() { return 1; }\n"
               "void f() { x = 2 * one(); }";
        ASSERT_EQUALS(2, valueOfTok(code, "*").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "*").isKnown());

        code = "int add(int x, int y) {\n"
               "  return x+y;\n"
               "}\n"
               "void f2() {\n"
               "    x = 2 * add(1,add(2,3));\n"
               "}";
        ASSERT_EQUALS(12, valueOfTok(code, "*").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "*").isKnown());

        code = "int f(int i, X x) {\n"
               "    if (i)\n"
               "        return g(std::move(x));\n"
               "    g(x);\n"
               "    return 0;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, ValueFlow::Value::MoveKind::MovedVariable));

        code = "class A\n"
               "{\n"
               "    int f1(int x) {\n"
               "        return x+1;\n"
               "    }\n"
               "    void f2() {\n"
               "        x = 10 - f1(2);\n"
               "    }\n"
               "};";
        ASSERT_EQUALS(7, valueOfTok(code, "-").intvalue);
        ASSERT_EQUALS(true, valueOfTok(code, "-").isKnown());

        code = "class A\n"
               "{\n"
               "    virtual int f1(int x) {\n"
               "        return x+1;\n"
               "    }\n"
               "    void f2() {\n"
               "        x = 10 - f1(2);\n"
               "    }\n"
               "};";
        TODO_ASSERT_EQUALS(7, 0, valueOfTok(code, "-").intvalue);
        ASSERT_EQUALS(false, valueOfTok(code, "-").isKnown());

        code = "struct base {\n"
               "    virtual int f() { return 0; }\n"
               "};\n"
               "void g(base* b) {\n"
               "    int x = b->f();\n"
               "    return x;\n"
               "}\n";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 6U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));

        code = "struct base {\n"
               "    virtual int f() { return 0; }\n"
               "};\n"
               "void g(base& b) {\n"
               "    int x = b.f();\n"
               "    return x;\n"
               "}\n";
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 6U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));

        code = "struct base {\n"
               "    virtual int f() { return 0; }\n"
               "};\n"
               "struct derived : base {\n"
               "    int f() override { return 1; }\n"
               "};\n"
               "void g(derived* d) {\n"
               "    base* b = d;\n"
               "    int x = b->f();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 10U, 0));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 10U, 1));

        code = "struct base {\n"
               "    virtual int f() final { return 0; }\n"
               "};\n"
               "void g(base* b) {\n"
               "    int x = b->f();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 6U, 0));

        code = "int* g(int& i, bool b) {\n"
               "    if(b)\n"
               "        return nullptr;\n"
               "    return &i;\n"
               "}   \n"
               "int f(int i) {\n"
               "    int* x = g(i, true);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 8U, 0));

        code = "int* g(int& i, bool b) {\n"
               "    if(b)\n"
               "        return nullptr;\n"
               "    return &i;\n"
               "}   \n"
               "int f(int i) {\n"
               "    int* x = g(i, false);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 8U, 0));

        code = "int* g(int& i, bool b) {\n"
               "    if(b)\n"
               "        return nullptr;\n"
               "    return &i;\n"
               "}   \n"
               "int f(int i) {\n"
               "    int* x = g(i, i == 3);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 8U, 0));

        code = "struct A {\n"
               "    unsigned i;\n"
               "    bool f(unsigned x) const {\n"
               "        return ((i & x) != 0);\n"
               "    }\n"
               "};\n"
               "int g(A& a) {\n"
               "    int x = a.f(2);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 1));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 9U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 9U, 1));

        code = "struct A {\n"
               "    enum {\n"
               "        b = 0,\n"
               "        c = 1,\n"
               "        d = 2\n"
               "    };\n"
               "    bool isb() const {\n"
               "        return e == b;\n"
               "    }\n"
               "    unsigned int e;\n"
               "};\n"
               "int f(A g) {\n"
               "  int x = !g.isb();\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 14U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 14U, 1));

        code = "bool h(char q);\n"
               "bool g(char q) {\n"
               "    if (!h(q))\n"
               "        return false;\n"
               "    return true;\n"
               "}\n"
               "int f() {\n"
               "    int x = g(0);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 0));
        ASSERT_EQUALS(false, testValueOfX(code, 9U, 1));
    }

    void valueFlowFunctionDefaultParameter() {
        const char *code;

        code = "class continuous_src_time {\n"
               "    continuous_src_time(std::complex<double> f, double st = 0.0, double et = infinity) {}\n"
               "};";
        (void)testValueOfX(code, 2U, 2); // Don't crash (#6494)

        code = "struct S {\n" // #14693
               "    int i;\n"
               "    explicit S(std::string&& s = {}) : i(0) {}\n"
               "};";
        ASSERT_EQUALS(false, testKnownValueOfTok(code, "{ } )", 0));
    }

    bool isNotKnownValues(const char code[], const char str[]) {
        const auto& values = tokenValues(code, str);
        return std::none_of(values.cbegin(), values.cend(), [](const ValueFlow::Value& v) {
            return v.isKnown();
        });
    }

    void knownValue() {
        const char *code;
        ValueFlow::Value value;

        ASSERT(valueOfTok("x = 1;", "1").isKnown());

        // after assignment
        code = "void f() {\n"
               "  int x = 1;\n"
               "  return x + 2;\n" // <- known value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(3, value.intvalue);
        ASSERT(value.isKnown());

        {
            code = "void f() {\n"
                   "  int x = 15;\n"
                   "  if (x == 15) { x += 7; }\n" // <- condition is true
                   "}";
            value = valueOfTok(code, "==");
            ASSERT_EQUALS(1, value.intvalue);
            ASSERT(value.isKnown());

            code = "int f() {\n"
                   "    int a = 0, x = 0;\n"
                   "    a = index();\n"
                   "    if (a != 0)\n"
                   "        x = next();\n"
                   "    return x + 1;\n"
                   "}\n";
            value = valueOfTok(code, "+");
            ASSERT(value.isPossible());
        }

        code = "void f() {\n"
               "  int x;\n"
               "  if (ab) { x = 7; }\n"
               "  return x + 2;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(9, value.intvalue);
        ASSERT(value.isPossible());

        code = "void f(int c) {\n"
               "  int x = 0;\n"
               "  if (c) {} else { x++; }\n"
               "  return x + 2;\n" // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "+"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(&x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(0 ? ptr : &x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(unknown ? ptr : &x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  fred.dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void dostuff(int x);\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        value = valueOfTok(code, "<");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT(value.isKnown());

        code = "void dostuff(int & x);\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void dostuff(const int & x);\n"
               "void f() {\n"
               "  int x = 0;\n"
               "  dostuff(x);\n"
               "  if (x < 0) {}\n"
               "}\n";
        value = valueOfTok(code, "<");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f() {\n"
               "  int x = 0;\n"
               "  do {\n"
               "    if (x < 0) {}\n"
               "    fred.dostuff(x);\n"
               "  } while (abc);\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "<"));

        code = "int x;\n"
               "void f() {\n"
               "  x = 4;\n"
               "  while (1) {\n"
               "    a = x+2;\n"
               "    dostuff();\n"
               "  }\n"
               "}";
        ASSERT(isNotKnownValues(code, "+"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  if (y) { dostuff(x); }\n"
               "  if (!x) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  MACRO( v, { if (y) { x++; } } );\n"
               "  if (!x) {}\n"
               "}\n";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  for (int i = 0; i < 10; i++) {\n"
               "    if (cond) {\n"
               "      x = 1;\n"
               "      break;\n"
               "    }\n"
               "  }\n"
               "  if (!x) {}\n"  // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n" // #8356
               "  bool b = false;\n"
               "  for(int x = 3; !b && x < 10; x++) {\n" // <- b has known value
               "    for(int y = 4; !b && y < 20; y++) {}\n"
               "  }\n"
               "}";
        value = valueOfTok(code, "!");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f() {\n"
               "  int x = 0;\n"
               "  switch (state) {\n"
               "  case 1:\n"
               "    x = 1;\n"
               "    break;\n"
               "  }\n"
               "  if (!x) {}\n"  // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n" // #7049
               "  int x = 0;\n"
               "  switch (a) {\n"
               "  case 1:\n"
               "    x = 1;\n"
               "  case 2:\n"
               "    if (!x) {}\n" // <- possible value
               "  }\n"
               "}";
        ASSERT(isNotKnownValues(code, "!"));

        code = "void f() {\n"
               "  int x = 0;\n"
               "  while (!x) {\n" // <- possible value
               "    scanf(\"%d\", &x);\n"
               "  }\n"
               "}";
        value = valueOfTok(code, "!");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isPossible());

        code = "void f() {\n"
               "  int x = 0;\n"
               "  do { } while (++x < 12);\n" // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, "<"));

        code = "void f() {\n"
               "  static int x = 0;\n"
               "  return x + 1;\n" // <- known value
               "}\n";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f() {\n"
               "  int x = 0;\n"
               "a:\n"
               "  a = x + 1;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isPossible());

        // in conditional code
        code = "void f(int x) {\n"
               "  if (!x) {\n"
               "    a = x+1;\n" // <- known value
               "  }\n"
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "void f(int x) {\n"
               "  if (a && 4==x && y) {\n"
               "    a = x+12;\n" // <- known value
               "  }\n"
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(16, value.intvalue);
        ASSERT(value.isKnown());

        // after condition
        code = "int f(int x) {\n"
               "  if (x == 4) {}\n"
               "  return x + 1;\n" // <- possible value
               "}";
        value = valueOfTok(code, "+");
        ASSERT_EQUALS(5, value.intvalue);
        ASSERT(value.isPossible());

        code = "int f(int x) {\n"
               "  if (x < 2) {}\n"
               "  else if (x >= 2) {}\n" // <- known value
               "}";
        value = valueOfTok(code, ">=");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT(value.isKnown());

        code = "int f(int x) {\n"
               "  if (x < 2) {}\n"
               "  else if (x > 2) {}\n" // <- possible value
               "}";
        ASSERT(isNotKnownValues(code, ">"));

        // known and possible value
        code = "void f() {\n"
               "    int x = 1;\n"
               "    int y = 2 + x;\n" // <- known value, don't care about condition
               "    if (x == 2) {}\n"
               "}";
        ASSERT_EQUALS(true,  testValueOfX(code, 3U, 1)); // value of x can be 1
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 2)); // value of x can't be 2

        code = "bool f() {\n"
               "  const int s( 4 );"
               "  return s == 4;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT(value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  const int s{ 4 };"
               "  return s == 4;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT(value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  const int s = int( 4 );"
               "  return s == 4;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT(value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  const int s = int{ 4 };"
               "  return s == 4;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT(value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  const int s = int{};"
               "  return s == 0;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  const int s = int();"
               "  return s == 0;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  const int s{};"
               "  return s == 0;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int* p{};\n"
               "  return p == nullptr;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int* p{ nullptr };\n"
               "  return p == nullptr;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int* p{ 0 };\n"
               "  return p == nullptr;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int* p = {};\n"
               "  return p == nullptr;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int i = {};\n"
               "  return i == 0;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int* p = { 0 };\n"
               "  return p == nullptr;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        code = "bool f() {\n"
               "  int i = { 1 };\n"
               "  return i == 1;\n" // <- known value
               "}";
        value = valueOfTok(code, "==");
        ASSERT_EQUALS(true, value.isKnown());
        ASSERT_EQUALS(1, value.intvalue);

        // calculation with known result
        code = "int f(int x) { a = x & 0; }"; // <- & is 0
        value = valueOfTok(code, "&");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT(value.isKnown());

        // template parameters are not known
        code = "template <int X> void f() { a = X; }\n"
               "f<1>();";
        value = valueOfTok(code, "1");
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT_EQUALS(false, value.isKnown());

        code = "void f(char c, struct T* t) {\n" // #11894
               "    (*t->func)(&c, 1, t->ptr);\n"
               "}\n";
        value = valueOfTok(code, ", 1");
        ASSERT_EQUALS(0, value.intvalue);
        ASSERT_EQUALS(false, value.isKnown());

        // #13959
        const Settings settingsOld = settings;
        settings.standards.c = Standards::C23;
        code = "void f(int* p) {\n"
               "    if (p == nullptr)\n"
               "        return;\n"
               "    if (p) {}\n"
               "}\n";
        value = valueOfTok(code, "p ) { }", &settings, /*cpp*/ false);
        ASSERT_EQUALS(1, value.intvalue);
        ASSERT_EQUALS(true, value.isKnown());

        settings.standards.c = Standards::C17;
        value = valueOfTok(code, "p ) { }", &settings, /*cpp*/ false);
        ASSERT(value == ValueFlow::Value());
        settings = settingsOld;
    }

    void valueFlowSizeofForwardDeclaredEnum() {
        const char *code = "enum E; sz=sizeof(E);";
        (void)valueOfTok(code, "="); // Don't crash (#7775)
    }

    void valueFlowGlobalVar() {
        const char *code;

        code = "int x;\n"
               "void f() {\n"
               "    x = 4;\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 4));

        code = "int x;\n"
               "void f() {\n"
               "    if (x == 4) {}\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 4));

        code = "int x;\n"
               "void f() {\n"
               "    x = 42;\n"
               "    unknownFunction();\n"
               "    a = x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 42));
    }

    void valueFlowGlobalConstVar() {
        const char* code;

        code = "const int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 321));

        code = "void f(const int x = 1) {\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 2U, 1));

        code = "volatile const int x = 42;\n"
               "void f(){ int a = x; }\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 2U, 42));

        code = "static const int x = 42;\n"
               "void f(){ int a = x; }\n";
        ASSERT_EQUALS(true, testValueOfX(code, 2U, 42));
    }

    void valueFlowGlobalStaticVar() {
        const char *code;

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { x=a; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { p = &x; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { x++; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 321;\n"
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { foo(x); }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 321));

        code = "static int x = 1;\n" // compound assignment
               "void f() {\n"
               "  a = x;\n"
               "}"
               "void other() { x += b; }\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));
    }

    void valueFlowInlineAssembly() {
        const char* code = "void f() {\n"
                           "    int x = 42;\n"
                           "    asm(\"\");\n"
                           "    a = x;\n"
                           "}";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 42));
    }

    void valueFlowSameExpression() {
        const char* code;

        code = "void f(int a) {\n"
               "    bool x = a == a;\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "void f(int a) {\n"
               "    bool x = a != a;\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(int a) {\n"
               "    int x = a - a;\n"
               "    int b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "void f(float a) {\n"
               "    bool x = a == a;\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, 1));

        code = "int f(int a) {\n"
               "    int x = a ^ a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int a) {\n"
               "    int x = a /= a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 1));

        code = "int f(int a) {\n"
               "    int x = a -= a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int a) {\n"
               "    int x = a %= a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));

        code = "int f(int a) {\n"
               "    int x = a ^= a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 0));
    }

    void valueFlowUninit() {
        const char* code;
        std::list<ValueFlow::Value> values;

        code = "void f() {\n"
               "    int x;\n"
               "    switch (x) {}\n"
               "}";
        values = tokenValues(code, "x )");
        ASSERT_EQUALS(true, values.size()==1U && values.front().isUninitValue());

        code = "void f() {\n"
               "    const C *c;\n"
               "    if (c->x() == 4) {}\n"
               "}";
        values = tokenValues(code, "c .");
        ASSERT_EQUALS(true, values.size()==1U && values.front().isUninitValue());

        code = "void f() {\n"
               "    C *c;\n"
               "    if (c->x() == 4) {}\n"
               "}";
        values = tokenValues(code, "c .");
        ASSERT_EQUALS(true, values.size()==1U && values.front().isUninitValue());

        code = "void f() {\n"
               "    int **x;\n"
               "    y += 10;\n"
               "    x = dostuff(sizeof(*x)*y);\n"
               "}";
        ASSERT_EQUALS(0U, tokenValues(code, "x )").size());

        // initialization
        code = "int foo() {\n"
               "  int x;\n"
               "  *((int *)(&x)) = 12;\n"
               "  a = x + 1;\n"
               "}";
        values = tokenValues(code, "x +");
        ASSERT_EQUALS(true, values.empty());
        // ASSERT_EQUALS(1U, values.size());
        // ASSERT(values.front().isIntValue());
        // ASSERT_EQUALS(12, values.front().intvalue);

        code = "struct AB { int a; };\n" // 11767
               "void fp(void) {\n"
               "    struct AB ab;\n"
               "    *((int*)(&(ab.a))) = 1;\n"
               "    x = ab.a + 1;\n" // <- not uninitialized
               "}\n";
        values = tokenValues(code, "ab . a +");
        ASSERT_EQUALS(0, values.size());
        // ASSERT_EQUALS(1U, values.size());
        // ASSERT(values.front().isIntValue());
        // ASSERT_EQUALS(1, values.front().intvalue);

        // #8036
        code = "void foo() {\n"
               "    int x;\n"
               "    f(x=3), return x+3;\n"
               "}";
        values = tokenValues(code, "x +");
        ASSERT_EQUALS(true, values.empty());
        // ASSERT_EQUALS(1U, values.size());
        // ASSERT(values.front().isIntValue());
        // ASSERT_EQUALS(3, values.front().intvalue);

        // #8195
        code = "void foo(std::istream &is) {\n"
               "  int x;\n"
               "  if (is >> x) {\n"
               "    a = x;\n"
               "  }\n"
               "}";
        values = tokenValues(code, "x ; }");
        ASSERT_EQUALS(true, values.empty());

        // return (#8173)
        code = "int repeat() {\n"
               "  const char *n;\n"
               "  return((n=42) && *n == 'A');\n"
               "}";
        values = tokenValues(code, "n ==");
        values.remove_if(&isNotUninitValue);
        ASSERT_EQUALS(true, values.empty());

        // #8233
        code = "void foo() {\n"
               "  int x;\n"
               "  int y = 1;\n"
               "  if (y>1)\n"
               "    x = 1;\n"
               "  else\n"
               "    x = 1;\n"
               "  if (x>1) {}\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 8U, 1));

        // #8348 - noreturn else
        code = "int test_input_int(int a, int b) {\n"
               "    int x;\n"
               "    if (a == 1)\n"
               "        x = b;\n"
               "    else\n"
               "        abort();\n"
               "    a = x + 1;\n"
               "}\n";
        values = tokenValues(code, "x +");
        values.remove_if(&isNotUninitValue);
        ASSERT_EQUALS(true, values.empty());

        // #8494 - overloaded operator &
        code = "void f() {\n"
               "    int x;\n"
               "    a & x;\n"
               "}";
        values = tokenValues(code, "x ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  if (e)\n"
               "    goto;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  if (e)\n"
               "    return;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  if (e)\n"
               "    exit();\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.empty());

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  else if (e)\n"
               "   c = 0;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        TODO_ASSERT_EQUALS(true, false, values.size() == 2);
        // ASSERT_EQUALS(true, values.front().isUninitValue() || values.back().isUninitValue());
        // ASSERT_EQUALS(true, values.front().isPossible() || values.back().isPossible());
        // ASSERT_EQUALS(true, values.front().intvalue == 0 || values.back().intvalue == 0);

        code = "void b(bool d, bool e) {\n"
               "  int c;\n"
               "  if (d)\n"
               "    c = 0;\n"
               "  else if (!d)\n"
               "   c = 0;\n"
               "  c++;\n"
               "}\n";
        values = tokenValues(code, "c ++ ; }");
        ASSERT_EQUALS(true, values.size() == 1);
        // TODO: Value should be known
        ASSERT_EQUALS(true, values.back().isPossible());
        ASSERT_EQUALS(true, values.back().intvalue == 0);

        code = "void f() {\n" // sqlite
               "  int szHdr;\n"
               "  idx = (A<0x80) ? (szHdr = 0) : dostuff(A, (int *)&(szHdr));\n"
               "  d = szHdr;\n" // szHdr can be 0.
               "}";
        values = tokenValues(code, "szHdr ; }");
        TODO_ASSERT_EQUALS(1, 0, values.size());
        if (values.size() == 1) {
            ASSERT_EQUALS(false, values.front().isUninitValue());
        }

        code = "void f () {\n"
               "  int szHdr;\n"
               "  idx = ((aKey<0x80) ? ((szHdr)=aKey), 1 : sqlite3GetVarint32(&(szHdr)));\n"
               "  d = szHdr;\n"
               "}";
        values = tokenValues(code, "szHdr ; }");
        ASSERT_EQUALS(0, values.size());

        // #9933
        code = "struct MyStruct { size_t value; }\n"
               "\n"
               "void foo() {\n"
               "    MyStruct x;\n"
               "    fread(((char *)&x) + 0, sizeof(x), f);\n"
               "    if (x.value < 432) {}\n"
               "}";
        values = tokenValues(code, "x . value");
        ASSERT_EQUALS(0, values.size());

        // #10166
        code = "int f(bool b) {\n"
               "    int x;\n"
               "    do {\n"
               "      if (b) {\n"
               "        x = 0;\n"
               "        break;\n"
               "      }\n"
               "    } while (true);\n"
               "    return x;\n"
               "}\n";
        values = tokenValues(code, "x ; }", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "int f(bool b) {\n"
               "    int x;\n"
               "    while (true) {\n"
               "      if (b) {\n"
               "        x = 0;\n"
               "        break;\n"
               "      }\n"
               "    }\n"
               "    return x;\n"
               "}\n";
        values = tokenValues(code, "x ; }", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "int f(bool b) {\n"
               "    int x;\n"
               "    for(;;) {\n"
               "      if (b) {\n"
               "        x = 0;\n"
               "        break;\n"
               "      }\n"
               "    }\n"
               "    return x;\n"
               "}\n";
        values = tokenValues(code, "x ; }", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "int f(bool b) {\n"
               "    int x;\n"
               "    switch (b) {\n"
               "      case 1: {\n"
               "        ret = 0;\n"
               "        break;\n"
               "      }\n"
               "    }\n"
               "    return x;\n"
               "}\n";
        values = tokenValues(code, "x ; }", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(1, values.size());
        ASSERT_EQUALS(true, values.front().isUninitValue());

        code = "void f(int x) {\n"
               "    int i;\n"
               "    if (x > 0) {\n"
               "        int y = -ENOMEM;\n" // assume constant ENOMEM is nonzero since it's negated
               "        if (y != 0) return;\n"
               "        i++;\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "i ++", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        // #11688
        code = "void f() {\n"
               "    int n;\n"
               "    for (int i = 0; i < 4; i = n)\n" // <- n is initialized in the loop body
               "        n = 10;\n"
               "}";
        values = tokenValues(code, "n )", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        // #11774 - function call to init data
        code = "struct id_struct { int id; };\n"
               "int init(const id_struct **id);\n"
               "void fp() {\n"
               "  const id_struct *id_st;\n"
               "  init(&id_st);\n"
               "  if (id_st->id > 0) {}\n"
               "}\n";
        values = tokenValues(code, ". id", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        // #11777 - false || ...
        code = "bool init(int *p);\n"
               "\n"
               "void uninitvar_FP9() {\n"
               "  int x;\n"
               "  if (false || init(&x)) {}\n"
               "  int b = x+1;\n"
               "}";
        values = tokenValues(code, "x + 1", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "void g() {\n"
               "  int y;\n"
               "  int *q = 1 ? &y : 0;\n"
               "}\n";
        values = tokenValues(code, "y :", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(1, values.size());
        ASSERT_EQUALS(true, values.front().isUninitValue());
        values = tokenValues(code, "& y :", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(1, values.size());
        ASSERT_EQUALS(true, values.front().isUninitValue());

        // #12012 - function init variable
        code = "void init(uintptr_t p);\n"
               "void fp() {\n"
               "  int x;\n"
               "  init((uintptr_t)&x);\n"
               "  if (x > 0) {}\n"
               "}\n";
        values = tokenValues(code, "x >", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        // #12031
        code = "bool g(int *p);\n"
               "bool h();\n"
               "void f(bool b, int y) {\n"
               "    int x;\n"
               "    if (b && y > 0) {\n"
               "        b = g(&x);\n"
               "    }\n"
               "    while (b && y > 0) {\n"
               "        if (x < 0) {}\n"
               "        break;\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "x <", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "bool do_something(int *p);\n"
               "int getY();\n"
               "bool bar();\n"
               "void foo() {\n"
               "    bool flag{true};\n"
               "    int x;\n"
               "    int y = getY();\n"
               "    if (flag == true) {\n"
               "        flag = bar();\n"
               "    }\n"
               "    if ((flag == true) && y > 0) {\n"
               "        flag = do_something(&x);\n"
               "    }\n"
               "    for (int i = 0; (flag == true) && i < y; i++) {\n"
               "        if (x < 0) {}\n"
               "    }\n"
               "}\n";
        values = tokenValues(code, "x <", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "void getX(int *p);\n"
               "bool do_something();\n"
               "void foo() {\n"
               "  int x;\n"
               "  bool flag;\n"
               "  bool success;\n"
               "  success = do_something();\n"
               "  flag = success;\n"
               "  if (success == true) {\n"
               "    getX(&x);\n"
               "  }\n"
               "  for (int i = 0; (flag == true) && (i < x); ++i) {}\n"
               "}\n";
        values = tokenValues(code, "x ) ; ++ i", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());

        code = "void g(bool *result, size_t *buflen) {\n" // #12091
               "    if (*result && *buflen >= 5) {}\n" // <- *buflen might not be initialized
               "}\n"
               "void f() {\n"
               "    size_t bytesCopied;\n"
               "    bool copied_all = true;\n"
               "    g(&copied_all, &bytesCopied);\n"
               "}";
        values = tokenValues(code, "buflen >=", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(1, values.size());

        code = "void foo() {\n"
               "  int counter = 0;\n"
               "  int x;\n"
               "  for (int i = 0; i < 5; i++) {\n"
               "    if (i % 2 == 0) {\n"
               "      x = i;\n"
               "      counter++;\n"
               "    }\n"
               "  }\n"
               "  for (int i = 0; i < counter; i++) {\n"
               "      if(x > 5) {}\n"
               "  }\n"
               "}\n";
        values = tokenValues(code, "x > 5", ValueFlow::Value::ValueType::UNINIT);
        ASSERT_EQUALS(0, values.size());
    }

    void valueFlowConditionExpressions() {
        const char* code;
        std::list<ValueFlow::Value> values;

        // opposite condition
        code = "void f(int i, int j) {\n"
               "    if (i == j) return;\n"
               "    if(i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(int i, int j) {\n"
               "    if (i == j) return;\n"
               "    i++;\n"
               "    if (i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(int i, int j, bool a) {\n"
               "    if (a) {\n"
               "        if (i == j) return;\n"
               "    }\n"
               "    if (i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);
        ASSERT_EQUALS(false, valueOfTok(code, "!=").isKnown());

        code = "void f(int i, int j, bool a) {\n"
               "    if (i != j) {}\n"
               "    if (i == j) return;\n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "!=").intvalue == 1);

        // same expression
        code = "void f(int i, int j) {\n"
               "    if (i != j) return;\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(int i, int j) {\n"
               "    if (i != j) return;\n"
               "    i++;\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 0));

        code = "void f(int i, int j, bool a) {\n"
               "    if (a) {\n"
               "        if (i != j) return;\n"
               "    }\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 0));

        code = "void f(int i, int j, bool a) {\n"
               "    bool x = (i != j);\n"
               "    bool b = x;\n"
               "    if (i != j) return;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, 0));

        code = "void f(int i, int j, bool b) {\n"
               "    if (i == j) { if(b) return; }\n"
               "    if(i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(bool b, int i, int j) {\n"
               "    if (b || i == j) return;\n"
               "    if(i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(bool b, int i, int j) {\n"
               "    if (b && i == j) return;\n"
               "    if(i != j) {}\n"
               "}\n";
        ASSERT_EQUALS(true, removeImpossible(tokenValues(code, "!=")).empty());

        code = "void f(int i, int j) {\n"
               "    if (i == j) {\n"
               "        if (i != j) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 0);

        code = "void f(int i, int j) {\n"
               "    if (i == j) {} else {\n"
               "        if (i != j) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(bool b, int i, int j) {\n"
               "    if (b && i == j) {\n"
               "        if (i != j) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 0);

        code = "void f(bool b, int i, int j) {\n"
               "    if (b || i == j) {\n"
               "        if (i != j) {}\n"
               "    }\n"
               "}\n";
        values = removeImpossible(tokenValues(code, "!="));
        ASSERT_EQUALS(1, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(true, values.front().isIntValue());
        ASSERT_EQUALS(true, values.front().isPossible());

        code = "void f(bool b, int i, int j) {\n"
               "    if (b || i == j) {} else {\n"
               "        if (i != j) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "!=").intvalue == 1);

        code = "void f(bool b, int i, int j) {\n"
               "    if (b && i == j) {} else {\n"
               "        if (i != j) {}\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, removeImpossible(tokenValues(code, "!=")).empty());

        code = "void foo()\n" // #8924
               "{\n"
               "    if ( this->FileIndex >= 0 )\n"
               "        return;\n"
               "\n"
               "    this->FileIndex = 1 ;\n"
               "    if ( this->FileIndex < 0 ) {}\n"
               "}";
        ASSERT_EQUALS(false, valueOfTok(code, "<").intvalue == 1);

        code = "int f(int p) {\n"
               "    int v = 0;\n"
               "    for (int i = 0; i < 1; ++i) {\n"
               "        if (p == 0)\n"
               "            v = 1;\n"
               "        if (v == 1)\n"
               "            break;\n"
               "    }\n"
               "    int x = v;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 1));

        code = "void f() {\n"
               "    const int size = arrayInfo.num(0);\n"
               "    if (size <= 0)\n"
               "        return;\n"
               "    for (;;)\n"
               "        if (size > 0) {}\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "> 0").isKnown());
        ASSERT_EQUALS(true, valueOfTok(code, "> 0").intvalue == 1);

        //  FP #10110
        code = "enum FENUMS { NONE = 0, CB = 8 };\n"
               "bool calc(int x) {\n"
               "    if (!x) {\n"
               "        return false;\n"
               "    }\n"
               "\n"
               "    if (x & CB) {\n"
               "        return true;\n"
               "    }\n"
               "    return false;\n"
               "}\n";
        ASSERT_EQUALS(false, valueOfTok(code, "& CB").isKnown());
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 7U, 0));

        code = "enum FENUMS { NONE = 0, CB = 8 };\n"
               "bool calc(int x) {\n"
               "    if (x) {\n"
               "        return false;\n"
               "    }\n"
               "\n"
               "    if ((!x) & CB) {\n"
               "        return true;\n"
               "    }\n"
               "    return false;\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "& CB").isKnown());
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 0));

        code = "enum FENUMS { NONE = 0, CB = 8 };\n"
               "bool calc(int x) {\n"
               "    if (!!x) {\n"
               "        return false;\n"
               "    }\n"
               "\n"
               "    if (x & CB) {\n"
               "        return true;\n"
               "    }\n"
               "    return false;\n"
               "}\n";
        ASSERT_EQUALS(true, valueOfTok(code, "& CB").isKnown());
        ASSERT_EQUALS(true, testValueOfXKnown(code, 7U, 0));

        code = "bool calc(bool x) {\n"
               "    if (!x) {\n"
               "        return false;\n"
               "    }\n"
               "\n"
               "    if (x) {\n"
               "        return true;\n"
               "    }\n"
               "    return false;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 1));

        code = "bool calc(bool x) {\n"
               "    if (x) {\n"
               "        return false;\n"
               "    }\n"
               "\n"
               "    if (!x) {\n"
               "        return true;\n"
               "    }\n"
               "    return false;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 0));

        code = "bool f(bool b1, bool b2) {\n"
               "    if (b1 && b2)\n"
               "        return;\n"
               "    int x = b1 && b2;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));

        code = "void f() {\n"
               "    if (int* x = {}) {}\n"
               "}\n";
        ASSERT_EQUALS(true, testKnownValueOfTok(code, "=", 0));
    }

    static std::string isPossibleContainerSizeValue(std::list<ValueFlow::Value> values,
                                                    MathLib::bigint i,
                                                    bool unique = true) {
        values.remove_if(std::mem_fn(&ValueFlow::Value::isSymbolicValue));
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
        if (!unique)
            values.remove_if(&isNotPossible);
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isPossible())
            return "Possible";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    static std::string isImpossibleContainerSizeValue(std::list<ValueFlow::Value> values,
                                                      MathLib::bigint i,
                                                      bool unique = true) {
        values.remove_if(std::mem_fn(&ValueFlow::Value::isSymbolicValue));
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
        if (!unique)
            values.remove_if(&isNotImpossible);
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isImpossible())
            return "Impossible";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    static std::string isInconclusiveContainerSizeValue(std::list<ValueFlow::Value> values,
                                                        MathLib::bigint i,
                                                        bool unique = true) {
        values.remove_if(std::mem_fn(&ValueFlow::Value::isSymbolicValue));
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
        if (!unique)
            values.remove_if(&isNotInconclusive);
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isInconclusive())
            return "Inconclusive";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    static std::string isKnownContainerSizeValue(std::list<ValueFlow::Value> values, MathLib::bigint i, bool unique = true) {
        values.remove_if(std::mem_fn(&ValueFlow::Value::isSymbolicValue));
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
        if (!unique)
            values.remove_if(&isNotKnown);
        if (values.size() != 1)
            return "values.size():" + std::to_string(values.size());
        if (!values.front().isContainerSizeValue())
            return "ContainerSizeValue";
        if (!values.front().isKnown())
            return "Known";
        if (values.front().intvalue != i)
            return "intvalue:" + std::to_string(values.front().intvalue);
        return "";
    }

    void valueFlowContainerSize() {
        const char *code;

        // condition
        code = "void f(const std::list<int> &ints) {\n"
               "  if (!static_cast<bool>(ints.empty()))\n"
               "    ints.front();\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        // valueFlowContainerReverse
        code = "void f(const std::list<int> &ints) {\n"
               "  ints.front();\n" // <- container can be empty
               "  if (ints.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  ints.front();\n" // <- container can be empty
               "  if (ints.size()==0) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(std::list<int> ints) {\n"
               "  ints.front();\n" // <- no container size
               "  ints.pop_back();\n"
               "  if (ints.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 1));

        code = "void f(std::vector<int> v) {\n"
               "  v[10] = 0;\n" // <- container size can be 10
               "  if (v.size() == 10) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "v ["), 10));

        code = "void f(std::vector<std::string> params) {\n"
               "  switch(x) {\n"
               "  case CMD_RESPONSE:\n"
               "    if(y) { break; }\n"
               "    params[2];\n" // <- container use
               "    break;\n"
               "  case CMD_DELETE:\n"
               "    if (params.size() < 2) { }\n" // <- condition
               "    break;\n"
               "  }\n"
               "}";
        ASSERT(tokenValues(code, "params [ 2 ]").empty());

        // valueFlowAfterCondition
        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(v.empty()) {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "v . front"), 0));

        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(std::empty(v)) {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "v . front"), 0));

        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(!v.empty()) {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "v . front"), 0));

        code = "void f(const std::vector<std::string>& v) {\n"
               "    if(!v.empty() && v[0] != \"\") {\n"
               "        v.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "v . front"), 0));

        // valueFlowContainerForward
        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) {}\n"
               "  ints.front();\n" // <- container can be empty
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) { continue; }\n"
               "  ints.front();\n" // <- no container size
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) { ints.push_back(0); }\n"
               "  ints.front();\n" // <- container is not empty
               "}";
        ASSERT(tokenValues(code, "ints . front").empty());

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty()) {\n"
               "    ints.front();\n" // <- container is empty
               "  }\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() == 3) {\n"
               "    ints.front();\n" // <- container size is 3
               "  }\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 3));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() <= 3) {\n"
               "    ints.front();\n" // <- container size is 3
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 3, false));
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 4, false));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() >= 3) {\n"
               "    ints.front();\n" // <- container size is 3
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 3, false));
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 2, false));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() < 3) {\n"
               "    ints.front();\n" // <- container size is 2
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 2, false));
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 3, false));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.size() > 3) {\n"
               "    ints.front();\n" // <- container size is 4
               "  }\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "ints . front"), 4, false));
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints . front"), 3, false));

        code = "void f(const std::list<int> &ints) {\n"
               "  if (ints.empty() == false) {\n"
               "    ints.front();\n" // <- container is not empty
               "  }\n"
               "}";
        ASSERT(tokenValues(code, "ints . front").empty());

        code = "void f(const std::vector<int> &v) {\n"
               "  if (v.empty()) {}\n"
               "  if (!v.empty() && v[10]==0) {}\n" // <- no container size for 'v[10]'
               "}";
        ASSERT(removeImpossible(tokenValues(code, "v [")).empty());

        code = "void f() {\n"
               "  std::list<int> ints;\n"  // No value => ints is empty
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 0));

        code = "void f() {\n"
               "  std::array<int,10> ints;\n" // Array size is 10
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("values.size():2", isKnownContainerSizeValue(tokenValues(code, "ints . front"), 10)); // uninit value

        code = "void f() {\n"
               "  std::string s;\n"
               "  cin >> s;\n"
               "  s[0];\n"
               "}";
        ASSERT(tokenValues(code, "s [").empty());

        code = "void f() {\n"
               "  std::string s = \"abc\";\n" // size of s is 3
               "  s.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s . size"), 3));

        code = "void f(const char* p) {\n"
               "  if (p == nullptr) return;\n"
               "  std::string s { p };\n" // size of s is unknown
               "  s.front();\n"
               "}";
        ASSERT(removeSymbolicTok(tokenValues(code, "s . front")).empty());

        code = "void f() {\n"
               "  std::string s = { 'a', 'b', 'c' };\n" // size of s is 3
               "  s.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s . size"), 3));

        code = "void f() {\n"
               "  std::string s=\"abc\";\n" // size of s is 3
               "  s += unknown;\n"
               "  s.size();\n"
               "}";
        ASSERT(tokenValues(code, "s . size").empty());

        code = "void f() {\n"
               "  std::string s=\"abc\";\n" // size of s is 3
               "  s += \"def\";\n" // size of s => 6
               "  s.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s . size"), 6));

        code = "void f(std::string s) {\n"
               "    if (s == \"hello\")\n"
               "        s[40] = c;\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f(std::string s) {\n"
               "    s[40] = c;\n"
               "    if (s == \"hello\") {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f(std::string s) {\n"
               "    if (s != \"hello\") {}\n"
               "    s[40] = c;\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "s ["), 5));

        code = "void f(std::string s) {\n"
               "    if (s != \"hello\")\n"
               "        s[40] = c;\n"
               "}";
        ASSERT(!isImpossibleContainerSizeValue(tokenValues(code, "s ["), 5).empty());

        code = "void f() {\n"
               "    static std::string s;\n"
               "    if (s.size() == 0)\n"
               "        s = x;\n"
               "}";
        ASSERT(tokenValues(code, "s . size").empty());

        code = "void f() {\n"
               "    const uint8_t data[] = { 1, 2, 3 };\n"
               "    std::vector<uint8_t> v{ data, data + sizeof(data) };\n"
               "    v.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "v . size"), 3, false));

        // valueFlowContainerForward, loop
        code = "void f() {\n"
               "    std::stack<Token *> links;\n"
               "    while (!links.empty() || indentlevel)\n"
               "        links.push(tok);\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "links . empty"), 0));

        // valueFlowContainerForward, function call
        code = "void f() {\n"
               "  std::list<int> x;\n"
               "  f(x);\n"
               "  x.front();\n" // <- unknown container size
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void f() {\n" // #8689
               "  std::list<int> x;\n"
               "  f<ns::a>(x);\n"
               "  x.front();\n" // <- unknown container size
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>*);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>* const);\n" // #9434
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(const std::list<int>&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(int&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x[0]);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(int&);\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x.back());\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>&) {}\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>& y) { y.push_back(1); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void g(std::list<int>*) {}\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void g(std::list<int>* y) { y->push_back(1); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(&x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void h(std::list<int>&);\n"
               "void g(std::list<int>& y) { h(y); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void h(const std::list<int>&);\n"
               "void g(std::list<int>& y) { h(y); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "x . front"), 0));

        code = "void h(const std::list<int>&);\n"
               "void g(std::list<int>& y) { h(y); y.push_back(1); }\n"
               "void f() {\n"
               "  std::list<int> x;\n"
               "  g(x);\n"
               "  x.front();\n"
               "}";
        ASSERT(tokenValues(code, "x . front").empty());

        code = "void f(std::vector<int> ints) {\n" // #8697
               "  if (ints.empty())\n"
               "    abort() << 123;\n"
               "  ints[0] = 0;\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "ints ["), 0));

        code = "struct A {\n"  // forward, nested function call, #9424
               "    double getMessage( std::vector<unsigned char> *message );\n"
               "};\n"
               "\n"
               "struct B {\n"
               "    A *a;\n"
               "    double getMessage( std::vector<unsigned char> *message ) { return a->getMessage( message ); }\n"
               "};\n"
               "\n"
               "void foo(B *ptr) {\n"
               "    std::vector<unsigned char> v;\n"
               "    ptr->getMessage (&v);\n"
               "    if (v.size () > 0) {}\n" // <- v has unknown size!
               "}";
        ASSERT_EQUALS(0U, tokenValues(code, "v . size ( )").size());

        // if
        code = "bool f(std::vector<int>&) {\n" // #9532
               "  return false;\n"
               "}\n"
               "int g() {\n"
               "    std::vector<int> v;\n"
               "    if (f(v) || v.empty())\n"
               "        return 0;\n"
               "    return v[0];\n"
               "}\n";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "v [ 0 ]"), 0));

        // container size => yields
        code = "void f() {\n"
               "  std::string s = \"abcd\";\n"
               "  s.size();\n"
               "}";
        ASSERT_EQUALS(4, tokenValues(code, "( ) ;").front().intvalue);

        code = "void f() {\n"
               "  std::string s;\n"
               "  s.empty();\n"
               "}";
        ASSERT_EQUALS(1, tokenValues(code, "( ) ;").front().intvalue);

        // Calculations
        code = "void f() {\n"
               "  std::string s = \"abcd\";\n"
               "  x = s + s;\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "+"), 8));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.clear();\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.resize(3);\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 3));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.resize(3);\n"
               "  ints.push_back(3);\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 4));

        code = "void f(const std::vector<int> &ints) {\n"
               "  ints.resize(3);\n"
               "  ints.pop_back();\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 2));

        code = "int f(bool b) {\n"
               "    std::map<int, int> m;\n"
               "    if (b)\n"
               "        m[0] = 1;\n"
               "    return m.at(0);\n"
               "}\n";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "m . at", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "struct Base {\n"
               "    virtual bool GetString(std::string &) const { return false; }\n"
               "};\n"
               "int f() {\n"
               "    std::string str;\n"
               "    Base *b = GetClass();\n"
               "    if (!b->GetString(str)) {\n"
               "        return -2;\n"
               "    }\n"
               "    else {\n"
               "        return str.front();\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(0U, tokenValues(code, "str . front").size());

        code = "void f() {\n"
               "  std::vector<int> ints{};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f() {\n"
               "  std::vector<int> ints{};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f() {\n"
               "  std::vector<int> ints{1};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 1));

        code = "void f() {\n"
               "  std::vector<int> ints{1};\n"
               "  std::vector<int> ints2{ints.begin(), ints.end()};\n"
               "  ints2.front();\n"
               "}";
        ASSERT_EQUALS(
            "", isKnownContainerSizeValue(tokenValues(code, "ints2 . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 1));

        code = "void f() {\n"
               "  std::vector<int> ints = {};\n"
               "  ints.front();\n"
               "}";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "ints . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\")\n"
               "        bool x = str.empty();\n"
               "}\n";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "str . empty", ValueFlow::Value::ValueType::CONTAINER_SIZE), 3));

        code = "int f() {\n"
               "    std::array<int, 10> a = {};\n"
               "    return a.front();\n"
               "}\n";
        ASSERT_EQUALS("",
                      isKnownContainerSizeValue(tokenValues(code, "a . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 10));

        code = "int f(const std::vector<int>& x) {\n"
               "    if (!x.empty() && x[0] == 0)\n"
               "        return 2;\n"
               "    return x.front();\n"
               "}\n";
        ASSERT_EQUALS("",
                      isPossibleContainerSizeValue(tokenValues(code, "x . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "int f(const std::vector<int>& x) {\n"
               "    if (!(x.empty() || x[0] != 0))\n"
               "        return 2;\n"
               "    return x.front();\n"
               "}\n";
        ASSERT_EQUALS("",
                      isPossibleContainerSizeValue(tokenValues(code, "x . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "int f() {\n"
               "    const size_t len = 6;\n"
               "    std::vector<char> v;\n"
               "    v.resize(1 + len);\n"
               "    return v.front();\n"
               "}\n";
        ASSERT_EQUALS(
            "",
            isKnownContainerSizeValue(tokenValues(code, "v . front", ValueFlow::Value::ValueType::CONTAINER_SIZE), 7));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str == \"\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str != \"\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str == \"321\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));

        code = "void f(std::string str) {\n"
               "    if (str == \"123\") {\n"
               "        bool x = (str != \"321\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 0));

        code = "void f(std::string str) {\n"
               "    if (str.size() == 1) {\n"
               "        bool x = (str == \"123\");\n"
               "        bool a = x;\n"
               "     }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "bool f(std::string s) {\n"
               "    if (!s.empty()) {\n"
               "        bool x = s == \"0\";\n"
               "        return x;\n"
               "    }\n"
               "    return false;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 4U, 0));

        code = "void f() {\n"
               "    std::vector<int> v;\n"
               "    int x = v.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f() {\n"
               "    std::vector<int> v;\n"
               "    int x = v.empty();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "void f() {\n"
               "    std::vector<int> v;\n"
               "    int x = std::size(v);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f() {\n"
               "    std::vector<int> v;\n"
               "    int x = std::empty(v);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "bool f() {\n"
               "    std::list<int> x1;\n"
               "    std::list<int> x2;\n"
               "    for (int i = 0; i < 10; ++i) {\n"
               "        std::list<int>& x = (i < 5) ? x1 : x2;\n"
               "        x.push_back(i);\n"
               "    }\n"
               "    return x1.empty() || x2.empty();\n"
               "}\n";
        ASSERT_EQUALS("", isInconclusiveContainerSizeValue(tokenValues(code, "x1 . empty", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));
        ASSERT_EQUALS("", isInconclusiveContainerSizeValue(tokenValues(code, "x2 . empty", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "std::vector<int> g();\n"
               "int f(bool b) {\n"
               "    std::set<int> a;\n"
               "    std::vector<int> c = g();\n"
               "    a.insert(c.begin(), c.end());\n"
               "    return a.size();\n"
               "}\n";
        ASSERT_EQUALS(true, tokenValues(code, "a . size", ValueFlow::Value::ValueType::CONTAINER_SIZE).empty());

        code = "void f() {\n" // #14060
               "    std::stack<std::pair<int, int>> s;\n"
               "    s.emplace(0, 0);\n"
               "    s.pop();\n"
               "    bool x = s.empty();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, 1));

        code = "std::vector<int> g();\n"
               "std::vector<int> f() {\n"
               "    std::vector<int> v = g();\n"
               "    if (!v.empty()) {\n"
               "        if (v[0] != 0)\n"
               "            v.clear();\n"
               "    }\n"
               "    if (!v.empty() && v[0] != 0) {}\n"
               "    return v;\n"
               "}\n";
        ASSERT_EQUALS(
            true,
            removeImpossible(tokenValues(code, "v [ 0 ] != 0 ) { }", ValueFlow::Value::ValueType::CONTAINER_SIZE)).empty());

        code = "std::vector<int> f() {\n"
               "    std::vector<int> v;\n"
               "    v.reserve(1);\n"
               "    v[1] = 42;\n"
               "    return v;\n"
               "}\n";
        ASSERT_EQUALS(
            "", isKnownContainerSizeValue(tokenValues(code, "v [", ValueFlow::Value::ValueType::CONTAINER_SIZE), 0));

        code = "void f() {\n"
               "  std::vector<int> v(3);\n"
               "  v.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "v . size"), 3));

        code = "void f() {\n"
               "  std::vector<int> v({ 1, 2, 3 });\n"
               "  v.size();\n"
               "}";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "v . size"), 3));

        code = "int f() {\n"
               "    std::vector<std::vector<int>> v;\n"
               "    auto it = v.begin();\n"
               "    auto x = it->size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));

        code = "std::vector<int> g();\n" // #11417
               "int f() {\n"
               "    std::vector<int> v{ g() };\n"
               "    auto x = v.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 1));

        code = "std::vector<int> g();\n"
               "int f() {\n"
               "    std::vector<std::vector<int>> v{ g() };\n"
               "    auto x = v.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        // #11548
        code = "void f(const std::string& a, const std::string& b) {\n"
               "  if (a.empty() && b.empty()) {}\n"
               "  else if (a.empty() == false && b.empty() == false) {}\n"
               "}\n";
        ASSERT(!isImpossibleContainerSizeValue(tokenValues(code, "a . empty ( ) == false"), 0).empty());

        code = "bool g(std::vector<int>& v) {\n"
               "    v.push_back(1);\n"
               "    int x = v.empty();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "std::vector<int> f() { return std::vector<int>(); }";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "( ) ;"), 0));

        code = "std::vector<int> f() { return std::vector<int>{}; }";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "{ } ;"), 0));

        code = "std::vector<int> f() { return {}; }";
        ASSERT_EQUALS("", isKnownContainerSizeValue(tokenValues(code, "{ } ;"), 0));

        code = "int f() { auto a = std::array<int, 2>{}; return a[1]; }";
        ASSERT_EQUALS("values.size():0", isKnownContainerSizeValue(tokenValues(code, "a ["), 0));

        code = "void g(std::vector<int>* w) {\n"
               "  std::vector<int> &r = *w;\n"
               "  r.push_back(0);\n"
               "}\n"
               "int f() {\n"
               "  std::vector<int> v;\n"
               "  g(&v);\n"
               "  return v[0];\n"
               "}\n";
        ASSERT(!isKnownContainerSizeValue(tokenValues(code, "v ["), 0).empty());
        ASSERT(!isPossibleContainerSizeValue(tokenValues(code, "v ["), 0).empty());

        code = "template<typename T>\n" // #12052
               "std::vector<T> g(T);\n"
               "int f() {\n"
               "    const std::vector<int> v{ g(3) };\n"
               "    auto x = v.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, 1));

        code = "template<typename T>\n"
               "std::vector<T> g(const std::stack<T>& s);\n"
               "int f() {\n"
               "    std::stack<int> s{};\n"
               "    s.push(42);\n"
               "    s.push(43);\n"
               "    const std::vector<int> r{ g(s) };\n"
               "    auto x = r.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 9U, 1));

        code = "int f() {\n" // #12987
               "    std::string s{\"0\"};\n"
               "    auto x = s.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "int f() {\n"
               "    std::string s;\n"
               "    s.append(\"0\");\n"
               "    auto x = s.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "int f() {\n"
               "    std::string s;\n"
               "    s = std::string(\"0\");\n"
               "    auto x = s.size();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "std::string f() {\n" // #12993
               "    std::string a[1];\n"
               "    a->clear();\n"
               "    return a[0];\n"
               "}\n";
        ASSERT(!isKnownContainerSizeValue(tokenValues(code, "a [ 0"), 0).empty());

        code = "void f(const std::string& a) {\n" // #12994
               "    std::string b = a + \"123\";\n"
               "    if (b.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "b ."), 2));

        code = "void f(const std::string& a) {\n"
               "    std::string b = \"123\" + a;\n"
               "    if (b.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "b ."), 2));

        code = "void f(const std::string& a, const std::string& b) {\n"
               "    std::string c = a + b + \"123\";\n"
               "    if (c.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "c ."), 2));

        code = "void f(const std::string& a) {\n"
               "    std::string b = a + \"123\" + \"456\";\n"
               "    if (b.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "b ."), 5));

        code = "void f(const std::string& a) {\n"
               "    std::string b = \"123\" + a + \"456\";\n"
               "    if (b.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "b ."), 5));

        code = "void f(const std::string& a, const std::string& b) {\n"
               "    std::string c = \"123\" + a + b;\n"
               "    if (c.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isImpossibleContainerSizeValue(tokenValues(code, "c ."), 2));

        code = "void f(const std::string& a) {\n"
               "    std::string s;\n"
               "    s.append(a);\n"
               "    if (s.empty()) {}\n"
               "}";
        ASSERT_EQUALS("", isPossibleContainerSizeValue(tokenValues(code, "s . empty"), 0));

        code = "int f(const std::string& str) {\n"
               "    std::istringstream iss(str);\n"
               "    std::vector<std::string> v{ std::istream_iterator<std::string>(iss), {} };\n"
               "    auto x = v.size();\n"
               "    return x;\n"
               "}";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 2));

        code = "auto f() {\n" // #13450
               "    auto v = std::vector<std::vector<S*>>(3, std::vector<S*>());\n"
               "    return v[2];\n"
               "}";
        ASSERT(isKnownContainerSizeValue(tokenValues(code, "v ["), 3).empty());

        code = "auto f() {\n" // #13654
               "    std::array<uint8_t, 6 * sizeof(uint16_t)> a{};\n"
               "    return a[0];\n"
               "}";
        ASSERT(!isKnownContainerSizeValue(tokenValues(code, "a ["), 6).empty());

        code = "void f(const char a[]) {\n" // #14518
               "    std::string s(a);\n"
               "    if (s.empty()) {}\n"
               "}";
        ASSERT(!isKnownContainerSizeValue(tokenValues(code, "s ."), 0).empty());
    }

    void valueFlowContainerElement()
    {
        const char* code;

        code = "int f() {\n"
               "    std::vector<int> v = {1, 2, 3, 4};\n"
               "    int x = v[1];\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 2));

        code = "int f() {\n"
               "    std::vector<int> v = {1, 2, 3, 4};\n"
               "    int x = v.at(1);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 2));

        code = "int f() {\n"
               "    std::string s = \"hello\";\n"
               "    int x = s[1];\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 'e'));
    }

    void valueFlowDynamicBufferSize() {
        const char *code;

        const Settings settingsOld = settings; // TODO: get rid of this
        settings = settingsBuilder(settings).library("posix.cfg").library("bsd.cfg").build();

        code = "void* f() {\n"
               "  void* x = malloc(10);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 10,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  void* x = calloc(4, 5);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, 20,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  const char* y = \"abcd\";\n"
               "  const char* x = strdup(y);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 5,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  void* y = malloc(10);\n"
               "  void* x = realloc(y, 20);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 20,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "void* f() {\n"
               "  void* y = calloc(10, 4);\n"
               "  void* x = reallocarray(y, 20, 5);\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 100,  ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "struct A {};\n" // #14305
               "void* f() {\n"
               "  A* x = new A();\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 1, ValueFlow::Value::ValueType::BUFFER_SIZE));

        code = "struct A {};\n"
               "void* f() {\n"
               "  void* x = new A;\n"
               "  return x;\n"
               "}";
        {
            auto values = tokenValues(code, "x ; }");
            ASSERT_EQUALS(1, values.size());
            ASSERT(values.front().isSymbolicValue());
            // TODO: add BUFFER_SIZE value = 1
        }

        code = "struct B { int32_t i; };\n"
               "void* f() {\n"
               "  B* x = new B();\n"
               "  return x;\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 4, ValueFlow::Value::ValueType::BUFFER_SIZE));

        settings = settingsOld;
    }

    void valueFlowSafeFunctionParameterValues() {
        const char *code;
        std::list<ValueFlow::Value> values;
        /*const*/ Settings s = settingsBuilder().library("std.cfg").build();
        s.safeChecks.classes = s.safeChecks.externalFunctions = s.safeChecks.internalFunctions = true;

        code = "short f(short x) {\n"
               "  return x + 0;\n"
               "}";
        values = removeSymbolicTok(tokenValues(code, "+", &s));
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(-0x8000, values.front().intvalue);
        ASSERT_EQUALS(0x7fff, values.back().intvalue);

        code = "short f(std::string x) {\n"
               "  return x[10];\n"
               "}";
        values = tokenValues(code, "x [", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(1000000, values.back().intvalue);

        code = "int f(float x) {\n"
               "  return x;\n"
               "}";
        values = tokenValues(code, "x ;", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT(values.front().floatValue < -1E20);
        ASSERT(values.back().floatValue > 1E20);

        code = "short f(__cppcheck_low__(0) __cppcheck_high__(100) short x) {\n"
               "  return x + 0;\n"
               "}";
        values = removeSymbolicTok(tokenValues(code, "+", &s));
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(100, values.back().intvalue);

        code = "unsigned short f(unsigned short x) [[expects: x <= 100]] {\n"
               "  return x + 0;\n"
               "}";
        values = removeSymbolicTok(tokenValues(code, "+", &s));
        values.remove_if([](const ValueFlow::Value& v) {
            return v.isImpossible();
        });
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(0, values.front().intvalue);
        ASSERT_EQUALS(100, values.back().intvalue);
    }

    void valueFlowUnknownFunctionReturn() {
        const char code[] = "template <typename T>\n" // #13409
                            "struct S {\n"
                            "    std::max_align_t T::* m;\n"
                            "    S(std::max_align_t T::* p) : m(p) {}\n"
                            "};\n";
        (void)valueOfTok(code, ":"); // don't crash
    }

    void valueFlowUnknownFunctionReturnRand() {
        const char *code;
        std::list<ValueFlow::Value> values;
        /*const*/ Settings s = settingsBuilder().library("std.cfg").build();
        s.checkUnknownFunctionReturn.insert("rand");

        code = "x = rand();";
        values = tokenValues(code, "(", &s);
        ASSERT_EQUALS(2, values.size());
        ASSERT_EQUALS(INT_MIN, values.front().intvalue);
        ASSERT_EQUALS(INT_MAX, values.back().intvalue);
    }

    void valueFlowUnknownFunctionReturnMalloc() { // #4626
        const char *code;

        code = "ptr = malloc(10);";
        const auto& values = tokenValues(code, "(");
        ASSERT_EQUALS(1, values.size());
        ASSERT_EQUALS(true, values.front().isIntValue());
        ASSERT_EQUALS(true, values.front().isPossible());
        ASSERT_EQUALS(0, values.front().intvalue);
    }


    void valueFlowPointerAliasDeref() {
        const char* code;

        code = "int f() {\n"
               "  int a = 123;\n"
               "  int *p = &a;\n"
               "  int x = *p;\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 5U, 123));
    }

    void valueFlowCrashIncompleteCode() {
        const char* code;

        code = "void SlopeFloor::setAttr(const Value &val) {\n"
               "    int x = val;\n"
               "    if (x >= -1)\n"
               "        state = x;\n"
               "}\n";
        (void)valueOfTok(code, "=");

        code = "void a() {\n"
               "  auto b = [b = 0] {\n"
               "    if (b) {\n"
               "    }\n"
               "  };\n"
               "}\n";
        (void)valueOfTok(code, "0");

        code = "namespace juce {\n"
               "PopupMenu::Item& PopupMenu::Item::operator= (Item&&) = default;\n"
               "PopupMenu::Options withDeletionCheck (Component& comp) const {\n"
               "    Options o (*this);\n"
               "    o.componentToWatchForDeletion = &comp;\n"
               "    o.isWatchingForDeletion = true;\n"
               "    return o;\n"
               "}}\n";
        (void)valueOfTok(code, "return");

        code = "class dummy_resource : public instrument_resource {\n"
               "public:\n"
               "    int reads;\n"
               "    static std::list<int> log;\n"
               "};\n"
               "void dummy_reader_reset() {\n"
               "    dummy_resource::log.clear();\n"
               "}\n";
        (void)valueOfTok(code, "log");

        code = "struct D : B<int> {\n"
               "    D(int i, const std::string& s) : B<int>(i, s) {}\n"
               "};\n"
               "template<> struct B<int>::S {\n"
               "    int j;\n"
               "};\n";
        (void)valueOfTok(code, "B");

        code = "void f(int& r) {\n" // #13515
               "    [0].p = &r;\n"
               "}\n";
        (void)valueOfTok(code, "=");
    }

    void valueFlowCrash() {
        const char* code;

        code = "void f(int x) {\n"
               "    if (0 * (x > 2)) {}\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "struct a {\n"
               "  void b();\n"
               "};\n"
               "void d(std::vector<a> c) {\n"
               "  a *e;\n"
               "  for (auto &child : c)\n"
               "    e = &child;\n"
               "  (*e).b();\n"
               "}\n";
        (void)valueOfTok(code, "e");

        code = "const int& f(int, const int& y = 0);\n"
               "const int& f(int, const int& y) {\n"
               "    return y;\n"
               "}\n"
               "const int& g(int x) {\n"
               "    const int& r = f(x);\n"
               "    return r;\n"
               "}\n";
        (void)valueOfTok(code, "0");

        code = "void fa(int &colors) {\n"
               "  for (int i = 0; i != 6; ++i) {}\n"
               "}\n"
               "void fb(not_null<int*> parent, int &&colors2) {\n"
               "  dostuff(1);\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "void a() {\n"
               "  static int x = 0;\n"
               "  struct c {\n"
               "    c(c &&) { ++x; }\n"
               "  };\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "void f(){\n"
               "      struct dwarf_data **pp;\n"
               "      for (pp = (struct dwarf_data **) (void *) &state->fileline_data;\n"
               "       *pp != NULL;\n"
               "       pp = &(*pp)->next)\n"
               "    ;\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "void *foo(void *x);\n"
               "void *foo(void *x)\n"
               "{\n"
               "    if (!x)\n"
               "yes:\n"
               "        return &&yes;\n"
               "    return x;\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "void f() {\n"
               "    std::string a = b[c->d()];\n"
               "    if(a.empty()) {\n"
               "        INFO(std::string{\"a\"} + c->d());\n"
               "        INFO(std::string{\"b\"} + a);\n"
               "    }\n"
               "}\n";
        (void)valueOfTok(code, "a");

        code = "class A{\n"
               "  void f() {\n"
               "    std::string c{s()};\n"
               "  }\n"
               "  std::string s() {\n"
               "    return \"\";\n"
               "  }\n"
               "};\n";
        (void)valueOfTok(code, "c");

        code = "void f() {\n"
               "   char* p = 0;\n"
               "   int pi =\n"
               "     p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 : p == \"a\" ? 1 \n"
               "   : 0;\n"
               "   int *i2 = 0;\n"
               "   if (i2) { }\n"
               "}\n";
        (void)valueOfTok(code, "p");

        code = "struct a;\n"
               "namespace e {\n"
               "struct f {\n"
               "  struct g {\n"
               "    enum {} h;\n"
               "    int arg;\n"
               "  };\n"
               "  std::vector<g> i;\n"
               "};\n"
               "} // namespace e\n"
               "void fn1() {\n"
               "  std::vector<a *> arguments;\n"
               "  e::f b;\n"
               "  for (e::f::g c : b.i)\n"
               "    if (c.h)\n"
               "      a *const d = arguments[c.arg];\n"
               "}\n";
        (void)valueOfTok(code, "c");

        code = "void h(char* p, int s) {\n"
               "  char *q = p+s;\n"
               "  char buf[100];\n"
               "  char *b = buf;\n"
               "  ++b;\n"
               "  if (p < q && buf < b)\n"
               "    diff = (buf-b);\n"
               "}\n";
        (void)valueOfTok(code, "diff");

        code = "void foo() {\n" // #10462
               "  std::tuple<float, float, float, float> t4(5.2f, 3.1f, 2.4f, 9.1f), t5(4, 6, 9, 27);\n"
               "  t4 = t5;\n"
               "  ASSERT(!(t4 < t5) && t4 <= t5);\n"
               "}";
        (void)valueOfTok(code, "<=");

        code = "void f() {\n"
               "    unsigned short Xoff = 10;\n"
               "    unsigned short Nx = 0;\n"
               "    int last;\n"
               "    do {\n"
               "        last = readData(0);\n"
               "        if (last && (last - Xoff < Nx))\n"
               "            Nx = last - Xoff;\n"
               "    } while (last > 0);\n"
               "}\n";
        (void)valueOfTok(code, "last");

        code = "struct a {\n"
               "  void clear();\n"
               "  int b();\n"
               "};\n"
               "struct d {\n"
               "  void c(int);\n"
               "  decltype(auto) f() { c(0 != e.b()); }\n"
               "  a e;\n"
               "};\n"
               "void d::c(int) { e.clear(); }\n";
        (void)valueOfTok(code, "e");

        code = "struct a {\n"
               "  int b;\n"
               "  int c;\n"
               "} f;\n"
               "unsigned g;\n"
               "struct {\n"
               "  a d;\n"
               "} e;\n"
               "void h() {\n"
               "  if (g && f.c)\n"
               "    e.d.b = g - f.c;\n"
               "}\n";
        (void)valueOfTok(code, "e");

        code = "struct a {\n"
               "  std::vector<a> b;\n"
               "  void c(unsigned d) {\n"
               "    size_t e = 0;\n"
               "    size_t f = 0;\n"
               "    for (auto child : b) {\n"
               "      f = e;\n"
               "      e = d - f;\n"
               "    }\n"
               "  }\n"
               "};\n";
        (void)valueOfTok(code, "e");

        code = "struct a {\n"
               "  struct b {\n"
               "    std::unique_ptr<a> c;\n"
               "  };\n"
               "  void d(int, void *);\n"
               "  void e() {\n"
               "    d(0, [f = b{}] { return f.c.get(); }());\n"
               "  }\n"
               "  void g() {\n"
               "    if (b *h = 0)\n"
               "      h->c.get();\n"
               "  }\n"
               "};\n";
        (void)valueOfTok(code, "f.c");

        code = "void d(fmpz_t a, fmpz_t b) {\n"
               "  if (fmpz_sgn(0)) {}\n"
               "  else if (b) {}\n"
               "}\n"
               "void e(psl2z_t f) {\n"
               "  f->b;\n"
               "  d(&f->a, c);\n"
               "}\n";
        (void)valueOfTok(code, "f");

        code = "struct bo {\n"
               "  int b, c, a, d;\n"
               "  char e, g, h, i, aa, j, k, l, m, n, o, p, q, r, t, u, v, w, x, y;\n"
               "  long z, ab, ac, ad, f, ae, af, ag, ah, ai, aj, ak, al, am, an, ao, ap, aq, ar,\n"
               "      as;\n"
               "  short at, au, av, aw, ax, ay, az, ba, bb, bc, bd, be, bf, bg, bh, bi, bj, bk,\n"
               "      bl, bm;\n"
               "};\n"
               "char bn;\n"
               "void bp() {\n"
               "  bo s;\n"
               "  if (bn)\n"
               "    return;\n"
               "  s;\n"
               "}\n";
        (void)valueOfTok(code, "s");

        code = "int f(int value) { return 0; }\n"
               "std::shared_ptr<Manager> g() {\n"
               "    static const std::shared_ptr<Manager> x{ new M{} };\n"
               "    return x;\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "int* g();\n"
               "void f() {\n"
               "    std::cout << (void*)(std::shared_ptr<int>{ g() }.get());\n"
               "}\n";
        (void)valueOfTok(code, ".");

        code = "class T;\n"
               "struct S {\n"
               "    void f(std::array<T*, 2>& a);\n"
               "};\n";
        (void)valueOfTok(code, "a");

        code = "void f(const char * const x) { !!system(x); }\n";
        (void)valueOfTok(code, "x");

        code = "struct struct1 {\n"
               "    int i1;\n"
               "    int i2;\n"
               "};\n"
               "struct struct2 {\n"
               "    char c1;\n"
               "    struct1 is1;\n"
               "    char c2[4];\n"
               "};\n"
               "void f() {\n"
               "    struct2 a = { 1, 2, 3, {4,5,6,7} }; \n"
               "}\n";
        (void)valueOfTok(code, "a");

        code = "void setDeltas(int life, int age, int multiplier) {\n"
               "    int dx = 0;\n"
               "    int dy = 0;\n"
               "    if (age <= 2 || life < 4) {\n"
               "        dy = 0;\n"
               "        dx = (rand() % 3) - 1;\n"
               "    }\n"
               "    else if (age < (multiplier * 3)) {\n"
               "        if (age % (int) (multiplier * 0.5) == 0) dy = -1;\n"
               "        else dy = 0;\n"
               "    }\n"
               "}\n";
        (void)valueOfTok(code, "age");

        code = "void a() {\n"
               "  struct b {\n"
               "    int d;\n"
               "  };\n"
               "  for (b c : {b{}, {}}) {}\n"
               "}\n";
        (void)valueOfTok(code, "c");

        code = "class T {\n"
               "private:\n"
               "    void f() { D& r = dynamic_cast<D&>(*m); }\n"
               "    void g() { m.reset(new D); }\n"
               "private:\n"
               "    std::shared_ptr<B> m;\n"
               "};\n";
        (void)valueOfTok(code, "r");

        code = "void g(int);\n"
               "void f(int x, int y) {\n"
               "    g(x < y ? : 1);\n"
               "};\n";
        (void)valueOfTok(code, "?");

        code = "struct C {\n"
               "    explicit C(bool);\n"
               "    operator bool();\n"
               "};\n"
               "void f(bool b) {\n"
               "    const C& c = C(b) ? : C(false);\n"
               "};\n";
        (void)valueOfTok(code, "?");

        code = "struct S {\n"
               "    void g(std::vector<int> (*f) () = nullptr);\n"
               "};\n";
        (void)valueOfTok(code, "=");

        code = "void f(bool b) {\n" // #11627
               "    (*printf)(\"%s %i\", strerror(errno), b ? 0 : 1);\n"
               "};\n";
        (void)valueOfTok(code, "?");

        code = "void f(int i) {\n" // #11914
               "    int& r = i;\n"
               "    int& q = (&r)[0];\n"
               "}\n";
        (void)valueOfTok(code, "&");

        code = "bool a(int *);\n"
               "void fn2(int b) {\n"
               "  if (b) {\n"
               "    bool c, d, e;\n"
               "    if (c && d)\n"
               "      return;\n"
               "    if (e && a(&b)) {\n"
               "    }\n"
               "  }\n"
               "}\n";
        (void)valueOfTok(code, "e");

        code = "void f(int a, int b, int c) {\n"
               "  if (c && (a || a && b))\n"
               "    if (a && b) {}\n"
               "}\n";
        (void)valueOfTok(code, "a");

        code = "void g(const char* fmt, ...);\n" // #12255
               "void f(const char* fmt, const char* msg) {\n"
               "    const char* p = msg;\n"
               "    g(\"%s\", msg);\n"
               "}\n"
               "void g(const char* fmt, ...) {\n"
               "    const char* q = fmt;\n"
               "    if (*q > 0 && *q < 100) {}\n"
               "}\n";
        (void)valueOfTok(code, "&&");

        code = "void f() { int& a = *&a; }\n"; // #12511
        (void)valueOfTok(code, "=");

        code = "void g(int*);\n" // #12716
               "void f(int a) {\n"
               "    do {\n"
               "        if (a)\n"
               "            break;\n"
               "        g((int[256]) { 0 });\n"
               "    } while (true);\n"
               "}\n";
        (void)valueOfTok(code, "0");

        code = "bool f() {\n"
               "    return (!std::is_reference<decltype(a)>::value);\n"
               "}\n";
        (void)valueOfTok(code, "0");

        code = "struct S { int a; };\n" // #14036
               "template <typename T>\n"
               "struct U {\n"
               "    U() = default;\n"
               "    U(int i, int* p) {\n"
               "        m = new T(i, p);\n"
               "    }\n"
               "    T m;\n"
               "};\n"
               "U<S*> u;\n";
        (void)valueOfTok(code, "new");

        code = "void f() {\n" // #14287
               "    auto a = { \"1\" };\n"
               "    auto b = a;\n"
               "}\n";
        (void)valueOfTok(code, "b");
    }

    void valueFlowHang() {
        const char* code;
        // #9659
        code = "float arr1[4][4] = {0.0};\n"
               "float arr2[4][4] = {0.0};\n"
               "void f() {\n"
               "    if(arr1[0][0] == 0.0 &&\n"
               "       arr1[0][1] == 0.0 &&\n"
               "       arr1[0][2] == 0.0 &&\n"
               "       arr1[0][3] == 0.0 &&\n"
               "       arr1[1][0] == 0.0 &&\n"
               "       arr1[1][1] == 0.0 &&\n"
               "       arr1[1][2] == 0.0 &&\n"
               "       arr1[1][3] == 0.0 &&\n"
               "       arr1[2][0] == 0.0 &&\n"
               "       arr1[2][1] == 0.0 &&\n"
               "       arr1[2][2] == 0.0 &&\n"
               "       arr1[2][3] == 0.0 &&\n"
               "       arr1[3][0] == 0.0 &&\n"
               "       arr1[3][1] == 0.0 &&\n"
               "       arr1[3][2] == 0.0 &&\n"
               "       arr1[3][3] == 0.0 &&\n"
               "       arr2[0][0] == 0.0 &&\n"
               "       arr2[0][1] == 0.0 &&\n"
               "       arr2[0][2] == 0.0 &&\n"
               "       arr2[0][3] == 0.0 &&\n"
               "       arr2[1][0] == 0.0 &&\n"
               "       arr2[1][1] == 0.0 &&\n"
               "       arr2[1][2] == 0.0 &&\n"
               "       arr2[1][3] == 0.0 &&\n"
               "       arr2[2][0] == 0.0 &&\n"
               "       arr2[2][1] == 0.0 &&\n"
               "       arr2[2][2] == 0.0 &&\n"
               "       arr2[2][3] == 0.0 &&\n"
               "       arr2[3][0] == 0.0 &&\n"
               "       arr2[3][1] == 0.0 &&\n"
               "       arr2[3][2] == 0.0 &&\n"
               "       arr2[3][3] == 0.0\n"
               "       ) {}\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "namespace {\n"
               "struct a {\n"
               "  a(...) {}\n"
               "  a(std::initializer_list<std::pair<int, std::vector<std::vector<a>>>>) {}\n"
               "} b{{0, {{&b, &b, &b, &b}}},\n"
               "    {0,\n"
               "     {{&b, &b, &b, &b, &b, &b, &b, &b, &b, &b},\n"
               "      {{&b, &b, &b, &b, &b, &b, &b}}}},\n"
               "    {0,\n"
               "     {{&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b},\n"
               "      {&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b}}}};\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "namespace {\n"
               "struct a {\n"
               "  a(...) {}\n"
               "  a(std::initializer_list<std::pair<int, std::vector<std::vector<a>>>>) {}\n"
               "} b{{0, {{&b}}},\n"
               "    {0, {{&b}}},\n"
               "    {0, {{&b}}},\n"
               "    {0, {{&b}}},\n"
               "    {0, {{&b}, {&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, {&b}}}},\n"
               "    {0,\n"
               "     {{&b},\n"
               "      {&b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b, &b,\n"
               "       &b}}}};\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "int &a(int &);\n"
               "int &b(int &);\n"
               "int &c(int &);\n"
               "int &d(int &e) {\n"
               "  if (!e)\n"
               "    return a(e);\n"
               "  if (e > 0)\n"
               "    return b(e);\n"
               "  if (e < 0)\n"
               "    return c(e);\n"
               "  return e;\n"
               "}\n"
               "int &a(int &e) { \n"
               "  if (!e)\n"
               "    return d(e); \n"
               "  if (e > 0)\n"
               "    return b(e);\n"
               "  if (e < 0)\n"
               "    return c(e);\n"
               "  return e;\n"
               "}\n"
               "int &b(int &e) { \n"
               "  if (!e)\n"
               "    return a(e); \n"
               "  if (e > 0)\n"
               "    return c(e);\n"
               "  if (e < 0)\n"
               "    return d(e);\n"
               "  return e;\n"
               "}\n"
               "int &c(int &e) { \n"
               "  if (!e)\n"
               "    return a(e); \n"
               "  if (e > 0)\n"
               "    return b(e);\n"
               "  if (e < 0)\n"
               "    return d(e);\n"
               "  return e;\n"
               "}\n";
        (void)valueOfTok(code, "x");

        code = "void a() {\n"
               "  int b = 0;\n"
               "  do {\n"
               "    for (;;)\n"
               "      break;\n"
               "  } while (b < 1);\n"
               "}\n";
        (void)valueOfTok(code, "b");

        code = "void ParseEvent(tinyxml2::XMLDocument& doc, std::set<Item*>& retItems) {\n"
               "    auto ParseAddItem = [&](Item* item) {\n"
               "        return retItems.insert(item).second;\n"
               "    };\n"
               "    tinyxml2::XMLElement *root = doc.RootElement();\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "    for (auto *el = root->FirstChildElement(\"Result\"); el && !ParseAddItem(GetItem(el)); el = el->NextSiblingElement(\"Result\")) ;\n"
               "}\n";
        (void)valueOfTok(code, "root");

        code = "bool isCharPotentialOperator(char ch)  {\n"
               "    return (ispunct((unsigned char) ch)\n"
               "            && ch != '{' && ch != '}'\n"
               "            && ch != '(' && ch != ')'\n"
               "            && ch != '[' && ch != ']'\n"
               "            && ch != ';' && ch != ','\n"
               "            && ch != '#' && ch != '\\\\'\n"
               "            && ch != '\\\'' && ch != '\\\"');\n"
               "}\n";
        (void)valueOfTok(code, "return");

        code = "void heapSort() {\n"
               "    int n = m_size;\n"
               "    while (n >= 1) {\n"
               "        swap(0, n - 1);\n"
               "    }\n"
               "}\n";
        (void)valueOfTok(code, "swap");

        code = "double a;\n"
               "int b, c, d, e, f, g;\n"
               "void h() { double i, j = i = g = f = e = d = c = b = a; }\n";
        (void)valueOfTok(code, "a");

        code = "double a, c;\n"
               "double *b;\n"
               "void d() {\n"
               "  double e, f, g, h = g = f = e = c = a;\n"
               "  b[8] = a;\n"
               "  b[1] = a;\n"
               "  a;\n"
               "}\n";
        (void)valueOfTok(code, "a");

        code = "void f(int i, int j, int n) {\n"
               "    if ((j == 0) != (i == 0)) {}\n"
               "    int t = 0;\n"
               "    if (j > 0) {\n"
               "        t = 1;\n"
               "        if (n < j)\n"
               "            n = j;\n"
               "    }\n"
               "}\n";
        (void)valueOfTok(code, "i");

        code = "void f() {\n" // #11701
               "    std::vector<int> v(500);\n"
               "    for (int i = 0; i < 500; i++) {\n"
               "        if (i < 122)\n"
               "            v[i] = 255;\n"
               "        else if (i == 122)\n"
               "            v[i] = 220;\n"
               "        else if (i < 386)\n"
               "            v[i] = 196;\n"
               "        else if (i == 386)\n"
               "            v[i] = 118;\n"
               "        else\n"
               "            v[i] = 0;\n"
               "    }\n"
               "}\n";
        (void)valueOfTok(code, "i");

        code = "void f() {\n"
               "    if (llabs(0x80000000ffffffffL) == 0x7fffffff00000001L) {}\n"
               "}\n";
        (void)valueOfTok(code, "f");

        code = "struct T {\n"
               "    T();\n"
               "    static T a[6][64];\n"
               "    static T b[2][64];\n"
               "    static T c[64][64];\n"
               "    static T d[2][64];\n"
               "    static T e[64];\n"
               "    static T f[64];\n"
               "};\n";
        (void)valueOfTok(code, "(");
    }

    void valueFlowCrashConstructorInitialization() { // #9577
        const char* code;
        code = "void Error()\n"
               "{\n"
               "    VfsPath path(\"\");\n"
               "    path = path / amtype;\n"
               "    size_t base = 0;\n"
               "    VfsPath standard(\"standard\");\n"
               "    if (path != standard)\n"
               "    {\n"
               "    }\n"
               "}";
        (void)valueOfTok(code, "path");

        code = "void Error()\n"
               "{\n"
               "    VfsPath path;\n"
               "    path = path / amtype;\n"
               "    size_t base = 0;\n"
               "    VfsPath standard(\"standard\");\n"
               "    if (path != standard)\n"
               "    {\n"
               "    }\n"
               "}";
        (void)valueOfTok(code, "path");

        code = "struct S {\n"
               "    std::string to_string() const {\n"
               "        return { this->p , (size_t)this->n };\n"
               "    }\n"
               "    const char* p;\n"
               "    int n;\n"
               "};\n"
               "void f(S s, std::string& str) {\n"
               "    str += s.to_string();\n"
               "}\n";
        (void)valueOfTok(code, "s");

        code = "void a(int e, int d, int c, int h) {\n"
               "  std::vector<int> b;\n"
               "  std::vector<int> f;\n"
               "  if (b == f && h)\n"
               "    return;\n"
               "  if (b == f && b == f && c && e < d) {}\n"
               "}\n";
        (void)valueOfTok(code, "b");
    }

    void valueFlowUnknownMixedOperators() {
        const char *code= "int f(int a, int b, bool x) {\n"
                          "  if (a == 1 && (!(b == 2 && x))) {\n"
                          "  } else {\n"
                          "    if (x) {\n"
                          "    }\n"
                          "  }\n"
                          "\n"
                          "  return 0;\n"
                          "}";

        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));
    }

    void valueFlowSolveExpr()
    {
        const char* code;
        code = "int f(int x) {\n"
               "    if ((64 - x) == 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 56));

        code = "int f(int x) {\n"
               "    if ((x - 64) == 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 72));

        code = "int f(int x) {\n"
               "    if ((x - 64) == 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 72));

        code = "int f(int x) {\n"
               "    if ((x + 64) == 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, -56));

        code = "int f(int x) {\n"
               "    if ((x * 2) == 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 4));

        code = "int f(int x) {\n"
               "    if ((x ^ 64) == 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, 72));

        code = "int f(int i) {\n"
               "    int j = i + 64;\n"
               "    int x = j;\n"
               "    return x;\n"
               "}\n";
        TODO_ASSERT_EQUALS(true, false, testValueOfXKnown(code, 4U, "i", 64));
    }

    void valueFlowIdempotent() {
        const char *code;

        code = "void f(bool a, bool b) {\n"
               "    bool x = true;\n"
               "    if (a)\n"
               "        x = x && b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 1));

        code = "void f(bool a, bool b) {\n"
               "    bool x = false;\n"
               "    if (a)\n"
               "        x = x && b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));

        code = "void f(bool a, bool b) {\n"
               "    bool x = true;\n"
               "    if (a)\n"
               "        x = x || b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "void f(bool a, bool b) {\n"
               "    bool x = false;\n"
               "    if (a)\n"
               "        x = x || b;\n"
               "    bool result = x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 5U, 0));

        code = "void foo() {\n"
               "    int x = 0;\n"
               "    for (int i = 0; i < 5; i++) {\n"
               "        int y = 0;\n"
               "        for (int j = 0; j < 10; j++)\n"
               "            y++;\n"
               "        if (y >= x)\n"
               "            x = y;\n"
               "    }\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 10U, 0));
    }

    void valueFlowUnsigned() {
        const char *code;

        code = "auto f(uint32_t i) {\n"
               "    auto x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));

        code = "auto f(uint32_t i) {\n"
               "    auto x = (int32_t)i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, -1));

        code = "auto f(uint32_t i) {\n"
               "    auto x = (int64_t)i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));

        code = "size_t g();\n"
               "auto f(uint16_t j) {\n"
               "    auto x = g() - j;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 4U, 0));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, -1));

        code = "auto f(uint32_t i) {\n"
               "    auto x = (i + 1) % 16;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 0));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));

        code = "auto f(uint32_t i) {\n"
               "    auto x = i ^ 3;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 2));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));

        code = "auto f(uint32_t i) {\n"
               "    auto x = i & 3;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 2));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));
    }

    void valueFlowMod() {
        const char *code;

        code = "auto f(int i) {\n"
               "    auto x = i % 2;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, 2));

        code = "auto f(int i) {\n"
               "    auto x = !(i % 2);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 1));
    }

    void valueFlowIncDec() {
        const char *code;
        std::list<ValueFlow::Value> values;

        // #11591
        code = "int f() {\n"
               "    const int a[1] = {};\n"
               "    unsigned char i = 255;\n"
               "    ++i;\n"
               "    return a[i];\n"
               "}\n";
        values = tokenValues(code, "i ]");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(0LLU, values.back().intvalue);

        code = "int f() {\n"
               "    const int a[1] = {};\n"
               "    unsigned char i = 255;\n"
               "    return a[++i];\n"
               "}\n";
        values = tokenValues(code, "++");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(0LLU, values.back().intvalue);

        code = "int f() {\n"
               "     const int a[128] = {};\n"
               "     char b = -128;\n"
               "     return a[--b];\n"
               "}\n";
        values = tokenValues(code, "--");
        ASSERT_EQUALS(1U, values.size());
        ASSERT_EQUALS(127LLU, values.back().intvalue);
    }

    void valueFlowNotNull()
    {
        const char* code;

        code = "int f(const std::string &str) {\n"
               "    int x = str.c_str();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, 0));

        code = "int f(const std::string_view &str) {\n"
               "    int x = str.c_str();\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 3U, 0));

        code = "auto f() {\n"
               "    std::shared_ptr<int> x = std::make_shared<int>(1);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, 0));

        code = "auto f() {\n"
               "    std::unique_ptr<int> x = std::make_unique<int>(1);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, 0));

        code = "struct A {\n"
               "    A* f() {\n"
               "        A* x = this;\n"
               "        return x;\n"
               "    }\n"
               "};\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, 0));
    }

    void valueFlowSymbolic() {
        const char* code;

        code = "int f(int i) {\n"
               "    int j = i;\n"
               "    int x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, "j", 0));
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, "i", 0));

        code = "int f(int i) {\n"
               "    int j = i;\n"
               "    int x = j;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, "i", 0));
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, "j", 0));

        code = "void g(int&);\n"
               "int f(int i) {\n"
               "    int j = i;\n"
               "    g(i);\n"
               "    int x = i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 6U, "i", 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 6U, "j", 0));

        code = "int f(int i) {\n"
               "    int j = i;\n"
               "    j++;\n"
               "    int x = i == j;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));

        code = "int f(int i) {\n"
               "    int j = i;\n"
               "    i++;\n"
               "    int x = i - j;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "int f(int i) {\n"
               "    int j = i;\n"
               "    i++;\n"
               "    int x = i > j;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "int f(int i) {\n"
               "    int j = i;\n"
               "    j++;\n"
               "    int x = j > i;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "int f(int i) {\n"
               "    int j = i++;\n"
               "    int x = i++;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, "i++", 0));

        code = "float foo() {\n"
               "    float f = 1.0f;\n"
               "    float x = f;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, "1.0f", 0));

        code = "int foo(float f) {\n"
               "    float g = f;\n"
               "    int x = f == g;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));

        code = "int f(int i) {\n"
               "  for(int j = i;;j++) {\n"
               "    int x = j;\n"
               "    return x;\n"
               "  }\n"
               "  return 0;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, "i", 0));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, "i", 1));
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, "j", 0));

        code = "void f(int x) {\n"
               "  int y = x + 1;\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, "y", 0));
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "y", -1));

        code = "void f(int x) {\n"
               "  int y = x * 2;\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, "y", 0));

        code = "int f(int i, int j) {\n"
               "    if (i == j) {\n"
               "        int x = i - j;\n"
               "        return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(int x, int y) {\n"
               "    if (x == y) {\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "y", 0));

        code = "void f(int x, int y) {\n"
               "    if (x != y) {\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "y", 0));

        code = "void f(int x, int y) {\n"
               "    if (x < y) {\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "y", -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "y", 0));

        code = "void f(int x, int y) {\n"
               "    if (x <= y) {\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "y", 0));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "y", 1));

        code = "void f(int x, int y) {\n"
               "    if (x > y) {\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "y", 1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "y", 0));

        code = "void f(int x, int y) {\n"
               "    if (x >= y) {\n"
               "        int a = x;\n"
               "    }\n"
               "}";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "y", 0));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "y", -1));

        code = "void f(int y) {\n"
               "  int x = y - 1;\n"
               "  if (y == 1)\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "void f(int y) {\n"
               "  int x = y * y;\n"
               "  if (y == 2)\n"
               "    int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 4));

        code = "void f(int x, int y) {\n"
               "  if (x == y*y)\n"
               "    if (y == 2)\n"
               "      int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 4));

        code = "void f(int x, int y) {\n"
               "  if (x > y*y)\n"
               "    if (y == 2)\n"
               "      int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, 4));

        code = "void f(int x, int y) {\n"
               "  if (x != y*y)\n"
               "    if (y == 2)\n"
               "      int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, 4));

        code = "void f(int x, int y) {\n"
               "  if (x >= y*y)\n"
               "    if (y == 2)\n"
               "      int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, 3));

        code = "void f(int x, int y) {\n"
               "  if (x == y*y)\n"
               "    if (y != 2)\n"
               "      int a = x;\n"
               "}\n";
        TODO_ASSERT_EQUALS(true, false, testValueOfXImpossible(code, 4U, 4));

        code = "void f(int x, int y) {\n"
               "  if (x == y*y)\n"
               "    if (y > 2)\n"
               "      int a = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 9));

        code = "struct A {\n"
               "    A* b();\n"
               "    int c() const;\n"
               "};\n"
               "void f(A *d) {\n"
               "    if (!d || d->c() != 1)\n"
               "        return;\n"
               "    A * y = d;\n"
               "    d = d->b();\n"
               "    A * x = d;\n"
               "    A* z = x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 11U, "d", 0));
        ASSERT_EQUALS(false, testValueOfXImpossible(code, 11U, 0));

        code = "void f(int * p, int len) {\n"
               "    for(int x = 0; x < len; ++x) {\n"
               "        p[x] = 1;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "len", -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "len", 0));

        code = "int f(int x) {\n"
               "    int i = 64 - x;\n"
               "    if(i < 8)\n"
               "        return x;\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 71));
        TODO_ASSERT_EQUALS(true, false, testValueOfX(code, 4U, 56));

        code = "int b(int a) {\n"
               "  unsigned long x = a ? 6 : 4;\n"
               "  assert(x < 6 && x > 0);\n"
               "  return 1 / x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 4U, 0));

        code = "void f(int k) {\n"
               "  int x = k;\n"
               "  int j = k;\n"
               "  x--;\n"
               "  if (k != 0) {\n"
               "    x;\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 6U, -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 6U, -1));

        code = "char* f() {\n"
               "    char *x = malloc(10);\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, "malloc(10)", 0));
    }

    void valueFlowSymbolicIdentity()
    {
        const char* code;

        code = "void f(int a) {\n"
               "    int x = a*1;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a/1;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a+0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a-0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a^0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a|0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a>>0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = a<<0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = 0>>a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int a) {\n"
               "    int x = 0<<a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 3U, "a", 0));

        code = "void f(int n) {\n"
               "    int x = 0 - n;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 3U, "n", ValueFlow::Value::ValueType::SYMBOLIC));

        code = "void f(int n) {\n"
               "    int x = n - 0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfX(code, 3U, "n", ValueFlow::Value::ValueType::SYMBOLIC));
    }

    void valueFlowSymbolicStrlen()
    {
        const char* code;

        code = "int f(char *s) {\n"
               "    size_t len = strlen(s);\n"
               "    int x = s[len];\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "int f(char *s, size_t i) {\n"
               "    if (i < strlen(s)) {\n"
               "      int x = s[i];\n"
               "      return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, 0));

        code = "int f(char *s, size_t i) {\n"
               "    if (i < strlen(s)) {\n"
               "      int x = s[i] != ' ';\n"
               "      return x;\n"
               "    }\n"
               "    return 0;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 1));

        code = "int f(char *s, size_t i) {\n"
               "    if (i == strlen(s)) {}\n"
               "    int x = s[i];\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfXKnown(code, 4U, 0));
        ASSERT_EQUALS(true, testValueOfX(code, 4U, 0));
    }

    void valueFlowSmartPointer()
    {
        const char* code;

        code = "int* df(int* expr);\n"
               "int * f() {\n"
               "    std::unique_ptr<int> x;\n"
               "    x.reset(df(x.release()));\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
    }

    void valueFlowImpossibleMinMax()
    {
        const char* code;

        code = "void f(int a, int b) {\n"
               "    int x = a < b ? a : b;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", 1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "b", 1));

        code = "void f(int a, int b) {\n"
               "    int x = a > b ? a : b;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "b", -1));

        code = "void f(int a, int b) {\n"
               "    int x = a > b ? b : a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", 1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "b", 1));

        code = "void f(int a, int b) {\n"
               "    int x = a < b ? b : a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "b", -1));

        code = "void f(int a) {\n"
               "    int x = a < 0 ? a : 0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", 1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, 1));

        code = "void f(int a) {\n"
               "    int x = a > 0 ? a : 0;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));

        code = "void f(int a) {\n"
               "    int x = a > 0 ? 0 : a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", 1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, 1));

        code = "void f(int a) {\n"
               "    int x = a < 0 ? 0 : a;\n"
               "    return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, "a", -1));
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 3U, -1));
    }

    void valueFlowImpossibleIncDec()
    {
        const char* code;

        code = "int f() {\n"
               "    for(int i = 0; i < 5; i++) {\n"
               "        int x = i;\n"
               "        return x;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, -1));

        code = "void f(int N, int z) {\n"
               "  std::vector<int> a(N);\n"
               "  int m = -1;\n"
               "  m = 0;\n"
               "  for (int k = 0; k < N; k++) {\n"
               "    int x = m + k;\n"
               "    if (z == a[x]) {}\n"
               "  }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 7U, -1));
        ASSERT_EQUALS(false, testValueOfXKnown(code, 7U, -1));
    }

    void valueFlowImpossibleUnknownConstant()
    {
        const char* code;

        code = "void f(bool b) {\n"
               "    if (b) {\n"
               "        int x = -ENOMEM;\n" // assume constant ENOMEM is nonzero since it's negated
               "        if (x != 0) return;\n"
               "    }\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXImpossible(code, 4U, 0));
    }

    void valueFlowContainerEqual()
    {
        const char* code;

        code = "bool f() {\n"
               "  std::string s = \"abc\";\n"
               "  bool x = (s == \"def\");\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 0));

        code = "bool f() {\n"
               "  std::string s = \"abc\";\n"
               "  bool x = (s != \"def\");\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 4U, 1));

        code = "bool f() {\n"
               "  std::vector<int> v1 = {1, 2};\n"
               "  std::vector<int> v2 = {1, 2};\n"
               "  bool x = (v1 == v2);\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 1));

        code = "bool f() {\n"
               "  std::vector<int> v1 = {1, 2};\n"
               "  std::vector<int> v2 = {1, 2};\n"
               "  bool x = (v1 != v2);\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));

        code = "bool f(int i) {\n"
               "  std::vector<int> v1 = {i, i+1};\n"
               "  std::vector<int> v2 = {i};\n"
               "  bool x = (v1 == v2);\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(true, testValueOfXKnown(code, 5U, 0));

        code = "bool f(int i, int j) {\n"
               "  std::vector<int> v1 = {i, i};\n"
               "  std::vector<int> v2 = {i, j};\n"
               "  bool x = (v1 == v2);\n"
               "  return x;\n"
               "}\n";
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 1));
        ASSERT_EQUALS(false, testValueOfX(code, 5U, 0));
    }

    void valueFlowBailoutIncompleteVar() {
        bailout( // #12526
            "int f1() {\n"
            "    return VALUE_1;\n"
            "}\n"
            "\n"
            "int f2() {\n"
            "    return VALUE_2;\n"
            "}\n"
            );
        ASSERT_EQUALS_WITHOUT_LINENUMBERS(
            "[test.cpp:2]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable VALUE_1\n"
            "[test.cpp:6]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable VALUE_2\n",
            errout_str());

        bailout(
            "std::string_view f() {\n"
            "    return \"abc\"sv;\n"
            "}\n"
            );
        ASSERT_EQUALS_WITHOUT_LINENUMBERS("", errout_str());
    }

    void valueFlowBailoutNoreturn() { // #13718
        bailout(
            "void f(const int* p) {\n"
            "    if (p)\n"
            "        (void)*p;\n"
            "}\n"
            );
        ASSERT_EQUALS_WITHOUT_LINENUMBERS("", errout_str());
    }

    void performanceIfCount() {
        /*const*/ Settings s(settings);
        s.vfOptions.maxIfCount = 1;

        const char *code;

        code = "int f() {\n"
               "  if (x>0){}\n"
               "  if (y>0){}\n"
               "  int a = 14;\n"
               "  return a+1;\n"
               "}\n";
        ASSERT_EQUALS(0U, tokenValues(code, "+", &s).size());
        ASSERT_EQUALS(1U, tokenValues(code, "+").size());

        // Do not skip all functions
        code = "void g(int i) {\n"
               "  if (i == 1) {}\n"
               "  if (i == 2) {}\n"
               "}\n"
               "int f() {\n"
               "  std::vector<int> v;\n"
               "  return v.front();\n"
               "}\n";
        ASSERT_EQUALS(1U, tokenValues(code, "v .", &s).size());
    }

#define testBitfields(...) testBitfields_(__FILE__, __LINE__, __VA_ARGS__)
    void testBitfields_(const char *file, int line, const std::string &structBody, std::size_t expectedSize) {
        const Settings settingsUnix64 = settingsBuilder().platform(Platform::Type::Unix64).build();
        const std::string code = "struct S { " + structBody + " }; const std::size_t size = sizeof(S);";
        const auto values = tokenValues(code.c_str(), "( S", &settingsUnix64);
        ASSERT_LOC(!values.empty(), file, line);
        ASSERT_EQUALS_LOC(expectedSize, values.back().intvalue, file, line);
    }

    void bitfields() {

        // #13653
        testBitfields("unsigned int data_rw: 1;\n"
                      "unsigned int page_address: 4;\n"
                      "unsigned int register_address: 3;\n",
                      4);

        testBitfields("unsigned char data_rw: 1;\n"
                      "unsigned char page_address: 4;\n"
                      "unsigned char register_address: 3;\n",
                      1);

        testBitfields("unsigned int a : 1;\n"
                      "unsigned int b;\n"
                      "unsigned int c : 1;\n",
                      12);

        testBitfields("unsigned int a : 1;\n"
                      "unsigned char b;\n"
                      "unsigned int c : 1;\n",
                      12);

        testBitfields("unsigned int a : 31;\n"
                      "unsigned int b : 2;\n",
                      8);

        // #13850
        testBitfields("int a : 32;\n"
                      "int b : 16;\n"
                      "unsigned short c;\n",
                      8);

        testBitfields("unsigned int a : 31;\n"
                      "unsigned int   : 2;\n",
                      8);

        testBitfields("unsigned int a : 16;\n"
                      "unsigned int   : 0;\n"
                      "unsigned int b : 16;\n",
                      8);

        testBitfields("unsigned char a : 16;\n",
                      2);
    }

    void bitfieldsHang() {
        const char *code = "struct S { unknown_type x : 1; };\n"
                           "const size_t size = sizeof(S);\n";
        (void)valueOfTok(code, "x");
    }
};

REGISTER_TEST(TestValueFlow)
