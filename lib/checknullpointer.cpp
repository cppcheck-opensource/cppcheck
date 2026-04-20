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
#include "checknullpointer.h"

#include "astutils.h"
#include "ctu.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "findtoken.h"
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <utility>
#include <vector>

//---------------------------------------------------------------------------

// CWE ids used:
static const CWE CWE_NULL_POINTER_DEREFERENCE(476U);
static const CWE CWE_INCORRECT_CALCULATION(682U);

// Register this check class (by creating a static instance of it)
namespace {
    CheckNullPointer instance;
}

//---------------------------------------------------------------------------

static bool checkNullpointerFunctionCallPlausibility(const Function* func, unsigned int arg)
{
    return !func || (func->argCount() >= arg && func->getArgumentVar(arg - 1) && func->getArgumentVar(arg - 1)->isPointer());
}

std::list<const Token*> CheckNullPointer::parseFunctionCall(const Token &tok, const Library &library, bool checkNullArg)
{
    if (Token::Match(&tok, "%name% ( )") || !tok.tokAt(2))
        return {};

    const std::vector<const Token *> args = getArguments(&tok);
    std::list<const Token*> var;
    for (int argnr = 1; argnr <= args.size(); ++argnr) {
        const Token *param = args[argnr - 1];
        if ((!checkNullArg || library.isnullargbad(&tok, argnr)) && checkNullpointerFunctionCallPlausibility(tok.function(), argnr))
            var.push_back(param);
        else if (tok.function()) {
            const Variable* argVar = tok.function()->getArgumentVar(argnr-1);
            if (argVar && argVar->isStlStringType() && !argVar->isArrayOrPointer())
                var.push_back(param);
        }
    }

    if (library.formatstr_function(&tok)) {
        const int formatStringArgNr = library.formatstr_argno(&tok);
        if (formatStringArgNr < 0 || formatStringArgNr >= args.size())
            return var;

        // 1st parameter..
        if (Token::Match(&tok, "snprintf|vsnprintf|fnprintf|vfnprintf") && args.size() > 1 && !(args[1] && args[1]->hasKnownIntValue() && args[1]->getKnownIntValue() == 0)) // Only if length (second parameter) is not zero
            var.push_back(args[0]);

        if (args[formatStringArgNr]->tokType() != Token::eString)
            return var;
        const std::string &formatString = args[formatStringArgNr]->strValue();
        int argnr = formatStringArgNr + 1;
        const bool scan = library.formatstr_scan(&tok);

        bool percent = false;
        for (auto i = formatString.cbegin(); i != formatString.cend(); ++i) {
            if (*i == '%') {
                percent = !percent;
            } else if (percent) {
                percent = false;

                bool _continue = false;
                while (!std::isalpha(static_cast<unsigned char>(*i))) {
                    if (*i == '*') {
                        if (scan)
                            _continue = true;
                        else
                            argnr++;
                    }
                    ++i;
                    if (i == formatString.end())
                        return var;
                }
                if (_continue)
                    continue;

                if (argnr < args.size() && (*i == 'n' || *i == 's' || scan))
                    var.push_back(args[argnr]);

                if (*i != 'm') // %m is a non-standard glibc extension that requires no parameter
                    argnr++;
            }
        }
    }
    return var;
}

namespace {
    const std::set<std::string> stl_stream = {
        "fstream", "ifstream", "iostream", "istream",
        "istringstream", "ofstream", "ostream", "ostringstream",
        "stringstream", "wistringstream", "wostringstream", "wstringstream"
    };
}

/**
 * Is there a pointer dereference? Everything that should result in
 * a nullpointer dereference error message will result in a true
 * return value. If it's unknown if the pointer is dereferenced false
 * is returned.
 * @param tok token for the pointer
 * @param unknown it is not known if there is a pointer dereference (could be reported as a debug message)
 * @return true => there is a dereference
 */
bool CheckNullPointer::isPointerDeRef(const Token *tok, bool &unknown) const
{
    return isPointerDeRef(tok, unknown, *mSettings);
}

