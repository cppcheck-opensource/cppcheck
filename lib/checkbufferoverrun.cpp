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

//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

#include "checkbufferoverrun.h"

#include "astutils.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <numeric> // std::accumulate
#include <sstream>
#include <utility>

#include "xml.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckBufferOverrun instance;
}

//---------------------------------------------------------------------------

// CWE ids used:
static const CWE CWE131(131U);  // Incorrect Calculation of Buffer Size
static const CWE CWE170(170U);  // Improper Null Termination
static const CWE CWE_ARGUMENT_SIZE(398U);  // Indicator of Poor Code Quality
static const CWE CWE_ARRAY_INDEX_THEN_CHECK(398U);  // Indicator of Poor Code Quality
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE_POINTER_ARITHMETIC_OVERFLOW(758U); // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE_BUFFER_UNDERRUN(786U);  // Access of Memory Location Before Start of Buffer
static const CWE CWE_BUFFER_OVERRUN(788U);   // Access of Memory Location After End of Buffer

//---------------------------------------------------------------------------

static const ValueFlow::Value *getBufferSizeValue(const Token *tok)
{
    const std::list<ValueFlow::Value> &tokenValues = tok->values();
    const auto it = std::find_if(tokenValues.cbegin(), tokenValues.cend(), std::mem_fn(&ValueFlow::Value::isBufferSizeValue));
    return it == tokenValues.cend() ? nullptr : &*it;
}

static const Token* getRealBufferTok(const Token* tok) {
    if (!tok->isUnaryOp("&"))
        return tok;

    const Token* op = tok->astOperand1();
    return (op->valueType() && op->valueType()->pointer) ? op : tok;
}

static int getMinFormatStringOutputLength(const std::vector<const Token*> &parameters, nonneg int formatStringArgNr)
{
    if (formatStringArgNr <= 0 || formatStringArgNr > parameters.size())
        return 0;
    if (parameters[formatStringArgNr - 1]->tokType() != Token::eString)
        return 0;
    const std::string &formatString = parameters[formatStringArgNr - 1]->str();
    bool percentCharFound = false;
    int outputStringSize = 0;
    bool handleNextParameter = false;
    std::string digits_string;
    bool i_d_x_f_found = false;
    int parameterLength = 0;
    nonneg int inputArgNr = formatStringArgNr;
    for (std::size_t i = 1; i + 1 < formatString.length(); ++i) {
        if (formatString[i] == '\\') {
            if (i < formatString.length() - 1 && formatString[i + 1] == '0')
                break;

            ++outputStringSize;
            ++i;
            continue;
        }

        if (percentCharFound) {
            switch (formatString[i]) {
            case 'f':
            case 'x':
            case 'X':
            case 'i':
                i_d_x_f_found = true;
                handleNextParameter = true;
                parameterLength = 1; // TODO
                break;
            case 'c':
            case 'e':
            case 'E':
            case 'g':
            case 'o':
            case 'u':
            case 'p':
            case 'n':
                handleNextParameter = true;
                parameterLength = 1; // TODO
                break;
            case 'd':
                i_d_x_f_found = true;
                parameterLength = 1;
                if (inputArgNr < parameters.size() && parameters[inputArgNr]->hasKnownIntValue())
                    parameterLength = MathLib::toString(parameters[inputArgNr]->getKnownIntValue()).length();

                handleNextParameter = true;
                break;
            case 's':
                parameterLength = 0;
                if (inputArgNr < parameters.size() && parameters[inputArgNr]->tokType() == Token::eString)
                    parameterLength = Token::getStrLength(parameters[inputArgNr]);

                handleNextParameter = true;
                break;
            }
        }

        if (formatString[i] == '%')
            percentCharFound = !percentCharFound;
        else if (percentCharFound) {
            digits_string.append(1, formatString[i]);
        }

        if (!percentCharFound)
            outputStringSize++;

        if (handleNextParameter) {
            // NOLINTNEXTLINE(bugprone-unchecked-string-to-number-conversion) - intentional use
            int tempDigits = std::abs(std::atoi(digits_string.c_str()));
            if (i_d_x_f_found)
                tempDigits = std::max(tempDigits, 1);

            if (digits_string.find('.') != std::string::npos) {
                const std::string endStr = digits_string.substr(digits_string.find('.') + 1);
                // NOLINTNEXTLINE(bugprone-unchecked-string-to-number-conversion) - intentional use
                const int maxLen = std::max(std::abs(std::atoi(endStr.c_str())), 1);

                if (formatString[i] == 's') {
                    // For strings, the length after the dot "%.2s" will limit
                    // the length of the string.
                    parameterLength = std::min(parameterLength, maxLen);
                } else {
                    // For integers, the length after the dot "%.2d" can
                    // increase required length
                    tempDigits = std::max(tempDigits, maxLen);
                }
            }

            if (tempDigits < parameterLength)
                outputStringSize += parameterLength;
            else
                outputStringSize += tempDigits;

            parameterLength = 0;
            digits_string.clear();
            i_d_x_f_found = false;
            percentCharFound = false;
            handleNextParameter = false;
            ++inputArgNr;
        }
    }

    return outputStringSize;
}

//---------------------------------------------------------------------------

