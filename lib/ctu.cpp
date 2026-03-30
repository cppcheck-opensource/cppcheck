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
#include "ctu.h"

#include "astutils.h"
#include "errortypes.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>  // back_inserter
#include <sstream>
#include <utility>

#include "xml.h"
//---------------------------------------------------------------------------

static constexpr char ATTR_CALL_ID[] = "call-id";
static constexpr char ATTR_CALL_FUNCNAME[] = "call-funcname";
static constexpr char ATTR_CALL_ARGNR[] = "call-argnr";
static constexpr char ATTR_CALL_ARGEXPR[] = "call-argexpr";
static constexpr char ATTR_CALL_ARGVALUETYPE[] = "call-argvaluetype";
static constexpr char ATTR_CALL_ARGVALUE[] = "call-argvalue";
static constexpr char ATTR_CALL_UNKNOWN_FUNCTION_RETURN[] = "call-argvalue-ufr";
static constexpr char ATTR_WARNING[] = "warning";
static constexpr char ATTR_LOC_FILENAME[] = "file";
static constexpr char ATTR_LOC_LINENR[] = "line";
static constexpr char ATTR_LOC_COLUMN[] = "col";
static constexpr char ATTR_INFO[] = "info";
static constexpr char ATTR_MY_ID[] = "my-id";
static constexpr char ATTR_MY_ARGNR[] = "my-argnr";
static constexpr char ATTR_MY_ARGNAME[] = "my-argname";
static constexpr char ATTR_VALUE[] = "value";

std::string CTU::getFunctionId(const Tokenizer &tokenizer, const Function *function)
{
    return tokenizer.list.file(function->tokenDef) + ':' + std::to_string(function->tokenDef->linenr()) + ':' + std::to_string(function->tokenDef->column());
}

CTU::FileInfo::Location::Location(const Tokenizer &tokenizer, const Token *tok)
    : fileName(tokenizer.list.file(tok))
    , lineNumber(tok->linenr())
    , column(tok->column())
{}

std::string CTU::FileInfo::toString() const
{
    std::ostringstream out;

    // Function calls..
    for (const CTU::FileInfo::FunctionCall &functionCall : functionCalls) {
        out << functionCall.toXmlString();
    }

    // Nested calls..
    for (const CTU::FileInfo::NestedCall &nestedCall : nestedCalls) {
        out << nestedCall.toXmlString() << "\n";
    }

    return out.str();
}

std::string CTU::FileInfo::CallBase::toBaseXmlString() const
{
    std::ostringstream out;
    out << " " << ATTR_CALL_ID << "=\"" << callId << "\""
        << " " << ATTR_CALL_FUNCNAME << "=\"" << ErrorLogger::toxml(callFunctionName) << "\""
        << " " << ATTR_CALL_ARGNR << "=\"" << callArgNr << "\""
        << " " << ATTR_LOC_FILENAME << "=\"" << ErrorLogger::toxml(location.fileName) << "\""
        << " " << ATTR_LOC_LINENR << "=\"" << location.lineNumber << "\""
        << " " << ATTR_LOC_COLUMN << "=\"" << location.column << "\"";
    return out.str();
}

std::string CTU::FileInfo::FunctionCall::toXmlString() const
{
    std::ostringstream out;
    out << "<function-call"
        << toBaseXmlString()
        << " " << ATTR_CALL_ARGEXPR << "=\"" << ErrorLogger::toxml(callArgumentExpression) << "\""
        << " " << ATTR_CALL_ARGVALUETYPE << "=\"" << static_cast<int>(callValueType) << "\""
        << " " << ATTR_CALL_ARGVALUE << "=\"" << callArgValue.value << "\""
        << " " << ATTR_CALL_UNKNOWN_FUNCTION_RETURN << "=\"" << static_cast<int>(callArgValue.unknownFunctionReturn) << "\"";
    if (warning)
        out << " " << ATTR_WARNING << "=\"true\"";
    if (callValuePath.empty())
        out << "/>";
    else {
        out << ">\n";
        for (const ErrorMessage::FileLocation &loc : callValuePath)
            out << "  <path"
                << " " << ATTR_LOC_FILENAME << "=\"" << ErrorLogger::toxml(loc.getfile(false)) << "\""
                << " " << ATTR_LOC_LINENR << "=\"" << loc.line << "\""
                << " " << ATTR_LOC_COLUMN << "=\"" << loc.column << "\""
                << " " << ATTR_INFO << "=\"" << ErrorLogger::toxml(loc.getinfo()) << "\"/>\n";
        out << "</function-call>";
    }
    return out.str();
}

