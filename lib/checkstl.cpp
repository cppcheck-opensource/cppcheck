/*
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

#include "checkstl.h"

#include "astutils.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "pathanalysis.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"
#include "valueflow.h"
#include "vfvalue.h"

#include "checknullpointer.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// Register this check class (by creating a static instance of it)
namespace {
    CheckStl instance;
}

// CWE IDs used:
static const CWE CWE398(398U);   // Indicator of Poor Code Quality
static const CWE CWE597(597U);   // Use of Wrong Operator in String Comparison
static const CWE CWE628(628U);   // Function Call with Incorrectly Specified Arguments
static const CWE CWE664(664U);   // Improper Control of a Resource Through its Lifetime
static const CWE CWE667(667U);   // Improper Locking
static const CWE CWE704(704U);   // Incorrect Type Conversion or Cast
static const CWE CWE762(762U);   // Mismatched Memory Management Routines
static const CWE CWE786(786U);   // Access of Memory Location Before Start of Buffer
static const CWE CWE788(788U);   // Access of Memory Location After End of Buffer
static const CWE CWE825(825U);   // Expired Pointer Dereference
static const CWE CWE833(833U);   // Deadlock
static const CWE CWE834(834U);   // Excessive Iteration

static bool isElementAccessYield(Library::Container::Yield yield)
{
    return contains({Library::Container::Yield::ITEM, Library::Container::Yield::AT_INDEX}, yield);
}

static bool containerAppendsElement(const Library::Container* container, const Token* parent)
{
    if (Token::Match(parent, ". %name% (")) {
        const Library::Container::Action action = container->getAction(parent->strAt(1));
        if (contains({Library::Container::Action::INSERT,
                      Library::Container::Action::APPEND,
                      Library::Container::Action::CHANGE,
                      Library::Container::Action::CHANGE_INTERNAL,
                      Library::Container::Action::PUSH,
                      Library::Container::Action::RESIZE},
                     action))
            return true;
    }
    return false;
}

static bool containerYieldsElement(const Library::Container* container, const Token* parent)
{
    if (Token::Match(parent, ". %name% (")) {
        const Library::Container::Yield yield = container->getYield(parent->strAt(1));
        if (isElementAccessYield(yield))
            return true;
    }
    return false;
}

static bool containerPopsElement(const Library::Container* container, const Token* parent)
{
    if (Token::Match(parent, ". %name% (")) {
        const Library::Container::Action action = container->getAction(parent->strAt(1));
        if (contains({ Library::Container::Action::POP }, action))
            return true;
    }
    return false;
}

static const Token* getContainerIndex(const Library::Container* container, const Token* parent)
{
    if (Token::Match(parent, ". %name% (")) {
        const Library::Container::Yield yield = container->getYield(parent->strAt(1));
        if (yield == Library::Container::Yield::AT_INDEX && !Token::simpleMatch(parent->tokAt(2), "( )"))
            return parent->tokAt(2)->astOperand2();
    }
    if (!container->arrayLike_indexOp && !container->stdStringLike)
        return nullptr;
    if (Token::simpleMatch(parent, "["))
        return parent->astOperand2();
    return nullptr;
}

static const Token* getContainerFromSize(const Library::Container* container, const Token* tok)
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok->tokAt(-2), ". %name% (")) {
        const Library::Container::Yield yield = container->getYield(tok->strAt(-1));
        if (yield == Library::Container::Yield::SIZE)
            return tok->tokAt(-2)->astOperand1();
    }
    return nullptr;
}

void CheckStl::outOfBounds()
{
    logChecker("CheckStl::outOfBounds");

    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            const Library::Container *container = getLibraryContainer(tok);
            if (!container || container->stdAssociativeLike)
                continue;
            const Token * parent = astParentSkipParens(tok);
            const Token* accessTok = parent;
            if (Token::simpleMatch(accessTok, ".") && Token::simpleMatch(accessTok->astParent(), "("))
                accessTok = accessTok->astParent();
            if (astIsIterator(accessTok) && Token::simpleMatch(accessTok->astParent(), "+"))
                accessTok = accessTok->astParent();
            const Token* indexTok = getContainerIndex(container, parent);
            if (indexTok == tok)
                continue;
            for (const ValueFlow::Value &value : tok->values()) {
                if (!value.isContainerSizeValue())
                    continue;
                if (value.isImpossible())
                    continue;
                if (value.isInconclusive() && !mSettings->certainty.isEnabled(Certainty::inconclusive))
                    continue;
                if (!value.errorSeverity() && !mSettings->severity.isEnabled(Severity::warning))
                    continue;
                if (value.intvalue == 0 && (indexTok ||
                                            (containerYieldsElement(container, parent) && !containerAppendsElement(container, parent)) ||
                                            containerPopsElement(container, parent))) {
                    std::string indexExpr;
                    if (indexTok && !indexTok->hasKnownValue())
                        indexExpr = indexTok->expressionString();
                    outOfBoundsError(accessTok, tok->expressionString(), &value, indexExpr, nullptr);
                    continue;
                }
                if (indexTok) {
                    std::vector<ValueFlow::Value> indexValues =
                        ValueFlow::isOutOfBounds(value, indexTok, mSettings->severity.isEnabled(Severity::warning));
                    if (!indexValues.empty()) {
                        outOfBoundsError(
                            accessTok, tok->expressionString(), &value, indexTok->expressionString(), &indexValues.front());
                        continue;
                    }
                }
            }
            if (indexTok && !indexTok->hasKnownIntValue()) {
                const ValueFlow::Value* value =
                    ValueFlow::findValue(indexTok->values(), *mSettings, [&](const ValueFlow::Value& v) {
                    if (!v.isSymbolicValue())
                        return false;
                    if (v.isImpossible())
                        return false;
                    if (v.intvalue < 0)
                        return false;
                    const Token* sizeTok = v.tokvalue;
                    if (sizeTok && sizeTok->isCast())
                        sizeTok = sizeTok->astOperand2() ? sizeTok->astOperand2() : sizeTok->astOperand1();
                    const Token* containerTok = getContainerFromSize(container, sizeTok);
                    if (!containerTok)
                        return false;
                    return containerTok->exprId() == tok->exprId();
                });
                if (!value)
                    continue;
                outOfBoundsError(accessTok, tok->expressionString(), nullptr, indexTok->expressionString(), value);
            }
        }
    }
}

static std::string indexValueString(const ValueFlow::Value& indexValue, const std::string& containerName = "")
{
    if (indexValue.isIteratorStartValue())
        return "at position " + MathLib::toString(indexValue.intvalue) + " from the beginning";
    if (indexValue.isIteratorEndValue())
        return "at position " + MathLib::toString(-indexValue.intvalue) + " from the end";
    std::string indexString = MathLib::toString(indexValue.intvalue);
    if (indexValue.isSymbolicValue()) {
        indexString = containerName + ".size()";
        if (indexValue.intvalue != 0)
            indexString += "+" + MathLib::toString(indexValue.intvalue);
    }
    if (indexValue.bound == ValueFlow::Value::Bound::Lower)
        return "greater or equal to " + indexString;
    return indexString;
}

void CheckStl::outOfBoundsError(const Token *tok, const std::string &containerName, const ValueFlow::Value *containerSize, const std::string &index, const ValueFlow::Value *indexValue)
{
    // Do not warn if both the container size and index value are possible
    if (containerSize && indexValue && containerSize->isPossible() && indexValue->isPossible())
        return;

    const std::string expression = tok ? tok->expressionString() : (containerName+"[x]");

    std::string errmsg;
    if (!containerSize) {
        if (indexValue && indexValue->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(indexValue->condition) + " or '" + index +
                     "' can have the value " + indexValueString(*indexValue, containerName) + ". Expression '" +
                     expression + "' causes access out of bounds.";
        else
            errmsg = "Out of bounds access in expression '" + expression + "'";
    } else if (containerSize->intvalue == 0) {
        if (containerSize->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(containerSize->condition) + " or expression '" + expression + "' causes access out of bounds.";
        else if (indexValue == nullptr && !index.empty() && tok->valueType() && tok->valueType()->type == ValueType::ITERATOR)
            errmsg = "Out of bounds access in expression '" + expression + "' because '$symbol' is empty and '" + index + "' may be non-zero.";
        else
            errmsg = "Out of bounds access in expression '" + expression + "' because '$symbol' is empty.";
    } else if (indexValue) {
        if (containerSize->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(containerSize->condition) + " or size of '$symbol' can be " + MathLib::toString(containerSize->intvalue) + ". Expression '" + expression + "' causes access out of bounds.";
        else if (indexValue->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(indexValue->condition) + " or '" + index + "' can have the value " + indexValueString(*indexValue) + ". Expression '" + expression + "' causes access out of bounds.";
        else
            errmsg = "Out of bounds access in '" + expression + "', if '$symbol' size is " + MathLib::toString(containerSize->intvalue) + " and '" + index + "' is " + indexValueString(*indexValue);
    } else {
        // should not happen
        return;
    }

    ErrorPath errorPath;
    if (!indexValue)
        errorPath = getErrorPath(tok, containerSize, "Access out of bounds");
    else {
        ErrorPath errorPath1 = getErrorPath(tok, containerSize, "Access out of bounds");
        ErrorPath errorPath2 = getErrorPath(tok, indexValue, "Access out of bounds");
        if (errorPath1.size() <= 1)
            errorPath = std::move(errorPath2);
        else if (errorPath2.size() <= 1)
            errorPath = std::move(errorPath1);
        else {
            errorPath = std::move(errorPath1);
            errorPath.splice(errorPath.end(), errorPath2);
        }
    }

    reportError(std::move(errorPath),
                (containerSize && !containerSize->errorSeverity()) || (indexValue && !indexValue->errorSeverity()) ? Severity::warning : Severity::error,
                "containerOutOfBounds",
                "$symbol:" + containerName +"\n" + errmsg,
                CWE398,
                (containerSize && containerSize->isInconclusive()) || (indexValue && indexValue->isInconclusive()) ? Certainty::inconclusive : Certainty::normal);
}

bool CheckStl::isContainerSize(const Token *containerToken, const Token *expr) const
{
    if (!Token::simpleMatch(expr, "( )"))
        return false;
    if (!Token::Match(expr->astOperand1(), ". %name% ("))
        return false;
    if (!isSameExpression(false, containerToken, expr->astOperand1()->astOperand1(), *mSettings, false, false))
        return false;
    return containerToken->valueType()->container->getYield(expr->strAt(-1)) == Library::Container::Yield::SIZE;
}

bool CheckStl::isContainerSizeGE(const Token * containerToken, const Token *expr) const
{
    if (!expr)
        return false;
    if (isContainerSize(containerToken, expr))
        return true;
    if (expr->str() == "*") {
        const Token *mul;
        if (isContainerSize(containerToken, expr->astOperand1()))
            mul = expr->astOperand2();
        else if (isContainerSize(containerToken, expr->astOperand2()))
            mul = expr->astOperand1();
        else
            return false;
        return mul && (!mul->hasKnownIntValue() || mul->getKnownIntValue() != 0);
    }
    if (expr->str() == "+") {
        const Token *op;
        if (isContainerSize(containerToken, expr->astOperand1()))
            op = expr->astOperand2();
        else if (isContainerSize(containerToken, expr->astOperand2()))
            op = expr->astOperand1();
        else
            return false;
        return op && op->getValueGE(0, *mSettings);
    }
    return false;
}

void CheckStl::outOfBoundsIndexExpression()
{
    logChecker("CheckStl::outOfBoundsIndexExpression");
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            if (!tok->isName() || !tok->valueType())
                continue;
            const Library::Container *container = tok->valueType()->container;
            if (!container)
                continue;
            if (!container->arrayLike_indexOp && !container->stdStringLike)
                continue;
            if (!Token::Match(tok, "%name% ["))
                continue;
            if (isContainerSizeGE(tok, tok->next()->astOperand2()))
                outOfBoundsIndexExpressionError(tok, tok->next()->astOperand2());
        }
    }
}

void CheckStl::outOfBoundsIndexExpressionError(const Token *tok, const Token *index)
{
    const std::string varname = tok ? tok->str() : std::string("var");
    const std::string i = index ? index->expressionString() : (varname + ".size()");

    std::string errmsg = "Out of bounds access of $symbol, index '" + i + "' is out of bounds.";

    reportError(tok,
                Severity::error,
                "containerOutOfBoundsIndexExpression",
                "$symbol:" + varname +"\n" + errmsg,
                CWE398,
                Certainty::normal);
}



// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &iteratorName)
{
    reportError(tok, Severity::error, "invalidIterator1", "$symbol:"+iteratorName+"\nInvalid iterator: $symbol", CWE664, Certainty::normal);
}

void CheckStl::iteratorsError(const Token* tok, const std::string& containerName1, const std::string& containerName2)
{
    reportError(tok, Severity::error, "iterators1",
                "$symbol:" + containerName1 + "\n"
                "$symbol:" + containerName2 + "\n"
                "Same iterator is used with different containers '" + containerName1 + "' and '" + containerName2 + "'.", CWE664, Certainty::normal);
}

void CheckStl::iteratorsError(const Token* tok, const Token* containerTok, const std::string& containerName)
{
    std::list<const Token*> callstack = { tok, containerTok };
    reportError(callstack,
                Severity::error,
                "iterators3",
                "$symbol:" + containerName +
                "\n"
                "Same iterator is used with containers '$symbol' that are temporaries or defined in different scopes.",
                CWE664,
                Certainty::normal);
}

// Error message used when dereferencing an iterator that has been erased..
void CheckStl::dereferenceErasedError(const Token *erased, const Token* deref, const std::string &itername, bool inconclusive)
{
    if (erased) {
        std::list<const Token*> callstack = { deref, erased };
        reportError(callstack, Severity::error, "eraseDereference",
                    "$symbol:" + itername + "\n"
                    "Iterator '$symbol' used after element has been erased.\n"
                    "The iterator '$symbol' is invalid after the element it pointed to has been erased. "
                    "Dereferencing or comparing it with another iterator is invalid operation.", CWE664, inconclusive ? Certainty::inconclusive : Certainty::normal);
    } else {
        reportError(deref, Severity::error, "eraseDereference",
                    "$symbol:" + itername + "\n"
                    "Invalid iterator '$symbol' used.\n"
                    "The iterator '$symbol' is invalid before being assigned. "
                    "Dereferencing or comparing it with another iterator is invalid operation.", CWE664, inconclusive ? Certainty::inconclusive : Certainty::normal);
    }
}

static const Token *skipMembers(const Token *tok)
{
    while (Token::Match(tok, "%name% ."))
        tok = tok->tokAt(2);
    return tok;
}

static bool isIterator(const Variable *var, bool& inconclusiveType)
{
    // Check that its an iterator
    if (!var || !var->isLocal() || !Token::Match(var->typeEndToken(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator|auto"))
        return false;

    inconclusiveType = false;
    if (var->typeEndToken()->str() == "auto")
        return (var->nameToken()->valueType() && var->nameToken()->valueType()->type == ValueType::Type::ITERATOR);

    if (var->type()) { // If it is defined, ensure that it is defined like an iterator
        // look for operator* and operator++
        const Function* end = var->type()->getFunction("operator*");
        const Function* incOperator = var->type()->getFunction("operator++");
        if (!end || end->argCount() > 0 || !incOperator)
            return false;

        inconclusiveType = true; // heuristics only
    }

    return true;
}

static std::string getContainerName(const Token *containerToken)
{
    if (!containerToken)
        return std::string();
    std::string ret(containerToken->str());
    for (const Token *nametok = containerToken; nametok; nametok = nametok->tokAt(-2)) {
        if (!Token::Match(nametok->tokAt(-2), "%name% ."))
            break;
        ret = nametok->strAt(-2) + '.' + ret;
    }
    return ret;
}

static bool isVector(const Token* tok)
{
    if (!tok)
        return false;
    const Variable *var = tok->variable();
    const Token *decltok = var ? var->typeStartToken() : nullptr;
    return Token::simpleMatch(decltok, "std :: vector");
}

void CheckStl::iterators()
{
    logChecker("CheckStl::iterators");

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // Filling map of iterators id and their scope begin
    std::map<int, const Token*> iteratorScopeBeginInfo;
    for (const Variable* var : symbolDatabase->variableList()) {
        bool inconclusiveType=false;
        if (!isIterator(var, inconclusiveType))
            continue;
        const int iteratorId = var->declarationId();
        if (iteratorId != 0)
            iteratorScopeBeginInfo[iteratorId] = var->nameToken();
    }

    for (const Variable* var : symbolDatabase->variableList()) {
        bool inconclusiveType=false;
        if (!isIterator(var, inconclusiveType))
            continue;
        if (inconclusiveType && !mSettings->certainty.isEnabled(Certainty::inconclusive))
            continue;

        const int iteratorId = var->declarationId();

        // the validIterator flag says if the iterator has a valid value or not
        bool validIterator = Token::Match(var->nameToken()->next(), "[(=:{[]");
        const Scope* invalidationScope = nullptr;

        // The container this iterator can be used with
        const Token* containerToken = nullptr;
        const Scope* containerAssignScope = nullptr;

        // When "validatingToken" is reached the validIterator is set to true
        const Token* validatingToken = nullptr;

        const Token* eraseToken = nullptr;

        // Scan through the rest of the code and see if the iterator is
        // used against other containers.
        for (const Token *tok2 = var->nameToken(); tok2 && tok2 != var->scope()->bodyEnd; tok2 = tok2->next()) {
            if (invalidationScope && tok2 == invalidationScope->bodyEnd)
                validIterator = true; // Assume that the iterator becomes valid again
            if (containerAssignScope && tok2 == containerAssignScope->bodyEnd)
                containerToken = nullptr; // We don't know which containers might be used with the iterator

            if (tok2 == validatingToken) {
                validIterator = true;
                eraseToken = nullptr;
                invalidationScope = nullptr;
            }

            // Is the iterator used in a insert/erase operation?
            if (Token::Match(tok2, "%name% . insert|erase ( *| %varid% )|,", iteratorId) && !isVector(tok2)) {
                const Token* itTok = tok2->tokAt(4);
                if (itTok->str() == "*") {
                    if (tok2->strAt(2) == "insert")
                        continue;

                    itTok = itTok->next();
                }
                // It is bad to insert/erase an invalid iterator
                if (!validIterator)
                    invalidIteratorError(tok2, itTok->str());

                // If insert/erase is used on different container then
                // report an error
                if (containerToken && tok2->varId() != containerToken->varId()) {
                    // skip error message if container is a set..
                    const Variable *variableInfo = tok2->variable();
                    const Token *decltok = variableInfo ? variableInfo->typeStartToken() : nullptr;

                    if (Token::simpleMatch(decltok, "std :: set"))
                        continue; // No warning

                    // skip error message if the iterator is erased/inserted by value
                    if (itTok->strAt(-1) == "*")
                        continue;

                    // inserting iterator range..
                    if (tok2->strAt(2) == "insert") {
                        const Token *par2 = itTok->nextArgument();
                        if (!par2 || par2->nextArgument())
                            continue;
                        while (par2->str() != ")") {
                            if (par2->varId() == containerToken->varId())
                                break;
                            bool inconclusiveType2=false;
                            if (isIterator(par2->variable(), inconclusiveType2))
                                break;  // TODO: check if iterator points at same container
                            if (par2->str() == "(")
                                par2 = par2->link();
                            par2 = par2->next();
                        }
                        if (par2->str() != ")")
                            continue;
                    }

                    // Not different containers if a reference is used..
                    if (containerToken && containerToken->variable() && containerToken->variable()->isReference()) {
                        const Token *nameToken = containerToken->variable()->nameToken();
                        if (Token::Match(nameToken, "%name% =")) {
                            const Token *name1 = nameToken->tokAt(2);
                            const Token *name2 = tok2;
                            while (Token::Match(name1, "%name%|.|::") && name2 && name1->str() == name2->str()) {
                                name1 = name1->next();
                                name2 = name2->next();
                            }
                            if (!Token::simpleMatch(name1, ";") || !Token::Match(name2, "[;,()=]"))
                                continue;
                        }
                    }

                    // Show error message, mismatching iterator is used.
                    iteratorsError(tok2, getContainerName(containerToken), getContainerName(tok2));
                }

                // invalidate the iterator if it is erased
                else if (tok2->strAt(2) == "erase" && (tok2->strAt(4) != "*" || (containerToken && tok2->varId() == containerToken->varId()))) {
                    validIterator = false;
                    eraseToken = tok2;
                    invalidationScope = tok2->scope();
                }

                // skip the operation
                tok2 = itTok->next();
            }

            // it = foo.erase(..
            // taking the result of an erase is ok
            else if (Token::Match(tok2, "%varid% = %name% .", iteratorId) &&
                     Token::simpleMatch(skipMembers(tok2->tokAt(2)), "erase (")) {
                // the returned iterator is valid
                validatingToken = skipMembers(tok2->tokAt(2))->linkAt(1);
                tok2 = validatingToken->link();
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% = %name% .", iteratorId) &&
                     Token::Match(skipMembers(tok2->tokAt(2)), "begin|rbegin|cbegin|crbegin|find (")) {
                validatingToken = skipMembers(tok2->tokAt(2))->linkAt(1);
                containerToken = skipMembers(tok2->tokAt(2))->tokAt(-2);
                if (containerToken->varId() == 0 || Token::simpleMatch(validatingToken, ") ."))
                    containerToken = nullptr;
                containerAssignScope = tok2->scope();

                // skip ahead
                tok2 = validatingToken->link();
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% =", iteratorId)) {
                break;
            }

            // Passing iterator to function. Iterator might be initialized
            else if (Token::Match(tok2, "%varid% ,|)", iteratorId)) {
                validIterator = true;
            }

            // Dereferencing invalid iterator?
            else if (!validIterator && Token::Match(tok2, "* %varid%", iteratorId)) {
                dereferenceErasedError(eraseToken, tok2, tok2->strAt(1), inconclusiveType);
                tok2 = tok2->next();
            } else if (!validIterator && Token::Match(tok2, "%varid% . %name%", iteratorId)) {
                dereferenceErasedError(eraseToken, tok2, tok2->str(), inconclusiveType);
                tok2 = tok2->tokAt(2);
            }

            // bailout handling. Assume that the iterator becomes valid if we see return/break.
            // TODO: better handling
            else if (tok2->scope() == invalidationScope && Token::Match(tok2, "return|break|continue")) {
                validatingToken = Token::findsimplematch(tok2->next(), ";");
            }

            // bailout handling. Assume that the iterator becomes valid if we see else.
            // TODO: better handling
            else if (tok2->str() == "else") {
                validIterator = true;
            }
        }
    }
}

void CheckStl::mismatchingContainerIteratorError(const Token* containerTok, const Token* iterTok, const Token* containerTok2)
{
    const std::string container(containerTok ? containerTok->expressionString() : std::string("v1"));
    const std::string container2(containerTok2 ? containerTok2->expressionString() : std::string("v2"));
    const std::string iter(iterTok ? iterTok->expressionString() : std::string("it"));
    reportError(containerTok,
                Severity::error,
                "mismatchingContainerIterator",
                "Iterator '" + iter + "' referring to container '" + container2 + "' is used with container '" + container + "'.",
                CWE664,
                Certainty::normal);
}

// Error message for bad iterator usage..
void CheckStl::mismatchingContainersError(const Token* tok1, const Token* tok2)
{
    const std::string expr1(tok1 ? tok1->expressionString() : std::string("v1"));
    const std::string expr2(tok2 ? tok2->expressionString() : std::string("v2"));
    reportError(tok1,
                Severity::error,
                "mismatchingContainers",
                "Iterators of different containers '" + expr1 + "' and '" + expr2 + "' are used together.",
                CWE664,
                Certainty::normal);
}

void CheckStl::mismatchingContainerExpressionError(const Token *tok1, const Token *tok2)
{
    const std::string expr1(tok1 ? tok1->expressionString() : std::string("v1"));
    const std::string expr2(tok2 ? tok2->expressionString() : std::string("v2"));
    reportError(tok1, Severity::warning, "mismatchingContainerExpression",
                "Iterators to containers from different expressions '" +
                expr1 + "' and '" + expr2 + "' are used together.", CWE664, Certainty::normal);
}

void CheckStl::sameIteratorExpressionError(const Token *tok)
{
    reportError(tok, Severity::style, "sameIteratorExpression", "Same iterators expression are used for algorithm.", CWE664, Certainty::normal);
}

static std::vector<const Token*> getAddressContainer(const Token* tok)
{
    if (Token::simpleMatch(tok, "[") && tok->astOperand1())
        return { tok->astOperand1() };
    while (Token::simpleMatch(tok, "::") && tok->astOperand2())
        tok = tok->astOperand2();
    std::vector<ValueFlow::Value> values = ValueFlow::getLifetimeObjValues(tok, /*inconclusive*/ false);
    std::vector<const Token*> res;
    for (const auto& v : values) {
        if (v.tokvalue)
            res.emplace_back(v.tokvalue);
    }
    if (res.empty())
        res.emplace_back(tok);
    return res;
}