static bool getDimensionsEtc(const Token * const arrayToken, const Settings &settings, std::vector<Dimension> &dimensions, ErrorPath &errorPath, bool &mightBeLarger, MathLib::bigint &path)
{
    const Token *array = arrayToken;
    while (Token::Match(array, ".|::"))
        array = array->astOperand2();

    if (array->variable() && array->variable()->isArray() && !array->variable()->dimensions().empty()) {
        dimensions = array->variable()->dimensions();
        if (dimensions[0].num <= 1 || !dimensions[0].tok) {
            visitAstNodes(arrayToken,
                          [&](const Token *child) {
                if (child->originalName() == "->") {
                    mightBeLarger = true;
                    return ChildrenToVisit::none;
                }
                return ChildrenToVisit::op1_and_op2;
            });
        }
    } else if (const Token *stringLiteral = array->getValueTokenMinStrSize(settings, &path)) {
        Dimension dim;
        dim.tok = nullptr;
        dim.num = Token::getStrArraySize(stringLiteral);
        dim.known = array->hasKnownValue();
        dimensions.emplace_back(dim);
    } else if (array->valueType() && array->valueType()->pointer >= 1 && (array->valueType()->isIntegral() || array->valueType()->isFloat())) {
        const ValueFlow::Value *value = getBufferSizeValue(array);
        if (!value)
            return false;
        path = value->path;
        errorPath = value->errorPath;
        Dimension dim;
        dim.known = value->isKnown();
        dim.tok = nullptr;
        const auto sizeOf = array->valueType()->pointer > 1 ? ValueType::SizeOf::Pointer : ValueType::SizeOf::Pointee;
        const size_t typeSize = array->valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, sizeOf);
        if (typeSize == 0)
            return false;
        dim.num = value->intvalue / typeSize;
        dimensions.emplace_back(dim);
    }
    return !dimensions.empty();
}

static ValueFlow::Value makeSizeValue(MathLib::bigint size, MathLib::bigint path)
{
    ValueFlow::Value v(size);
    v.path = path;
    return v;
}

static std::vector<ValueFlow::Value> getOverrunIndexValues(const Token* tok,
                                                           const Token* arrayToken,
                                                           const std::vector<Dimension>& dimensions,
                                                           const std::vector<const Token*>& indexTokens,
                                                           MathLib::bigint path)
{
    const Token *array = arrayToken;
    while (Token::Match(array, ".|::"))
        array = array->astOperand2();

    bool isArrayIndex = tok->str() == "[";
    if (isArrayIndex) {
        const Token* parent = tok;
        while (Token::simpleMatch(parent, "["))
            parent = parent->astParent();
        if (!parent || parent->isUnaryOp("&"))
            isArrayIndex = false;
    }

    bool overflow = false;
    std::vector<ValueFlow::Value> indexValues;
    for (std::size_t i = 0; i < dimensions.size() && i < indexTokens.size(); ++i) {
        MathLib::bigint size = dimensions[i].num;
        if (!isArrayIndex)
            size++;
        const bool zeroArray = array->variable() && array->variable()->isArray() && dimensions[i].num == 0;
        std::vector<ValueFlow::Value> values = !zeroArray
                                                   ? ValueFlow::isOutOfBounds(makeSizeValue(size, path), indexTokens[i])
                                                   : std::vector<ValueFlow::Value>{};
        if (values.empty()) {
            if (const ValueFlow::Value* v = indexTokens[i]->getKnownValue(ValueFlow::Value::ValueType::INT))
                indexValues.push_back(*v);
            else
                indexValues.push_back(ValueFlow::Value::unknown());
            continue;
        }
        overflow = true;
        indexValues.push_back(values.front());
    }
    if (overflow)
        return indexValues;
    return {};
}

void CheckBufferOverrun::arrayIndex()
{
    logChecker("CheckBufferOverrun::arrayIndex");

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "[")
            continue;
        const Token *array = tok->astOperand1();
        while (Token::Match(array, ".|::"))
            array = array->astOperand2();
        if (!array || ((!array->variable() || array->variable()->nameToken() == array) && array->tokType() != Token::eString))
            continue;
        if (!array->scope()->isExecutable()) {
            // LHS in non-executable scope => This is just a definition
            const Token *parent = tok;
            while (parent && !Token::simpleMatch(parent->astParent(), "="))
                parent = parent->astParent();
            if (!parent || parent == parent->astParent()->astOperand1())
                continue;
        }

        if (astIsContainer(array))
            continue;

        std::vector<const Token *> indexTokens;
        for (const Token *tok2 = tok; tok2 && tok2->str() == "["; tok2 = tok2->link()->next()) {
            if (!tok2->astOperand2()) {
                indexTokens.clear();
                break;
            }
            indexTokens.emplace_back(tok2->astOperand2());
        }
        if (indexTokens.empty())
            continue;

        std::vector<Dimension> dimensions;
        ErrorPath errorPath;
        bool mightBeLarger = false;
        MathLib::bigint path = 0;
        if (!getDimensionsEtc(tok->astOperand1(), *mSettings, dimensions, errorPath, mightBeLarger, path))
            continue;

        const Variable* const var = array->variable();
        if (var && var->isArgument() && var->scope()) {
            const Token* changeTok = var->scope()->bodyStart;
            bool isChanged = false;
            while ((changeTok = findVariableChanged(changeTok->next(), var->scope()->bodyEnd, /*indirect*/ 0, var->declarationId(),
                                                    /*globalvar*/ false, *mSettings))) {
                if (!Token::simpleMatch(changeTok->astParent(), "[")) {
                    isChanged = true;
                    break;
                }
            }
            if (isChanged)
                continue;
        }

        // Positive index
        if (!mightBeLarger) { // TODO check arrays with dim 1 also
            const std::vector<ValueFlow::Value>& indexValues =
                getOverrunIndexValues(tok, tok->astOperand1(), dimensions, indexTokens, path);
            if (!indexValues.empty())
                arrayIndexError(tok, dimensions, indexValues);
        }

        // Negative index
        bool neg = false;
        std::vector<ValueFlow::Value> negativeIndexes;
        for (const Token * indexToken : indexTokens) {
            const ValueFlow::Value *negativeValue = indexToken->getValueLE(-1, *mSettings);
            if (negativeValue) {
                negativeIndexes.emplace_back(*negativeValue);
                neg = true;
            } else {
                negativeIndexes.emplace_back(ValueFlow::Value::unknown());
            }
        }
        if (neg) {
            negativeIndexError(tok, dimensions, negativeIndexes);
        }
    }
}