bool CheckNullPointer::isPointerDeRef(const Token *tok, bool &unknown, const Settings &settings, bool checkNullArg)
{
    unknown = false;

    // Is pointer used as function parameter?
    if (Token::Match(tok->previous(), "[(,] %name% [,)]")) {
        const Token *ftok = tok->previous();
        while (ftok && ftok->str() != "(") {
            if (ftok->str() == ")")
                ftok = ftok->link();
            ftok = ftok->previous();
        }
        if (ftok && ftok->previous()) {
            const std::list<const Token *> varlist = parseFunctionCall(*ftok->previous(), settings.library, checkNullArg);
            if (std::find(varlist.cbegin(), varlist.cend(), tok) != varlist.cend()) {
                return true;
            }
        }
    }

    if (tok->str() == "(" && !tok->scope()->isExecutable())
        return false;

    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    const bool addressOf = parent->astParent() && parent->astParent()->str() == "&";
    if (parent->str() == "." && astIsRHS(tok))
        return isPointerDeRef(parent, unknown, settings);
    const bool firstOperand = parent->astOperand1() == tok;
    parent = astParentSkipParens(tok);
    if (!parent)
        return false;

    // Dereferencing pointer..
    const Token* grandParent = parent->astParent();
    if (parent->isUnaryOp("*") && !(grandParent && isUnevaluated(grandParent->previous()))) {
        // declaration of function pointer
        if (tok->variable() && tok->variable()->nameToken() == tok)
            return false;
        if (!addressOf)
            return true;
    }

    // array access
    if (firstOperand && parent->str() == "[" && !addressOf)
        return true;

    // address of member variable / array element
    const Token *parent2 = parent;
    while (Token::Match(parent2, "[|."))
        parent2 = parent2->astParent();
    if (parent2 != parent && parent2 && parent2->isUnaryOp("&"))
        return false;

    // read/write member variable
    if (firstOperand && parent->originalName() == "->" && !addressOf)
        return true;

    // If its a function pointer then check if its called
    if (tok->variable() && tok->variable()->isPointer() && Token::Match(tok->variable()->nameToken(), "%name% ) (") &&
        Token::Match(tok, "%name% ("))
        return true;

    if (Token::Match(tok, "%var% = %var% .") &&
        tok->varId() == tok->tokAt(2)->varId())
        return true;

    // std::string dereferences nullpointers
    if (Token::Match(parent->tokAt(-3), "std :: string|wstring (|{"))
        return true;
    if (Token::Match(parent->previous(), "%name% (|{")) {
        const Variable* var = parent->previous()->variable();
        if (var && !var->isPointer() && !var->isArray() && var->isStlStringType())
            return true;
    }

    // streams dereference nullpointers
    if (Token::Match(parent, "<<|>>") && !firstOperand) {
        const Variable* var = tok->variable();
        if (var && var->isPointer() && Token::Match(var->typeStartToken(), "char|wchar_t")) { // Only outputting or reading to char* can cause problems
            const Token* tok2 = parent; // Find start of statement
            for (; tok2; tok2 = tok2->previous()) {
                if (Token::Match(tok2->previous(), ";|{|}|:"))
                    break;
            }
            if (Token::Match(tok2, "std :: cout|cin|cerr"))
                return true;
            if (tok2 && tok2->varId() != 0) {
                const Variable* var2 = tok2->variable();
                if (var2 && var2->isStlType(stl_stream))
                    return true;
            }
        }
    }

    const Variable *ovar = nullptr;
    if (Token::Match(parent, "+|==|!=") || (parent->str() == "=" && !firstOperand)) {
        if (parent->astOperand1() == tok && parent->astOperand2())
            ovar = parent->astOperand2()->variable();
        else if (parent->astOperand1() && parent->astOperand2() == tok)
            ovar = parent->astOperand1()->variable();
    }
    if (ovar && !ovar->isPointer() && !ovar->isArray() && ovar->isStlStringType())
        return true;

    // assume that it's not a dereference (no false positives)
    return false;
}


static bool isNullablePointer(const Token* tok)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "new") && tok->varId() == 0)
        return false;
    if (astIsPointer(tok))
        return true;
    if (astIsSmartPointer(tok))
        return true;
    if (Token::simpleMatch(tok, "."))
        return isNullablePointer(tok->astOperand2());
    if (const Variable* var = tok->variable()) {
        return (var->isPointer() || var->isSmartPointer());
    }
    return false;
}