static bool isSameIteratorContainerExpression(const Token* tok1,
                                              const Token* tok2,
                                              const Settings& settings,
                                              ValueFlow::Value::LifetimeKind kind = ValueFlow::Value::LifetimeKind::Iterator)
{
    if (isSameExpression(false, tok1, tok2, settings, false, false)) {
        return !astIsContainerOwned(tok1) || !isTemporary(tok1, &settings.library);
    }
    if (astContainerYield(tok2, settings.library) == Library::Container::Yield::ITEM)
        return true;
    if (kind == ValueFlow::Value::LifetimeKind::Address || kind == ValueFlow::Value::LifetimeKind::Iterator) {
        const auto address1 = getAddressContainer(tok1);
        const auto address2 = getAddressContainer(tok2);
        return std::any_of(address1.begin(), address1.end(), [&](const Token* tok1) {
            return std::any_of(address2.begin(), address2.end(), [&](const Token* tok2) {
                return isSameExpression(false, tok1, tok2, settings, false, false);
            });
        });
    }
    return false;
}

// First it groups the lifetimes together using std::partition with the lifetimes that refer to the same token or token of a subexpression.
// Then it finds the lifetime in that group that refers to the "highest" parent using std::min_element and adds that to the vector.
static std::vector<ValueFlow::Value> pruneLifetimes(std::vector<ValueFlow::Value> lifetimes)
{
    std::vector<ValueFlow::Value> result;
    auto start = lifetimes.begin();
    while (start != lifetimes.end())
    {
        const Token* tok1 = start->tokvalue;
        auto it = std::partition(start, lifetimes.end(), [&](const ValueFlow::Value& v) {
            const Token* tok2 = v.tokvalue;
            return start->lifetimeKind == v.lifetimeKind && (astHasToken(tok1, tok2) || astHasToken(tok2, tok1));
        });
        auto root = std::min_element(start, it, [](const ValueFlow::Value& x, const ValueFlow::Value& y) {
            return x.tokvalue != y.tokvalue && astHasToken(x.tokvalue, y.tokvalue);
        });
        result.push_back(*root);
        start = it;
    }
    return result;
}

static ValueFlow::Value getLifetimeIteratorValue(const Token* tok, MathLib::bigint path = 0)
{
    auto findIterVal = [](const std::vector<ValueFlow::Value>& values, const std::vector<ValueFlow::Value>::const_iterator beg) {
        return std::find_if(beg, values.cend(), [](const ValueFlow::Value& v) {
            return v.lifetimeKind == ValueFlow::Value::LifetimeKind::Iterator;
        });
    };
    std::vector<ValueFlow::Value> values = pruneLifetimes(ValueFlow::getLifetimeObjValues(tok, false, path));
    auto it = findIterVal(values, values.begin());
    if (it != values.end()) {
        auto it2 = findIterVal(values, it + 1);
        if (it2 == values.cend())
            return *it;
    }
    if (values.size() == 1)
        return values.front();
    return ValueFlow::Value{};
}

bool CheckStl::checkIteratorPair(const Token* tok1, const Token* tok2)
{
    if (!tok1)
        return false;
    if (!tok2)
        return false;
    ValueFlow::Value val1 = getLifetimeIteratorValue(tok1);
    ValueFlow::Value val2 = getLifetimeIteratorValue(tok2);
    if (val1.tokvalue && val2.tokvalue && val1.lifetimeKind == val2.lifetimeKind) {
        if (val1.lifetimeKind == ValueFlow::Value::LifetimeKind::Lambda)
            return false;
        if (tok1->astParent() == tok2->astParent() && Token::Match(tok1->astParent(), "%comp%|-")) {
            if (val1.lifetimeKind == ValueFlow::Value::LifetimeKind::Address)
                return false;
            if (val1.lifetimeKind == ValueFlow::Value::LifetimeKind::Object &&
                (!astIsContainer(val1.tokvalue) || !astIsContainer(val2.tokvalue)))
                return false;
        }
        if (isSameIteratorContainerExpression(val1.tokvalue, val2.tokvalue, *mSettings, val1.lifetimeKind))
            return false;
        if (val1.tokvalue->expressionString() == val2.tokvalue->expressionString())
            iteratorsError(tok1, val1.tokvalue, val1.tokvalue->expressionString());
        else
            mismatchingContainersError(val1.tokvalue, val2.tokvalue);
        return true;
    }

    if (Token::Match(tok1->astParent(), "%comp%|-")) {
        if (astIsIntegral(tok1, true) || astIsIntegral(tok2, true) ||
            astIsFloat(tok1, true) || astIsFloat(tok2, true))
            return false;
    }
    const Token* iter1 = getIteratorExpression(tok1);
    if (!iter1)
        return false;
    const Token* iter2 = getIteratorExpression(tok2);
    if (!iter2)
        return false;
    if (!isSameIteratorContainerExpression(iter1, iter2, *mSettings)) {
        mismatchingContainerExpressionError(iter1, iter2);
        return true;
    }
    return false;
}

namespace {
    struct ArgIteratorInfo {
        const Token* tok;
        const Library::ArgumentChecks::IteratorInfo* info;
    };
}

void CheckStl::mismatchingContainers()
{
    logChecker("CheckStl::misMatchingContainers");

    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%comp%|-")) {
                if (checkIteratorPair(tok->astOperand1(), tok->astOperand2()))
                    continue;
            }
            if (!Token::Match(tok, "%name% ( !!)"))
                continue;
            const Token * const ftok = tok;

            const std::vector<const Token *> args = getArguments(ftok);
            if (args.size() < 2)
                continue;

            // Group args together by container
            std::map<int, std::vector<ArgIteratorInfo>> containers;
            for (int argnr = 1; argnr <= args.size(); ++argnr) {
                const Library::ArgumentChecks::IteratorInfo *i = mSettings->library.getArgIteratorInfo(ftok, argnr);
                if (!i)
                    continue;
                const Token * const argTok = args[argnr - 1];
                containers[i->container].emplace_back(ArgIteratorInfo{argTok, i});
            }

            // Lambda is used to escape the nested loops
            [&] {
                for (const auto& p : containers)
                {
                    const std::vector<ArgIteratorInfo>& cargs = p.second;
                    for (ArgIteratorInfo iter1 : cargs) {
                        for (ArgIteratorInfo iter2 : cargs) {
                            if (iter1.tok == iter2.tok)
                                continue;
                            if (iter1.info->first && iter2.info->last &&
                                isSameExpression(false, iter1.tok, iter2.tok, *mSettings, false, false))
                                sameIteratorExpressionError(iter1.tok);
                            if (checkIteratorPair(iter1.tok, iter2.tok))
                                return;
                        }
                    }
                }
            }();
        }
    }
    for (const Variable *var : symbolDatabase->variableList()) {
        if (var && var->isStlStringType() && Token::Match(var->nameToken(), "%var% (") &&
            Token::Match(var->nameToken()->tokAt(2), "%name% . begin|cbegin|rbegin|crbegin ( ) , %name% . end|cend|rend|crend ( ) ,|)")) {
            if (var->nameToken()->strAt(2) != var->nameToken()->strAt(8)) {
                mismatchingContainersError(var->nameToken(), var->nameToken()->tokAt(2));
            }
        }
    }
}

void CheckStl::mismatchingContainerIterator()
{
    logChecker("CheckStl::misMatchingContainerIterator");

    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!astIsContainer(tok))
                continue;
            if (!astIsLHS(tok))
                continue;
            if (!Token::Match(tok->astParent(), ". %name% ( !!)"))
                continue;
            const Token* const ftok = tok->astParent()->next();
            const std::vector<const Token *> args = getArguments(ftok);

            const Library::Container * c = tok->valueType()->container;
            const Library::Container::Action action = c->getAction(tok->strAt(2));
            const Token* iterTok = nullptr;
            if (action == Library::Container::Action::INSERT && args.size() == 2) {
                // Skip if iterator pair
                if (astIsIterator(args.back()))
                    continue;
                if (!astIsIterator(args.front()))
                    continue;
                iterTok = args.front();
            } else if (action == Library::Container::Action::ERASE) {
                if (!astIsIterator(args.front()))
                    continue;
                iterTok = args.front();
            } else {
                continue;
            }

            ValueFlow::Value val = getLifetimeIteratorValue(iterTok);
            if (!val.tokvalue)
                continue;
            if (!val.isKnown() && Token::simpleMatch(val.tokvalue->astParent(), ":"))
                continue;
            if (val.lifetimeKind != ValueFlow::Value::LifetimeKind::Iterator)
                continue;
            if (iterTok->str() == "*" && iterTok->astOperand1()->valueType() && iterTok->astOperand1()->valueType()->type == ValueType::ITERATOR)
                continue;
            if (isSameIteratorContainerExpression(tok, val.tokvalue, *mSettings))
                continue;
            mismatchingContainerIteratorError(tok, iterTok, val.tokvalue);
        }
    }
}

