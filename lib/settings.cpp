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
#include "settings.h"
#include "path.h"
#include "summaries.h"
#include "suppressions.h"
#include "vfvalue.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>

#include "json.h"

#ifndef _WIN32
#include <unistd.h> // for getpid()
#else
#include <process.h> // for getpid()
#endif


std::atomic<bool> Settings::mTerminated;

const int Settings::maxConfigsNotAssigned = 0;
const int Settings::maxConfigsDefault = 12;

const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";

static int getPid()
{
#ifndef _WIN32
    return getpid();
#else
    return _getpid();
#endif
}

Settings::Settings()
{
    severity.setEnabled(Severity::error, true);
    certainty.setEnabled(Certainty::normal, true);
    setCheckLevel(Settings::CheckLevel::exhaustive);
    executor = defaultExecutor();
    pid = getPid();
}

std::string Settings::loadCppcheckCfg(Settings& settings, Suppressions& suppressions, bool debug)
{
    static const std::string cfgFilename = "cppcheck.cfg";
    std::string fileName;
#ifdef FILESDIR
    {
        const std::string filesdirCfg = Path::join(FILESDIR, cfgFilename);
        if (debug)
            std::cout << "looking for '" << filesdirCfg << "'" << std::endl;
        if (Path::isFile(filesdirCfg))
            fileName = filesdirCfg;
    }
#endif
    // cppcheck-suppress knownConditionTrueFalse
    if (fileName.empty()) {
        // TODO: make sure that exename is set
        fileName = Path::join(Path::getPathFromFilename(settings.exename), cfgFilename);
        if (debug)
            std::cout << "looking for '" << fileName << "'" << std::endl;
        if (!Path::isFile(fileName))
        {
            if (debug)
                std::cout << "no configuration found" << std::endl;
            return "";
        }
    }

    std::ifstream fin(fileName);
    if (!fin.is_open())
        return "could not open file";
    picojson::value json;
    fin >> json;
    {
        const std::string& lastErr = picojson::get_last_error();
        if (!lastErr.empty())
            return "not a valid JSON - " + lastErr;
    }
    const picojson::object& obj = json.get<picojson::object>();
    {
        const auto it = obj.find("productName");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<std::string>())
                return "'productName' is not a string";
            settings.cppcheckCfgProductName = v.get<std::string>();
        }
    }
    {
        const auto it = obj.find("manualUrl");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<std::string>())
                return "'manualUrl' is not a string";
            settings.manualUrl = v.get<std::string>();
        }
    }
    {
        const auto it = obj.find("about");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<std::string>())
                return "'about' is not a string";
            settings.cppcheckCfgAbout = v.get<std::string>();
        }
    }
    {
        const auto it = obj.find("addons");
        if (it != obj.cend()) {
            const auto& entry = it->second;
            if (!entry.is<picojson::array>())
                return "'addons' is not an array";
            for (const picojson::value &v : entry.get<picojson::array>())
            {
                if (!v.is<std::string>())
                    return "'addons' array entry is not a string";
                const std::string &s = v.get<std::string>();
                if (!Path::isAbsolute(s))
                    settings.addons.emplace(Path::join(Path::getPathFromFilename(fileName), s));
                else
                    settings.addons.emplace(s);
            }
        }
    }
    {
        const auto it = obj.find("suppressions");
        if (it != obj.cend()) {
            const auto& entry = it->second;
            if (!entry.is<picojson::array>())
                return "'suppressions' is not an array";
            for (const picojson::value &v : entry.get<picojson::array>())
            {
                if (!v.is<std::string>())
                    return "'suppressions' array entry is not a string";
                const std::string &s = v.get<std::string>();
                const std::string err = suppressions.nomsg.addSuppressionLine(s);
                if (!err.empty())
                    return "could not parse suppression '" + s + "' - " + err;
            }
        }
    }
    {
        const auto it = obj.find("safety");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<bool>())
                return "'safety' is not a bool";
            settings.safety = v.get<bool>();
        }
    }

    settings.settingsFiles.emplace_back(std::move(fileName));

    return "";
}

std::pair<std::string, std::string> Settings::getNameAndVersion(const std::string& productName) {
    if (productName.empty())
        return {};
    const std::string::size_type pos1 = productName.rfind(' ');
    if (pos1 == std::string::npos)
        return {};
    if (pos1 + 2 >= productName.length())
        return {};
    for (auto pos2 = pos1 + 1; pos2 < productName.length(); ++pos2) {
        const char c = productName[pos2];
        const char prev = productName[pos2-1];
        if (std::isdigit(c))
            continue;
        if (c == '.' && std::isdigit(prev))
            continue;
        if (c == 's' && pos2 + 1 == productName.length() && std::isdigit(prev))
            continue;
        return {};
    }
    return {productName.substr(0, pos1), productName.substr(pos1+1)};
}