static std::string stringifyIndexes(const std::string& array, const std::vector<ValueFlow::Value>& indexValues)
{
    if (indexValues.size() == 1)
        return MathLib::toString(indexValues[0].intvalue);

    std::ostringstream ret;
    ret << array;
    for (const ValueFlow::Value& index : indexValues) {
        ret << "[";
        if (index.isNonValue())
            ret << "*";
        else
            ret << index.intvalue;
        ret << "]";
    }
    return ret.str();
}

static std::string arrayIndexMessage(const Token* tok,
                                     const std::vector<Dimension>& dimensions,
                                     const std::vector<ValueFlow::Value>& indexValues,
                                     const Token* condition)
{
    auto add_dim = [](const std::string &s, const Dimension &dim) {
        return s + "[" + MathLib::toString(dim.num) + "]";
    };
    const std::string array = std::accumulate(dimensions.cbegin(), dimensions.cend(), tok->astOperand1()->expressionString(), std::move(add_dim));

    std::ostringstream errmsg;
    if (condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(condition)
               << " or the array '" << array << "' is accessed at index " << stringifyIndexes(tok->astOperand1()->expressionString(), indexValues) << ", which is out of bounds.";
    else
        errmsg << "Array '" << array << "' accessed at index " << stringifyIndexes(tok->astOperand1()->expressionString(), indexValues) <<  ", which is out of bounds.";

    return errmsg.str();
}

void CheckBufferOverrun::arrayIndexError(const Token* tok,
                                         const std::vector<Dimension>& dimensions,
                                         const std::vector<ValueFlow::Value>& indexes)
{
    if (!tok) {
        reportError(tok, Severity::error, "arrayIndexOutOfBounds", "Array 'arr[16]' accessed at index 16, which is out of bounds.", CWE_BUFFER_OVERRUN, Certainty::normal);
        reportError(tok, Severity::warning, "arrayIndexOutOfBoundsCond", "Array 'arr[16]' accessed at index 16, which is out of bounds.", CWE_BUFFER_OVERRUN, Certainty::normal);
        return;
    }

    const Token *condition = nullptr;
    const ValueFlow::Value *index = nullptr;
    for (const ValueFlow::Value& indexValue : indexes) {
        if (!indexValue.errorSeverity() && !mSettings->severity.isEnabled(Severity::warning))
            return;
        if (indexValue.condition)
            condition = indexValue.condition;
        if (!index || !indexValue.errorPath.empty())
            index = &indexValue;
    }

    reportError(getErrorPath(tok, index, "Array index out of bounds"),
                index->errorSeverity() ? Severity::error : Severity::warning,
                index->condition ? "arrayIndexOutOfBoundsCond" : "arrayIndexOutOfBounds",
                arrayIndexMessage(tok, dimensions, indexes, condition),
                CWE_BUFFER_OVERRUN,
                index->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

void CheckBufferOverrun::negativeIndexError(const Token* tok,
                                            const std::vector<Dimension>& dimensions,
                                            const std::vector<ValueFlow::Value>& indexes)
{
    if (!tok) {
        reportError(tok, Severity::error, "negativeIndex", "Negative array index", CWE_BUFFER_UNDERRUN, Certainty::normal);
        return;
    }

    const Token *condition = nullptr;
    const ValueFlow::Value *negativeValue = nullptr;
    for (const ValueFlow::Value& indexValue : indexes) {
        if (!indexValue.errorSeverity() && !mSettings->severity.isEnabled(Severity::warning))
            return;
        if (indexValue.condition)
            condition = indexValue.condition;
        if (!negativeValue || !indexValue.errorPath.empty())
            negativeValue = &indexValue;
    }

    reportError(getErrorPath(tok, negativeValue, "Negative array index"),
                negativeValue->errorSeverity() ? Severity::error : Severity::warning,
                "negativeIndex",
                arrayIndexMessage(tok, dimensions, indexes, condition),
                CWE_BUFFER_UNDERRUN,
                negativeValue->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::pointerArithmetic()
{
    if (!mSettings->severity.isEnabled(Severity::portability) && !mSettings->isPremiumEnabled("pointerOutOfBounds"))
        return;

    logChecker("CheckBufferOverrun::pointerArithmetic"); // portability

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "+|-"))
            continue;
        if (!tok->valueType() || tok->valueType()->pointer == 0)
            continue;
        if (!tok->isBinaryOp())
            continue;
        if (!tok->astOperand1()->valueType() || !tok->astOperand2()->valueType())
            continue;

        const Token *arrayToken, *indexToken;
        if (tok->astOperand1()->valueType()->pointer > 0) {
            arrayToken = tok->astOperand1();
            indexToken = tok->astOperand2();
        } else {
            arrayToken = tok->astOperand2();
            indexToken = tok->astOperand1();
        }

        if (!indexToken || !indexToken->valueType() || indexToken->valueType()->pointer > 0 || !indexToken->valueType()->isIntegral())
            continue;

        std::vector<Dimension> dimensions;
        ErrorPath errorPath;
        bool mightBeLarger = false;
        MathLib::bigint path = 0;
        if (!getDimensionsEtc(arrayToken, *mSettings, dimensions, errorPath, mightBeLarger, path))
            continue;

        if (tok->str() == "+") {
            // Positive index
            if (!mightBeLarger) { // TODO check arrays with dim 1 also
                const std::vector<const Token *> indexTokens{indexToken};
                const std::vector<ValueFlow::Value>& indexValues =
                    getOverrunIndexValues(tok, arrayToken, dimensions, indexTokens, path);
                if (!indexValues.empty() && !isUnreachableOperand(tok))
                    pointerArithmeticError(tok, indexToken, &indexValues.front());
            }

            if (const ValueFlow::Value *neg = indexToken->getValueLE(-1, *mSettings))
                pointerArithmeticError(tok, indexToken, neg);
        } else if (tok->str() == "-") {
            if (arrayToken->variable() && arrayToken->variable()->isArgument())
                continue;

            const Token *array = arrayToken;
            while (Token::Match(array, ".|::"))
                array = array->astOperand2();
            if (array->variable() && array->variable()->isArray()) {
                const ValueFlow::Value *v = indexToken->getValueGE(1, *mSettings);
                if (v)
                    pointerArithmeticError(tok, indexToken, v);
            }
        }
    }
}

void CheckBufferOverrun::pointerArithmeticError(const Token *tok, const Token *indexToken, const ValueFlow::Value *indexValue)
{
    if (!tok) {
        reportError(tok, Severity::portability, "pointerOutOfBounds", "Pointer arithmetic overflow.", CWE_POINTER_ARITHMETIC_OVERFLOW, Certainty::normal);
        reportError(tok, Severity::portability, "pointerOutOfBoundsCond", "Pointer arithmetic overflow.", CWE_POINTER_ARITHMETIC_OVERFLOW, Certainty::normal);
        return;
    }

    std::string errmsg;
    if (indexValue->condition)
        errmsg = "Undefined behaviour, when '" + indexToken->expressionString() + "' is " + MathLib::toString(indexValue->intvalue) + " the pointer arithmetic '" + tok->expressionString() + "' is out of bounds.";
    else
        errmsg = "Undefined behaviour, pointer arithmetic '" + tok->expressionString() + "' is out of bounds.";

    reportError(getErrorPath(tok, indexValue, "Pointer arithmetic overflow"),
                Severity::portability,
                indexValue->condition ? "pointerOutOfBoundsCond" : "pointerOutOfBounds",
                errmsg,
                CWE_POINTER_ARITHMETIC_OVERFLOW,
                indexValue->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------

ValueFlow::Value CheckBufferOverrun::getBufferSize(const Token *bufTok) const
{
    if (!bufTok->valueType())
        return ValueFlow::Value(-1);
    if (bufTok->isUnaryOp("&"))
        bufTok = bufTok->astOperand1();
    const Variable *var = bufTok->variable();

    if (!var || var->dimensions().empty()) {
        const ValueFlow::Value *value = getBufferSizeValue(bufTok);
        if (value)
            return *value;
    }

    if (!var || var->isPointer())
        return ValueFlow::Value(-1);

    const MathLib::bigint dim = std::accumulate(var->dimensions().cbegin(), var->dimensions().cend(), MathLib::bigint(1), [](MathLib::bigint i1, const Dimension &dim) {
        return i1 * dim.num;
    });

    ValueFlow::Value v;
    v.setKnown();
    v.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;

    if (var->isPointerArray())
        v.intvalue = dim * mSettings->platform.sizeof_pointer;
    else {
        const size_t typeSize = bufTok->valueType()->getSizeOf(*mSettings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
        v.intvalue = dim * typeSize;
    }

    return v;
}
//---------------------------------------------------------------------------

static bool checkBufferSize(const Token *ftok, const Library::ArgumentChecks::MinSize &minsize, const std::vector<const Token *> &args, const MathLib::bigint bufferSize, const Settings &settings, const Tokenizer* tokenizer)
{
    const Token * const arg = (minsize.arg > 0 && minsize.arg - 1 < args.size()) ? args[minsize.arg - 1] : nullptr;
    const Token * const arg2 = (minsize.arg2 > 0 && minsize.arg2 - 1 < args.size()) ? args[minsize.arg2 - 1] : nullptr;

    switch (minsize.type) {
    case Library::ArgumentChecks::MinSize::Type::STRLEN:
        if (settings.library.isargformatstr(ftok, minsize.arg)) {
            return getMinFormatStringOutputLength(args, minsize.arg) < bufferSize;
        } else if (arg) {
            const Token *strtoken = arg->getValueTokenMaxStrLength();
            if (strtoken)
                return Token::getStrLength(strtoken) < bufferSize;
        }
        break;
    case Library::ArgumentChecks::MinSize::Type::ARGVALUE: {
        if (arg && arg->hasKnownIntValue()) {
            MathLib::bigint myMinsize = arg->getKnownIntValue();
            const int baseSize = tokenizer->sizeOfType(minsize.baseType);
            if (baseSize != 0)
                myMinsize *= baseSize;
            return myMinsize <= bufferSize;
        }
        break;
    }
    case Library::ArgumentChecks::MinSize::Type::SIZEOF:
        // TODO
        break;
    case Library::ArgumentChecks::MinSize::Type::MUL:
        if (arg && arg2 && arg->hasKnownIntValue() && arg2->hasKnownIntValue())
            return (arg->getKnownIntValue() * arg2->getKnownIntValue()) <= bufferSize;
        break;
    case Library::ArgumentChecks::MinSize::Type::VALUE: {
        MathLib::bigint myMinsize = minsize.value;
        const int baseSize = tokenizer->sizeOfType(minsize.baseType);
        if (baseSize != 0)
            myMinsize *= baseSize;
        return myMinsize <= bufferSize;
    }
    case Library::ArgumentChecks::MinSize::Type::NONE:
        break;
    }
    return true;
}


void CheckBufferOverrun::bufferOverflow()
{
    logChecker("CheckBufferOverrun::bufferOverflow");

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% (") || Token::simpleMatch(tok, ") {"))
                continue;
            if (!mSettings->library.hasminsize(tok))
                continue;
            const std::vector<const Token *> args = getArguments(tok);
            for (int argnr = 0; argnr < args.size(); ++argnr) {
                if (!args[argnr]->valueType() || args[argnr]->valueType()->pointer == 0)
                    continue;
                const std::vector<Library::ArgumentChecks::MinSize> *minsizes = mSettings->library.argminsizes(tok, argnr + 1);
                if (!minsizes || minsizes->empty())
                    continue;
                // Get buffer size..
                const Token *argtok = args[argnr];
                while (argtok && argtok->isCast())
                    argtok = argtok->astOperand2() ? argtok->astOperand2() : argtok->astOperand1();
                while (Token::Match(argtok, ".|::"))
                    argtok = argtok->astOperand2();
                if (!argtok)
                    continue;
                if (argtok->valueType() && argtok->valueType()->pointer == 0)
                    continue;
                // TODO: strcpy(buf+10, "hello");
                const ValueFlow::Value bufferSize = getBufferSize(argtok);
                if (bufferSize.intvalue <= 0)
                    continue;
                // buffer size == 1 => do not warn for dynamic memory
                if (bufferSize.intvalue == 1 && Token::simpleMatch(argtok->astParent(), ".")) { // TODO: check if parent was allocated dynamically
                    const Token *tok2 = argtok;
                    while (Token::simpleMatch(tok2->astParent(), "."))
                        tok2 = tok2->astParent();
                    while (Token::Match(tok2, "[|."))
                        tok2 = tok2->astOperand1();
                    const Variable *var = tok2 ? tok2->variable() : nullptr;
                    if (var) {
                        if (var->isPointer())
                            continue;
                        if (var->isArgument() && var->isReference())
                            continue;
                    }
                }
                const bool error = std::none_of(minsizes->begin(), minsizes->end(), [&](const Library::ArgumentChecks::MinSize &minsize) {
                    return checkBufferSize(tok, minsize, args, bufferSize.intvalue, *mSettings, mTokenizer);
                });
                if (error)
                    bufferOverflowError(args[argnr], &bufferSize, Certainty::normal);
            }
        }
    }
}

void CheckBufferOverrun::bufferOverflowError(const Token *tok, const ValueFlow::Value *value, Certainty certainty)
{
    reportError(getErrorPath(tok, value, "Buffer overrun"), Severity::error, "bufferAccessOutOfBounds", "Buffer is accessed out of bounds: " + (tok ? getRealBufferTok(tok)->expressionString() : "buf"), CWE_BUFFER_OVERRUN, certainty);
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::arrayIndexThenCheck()
{
    if (!mSettings->severity.isEnabled(Severity::style))
        return;

    logChecker("CheckBufferOverrun::arrayIndexThenCheck"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * const scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::simpleMatch(tok, "sizeof (")) {
                tok = tok->linkAt(1);
                continue;
            }

            if (Token::Match(tok, "%name% [ %var% ]")) {
                tok = tok->next();

                const int indexID = tok->next()->varId();
                const std::string& indexName(tok->strAt(1));

                // Iterate AST upwards
                const Token* tok2 = tok;
                const Token* tok3 = tok2;
                while (tok2->astParent() && tok2->tokType() != Token::eLogicalOp && tok2->str() != "?") {
                    tok3 = tok2;
                    tok2 = tok2->astParent();
                }

                // Ensure that we ended at a logical operator and that we came from its left side
                if (tok2->tokType() != Token::eLogicalOp || tok2->astOperand1() != tok3)
                    continue;

                // check if array index is ok
                // statement can be closed in parentheses, so "(| " is using
                if (Token::Match(tok2, "&& (| %varid% <|<=", indexID))
                    arrayIndexThenCheckError(tok, indexName);
                else if (Token::Match(tok2, "&& (| %any% >|>= %varid% !!+", indexID))
                    arrayIndexThenCheckError(tok, indexName);
            }
        }
    }
}

void CheckBufferOverrun::arrayIndexThenCheckError(const Token *tok, const std::string &indexName)
{
    reportError(tok, Severity::style, "arrayIndexThenCheck",
                "$symbol:" + indexName + "\n"
                "Array index '$symbol' is used before limits check.\n"
                "Defensive programming: The variable '$symbol' is used as an array index before it "
                "is checked that is within limits. This can mean that the array might be accessed out of bounds. "
                "Reorder conditions such as '(a[i] && i < 10)' to '(i < 10 && a[i])'. That way the array will "
                "not be accessed if the index is out of limits.", CWE_ARRAY_INDEX_THEN_CHECK, Certainty::normal);
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::stringNotZeroTerminated()
{
    // this is currently 'inconclusive'. See TestBufferOverrun::terminateStrncpy3
    if (!mSettings->severity.isEnabled(Severity::warning) || !mSettings->certainty.isEnabled(Certainty::inconclusive))
        return;

    logChecker("CheckBufferOverrun::stringNotZeroTerminated"); // warning,inconclusive

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * const scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "strncpy ("))
                continue;
            const std::vector<const Token *> args = getArguments(tok);
            if (args.size() != 3)
                continue;
            const Token *sizeToken = args[2];
            if (!sizeToken->hasKnownIntValue())
                continue;
            const ValueFlow::Value &bufferSize = getBufferSize(args[0]);
            if (bufferSize.intvalue < 0 || sizeToken->getKnownIntValue() < bufferSize.intvalue)
                continue;
            if (Token::simpleMatch(args[1], "(") && Token::simpleMatch(args[1]->astOperand1(), ". c_str") && args[1]->astOperand1()->astOperand1()) {
                const std::list<ValueFlow::Value>& contValues = args[1]->astOperand1()->astOperand1()->values();
                auto it = std::find_if(contValues.cbegin(), contValues.cend(), [](const ValueFlow::Value& value) {
                    return value.isContainerSizeValue() && !value.isImpossible();
                });
                if (it != contValues.end() && it->intvalue < sizeToken->getKnownIntValue())
                    continue;
            } else {
                const Token* srcValue = args[1]->getValueTokenMaxStrLength();
                if (srcValue && Token::getStrLength(srcValue) < sizeToken->getKnownIntValue())
                    continue;
            }
            // Is the buffer zero terminated after the call?
            bool isZeroTerminated = false;
            for (const Token *tok2 = tok->linkAt(1); tok2 != scope->bodyEnd; tok2 = tok2->next()) {
                if (!Token::simpleMatch(tok2, "] ="))
                    continue;
                const Token *rhs = tok2->next()->astOperand2();
                if (!rhs || !rhs->hasKnownIntValue() || rhs->getKnownIntValue() != 0)
                    continue;
                if (isSameExpression(false, args[0], tok2->link()->astOperand1(), *mSettings, false, false))
                    isZeroTerminated = true;
            }
            if (isZeroTerminated)
                continue;
            // TODO: Locate unsafe string usage..
            terminateStrncpyError(tok, getRealBufferTok(args[0])->expressionString());
        }
    }
}

