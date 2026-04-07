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
#include "checkunusedfunctions.h"

#include "analyzerinfo.h"
#include "astutils.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <map>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>

#include "xml.h"

//---------------------------------------------------------------------------

static const CWE CWE561(561U);   // Dead Code

static std::string stripTemplateParameters(const std::string& funcName) {
    std::string name = funcName;
    const auto pos = name.find('<');
    if (pos > 0 && pos != std::string::npos)
        name.erase(pos - 1);
    return name;
}

//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

static bool isRecursiveCall(const Token* ftok)
{
    return ftok->function() && ftok->function() == Scope::nestedInFunction(ftok->scope());
}

void CheckUnusedFunctions::parseTokens(const Tokenizer &tokenizer, const Settings &settings)
{
    const char * const FileName = tokenizer.list.getFiles().front().c_str();

    const bool doMarkup = settings.library.markupFile(FileName);

    // Function declarations..
    if (!doMarkup) {
        const SymbolDatabase* symbolDatabase = tokenizer.getSymbolDatabase();
        for (const Scope* scope : symbolDatabase->functionScopes) {
            const Function* func = scope->function;
            if (!func || !func->token)
                continue;

            // Don't warn about functions that are marked by __attribute__((constructor)) or __attribute__((destructor))
            if (func->isAttributeConstructor() || func->isAttributeDestructor() || func->type != FunctionType::eFunction || func->isOperator())
                continue;

            if (func->isAttributeUnused() || func->isAttributeMaybeUnused())
                continue;

            if (func->isExtern())
                continue;

            bool foundAllBaseClasses{};
            if (const Function* ofunc = func->getOverriddenFunction(&foundAllBaseClasses)) {
                if (!foundAllBaseClasses || ofunc->isPure())
                    continue;
            }
            else if (func->isImplicitlyVirtual()) {
                continue;
            }

            mFunctionDecl.emplace_back(func);

            FunctionUsage &usage = mFunctions[stripTemplateParameters(func->name())];

            if (func->retDef && (func->retDef->isAttributeUnused() || func->retDef->isAttributeMaybeUnused())) {
                usage.usedOtherFile = true;
            }

            if (!usage.lineNumber) {
                usage.lineNumber = func->token->linenr();
                usage.column = func->token->column();
            }
            usage.isC = func->token->isC();
            usage.isStatic = func->isStatic();

            // TODO: why always overwrite this but not the filename and line?
            usage.fileIndex = func->token->fileIndex();
            const std::string& fileName = tokenizer.list.file(func->token);

            // No filename set yet..
            if (usage.filename.empty()) {
                usage.filename = fileName;
            }
            // Multiple files => filename = "+"
            else if (usage.filename != fileName) {
                //func.filename = "+";
                usage.usedOtherFile |= usage.usedSameFile;
            }
        }
    }

    // Function usage..
    const Token *lambdaEndToken = nullptr;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {

        if (tok == lambdaEndToken)
            lambdaEndToken = nullptr;
        else if (!lambdaEndToken && tok->str() == "[")
            lambdaEndToken = findLambdaEndToken(tok);

        // parsing of library code to find called functions
        if (settings.library.isexecutableblock(FileName, tok->str())) {
            const Token * markupVarToken = tok->tokAt(settings.library.blockstartoffset(FileName));
            // not found
            if (!markupVarToken)
                continue;
            int scope = 0;
            bool start = true;
            // find all function calls in library code (starts with '(', not if or while etc)
            while ((scope || start) && markupVarToken) {
                if (markupVarToken->str() == settings.library.blockstart(FileName)) {
                    scope++;
                    start = false;
                } else if (markupVarToken->str() == settings.library.blockend(FileName))
                    scope--;
                else if (!settings.library.iskeyword(FileName, markupVarToken->str())) {
                    mFunctionCalls.insert(markupVarToken->str());
                    if (mFunctions.find(markupVarToken->str()) != mFunctions.end())
                        mFunctions[markupVarToken->str()].usedOtherFile = true;
                    else if (markupVarToken->strAt(1) == "(") {
                        FunctionUsage &func = mFunctions[markupVarToken->str()];
                        func.filename = tokenizer.list.getFiles()[markupVarToken->fileIndex()];
                        if (func.filename.empty() || func.filename == "+")
                            func.usedOtherFile = true;
                        else
                            func.usedSameFile = true;
                    }
                }
                markupVarToken = markupVarToken->next();
            }
        }

        if (!doMarkup // only check source files
            && settings.library.isexporter(tok->str()) && tok->next() != nullptr) {
            const Token * propToken = tok->next();
            while (propToken && propToken->str() != ")") {
                if (settings.library.isexportedprefix(tok->str(), propToken->str())) {
                    const Token* nextPropToken = propToken->next();
                    const std::string& value = nextPropToken->str();
                    if (mFunctions.find(value) != mFunctions.end()) {
                        mFunctions[value].usedOtherFile = true;
                    }
                    mFunctionCalls.insert(value);
                }
                if (settings.library.isexportedsuffix(tok->str(), propToken->str())) {
                    const Token* prevPropToken = propToken->previous();
                    const std::string& value = prevPropToken->str();
                    if (value != ")" && mFunctions.find(value) != mFunctions.end()) {
                        mFunctions[value].usedOtherFile = true;
                    }
                    mFunctionCalls.insert(value);
                }
                propToken = propToken->next();
            }
        }

        if (doMarkup && settings.library.isimporter(FileName, tok->str()) && tok->next()) {
            const Token * propToken = tok->next();
            if (propToken->next()) {
                propToken = propToken->next();
                while (propToken && propToken->str() != ")") {
                    const std::string& value = propToken->str();
                    if (!value.empty()) {
                        mFunctions[value].usedOtherFile = true;
                        mFunctionCalls.insert(value);
                        break;
                    }
                    propToken = propToken->next();
                }
            }
        }

        if (settings.library.isreflection(tok->str())) {
            const int argIndex = settings.library.reflectionArgument(tok->str());
            if (argIndex >= 0) {
                const Token * funcToken = tok->next();
                int index = 0;
                std::string value;
                while (funcToken) {
                    if (funcToken->str()==",") {
                        if (++index == argIndex)
                            break;
                        value.clear();
                    } else
                        value += funcToken->str();
                    funcToken = funcToken->next();
                }
                if (index == argIndex) {
                    value = value.substr(1, value.length() - 2);
                    mFunctions[value].usedOtherFile = true;
                    mFunctionCalls.insert(std::move(value));
                }
            }
        }

        if (tok->hasAttributeCleanup()) {
            const std::string& funcname = tok->getAttributeCleanup();
            mFunctions[funcname].usedOtherFile = true;
            mFunctionCalls.insert(funcname);
            continue;
        }

        const Token *funcname = nullptr;

        if (doMarkup)
            funcname = Token::Match(tok, "%name% (") ? tok : nullptr;
        else if ((lambdaEndToken || tok->scope()->isExecutable()) && Token::Match(tok, "%name% (")) {
            funcname = tok;
        } else if ((lambdaEndToken || tok->scope()->isExecutable()) && Token::Match(tok, "%name% <") && Token::simpleMatch(tok->linkAt(1), "> (")) {
            funcname = tok;
        } else if (Token::Match(tok, "< %name%") && tok->link()) {
            funcname = tok->next();
            while (Token::Match(funcname, "%name% :: %name%"))
                funcname = funcname->tokAt(2);
        } else if (tok->scope()->type != ScopeType::eEnum && (Token::Match(tok, "[;{}.,()[=+-/|!?:]") || Token::Match(tok, "return|throw"))) {
            funcname = tok->next();
            if (funcname && funcname->str() == "&")
                funcname = funcname->next();
            if (funcname && funcname->str() == "::")
                funcname = funcname->next();
            while (Token::Match(funcname, "%name% :: %name%"))
                funcname = funcname->tokAt(2);

            if (!Token::Match(funcname, "%name% [(),;]:}<>]"))
                continue;
        }

        if (!funcname || funcname->isKeyword() || funcname->isStandardType() || funcname->varId() || funcname->enumerator() || funcname->type() || funcname->isLiteral())
            continue;

        // funcname ( => Assert that the end parentheses isn't followed by {
        if (Token::Match(funcname, "%name% (|<") && funcname->linkAt(1)) {
            const Token *ftok = funcname->next();
            if (ftok->str() == "<")
                ftok = ftok->link();
            if (Token::Match(ftok->linkAt(1), ") const|throw|{"))
                funcname = nullptr;
        }

        if (funcname) {
            if (isRecursiveCall(funcname))
                continue;
            auto baseName = stripTemplateParameters(funcname->str());
            FunctionUsage &func = mFunctions[baseName];
            const std::string& called_from_file = tokenizer.list.getFiles()[funcname->fileIndex()];

            if (func.filename.empty() || func.filename == "+" || func.filename != called_from_file)
                func.usedOtherFile = true;
            else
                func.usedSameFile = true;

            mFunctionCalls.insert(std::move(baseName));
        }
    }
}