std::string CTU::FileInfo::NestedCall::toXmlString() const
{
    std::ostringstream out;
    out << "<function-call"
        << toBaseXmlString()
        << " " << ATTR_MY_ID << "=\"" << myId << "\""
        << " " << ATTR_MY_ARGNR << "=\"" << myArgNr << "\""
        << "/>";
    return out.str();
}

std::string CTU::FileInfo::UnsafeUsage::toString() const
{
    std::ostringstream out;
    out << "    <unsafe-usage"
        << " " << ATTR_MY_ID << "=\"" << myId << '\"'
        << " " << ATTR_MY_ARGNR << "=\"" << myArgNr << '\"'
        << " " << ATTR_MY_ARGNAME << "=\"" << myArgumentName << '\"'
        << " " << ATTR_LOC_FILENAME << "=\"" << ErrorLogger::toxml(location.fileName) << '\"'
        << " " << ATTR_LOC_LINENR << "=\"" << location.lineNumber << '\"'
        << " " << ATTR_LOC_COLUMN << "=\"" << location.column << '\"'
        << " " << ATTR_VALUE << "=\"" << value << "\""
        << "/>\n";
    return out.str();
}

std::string CTU::toString(const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage)
{
    std::ostringstream ret;
    for (const CTU::FileInfo::UnsafeUsage &u : unsafeUsage)
        ret << u.toString();
    return ret.str();
}

CTU::FileInfo::CallBase::CallBase(const Tokenizer &tokenizer, const Token *callToken)
    : callId(getFunctionId(tokenizer, callToken->function()))
    , callFunctionName(callToken->next()->astOperand1()->expressionString())
    , location(CTU::FileInfo::Location(tokenizer, callToken))
{}

CTU::FileInfo::NestedCall::NestedCall(const Tokenizer &tokenizer, const Function *myFunction, const Token *callToken)
    : CallBase(tokenizer, callToken)
    , myId(getFunctionId(tokenizer, myFunction))
{}

static std::string readAttrString(const tinyxml2::XMLElement *e, const char *attr, bool *error)
{
    const char *value = e->Attribute(attr);
    if (!value && error)
        *error = true;
    return empty_if_null(value);
}

static long long readAttrInt(const tinyxml2::XMLElement *e, const char *attr, bool *error)
{
    int64_t value = 0;
    const bool err = (e->QueryInt64Attribute(attr, &value) != tinyxml2::XML_SUCCESS);
    if (error)
        *error = err;
    return value;
}

bool CTU::FileInfo::CallBase::loadBaseFromXml(const tinyxml2::XMLElement *xmlElement)
{
    bool error = false;
    callId = readAttrString(xmlElement, ATTR_CALL_ID, &error);
    callFunctionName = readAttrString(xmlElement, ATTR_CALL_FUNCNAME, &error);
    callArgNr = readAttrInt(xmlElement, ATTR_CALL_ARGNR, &error);
    location.fileName = readAttrString(xmlElement, ATTR_LOC_FILENAME, &error);
    location.lineNumber = readAttrInt(xmlElement, ATTR_LOC_LINENR, &error);
    location.column = readAttrInt(xmlElement, ATTR_LOC_COLUMN, &error);
    return !error;
}