void CheckBufferOverrun::terminateStrncpyError(const Token *tok, const std::string &varname)
{
    const std::string shortMessage = "The buffer '$symbol' may not be null-terminated after the call to strncpy().";
    reportError(tok, Severity::warning, "terminateStrncpy",
                "$symbol:" + varname + '\n' +
                shortMessage + '\n' +
                shortMessage + ' ' +
                "If the source string's size fits or exceeds the given size, strncpy() does not add a "
                "zero at the end of the buffer. This causes bugs later in the code if the code "
                "assumes buffer is null-terminated.", CWE170, Certainty::inconclusive);
}
//---------------------------------------------------------------------------

void CheckBufferOverrun::argumentSize()
{
    // Check '%type% x[10]' arguments
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("argumentSize"))
        return;

    logChecker("CheckBufferOverrun::argumentSize"); // warning

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * const scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->function() || !Token::Match(tok, "%name% ("))
                continue;

            // If argument is '%type% a[num]' then check bounds against num
            const Function *callfunc = tok->function();
            const std::vector<const Token *> callargs = getArguments(tok);
            for (nonneg int paramIndex = 0; paramIndex < callargs.size() && paramIndex < callfunc->argCount(); ++paramIndex) {
                const Variable* const argument = callfunc->getArgumentVar(paramIndex);
                if (!argument || !argument->nameToken() || !argument->isArray())
                    continue;
                if (!argument->valueType() || !callargs[paramIndex]->valueType())
                    continue;
                if (argument->valueType()->type != callargs[paramIndex]->valueType()->type)
                    continue;
                const Token * calldata = callargs[paramIndex];
                while (Token::Match(calldata, "::|."))
                    calldata = calldata->astOperand2();
                if (!calldata->variable() || !calldata->variable()->isArray())
                    continue;
                if (calldata->variable()->dimensions().size() != argument->dimensions().size())
                    continue;
                bool err = false;
                for (std::size_t d = 0; d < argument->dimensions().size(); ++d) {
                    const auto& dim1 = calldata->variable()->dimensions()[d];
                    const auto& dim2 = argument->dimensions()[d];
                    if (!dim1.known || !dim2.known)
                        break;
                    if (dim1.num < dim2.num)
                        err = true;
                }
                if (err)
                    argumentSizeError(tok, tok->str(), paramIndex, callargs[paramIndex]->expressionString(), calldata->variable(), argument);
            }
        }
    }
}