std::string Settings::parseEnabled(const std::string &str, std::tuple<SimpleEnableGroup<Severity>, SimpleEnableGroup<Checks>> &groups)
{
    // Enable parameters may be comma separated...
    if (str.find(',') != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(',', pos)) != std::string::npos) {
            if (pos == prevPos)
                return std::string("--enable parameter is empty");
            std::string errmsg(parseEnabled(str.substr(prevPos, pos - prevPos), groups));
            if (!errmsg.empty())
                return errmsg;
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            return std::string("--enable parameter is empty");
        return parseEnabled(str.substr(prevPos), groups);
    }

    auto& severity = std::get<0>(groups);
    auto& checks = std::get<1>(groups);

    if (str == "all") {
        // "error" is always enabled and cannot be controlled - so exclude it from "all"
        SimpleEnableGroup<Severity> newSeverity;
        newSeverity.fill();
        newSeverity.disable(Severity::error);
        severity.enable(newSeverity);
        checks.enable(Checks::missingInclude);
        checks.enable(Checks::unusedFunction);
    } else if (str == "warning") {
        severity.enable(Severity::warning);
    } else if (str == "style") {
        severity.enable(Severity::style);
    } else if (str == "performance") {
        severity.enable(Severity::performance);
    } else if (str == "portability") {
        severity.enable(Severity::portability);
    } else if (str == "information") {
        severity.enable(Severity::information);
    } else if (str == "unusedFunction") {
        checks.enable(Checks::unusedFunction);
    } else if (str == "missingInclude") {
        checks.enable(Checks::missingInclude);
    }
#ifdef CHECK_INTERNAL
    else if (str == "internal") {
        checks.enable(Checks::internalCheck);
    }
#endif
    else {
        // the actual option is prepending in the applyEnabled() call
        if (str.empty())
            return " parameter is empty";
        return " parameter with the unknown name '" + str + "'";
    }

    return "";
}

std::string Settings::addEnabled(const std::string &str)
{
    return applyEnabled(str, true);
}

std::string Settings::removeEnabled(const std::string &str)
{
    return applyEnabled(str, false);
}

std::string Settings::applyEnabled(const std::string &str, bool enable)
{
    std::tuple<SimpleEnableGroup<Severity>, SimpleEnableGroup<Checks>> groups;
    std::string errmsg = parseEnabled(str, groups);
    if (!errmsg.empty())
        return (enable ? "--enable" : "--disable") + errmsg;

    const auto s = std::get<0>(groups);
    const auto c = std::get<1>(groups);
    if (enable) {
        severity.enable(s);
        checks.enable(c);
    }
    else {
        severity.disable(s);
        checks.disable(c);
    }
    // FIXME: hack to make sure "error" is always enabled
    severity.enable(Severity::error);
    return {};
}

bool Settings::isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck) const
{
    if (!severity.isEnabled(Severity::warning) && (value->condition || value->defaultArg))
        return false;
    if (!certainty.isEnabled(Certainty::inconclusive) && (inconclusiveCheck || value->isInconclusive()))
        return false;
    return true;
}

void Settings::loadSummaries()
{
    Summaries::loadReturn(buildDir, summaryReturn);
}

void Settings::setCheckLevel(CheckLevel level)
{
    if (level == CheckLevel::reduced) {
        // Checking should finish quickly.
        checkLevel = level;
        vfOptions.maxSubFunctionArgs = 8;
        vfOptions.maxIfCount = 100;
        vfOptions.doConditionExpressionAnalysis = false;
        vfOptions.maxForwardBranches = 4;
        vfOptions.maxIterations = 1;
    }
    else if (level == CheckLevel::normal) {
        // Checking should finish in reasonable time.
        checkLevel = level;
        vfOptions.maxSubFunctionArgs = 8;
        vfOptions.maxIfCount = 100;
        vfOptions.doConditionExpressionAnalysis = false;
        vfOptions.maxForwardBranches = 4;
    }
    else if (level == CheckLevel::exhaustive) {
        // Checking can take a little while. ~ 10 times slower than normal analysis is OK.
        checkLevel = CheckLevel::exhaustive;
        vfOptions.maxIfCount = -1;
        vfOptions.maxSubFunctionArgs = 256;
        vfOptions.doConditionExpressionAnalysis = true;
        vfOptions.maxForwardBranches = -1;
    }
}