static const Token* getInvalidMethod(const Token* tok)
{
    if (!astIsLHS(tok))
        return nullptr;
    if (Token::Match(tok->astParent(), ". assign|clear|swap"))
        return tok->astParent()->next();
    if (Token::Match(tok->astParent(), "%assign%"))
        return tok->astParent();
    const Token* ftok = nullptr;
    if (Token::Match(tok->astParent(), ". %name% ("))
        ftok = tok->astParent()->next();
    if (!ftok)
        return nullptr;
    if (const Library::Container * c = tok->valueType()->container) {
        const Library::Container::Action action = c->getAction(ftok->str());
        if (c->unstableErase) {
            if (action == Library::Container::Action::ERASE)
                return ftok;
        }
        if (c->unstableInsert) {
            if (action == Library::Container::Action::RESIZE)
                return ftok;
            if (action == Library::Container::Action::CLEAR)
                return ftok;
            if (action == Library::Container::Action::PUSH)
                return ftok;
            if (action == Library::Container::Action::POP)
                return ftok;
            if (action == Library::Container::Action::INSERT)
                return ftok;
            if (action == Library::Container::Action::CHANGE)
                return ftok;
            if (action == Library::Container::Action::CHANGE_INTERNAL)
                return ftok;
            if (Token::Match(ftok, "insert|emplace"))
                return ftok;
        }
    }
    return nullptr;
}

namespace {
    struct InvalidContainerAnalyzer {
        struct Info {
            struct Reference {
                const Token* tok;
                ErrorPath errorPath;
                const Token* ftok;
            };
            std::unordered_map<int, Reference> expressions;

            void add(const std::vector<Reference>& refs) {
                for (const Reference& r : refs) {
                    add(r);
                }
            }
            void add(const Reference& r) {
                if (!r.tok)
                    return;
                expressions.emplace(r.tok->exprId(), r);
            }

            std::vector<Reference> invalidTokens() const {
                std::vector<Reference> result;
                std::transform(expressions.cbegin(), expressions.cend(), std::back_inserter(result), SelectMapValues{});
                return result;
            }
        };
        std::unordered_map<const Function*, Info> invalidMethods;

        std::vector<Info::Reference> invalidatesContainer(const Token* tok) const {
            std::vector<Info::Reference> result;
            if (Token::Match(tok, "%name% (")) {
                const Function* f = tok->function();
                if (!f)
                    return result;
                ErrorPathItem epi = std::make_pair(tok, "Calling function " + tok->str());
                const bool dependsOnThis = exprDependsOnThis(tok->next());
                auto it = invalidMethods.find(f);
                if (it != invalidMethods.end()) {
                    std::vector<Info::Reference> refs = it->second.invalidTokens();
                    std::copy_if(refs.cbegin(), refs.cend(), std::back_inserter(result), [&](const Info::Reference& r) {
                        const Variable* var = r.tok->variable();
                        if (!var)
                            return false;
                        if (dependsOnThis && !var->isLocal() && !var->isGlobal() && !var->isStatic())
                            return true;
                        if (!var->isArgument())
                            return false;
                        if (!var->isReference())
                            return false;
                        return true;
                    });
                    std::vector<const Token*> args = getArguments(tok);
                    for (Info::Reference& r : result) {
                        r.errorPath.push_front(epi);
                        r.ftok = tok;
                        const Variable* var = r.tok->variable();
                        if (!var)
                            continue;
                        if (var->isArgument()) {
                            const int n = getArgumentPos(var, f);
                            const Token* tok2 = nullptr;
                            if (n >= 0 && n < args.size())
                                tok2 = args[n];
                            r.tok = tok2;
                        }
                    }
                }
            } else if (astIsContainer(tok)) {
                const Token* ftok = getInvalidMethod(tok);
                if (ftok) {
                    ErrorPath ep;
                    ep.emplace_front(ftok,
                                     "After calling '" + ftok->expressionString() +
                                     "', iterators or references to the container's data may be invalid .");
                    result.emplace_back(Info::Reference{tok, std::move(ep), ftok});
                }
            }
            return result;
        }

        void analyze(const SymbolDatabase* symboldatabase) {
            for (const Scope* scope : symboldatabase->functionScopes) {
                const Function* f = scope->function;
                if (!f)
                    continue;
                for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
                    if (Token::Match(tok, "if|while|for|goto|return"))
                        break;
                    std::vector<Info::Reference> c = invalidatesContainer(tok);
                    if (c.empty())
                        continue;
                    invalidMethods[f].add(c);
                }
            }
        }
    };
}

static const Token* getLoopContainer(const Token* tok)
{
    if (!Token::simpleMatch(tok, "for ("))
        return nullptr;
    const Token* sepTok = tok->next()->astOperand2();
    if (!Token::simpleMatch(sepTok, ":"))
        return nullptr;
    return sepTok->astOperand2();
}

static const ValueFlow::Value* getInnerLifetime(const Token* tok,
                                                nonneg int id,
                                                ErrorPath* errorPath = nullptr,
                                                int depth = 4)
{
    if (depth < 0)
        return nullptr;
    if (!tok)
        return nullptr;
    for (const ValueFlow::Value& val : tok->values()) {
        if (!val.isLocalLifetimeValue())
            continue;
        if (contains({ValueFlow::Value::LifetimeKind::Address,
                      ValueFlow::Value::LifetimeKind::SubObject,
                      ValueFlow::Value::LifetimeKind::Lambda},
                     val.lifetimeKind)) {
            if (val.isInconclusive())
                return nullptr;
            if (val.capturetok)
                if (const ValueFlow::Value* v = getInnerLifetime(val.capturetok, id, errorPath, depth - 1))
                    return v;
            if (errorPath)
                errorPath->insert(errorPath->end(), val.errorPath.cbegin(), val.errorPath.cend());
            if (const ValueFlow::Value* v = getInnerLifetime(val.tokvalue, id, errorPath, depth - 1))
                return v;
            continue;
        }
        if (!val.tokvalue->variable())
            continue;
        if (val.tokvalue->varId() != id)
            continue;
        return &val;
    }
    return nullptr;
}

static const Token* endOfExpression(const Token* tok)
{
    if (!tok)
        return nullptr;
    const Token* parent = tok->astParent();
    while (Token::simpleMatch(parent, "."))
        parent = parent->astParent();
    if (!parent)
        return tok->next();
    const Token* endToken = nextAfterAstRightmostLeaf(parent);
    if (!endToken)
        return parent->next();
    return endToken;
}

void CheckStl::invalidContainer()
{
    logChecker("CheckStl::invalidContainer");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    InvalidContainerAnalyzer analyzer;
    analyzer.analyze(symbolDatabase);
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (const Token* contTok = getLoopContainer(tok)) {
                const Token* blockStart = tok->linkAt(1)->next();
                const Token* blockEnd = blockStart->link();
                if (contTok->exprId() == 0)
                    continue;
                if (!astIsContainer(contTok))
                    continue;
                for (const Token* tok2 = blockStart; tok2 != blockEnd; tok2 = tok2->next()) {
                    bool bail = false;
                    for (const InvalidContainerAnalyzer::Info::Reference& r : analyzer.invalidatesContainer(tok2)) {
                        if (!astIsContainer(r.tok))
                            continue;
                        if (r.tok->exprId() != contTok->exprId())
                            continue;
                        const Scope* s = tok2->scope();
                        if (!s)
                            continue;
                        if (isReturnScope(s->bodyEnd, mSettings->library))
                            continue;
                        invalidContainerLoopError(r.ftok, tok, r.errorPath);
                        bail = true;
                        break;
                    }
                    if (bail)
                        break;
                }
            } else {
                for (const InvalidContainerAnalyzer::Info::Reference& r : analyzer.invalidatesContainer(tok)) {
                    if (!astIsContainer(r.tok))
                        continue;
                    std::set<nonneg int> skipVarIds;
                    // Skip if the variable is assigned to
                    const Token* assignExpr = tok;
                    while (assignExpr->astParent()) {
                        const bool isRHS = astIsRHS(assignExpr);
                        assignExpr = assignExpr->astParent();
                        if (Token::Match(assignExpr, "%assign%")) {
                            if (!isRHS)
                                assignExpr = nullptr;
                            break;
                        }
                    }
                    if (Token::Match(assignExpr, "%assign%") && Token::Match(assignExpr->astOperand1(), "%var%"))
                        skipVarIds.insert(assignExpr->astOperand1()->varId());
                    const Token* endToken = endOfExpression(tok);
                    const ValueFlow::Value* v = nullptr;
                    ErrorPath errorPath;
                    PathAnalysis::Info info =
                        PathAnalysis{endToken}.forwardFind([&](const PathAnalysis::Info& info) {
                        if (!info.tok->variable())
                            return false;
                        if (info.tok->varId() == 0)
                            return false;
                        if (skipVarIds.count(info.tok->varId()) > 0)
                            return false;
                        // if (Token::simpleMatch(info.tok->next(), "."))
                        // return false;
                        if (Token::Match(info.tok->astParent(), "%assign%") && astIsLHS(info.tok))
                            skipVarIds.insert(info.tok->varId());
                        if (info.tok->variable()->isReference() && !isVariableDecl(info.tok) &&
                            reaches(info.tok->variable()->nameToken(), tok, nullptr)) {

                            if ((assignExpr && Token::Match(assignExpr->astOperand1(), "& %varid%", info.tok->varId())) || // TODO: fix AST
                                Token::Match(assignExpr, "& %varid% {|(", info.tok->varId())) {
                                return false;
                            }

                            ErrorPath ep;
                            bool addressOf = false;
                            const Variable* var = ValueFlow::getLifetimeVariable(info.tok, ep, *mSettings, &addressOf);
                            // Check the reference is created before the change
                            if (var && var->declarationId() == r.tok->varId() && !addressOf) {
                                // An argument always reaches
                                if (var->isArgument() ||
                                    (!var->isReference() && !var->isRValueReference() && !isVariableDecl(tok) &&
                                     reaches(var->nameToken(), tok, &ep))) {
                                    errorPath = std::move(ep);
                                    return true;
                                }
                            }
                        }
                        ErrorPath ep;
                        const ValueFlow::Value* val = getInnerLifetime(info.tok, r.tok->varId(), &ep);
                        // Check the iterator is created before the change
                        if (val && val->tokvalue != tok && reaches(val->tokvalue, tok, &ep)) {
                            v = val;
                            errorPath = std::move(ep);
                            return true;
                        }
                        return false;
                    });
                    if (!info.tok)
                        continue;
                    errorPath.insert(errorPath.end(), info.errorPath.cbegin(), info.errorPath.cend());
                    errorPath.insert(errorPath.end(), r.errorPath.cbegin(), r.errorPath.cend());
                    if (v) {
                        invalidContainerError(info.tok, v, std::move(errorPath));
                    } else {
                        invalidContainerReferenceError(info.tok, r.tok, std::move(errorPath));
                    }
                }
            }
        }
    }
}

void CheckStl::invalidContainerLoopError(const Token* tok, const Token* loopTok, ErrorPath errorPath)
{
    const std::string method = tok ? tok->str() : "erase";
    errorPath.emplace_back(loopTok, "Iterating container here.");

    // Remove duplicate entries from error path
    errorPath.remove_if([&](const ErrorPathItem& epi) {
        return epi.first == tok;
    });

    const std::string msg = "Calling '" + method + "' while iterating the container is invalid.";
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "invalidContainerLoop", msg, CWE664, Certainty::normal);
}

void CheckStl::invalidContainerError(const Token *tok, const ValueFlow::Value *val, ErrorPath errorPath)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    if (val)
        errorPath.insert(errorPath.begin(), val->errorPath.cbegin(), val->errorPath.cend());
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "invalidContainer", msg + " that may be invalid.", CWE664, inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckStl::invalidContainerReferenceError(const Token* tok, const Token* contTok, ErrorPath errorPath)
{
    std::string name = contTok ? contTok->expressionString() : "x";
    std::string msg = "Reference to " + name;
    errorPath.emplace_back(tok, "");
    reportError(std::move(errorPath), Severity::error, "invalidContainerReference", msg + " that may be invalid.", CWE664, Certainty::normal);
}

void CheckStl::stlOutOfBounds()
{
    logChecker("CheckStl::stlOutOfBounds");

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    // Scan through all scopes..
    for (const Scope &scope : symbolDatabase->scopeList) {
        const Token* tok = scope.classDef;
        // only interested in conditions
        if ((!scope.isLoopScope() && scope.type != ScopeType::eIf) || !tok)
            continue;

        const Token *condition = nullptr;
        if (scope.type == ScopeType::eFor) {
            if (Token::simpleMatch(tok->next()->astOperand2(), ";") && Token::simpleMatch(tok->next()->astOperand2()->astOperand2(), ";"))
                condition = tok->next()->astOperand2()->astOperand2()->astOperand1();
        } else if (Token::simpleMatch(tok, "do {") && Token::simpleMatch(tok->linkAt(1), "} while ("))
            condition = tok->linkAt(1)->tokAt(2)->astOperand2();
        else
            condition = tok->next()->astOperand2();

        if (!condition)
            continue;

        std::vector<const Token *> conds;

        visitAstNodes(condition,
                      [&](const Token *cond) {
            if (Token::Match(cond, "%oror%|&&"))
                return ChildrenToVisit::op1_and_op2;
            if (cond->isComparisonOp())
                conds.emplace_back(cond);
            return ChildrenToVisit::none;
        });

        for (const Token *cond : conds) {
            const Token *vartok;
            const Token *containerToken;
            // check in the ast that cond is of the form "%var% <= %var% . %name% ( )"
            if (cond->str() == "<=" && Token::Match(cond->astOperand1(), "%var%") &&
                cond->astOperand2()->str() == "(" && cond->astOperand2()->astOperand1()->str() == "." &&
                Token::Match(cond->astOperand2()->astOperand1()->astOperand1(), "%var%") &&
                Token::Match(cond->astOperand2()->astOperand1()->astOperand2(), "%name%")) {
                vartok = cond->astOperand1();
                containerToken = cond->next();
            } else {
                continue;
            }

            if (containerToken->hasKnownValue(ValueFlow::Value::ValueType::CONTAINER_SIZE))
                continue;

            // Is it a array like container?
            const Library::Container* container = containerToken->valueType() ? containerToken->valueType()->container : nullptr;
            if (!container)
                continue;
            if (container->getYield(containerToken->strAt(2)) != Library::Container::Yield::SIZE)
                continue;

            // variable id for loop variable.
            const int numId = vartok->varId();

            // variable id for the container variable
            const int declarationId = containerToken->varId();
            const std::string &containerName = containerToken->str();

            for (const Token *tok3 = scope.bodyStart; tok3 && tok3 != scope.bodyEnd; tok3 = tok3->next()) {
                if (tok3->varId() == declarationId) {
                    tok3 = tok3->next();
                    if (Token::Match(tok3, ". %name% ( )")) {
                        if (container->getYield(tok3->strAt(1)) == Library::Container::Yield::SIZE)
                            break;
                    } else if (container->arrayLike_indexOp && Token::Match(tok3, "[ %varid% ]", numId))
                        stlOutOfBoundsError(tok3, tok3->strAt(1), containerName, false);
                    else if (Token::Match(tok3, ". %name% ( %varid% )", numId)) {
                        const Library::Container::Yield yield = container->getYield(tok3->strAt(1));
                        if (yield == Library::Container::Yield::AT_INDEX)
                            stlOutOfBoundsError(tok3, tok3->strAt(3), containerName, true);
                    }
                }
            }
        }
    }
}

void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var, bool at)
{
    if (at)
        reportError(tok, Severity::error, "stlOutOfBounds", "$symbol:" + var + "\nWhen " + num + "==$symbol.size(), $symbol.at(" + num + ") is out of bounds.", CWE788, Certainty::normal);
    else
        reportError(tok, Severity::error, "stlOutOfBounds", "$symbol:" + var + "\nWhen " + num + "==$symbol.size(), $symbol[" + num + "] is out of bounds.", CWE788, Certainty::normal);
}