bool CTU::FileInfo::FunctionCall::loadFromXml(const tinyxml2::XMLElement *xmlElement)
{
    if (!loadBaseFromXml(xmlElement))
        return false;
    bool error=false;
    callArgumentExpression = readAttrString(xmlElement, ATTR_CALL_ARGEXPR, &error);
    callValueType = static_cast<ValueFlow::Value::ValueType>(readAttrInt(xmlElement, ATTR_CALL_ARGVALUETYPE, &error));
    callArgValue.value = readAttrInt(xmlElement, ATTR_CALL_ARGVALUE, &error);
    const auto ufr = readAttrInt(xmlElement, ATTR_CALL_UNKNOWN_FUNCTION_RETURN, &error);
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange) - TODO: fix this - #13726
    callArgValue.unknownFunctionReturn = static_cast<ValueFlow::Value::UnknownFunctionReturn>(ufr>=0 && ufr <=0xff ? ufr : 0xff);
    const char *w = xmlElement->Attribute(ATTR_WARNING);
    warning = w && std::strcmp(w, "true") == 0;
    for (const tinyxml2::XMLElement *e2 = xmlElement->FirstChildElement(); !error && e2; e2 = e2->NextSiblingElement()) {
        if (std::strcmp(e2->Name(), "path") != 0)
            continue;
        std::string file = readAttrString(e2, ATTR_LOC_FILENAME, &error);
        std::string info = readAttrString(e2, ATTR_INFO, &error);
        const int line = readAttrInt(e2, ATTR_LOC_LINENR, &error);
        const int column = readAttrInt(e2, ATTR_LOC_COLUMN, &error);
        ErrorMessage::FileLocation loc(file, std::move(info), line, column);
        callValuePath.emplace_back(std::move(loc));
    }
    return !error;
}

bool CTU::FileInfo::NestedCall::loadFromXml(const tinyxml2::XMLElement *xmlElement)
{
    if (!loadBaseFromXml(xmlElement))
        return false;
    bool error = false;
    myId = readAttrString(xmlElement, ATTR_MY_ID, &error);
    myArgNr = readAttrInt(xmlElement, ATTR_MY_ARGNR, &error);
    return !error;
}

// TODO: bail out on unexpected data
void CTU::FileInfo::loadFromXml(const tinyxml2::XMLElement *xmlElement)
{
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        const char* name = e->Name();
        if (std::strcmp(name, "function-call") == 0) {
            FunctionCall functionCall;
            if (functionCall.loadFromXml(e))
                functionCalls.push_back(std::move(functionCall));
        } else if (std::strcmp(name, "nested-call") == 0) {
            NestedCall nestedCall;
            if (nestedCall.loadFromXml(e))
                nestedCalls.push_back(std::move(nestedCall));
        }
    }
}

std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> CTU::FileInfo::getCallsMap() const
{
    std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> ret;
    for (const CTU::FileInfo::NestedCall &nc : nestedCalls)
        ret[nc.callId].push_back(&nc);
    for (const CTU::FileInfo::FunctionCall &fc : functionCalls)
        ret[fc.callId].push_back(&fc);
    return ret;
}

std::list<CTU::FileInfo::UnsafeUsage> CTU::loadUnsafeUsageListFromXml(const tinyxml2::XMLElement *xmlElement)
{
    std::list<CTU::FileInfo::UnsafeUsage> ret;
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "unsafe-usage") != 0)
            continue;
        bool error = false;
        FileInfo::UnsafeUsage unsafeUsage;
        unsafeUsage.myId = readAttrString(e, ATTR_MY_ID, &error);
        unsafeUsage.myArgNr = readAttrInt(e, ATTR_MY_ARGNR, &error);
        unsafeUsage.myArgumentName = readAttrString(e, ATTR_MY_ARGNAME, &error);
        unsafeUsage.location.fileName = readAttrString(e, ATTR_LOC_FILENAME, &error);
        unsafeUsage.location.lineNumber = readAttrInt(e, ATTR_LOC_LINENR, &error);
        unsafeUsage.location.column = readAttrInt(e, ATTR_LOC_COLUMN, &error);
        unsafeUsage.value = readAttrInt(e, ATTR_VALUE, &error);

        if (!error)
            ret.push_back(std::move(unsafeUsage));
    }
    return ret;
}