// These tables are auto generated from Cppcheck Premium script

static const std::set<std::string> autosarCheckers{
    "accessMoved",
    "argumentSize",
    "arrayIndexOutOfBounds",
    "arrayIndexOutOfBoundsCond",
    "arrayIndexThenCheck",
    "bufferAccessOutOfBounds",
    "comparePointers",
    "constParameter",
    "ctuOneDefinitionRuleViolation",
    "doubleFree",
    "duplInheritedMember",
    "duplicateBreak",
    "exceptThrowInDestructor",
    "funcArgNamesDifferent",
    "functionConst",
    "functionStatic",
    "invalidContainer",
    "memleak",
    "mismatchAllocDealloc",
    "missingReturn",
    "negativeIndex",
    "noExplicitConstructor",
    "nullPointer",
    "nullPointerArithmetic",
    "nullPointerArithmeticRedundantCheck",
    "nullPointerDefaultArg",
    "nullPointerRedundantCheck",
    "objectIndex",
    "overlappingWriteFunction",
    "overlappingWriteUnion",
    "pointerOutOfBounds",
    "pointerOutOfBoundsCond",
    "preprocessorErrorDirective",
    "redundantAssignment",
    "redundantInitialization",
    "returnDanglingLifetime",
    "shadowArgument",
    "shadowFunction",
    "shadowVariable",
    "shiftTooManyBits",
    "sizeofFunctionCall",
    "throwInNoexceptFunction",
    "uninitMemberVar",
    "uninitdata",
    "unreachableCode",
    "unreadVariable",
    "unsignedLessThanZero",
    "unusedFunction",
    "unusedStructMember",
    "unusedVariable",
    "useInitializationList",
    "variableScope",
    "virtualCallInConstructor",
    "zerodiv",
    "zerodivcond"
};

static const std::set<std::string> certCCheckers{
    "IOWithoutPositioning",
    "argumentSize",
    "arrayIndexOutOfBounds",
    "arrayIndexOutOfBoundsCond",
    "arrayIndexThenCheck",
    "autoVariables",
    "autovarInvalidDeallocation",
    "bitwiseOnBoolean",
    "bufferAccessOutOfBounds",
    "comparePointers",
    "danglingLifetime",
    "deallocret",
    "deallocuse",
    "doubleFree",
    "floatConversionOverflow",
    "integerOverflow",
    "invalidFunctionArg",
    "invalidLengthModifierError",
    "invalidLifetime",
    "invalidScanfFormatWidth",
    "invalidscanf",
    "leakReturnValNotUsed",
    "leakUnsafeArgAlloc",
    "memleak",
    "memleakOnRealloc",
    "mismatchAllocDealloc",
    "missingReturn",
    "negativeIndex",
    "nullPointer",
    "nullPointerArithmetic",
    "nullPointerArithmeticRedundantCheck",
    "nullPointerDefaultArg",
    "nullPointerRedundantCheck",
    "objectIndex",
    "pointerOutOfBounds",
    "pointerOutOfBoundsCond",
    "preprocessorErrorDirective",
    "resourceLeak",
    "returnDanglingLifetime",
    "sizeofCalculation",
    "stringLiteralWrite",
    "uninitStructMember",
    "uninitdata",
    "uninitvar",
    "unknownEvaluationOrder",
    "useClosedFile",
    "wrongPrintfScanfArgNum",
    "wrongPrintfScanfParameterPositionError"
};

static const std::set<std::string> certCppCheckers{
    "IOWithoutPositioning",
    "accessMoved",
    "argumentSize",
    "arrayIndexOutOfBounds",
    "arrayIndexOutOfBoundsCond",
    "arrayIndexThenCheck",
    "autoVariables",
    "autovarInvalidDeallocation",
    "bitwiseOnBoolean",
    "bufferAccessOutOfBounds",
    "comparePointers",
    "containerOutOfBounds",
    "ctuOneDefinitionRuleViolation",
    "danglingLifetime",
    "danglingReference",
    "danglingTempReference",
    "danglingTemporaryLifetime",
    "deallocret",
    "deallocuse",
    "doubleFree",
    "eraseDereference",
    "exceptDeallocThrow",
    "exceptThrowInDestructor",
    "floatConversionOverflow",
    "initializerList",
    "integerOverflow",
    "invalidContainer",
    "invalidFunctionArg",
    "invalidLengthModifierError",
    "invalidLifetime",
    "invalidScanfFormatWidth",
    "invalidscanf",
    "leakReturnValNotUsed",
    "leakUnsafeArgAlloc",
    "memleak",
    "memleakOnRealloc",
    "mismatchAllocDealloc",
    "missingReturn",
    "negativeIndex",
    "nullPointer",
    "nullPointerArithmetic",
    "nullPointerArithmeticRedundantCheck",
    "nullPointerDefaultArg",
    "nullPointerRedundantCheck",
    "objectIndex",
    "operatorEqToSelf",
    "pointerOutOfBounds",
    "pointerOutOfBoundsCond",
    "preprocessorErrorDirective",
    "resourceLeak",
    "returnDanglingLifetime",
    "sizeofCalculation",
    "stringLiteralWrite",
    "uninitStructMember",
    "uninitdata",
    "uninitvar",
    "useClosedFile",
    "virtualCallInConstructor",
    "virtualDestructor",
    "wrongPrintfScanfArgNum",
    "wrongPrintfScanfParameterPositionError"
};