void CheckBufferOverrun::argumentSizeError(const Token *tok, const std::string &functionName, nonneg int paramIndex, const std::string &paramExpression, const Variable *paramVar, const Variable *functionArg)
{
    const std::string strParamNum = std::to_string(paramIndex + 1) + getOrdinalText(paramIndex + 1);
    ErrorPath errorPath;
    errorPath.emplace_back(tok, "Function '" + functionName + "' is called");
    if (functionArg)
        errorPath.emplace_back(functionArg->nameToken(), "Declaration of " + strParamNum + " function argument.");
    if (paramVar)
        errorPath.emplace_back(paramVar->nameToken(), "Passing buffer '" + paramVar->name() + "' to function that is declared here");
    errorPath.emplace_back(tok, "");

    reportError(std::move(errorPath), Severity::warning, "argumentSize",
                "$symbol:" + functionName + '\n' +
                "Buffer '" + paramExpression + "' is too small, the function '" + functionName + "' expects a bigger buffer in " + strParamNum + " argument", CWE_ARGUMENT_SIZE, Certainty::normal);
}

//---------------------------------------------------------------------------
// CTU..
//---------------------------------------------------------------------------

// a Clang-built executable will crash when using the anonymous MyFileInfo later on - so put it in a unique namespace for now
// see https://trac.cppcheck.net/ticket/12108 for more details
#ifdef __clang__
inline namespace CheckBufferOverrun_internal
#else
namespace
#endif
{
    /** data for multifile checking */
    class MyFileInfo : public Check::FileInfo {
    public:
        using Check::FileInfo::FileInfo;
        /** unsafe array index usage */
        std::list<CTU::FileInfo::UnsafeUsage> unsafeArrayIndex;

        /** unsafe pointer arithmetics */
        std::list<CTU::FileInfo::UnsafeUsage> unsafePointerArith;

        /** Convert data into xml string */
        std::string toString() const override
        {
            std::string xml;
            if (!unsafeArrayIndex.empty())
                xml = "    <array-index>\n" + CTU::toString(unsafeArrayIndex) + "    </array-index>\n";
            if (!unsafePointerArith.empty())
                xml += "    <pointer-arith>\n" + CTU::toString(unsafePointerArith) + "    </pointer-arith>\n";
            return xml;
        }
    };
}