static bool isOperatorFunction(const std::string & funcName)
{
    /* Operator functions are invalid function names for C, so no need to check
     * this in here. As result the returned error function might be incorrect.
     *
     * List of valid operators can be found at:
     * http://en.cppreference.com/w/cpp/language/operators
     *
     * Conversion functions must be a member function (at least for gcc), so no
     * need to cover them for unused functions.
     *
     * To speed up the comparison, not the whole list of operators is used.
     * Instead only the character after the operator prefix is checked to be a
     * none alpa numeric value, but the '_', to cover function names like
     * "operator_unused". In addition the following valid operators are checked:
     * - new
     * - new[]
     * - delete
     * - delete[]
     */
    const std::string operatorPrefix = "operator";
    if (funcName.compare(0, operatorPrefix.length(), operatorPrefix) != 0) {
        return false;
    }

    // Taking care of funcName == "operator", which is no valid operator
    if (funcName.length() == operatorPrefix.length()) {
        return false;
    }

    const char firstOperatorChar = funcName[operatorPrefix.length()];
    if (firstOperatorChar == '_') {
        return false;
    }

    if (!std::isalnum(firstOperatorChar)) {
        return true;
    }

    const std::vector<std::string> additionalOperators = {
        "new", "new[]", "delete", "delete[]"
    };


    return std::find(additionalOperators.cbegin(), additionalOperators.cend(), funcName.substr(operatorPrefix.length())) != additionalOperators.cend();
}