static const std::set<std::string> misrac2012Checkers{
    "argumentSize",
    "autovarInvalidDeallocation",
    "bufferAccessOutOfBounds",
    "comparePointers",
    "compareValueOutOfTypeRangeError",
    "constParameterPointer",
    "constStatement",
    "danglingLifetime",
    "danglingTemporaryLifetime",
    "duplicateBreak",
    "funcArgNamesDifferent",
    "incompatibleFileOpen",
    "invalidFunctionArg",
    "knownConditionTrueFalse",
    "leakNoVarFunctionCall",
    "leakReturnValNotUsed",
    "memleak",
    "memleakOnRealloc",
    "missingReturn",
    "overlappingWriteFunction",
    "overlappingWriteUnion",
    "pointerOutOfBounds",
    "preprocessorErrorDirective",
    "redundantAssignInSwitch",
    "redundantAssignment",
    "redundantCondition",
    "resourceLeak",
    "returnDanglingLifetime",
    "shadowVariable",
    "sizeofCalculation",
    "sizeofwithsilentarraypointer",
    "syntaxError",
    "uninitvar",
    "unknownEvaluationOrder",
    "unreachableCode",
    "unreadVariable",
    "unusedFunction",
    "unusedLabel",
    "unusedVariable",
    "useClosedFile",
    "writeReadOnlyFile"
};

static const std::set<std::string> misrac2023Checkers{
    "argumentSize",
    "autovarInvalidDeallocation",
    "bufferAccessOutOfBounds",
    "comparePointers",
    "compareValueOutOfTypeRangeError",
    "constParameterPointer",
    "constStatement",
    "danglingLifetime",
    "danglingTemporaryLifetime",
    "duplicateBreak",
    "funcArgNamesDifferent",
    "incompatibleFileOpen",
    "invalidFunctionArg",
    "knownConditionTrueFalse",
    "leakNoVarFunctionCall",
    "leakReturnValNotUsed",
    "memleak",
    "memleakOnRealloc",
    "missingReturn",
    "overlappingWriteFunction",
    "overlappingWriteUnion",
    "pointerOutOfBounds",
    "preprocessorErrorDirective",
    "redundantAssignInSwitch",
    "redundantAssignment",
    "redundantCondition",
    "resourceLeak",
    "returnDanglingLifetime",
    "shadowVariable",
    "sizeofCalculation",
    "sizeofwithsilentarraypointer",
    "syntaxError",
    "uninitvar",
    "unknownEvaluationOrder",
    "unreachableCode",
    "unreadVariable",
    "unusedFunction",
    "unusedLabel",
    "unusedVariable",
    "useClosedFile",
    "writeReadOnlyFile"
};

static const std::set<std::string> misrac2025Checkers{
    "argumentSize",
    "autovarInvalidDeallocation",
    "bufferAccessOutOfBounds",
    "comparePointers",
    "compareValueOutOfTypeRangeError",
    "constParameterPointer",
    "constStatement",
    "danglingLifetime",
    "danglingTemporaryLifetime",
    "duplicateBreak",
    "funcArgNamesDifferent",
    "incompatibleFileOpen",
    "invalidFunctionArg",
    "knownConditionTrueFalse",
    "leakNoVarFunctionCall",
    "leakReturnValNotUsed",
    "memleak",
    "memleakOnRealloc",
    "missingReturn",
    "overlappingWriteFunction",
    "overlappingWriteUnion",
    "pointerOutOfBounds",
    "preprocessorErrorDirective",
    "redundantAssignInSwitch",
    "redundantAssignment",
    "redundantCondition",
    "resourceLeak",
    "returnDanglingLifetime",
    "shadowVariable",
    "sizeofCalculation",
    "sizeofwithsilentarraypointer",
    "syntaxError",
    "uninitvar",
    "unknownEvaluationOrder",
    "unreachableCode",
    "unreadVariable",
    "unusedFunction",
    "unusedLabel",
    "unusedVariable",
    "useClosedFile",
    "writeReadOnlyFile"
};