static int isCallFunction(const Scope *scope, int argnr, const Token *&tok)
{
    const Variable * const argvar = scope->function->getArgumentVar(argnr);
    if (!argvar->isPointer())
        return -1;
    for (const Token *tok2 = scope->bodyStart; tok2 != scope->bodyEnd; tok2 = tok2->next()) {
        if (tok2->variable() != argvar)
            continue;
        if (!Token::Match(tok2->previous(), "[(,] %var% [,)]"))
            break;
        int argnr2 = 1;
        const Token *prev = tok2;
        while (prev && prev->str() != "(") {
            if (Token::Match(prev,"]|)"))
                prev = prev->link();
            else if (prev->str() == ",")
                ++argnr2;
            prev = prev->previous();
        }
        if (!prev || !Token::Match(prev->previous(), "%name% ("))
            break;
        if (!prev->astOperand1() || !prev->astOperand1()->function())
            break;
        tok = prev->previous();
        return argnr2;
    }
    return -1;
}


CTU::FileInfo *CTU::getFileInfo(const Tokenizer &tokenizer)
{
    const SymbolDatabase * const symbolDatabase = tokenizer.getSymbolDatabase();

    auto *fileInfo = new FileInfo;

    // Parse all functions in TU
    for (const Scope &scope : symbolDatabase->scopeList) {
        if (!scope.isExecutable() || scope.type != ScopeType::eFunction || !scope.function)
            continue;
        const Function *const scopeFunction = scope.function;

        // source function calls
        for (const Token *tok = scope.bodyStart; tok != scope.bodyEnd; tok = tok->next()) {
            if (tok->str() != "(" || !tok->astOperand1() || !tok->astOperand2())
                continue;
            const Function* tokFunction = tok->astOperand1()->function();
            if (!tokFunction)
                continue;
            const std::vector<const Token *> args(getArguments(tok->previous()));
            for (int argnr = 0; argnr < args.size(); ++argnr) {
                const Token *argtok = args[argnr];
                if (!argtok)
                    continue;
                for (const ValueFlow::Value &value : argtok->values()) {
                    if ((!value.isIntValue() || value.intvalue != 0 || value.isInconclusive()) && !value.isBufferSizeValue())
                        continue;
                    // Skip impossible values since they cannot be represented
                    if (value.isImpossible())
                        continue;
                    FileInfo::FunctionCall functionCall;
                    functionCall.callValueType = value.valueType;
                    functionCall.callId = getFunctionId(tokenizer, tokFunction);
                    functionCall.callFunctionName = tok->astOperand1()->expressionString();
                    functionCall.location = FileInfo::Location(tokenizer,tok);
                    functionCall.callArgNr = argnr + 1;
                    functionCall.callArgumentExpression = argtok->expressionString();
                    functionCall.callArgValue = value;
                    functionCall.warning = !value.errorSeverity();
                    for (const ErrorPathItem &i : value.errorPath) {
                        const std::string& file = tokenizer.list.file(i.first);
                        const std::string& info = i.second;
                        const int line = i.first->linenr();
                        const int column = i.first->column();
                        ErrorMessage::FileLocation loc(file, info, line, column);
                        functionCall.callValuePath.push_back(std::move(loc));
                    }
                    fileInfo->functionCalls.push_back(std::move(functionCall));
                }
                // array
                if (argtok->variable() && argtok->variable()->isArray() && argtok->variable()->dimensions().size() == 1 && argtok->variable()->dimensionKnown(0)) {
                    FileInfo::FunctionCall functionCall;
                    functionCall.callValueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
                    functionCall.callId = getFunctionId(tokenizer, tokFunction);
                    functionCall.callFunctionName = tok->astOperand1()->expressionString();
                    functionCall.location = FileInfo::Location(tokenizer, tok);
                    functionCall.callArgNr = argnr + 1;
                    functionCall.callArgumentExpression = argtok->expressionString();
                    const auto typeSize = argtok->valueType()->getSizeOf(tokenizer.getSettings(), ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
                    functionCall.callArgValue.value = typeSize > 0 ? argtok->variable()->dimension(0) * typeSize : -1;
                    functionCall.warning = false;
                    fileInfo->functionCalls.push_back(std::move(functionCall));
                }
                // &var => buffer
                if (argtok->isUnaryOp("&") && argtok->astOperand1()->variable() && argtok->astOperand1()->valueType() && !argtok->astOperand1()->variable()->isArray()) {
                    FileInfo::FunctionCall functionCall;
                    functionCall.callValueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
                    functionCall.callId = getFunctionId(tokenizer, tokFunction);
                    functionCall.callFunctionName = tok->astOperand1()->expressionString();
                    functionCall.location = FileInfo::Location(tokenizer, tok);
                    functionCall.callArgNr = argnr + 1;
                    functionCall.callArgumentExpression = argtok->expressionString();
                    functionCall.callArgValue.value = argtok->astOperand1()->valueType()->getSizeOf(tokenizer.getSettings(), ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointee);
                    functionCall.warning = false;
                    fileInfo->functionCalls.push_back(std::move(functionCall));
                }
                // pointer/reference to uninitialized data
                auto isAddressOfArg = [](const Token* argtok) -> const Token* {
                    if (!argtok->isUnaryOp("&"))
                        return nullptr;
                    argtok = argtok->astOperand1();
                    if (!argtok || !argtok->valueType() || argtok->valueType()->pointer != 0)
                        return nullptr;
                    return argtok;
                };
                auto isReferenceArg = [&](const Token* argtok) -> const Token* {
                    const Variable* argvar = tokFunction->getArgumentVar(argnr);
                    if (!argvar || !argvar->valueType() || argvar->valueType()->reference == Reference::None)
                        return nullptr;
                    return argtok;
                };
                const Token* addr = isAddressOfArg(argtok);
                argtok = addr ? addr : isReferenceArg(argtok);
                if (!argtok || argtok->values().size() != 1U)
                    continue;
                if (argtok->variable() && argtok->variable()->isClass())
                    continue;

                const ValueFlow::Value &v = argtok->values().front();
                if (v.valueType == ValueFlow::Value::ValueType::UNINIT && !v.isInconclusive()) {
                    FileInfo::FunctionCall functionCall;
                    functionCall.callValueType = ValueFlow::Value::ValueType::UNINIT;
                    functionCall.callId = getFunctionId(tokenizer, tokFunction);
                    functionCall.callFunctionName = tok->astOperand1()->expressionString();
                    functionCall.location = FileInfo::Location(tokenizer, tok);
                    functionCall.callArgNr = argnr + 1;
                    functionCall.callArgValue = v;
                    functionCall.callArgValue.value = 0;
                    functionCall.callArgumentExpression = argtok->expressionString();
                    functionCall.warning = false;
                    fileInfo->functionCalls.push_back(std::move(functionCall));
                    continue;
                }
            }
        }

        // Nested function calls
        for (int argnr = 0; argnr < scopeFunction->argCount(); ++argnr) {
            const Token *tok;
            const int argnr2 = isCallFunction(&scope, argnr, tok);
            if (argnr2 > 0) {
                FileInfo::NestedCall nestedCall(tokenizer, scopeFunction, tok);
                nestedCall.myArgNr = argnr + 1;
                nestedCall.callArgNr = argnr2;
                fileInfo->nestedCalls.push_back(std::move(nestedCall));
            }
        }
    }

    return fileInfo;
}

static std::vector<std::pair<const Token *, CTU::FileInfo::Value>> getUnsafeFunction(const Settings &settings, const Scope *scope, int argnr, bool (*isUnsafeUsage)(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *value))
{
    std::vector<std::pair<const Token *, CTU::FileInfo::Value>> ret;
    const Variable * const argvar = scope->function->getArgumentVar(argnr);
    if (!argvar->isArrayOrPointer() && !argvar->isReference())
        return ret;
    for (const Token *tok2 = scope->bodyStart; tok2 != scope->bodyEnd; tok2 = tok2->next()) {
        if (Token::Match(tok2, ")|else {")) {
            tok2 = tok2->linkAt(1);
            if (Token::findmatch(tok2->link(), "return|throw", tok2))
                return ret;
            int indirect = 0;
            if (argvar->valueType())
                indirect = argvar->valueType()->pointer;
            if (isVariableChanged(tok2->link(), tok2, indirect, argvar->declarationId(), false, settings))
                return ret;
        }
        if (Token::Match(tok2, "%oror%|&&|?")) {
            tok2 = tok2->findExpressionStartEndTokens().second;
            continue;
        }
        if (tok2->variable() != argvar)
            continue;
        CTU::FileInfo::Value value;
        if (!isUnsafeUsage(settings, tok2, &value))
            return ret; // TODO: Is this a read? then continue..
        ret.emplace_back(tok2, value);
        return ret;
    }
    return ret;
}

std::list<CTU::FileInfo::UnsafeUsage> CTU::getUnsafeUsage(const Tokenizer &tokenizer, const Settings &settings, bool (*isUnsafeUsage)(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *value))
{
    std::list<CTU::FileInfo::UnsafeUsage> unsafeUsage;

    // Parse all functions in TU
    const SymbolDatabase * const symbolDatabase = tokenizer.getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (!scope.isExecutable() || scope.type != ScopeType::eFunction || !scope.function)
            continue;
        const Function *const function = scope.function;

        // "Unsafe" functions unconditionally reads data before it is written..
        for (int argnr = 0; argnr < function->argCount(); ++argnr) {
            for (const std::pair<const Token *, CTU::FileInfo::Value> &v : getUnsafeFunction(settings, &scope, argnr, isUnsafeUsage)) {
                const Token *tok = v.first;
                const MathLib::bigint val = v.second.value;
                unsafeUsage.emplace_back(CTU::getFunctionId(tokenizer, function), argnr+1, tok->str(), CTU::FileInfo::Location(tokenizer,tok), val);
            }
        }
    }

    return unsafeUsage;
}