void CheckStl::negativeIndex()
{
    logChecker("CheckStl::negativeIndex");

    // Negative index is out of bounds..
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%var% [") || !tok->next()->astOperand2())
                continue;
            const Variable * const var = tok->variable();
            if (!var || tok == var->nameToken())
                continue;
            const Library::Container * const container = mSettings->library.detectContainer(var->typeStartToken());
            if (!container || !container->arrayLike_indexOp)
                continue;
            const ValueFlow::Value *index = tok->next()->astOperand2()->getValueLE(-1, *mSettings);
            if (!index)
                continue;
            negativeIndexError(tok, *index);
        }
    }
}

void CheckStl::negativeIndexError(const Token *tok, const ValueFlow::Value &index)
{
    ErrorPath errorPath = getErrorPath(tok, &index, "Negative array index");
    std::ostringstream errmsg;
    if (index.condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(index.condition)
               << ", otherwise there is negative array index " << index.intvalue << ".";
    else
        errmsg << "Array index " << index.intvalue << " is out of bounds.";
    const auto severity = index.errorSeverity() && index.isKnown() ? Severity::error : Severity::warning;
    const auto certainty = index.isInconclusive() ? Certainty::inconclusive : Certainty::normal;
    reportError(std::move(errorPath), severity, "negativeContainerIndex", errmsg.str(), CWE786, certainty);
}

void CheckStl::erase()
{
    logChecker("CheckStl::erase");

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type == ScopeType::eFor && Token::simpleMatch(scope.classDef, "for (")) {
            const Token *tok = scope.classDef->linkAt(1);
            if (!Token::Match(tok->tokAt(-3), "; ++| %var% ++| ) {"))
                continue;
            tok = tok->previous();
            if (!tok->isName())
                tok = tok->previous();
            eraseCheckLoopVar(scope, tok->variable());
        } else if (scope.type == ScopeType::eWhile && Token::Match(scope.classDef, "while ( %var% !=")) {
            eraseCheckLoopVar(scope, scope.classDef->tokAt(2)->variable());
        }
    }
}

void CheckStl::eraseCheckLoopVar(const Scope &scope, const Variable *var)
{
    bool inconclusiveType=false;
    if (!isIterator(var, inconclusiveType))
        return;
    for (const Token *tok = scope.bodyStart; tok != scope.bodyEnd; tok = tok->next()) {
        if (tok->str() != "(")
            continue;
        if (!Token::Match(tok->tokAt(-2), ". erase ( ++| %varid% )", var->declarationId()))
            continue;
        // Vector erases are handled by invalidContainer check
        if (isVector(tok->tokAt(-3)))
            continue;
        if (Token::Match(tok->astParent(), "=|return"))
            continue;
        // Iterator is invalid..
        int indentlevel = 0U;
        const Token *tok2 = tok->link();
        for (; tok2 != scope.bodyEnd; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                ++indentlevel;
                continue;
            }
            if (tok2->str() == "}") {
                if (indentlevel > 0U)
                    --indentlevel;
                else if (Token::simpleMatch(tok2, "} else {"))
                    tok2 = tok2->linkAt(2);
                continue;
            }
            if (tok2->varId() == var->declarationId()) {
                if (Token::simpleMatch(tok2->next(), "="))
                    break;
                dereferenceErasedError(tok, tok2, tok2->str(), inconclusiveType);
                break;
            }
            if (indentlevel == 0U && Token::Match(tok2, "break|return|goto"))
                break;
        }
        if (tok2 == scope.bodyEnd)
            dereferenceErasedError(tok, scope.classDef, var->nameToken()->str(), inconclusiveType);
    }
}

void CheckStl::stlBoundaries()
{
    logChecker("CheckStl::stlBoundaries");

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->scope() || !var->scope()->isExecutable())
            continue;

        const Library::Container* container = mSettings->library.detectIterator(var->typeStartToken());
        if (!container || container->opLessAllowed)
            continue;

        const Token* const end = var->scope()->bodyEnd;
        for (const Token *tok = var->nameToken(); tok != end; tok = tok->next()) {
            if (Token::Match(tok, "!!* %varid% <", var->declarationId())) {
                stlBoundariesError(tok);
            } else if (Token::Match(tok, "> %varid% !!.", var->declarationId())) {
                stlBoundariesError(tok);
            }
        }
    }
}

// Error message for bad boundary usage..
void CheckStl::stlBoundariesError(const Token *tok)
{
    reportError(tok, Severity::error, "stlBoundaries",
                "Dangerous comparison using operator< on iterator.\n"
                "Iterator compared with operator<. This is dangerous since the order of items in the "
                "container is not guaranteed. One should use operator!= instead to compare iterators.", CWE664, Certainty::normal);
}

static bool if_findCompare(const Token * const tokBack, bool stdStringLike)
{
    const Token *tok = tokBack->astParent();
    if (!tok)
        return true;
    if (tok->isComparisonOp()) {
        if (stdStringLike) {
            const Token * const tokOther = tokBack->astSibling();
            return !tokOther || !tokOther->hasKnownIntValue() || tokOther->getKnownIntValue() != 0;
        }
        return (!tok->astOperand1()->isNumber() && !tok->astOperand2()->isNumber());
    }
    if (tok->isArithmeticalOp()) // result is used in some calculation
        return true;  // TODO: check if there is a comparison of the result somewhere
    if (tok->str() == ".")
        return true; // Dereferencing is OK, the programmer might know that the element exists - TODO: An inconclusive warning might be appropriate
    if (tok->isAssignmentOp())
        return if_findCompare(tok, stdStringLike); // Go one step upwards in the AST
    return false;
}

void CheckStl::if_find()
{
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    const bool printPerformance = mSettings->severity.isEnabled(Severity::performance);
    if (!printWarning && !printPerformance)
        return;

    logChecker("CheckStl::if_find"); // warning,performance

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if ((scope.type != ScopeType::eIf && scope.type != ScopeType::eWhile) || !scope.classDef)
            continue;

        const Token *conditionStart = scope.classDef->next();
        if (Token::simpleMatch(conditionStart->astOperand2(), ";"))
            conditionStart = conditionStart->astOperand2();

        for (const Token *tok = conditionStart; tok->str() != "{"; tok = tok->next()) {
            const Token* funcTok = nullptr;
            const Library::Container* container = nullptr;

            if (Token::Match(tok, "%name% ("))
                tok = tok->linkAt(1);

            else if (tok->variable() && Token::Match(tok, "%var% . %name% (")) {
                container = mSettings->library.detectContainer(tok->variable()->typeStartToken());
                funcTok = tok->tokAt(2);
            }

            // check also for vector-like or pointer containers
            else if (tok->variable() && tok->astParent() && (tok->astParent()->str() == "*" || tok->astParent()->str() == "[")) {
                const Token *tok2 = tok->astParent();

                if (!Token::Match(tok2->astParent(), ". %name% ("))
                    continue;

                funcTok = tok2->astParent()->next();

                if (tok->variable()->isArrayOrPointer())
                    container = mSettings->library.detectContainer(tok->variable()->typeStartToken());
                else { // Container of container - find the inner container
                    container = mSettings->library.detectContainer(tok->variable()->typeStartToken()); // outer container
                    tok2 = Token::findsimplematch(tok->variable()->typeStartToken(), "<", tok->variable()->typeEndToken());
                    if (container && container->type_templateArgNo >= 0 && tok2) {
                        tok2 = tok2->next();
                        for (int j = 0; j < container->type_templateArgNo; j++)
                            tok2 = tok2->nextTemplateArgument();

                        container = mSettings->library.detectContainer(tok2); // inner container
                    } else
                        container = nullptr;
                }
            }

            Library::Container::Action action{};
            if (container &&
                ((action = container->getAction(funcTok->str())) == Library::Container::Action::FIND || action == Library::Container::Action::FIND_CONST)) {
                if (if_findCompare(funcTok->next(), container->stdStringLike))
                    continue;

                if (printWarning && container->getYield(funcTok->str()) == Library::Container::Yield::ITERATOR)
                    if_findError(tok, false);
                else if (printPerformance && container->stdStringLike && funcTok->str() == "find")
                    if_findError(tok, true);
            } else if (printWarning && Token::Match(tok, "std :: find|find_if (")) {
                // check that result is checked properly
                if (!if_findCompare(tok->tokAt(3), false)) {
                    if_findError(tok, false);
                }
            }
        }
    }
}


void CheckStl::if_findError(const Token *tok, bool str)
{
    if (str && mSettings->standards.cpp >= Standards::CPP20)
        reportError(tok, Severity::performance, "stlIfStrFind",
                    "Inefficient usage of string::find() in condition; string::starts_with() could be faster.\n"
                    "Either inefficient or wrong usage of string::find(). string::starts_with() will be faster if "
                    "string::find's result is compared with 0, because it will not scan the whole "
                    "string. If your intention is to check that there are no findings in the string, "
                    "you should compare with std::string::npos.", CWE597, Certainty::normal);
    if (!str)
        reportError(tok, Severity::warning, "stlIfFind", "Suspicious condition. The result of find() is an iterator, but it is not properly checked.", CWE398, Certainty::normal);
}

static std::pair<const Token *, const Token *> isMapFind(const Token *tok)
{
    if (!Token::simpleMatch(tok, "("))
        return {};
    if (!Token::simpleMatch(tok->astOperand1(), "."))
        return {};
    if (!astIsContainer(tok->astOperand1()->astOperand1()))
        return {};
    const Token * contTok = tok->astOperand1()->astOperand1();
    const Library::Container * container = contTok->valueType()->container;
    if (!container)
        return {};
    if (!container->stdAssociativeLike)
        return {};
    if (!Token::Match(tok->astOperand1(), ". find|count ("))
        return {};
    if (!tok->astOperand2())
        return {};
    return {contTok, tok->astOperand2()};
}

static const Token* skipLocalVars(const Token* const tok)
{
    if (!tok)
        return tok;
    if (Token::simpleMatch(tok, "{"))
        return skipLocalVars(tok->next());

    if (tok->isAssignmentOp()) {
        const Token *top = tok->astTop();
        const Token *varTok = top->astOperand1();
        const Variable *var = varTok->variable();
        if (!var)
            return tok;
        if (var->scope() != tok->scope())
            return tok;
        const Token *endTok = nextAfterAstRightmostLeaf(top);
        if (!endTok)
            return tok;
        return skipLocalVars(endTok->next());
    }
    return tok;
}

static const Token *findInsertValue(const Token *tok, const Token *containerTok, const Token *keyTok, const Settings &settings)
{
    const Token *startTok = skipLocalVars(tok);
    const Token *top = startTok->astTop();

    const Token *icontainerTok = nullptr;
    const Token *ikeyTok = nullptr;
    const Token *ivalueTok = nullptr;
    if (Token::simpleMatch(top, "=") && Token::simpleMatch(top->astOperand1(), "[")) {
        icontainerTok = top->astOperand1()->astOperand1();
        ikeyTok = top->astOperand1()->astOperand2();
        ivalueTok = top->astOperand2();
    }
    if (Token::simpleMatch(top, "(") && Token::Match(top->astOperand1(), ". insert|emplace (") && !astIsIterator(top->astOperand1()->tokAt(2))) {
        icontainerTok = top->astOperand1()->astOperand1();
        const Token *itok = top->astOperand1()->tokAt(2)->astOperand2();
        if (Token::simpleMatch(itok, ",")) {
            ikeyTok = itok->astOperand1();
            ivalueTok = itok->astOperand2();
        } else {
            ikeyTok = itok;
        }
    }
    if (!ikeyTok || !icontainerTok)
        return nullptr;
    if (isSameExpression(true, containerTok, icontainerTok, settings, true, false) &&
        isSameExpression(true, keyTok, ikeyTok, settings, true, true)) {
        if (ivalueTok)
            return ivalueTok;
        return ikeyTok;
    }
    return nullptr;
}

void CheckStl::checkFindInsert()
{
    if (!mSettings->severity.isEnabled(Severity::performance))
        return;

    logChecker("CheckStl::checkFindInsert"); // performance

    const SymbolDatabase *const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "if ("))
                continue;
            if (!Token::simpleMatch(tok->linkAt(1), ") {"))
                continue;
            if (!Token::Match(tok->next()->astOperand2(), "%comp%"))
                continue;
            const Token *condTok = tok->next()->astOperand2();
            const Token *containerTok;
            const Token *keyTok;
            std::tie(containerTok, keyTok) = isMapFind(condTok->astOperand1());
            if (!containerTok)
                continue;
            // In < C++17 we only warn for small simple types
            if (mSettings->standards.cpp < Standards::CPP17 && !(keyTok && keyTok->valueType() && (keyTok->valueType()->isIntegral() || keyTok->valueType()->pointer > 0)))
                continue;

            const Token *thenTok = tok->linkAt(1)->next();
            const Token *valueTok = findInsertValue(thenTok, containerTok, keyTok, *mSettings);
            if (!valueTok)
                continue;

            if (Token::simpleMatch(thenTok->link(), "} else {")) {
                const Token *valueTok2 =
                    findInsertValue(thenTok->link()->tokAt(2), containerTok, keyTok, *mSettings);
                if (!valueTok2)
                    continue;
                if (isSameExpression(true, valueTok, valueTok2, *mSettings, true, true)) {
                    checkFindInsertError(valueTok);
                }
            } else {
                checkFindInsertError(valueTok);
            }
        }
    }
}

void CheckStl::checkFindInsertError(const Token *tok)
{
    std::string replaceExpr;
    if (tok && Token::simpleMatch(tok->astParent(), "=") && tok == tok->astParent()->astOperand2() && Token::simpleMatch(tok->astParent()->astOperand1(), "[")) {
        if (mSettings->standards.cpp < Standards::CPP11)
            // We will recommend using emplace/try_emplace instead
            return;
        const std::string f = (mSettings->standards.cpp < Standards::CPP17) ? "emplace" : "try_emplace";
        replaceExpr = " Instead of '" + tok->astParent()->expressionString() + "' consider using '" +
                      tok->astParent()->astOperand1()->astOperand1()->expressionString() +
                      "." + f + "(" +
                      tok->astParent()->astOperand1()->astOperand2()->expressionString() +
                      ", " +
                      tok->expressionString() +
                      ");'.";
    }

    reportError(
        tok, Severity::performance, "stlFindInsert", "Searching before insertion is not necessary." + replaceExpr, CWE398, Certainty::normal);
}

/**
 * Is container.size() slow?
 */
static bool isCpp03ContainerSizeSlow(const Token *tok)
{
    if (!tok)
        return false;
    const Variable* var = tok->variable();
    return var && var->isStlType("list");
}

void CheckStl::size()
{
    if (!mSettings->severity.isEnabled(Severity::performance))
        return;

    if (mSettings->standards.cpp >= Standards::CPP11)
        return;

    logChecker("CheckStl::size"); // performance,c++03

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% . size ( )") ||
                Token::Match(tok, "%name% . %var% . size ( )")) {
                // get the variable
                const Token *varTok = tok;
                if (tok->strAt(2) != "size")
                    varTok = varTok->tokAt(2);

                const Token* const end = varTok->tokAt(5);

                // check for comparison to zero
                if ((!tok->previous()->isArithmeticalOp() && Token::Match(end, "==|<=|!=|> 0")) ||
                    (end->next() && !end->next()->isArithmeticalOp() && Token::Match(tok->tokAt(-2), "0 ==|>=|!=|<"))) {
                    if (isCpp03ContainerSizeSlow(varTok)) {
                        sizeError(varTok);
                        continue;
                    }
                }

                // check for comparison to one
                if ((!tok->previous()->isArithmeticalOp() && Token::Match(end, ">=|< 1") && !end->tokAt(2)->isArithmeticalOp()) ||
                    (end->next() && !end->next()->isArithmeticalOp() && Token::Match(tok->tokAt(-2), "1 <=|>") && !tok->tokAt(-3)->isArithmeticalOp())) {
                    if (isCpp03ContainerSizeSlow(varTok))
                        sizeError(varTok);
                }

                // check for using as boolean expression
                else if ((Token::Match(tok->tokAt(-2), "if|while (") && end->str() == ")") ||
                         (tok->previous()->tokType() == Token::eLogicalOp && Token::Match(end, "&&|)|,|;|%oror%"))) {
                    if (isCpp03ContainerSizeSlow(varTok))
                        sizeError(varTok);
                }
            }
        }
    }
}

void CheckStl::sizeError(const Token *tok)
{
    const std::string varname = tok ? tok->str() : std::string("list");
    reportError(tok, Severity::performance, "stlSize",
                "$symbol:" + varname + "\n"
                "Possible inefficient checking for '$symbol' emptiness.\n"
                "Checking for '$symbol' emptiness might be inefficient. "
                "Using $symbol.empty() instead of $symbol.size() can be faster. "
                "$symbol.size() can take linear time but $symbol.empty() is "
                "guaranteed to take constant time.", CWE398, Certainty::normal);
}