void CheckUnusedFunctions::staticFunctionError(ErrorLogger& errorLogger,
                                               const std::string &filename, nonneg int fileIndex, nonneg int lineNumber, nonneg int column,
                                               const std::string &funcname)
{
    std::list<ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        locationList.emplace_back(filename, lineNumber, column);
        locationList.back().fileIndex = fileIndex;
    }

    const ErrorMessage errmsg(std::move(locationList), "", Severity::style, "$symbol:" + funcname + "\nThe function '$symbol' should have static linkage since it is not used outside of its translation unit.", "staticFunction", Certainty::normal);
    errorLogger.reportErr(errmsg);
}


#define logChecker(id) \
    do { \
        const ErrorMessage errmsg({}, nullptr, Severity::internal, "logChecker", (id), CWE(0U), Certainty::normal); \
        errorLogger.reportErr(errmsg); \
    } while (false)

bool CheckUnusedFunctions::check(const Settings& settings, ErrorLogger& errorLogger) const
{
    logChecker("CheckUnusedFunctions::check"); // unusedFunction

    // filename, fileindex, line, column
    using ErrorParams = std::tuple<std::string, nonneg int, nonneg int, nonneg int, std::string>;
    std::vector<ErrorParams> errors; // ensure well-defined order
    std::vector<ErrorParams> staticFunctionErrors;

    for (auto it = mFunctions.cbegin(); it != mFunctions.cend(); ++it) {
        const FunctionUsage &func = it->second;
        if (func.usedOtherFile || func.filename.empty())
            continue;
        if (settings.library.isentrypoint(it->first))
            continue;
        if (!func.usedSameFile) {
            if (isOperatorFunction(it->first))
                continue;
            std::string filename;
            if (func.filename != "+")
                filename = func.filename;
            errors.emplace_back(filename, func.fileIndex, func.lineNumber, func.column, it->first);
        } else if (func.isC && !func.isStatic) {
            std::string filename;
            if (func.filename != "+")
                filename = func.filename;
            staticFunctionErrors.emplace_back(filename, func.fileIndex, func.lineNumber, func.column, it->first);
        }
    }

    std::sort(errors.begin(), errors.end());
    for (const auto& e : errors)
        unusedFunctionError(errorLogger, std::get<0>(e), std::get<1>(e), std::get<2>(e), std::get<3>(e), std::get<4>(e));

    std::sort(staticFunctionErrors.begin(), staticFunctionErrors.end());
    for (const auto& e : staticFunctionErrors)
        staticFunctionError(errorLogger, std::get<0>(e), std::get<1>(e), std::get<2>(e), std::get<3>(e), std::get<4>(e));

    return !errors.empty();
}

void CheckUnusedFunctions::unusedFunctionError(ErrorLogger& errorLogger,
                                               const std::string &filename, nonneg int fileIndex, nonneg int lineNumber, nonneg int column,
                                               const std::string &funcname)
{
    std::list<ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        locationList.emplace_back(filename, lineNumber, column);
        locationList.back().fileIndex = fileIndex;
    }

    const ErrorMessage errmsg(std::move(locationList), "", Severity::style, "$symbol:" + funcname + "\nThe function '$symbol' is never used.", "unusedFunction", CWE561, Certainty::normal);
    errorLogger.reportErr(errmsg);
}