void CheckNullPointer::nullPointerByDeRefAndCheck()
{
    const bool printInconclusive = (mSettings->certainty.isEnabled(Certainty::inconclusive));

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        auto pred = [printInconclusive](const Token* tok) -> bool {
            if (!tok)
                return false;

            if (Token::Match(tok, "%num%|%char%|%str%"))
                return false;

            if (!isNullablePointer(tok) ||
                (tok->str() == "." && isNullablePointer(tok->astOperand2()) && tok->astOperand2()->getValue(0))) // avoid duplicate warning
                return false;

            // Can pointer be NULL?
            const ValueFlow::Value *value = tok->getValue(0);
            if (!value)
                return false;

            if (!printInconclusive && value->isInconclusive())
                return false;

            return true;
        };
        const Token* const start = (scope->function && scope->function->isConstructor()) ? scope->function->token : scope->bodyStart; // Check initialization list
        std::vector<const Token *> tokens = findTokensSkipDeadAndUnevaluatedCode(mSettings->library, start, scope->bodyEnd, pred);
        for (const Token *tok : tokens) {
            const ValueFlow::Value *value = tok->getValue(0);

            // Pointer dereference.
            bool unknown = false;
            if (!isPointerDeRef(tok, unknown)) {
                if (unknown)
                    nullPointerError(tok, tok->expressionString(), value, true);
                continue;
            }

            nullPointerError(tok, tok->expressionString(), value, value->isInconclusive());
        }
    }
}

void CheckNullPointer::nullPointer()
{
    logChecker("CheckNullPointer::nullPointer");
    nullPointerByDeRefAndCheck();
}

namespace {
    const std::set<std::string> stl_istream = {
        "fstream", "ifstream", "iostream", "istream",
        "istringstream", "stringstream", "wistringstream", "wstringstream"
    };
}

/** Dereferencing null constant (simplified token list) */
void CheckNullPointer::nullConstantDereference()
{
    logChecker("CheckNullPointer::nullConstantDereference");

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (scope->function == nullptr || !scope->function->hasBody()) // We only look for functions with a body
            continue;

        const Token *tok = scope->bodyStart;

        if (scope->function->isConstructor())
            tok = scope->function->token; // Check initialization list

        for (; tok != scope->bodyEnd; tok = tok->next()) {
            if (isUnevaluated(tok))
                tok = tok->linkAt(1);

            else if (Token::simpleMatch(tok, "* 0")) {
                if (Token::Match(tok->previous(), "return|throw|;|{|}|:|[|(|,") || tok->previous()->isOp()) {
                    nullPointerError(tok);
                }
            }

            else if (Token::Match(tok, "0 [") && (tok->strAt(-1) != "&" || !Token::Match(tok->linkAt(1)->next(), "[.(]")))
                nullPointerError(tok);

            else if (Token::Match(tok->previous(), "!!. %name% (|{") && (tok->strAt(-1) != "::" || tok->strAt(-2) == "std")) {
                if (Token::Match(tok->tokAt(2), "0|NULL|nullptr )|}") && tok->varId()) { // constructor call
                    const Variable *var = tok->variable();
                    if (var && !var->isPointer() && !var->isArray() && var->isStlStringType())
                        nullPointerError(tok);
                } else { // function call
                    const std::list<const Token *> var = parseFunctionCall(*tok, mSettings->library);

                    // is one of the var items a NULL pointer?
                    for (const Token *vartok : var) {
                        if (vartok->hasKnownIntValue() && vartok->getKnownIntValue() == 0)
                            nullPointerError(vartok);
                    }
                }
            } else if (Token::Match(tok, "std :: string|wstring ( 0|NULL|nullptr )"))
                nullPointerError(tok);

            else if (Token::Match(tok->previous(), "::|. %name% (")) {
                const std::vector<const Token *> &args = getArguments(tok);
                for (int argnr = 0; argnr < args.size(); ++argnr) {
                    const Token *argtok = args[argnr];
                    if (!argtok->hasKnownIntValue())
                        continue;
                    if (argtok->getKnownIntValue() != 0)
                        continue;
                    if (mSettings->library.isnullargbad(tok, argnr+1))
                        nullPointerError(argtok);
                }
            }

            else if (Token::Match(tok->previous(), ">> 0|NULL|nullptr")) { // Only checking input stream operations is safe here, because otherwise 0 can be an integer as well
                const Token* tok2 = tok->previous(); // Find start of statement
                for (; tok2; tok2 = tok2->previous()) {
                    if (Token::Match(tok2->previous(), ";|{|}|:|("))
                        break;
                }
                if (tok2 && tok2->previous() && tok2->strAt(-1)=="(")
                    continue;
                if (Token::simpleMatch(tok2, "std :: cin"))
                    nullPointerError(tok);
                if (tok2 && tok2->varId() != 0) {
                    const Variable *var = tok2->variable();
                    if (var && var->isStlType(stl_istream))
                        nullPointerError(tok);
                }
            }

            const Variable *ovar = nullptr;
            const Token *tokNull = nullptr;
            if (Token::Match(tok, "0|NULL|nullptr ==|!=|>|>=|<|<= %var%")) {
                if (!Token::Match(tok->tokAt(3),".|[")) {
                    ovar = tok->tokAt(2)->variable();
                    tokNull = tok;
                }
            } else if (Token::Match(tok, "%var% ==|!=|>|>=|<|<= 0|NULL|nullptr") ||
                       Token::Match(tok, "%var% =|+ 0|NULL|nullptr )|]|,|;|+")) {
                ovar = tok->variable();
                tokNull = tok->tokAt(2);
            }
            if (ovar && !ovar->isPointer() && !ovar->isArray() && ovar->isStlStringType() && tokNull && tokNull->originalName() != "'\\0'")
                nullPointerError(tokNull);
        }
    }
}