void CheckStl::redundantCondition()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("redundantIfRemove"))
        return;

    logChecker("CheckStl::redundantCondition"); // style

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eIf)
            continue;

        const Token* tok = scope.classDef->tokAt(2);
        if (!Token::Match(tok, "%name% . find ( %any% ) != %name% . end|rend|cend|crend ( ) ) { %name% . remove|erase ( %any% ) ;"))
            continue;

        // Get tokens for the fields %name% and %any%
        const Token *var1 = tok;
        const Token *any1 = var1->tokAt(4);
        const Token *var2 = any1->tokAt(3);
        const Token *var3 = var2->tokAt(7);
        const Token *any2 = var3->tokAt(4);

        // Check if all the "%name%" fields are the same and if all the "%any%" are the same..
        if (var1->str() == var2->str() &&
            var2->str() == var3->str() &&
            any1->str() == any2->str()) {
            redundantIfRemoveError(tok);
        }
    }
}

void CheckStl::redundantIfRemoveError(const Token *tok)
{
    reportError(tok, Severity::style, "redundantIfRemove",
                "Redundant checking of STL container element existence before removing it.\n"
                "Redundant checking of STL container element existence before removing it. "
                "It is safe to call the remove method on a non-existing element.", CWE398, Certainty::normal);
}

void CheckStl::missingComparison()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckStl::missingComparison"); // warning

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eFor || !scope.classDef)
            continue;

        for (const Token *tok2 = scope.classDef->tokAt(2); tok2 != scope.bodyStart; tok2 = tok2->next()) {
            if (tok2->str() == ";")
                break;

            if (!Token::Match(tok2, "%var% = %name% . begin|rbegin|cbegin|crbegin ( ) ; %name% != %name% . end|rend|cend|crend ( ) ; ++| %name% ++| ) {"))
                continue;

            // same container
            if (tok2->strAt(2) != tok2->strAt(10))
                break;

            const int iteratorId(tok2->varId());

            // same iterator
            if (iteratorId == tok2->tokAt(10)->varId())
                break;

            // increment iterator
            if (!Token::Match(tok2->tokAt(16), "++ %varid% )", iteratorId) &&
                !Token::Match(tok2->tokAt(16), "%varid% ++ )", iteratorId)) {
                break;
            }

            const Token *incrementToken = nullptr;
            // Parse loop..
            for (const Token *tok3 = scope.bodyStart; tok3 != scope.bodyEnd; tok3 = tok3->next()) {
                if (tok3->varId() == iteratorId) {
                    if (Token::Match(tok3, "%varid% = %name% . insert ( ++| %varid% ++| ,", iteratorId)) {
                        // skip insertion..
                        tok3 = tok3->linkAt(6);
                        if (!tok3)
                            break;
                    } else if (Token::simpleMatch(tok3->astParent(), "++"))
                        incrementToken = tok3;
                    else if (Token::simpleMatch(tok3->astParent(), "+")) {
                        if (Token::Match(tok3->astSibling(), "%num%")) {
                            const Token* tokenGrandParent = tok3->astParent()->astParent();
                            if (Token::Match(tokenGrandParent, "==|!="))
                                break;
                        }
                    } else if (Token::Match(tok3->astParent(), "==|!="))
                        incrementToken = nullptr;
                } else if (tok3->str() == "break" || tok3->str() == "return")
                    incrementToken = nullptr;
            }
            if (incrementToken)
                missingComparisonError(incrementToken, tok2->tokAt(16));
        }
    }
}

void CheckStl::missingComparisonError(const Token *incrementToken1, const Token *incrementToken2)
{
    std::list<const Token*> callstack = { incrementToken1,incrementToken2 };

    std::ostringstream errmsg;
    errmsg << "Missing bounds check for extra iterator increment in loop.\n"
           << "The iterator incrementing is suspicious - it is incremented at line ";
    if (incrementToken1)
        errmsg << incrementToken1->linenr();
    errmsg << " and then at line ";
    if (incrementToken2)
        errmsg << incrementToken2->linenr();
    errmsg << ". The loop might unintentionally skip an element in the container. "
           << "There is no comparison between these increments to prevent that the iterator is "
           << "incremented beyond the end.";

    reportError(callstack, Severity::warning, "StlMissingComparison", errmsg.str(), CWE834, Certainty::normal);
}


static bool isLocal(const Token *tok)
{
    const Variable *var = tok->variable();
    return var && !var->isStatic() && var->isLocal();
}

static bool isc_strCall(const Token* tok, const Library::Container* container)
{
    if (!Token::simpleMatch(tok, "("))
        return false;
    const Token* dot = tok->astOperand1();
    if (!Token::simpleMatch(dot, "."))
        return false;
    const Token* obj = dot->astOperand1();
    if (!obj || !obj->valueType())
        return false;
    const Library::Container* objContainer = obj->valueType()->container;
    if (!objContainer || !container || !objContainer->stdStringLike || (objContainer != container && !container->view))
        return false;
    return Token::Match(dot->astOperand2(), "c_str|data ( )");
}

static bool isc_strConcat(const Token* tok)
{
    if (!tok->isBinaryOp() || !Token::simpleMatch(tok, "+"))
        return false;
    for (const Token* op : { tok->astOperand1(), tok->astOperand2() }) { // NOLINT(readability-use-anyofallof)
        const Token* sibling = op->astSibling();
        if (!sibling->valueType())
            continue;
        if (isc_strCall(op, sibling->valueType()->container))
            return true;
    }
    return false;
}

static bool isc_strInPlusChain(const Token* tok, const Library::Container* container)
{
    if (!tok || !tok->valueType() || !tok->valueType()->pointer)
        return false;
    bool result = false;
    visitAstNodes(tok, [&](const Token* tok2) {
        if (Token::simpleMatch(tok2, "+"))
            return ChildrenToVisit::op1_and_op2;
        if (isc_strCall(tok2, container)) {
            result = true;
            return ChildrenToVisit::done;
        }
        return ChildrenToVisit::none;
    });
    return result;
}

static bool isc_strAssignment(const Token* tok)
{
    if (!Token::simpleMatch(tok, "="))
        return false;
    const Token* strTok = tok->astOperand1();
    if (!strTok || !strTok->valueType())
        return false;
    return isc_strCall(tok->astOperand2(), strTok->valueType()->container);
}

static bool isc_strConstructor(const Token* tok)
{
    if (!tok->valueType() || !Token::Match(tok, "%var% (|{"))
        return false;
    return isc_strInPlusChain(tok->tokAt(1)->astOperand2(), tok->valueType()->container);
}

namespace {
    const std::set<std::string> stl_string_stream = {
        "istringstream", "ostringstream", "stringstream", "wstringstream"
    };
}

void CheckStl::string_c_str()
{
    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const bool printPerformance = mSettings->severity.isEnabled(Severity::performance);

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();

    logChecker("CheckStl::string_c_str");

    // Find all functions that take std::string as argument
    struct StrArg {
        nonneg int n;
        std::string argtype;
    };
    std::multimap<const Function*, StrArg> c_strFuncParam;
    if (printPerformance) {
        for (const Scope &scope : symbolDatabase->scopeList) {
            for (const Function &func : scope.functionList) {
                nonneg int numpar = 0;
                for (const Variable &var : func.argumentList) {
                    numpar++;
                    if ((var.isStlStringType() || var.isStlStringViewType()) && (!var.isReference() || var.isConst()))
                        c_strFuncParam.emplace(&func, StrArg{ numpar, var.getTypeName() });
                }
            }
        }
    }

    auto isString = [](const Token* str) -> bool {
        while (Token::Match(str, "::|."))
            str = str->astOperand2();
        if (Token::Match(str, "(|[") && !(str->valueType() && str->valueType()->type == ValueType::ITERATOR))
            str = str->previous();
        return str && ((str->variable() && str->variable()->isStlStringType()) || // variable
                       (str->function() && isStlStringType(str->function()->retDef)) || // function returning string
                       (str->valueType() && str->valueType()->type == ValueType::ITERATOR && isStlStringType(str->valueType()->containerTypeToken))); // iterator pointing to string
    };

    // Try to detect common problems when using string::c_str()
    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != ScopeType::eFunction || !scope.function)
            continue;

        enum : std::uint8_t {charPtr, stdString, stdStringConstRef, Other} returnType = Other;
        if (Token::Match(scope.function->tokenDef->tokAt(-2), "char|wchar_t *"))
            returnType = charPtr;
        else if (Token::Match(scope.function->tokenDef->tokAt(-5), "const std :: string|wstring &"))
            returnType = stdStringConstRef;
        else if (Token::Match(scope.function->tokenDef->tokAt(-3), "std :: string|wstring !!&"))
            returnType = stdString;

        for (const Token *tok = scope.bodyStart; tok && tok != scope.bodyEnd; tok = tok->next()) {
            // Invalid usage..
            if (Token::Match(tok, "throw %var% . c_str|data ( ) ;") && isLocal(tok->next()) &&
                tok->next()->variable() && tok->next()->variable()->isStlStringType()) {
                string_c_strThrowError(tok);
            } else if (tok->variable() && tok->strAt(1) == "=") {
                if (Token::Match(tok->tokAt(2), "%var% . str ( ) . c_str|data ( ) ;")) {
                    const Variable* var = tok->variable();
                    const Variable* var2 = tok->tokAt(2)->variable();
                    if (var->isPointer() && var2 && var2->isStlType(stl_string_stream))
                        string_c_strError(tok);
                } else if (printPerformance && isc_strAssignment(tok->tokAt(1))) {
                    string_c_strAssignment(tok, tok->variable()->getTypeName());
                } else if (Token::Match(tok->tokAt(2), "%name% (") &&
                           Token::Match(tok->linkAt(3), ") . c_str|data ( ) ;") &&
                           tok->tokAt(2)->function() && Token::Match(tok->tokAt(2)->function()->retDef, "std :: string|wstring %name%")) {
                    const Variable* var = tok->variable();
                    if (var->isPointer())
                        string_c_strError(tok);
                }
            } else if (printPerformance && tok->function() && Token::Match(tok, "%name% ( !!)") && tok->str() != scope.className) {
                const auto range = c_strFuncParam.equal_range(tok->function());
                for (std::multimap<const Function*, StrArg>::const_iterator i = range.first; i != range.second; ++i) {
                    if (i->second.n == 0)
                        continue;

                    const Token* tok2 = tok->tokAt(2);
                    int j;
                    for (j = 0; tok2 && j < i->second.n - 1; j++)
                        tok2 = tok2->nextArgument();
                    if (tok2)
                        tok2 = tok2->nextArgument();
                    else
                        break;
                    if (!tok2 && j == i->second.n - 1)
                        tok2 = tok->linkAt(1);
                    else if (tok2)
                        tok2 = tok2->previous();
                    else
                        break;
                    if (tok2 && Token::Match(tok2->tokAt(-4), ". c_str|data ( )")) {
                        if (isString(tok2->tokAt(-4)->astOperand1())) {
                            string_c_strParam(tok, i->second.n, i->second.argtype);
                        } else if (Token::Match(tok2->tokAt(-9), "%name% . str ( )")) { // Check ss.str().c_str() as parameter
                            const Variable* ssVar = tok2->tokAt(-9)->variable();
                            if (ssVar && ssVar->isStlType(stl_string_stream))
                                string_c_strParam(tok, i->second.n, i->second.argtype);
                        }
                    }
                }
            } else if (printPerformance && isc_strConstructor(tok)) {
                string_c_strConstructor(tok, tok->variable()->getTypeName());
            } else if (printPerformance && isc_strConcat(tok)) {
                string_c_strConcat(tok);
            } else if (printPerformance && Token::simpleMatch(tok, "<<") && tok->astOperand2() && Token::Match(tok->astOperand2()->astOperand1(), ". c_str|data ( )")) {
                const Token* str = tok->astOperand2()->astOperand1()->astOperand1();
                if (isString(str)) {
                    const Token* strm = tok;
                    while (Token::simpleMatch(strm, "<<"))
                        strm = strm->astOperand1();
                    if (strm && ((strm->variable() && strm->variable()->isStlType()) || Token::Match(strm->tokAt(-1), "std :: cout|cerr")))
                        string_c_strStream(tok);
                }
            }

            // Using c_str() to get the return value is only dangerous if the function returns a char*
            else if ((returnType == charPtr || (printPerformance && (returnType == stdString || returnType == stdStringConstRef))) && tok->str() == "return") {
                bool err = false;

                const Token* tok2 = tok->next();
                if (Token::Match(tok2, "std :: string|wstring (") &&
                    Token::Match(tok2->linkAt(3), ") . c_str|data ( ) ;")) {
                    err = true;
                } else if (Token::simpleMatch(tok2, "(") &&
                           Token::Match(tok2->link(), ") . c_str|data ( ) ;")) {
                    // Check for "+ localvar" or "+ std::string(" inside the bracket
                    bool is_implicit_std_string = printInconclusive;
                    const Token *search_end = tok2->link();
                    for (const Token *search_tok = tok2->next(); search_tok != search_end; search_tok = search_tok->next()) {
                        if (Token::Match(search_tok, "+ %var%") && isLocal(search_tok->next()) &&
                            search_tok->next()->variable() && search_tok->next()->variable()->isStlStringType()) {
                            is_implicit_std_string = true;
                            break;
                        }
                        if (Token::Match(search_tok, "+ std :: string|wstring (")) {
                            is_implicit_std_string = true;
                            break;
                        }
                    }

                    if (is_implicit_std_string)
                        err = true;
                }

                bool local = false;
                bool ptrOrRef = false;
                const Variable* lastVar = nullptr;
                const Function* lastFunc = nullptr;
                bool funcStr = false;
                if (Token::Match(tok2, "%var% .")) {
                    local = isLocal(tok2);
                    bool refToNonLocal = false;
                    if (tok2->variable() && tok2->variable()->isReference()) {
                        const Token *refTok = tok2->variable()->nameToken();
                        refToNonLocal = true; // safe assumption is default to avoid FPs
                        if (Token::Match(refTok, "%var% = %var% .|;|["))
                            refToNonLocal = !isLocal(refTok->tokAt(2));
                    }
                    ptrOrRef = refToNonLocal || (tok2->variable() && (tok2->variable()->isPointer() || tok2->variable()->isSmartPointer()));
                }
                while (tok2) {
                    if (Token::Match(tok2, "%var% .|::")) {
                        if (ptrOrRef)
                            local = false;
                        lastVar = tok2->variable();
                        tok2 = tok2->tokAt(2);
                    } else if (Token::Match(tok2, "%name% (") && Token::simpleMatch(tok2->linkAt(1), ") .")) {
                        lastFunc = tok2->function();
                        local = false;
                        funcStr = tok2->str() == "str";
                        tok2 = tok2->linkAt(1)->tokAt(2);
                    } else
                        break;
                }

                if (Token::Match(tok2, "c_str|data ( ) ;")) {
                    if ((local || returnType != charPtr) && lastVar && lastVar->isStlStringType())
                        err = true;
                    else if (funcStr && lastVar && lastVar->isStlType(stl_string_stream))
                        err = true;
                    else if (lastFunc && Token::Match(lastFunc->tokenDef->tokAt(-3), "std :: string|wstring"))
                        err = true;
                }

                if (err) {
                    if (returnType == charPtr)
                        string_c_strError(tok);
                    else
                        string_c_strReturn(tok);
                }
            }
        }
    }
}

void CheckStl::string_c_strThrowError(const Token* tok)
{
    reportError(tok, Severity::error, "stlcstrthrow", "Dangerous usage of c_str(). The value returned by c_str() is invalid after throwing exception.\n"
                "Dangerous usage of c_str(). The string is destroyed after the c_str() call so the thrown pointer is invalid.");
}

void CheckStl::string_c_strError(const Token* tok)
{
    reportError(tok, Severity::error, "stlcstr", "Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n"
                "Dangerous usage of c_str(). The c_str() return value is only valid until its string is deleted.", CWE664, Certainty::normal);
}

void CheckStl::string_c_strReturn(const Token* tok)
{
    reportError(tok, Severity::performance, "stlcstrReturn", "Returning the result of c_str() in a function that returns std::string is slow and redundant.\n"
                "The conversion from const char* as returned by c_str() to std::string creates an unnecessary string copy. Solve that by directly returning the string.", CWE704, Certainty::normal);
}