CheckUnusedFunctions::FunctionDecl::FunctionDecl(const Function *f)
    : functionName(f->name())
    , fileIndex(f->token->fileIndex())
    , lineNumber(f->token->linenr())
    , column(f->token->column())
{}

std::string CheckUnusedFunctions::analyzerInfo(const Tokenizer &tokenizer) const
{
    std::ostringstream ret;
    for (const FunctionDecl &functionDecl : mFunctionDecl) {
        ret << "    <functiondecl"
            << " file=\"" << ErrorLogger::toxml(tokenizer.list.getFiles()[functionDecl.fileIndex]) << '\"'
            << " functionName=\"" << ErrorLogger::toxml(functionDecl.functionName) << '\"'
            << " lineNumber=\"" << functionDecl.lineNumber << '\"'
            << " column=\"" << functionDecl.column << '\"'
            << "/>\n";
    }
    for (const std::string &fc : mFunctionCalls) {
        ret << "    <functioncall functionName=\"" << ErrorLogger::toxml(fc) << "\"/>\n";
    }
    return ret.str();
}

namespace {
    struct Location {
        Location() : lineNumber(0), column(0) {}
        Location(std::string f, nonneg int l, nonneg int c) : fileName(std::move(f)), lineNumber(l), column(c) {}
        std::string fileName;
        nonneg int lineNumber;
        nonneg int column;
    };
}

// TODO: bail out on unexpected data
void CheckUnusedFunctions::analyseWholeProgram(const Settings &settings, ErrorLogger &errorLogger, const std::string &buildDir)
{
    std::map<std::string, Location> decls;
    std::set<std::string> calls;

    const auto handler = [&decls, &calls](const char* checkattr, const tinyxml2::XMLElement* e, const AnalyzerInformation::Info& filesTxtInfo) {
        if (std::strcmp(checkattr,"CheckUnusedFunctions") != 0)
            return;
        for (const tinyxml2::XMLElement *e2 = e->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
            const char* functionName = e2->Attribute("functionName");
            if (functionName == nullptr)
                continue;
            const char* name = e2->Name();
            if (std::strcmp(name,"functioncall") == 0) {
                calls.insert(functionName);
                continue;
            }
            if (std::strcmp(name,"functiondecl") == 0) {
                const char* lineNumber = e2->Attribute("lineNumber");
                if (lineNumber) {
                    const char* file = e2->Attribute("file");
                    const char* column = default_if_null(e2->Attribute("column"), "0");
                    // cppcheck-suppress templateInstantiation - TODO: fix this - see #11631
                    decls[functionName] = Location(file ? file : filesTxtInfo.sourceFile, strToInt<int>(lineNumber), strToInt<int>(column));
                }
            }
        }
    };

    const std::string err = AnalyzerInformation::processFilesTxt(buildDir, handler, settings.debugainfo);
    if (!err.empty()) {
        const ErrorMessage errmsg({}, "", Severity::error, err, "internalError", Certainty::normal);
        errorLogger.reportErr(errmsg);
        return;
    }

    for (auto decl = decls.cbegin(); decl != decls.cend(); ++decl) {
        const std::string &functionName = stripTemplateParameters(decl->first);

        if (settings.library.isentrypoint(functionName))
            continue;

        if (calls.find(functionName) == calls.end() && !isOperatorFunction(functionName)) {
            const Location &loc = decl->second;
            unusedFunctionError(errorLogger, loc.fileName, /*fileIndex*/ 0, loc.lineNumber, loc.column, functionName);
        }
    }
}

void CheckUnusedFunctions::updateFunctionData(const CheckUnusedFunctions& check)
{
    for (const auto& entry : check.mFunctions)
    {
        FunctionUsage &usage = mFunctions[entry.first];
        if (!usage.lineNumber) {
            usage.lineNumber = entry.second.lineNumber;
            usage.column = entry.second.column;
        }
        // TODO: why always overwrite this but not the filename and line?
        usage.fileIndex = entry.second.fileIndex;
        if (usage.filename.empty())
            usage.filename = entry.second.filename;
        usage.usedOtherFile |= entry.second.usedOtherFile;
        usage.usedSameFile |= entry.second.usedSameFile;
    }
    mFunctionDecl.insert(mFunctionDecl.cend(), check.mFunctionDecl.cbegin(), check.mFunctionDecl.cend());
    mFunctionCalls.insert(check.mFunctionCalls.cbegin(), check.mFunctionCalls.cend());
}