bool CheckBufferOverrun::isCtuUnsafeBufferUsage(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *offset, int type)
{
    if (!offset)
        return false;
    if (!argtok->valueType() || argtok->valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee) == 0)
        return false;
    const Token *indexTok = nullptr;
    if (type == 1 && Token::Match(argtok, "%name% [") && argtok->astParent() == argtok->next() && !Token::simpleMatch(argtok->linkAt(1), "] ["))
        indexTok = argtok->next()->astOperand2();
    else if (type == 2 && Token::simpleMatch(argtok->astParent(), "+"))
        indexTok = (argtok == argtok->astParent()->astOperand1()) ?
                   argtok->astParent()->astOperand2() :
                   argtok->astParent()->astOperand1();
    if (!indexTok)
        return false;
    if (!indexTok->hasKnownIntValue())
        return false;
    offset->value = indexTok->getKnownIntValue() * argtok->valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
    return true;
}

bool CheckBufferOverrun::isCtuUnsafeArrayIndex(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *offset)
{
    return isCtuUnsafeBufferUsage(settings, argtok, offset, 1);
}

bool CheckBufferOverrun::isCtuUnsafePointerArith(const Settings &settings, const Token *argtok, CTU::FileInfo::Value* offset)
{
    return isCtuUnsafeBufferUsage(settings, argtok, offset, 2);
}