void CheckStl::string_c_strParam(const Token* tok, nonneg int number, const std::string& argtype)
{
    std::ostringstream oss;
    oss << "Passing the result of c_str() to a function that takes " << argtype << " as argument no. " << number << " is slow and redundant.\n"
        "The conversion from const char* as returned by c_str() to " << argtype << " creates an unnecessary string copy or length calculation. Solve that by directly passing the string.";
    reportError(tok, Severity::performance, "stlcstrParam", oss.str(), CWE704, Certainty::normal);
}

void CheckStl::string_c_strConstructor(const Token* tok, const std::string& argtype)
{
    std::string msg = "Constructing a " + argtype + " from the result of c_str() is slow and redundant.\n"
                      "Constructing a " + argtype + " from const char* requires a call to strlen(). Solve that by directly passing the string.";
    reportError(tok, Severity::performance, "stlcstrConstructor", msg, CWE704, Certainty::normal);
}

void CheckStl::string_c_strAssignment(const Token* tok, const std::string& argtype)
{
    std::string msg = "Assigning the result of c_str() to a " + argtype + " is slow and redundant.\n"
                      "Assigning a const char* to a " + argtype + " requires a call to strlen(). Solve that by directly assigning the string.";
    reportError(tok, Severity::performance, "stlcstrAssignment", msg, CWE704, Certainty::normal);
}

void CheckStl::string_c_strConcat(const Token* tok)
{
    std::string msg = "Concatenating the result of c_str() and a std::string is slow and redundant.\n"
                      "Concatenating a const char* with a std::string requires a call to strlen(). Solve that by directly concatenating the strings.";
    reportError(tok, Severity::performance, "stlcstrConcat", msg, CWE704, Certainty::normal);
}

void CheckStl::string_c_strStream(const Token* tok)
{
    std::string msg = "Passing the result of c_str() to a stream is slow and redundant.\n"
                      "Passing a const char* to a stream requires a call to strlen(). Solve that by directly passing the string.";
    reportError(tok, Severity::performance, "stlcstrStream", msg, CWE704, Certainty::normal);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

namespace {
    const std::set<std::string> stl_containers_with_empty_and_clear = {
        "deque",  "forward_list",  "list",
        "map",  "multimap",  "multiset",  "set",  "string",
        "unordered_map",  "unordered_multimap",  "unordered_multiset",
        "unordered_set",  "vector",  "wstring"
    };

}

void CheckStl::uselessCalls()
{
    const bool printPerformance = mSettings->severity.isEnabled(Severity::performance);
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    if (!printPerformance && !printWarning)
        return;

    logChecker("CheckStl::uselessCalls"); // performance,warning

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (printWarning && Token::Match(tok, "%var% . compare|find|rfind|find_first_not_of|find_first_of|find_last_not_of|find_last_of ( %name% [,)]") &&
                tok->varId() == tok->tokAt(4)->varId()) {
                const Variable* var = tok->variable();
                if (!var || !var->isStlType())
                    continue;
                uselessCallsReturnValueError(tok->tokAt(4), tok->str(), tok->strAt(2));
            } else if (printPerformance && Token::Match(tok, "%var% . swap ( %name% )") &&
                       tok->varId() == tok->tokAt(4)->varId()) {
                const Variable* var = tok->variable();
                if (!var || !var->isStlType())
                    continue;
                uselessCallsSwapError(tok, tok->str());
            } else if (printPerformance && Token::Match(tok, "%var% . substr (") && tok->variable() && tok->variable()->isStlStringType()) {
                const Token* funcTok = tok->tokAt(3);
                const std::vector<const Token*> args = getArguments(funcTok);
                if (Token::Match(tok->tokAt(-2), "%var% =") && tok->varId() == tok->tokAt(-2)->varId() &&
                    !args.empty() && args[0]->hasKnownIntValue() && args[0]->getKnownIntValue() == 0) {
                    uselessCallsSubstrError(tok, Token::simpleMatch(funcTok->astParent(), "=") ? SubstrErrorType::PREFIX : SubstrErrorType::PREFIX_CONCAT);
                } else if (args.empty() || (args[0]->hasKnownIntValue() && args[0]->getKnownIntValue() == 0 &&
                                            (args.size() == 1 || (args.size() == 2 && tok->linkAt(3)->strAt(-1) == "npos" && !tok->linkAt(3)->previous()->variable())))) {
                    uselessCallsSubstrError(tok, SubstrErrorType::COPY);
                } else if (args.size() == 2 && args[1]->hasKnownIntValue() && args[1]->getKnownIntValue() == 0) {
                    uselessCallsSubstrError(tok, SubstrErrorType::EMPTY);
                }
            } else if (printWarning && Token::Match(tok, "[{};] %var% . empty ( ) ;") &&
                       !tok->tokAt(4)->astParent() &&
                       tok->next()->variable() && tok->next()->variable()->isStlType(stl_containers_with_empty_and_clear))
                uselessCallsEmptyError(tok->next());
            else if (Token::Match(tok, "[{};] std :: remove|remove_if|unique (") && tok->tokAt(5)->nextArgument())
                uselessCallsRemoveError(tok->next(), tok->strAt(3));
            else if (printPerformance && tok->valueType() && tok->valueType()->type == ValueType::CONTAINER) {
                if (Token::Match(tok, "%var% = { %var% . begin ( ) ,") && tok->varId() == tok->tokAt(3)->varId())
                    uselessCallsConstructorError(tok);
                else if (const Variable* var = tok->variable()) {
                    std::string pattern = "%var% = ";
                    for (const Token* t = var->typeStartToken(); t != var->typeEndToken()->next(); t = t->next()) {
                        pattern += t->str();
                        pattern += ' ';
                    }
                    pattern += "{|( %varid% . begin ( ) ,";
                    if (Token::Match(tok, pattern.c_str(), tok->varId()))
                        uselessCallsConstructorError(tok);
                }
            }
        }
    }
}


void CheckStl::uselessCallsReturnValueError(const Token *tok, const std::string &varname, const std::string &function)
{
    std::ostringstream errmsg;
    errmsg << "$symbol:" << varname << '\n';
    errmsg << "$symbol:" << function << '\n';
    errmsg << "It is inefficient to call '" << varname << "." << function << "(" << varname << ")' as it always returns 0.\n"
           << "'std::string::" << function << "()' returns zero when given itself as parameter "
           << "(" << varname << "." << function << "(" << varname << ")). As it is currently the "
           << "code is inefficient. It is possible either the string searched ('"
           << varname << "') or searched for ('" << varname << "') is wrong.";
    reportError(tok, Severity::warning, "uselessCallsCompare", errmsg.str(), CWE628, Certainty::normal);
}

void CheckStl::uselessCallsSwapError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::performance, "uselessCallsSwap",
                "$symbol:" + varname + "\n"
                "It is inefficient to swap a object with itself by calling '$symbol.swap($symbol)'\n"
                "The 'swap()' function has no logical effect when given itself as parameter "
                "($symbol.swap($symbol)). As it is currently the "
                "code is inefficient. Is the object or the parameter wrong here?", CWE628, Certainty::normal);
}

void CheckStl::uselessCallsSubstrError(const Token *tok, SubstrErrorType type)
{
    std::string msg = "Ineffective call of function 'substr' because ";
    switch (type) {
    case SubstrErrorType::EMPTY:
        msg += "it returns an empty string.";
        break;
    case SubstrErrorType::COPY:
        msg += "it returns a copy of the object. Use operator= instead.";
        break;
    case SubstrErrorType::PREFIX:
        msg += "a prefix of the string is assigned to itself. Use resize() or pop_back() instead.";
        break;
    case SubstrErrorType::PREFIX_CONCAT:
        msg += "a prefix of the string is assigned to itself. Use replace() instead.";
        break;
    }
    reportError(tok, Severity::performance, "uselessCallsSubstr", msg, CWE398, Certainty::normal);
}

void CheckStl::uselessCallsConstructorError(const Token *tok)
{
    const std::string container = tok ? tok->str() : "";
    const std::string msg = "Inefficient constructor call: container '" + container + "' is assigned a partial copy of itself. Use erase() or resize() instead.";
    reportError(tok, Severity::performance, "uselessCallsConstructor", msg, CWE398, Certainty::normal);
}

void CheckStl::uselessCallsEmptyError(const Token *tok)
{
    reportError(tok, Severity::warning, "uselessCallsEmpty", "Ineffective call of function 'empty()'. Did you intend to call 'clear()' instead?", CWE398, Certainty::normal);
}

void CheckStl::uselessCallsRemoveError(const Token *tok, const std::string& function)
{
    reportError(tok, Severity::warning, "uselessCallsRemove",
                "$symbol:" + function + "\n"
                "Return value of std::$symbol() ignored. Elements remain in container.\n"
                "The return value of std::$symbol() is ignored. This function returns an iterator to the end of the range containing those elements that should be kept. "
                "Elements past new end remain valid but with unspecified values. Use the erase method of the container to delete them.", CWE762, Certainty::normal);
}

// Check for iterators being dereferenced before being checked for validity.
// E.g.  if (*i && i != str.end()) { }
void CheckStl::checkDereferenceInvalidIterator()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    logChecker("CheckStl::checkDereferenceInvalidIterator"); // warning

    // Iterate over "if", "while", and "for" conditions where there may
    // be an iterator that is dereferenced before being checked for validity.
    for (const Scope &scope : mTokenizer->getSymbolDatabase()->scopeList) {
        if (!(scope.type == ScopeType::eIf || scope.isLoopScope()))
            continue;

        const Token* const tok = scope.classDef;
        const Token* startOfCondition = tok->next();
        if (scope.type == ScopeType::eDo)
            startOfCondition = startOfCondition->link()->tokAt(2);
        if (!startOfCondition) // ticket #6626 invalid code
            continue;
        const Token* endOfCondition = startOfCondition->link();
        if (!endOfCondition)
            continue;

        // For "for" loops, only search between the two semicolons
        if (scope.type == ScopeType::eFor) {
            startOfCondition = Token::findsimplematch(tok->tokAt(2), ";", endOfCondition);
            if (!startOfCondition)
                continue;
            endOfCondition = Token::findsimplematch(startOfCondition->next(), ";", endOfCondition);
            if (!endOfCondition)
                continue;
        }

        // Only consider conditions composed of all "&&" terms and
        // conditions composed of all "||" terms
        const bool isOrExpression =
            Token::findsimplematch(startOfCondition, "||", endOfCondition) != nullptr;
        const bool isAndExpression =
            Token::findsimplematch(startOfCondition, "&&", endOfCondition) != nullptr;

        // Look for a check of the validity of an iterator
        const Token* validityCheckTok = nullptr;
        if (!isOrExpression && isAndExpression) {
            validityCheckTok =
                Token::findmatch(startOfCondition, "&& %var% != %name% . end|rend|cend|crend ( )", endOfCondition);
        } else if (isOrExpression && !isAndExpression) {
            validityCheckTok =
                Token::findmatch(startOfCondition, "%oror% %var% == %name% . end|rend|cend|crend ( )", endOfCondition);
        }

        if (!validityCheckTok)
            continue;
        const int iteratorVarId = validityCheckTok->next()->varId();

        // If the iterator dereference is to the left of the check for
        // the iterator's validity, report an error.
        const Token* const dereferenceTok =
            Token::findmatch(startOfCondition, "* %varid%", validityCheckTok, iteratorVarId);
        if (dereferenceTok)
            dereferenceInvalidIteratorError(dereferenceTok, dereferenceTok->strAt(1));
    }
}


void CheckStl::checkDereferenceInvalidIterator2()
{
    const bool printInconclusive = (mSettings->certainty.isEnabled(Certainty::inconclusive));

    logChecker("CheckStl::checkDereferenceInvalidIterator2");

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof|decltype|typeid|typeof (")) {
            tok = tok->linkAt(1);
            continue;
        }

        if (Token::Match(tok, "%assign%"))
            continue;

        std::vector<ValueFlow::Value> contValues;
        std::copy_if(tok->values().cbegin(), tok->values().cend(), std::back_inserter(contValues), [&](const ValueFlow::Value& value) {
            if (value.isImpossible())
                return false;
            if (!printInconclusive && value.isInconclusive())
                return false;
            return value.isContainerSizeValue();
        });


        // Can iterator point to END or before START?
        for (const ValueFlow::Value& value:tok->values()) {
            if (value.isImpossible())
                continue;
            if (!printInconclusive && value.isInconclusive())
                continue;
            if (!value.isIteratorValue())
                continue;
            bool isInvalidIterator = false;
            const ValueFlow::Value* cValue = nullptr;
            if (value.isIteratorEndValue() && value.intvalue >= 0) {
                isInvalidIterator = value.intvalue > 0;
            } else if (value.isIteratorStartValue() && value.intvalue < 0) {
                isInvalidIterator = true;
            } else {
                auto it = std::find_if(contValues.cbegin(), contValues.cend(), [&](const ValueFlow::Value& c) {
                    if (value.path != c.path)
                        return false;
                    if (value.isIteratorStartValue() && value.intvalue >= c.intvalue)
                        return true;
                    if (value.isIteratorEndValue() && -value.intvalue > c.intvalue)
                        return true;
                    return false;
                });
                if (it == contValues.end())
                    continue;
                cValue = &*it;
                if (value.isIteratorStartValue() && value.intvalue > cValue->intvalue)
                    isInvalidIterator = true;
            }
            bool inconclusive = false;
            bool unknown = false;
            const Token* emptyAdvance = nullptr;
            const Token* advanceIndex = nullptr;
            if (cValue && cValue->intvalue == 0) {
                if (Token::Match(tok->astParent(), "+|-") && astIsIntegral(tok->astSibling(), false)) {
                    if (tok->astSibling() && tok->astSibling()->hasKnownIntValue()) {
                        if (tok->astSibling()->getKnownIntValue() == 0)
                            continue;
                    } else {
                        advanceIndex = tok->astSibling();
                    }
                    emptyAdvance = tok->astParent();
                } else if (Token::Match(tok->astParent(), "++|--")) {
                    emptyAdvance = tok->astParent();
                }
            }
            if (!CheckNullPointer::isPointerDeRef(tok, unknown, *mSettings) && !isInvalidIterator && !emptyAdvance) {
                if (!unknown)
                    continue;
                inconclusive = true;
            }
            if (cValue) {
                const ValueFlow::Value& lValue = getLifetimeIteratorValue(tok, cValue->path);
                if (!lValue.isLifetimeValue())
                    continue;
                if (emptyAdvance)
                    outOfBoundsError(emptyAdvance,
                                     lValue.tokvalue->expressionString(),
                                     cValue,
                                     advanceIndex ? advanceIndex->expressionString() : "",
                                     nullptr);
                else
                    outOfBoundsError(tok, lValue.tokvalue->expressionString(), cValue, tok->expressionString(), &value);
            } else {
                dereferenceInvalidIteratorError(tok, &value, inconclusive);
            }
        }
    }
}

void CheckStl::dereferenceInvalidIteratorError(const Token* tok, const ValueFlow::Value *value, bool inconclusive)
{
    const std::string& varname = tok ? tok->expressionString() : "var";
    const std::string errmsgcond("$symbol:" + varname + '\n' + ValueFlow::eitherTheConditionIsRedundant(value ? value->condition : nullptr) + " or there is possible dereference of an invalid iterator: $symbol.");
    if (!tok || !value) {
        reportError(tok, Severity::error, "derefInvalidIterator", "Dereference of an invalid iterator", CWE825, Certainty::normal);
        reportError(tok, Severity::warning, "derefInvalidIteratorRedundantCheck", errmsgcond, CWE825, Certainty::normal);
        return;
    }
    if (!mSettings->isEnabled(value, inconclusive))
        return;

    ErrorPath errorPath = getErrorPath(tok, value, "Dereference of an invalid iterator");

    if (value->condition) {
        reportError(std::move(errorPath), Severity::warning, "derefInvalidIteratorRedundantCheck", errmsgcond, CWE825, (inconclusive || value->isInconclusive()) ? Certainty::inconclusive : Certainty::normal);
    } else {
        std::string errmsg = std::string(value->isKnown() ? "Dereference" : "Possible dereference") + " of an invalid iterator";
        if (!varname.empty())
            errmsg = "$symbol:" + varname + '\n' + errmsg + ": $symbol";

        reportError(std::move(errorPath),
                    value->isKnown() ? Severity::error : Severity::warning,
                    "derefInvalidIterator",
                    errmsg,
                    CWE825, (inconclusive || value->isInconclusive()) ? Certainty::inconclusive : Certainty::normal);
    }
}

void CheckStl::dereferenceInvalidIteratorError(const Token* deref, const std::string &iterName)
{
    reportError(deref, Severity::warning,
                "derefInvalidIterator",
                "$symbol:" + iterName + "\n"
                "Possible dereference of an invalid iterator: $symbol\n"
                "Possible dereference of an invalid iterator: $symbol. Make sure to check that the iterator is valid before dereferencing it - not after.", CWE825, Certainty::normal);
}