static const std::set<std::string> misracpp2008Checkers{
    "autoVariables",
    "comparePointers",
    "constParameter",
    "constVariable",
    "cstyleCast",
    "ctuOneDefinitionRuleViolation",
    "dangerousTypeCast",
    "danglingLifetime",
    "duplInheritedMember",
    "duplicateBreak",
    "exceptThrowInDestructor",
    "funcArgNamesDifferent",
    "functionConst",
    "functionStatic",
    "missingReturn",
    "noExplicitConstructor",
    "overlappingWriteFunction",
    "overlappingWriteUnion",
    "pointerOutOfBounds",
    "preprocessorErrorDirective",
    "redundantAssignment",
    "redundantInitialization",
    "returnReference",
    "returnTempReference",
    "shadowVariable",
    "shiftTooManyBits",
    "sizeofFunctionCall",
    "uninitDerivedMemberVar",
    "uninitDerivedMemberVarPrivate",
    "uninitMemberVar",
    "uninitMemberVarPrivate",
    "uninitStructMember",
    "uninitdata",
    "uninitvar",
    "unknownEvaluationOrder",
    "unreachableCode",
    "unreadVariable",
    "unsignedLessThanZero",
    "unusedFunction",
    "unusedStructMember",
    "unusedVariable",
    "variableScope",
    "virtualCallInConstructor"
};

static const std::set<std::string> misracpp2023Checkers{
    "accessForwarded",
    "accessMoved",
    "autoVariables",
    "compareBoolExpressionWithInt",
    "comparePointers",
    "compareValueOutOfTypeRangeError",
    "constParameter",
    "constParameterReference",
    "ctuOneDefinitionRuleViolation",
    "danglingLifetime",
    "floatConversionOverflow",
    "identicalConditionAfterEarlyExit",
    "identicalInnerCondition",
    "ignoredReturnValue",
    "invalidFunctionArg",
    "invalidFunctionArgBool",
    "invalidFunctionArgStr",
    "knownConditionTrueFalse",
    "missingReturn",
    "noExplicitConstructor",
    "operatorEqToSelf",
    "overlappingWriteUnion",
    "pointerOutOfBounds",
    "pointerOutOfBoundsCond",
    "preprocessorErrorDirective",
    "redundantAssignInSwitch",
    "redundantAssignment",
    "redundantCopy",
    "redundantInitialization",
    "shadowVariable",
    "subtractPointers",
    "syntaxError",
    "uninitMemberVar",
    "uninitvar",
    "unknownEvaluationOrder",
    "unreachableCode",
    "unreadVariable",
    "virtualCallInConstructor"
};

bool Settings::isPremiumEnabled(const char id[]) const
{
    if (premiumArgs.find("autosar") != std::string::npos && autosarCheckers.count(id))
        return true;
    if (premiumArgs.find("cert-c-") != std::string::npos && certCCheckers.count(id))
        return true;
    if (premiumArgs.find("cert-c++") != std::string::npos && certCppCheckers.count(id))
        return true;
    if (premiumArgs.find("misra-c-") != std::string::npos && (misrac2012Checkers.count(id) || misrac2023Checkers.count(id) || misrac2025Checkers.count(id)))
        return true;
    if (premiumArgs.find("misra-c++-2008") != std::string::npos && misracpp2008Checkers.count(id))
        return true;
    if (premiumArgs.find("misra-c++-2023") != std::string::npos && misracpp2023Checkers.count(id))
        return true;
    return false;
}

Settings::ExecutorType Settings::defaultExecutor()
{
    static constexpr ExecutorType defaultExecutor =
#if defined(HAS_THREADING_MODEL_FORK)
        ExecutorType::Process;
#elif defined(HAS_THREADING_MODEL_THREAD)
        ExecutorType::Thread;
#endif
    return defaultExecutor;
}

bool Settings::unusedFunctionOnly()
{
    const char* unusedFunctionOnly = std::getenv("UNUSEDFUNCTION_ONLY");
    return unusedFunctionOnly && (std::strcmp(unusedFunctionOnly, "1") == 0);
}