/** @brief Parse current TU and extract file info */
Check::FileInfo *CheckBufferOverrun::getFileInfo(const Tokenizer &tokenizer, const Settings &settings, const std::string& /*currentConfig*/) const
{
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeArrayIndex = CTU::getUnsafeUsage(tokenizer, settings, isCtuUnsafeArrayIndex);
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafePointerArith = CTU::getUnsafeUsage(tokenizer, settings, isCtuUnsafePointerArith);
    if (unsafeArrayIndex.empty() && unsafePointerArith.empty()) {
        return nullptr;
    }
    auto *fileInfo = new MyFileInfo(tokenizer.list.getFiles()[0]);
    fileInfo->unsafeArrayIndex = unsafeArrayIndex;
    fileInfo->unsafePointerArith = unsafePointerArith;
    return fileInfo;
}

Check::FileInfo * CheckBufferOverrun::loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const
{
    // cppcheck-suppress shadowFunction - TODO: fix this
    const std::string arrayIndex("array-index");
    const std::string pointerArith("pointer-arith");

    auto *fileInfo = new MyFileInfo;
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        const char* name = e->Name();
        if (name == arrayIndex)
            fileInfo->unsafeArrayIndex = CTU::loadUnsafeUsageListFromXml(e);
        else if (name == pointerArith)
            fileInfo->unsafePointerArith = CTU::loadUnsafeUsageListFromXml(e);
    }

    if (fileInfo->unsafeArrayIndex.empty() && fileInfo->unsafePointerArith.empty()) {
        delete fileInfo;
        return nullptr;
    }

    return fileInfo;
}

/** @brief Analyse all file infos for all TU */
bool CheckBufferOverrun::analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    CheckBufferOverrun dummy(nullptr, &settings, &errorLogger);
    dummy.
    logChecker("CheckBufferOverrun::analyseWholeProgram");

    if (fileInfo.empty())
        return false;

    const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> callsMap = ctu.getCallsMap();

    bool foundErrors = false;

    for (const Check::FileInfo* fi1 : fileInfo) {
        const auto *fi = dynamic_cast<const MyFileInfo*>(fi1);
        if (!fi)
            continue;
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafeArrayIndex)
            foundErrors |= analyseWholeProgram1(callsMap, unsafeUsage, 1, errorLogger, settings.maxCtuDepth, fi->file0);
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafePointerArith)
            foundErrors |= analyseWholeProgram1(callsMap, unsafeUsage, 2, errorLogger, settings.maxCtuDepth, fi->file0);
    }
    return foundErrors;
}

bool CheckBufferOverrun::analyseWholeProgram1(const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap, const CTU::FileInfo::UnsafeUsage &unsafeUsage,
                                              int type, ErrorLogger &errorLogger, int maxCtuDepth, const std::string& file0)
{
    const CTU::FileInfo::FunctionCall *functionCall = nullptr;

    std::list<ErrorMessage::FileLocation> locationList =
        CTU::FileInfo::getErrorPath(CTU::FileInfo::InvalidValueType::bufferOverflow,
                                    unsafeUsage,
                                    callsMap,
                                    "Using argument ARG",
                                    &functionCall,
                                    false,
                                    maxCtuDepth);
    if (locationList.empty())
        return false;

    const char *errorId = nullptr;
    std::string errmsg;
    CWE cwe(0);

    if (type == 1) {
        errorId = "ctuArrayIndex";
        if (unsafeUsage.value > 0)
            errmsg = "Array index out of bounds; '" + unsafeUsage.myArgumentName + "' buffer size is " + MathLib::toString(functionCall->callArgValue.value) + " and it is accessed at offset " + MathLib::toString(unsafeUsage.value) + ".";
        else
            errmsg = "Array index out of bounds; buffer '" + unsafeUsage.myArgumentName + "' is accessed at offset " + MathLib::toString(unsafeUsage.value) + ".";
        cwe = (unsafeUsage.value > 0) ? CWE_BUFFER_OVERRUN : CWE_BUFFER_UNDERRUN;
    } else {
        errorId = "ctuPointerArith";
        errmsg = "Pointer arithmetic overflow; '" + unsafeUsage.myArgumentName + "' buffer size is " + MathLib::toString(functionCall->callArgValue.value);
        cwe = CWE_POINTER_ARITHMETIC_OVERFLOW;
    }

    const ErrorMessage errorMessage(std::move(locationList),
                                    file0,
                                    Severity::error,
                                    errmsg,
                                    errorId,
                                    cwe, Certainty::normal);
    errorLogger.reportErr(errorMessage);

    return true;
}

void CheckBufferOverrun::objectIndex()
{
    logChecker("CheckBufferOverrun::objectIndex");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "["))
                continue;
            const Token *obj = tok->astOperand1();
            const Token *idx = tok->astOperand2();
            if (!idx || !obj)
                continue;
            if (const ValueFlow::Value* v = idx->getKnownValue(ValueFlow::Value::ValueType::INT)) {
                if (v->intvalue == 0)
                    continue;
            }

            std::vector<ValueFlow::Value> values = ValueFlow::getLifetimeObjValues(obj, false, -1);
            for (const ValueFlow::Value& v:values) {
                if (v.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
                    continue;
                const Variable *var = v.tokvalue->variable();
                if (!var)
                    continue;
                if (var->isReference())
                    continue;
                if (var->isRValueReference())
                    continue;
                if (var->isArray())
                    continue;
                if (var->isPointer()) {
                    if (!var->valueType())
                        continue;
                    if (!obj->valueType())
                        continue;
                    if (var->valueType()->pointer > obj->valueType()->pointer)
                        continue;
                }
                if (obj->valueType() && var->valueType() && (obj->isCast() || (obj->isCpp() && isCPPCast(obj)) || obj->valueType()->pointer)) { // allow cast to a different type
                    const auto varSize = var->valueType()->getSizeOf(*mSettings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
                    if (varSize == 0)
                        continue;
                    if (obj->valueType()->type != var->valueType()->type) {
                        if (ValueFlow::isOutOfBounds(makeSizeValue(varSize, v.path), idx).empty())
                            continue;
                    }
                }
                if (v.path != 0) {
                    std::vector<ValueFlow::Value> idxValues;
                    std::copy_if(idx->values().cbegin(),
                                 idx->values().cend(),
                                 std::back_inserter(idxValues),
                                 [&](const ValueFlow::Value& vidx) {
                        if (!vidx.isIntValue())
                            return false;
                        return vidx.path == v.path || vidx.path == 0;
                    });
                    if (std::any_of(idxValues.cbegin(), idxValues.cend(), [&](const ValueFlow::Value& vidx) {
                        if (vidx.isImpossible())
                            return (vidx.intvalue == 0);
                        return (vidx.intvalue != 0);
                    })) {
                        objectIndexError(tok, &v, idx->hasKnownIntValue());
                    }
                } else {
                    objectIndexError(tok, &v, idx->hasKnownIntValue());
                }
            }
        }
    }
}