void CheckStl::useStlAlgorithmError(const Token *tok, const std::string &algoName)
{
    reportError(tok, Severity::style, "useStlAlgorithm",
                "Consider using " + algoName + " algorithm instead of a raw loop.", CWE398, Certainty::normal);
}

static bool isEarlyExit(const Token *start)
{
    if (start->str() != "{")
        return false;
    const Token *endToken = start->link();
    const Token *tok = Token::findmatch(start, "return|throw|break|continue", endToken);
    if (!tok || tok->scope() != start->scope() || tok->str() == "continue")
        return false;
    const Token *endStatement = Token::findsimplematch(tok, "; }", endToken);
    if (!endStatement)
        return false;
    if (endStatement->next() != endToken)
        return false;
    return true;
}

static const Token *singleStatement(const Token *start)
{
    if (start->str() != "{")
        return nullptr;
    const Token *endToken = start->link();
    const Token *endStatement = Token::findsimplematch(start->next(), ";");
    if (!Token::simpleMatch(endStatement, "; }"))
        return nullptr;
    if (endStatement->next() != endToken)
        return nullptr;
    return endStatement;
}

enum class LoopType : std::uint8_t { OTHER, RANGE, ITERATOR, INDEX };

static const Token *singleAssignInScope(const Token *start, nonneg int varid, bool &input, bool &hasBreak, LoopType loopType, const Settings& settings)
{
    const Token *endStatement = singleStatement(start);
    if (!endStatement)
        return nullptr;
    if (!Token::Match(start->next(), "%var% %assign%"))
        return nullptr;
    const Token *assignTok = start->tokAt(2);
    if (isVariableChanged(assignTok->next(), endStatement, assignTok->astOperand1()->varId(), /*globalvar*/ false, settings))
        return nullptr;
    if (isVariableChanged(assignTok->next(), endStatement, varid, /*globalvar*/ false, settings))
        return nullptr;
    input = Token::findmatch(assignTok->next(), "%varid%", endStatement, varid) || !Token::Match(start->next(), "%var% =");
    hasBreak = Token::simpleMatch(endStatement->previous(), "break");

    if (loopType == LoopType::INDEX) { // check for container access
        nonneg int containerId{};
        for (const Token* tok = assignTok->next(); tok != endStatement; tok = tok->next()) {
            if (tok->varId() == varid) {
                if (!Token::simpleMatch(tok->astParent(), "["))
                    return nullptr;
                const Token* contTok = tok->astParent()->astOperand1();
                if (!contTok->valueType() || !contTok->valueType()->container || contTok->varId() == 0)
                    return nullptr;
                if (containerId > 0 && containerId != contTok->varId()) // allow only one container
                    return nullptr;
                containerId = contTok->varId();
            }
        }
        return containerId > 0 ? assignTok : nullptr;
    }
    return assignTok;
}

static const Token *singleMemberCallInScope(const Token *start, nonneg int varid, bool &input, const Settings& settings)
{
    if (start->str() != "{")
        return nullptr;
    const Token *endToken = start->link();
    if (!Token::Match(start->next(), "%var% . %name% ("))
        return nullptr;
    if (!Token::simpleMatch(start->linkAt(4), ") ; }"))
        return nullptr;
    const Token *endStatement = start->linkAt(4)->next();
    if (endStatement->next() != endToken)
        return nullptr;

    const Token *dotTok = start->tokAt(2);
    if (!Token::findmatch(dotTok->tokAt(2), "%varid%", endStatement, varid))
        return nullptr;
    input = Token::Match(start->next(), "%var% . %name% ( %varid% )", varid);
    if (isVariableChanged(dotTok->next(), endStatement, dotTok->astOperand1()->varId(), /*globalvar*/ false, settings))
        return nullptr;
    return dotTok;
}

static const Token *singleIncrementInScope(const Token *start, nonneg int varid, bool &input)
{
    if (start->str() != "{")
        return nullptr;
    const Token *varTok = nullptr;
    if (Token::Match(start->next(), "++ %var% ; }"))
        varTok = start->tokAt(2);
    else if (Token::Match(start->next(), "%var% ++ ; }"))
        varTok = start->tokAt(1);
    if (!varTok)
        return nullptr;
    input = varTok->varId() == varid;
    return varTok;
}

static const Token *singleConditionalInScope(const Token *start, nonneg int varid, LoopType loopType, const Settings& settings)
{
    if (start->str() != "{")
        return nullptr;
    const Token *endToken = start->link();
    if (!Token::simpleMatch(start->next(), "if ("))
        return nullptr;
    if (!Token::simpleMatch(start->linkAt(2), ") {"))
        return nullptr;
    const Token *bodyTok = start->linkAt(2)->next();
    const Token *endBodyTok = bodyTok->link();
    if (!Token::simpleMatch(endBodyTok, "} }"))
        return nullptr;
    if (endBodyTok->next() != endToken)
        return nullptr;
    if (!Token::findmatch(start, "%varid%", bodyTok, varid))
        return nullptr;
    if (isVariableChanged(start, bodyTok, varid, /*globalvar*/ false, settings))
        return nullptr;
    if (loopType == LoopType::INDEX) { // check for container access
        nonneg int containerId{};
        for (const Token* tok = start->tokAt(2); tok != start->linkAt(2); tok = tok->next()) {
            if (tok->varId() == varid) {
                if (!Token::simpleMatch(tok->astParent(), "["))
                    return nullptr;
                const Token* contTok = tok->astParent()->astOperand1();
                if (!contTok->valueType() || !contTok->valueType()->container || contTok->varId() == 0)
                    return nullptr;
                if (containerId > 0 && containerId != contTok->varId()) // allow only one container
                    return nullptr;
                containerId = contTok->varId();
            }
        }
        return containerId > 0 ? bodyTok : nullptr;
    }
    return bodyTok;
}

static bool addByOne(const Token *tok, nonneg int varid)
{
    if (Token::Match(tok, "+= %any% ;") &&
        tok->tokAt(1)->hasKnownIntValue() &&
        tok->tokAt(1)->getValue(1)) {
        return true;
    }
    if (Token::Match(tok, "= %varid% + %any% ;", varid) &&
        tok->tokAt(3)->hasKnownIntValue() &&
        tok->tokAt(3)->getValue(1)) {
        return true;
    }
    return false;
}

static bool accumulateBoolLiteral(const Token *tok, nonneg int varid)
{
    if (Token::Match(tok, "%assign% %bool% ;") &&
        tok->tokAt(1)->hasKnownIntValue()) {
        return true;
    }
    if (Token::Match(tok, "= %varid% %oror%|%or%|&&|& %bool% ;", varid) &&
        tok->tokAt(3)->hasKnownIntValue()) {
        return true;
    }
    return false;
}

static bool accumulateBool(const Token *tok, nonneg int varid)
{
    // Missing %oreq% so we have to check both manually
    if (Token::simpleMatch(tok, "&=") || Token::simpleMatch(tok, "|=")) {
        return true;
    }
    if (Token::Match(tok, "= %varid% %oror%|%or%|&&|&", varid)) {
        return true;
    }
    return false;
}

static bool hasVarIds(const Token *tok, nonneg int var1, nonneg int var2)
{
    if (tok->astOperand1()->varId() == tok->astOperand2()->varId())
        return false;
    if (tok->astOperand1()->varId() == var1 || tok->astOperand1()->varId() == var2) {
        if (tok->astOperand2()->varId() == var1 || tok->astOperand2()->varId() == var2) {
            return true;
        }
    }
    return false;
}

static std::string flipMinMax(const std::string &algo)
{
    if (algo == "std::max_element")
        return "std::min_element";
    if (algo == "std::min_element")
        return "std::max_element";
    return algo;
}

static std::string minmaxCompare(const Token *condTok, nonneg int loopVar, nonneg int assignVar, LoopType loopType, bool invert = false)
{
    if (loopType == LoopType::RANGE && !hasVarIds(condTok, loopVar, assignVar))
        return "std::accumulate";
    std::string algo = "std::max_element";
    if (Token::Match(condTok, "<|<="))
        algo = "std::min_element";
    if (condTok->astOperand1()->varId() == assignVar)
        algo = flipMinMax(algo);
    if (invert)
        algo = flipMinMax(algo);
    return algo;
}

static bool isTernaryAssignment(const Token* assignTok, nonneg int loopVarId, nonneg int assignVarId, LoopType loopType, std::string& algo)
{
    if (!Token::simpleMatch(assignTok->astOperand2(), "?"))
        return false;
    const Token* condTok = assignTok->astOperand2()->astOperand1();
    if (!Token::Match(condTok, "<|<=|>=|>"))
        return false;

    const Token* colon = assignTok->astOperand2()->astOperand2();
    if (loopType == LoopType::RANGE) {
        if (!(condTok->astOperand1()->varId() && condTok->astOperand2()->varId() && colon->astOperand1()->varId() && colon->astOperand2()->varId()))
            return false;
    }
    else if (loopType == LoopType::INDEX) {
        int nVar = 0, nCont = 0;
        for (const Token* tok : { condTok->astOperand1(), condTok->astOperand2(), colon->astOperand1(), colon->astOperand2() }) {
            if (tok->varId())
                ++nVar;
            else if (tok->str() == "[" && tok->astOperand1()->varId() && tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->container &&
                     tok->astOperand2()->varId() == loopVarId)
                ++nCont;
        }
        if (nVar != 2 || nCont != 2)
            return false;
    }
    else
        return false;

    algo = minmaxCompare(condTok, loopVarId, assignVarId, loopType, colon->astOperand1()->varId() == assignVarId);
    return true;
}

namespace {
    struct LoopAnalyzer {
        const Token* bodyTok = nullptr;
        const Token* loopVar = nullptr;
        const Settings* settings = nullptr;
        std::set<nonneg int> varsChanged;

        explicit LoopAnalyzer(const Token* tok, const Settings* psettings)
            : bodyTok(tok->linkAt(1)->next()), settings(psettings)
        {
            const Token* splitTok = tok->next()->astOperand2();
            if (Token::simpleMatch(splitTok, ":") && splitTok->previous()->varId() != 0) {
                loopVar = splitTok->previous();
            }
            if (valid()) {
                findChangedVariables();
            }
        }
        bool isLoopVarChanged() const {
            return varsChanged.count(loopVar->varId()) > 0;
        }

        bool isModified(const Token* tok) const
        {
            if (tok->variable() && tok->variable()->isConst())
                return false;
            int n = 1 + (astIsPointer(tok) ? 1 : 0);
            for (int i = 0; i < n; i++) {
                bool inconclusive = false;
                if (isVariableChangedByFunctionCall(tok, i, *settings, &inconclusive))
                    return true;
                if (inconclusive)
                    return true;
                if (isVariableChanged(tok, i, *settings))
                    return true;
            }
            return false;
        }

        template<class Predicate, class F>
        void findTokens(Predicate pred, F f) const
        {
            for (const Token* tok = bodyTok; precedes(tok, bodyTok->link()); tok = tok->next()) {
                if (pred(tok))
                    f(tok);
            }
        }

        template<class Predicate>
        const Token* findToken(Predicate pred) const
        {
            for (const Token* tok = bodyTok; precedes(tok, bodyTok->link()); tok = tok->next()) {
                if (pred(tok))
                    return tok;
            }
            return nullptr;
        }

        bool hasGotoOrBreak() const
        {
            return findToken([](const Token* tok) {
                return Token::Match(tok, "goto|break");
            });
        }

        bool valid() const {
            return bodyTok && loopVar;
        }

        std::string findAlgo() const
        {
            if (!valid())
                return "";
            bool loopVarChanged = isLoopVarChanged();
            if (!loopVarChanged && varsChanged.empty()) {
                if (hasGotoOrBreak())
                    return "";
                bool alwaysTrue = true;
                bool alwaysFalse = true;
                auto hasReturn = [](const Token* tok) {
                    return Token::simpleMatch(tok, "return");
                };
                findTokens(hasReturn, [&](const Token* tok) {
                    const Token* returnTok = tok->astOperand1();
                    if (!returnTok || !returnTok->hasKnownIntValue() || !astIsBool(returnTok)) {
                        alwaysTrue = false;
                        alwaysFalse = false;
                        return;
                    }
                    (returnTok->getKnownIntValue() ? alwaysTrue : alwaysFalse) &= true;
                    (returnTok->getKnownIntValue() ? alwaysFalse : alwaysTrue) &= false;
                });
                if (alwaysTrue == alwaysFalse)
                    return "";
                if (alwaysTrue)
                    return "std::any_of";
                return "std::all_of or std::none_of";
            }
            return "";
        }

        bool isLocalVar(const Variable* var) const
        {
            if (!var)
                return false;
            if (var->isPointer() || var->isReference())
                return false;
            if (var->declarationId() == loopVar->varId())
                return false;
            const Scope* scope = var->scope();
            return scope && scope->isNestedIn(bodyTok->scope());
        }

    private:
        void findChangedVariables()
        {
            std::set<nonneg int> vars;
            for (const Token* tok = bodyTok; precedes(tok, bodyTok->link()); tok = tok->next()) {
                if (tok->varId() == 0)
                    continue;
                if (vars.count(tok->varId()) > 0)
                    continue;
                if (isLocalVar(tok->variable())) {
                    vars.insert(tok->varId());
                    continue;
                }
                if (!isModified(tok))
                    continue;
                varsChanged.insert(tok->varId());
                vars.insert(tok->varId());
            }
        }
    };
} // namespace