void CheckNullPointer::nullPointerError(const Token *tok)
{
    ValueFlow::Value v(0);
    v.setKnown();
    nullPointerError(tok, "", &v, false);
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, const ValueFlow::Value *value, bool inconclusive)
{
    const std::string errmsgcond("$symbol:" + varname + '\n' + ValueFlow::eitherTheConditionIsRedundant(value ? value->condition : nullptr) + " or there is possible null pointer dereference: $symbol.");
    const std::string errmsgdefarg("$symbol:" + varname + "\nPossible null pointer dereference if the default parameter value is used: $symbol");

    if (!tok) {
        reportError(tok, Severity::error, "nullPointer", "Null pointer dereference", CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        reportError(tok, Severity::warning, "nullPointerDefaultArg", errmsgdefarg, CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        reportError(tok, Severity::warning, "nullPointerRedundantCheck", errmsgcond, CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        reportError(tok, Severity::warning, "nullPointerOutOfMemory", "Null pointer dereference", CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        reportError(tok, Severity::warning, "nullPointerOutOfResources", "Null pointer dereference", CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        return;
    }
    if (diag(tok))
        return;

    if (!value) {
        reportError(tok, Severity::error, "nullPointer", "Null pointer dereference", CWE_NULL_POINTER_DEREFERENCE, inconclusive ? Certainty::inconclusive : Certainty::normal);
        return;
    }

    if (!mSettings->isEnabled(value, inconclusive) && !mSettings->isPremiumEnabled("nullPointer"))
        return;

    ErrorPath errorPath = getErrorPath(tok, value, "Null pointer dereference");

    if (value->condition) {
        reportError(std::move(errorPath), Severity::warning, "nullPointerRedundantCheck", errmsgcond, CWE_NULL_POINTER_DEREFERENCE, inconclusive || value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    } else if (value->defaultArg) {
        reportError(std::move(errorPath), Severity::warning, "nullPointerDefaultArg", errmsgdefarg, CWE_NULL_POINTER_DEREFERENCE, inconclusive || value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    } else {
        std::string errmsg = std::string(value->isKnown() ? "Null" : "Possible null") + " pointer dereference";

        std::string id = "nullPointer";
        if (value->unknownFunctionReturn == ValueFlow::Value::UnknownFunctionReturn::outOfMemory) {
            errmsg = "If memory allocation fails, then there is a " + (static_cast<char>(std::tolower(errmsg[0])) + errmsg.substr(1));
            id += "OutOfMemory";
        }
        else if (value->unknownFunctionReturn == ValueFlow::Value::UnknownFunctionReturn::outOfResources) {
            errmsg = "If resource allocation fails, then there is a " + (static_cast<char>(std::tolower(errmsg[0])) + errmsg.substr(1));
            id += "OutOfResources";
        }

        if (!varname.empty())
            errmsg = "$symbol:" + varname + '\n' + errmsg + ": $symbol";

        reportError(std::move(errorPath),
                    value->isKnown() ? Severity::error : Severity::warning,
                    id.c_str(),
                    errmsg,
                    CWE_NULL_POINTER_DEREFERENCE, inconclusive || value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    }
}

void CheckNullPointer::arithmetic()
{
    logChecker("CheckNullPointer::arithmetic");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "-|+|+=|-=|++|--"))
                continue;
            const Token *pointerOperand;
            const Token *numericOperand;
            if (tok->astOperand1() && tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->pointer != 0) {
                pointerOperand = tok->astOperand1();
                numericOperand = tok->astOperand2();
            } else if (tok->astOperand2() && tok->astOperand2()->valueType() && tok->astOperand2()->valueType()->pointer != 0) {
                pointerOperand = tok->astOperand2();
                numericOperand = tok->astOperand1();
            } else
                continue;
            if (numericOperand && numericOperand->valueType() && !numericOperand->valueType()->isIntegral())
                continue;
            const ValueFlow::Value* numValue = numericOperand ? numericOperand->getValue(0) : nullptr;
            if (numValue && numValue->intvalue == 0) // don't warn for arithmetic with 0
                continue;
            const ValueFlow::Value* value = pointerOperand->getValue(0);
            if (!value)
                continue;
            if (!mSettings->certainty.isEnabled(Certainty::inconclusive) && value->isInconclusive())
                continue;
            if (value->condition && !mSettings->severity.isEnabled(Severity::warning))
                continue;
            if (value->condition)
                redundantConditionWarning(tok, value, value->condition, value->isInconclusive());
            else
                pointerArithmeticError(tok, value, value->isInconclusive());
        }
    }
}

static std::string arithmeticTypeString(const Token *tok)
{
    if (tok && tok->str()[0] == '-')
        return "subtraction";
    if (tok && tok->str()[0] == '+')
        return "addition";
    return "arithmetic";
}

void CheckNullPointer::pointerArithmeticError(const Token* tok, const ValueFlow::Value *value, bool inconclusive)
{
    // cppcheck-suppress shadowFunction - TODO: fix this
    std::string arithmetic = arithmeticTypeString(tok);
    std::string errmsg;
    if (tok && tok->str()[0] == '-') {
        errmsg = "Overflow in pointer arithmetic, NULL pointer is subtracted.";
    } else {
        errmsg = "Pointer " + arithmetic + " with NULL pointer.";
    }

    std::string id = "nullPointerArithmetic";
    if (value && value->unknownFunctionReturn == ValueFlow::Value::UnknownFunctionReturn::outOfMemory) {
        errmsg = "If memory allocation fails: " + (static_cast<char>(std::tolower(errmsg[0])) + errmsg.substr(1));
        id += "OutOfMemory";
    }
    else if (value && value->unknownFunctionReturn == ValueFlow::Value::UnknownFunctionReturn::outOfResources) {
        errmsg = "If resource allocation fails: " + (static_cast<char>(std::tolower(errmsg[0])) + errmsg.substr(1));
        id += "OutOfResources";
    }

    ErrorPath errorPath = getErrorPath(tok, value, "Null pointer " + arithmetic);
    reportError(std::move(errorPath),
                Severity::error,
                id.c_str(),
                errmsg,
                CWE_INCORRECT_CALCULATION,
                inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckNullPointer::redundantConditionWarning(const Token* tok, const ValueFlow::Value *value, const Token *condition, bool inconclusive)
{
    // cppcheck-suppress shadowFunction - TODO: fix this
    std::string arithmetic = arithmeticTypeString(tok);
    std::string errmsg;
    if (tok && tok->str()[0] == '-') {
        errmsg = ValueFlow::eitherTheConditionIsRedundant(condition) + " or there is overflow in pointer " + arithmetic + ".";
    } else {
        errmsg = ValueFlow::eitherTheConditionIsRedundant(condition) + " or there is pointer arithmetic with NULL pointer.";
    }
    ErrorPath errorPath = getErrorPath(tok, value, "Null pointer " + arithmetic);
    reportError(std::move(errorPath),
                Severity::warning,
                "nullPointerArithmeticRedundantCheck",
                errmsg,
                CWE_INCORRECT_CALCULATION,
                inconclusive ? Certainty::inconclusive : Certainty::normal);
}

// NOLINTNEXTLINE(readability-non-const-parameter) - used as callback so we need to preserve the signature
static bool isUnsafeUsage(const Settings &settings, const Token *vartok, CTU::FileInfo::Value *value)
{
    (void)value;
    bool unknown = false;
    return CheckNullPointer::isPointerDeRef(vartok, unknown, settings);
}

// a Clang-built executable will crash when using the anonymous MyFileInfo later on - so put it in a unique namespace for now
// see https://trac.cppcheck.net/ticket/12108 for more details
#ifdef __clang__
inline namespace CheckNullPointer_internal
#else
namespace
#endif
{
    /* data for multifile checking */
    class MyFileInfo : public Check::FileInfo {
    public:
        using Check::FileInfo::FileInfo;
        /** function arguments that are dereferenced without checking if they are null */
        std::list<CTU::FileInfo::UnsafeUsage> unsafeUsage;

        /** Convert data into xml string */
        std::string toString() const override
        {
            return CTU::toString(unsafeUsage);
        }
    };
}

Check::FileInfo *CheckNullPointer::getFileInfo(const Tokenizer &tokenizer, const Settings &settings, const std::string& /*currentConfig*/) const
{
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage = CTU::getUnsafeUsage(tokenizer, settings, isUnsafeUsage);
    if (unsafeUsage.empty())
        return nullptr;

    auto *fileInfo = new MyFileInfo(tokenizer.list.getFiles()[0]);
    fileInfo->unsafeUsage = unsafeUsage;
    return fileInfo;
}

Check::FileInfo * CheckNullPointer::loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const
{
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage = CTU::loadUnsafeUsageListFromXml(xmlElement);
    if (unsafeUsage.empty())
        return nullptr;

    auto *fileInfo = new MyFileInfo;
    fileInfo->unsafeUsage = unsafeUsage;
    return fileInfo;
}

bool CheckNullPointer::analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    (void)settings;

    CheckNullPointer dummy(nullptr, &settings, &errorLogger);
    dummy.
    logChecker("CheckNullPointer::analyseWholeProgram"); // unusedfunctions

    if (fileInfo.empty())
        return false;

    const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> callsMap = ctu.getCallsMap();

    bool foundErrors = false;

    for (const Check::FileInfo* fi1 : fileInfo) {
        const auto *fi = dynamic_cast<const MyFileInfo*>(fi1);
        if (!fi)
            continue;
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafeUsage) {
            for (int warning = 0; warning <= 1; warning++) {
                if (warning == 1 && !settings.severity.isEnabled(Severity::warning))
                    break;

                ValueFlow::Value::UnknownFunctionReturn unknownFunctionReturn = ValueFlow::Value::UnknownFunctionReturn::no;
                std::list<ErrorMessage::FileLocation> locationList =
                    CTU::FileInfo::getErrorPath(CTU::FileInfo::InvalidValueType::null,
                                                unsafeUsage,
                                                callsMap,
                                                "Dereferencing argument ARG that is null",
                                                nullptr,
                                                warning,
                                                settings.maxCtuDepth,
                                                &unknownFunctionReturn);
                if (locationList.empty())
                    continue;

                std::string id = "ctunullpointer";
                std::string message = "Null pointer dereference: " + unsafeUsage.myArgumentName;
                if (unknownFunctionReturn == ValueFlow::Value::UnknownFunctionReturn::outOfMemory) {
                    id += "OutOfMemory";
                    message = "If memory allocation fails, then there is a possible null pointer dereference: " + unsafeUsage.myArgumentName;
                } else if (unknownFunctionReturn == ValueFlow::Value::UnknownFunctionReturn::outOfResources) {
                    id += "OutOfResources";
                    message = "If resource allocation fails, then there is a possible null pointer dereference: " + unsafeUsage.myArgumentName;
                }

                ErrorMessage errmsg(std::move(locationList),
                                    fi->file0,
                                    warning ? Severity::warning : Severity::error,
                                    message,
                                    std::move(id),
                                    CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
                errorLogger.reportErr(errmsg);

                foundErrors = true;
                break;
            }
        }
    }

    return foundErrors;
}

void CheckNullPointer::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckNullPointer checkNullPointer(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkNullPointer.nullPointer();
    checkNullPointer.arithmetic();
    checkNullPointer.nullConstantDereference();
}

void CheckNullPointer::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckNullPointer c(nullptr, settings, errorLogger);
    c.nullPointerError(nullptr, "pointer", nullptr, false);
    c.pointerArithmeticError(nullptr, nullptr, false);
    // TODO: nullPointerArithmeticOutOfMemory
    c.redundantConditionWarning(nullptr, nullptr, nullptr, false);
    // TODO: ctunullpointer
    // TODO: ctunullpointerOutOfMemory
    // TODO: ctunullpointerOutOfResources
}