void CheckBufferOverrun::objectIndexError(const Token *tok, const ValueFlow::Value *v, bool known)
{
    ErrorPath errorPath;
    std::string name;
    if (v) {
        const Token* expr = v->tokvalue;
        while (Token::simpleMatch(expr->astParent(), "."))
            expr = expr->astParent();
        name = expr->expressionString();
        errorPath = v->errorPath;
    }
    errorPath.emplace_back(tok, "");
    std::string verb = known ? "is" : "might be";
    reportError(std::move(errorPath),
                known ? Severity::error : Severity::warning,
                "objectIndex",
                "The address of variable '" + name + "' " + verb + " accessed at non-zero index.",
                CWE758,
                Certainty::normal);
}

static bool isVLAIndex(const Token* tok)
{
    if (!tok)
        return false;
    if (tok->varId() != 0U)
        return true;
    if (tok->str() == "?") {
        // this is a VLA index if both expressions around the ":" is VLA index
        return tok->astOperand2() &&
               tok->astOperand2()->str() == ":" &&
               isVLAIndex(tok->astOperand2()->astOperand1()) &&
               isVLAIndex(tok->astOperand2()->astOperand2());
    }
    return isVLAIndex(tok->astOperand1()) || isVLAIndex(tok->astOperand2());
}

void CheckBufferOverrun::negativeArraySize()
{
    logChecker("CheckBufferOverrun::negativeArraySize");
    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->isArray())
            continue;
        const Token* const nameToken = var->nameToken();
        if (!Token::Match(nameToken, "%var% [") || !nameToken->next()->astOperand2())
            continue;
        const ValueFlow::Value* sz = nameToken->next()->astOperand2()->getValueLE(-1, *mSettings);
        // don't warn about constant negative index because that is a compiler error
        if (sz && isVLAIndex(nameToken->next()->astOperand2()))
            negativeArraySizeError(nameToken);
    }

    for (const Scope* functionScope : symbolDatabase->functionScopes) {
        for (const Token* tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!tok->isKeyword() || tok->str() != "new" || !tok->astOperand1() || tok->astOperand1()->str() != "[")
                continue;
            const Token* valOperand = tok->astOperand1()->astOperand2();
            if (!valOperand)
                continue;
            const ValueFlow::Value* sz = valOperand->getValueLE(-1, *mSettings);
            if (sz)
                negativeMemoryAllocationSizeError(tok, sz);
        }
    }
}

void CheckBufferOverrun::negativeArraySizeError(const Token* tok)
{
    const std::string arrayName = tok ? tok->expressionString() : std::string();
    const std::string line1 = arrayName.empty() ? std::string() : ("$symbol:" + arrayName + '\n');
    reportError(tok, Severity::error, "negativeArraySize",
                line1 +
                "Declaration of array '" + arrayName + "' with negative size is undefined behaviour", CWE758, Certainty::normal);
}

void CheckBufferOverrun::negativeMemoryAllocationSizeError(const Token* tok, const ValueFlow::Value* value)
{
    const std::string msg = "Memory allocation size is negative.";
    ErrorPath errorPath = getErrorPath(tok, value, msg);
    const bool inconclusive = value != nullptr && !value->isKnown();
    reportError(std::move(errorPath), inconclusive ? Severity::warning : Severity::error, "negativeMemoryAllocationSize",
                msg, CWE131, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckBufferOverrun::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckBufferOverrun checkBufferOverrun(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkBufferOverrun.arrayIndex();
    checkBufferOverrun.pointerArithmetic();
    checkBufferOverrun.bufferOverflow();
    checkBufferOverrun.arrayIndexThenCheck();
    checkBufferOverrun.stringNotZeroTerminated();
    checkBufferOverrun.objectIndex();
    checkBufferOverrun.argumentSize();
    checkBufferOverrun.negativeArraySize();
}

void CheckBufferOverrun::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckBufferOverrun c(nullptr, settings, errorLogger);
    c.arrayIndexError(nullptr, std::vector<Dimension>(), std::vector<ValueFlow::Value>());
    c.pointerArithmeticError(nullptr, nullptr, nullptr);
    c.negativeIndexError(nullptr, std::vector<Dimension>(), std::vector<ValueFlow::Value>());
    c.arrayIndexThenCheckError(nullptr, "i");
    c.bufferOverflowError(nullptr, nullptr, Certainty::normal);
    c.objectIndexError(nullptr, nullptr, true);
    c.argumentSizeError(nullptr, "function", 1, "buffer", nullptr, nullptr);
    c.negativeMemoryAllocationSizeError(nullptr, nullptr);
    c.negativeArraySizeError(nullptr);
    c.terminateStrncpyError(nullptr, "var_name");
    // TODO: ctuArrayIndex
    // TODO: ctuPointerArith
}