void CheckStl::useStlAlgorithm()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("useStlAlgorithm"))
        return;

    logChecker("CheckStl::useStlAlgorithm"); // style

    auto checkAssignee = [](const Token* tok) {
        if (astIsBool(tok)) // std::accumulate is not a good fit for bool values, std::all/any/none_of return early
            return false;
        return !astIsContainer(tok); // don't warn for containers, where overloaded operators can be costly
    };

    enum class ConditionOpType : std::uint8_t { OTHER, MIN, MAX };
    auto isConditionWithoutSideEffects = [this](const Token* tok, ConditionOpType& type) -> bool {
        if (!Token::simpleMatch(tok, "{") || !Token::simpleMatch(tok->previous(), ")"))
            return false;
        const Token* condTok = tok->linkAt(-1)->astOperand2();
        if (isConstExpression(condTok, mSettings->library)) {
            if (condTok->str() == "<")
                type = ConditionOpType::MIN;
            else if (condTok->str() == ">")
                type = ConditionOpType::MAX;
            else
                type = ConditionOpType::OTHER;
            return true;
        }
        return false;
    };

    auto isAccumulation = [](const Token* tok, int varId) {
        if (tok->str() != "=")
            return true;
        const Token* end = Token::findmatch(tok, "%varid%|;", varId); // TODO: lambdas?
        return end && end->varId() != 0;
    };

    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            // Parse range-based for loop
            if (!Token::simpleMatch(tok, "for ("))
                continue;
            if (!Token::simpleMatch(tok->linkAt(1), ") {"))
                continue;
            LoopAnalyzer a{tok, mSettings};
            std::string algoName = a.findAlgo();
            if (!algoName.empty()) {
                useStlAlgorithmError(tok, algoName);
                continue;
            }

            const Token *bodyTok = tok->linkAt(1)->next();
            const Token *splitTok = tok->next()->astOperand2();
            const Token* loopVar{};
            LoopType loopType{};
            if (Token::simpleMatch(splitTok, ":")) {
                loopVar = splitTok->previous();
                if (loopVar->varId() == 0)
                    continue;
                if (Token::simpleMatch(splitTok->astOperand2(), "{"))
                    continue;
                loopType = LoopType::RANGE;
            }
            else { // iterator-based loop?
                const Token* initTok = getInitTok(tok);
                const Token* condTok = getCondTok(tok);
                const Token* stepTok = getStepTok(tok);
                if (!initTok || !condTok || !stepTok)
                    continue;
                loopVar = Token::Match(condTok, "%comp%") ? condTok->astOperand1() : nullptr;
                if (!Token::Match(loopVar, "%var%") || !loopVar->valueType() ||
                    (loopVar->valueType()->type != ValueType::Type::ITERATOR && !loopVar->valueType()->isIntegral()))
                    continue;
                if (!Token::simpleMatch(initTok, "=") || !Token::Match(initTok->astOperand1(), "%varid%", loopVar->varId()))
                    continue;
                if (!stepTok->isIncDecOp())
                    continue;
                loopType = (loopVar->valueType()->type == ValueType::Type::ITERATOR) ? LoopType::ITERATOR : LoopType::INDEX;
            }

            // Check for single assignment
            bool useLoopVarInAssign{}, hasBreak{};
            const Token *assignTok = singleAssignInScope(bodyTok, loopVar->varId(), useLoopVarInAssign, hasBreak, loopType, *mSettings);
            if (assignTok) {
                if (!checkAssignee(assignTok->astOperand1()))
                    continue;
                const int assignVarId = assignTok->astOperand1()->varId();
                std::string algo;
                if (assignVarId == loopVar->varId()) {
                    if (useLoopVarInAssign)
                        algo = "std::transform";
                    else if (Token::Match(assignTok->next(), "%var%|%bool%|%num%|%char% ;"))
                        algo = "std::fill";
                    else if (Token::Match(assignTok->next(), "%name% ( )"))
                        algo = "std::generate";
                    else
                        algo = "std::fill or std::generate";
                } else {
                    if (addByOne(assignTok, assignVarId))
                        algo = "std::distance";
                    else if (accumulateBool(assignTok, assignVarId))
                        algo = "std::any_of, std::all_of, std::none_of, or std::accumulate";
                    else if (isTernaryAssignment(assignTok, loopVar->varId(), assignVarId, loopType, algo))
                        ;
                    else if (isAccumulation(assignTok, assignVarId))
                        algo = "std::accumulate";
                    else
                        continue;
                }
                useStlAlgorithmError(assignTok, algo);
                continue;
            }
            // Check for container calls
            bool useLoopVarInMemCall;
            const Token *memberAccessTok = singleMemberCallInScope(bodyTok, loopVar->varId(), useLoopVarInMemCall, *mSettings);
            if (memberAccessTok && loopType == LoopType::RANGE) {
                const Token *memberCallTok = memberAccessTok->astOperand2();
                const int contVarId = memberAccessTok->astOperand1()->varId();
                if (contVarId == loopVar->varId())
                    continue;
                if (memberCallTok->str() == "push_back" ||
                    memberCallTok->str() == "push_front" ||
                    memberCallTok->str() == "emplace_back") {
                    std::string algo;
                    if (useLoopVarInMemCall)
                        algo = "std::copy";
                    else
                        algo = "std::transform";
                    useStlAlgorithmError(memberCallTok, algo);
                }
                continue;
            }

            // Check for increment in loop
            bool useLoopVarInIncrement;
            const Token *incrementTok = singleIncrementInScope(bodyTok, loopVar->varId(), useLoopVarInIncrement);
            if (incrementTok) {
                std::string algo;
                if (useLoopVarInIncrement)
                    algo = "std::transform";
                else
                    algo = "std::distance";
                useStlAlgorithmError(incrementTok, algo);
                continue;
            }

            // Check for conditionals
            const Token *condBodyTok = singleConditionalInScope(bodyTok, loopVar->varId(), loopType, *mSettings);
            if (condBodyTok) {
                // Check for single assign
                assignTok = singleAssignInScope(condBodyTok, loopVar->varId(), useLoopVarInAssign, hasBreak, loopType, *mSettings);
                if (assignTok) {
                    if (!checkAssignee(assignTok->astOperand1()))
                        continue;
                    const int assignVarId = assignTok->astOperand1()->varId();
                    std::string algo;
                    if (assignVarId == loopVar->varId()) {
                        if (useLoopVarInAssign)
                            algo = "std::transform";
                        else
                            algo = "std::replace_if";
                    } else {
                        ConditionOpType type{};
                        if (addByOne(assignTok, assignVarId))
                            algo = "std::count_if";
                        else if (accumulateBoolLiteral(assignTok, assignVarId))
                            algo = "std::any_of, std::all_of, std::none_of, or std::accumulate";
                        else if (assignTok->str() != "=")
                            algo = "std::accumulate";
                        else if (isConditionWithoutSideEffects(condBodyTok, type)) {
                            if (hasBreak)
                                algo = "std::any_of, std::all_of, std::none_of";
                            else if (assignTok->astOperand2()->varId() == loopVar->varId()) {
                                if (type == ConditionOpType::MIN)
                                    algo = "std::min_element";
                                else if (type == ConditionOpType::MAX)
                                    algo = "std::max_element";
                                else
                                    continue;
                            }
                            else
                                continue;
                        }
                        else
                            continue;
                    }
                    useStlAlgorithmError(assignTok, algo);
                    continue;
                }

                // Check for container call
                memberAccessTok = singleMemberCallInScope(condBodyTok, loopVar->varId(), useLoopVarInMemCall, *mSettings);
                if (memberAccessTok) {
                    const Token *memberCallTok = memberAccessTok->astOperand2();
                    const int contVarId = memberAccessTok->astOperand1()->varId();
                    if (contVarId == loopVar->varId())
                        continue;
                    if (memberCallTok->str() == "push_back" ||
                        memberCallTok->str() == "push_front" ||
                        memberCallTok->str() == "emplace_back") {
                        if (useLoopVarInMemCall)
                            useStlAlgorithmError(memberAccessTok, "std::copy_if");
                        // There is no transform_if to suggest
                    }
                    continue;
                }

                // Check for increment in loop
                incrementTok = singleIncrementInScope(condBodyTok, loopVar->varId(), useLoopVarInIncrement);
                if (incrementTok) {
                    std::string algo;
                    if (useLoopVarInIncrement)
                        algo = "std::transform";
                    else
                        algo = "std::count_if";
                    useStlAlgorithmError(incrementTok, algo);
                    continue;
                }

                // Check early return
                if (isEarlyExit(condBodyTok)) {
                    const Token *loopVar2 = Token::findmatch(condBodyTok, "%varid%", condBodyTok->link(), loopVar->varId());
                    std::string algo;
                    if (loopVar2 ||
                        (loopType == LoopType::ITERATOR && loopVar->variable() && precedes(loopVar->variable()->nameToken(), tok))) // iterator declared outside the loop
                        algo = "std::find_if";
                    else
                        algo = "std::any_of";
                    useStlAlgorithmError(condBodyTok, algo);
                    continue;
                }
            }
        }
    }
}

void CheckStl::knownEmptyContainerError(const Token *tok, const std::string& algo)
{
    const std::string var = tok ? tok->expressionString() : std::string("var");

    std::string msg;
    if (astIsIterator(tok)) {
        msg = "Using " + algo + " with iterator '" + var + "' that is always empty.";
    } else {
        msg = "Iterating over container '" + var + "' that is always empty.";
    }

    reportError(tok, Severity::style,
                "knownEmptyContainer",
                msg, CWE398, Certainty::normal);
}

static bool isKnownEmptyContainer(const Token* tok)
{
    if (!tok)
        return false;
    return std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& v) {
        if (!v.isKnown())
            return false;
        if (!v.isContainerSizeValue())
            return false;
        if (v.intvalue != 0)
            return false;
        return true;
    });
}

void CheckStl::knownEmptyContainer()
{
    if (!mSettings->severity.isEnabled(Severity::style) && !mSettings->isPremiumEnabled("knownEmptyContainer"))
        return;
    logChecker("CheckStl::knownEmptyContainer"); // style
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {

            if (!Token::Match(tok, "%name% ( !!)"))
                continue;

            // Parse range-based for loop
            if (tok->str() == "for") {
                if (!Token::simpleMatch(tok->linkAt(1), ") {"))
                    continue;
                const Token *splitTok = tok->next()->astOperand2();
                if (!Token::simpleMatch(splitTok, ":"))
                    continue;
                const Token* contTok = splitTok->astOperand2();
                if (!isKnownEmptyContainer(contTok))
                    continue;
                knownEmptyContainerError(contTok, "");
            } else {
                const std::vector<const Token *> args = getArguments(tok);
                if (args.empty())
                    continue;

                for (int argnr = 1; argnr <= args.size(); ++argnr) {
                    const Library::ArgumentChecks::IteratorInfo *i = mSettings->library.getArgIteratorInfo(tok, argnr);
                    if (!i)
                        continue;
                    const Token * const argTok = args[argnr - 1];
                    if (!isKnownEmptyContainer(argTok))
                        continue;
                    knownEmptyContainerError(argTok, tok->str());
                    break;

                }
            }
        }
    }
}

void CheckStl::eraseIteratorOutOfBoundsError(const Token *ftok, const Token* itertok, const ValueFlow::Value* val)
{
    if (!ftok || !itertok || !val) {
        reportError(ftok, Severity::error, "eraseIteratorOutOfBounds",
                    "Calling function 'erase()' on the iterator 'iter' which is out of bounds.", CWE628, Certainty::normal);
        reportError(ftok, Severity::warning, "eraseIteratorOutOfBoundsCond",
                    "Either the condition 'x' is redundant or function 'erase()' is called on the iterator 'iter' which is out of bounds.", CWE628, Certainty::normal);
        return;
    }
    const std::string& func = ftok->str();
    const std::string iter = itertok->expressionString();

    const bool isConditional = val->isPossible();
    std::string msg;
    if (isConditional) {
        msg = ValueFlow::eitherTheConditionIsRedundant(val->condition) + " or function '" + func + "()' is called on the iterator '" + iter + "' which is out of bounds.";
    } else {
        msg = "Calling function '" + func + "()' on the iterator '" + iter + "' which is out of bounds.";
    }

    const Severity severity = isConditional ? Severity::warning : Severity::error;
    const std::string id = isConditional ? "eraseIteratorOutOfBoundsCond" : "eraseIteratorOutOfBounds";
    reportError(ftok, severity,
                id,
                msg, CWE628, Certainty::normal);
}

static const ValueFlow::Value* getOOBIterValue(const Token* tok, const ValueFlow::Value* sizeVal)
{
    auto it = std::find_if(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& v) {
        if (v.isPossible() || v.isKnown()) {
            switch (v.valueType) {
            case ValueFlow::Value::ValueType::ITERATOR_END:
                return v.intvalue >= 0;
            case ValueFlow::Value::ValueType::ITERATOR_START:
                return (v.intvalue < 0) || (sizeVal && v.intvalue >= sizeVal->intvalue);
            default:
                break;
            }
        }
        return false;
    });
    return it != tok->values().end() ? &*it : nullptr;
}

void CheckStl::eraseIteratorOutOfBounds()
{
    logChecker("CheckStl::eraseIteratorOutOfBounds");
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {

            if (!tok->valueType())
                continue;
            const Library::Container* container = tok->valueType()->container;
            if (!container || !astIsLHS(tok) || !Token::simpleMatch(tok->astParent(), "."))
                continue;
            const Token* const ftok = tok->astParent()->astOperand2();
            const Library::Container::Action action = container->getAction(ftok->str());
            if (action != Library::Container::Action::ERASE)
                continue;
            const std::vector<const Token*> args = getArguments(ftok);
            if (args.size() != 1) // TODO: check range overload
                continue;

            const ValueFlow::Value* sizeVal = tok->getKnownValue(ValueFlow::Value::ValueType::CONTAINER_SIZE);
            if (const ValueFlow::Value* errVal = getOOBIterValue(args[0], sizeVal))
                eraseIteratorOutOfBoundsError(ftok, args[0], errVal);
        }
    }
}

static bool isMutex(const Variable* var)
{
    const Token* tok = Token::typeDecl(var->nameToken()).first;
    return Token::Match(tok, "std :: mutex|recursive_mutex|timed_mutex|recursive_timed_mutex|shared_mutex");
}

static bool isLockGuard(const Variable* var)
{
    const Token* tok = Token::typeDecl(var->nameToken()).first;
    return Token::Match(tok, "std :: lock_guard|unique_lock|scoped_lock|shared_lock");
}

static bool isLocalMutex(const Variable* var, const Scope* scope)
{
    if (!var)
        return false;
    if (isLockGuard(var))
        return false;
    return !var->isReference() && !var->isRValueReference() && !var->isStatic() && var->scope() == scope;
}

void CheckStl::globalLockGuardError(const Token* tok)
{
    reportError(tok, Severity::warning,
                "globalLockGuard",
                "Lock guard is defined globally. Lock guards are intended to be local. A global lock guard could lead to a deadlock since it won't unlock until the end of the program.", CWE833, Certainty::normal);
}

void CheckStl::localMutexError(const Token* tok)
{
    reportError(tok, Severity::warning,
                "localMutex",
                "The lock is ineffective because the mutex is locked at the same scope as the mutex itself.", CWE667, Certainty::normal);
}

void CheckStl::checkMutexes()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;
    logChecker("CheckStl::checkMutexes"); // warning
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        std::set<nonneg int> checkedVars;
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%var%"))
                continue;
            const Variable* var = tok->variable();
            if (!var)
                continue;
            if (Token::Match(tok, "%var% . lock ( )")) {
                if (!isMutex(var))
                    continue;
                if (!checkedVars.insert(var->declarationId()).second)
                    continue;
                if (isLocalMutex(var, tok->scope()))
                    localMutexError(tok);
            } else if (Token::Match(tok, "%var% (|{ %var% )|}|,")) {
                if (!isLockGuard(var))
                    continue;
                const Variable* mvar = tok->tokAt(2)->variable();
                if (!mvar)
                    continue;
                if (!checkedVars.insert(mvar->declarationId()).second)
                    continue;
                if (var->isStatic() || var->isGlobal())
                    globalLockGuardError(tok);
                else if (isLocalMutex(mvar, tok->scope()))
                    localMutexError(tok);
            }
        }
    }
}

void CheckStl::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    if (!tokenizer.isCPP()) {
        return;
    }

    CheckStl checkStl(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkStl.erase();
    checkStl.if_find();
    checkStl.checkFindInsert();
    checkStl.iterators();
    checkStl.missingComparison();
    checkStl.outOfBounds();
    checkStl.outOfBoundsIndexExpression();
    checkStl.redundantCondition();
    checkStl.string_c_str();
    checkStl.uselessCalls();
    checkStl.useStlAlgorithm();

    checkStl.stlOutOfBounds();
    checkStl.negativeIndex();

    checkStl.invalidContainer();
    checkStl.mismatchingContainers();
    checkStl.mismatchingContainerIterator();
    checkStl.knownEmptyContainer();
    checkStl.eraseIteratorOutOfBounds();

    checkStl.stlBoundaries();
    checkStl.checkDereferenceInvalidIterator();
    checkStl.checkDereferenceInvalidIterator2();
    checkStl.checkMutexes();

    // Style check
    checkStl.size();
}

void CheckStl::getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const
{
    CheckStl c(nullptr, settings, errorLogger);
    c.outOfBoundsError(nullptr, "container", nullptr, "x", nullptr);
    c.invalidIteratorError(nullptr, "iterator");
    c.iteratorsError(nullptr, "container1", "container2");
    c.iteratorsError(nullptr, nullptr, "container");
    c.invalidContainerLoopError(nullptr, nullptr, ErrorPath{});
    c.invalidContainerError(nullptr, nullptr, ErrorPath{});
    c.invalidContainerReferenceError(nullptr, nullptr, ErrorPath{});
    c.mismatchingContainerIteratorError(nullptr, nullptr, nullptr);
    c.mismatchingContainersError(nullptr, nullptr);
    c.mismatchingContainerExpressionError(nullptr, nullptr);
    c.sameIteratorExpressionError(nullptr);
    c.dereferenceErasedError(nullptr, nullptr, "iter", false);
    c.stlOutOfBoundsError(nullptr, "i", "foo", false);
    c.negativeIndexError(nullptr, ValueFlow::Value(-1));
    c.stlBoundariesError(nullptr);
    c.if_findError(nullptr, false);
    c.if_findError(nullptr, true);
    c.checkFindInsertError(nullptr);
    c.string_c_strError(nullptr);
    c.string_c_strReturn(nullptr);
    c.string_c_strParam(nullptr, 0);
    c.string_c_strConstructor(nullptr);
    c.string_c_strAssignment(nullptr);
    c.string_c_strConcat(nullptr);
    c.string_c_strStream(nullptr);
    c.string_c_strThrowError(nullptr);
    c.sizeError(nullptr);
    c.missingComparisonError(nullptr, nullptr);
    c.redundantIfRemoveError(nullptr);
    c.uselessCallsReturnValueError(nullptr, "str", "find");
    c.uselessCallsSwapError(nullptr, "str");
    c.uselessCallsSubstrError(nullptr, SubstrErrorType::COPY);
    c.uselessCallsConstructorError(nullptr);
    c.uselessCallsEmptyError(nullptr);
    c.uselessCallsRemoveError(nullptr, "remove");
    c.dereferenceInvalidIteratorError(nullptr, "i");
    // TODO: derefInvalidIteratorRedundantCheck
    c.eraseIteratorOutOfBoundsError(nullptr, nullptr);
    c.useStlAlgorithmError(nullptr, "");
    c.knownEmptyContainerError(nullptr, "");
    c.globalLockGuardError(nullptr);
    c.localMutexError(nullptr);
    c.outOfBoundsIndexExpressionError(nullptr, nullptr);
}