static bool findPath(const std::string &callId,
                     nonneg int callArgNr,
                     MathLib::bigint unsafeValue,
                     CTU::FileInfo::InvalidValueType invalidValue,
                     const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap,
                     const CTU::FileInfo::CallBase *path[10],
                     int index,
                     bool warning,
                     int maxCtuDepth)
{
    if (index >= maxCtuDepth)
        return false; // TODO: add bailout message?

    const auto it = callsMap.find(callId);
    if (it == callsMap.end())
        return false;

    for (const CTU::FileInfo::CallBase *c : it->second) {
        if (c->callArgNr != callArgNr)
            continue;

        const auto *functionCall = dynamic_cast<const CTU::FileInfo::FunctionCall *>(c);
        if (functionCall) {
            if (!warning && functionCall->warning)
                continue;
            if (!warning && functionCall->callArgValue.unknownFunctionReturn != ValueFlow::Value::UnknownFunctionReturn::no)
                continue;
            switch (invalidValue) {
            case CTU::FileInfo::InvalidValueType::null:
                if (functionCall->callValueType != ValueFlow::Value::ValueType::INT || functionCall->callArgValue.value != 0)
                    continue;
                break;
            case CTU::FileInfo::InvalidValueType::uninit:
                if (functionCall->callValueType != ValueFlow::Value::ValueType::UNINIT)
                    continue;
                break;
            case CTU::FileInfo::InvalidValueType::bufferOverflow:
                if (functionCall->callValueType != ValueFlow::Value::ValueType::BUFFER_SIZE)
                    continue;
                if (unsafeValue < 0 || (unsafeValue >= functionCall->callArgValue.value && functionCall->callArgValue.value >= 0))
                    break;
                continue;
            }
            path[index] = functionCall;
            return true;
        }

        const auto *nestedCall = dynamic_cast<const CTU::FileInfo::NestedCall *>(c);
        if (!nestedCall)
            continue;

        if (findPath(nestedCall->myId, nestedCall->myArgNr, unsafeValue, invalidValue, callsMap, path, index + 1, warning, maxCtuDepth)) {
            path[index] = nestedCall;
            return true;
        }
    }

    return false;
}

static std::string getInvalidValueString(CTU::FileInfo::InvalidValueType invalidValue)
{
    using InvalidValueType = CTU::FileInfo::InvalidValueType;
    switch (invalidValue) {
    case InvalidValueType::null:
        return "null";
    case InvalidValueType::uninit:
        return "uninitialized";
    case InvalidValueType::bufferOverflow:
        return "accessed out of bounds";
    }
    cppcheck::unreachable();
}

std::list<ErrorMessage::FileLocation> CTU::FileInfo::getErrorPath(InvalidValueType invalidValue,
                                                                  const CTU::FileInfo::UnsafeUsage &unsafeUsage,
                                                                  const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap,
                                                                  const char info[],
                                                                  const FunctionCall ** const functionCallPtr,
                                                                  bool warning,
                                                                  int maxCtuDepth,
                                                                  ValueFlow::Value::UnknownFunctionReturn *unknownFunctionReturn)
{
    const CTU::FileInfo::CallBase *path[10] = {nullptr};

    if (!findPath(unsafeUsage.myId, unsafeUsage.myArgNr, unsafeUsage.value, invalidValue, callsMap, path, 0, warning, maxCtuDepth))
        return {};

    if (unknownFunctionReturn && path[0]) {
        const auto* functionCall = dynamic_cast<const CTU::FileInfo::FunctionCall *>(path[0]);
        if (functionCall)
            *unknownFunctionReturn = functionCall->callArgValue.unknownFunctionReturn;
    }

    std::list<ErrorMessage::FileLocation> locationList;

    const std::string value1 = getInvalidValueString(invalidValue);

    for (int index = 9; index >= 0; index--) {
        if (!path[index])
            continue;

        const auto *functionCall = dynamic_cast<const CTU::FileInfo::FunctionCall *>(path[index]);

        if (functionCall) {
            if (functionCallPtr)
                *functionCallPtr = functionCall;
            std::copy(functionCall->callValuePath.cbegin(), functionCall->callValuePath.cend(), std::back_inserter(locationList));
        }

        std::string info_s = "Calling function " + path[index]->callFunctionName + ", " + std::to_string(path[index]->callArgNr) + getOrdinalText(path[index]->callArgNr) + " argument is " + value1;
        ErrorMessage::FileLocation fileLoc(path[index]->location.fileName, std::move(info_s), path[index]->location.lineNumber, path[index]->location.column);
        locationList.push_back(std::move(fileLoc));
    }

    std::string info_s = replaceStr(info, "ARG", unsafeUsage.myArgumentName);
    ErrorMessage::FileLocation fileLoc2(unsafeUsage.location.fileName, std::move(info_s), unsafeUsage.location.lineNumber, unsafeUsage.location.column);
    locationList.push_back(std::move(fileLoc2));

    return locationList;
}
