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

#include "importproject.h"

#include "path.h"
#include "pathmatch.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stack>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include "xml.h"

#include "json.h"

std::string ImportProject::collectArgs(const std::string &cmd, std::vector<std::string> &args)
{
    args.clear();

    std::string::size_type pos = 0;
    const std::string::size_type end = cmd.size();
    std::string arg;

    bool inDoubleQuotes = false;
    bool inSingleQuotes = false;

    while (pos < end) {
        char c = cmd[pos++];

        if (c == ' ') {
            if (inDoubleQuotes || inSingleQuotes) {
                arg.push_back(c);
                continue;
            }

            if (!arg.empty())
                args.push_back(arg);
            arg.clear();

            pos = cmd.find_first_not_of(' ', pos);

            continue;
        }

        if (c == '\"' && !inSingleQuotes) {
            inDoubleQuotes = !inDoubleQuotes;
            continue;
        }

        if (c == '\'' && !inDoubleQuotes) {
            inSingleQuotes = !inSingleQuotes;
            continue;
        }

        if (c == '\\' && !inSingleQuotes) {
            if (pos == end) {
                arg.push_back('\\');
                break;
            }

            c = cmd[pos++];

            if (!std::strchr("\\\"\' ", c))
                arg.push_back('\\');

            arg.push_back(c);
            continue;
        }

        arg.push_back(c);
    }

    if (inSingleQuotes || inDoubleQuotes)
        return "Missing closing quote in command string";

    if (!arg.empty())
        args.push_back(std::move(arg));

    return "";
}

void ImportProject::parseArgs(FileSettings &fs, const std::vector<std::string> &args)
{
    const auto getOptArg = [&args](std::initializer_list<std::string> optNames,
                                   std::size_t &i) {
        const auto &arg = args[i];
        const auto *const it = std::find_if(optNames.begin(),
                                            optNames.end(),
                                            [&arg] (const std::string &optName) {
            return startsWith(arg, optName);
        });

        if (it == optNames.end())
            return std::string();

        const std::size_t optLen = it->size();
        if (arg.size() == optLen)
            return ++i >= args.size() ? std::string() : args[i];

        return arg.substr(optLen);
    };

    std::string defs;
    for (std::size_t i = 0; i < args.size(); i++) {
        std::string optArg;

        if (!(optArg = getOptArg({ "-I", "/I" }, i)).empty()) {
            if (std::none_of(fs.includePaths.cbegin(), fs.includePaths.cend(),
                             [&](const std::string &path) {
                return path == optArg;
            }))
                fs.includePaths.push_back(std::move(optArg));
            continue;
        }

        if (!(optArg = getOptArg({ "-isystem" }, i)).empty()) {
            fs.systemIncludePaths.push_back(std::move(optArg));
            continue;
        }

        if (!(optArg = getOptArg({ "-include", "/FI", "-FI" }, i)).empty()) {
            fs.forcedIncludes.push_back(std::move(optArg));
            continue;
        }

        if (!(optArg = getOptArg({ "-D", "/D" }, i)).empty()) {
            defs += optArg + ";";
            continue;
        }

        if (!(optArg = getOptArg({ "-U", "/U" }, i)).empty()) {
            fs.undefs.insert(std::move(optArg));
            continue;
        }

        if (!(optArg = getOptArg({ "-std=", "/std:" }, i)).empty()) {
            fs.standard = std::move(optArg);
            continue;
        }

        if (!(optArg = getOptArg({ "-f" }, i)).empty()) {
            if (optArg == "pic")
                defs += "__pic__;";
            else if (optArg == "PIC")
                defs += "__PIC__;";
            else if (optArg == "pie")
                defs += "__pie__;";
            else if (optArg == "PIE")
                defs += "__PIE__;";
            continue;
        }

        if (!(optArg = getOptArg({ "-m" }, i)).empty()) {
            if (optArg == "unicode")
                defs += "UNICODE;";
            continue;
        }
    }

    fsSetDefines(fs, std::move(defs));
}

void ImportProject::ignorePaths(const std::vector<std::string> &ipaths, bool debug)
{
    PathMatch matcher(ipaths, Path::getCurrentPath());
    for (auto it = fileSettings.cbegin(); it != fileSettings.cend();) {
        if (matcher.match(it->filename())) {
            if (debug)
                std::cout << "ignored path: " << it->filename() << std::endl;
            it = fileSettings.erase(it);
        }
        else
            ++it;
    }
}

void ImportProject::ignoreOtherConfigs(const std::string &cfg)
{
    for (auto it = fileSettings.cbegin(); it != fileSettings.cend();) {
        if (it->cfg != cfg)
            it = fileSettings.erase(it);
        else
            ++it;
    }
}

void ImportProject::fsSetDefines(FileSettings& fs, std::string defs)
{
    while (defs.find(";%(") != std::string::npos) {
        const std::string::size_type pos1 = defs.find(";%(");
        const std::string::size_type pos2 = defs.find(';', pos1+1);
        defs.erase(pos1, pos2 == std::string::npos ? pos2 : (pos2-pos1));
    }
    while (defs.find(";;") != std::string::npos)
        defs.erase(defs.find(";;"),1);
    while (!defs.empty() && defs[0] == ';')
        defs.erase(0, 1);
    while (!defs.empty() && endsWith(defs,';'))
        defs.pop_back();
    bool eq = false;
    for (std::size_t pos = 0; pos < defs.size(); ++pos) {
        if (defs[pos] == '(' || defs[pos] == '=')
            eq = true;
        else if (defs[pos] == ';') {
            if (!eq) {
                defs.insert(pos,"=1");
                pos += 3;
            }
            if (pos < defs.size())
                eq = false;
        }
    }
    if (!eq && !defs.empty())
        defs += "=1";
    fs.defines.swap(defs);
}

static bool simplifyPathWithVariables(std::string &s, VariablesMap &variables)
{
    std::set<std::string, cppcheck::stricmp> expanded;
    std::string::size_type start = 0;
    while ((start = s.find("$(")) != std::string::npos) {
        const std::string::size_type end = s.find(')',start);
        if (end == std::string::npos)
            break;
        const std::string var = s.substr(start+2,end-start-2);
        if (expanded.find(var) != expanded.end())
            break;
        expanded.insert(var);
        auto it1 = utils::as_const(variables).find(var);
        // variable was not found within defined variables
        if (it1 == variables.end()) {
            const char *envValue = std::getenv(var.c_str());
            if (!envValue) {
                //! \todo generate a debug/info message about undefined variable
                break;
            }
            variables[var] = std::string(envValue);
            it1 = variables.find(var);
        }
        s.replace(start, end - start + 1, it1->second);
    }
    if (s.find("$(") != std::string::npos)
        return false;
    s = Path::simplifyPath(std::move(s));
    return true;
}

static std::string toAbsolute(const std::string &path)
{
    if (Path::isAbsolute(path))
        return Path::simplifyPath(path);
    return Path::simplifyPath(Path::getCurrentPath() + "/" + path);
}

static std::string toAbsolute(const std::string &filename, const std::string &baseDir, VariablesMap &variables)
{
    std::string resolved(filename);
    simplifyPathWithVariables(resolved, variables);

    if (Path::isAbsolute(resolved))
        return Path::simplifyPath(resolved);
    return Path::simplifyPath(baseDir + resolved);
}

void ImportProject::fsSetIncludePaths(FileSettings& fs, const std::string &basepath, const std::list<std::string> &in, VariablesMap &variables)
{
    std::set<std::string> found;
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const std::list<std::string> copyIn(in);
    fs.includePaths.clear();
    for (const std::string &ipath : copyIn) {
        if (ipath.empty())
            continue;
        if (startsWith(ipath,"%("))
            continue;
        std::string s(Path::fromNativeSeparators(ipath));
        if (!found.insert(s).second)
            continue;
        if (s[0] == '/' || (s.size() > 1U && s.compare(1,2,":/") == 0)) {
            if (!endsWith(s,'/'))
                s += '/';
            fs.includePaths.push_back(std::move(s));
            continue;
        }

        if (endsWith(s,'/')) // this is a temporary hack, simplifyPath can crash if path ends with '/'
            s.pop_back();

        if (s.find("$(") == std::string::npos) {
            s = Path::simplifyPath(basepath + s);
        } else {
            if (!simplifyPathWithVariables(s, variables))
                continue;
        }
        if (s.empty())
            continue;
        fs.includePaths.push_back(s.back() == '/' ? s : (s + '/'));
    }
}

ImportProject::Type ImportProject::import(const std::string &filename, Settings *settings, Suppressions *supprs)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
        return ImportProject::Type::MISSING;

    mPath = Path::getPathFromFilename(Path::fromNativeSeparators(filename));
    if (!mPath.empty() && !endsWith(mPath,'/'))
        mPath += '/';

    const std::vector<std::string> fileFilters =
        settings ? settings->fileFilters : std::vector<std::string>();

    if (endsWith(filename, ".json")) {
        if (importCompileCommands(fin)) {
            setRelativePaths(filename);
            return ImportProject::Type::COMPILE_DB;
        }
    } else if (endsWith(filename, ".sln")) {
        if (importSln(fin, mPath, fileFilters)) {
            setRelativePaths(filename);
            return ImportProject::Type::VS_SLN;
        }
    } else if (endsWith(filename, ".slnx")) {
        if (importSlnx(filename, fileFilters)) {
            setRelativePaths(filename);
            return ImportProject::Type::VS_SLNX;
        }
    } else if (endsWith(filename, ".vcxproj")) {
        if (importVcxproj(toAbsolute(filename), mVariables, fileFilters)) {
            setRelativePaths(filename);
            return ImportProject::Type::VS_VCXPROJ;
        }
    } else if (endsWith(filename, ".bpr")) {
        if (importBcb6Prj(filename)) {
            setRelativePaths(filename);
            return ImportProject::Type::BORLAND;
        }
    } else if (settings && supprs && endsWith(filename, ".cppcheck")) {
        if (importCppcheckGuiProject(fin, *settings, *supprs)) {
            setRelativePaths(filename);
            return ImportProject::Type::CPPCHECK_GUI;
        }
    } else {
        return ImportProject::Type::UNKNOWN;
    }
    return ImportProject::Type::FAILURE;
}

bool ImportProject::importCompileCommands(std::istream &istr)
{
    picojson::value compileCommands;
    istr >> compileCommands;
    if (!compileCommands.is<picojson::array>()) {
        errors.emplace_back("compilation database is not a JSON array");
        return false;
    }

    std::map<std::string, std::size_t> fsFileIds;

    for (const picojson::value &fileInfo : compileCommands.get<picojson::array>()) {
        picojson::object obj = fileInfo.get<picojson::object>();

        if (obj.count("directory") == 0) {
            errors.emplace_back("'directory' field in compilation database entry missing");
            return false;
        }

        if (!obj["directory"].is<std::string>()) {
            errors.emplace_back("'directory' field in compilation database entry is not a string");
            return false;
        }

        std::string dirpath = Path::fromNativeSeparators(obj["directory"].get<std::string>());

        /* CMAKE produces the directory without trailing / so add it if not
         * there - it is needed by setIncludePaths() */
        if (!endsWith(dirpath, '/'))
            dirpath += '/';

        const std::string directory = std::move(dirpath);

        std::vector<std::string> arguments;
        if (obj.count("arguments")) {
            if (obj["arguments"].is<picojson::array>()) {
                for (const picojson::value& arg : obj["arguments"].get<picojson::array>()) {
                    if (arg.is<std::string>())
                        arguments.push_back(arg.get<std::string>());
                }
            } else {
                errors.emplace_back("'arguments' field in compilation database entry is not a JSON array");
                return false;
            }
        } else if (obj.count("command")) {
            std::string command;
            if (obj["command"].is<std::string>()) {
                command = obj["command"].get<std::string>();
            } else {
                errors.emplace_back("'command' field in compilation database entry is not a string");
                return false;
            }

            std::string error = collectArgs(command, arguments);
            if (!error.empty()) {
                errors.emplace_back(error);
                return false;
            }
        } else {
            errors.emplace_back("no 'arguments' or 'command' field found in compilation database entry");
            return false;
        }

        if (!obj.count("file") || !obj["file"].is<std::string>()) {
            errors.emplace_back("skip compilation database entry because it does not have a proper 'file' field");
            continue;
        }

        std::string file = Path::fromNativeSeparators(obj["file"].get<std::string>());

        // Accept file?
        if (!Path::acceptFile(file))
            continue;

        std::string path;
        if (Path::isAbsolute(file))
            path = Path::simplifyPath(std::move(file));
#ifdef _WIN32
        else if (file[0] == '/' && directory.size() > 2 && std::isalpha(directory[0]) && directory[1] == ':')
            // directory: C:\foo\bar
            // file: /xy/z.c
            // => c:/xy/z.c
            path = Path::simplifyPath(directory.substr(0,2) + file);
#endif
        else
            path = Path::simplifyPath(directory + file);
        FileSettings fs{path, Standards::Language::None, 0}; // file will be identified later on
        parseArgs(fs, arguments);
        VariablesMap variables;
        fsSetIncludePaths(fs, directory, fs.includePaths, variables);
        // Assign a unique index to each file path. If the file path already exists in the map,
        // increment the index to handle duplicate file entries.
        fs.file.setFsFileId(fsFileIds[path]++);
        fileSettings.push_back(std::move(fs));
    }

    return true;
}

bool ImportProject::importSln(std::istream &istr, const std::string &path, const std::vector<std::string> &fileFilters)
{
    std::string line;

    if (!std::getline(istr,line)) {
        errors.emplace_back("Visual Studio solution file is empty");
        return false;
    }

    if (!startsWith(line, "Microsoft Visual Studio Solution File")) {
        // Skip BOM
        if (!std::getline(istr, line) || !startsWith(line, "Microsoft Visual Studio Solution File")) {
            errors.emplace_back("Visual Studio solution file header not found");
            return false;
        }
    }

    const std::string solutionDir = toAbsolute(path);
    mVariables["SolutionDir"] = solutionDir;

    bool found = false;
    while (std::getline(istr,line)) {
        if (!startsWith(line,"Project("))
            continue;
        const std::string::size_type pos = line.find(".vcxproj");
        if (pos == std::string::npos)
            continue;
        const std::string::size_type pos1 = line.rfind('\"',pos);
        if (pos1 == std::string::npos)
            continue;
        std::string vcxproj(line.substr(pos1+1, pos-pos1+7));
        vcxproj = Path::toNativeSeparators(std::move(vcxproj));
        vcxproj = toAbsolute(vcxproj, solutionDir, mVariables);
        vcxproj = Path::fromNativeSeparators(std::move(vcxproj));
        if (!importVcxproj(vcxproj, mVariables, fileFilters)) {
            errors.emplace_back("failed to load '" + vcxproj + "' from Visual Studio solution");
            return false;
        }
        found = true;
    }

    if (!found) {
        errors.emplace_back("no projects found in Visual Studio solution file");
        return false;
    }

    return true;
}

bool ImportProject::importSlnx(const std::string& filename, const std::vector<std::string>& fileFilters)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        errors.emplace_back(std::string("Visual Studio solution file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }

    const tinyxml2::XMLElement* const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr) {
        errors.emplace_back("Visual Studio solution file has no XML root node");
        return false;
    }

    if (std::strcmp(rootnode->Name(), "Solution") != 0) {
        errors.emplace_back("Invalid Visual Studio solution file format");
        return false;
    }

    mVariables["SolutionDir"] = toAbsolute(Path::getPathFromFilename(filename));

    bool found = false;

    auto processProject = [&](const tinyxml2::XMLElement* projectNode) {
        const char* pathAttribute = projectNode->Attribute("Path");
        if (pathAttribute == nullptr)
            return true;

        std::string vcxproj(pathAttribute);
        vcxproj = Path::toNativeSeparators(std::move(vcxproj));

        if (Path::getFilenameExtensionInLowerCase(vcxproj) != ".vcxproj")
            return true; // skip other project types

        vcxproj = toAbsolute(vcxproj, mVariables["SolutionDir"], mVariables);

        vcxproj = Path::fromNativeSeparators(std::move(vcxproj));
        if (!importVcxproj(vcxproj, mVariables, fileFilters)) {
            errors.emplace_back("failed to load '" + vcxproj + "' from Visual Studio solution");
            return false;
        }
        found = true;
        return true;
    };

    for (const tinyxml2::XMLElement* node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const char* name = node->Name();
        if (std::strcmp(name, "Project") == 0) {
            if (!processProject(node))
                return false;
        } else if (std::strcmp(name, "Folder") == 0) {
            for (const tinyxml2::XMLElement* childNode = node->FirstChildElement(); childNode; childNode = childNode->NextSiblingElement()) {
                if (std::strcmp(childNode->Name(), "Project") == 0) {
                    if (!processProject(childNode))
                        return false;
                }
            }
        }
    }

    if (!found) {
        errors.emplace_back("no projects found in Visual Studio solution file");
        return false;
    }

    return true;
}

namespace {
    struct ProjectConfiguration {
        explicit ProjectConfiguration(const tinyxml2::XMLElement *cfg) {
            const char *a = cfg->Attribute("Include");
            if (a)
                name = a;
            for (const tinyxml2::XMLElement *e = cfg->FirstChildElement(); e; e = e->NextSiblingElement()) {
                const char * const text = e->GetText();
                if (!text)
                    continue;
                const char * ename = e->Name();
                if (std::strcmp(ename,"Configuration")==0)
                    configuration = text;
                else if (std::strcmp(ename,"Platform")==0) {
                    platformStr = text;
                    if (platformStr == "Win32")
                        platform = Win32;
                    else if (platformStr == "x64")
                        platform = x64;
                    else
                        platform = Unknown;
                }
            }
        }
        std::string name;
        std::string configuration;
        enum : std::uint8_t { Win32, x64, Unknown } platform = Unknown;
        std::string platformStr;
    };

    // see https://learn.microsoft.com/en-us/visualstudio/msbuild/msbuild-conditions
    // properties are .NET String objects and you can call any of its members on them
    std::string execute(const Token* tok, const VariablesMap &variables) {
        if (!tok)
            throw std::runtime_error("Missing operator");
        auto boolResult = [](bool b) -> std::string {
            return b ? "True" : "False";
        };
        if (tok->isUnaryOp("!"))
            return boolResult(execute(tok->astOperand1(), variables) == "False");
        if (tok->str() == "==")
            return boolResult(execute(tok->astOperand1(), variables) == execute(tok->astOperand2(), variables));
        if (tok->str() == "!=")
            return boolResult(execute(tok->astOperand1(), variables) != execute(tok->astOperand2(), variables));
        if (tok->str() == "<" || tok->str() == ">" || tok->str() == "<=" || tok->str() == ">=") {
            const std::string lhs = execute(tok->astOperand1(), variables);
            const std::string rhs = execute(tok->astOperand2(), variables);
            // Try integer comparison first
            char* lhsEnd = nullptr;
            char* rhsEnd = nullptr;
            const long lhsInt = std::strtol(lhs.c_str(), &lhsEnd, 10);
            const long rhsInt = std::strtol(rhs.c_str(), &rhsEnd, 10);
            if (lhsEnd == lhs.c_str() + lhs.size() && rhsEnd == rhs.c_str() + rhs.size()) {
                if (tok->str() == "<") return boolResult(lhsInt <  rhsInt);
                if (tok->str() == ">") return boolResult(lhsInt >  rhsInt);
                if (tok->str() == "<=") return boolResult(lhsInt <= rhsInt);
                if (tok->str() == ">=") return boolResult(lhsInt >= rhsInt);
            }
            // Fall back to dotted-version comparison (e.g. "14.0" >= "12.0")
            const auto parseVersion = [](const std::string& s) -> std::vector<int> {
                std::vector<int> parts;
                std::string::size_type pos = 0;
                // strip leading v/V
                if (!s.empty() && (s[0] == 'v' || s[0] == 'V'))
                    pos = 1;
                while (pos <= s.size()) {
                    const std::string::size_type dot = s.find('.', pos);
                    const std::string part = s.substr(pos, dot == std::string::npos ? std::string::npos : dot - pos);
                    char* end = nullptr;
                    const long val = std::strtol(part.c_str(), &end, 10);
                    if (end == part.c_str())
                        return {}; // not a valid version part
                    parts.push_back(static_cast<int>(val));
                    if (dot == std::string::npos)
                        break;
                    pos = dot + 1;
                }
                // pad to 4 parts with zeros, matching System.Version semantics
                while (parts.size() < 4)
                    parts.push_back(0);
                return parts;
            };
            const std::vector<int> lhsVer = parseVersion(lhs);
            const std::vector<int> rhsVer = parseVersion(rhs);
            if (!lhsVer.empty() && !rhsVer.empty()) {
                if (tok->str() == "<") return boolResult(lhsVer <  rhsVer);
                if (tok->str() == ">") return boolResult(lhsVer >  rhsVer);
                if (tok->str() == "<=") return boolResult(lhsVer <= rhsVer);
                if (tok->str() == ">=") return boolResult(lhsVer >= rhsVer);
            }
            throw std::runtime_error("Cannot compare '" + lhs + "' and '" + rhs + "'");
        }
        if (tok->str() == "&&")
            return boolResult(execute(tok->astOperand1(), variables) == "True" && execute(tok->astOperand2(), variables) == "True");
        if (tok->str() == "||")
            return boolResult(execute(tok->astOperand1(), variables) == "True" || execute(tok->astOperand2(), variables) == "True");
        if (tok->str() == "(" && Token::Match(tok->previous(), "$ ( %name% . %name% (")) {
            const std::string &varName = tok->strAt(1);
            auto varIt = variables.find(varName);
            std::string property = (varIt != variables.end()) ? varIt->second : varName;
            const std::string &method = tok->strAt(3);
            if (method == "ToUpper") {
                std::string result = property;
                std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
                    return std::toupper(c);
                });
                return result;
            }
            if (method == "ToLower") {
                std::string result = property;
                std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
                    return std::tolower(c);
                });
                return result;
            }
            std::string arg = execute(tok->tokAt(4)->astOperand2(), variables);
            if (method == "Contains")
                return boolResult(property.find(arg) != std::string::npos);
            if (method == "EndsWith")
                return boolResult(endsWith(property,arg.c_str(),arg.size()));
            if (method == "StartsWith")
                return boolResult(startsWith(property,arg));
            throw std::runtime_error("Unhandled method '" + method + "'");
        }
        if (tok->str().size() >= 2 && tok->str()[0] == '\'') // String Literal
            return tok->str().substr(1, tok->str().size() - 2);
        if (tok->str() == "(" && tok->previous() && caseInsensitiveStringCompare(tok->strAt(-1), "exists") == 0) {
            return boolResult(false);
        }

        throw std::runtime_error("Unknown/unhandled operator/operand '" + tok->str() + "'");
    }

    bool evalCondition(const std::string &condition, const VariablesMap &variables) {
        std::string c = '(' + condition + ")";
        for (const auto &var : variables)
            findAndReplace(c, "$(" + var.first + ")", var.second);

        const Settings s;
        TokenList tokenlist(s, Standards::Language::C);
        if (!tokenlist.createTokensFromBuffer(c.data(), c.size())) {
            throw std::runtime_error("Can not tokenize condition");
        }

        // generate links
        {
            std::stack<Token *> lpar;
            for (Token *tok2 = tokenlist.front(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    lpar.push(tok2);
                else if (tok2->str() == ")") {
                    if (lpar.empty())
                        throw std::runtime_error("unmatched ')' in condition " + condition);
                    Token::createMutualLinks(lpar.top(), tok2);
                    lpar.pop();
                }
            }
            if (!lpar.empty())
                throw std::runtime_error("'(' without closing ')'!");
        }

        // Replace "And" and "Or" with "&&" and "||"
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->str() == "And")
                tok->str("&&");
            else if (tok->str() == "Or")
                tok->str("||");
        }

        tokenlist.createAst();

        // Locate ast top and execute the condition
        for (const Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->astParent()) {
                return execute(tok->astTop(), variables) == "True";
            }
        }
        throw std::runtime_error("Invalid condition: '" + condition + "'");
    }

    bool conditionIsTrue(const tinyxml2::XMLElement *node, const VariablesMap &variables) {
        const char *condAttr = node->Attribute("Condition");
        if (!condAttr)
            return true;
        std::string condition(condAttr);
        for (const auto &var : variables)
            findAndReplace(condition, "$(" + var.first + ")", var.second);
        return evalCondition(condition, variables);
    }

    bool hasName(const tinyxml2::XMLElement *node, const char *nodeName, const VariablesMap &variables) {
        const char *name = node->Name();
        if (!name || std::strcmp(nodeName, name) != 0)
            return false;
        return conditionIsTrue(node, variables);
    }

    bool hasNameAndLabel(const tinyxml2::XMLElement *node, const char *nodeName, const char *nodeAttr, const VariablesMap &variables) {
        const char *name = node->Name();
        const char *label = node->Attribute("Label");
        if (!name || !label || std::strcmp(nodeName, name) != 0 || std::strcmp(label, nodeAttr) != 0)
            return false;
        return conditionIsTrue(node, variables);
    }

    bool hasNameAndNotLabel(const tinyxml2::XMLElement *node, const char *nodeName, const char *nodeAttr, const VariablesMap &variables) {
        const char *name = node->Name();
        if (!name || std::strcmp(nodeName, name) != 0)
            return false;
        const char *label = node->Attribute("Label");
        if (label && std::strcmp(label, nodeAttr) == 0)
            return false;
        return conditionIsTrue(node, variables);
    }

    void addProperty(const tinyxml2::XMLElement *node, VariablesMap &variables) {
        const char *eName = node->Name();
        const char *eText = node->GetText();
        if (eName && eText && conditionIsTrue(node, variables)) {
            std::string original = variables[eName];
            std::string text(eText);
            for (const auto &var : variables)
                findAndReplace(text, "$(" + var.first + ")", var.second);
            findAndReplace(text, "%(" + std::string(eName) + ")", original);
            variables[eName] = text;
        }
    }

    std::string getProperty(const tinyxml2::XMLElement *node, const VariablesMap &variables, const std::string &original) {
        std::string text;
        const char *eName = node->Name();
        const char *eText = node->GetText();
        if (eName && eText && conditionIsTrue(node, variables)) {
            text = eText;
            for (const auto &var : variables)
                findAndReplace(text, "$(" + var.first + ")", var.second);
            findAndReplace(text, "%(" + std::string(eName) + ")", original);
        }
        return text;
    }

    struct ItemGroupClCompile {
        explicit ItemGroupClCompile(std::string filename) : filename(std::move(filename)) {}
        std::string filename;
        std::string additionalIncludeDirectories;
        std::string forcedIncludeFiles;
        std::string preprocessorDefinitions;
        std::string languageStandard;
    };

    std::list<std::string> toStringList(const std::string &s)
    {
        std::list<std::string> ret;
        std::string::size_type pos1 = 0;
        std::string::size_type pos2;
        while ((pos2 = s.find(';',pos1)) != std::string::npos) {
            ret.push_back(s.substr(pos1, pos2-pos1));
            pos1 = pos2 + 1;
            if (pos1 >= s.size())
                break;
        }
        if (pos1 < s.size())
            ret.push_back(s.substr(pos1));
        return ret;
    }
}

enum class PropsImportResult : std::uint8_t { Ok, NotFound, Cycle };

static PropsImportResult loadVisualStudioProperties(const std::string &props,
                                                    VariablesMap &variables,
                                                    std::unordered_set<std::string> &importStack)
{
    std::string filename(props);
    // variables can't be resolved
    if (!simplifyPathWithVariables(filename, variables))
        return PropsImportResult::NotFound;

    // prepend project dir (if it exists) to transform relative paths into absolute ones
    if (!Path::isAbsolute(filename) && variables.count("ProjectDir") > 0)
        filename = toAbsolute(filename, variables.at("ProjectDir"), variables);

    // detect circular property sheet imports (A imports B, B imports A, a file importing
    // itself, ...) instead of recursing until the stack overflows - mirrors MSBuild's own
    // import-cycle detection, which errors out rather than looping forever
    const std::string simplifiedFilename = Path::simplifyPath(filename);
    if (!importStack.insert(simplifiedFilename).second)
        return PropsImportResult::Cycle;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        importStack.erase(simplifiedFilename);
        return PropsImportResult::NotFound;
    }
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr) {
        importStack.erase(simplifiedFilename);
        return PropsImportResult::NotFound;
    }

    const std::string previousThisFileDir = variables["MSBuildThisFileDirectory"];
    variables["MSBuildThisFileDirectory"] = Path::simplifyPath(Path::getPathFromFilename(filename));

    PropsImportResult ret = PropsImportResult::Ok;
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); ret != PropsImportResult::Cycle && node; node = node->NextSiblingElement()) {
        if (hasNameAndLabel(node, "ImportGroup", "PropertySheets", variables)) {
            for (const tinyxml2::XMLElement *importGroup = node->FirstChildElement(); ret != PropsImportResult::Cycle && importGroup; importGroup = importGroup->NextSiblingElement()) {
                if (hasName(importGroup, "Import", variables)) {
                    const char *projectAttribute = importGroup->Attribute("Project");
                    if (projectAttribute == nullptr)
                        continue;
                    std::string loadprj(projectAttribute);
                    if (loadprj.find('$') == std::string::npos) {
                        loadprj = Path::getPathFromFilename(filename) + loadprj;
                    }
                    const PropsImportResult nestedRet = loadVisualStudioProperties(loadprj, variables, importStack);
                    if (nestedRet == PropsImportResult::Cycle)
                        ret = PropsImportResult::Cycle;
                }
            }
        } else if (hasName(node, "PropertyGroup", variables)) {
            for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement())
                addProperty(e, variables);
        } else if (hasName(node, "ItemDefinitionGroup", variables)) {
            for (const tinyxml2::XMLElement *e1 = node->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                if (hasName(e1, "ClCompile", variables)) {
                    for (const tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
                        addProperty(e2, variables);
                    }
                } else if (hasName(e1, "Link", variables)) {
                    for (const tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
                        addProperty(e2, variables); // fixme
                    }
                }
            }
        }
    }

    variables["MSBuildThisFileDirectory"] = previousThisFileDir;
    importStack.erase(simplifiedFilename);
    return ret;
}

// returns false only for a genuine import cycle; a props file that couldn't be found/parsed
// is silently ignored (see PropsImportResult::NotFound above) and this still returns true
static bool loadVisualStudioProperties(const std::string &props,
                                       VariablesMap &variables)
{
    std::unordered_set<std::string> importStack;
    return loadVisualStudioProperties(props, variables, importStack) != PropsImportResult::Cycle;
}

static bool importVcxitems(const std::string &filename,
                           VariablesMap &variables,
                           std::list<ItemGroupClCompile> &compileList)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
    if (error != tinyxml2::XML_SUCCESS)
        return false;

    const tinyxml2::XMLElement *const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr)
        return false;

    const std::string itemsDir = Path::simplifyPath(Path::getPathFromFilename(filename));
    const std::string previousThisFileDir = variables["MSBuildThisFileDirectory"];
    variables["MSBuildThisFileDirectory"] = itemsDir;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (hasName(node, "ItemGroup", variables)) {
            for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                if (hasName(e, "ClCompile", variables)) {
                    const char *include = e->Attribute("Include");
                    if (!include || !Path::acceptFile(include))
                        continue;
                    std::string toInclude = toAbsolute(include, itemsDir, variables);

                    ItemGroupClCompile compile(toInclude);
                    // a file with no override of its own inherits the ItemDefinitionGroup value outright
                    compile.additionalIncludeDirectories = variables["AdditionalIncludeDirectories"];
                    compile.forcedIncludeFiles = variables["ForcedIncludeFiles"];
                    compile.preprocessorDefinitions = variables["PreprocessorDefinitions"];
                    compile.languageStandard = variables["LanguageStandard"];
                    bool excludedFromBuild = false;

                    for (const tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                        const char *text = e1->GetText();
                        if (!text)
                            continue;

                        if (hasName(e1, "ExcludedFromBuild", variables)) {
                            if (std::strcmp(text, "true") == 0) {
                                excludedFromBuild = true;
                                break;
                            }
                        } else if (hasName(e1, "AdditionalIncludeDirectories", variables)) {
                            compile.additionalIncludeDirectories = getProperty(e1, variables, compile.additionalIncludeDirectories);
                        } else if (hasName(e1, "ForcedIncludeFiles", variables)) {
                            compile.forcedIncludeFiles = getProperty(e1, variables, compile.forcedIncludeFiles);
                        } else if (hasName(e1, "PreprocessorDefinitions", variables)) {
                            compile.preprocessorDefinitions = getProperty(e1, variables, compile.preprocessorDefinitions);
                        } else if (hasName(e1, "LanguageStandard", variables)) {
                            compile.languageStandard = getProperty(e1, variables, compile.languageStandard);
                        }
                    }

                    if (!excludedFromBuild)
                        compileList.emplace_back(std::move(compile));
                }
            }
        } else if (hasName(node, "ItemDefinitionGroup", variables)) {
            for (const tinyxml2::XMLElement *e1 = node->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                if (hasName(e1, "ClCompile", variables)) {
                    for (const tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
                        addProperty(e2, variables);
                    }
                }
            }
        }
    }

    variables["MSBuildThisFileDirectory"] = previousThisFileDir;
    return true;
}

bool ImportProject::importVcxproj(const std::string &filename,
                                  VariablesMap &variables,
                                  const std::vector<std::string> &fileFilters)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        errors.emplace_back(std::string("Visual Studio project file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }

    const auto solutionDirIt = variables.find("SolutionDir");
    const bool hasSolutionDir = solutionDirIt != variables.end();
    const std::string solutionDir = hasSolutionDir ? solutionDirIt->second : std::string();
    variables.clear();
    if (hasSolutionDir)
        variables["SolutionDir"] = solutionDir;

    std::string projectDir = Path::simplifyPath(Path::getPathFromFilename(filename));

    variables["ProjectDir"] = projectDir;
    variables.emplace("SolutionDir", projectDir);
    variables["MSBuildThisFileDirectory"] = projectDir;

    VariablesMap originalVariables = variables;

    std::list<ProjectConfiguration> projectConfigurationList;
    std::list<ItemGroupClCompile> compileList;

    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr) {
        errors.emplace_back("Visual Studio project file has no XML root node");
        return false;
    }

    // find all Visual Studio project configurations
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (hasNameAndLabel(node, "ItemGroup", "ProjectConfigurations", variables)) {
            for (const tinyxml2::XMLElement *pcNode = node->FirstChildElement("ProjectConfiguration"); pcNode; pcNode = pcNode->NextSiblingElement("ProjectConfiguration")) {
                const ProjectConfiguration pc(pcNode);
                if (pc.platform != ProjectConfiguration::Unknown) {
                    projectConfigurationList.emplace_back(pc);
                    mAllVSConfigs.insert(pc.configuration);
                }
            }
        }
    }

    bool first = true;

    for (const ProjectConfiguration &pc : projectConfigurationList) {
        if (!first) {
            compileList.clear();
            variables = originalVariables;
        } else
            first = false;

        variables["Configuration"] = pc.configuration;
        variables["Platform"] = pc.platformStr;

        for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
            if (hasNameAndNotLabel(node, "ItemGroup", "ProjectConfigurations", variables)) {
                for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                    if (hasName(e, "ClCompile", variables)) {
                        const char *include = e->Attribute("Include");
                        if (include && Path::acceptFile(include)) {
                            std::string toInclude = toAbsolute(include, projectDir, variables);
                            ItemGroupClCompile compile(toInclude);
                            // a file with no override of its own inherits the ItemDefinitionGroup value outright
                            compile.additionalIncludeDirectories = variables["AdditionalIncludeDirectories"];
                            compile.forcedIncludeFiles = variables["ForcedIncludeFiles"];
                            compile.preprocessorDefinitions = variables["PreprocessorDefinitions"];
                            compile.languageStandard = variables["LanguageStandard"];
                            bool excludedFromBuild = false;

                            for (const tinyxml2::XMLElement *e1 = e->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                                const char *text = e1->GetText();
                                if (!text)
                                    continue;

                                if (hasName(e1, "ExcludedFromBuild", variables)) {
                                    if (std::strcmp(text, "true") == 0) {
                                        excludedFromBuild = true;
                                        break;
                                    }
                                } else if (hasName(e1, "AdditionalIncludeDirectories", variables)) {
                                    compile.additionalIncludeDirectories = getProperty(e1, variables, compile.additionalIncludeDirectories);
                                } else if (hasName(e1, "ForcedIncludeFiles", variables)) {
                                    compile.forcedIncludeFiles = getProperty(e1, variables, compile.forcedIncludeFiles);
                                } else if (hasName(e1, "PreprocessorDefinitions", variables)) {
                                    compile.preprocessorDefinitions = getProperty(e1, variables, compile.preprocessorDefinitions);
                                } else if (hasName(e1, "LanguageStandard", variables)) {
                                    compile.languageStandard = getProperty(e1, variables, compile.languageStandard);
                                }
                            }

                            if (!excludedFromBuild)
                                compileList.emplace_back(compile);
                        }
                    }
                }
            } else if (hasName(node, "ItemDefinitionGroup", variables)) {
                for (const tinyxml2::XMLElement *e1 = node->FirstChildElement(); e1; e1 = e1->NextSiblingElement()) {
                    if (hasName(e1, "ClCompile", variables)) {
                        for (const tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
                            addProperty(e2, variables);
                        }
                    } else if (hasName(e1, "Link", variables)) {
                        for (const tinyxml2::XMLElement *e2 = e1->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
                            addProperty(e2, variables); // fixme
                        }
                    }
                }
            } else if (hasName(node, "PropertyGroup", variables)) {
                for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement())
                    addProperty(e, variables);
            } else if (hasName(node, "ImportGroup", variables)) {
                const char *labelAttribute = node->Attribute("Label");
                if (labelAttribute && std::strcmp(labelAttribute, "PropertySheets") == 0) {
                    for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                        if (hasName(e, "Import", variables)) {
                            const char *projectAttribute = e->Attribute("Project");
                            if (projectAttribute && !loadVisualStudioProperties(projectAttribute, variables)) {
                                errors.emplace_back("Could not load property sheet \"" + std::string(projectAttribute) + "\" - it may be missing, invalid, or part of a circular import");
                                return false;
                            }
                        }
                    }
                } else if (labelAttribute && std::strcmp(labelAttribute, "Shared") == 0) {
                    for (const tinyxml2::XMLElement *e = node->FirstChildElement(); e; e = e->NextSiblingElement()) {
                        if (hasName(e, "Import", variables)) {
                            const char *projectAttribute = e->Attribute("Project");
                            if (projectAttribute) {
                                // Path to shared items project is relative to current project directory,
                                // unless the string starts with $(SolutionDir)
                                std::string pathToSharedItemsFile = projectAttribute;
                                if (!simplifyPathWithVariables(pathToSharedItemsFile, variables)) {
                                    errors.emplace_back("Could not simplify path to referenced shared items project");
                                    return false;
                                }

                                if (!importVcxitems(pathToSharedItemsFile, variables, compileList)) {
                                    errors.emplace_back("Could not load shared items project \"" + pathToSharedItemsFile + "\" from original path \"" + std::string(projectAttribute) + "\".");
                                    return false;
                                }
                            }
                        }
                    }
                }
            } else if (hasName(node, "Import", variables)) {
                const char *projectAttribute = node->Attribute("Project");
                if (projectAttribute && !loadVisualStudioProperties(projectAttribute, variables)) {
                    errors.emplace_back("Could not load property sheet \"" + std::string(projectAttribute) + "\" - it may be missing, invalid, or part of a circular import");
                    return false;
                }
            }
        }

        // # TODO: support signedness of char via /J (and potential XML option for it)?
        // we can only set it globally but in this context it needs to be treated per file

        // Project files
        PathMatch filtermatcher(fileFilters, Path::getCurrentPath());
        for (const ItemGroupClCompile &compile : compileList) {
            if (!fileFilters.empty() && !filtermatcher.match(compile.filename))
                continue;

            if (!guiProject.checkVsConfigs.empty()) {
                const bool doChecking = std::any_of(guiProject.checkVsConfigs.cbegin(), guiProject.checkVsConfigs.cend(), [&](const std::string &c) {
                    return c == pc.configuration;
                });
                if (!doChecking)
                    continue;
            }

            FileSettings fs{ compile.filename, Standards::Language::None, 0 }; // file will be identified later on
            fs.cfg = pc.name;
            // TODO: detect actual MSC version
            fs.msc = true;
            fs.defines = "_WIN32=1";
            if (pc.platform == ProjectConfiguration::Win32)
                fs.platformType = Platform::Type::Win32W;
            else if (pc.platform == ProjectConfiguration::x64) {
                fs.platformType = Platform::Type::Win64;
                fs.defines += ";_WIN64=1";
            }

            Standards::cppstd_t cppstd = Standards::CPPLatest;
            const std::string &languageStandard = compile.languageStandard;
            if (languageStandard == "stdcpp14")
                cppstd = Standards::CPP14;
            else if (languageStandard == "stdcpp17")
                cppstd = Standards::CPP17;
            else if (languageStandard == "stdcpp20")
                cppstd = Standards::CPP20;
            else if (languageStandard == "stdcpp23")
                cppstd = Standards::CPP23;
            else if (languageStandard == "stdcpplatest")
                cppstd = Standards::CPPLatest;
            fs.standard = Standards::getCPP(cppstd);

            std::string enableEnhancedInstructionSet = variables["EnableEnhancedInstructionSet"];
            if (enableEnhancedInstructionSet == "StreamingSIMDExtensions")
                fs.defines += ";__SSE__";
            else if (enableEnhancedInstructionSet == "StreamingSIMDExtensions2")
                fs.defines += ";__SSE2__";
            else if (enableEnhancedInstructionSet == "AdvancedVectorExtensions")
                fs.defines += ";__AVX__";
            else if (enableEnhancedInstructionSet == "AdvancedVectorExtensions2")
                fs.defines += ";__AVX2__";
            else if (enableEnhancedInstructionSet == "AdvancedVectorExtensions512")
                fs.defines += ";__AVX512__";

            auto it = variables.find("CharacterSet");
            bool useUnicode = it != variables.end() && it->second == "Unicode";

            const auto useOfMfcIt = variables.find("UseOfMfc");
            fs.useMfc = useOfMfcIt != variables.end() && !useOfMfcIt->second.empty() &&
                        caseInsensitiveStringCompare(useOfMfcIt->second, "false") != 0;

            if (useUnicode) {
                fs.defines += ";UNICODE=1;_UNICODE=1";
            }

            std::string defines = fs.defines;
            defines += (";" + compile.preprocessorDefinitions);
            fsSetDefines(fs, defines);
            fsSetIncludePaths(fs, projectDir, toStringList(variables["IncludePath"]), variables);
            fs.systemIncludePaths = std::move(fs.includePaths);
            fsSetIncludePaths(fs, projectDir, toStringList(compile.additionalIncludeDirectories), variables);
            fs.forcedIncludes = toStringList(compile.forcedIncludeFiles);
            for (auto &forcedInclude : fs.forcedIncludes)
                forcedInclude = toAbsolute(forcedInclude, projectDir, variables);

            fileSettings.push_back(std::move(fs));
        }
    }

    return true;
}

bool ImportProject::importBcb6Prj(const std::string &projectFilename)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(projectFilename.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        errors.emplace_back(std::string("Borland project file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr) {
        errors.emplace_back("Borland project file has no XML root node");
        return false;
    }

    const std::string& projectDir = Path::simplifyPath(Path::getPathFromFilename(projectFilename));

    std::list<std::string> compileList;
    std::string includePath;
    std::string userdefines;
    std::string sysdefines;
    std::string cflag1;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const char* name = node->Name();
        if (std::strcmp(name, "FILELIST") == 0) {
            for (const tinyxml2::XMLElement *f = node->FirstChildElement(); f; f = f->NextSiblingElement()) {
                if (std::strcmp(f->Name(), "FILE") == 0) {
                    const char *filename = f->Attribute("FILENAME");
                    if (filename && Path::acceptFile(filename))
                        compileList.emplace_back(filename);
                }
            }
        } else if (std::strcmp(name, "MACROS") == 0) {
            for (const tinyxml2::XMLElement *m = node->FirstChildElement(); m; m = m->NextSiblingElement()) {
                const char* mname = m->Name();
                if (std::strcmp(mname, "INCLUDEPATH") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        includePath = v;
                } else if (std::strcmp(mname, "USERDEFINES") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        userdefines = v;
                } else if (std::strcmp(mname, "SYSDEFINES") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        sysdefines = v;
                }
            }
        } else if (std::strcmp(name, "OPTIONS") == 0) {
            for (const tinyxml2::XMLElement *m = node->FirstChildElement(); m; m = m->NextSiblingElement()) {
                if (std::strcmp(m->Name(), "CFLAG1") == 0) {
                    const char *v = m->Attribute("value");
                    if (v)
                        cflag1 = v;
                }
            }
        }
    }

    std::set<std::string> cflags;

    // parse cflag1 and fill the cflags set
    {
        std::string arg;

        for (const char i : cflag1) {
            if (i == ' ' && !arg.empty()) {
                cflags.insert(arg);
                arg.clear();
                continue;
            }
            arg += i;
        }

        if (!arg.empty()) {
            cflags.insert(std::move(arg));
        }

        // cleanup: -t is "An alternate name for the -Wxxx switches; there is no difference"
        // -> Remove every known -txxx argument and replace it with its -Wxxx counterpart.
        //    This way, we know what we have to check for later on.
        static const std::map<std::string, std::string> synonyms = {
            { "-tC","-WC" },
            { "-tCDR","-WCDR" },
            { "-tCDV","-WCDV" },
            { "-tW","-W" },
            { "-tWC","-WC" },
            { "-tWCDR","-WCDR" },
            { "-tWCDV","-WCDV" },
            { "-tWD","-WD" },
            { "-tWDR","-WDR" },
            { "-tWDV","-WDV" },
            { "-tWM","-WM" },
            { "-tWP","-WP" },
            { "-tWR","-WR" },
            { "-tWU","-WU" },
            { "-tWV","-WV" }
        };

        for (auto i = synonyms.cbegin(); i != synonyms.cend(); ++i) {
            if (cflags.erase(i->first) > 0) {
                cflags.insert(i->second);
            }
        }
    }

    std::string predefines;
    std::string cppPredefines;

    // Collecting predefines. See BCB6 help topic "Predefined macros"
    {
        cppPredefines +=
            // Defined if you've selected C++ compilation; will increase in later releases.
            // value 0x0560 (but 0x0564 for our BCB6 SP4)
            // @see http://docwiki.embarcadero.com/RADStudio/Tokyo/en/Predefined_Macros#C.2B.2B_Compiler_Versions_in_Predefined_Macros
            ";__BCPLUSPLUS__=0x0560"

            // Defined if in C++ mode; otherwise, undefined.
            ";__cplusplus=1"

            // Defined as 1 for C++ files(meaning that templates are supported); otherwise, it is undefined.
            ";__TEMPLATES__=1"

            // Defined only for C++ programs to indicate that wchar_t is an intrinsically defined data type.
            ";_WCHAR_T"

            // Defined only for C++ programs to indicate that wchar_t is an intrinsically defined data type.
            ";_WCHAR_T_DEFINED"

            // Defined in any compiler that has an optimizer.
            ";__BCOPT__=1"

            // Version number.
            // BCB6 is 0x056X (SP4 is 0x0564)
            // @see http://docwiki.embarcadero.com/RADStudio/Tokyo/en/Predefined_Macros#C.2B.2B_Compiler_Versions_in_Predefined_Macros
            ";__BORLANDC__=0x0560"
            ";__TCPLUSPLUS__=0x0560"
            ";__TURBOC__=0x0560";

        // Defined if Calling Convention is set to cdecl; otherwise undefined.
        const bool useCdecl = (cflags.find("-p") == cflags.end()
                               && cflags.find("-pm") == cflags.end()
                               && cflags.find("-pr") == cflags.end()
                               && cflags.find("-ps") == cflags.end());
        if (useCdecl)
            predefines += ";__CDECL=1";

        // Defined by default indicating that the default char is unsigned char. Use the -K compiler option to undefine this macro.
        const bool treatCharAsUnsignedChar = (cflags.find("-K") != cflags.end());
        if (treatCharAsUnsignedChar)
            predefines += ";_CHAR_UNSIGNED=1";

        // Defined whenever one of the CodeGuard compiler options is used; otherwise it is undefined.
        const bool codeguardUsed = (cflags.find("-vGd") != cflags.end()
                                    || cflags.find("-vGt") != cflags.end()
                                    || cflags.find("-vGc") != cflags.end());
        if (codeguardUsed)
            predefines += ";__CODEGUARD__";

        // When defined, the macro indicates that the program is a console application.
        const bool isConsoleApp = (cflags.find("-WC") != cflags.end());
        if (isConsoleApp)
            predefines += ";__CONSOLE__=1";

        // Enable stack unwinding. This is true by default; use -xd- to disable.
        const bool enableStackUnwinding = (cflags.find("-xd-") == cflags.end());
        if (enableStackUnwinding)
            predefines += ";_CPPUNWIND=1";

        // Defined whenever the -WD compiler option is used; otherwise it is undefined.
        const bool isDLL = (cflags.find("-WD") != cflags.end());
        if (isDLL)
            predefines += ";__DLL__=1";

        // Defined when compiling in 32-bit flat memory model.
        // TODO: not sure how to switch to another memory model or how to read configuration from project file
        predefines += ";__FLAT__=1";

        // Always defined. The default value is 300. You can change the value to 400 or 500 by using the /4 or /5 compiler options.
        if (cflags.find("-6") != cflags.end())
            predefines += ";_M_IX86=600";
        else if (cflags.find("-5") != cflags.end())
            predefines += ";_M_IX86=500";
        else if (cflags.find("-4") != cflags.end())
            predefines += ";_M_IX86=400";
        else
            predefines += ";_M_IX86=300";

        // Defined only if the -WM option is used. It specifies that the multithread library is to be linked.
        const bool linkMtLib = (cflags.find("-WM") != cflags.end());
        if (linkMtLib)
            predefines += ";__MT__=1";

        // Defined if Calling Convention is set to Pascal; otherwise undefined.
        const bool usePascalCallingConvention = (cflags.find("-p") != cflags.end());
        if (usePascalCallingConvention)
            predefines += ";__PASCAL__=1";

        // Defined if you compile with the -A compiler option; otherwise, it is undefined.
        const bool useAnsiKeywordExtensions = (cflags.find("-A") != cflags.end());
        if (useAnsiKeywordExtensions)
            predefines += ";__STDC__=1";

        // Thread Local Storage. Always true in C++Builder.
        predefines += ";__TLC__=1";

        // Defined for Windows-only code.
        const bool isWindowsTarget = (cflags.find("-WC") != cflags.end()
                                      || cflags.find("-WCDR") != cflags.end()
                                      || cflags.find("-WCDV") != cflags.end()
                                      || cflags.find("-WD") != cflags.end()
                                      || cflags.find("-WDR") != cflags.end()
                                      || cflags.find("-WDV") != cflags.end()
                                      || cflags.find("-WM") != cflags.end()
                                      || cflags.find("-WP") != cflags.end()
                                      || cflags.find("-WR") != cflags.end()
                                      || cflags.find("-WU") != cflags.end()
                                      || cflags.find("-WV") != cflags.end());
        if (isWindowsTarget)
            predefines += ";_Windows";

        // Defined for console and GUI applications.
        // TODO: I'm not sure about the difference to define "_Windows".
        //       From description, I would assume __WIN32__ is only defined for
        //       executables, while _Windows would also be defined for DLLs, etc.
        //       However, in a newly created DLL project, both __WIN32__ and
        //       _Windows are defined. -> treating them the same for now.
        //       Also boost uses __WIN32__ for OS identification.
        const bool isConsoleOrGuiApp = isWindowsTarget;
        if (isConsoleOrGuiApp)
            predefines += ";__WIN32__=1";
    }

    // Include paths may contain variables like "$(BCB)\include" or "$(BCB)\include\vcl".
    // Those get resolved by ImportProject::FileSettings::setIncludePaths by
    // 1. checking the provided variables map ("BCB" => "C:\\Program Files (x86)\\Borland\\CBuilder6")
    // 2. checking env variables as a fallback
    // Setting env is always possible. Configuring the variables via cli might be an addition.
    // Reading the BCB6 install location from registry in windows environments would also be possible,
    // but I didn't see any such functionality around the source. Not in favor of adding it only
    // for the BCB6 project loading.
    VariablesMap variables;
    const std::string defines = predefines + ";" + sysdefines + ";" + userdefines;
    const std::string cppDefines  = cppPredefines + ";" + defines;
    const bool forceCppMode = (cflags.find("-P") != cflags.end());

    for (const std::string &c : compileList) {
        // C++ compilation is selected by file extension by default, so these
        // defines have to be configured on a per-file base.
        //
        // > Files with the .CPP extension compile as C++ files. Files with a .C
        // > extension, with no extension, or with extensions other than .CPP,
        // > .OBJ, .LIB, or .ASM compile as C files.
        // (http://docwiki.embarcadero.com/RADStudio/Tokyo/en/BCC32.EXE,_the_C%2B%2B_32-bit_Command-Line_Compiler)
        //
        // We can also force C++ compilation for all files using the -P command line switch.
        const bool cppMode = forceCppMode || Path::getFilenameExtensionInLowerCase(c) == ".cpp";
        // TODO: needs to set language and ignore later identification and language enforcement
        FileSettings fs{Path::simplifyPath(Path::isAbsolute(c) ? c : projectDir + c), Standards::Language::None, 0}; // file will be identified later on
        fsSetIncludePaths(fs, projectDir, toStringList(includePath), variables);
        fsSetDefines(fs, cppMode ? cppDefines : defines);
        fileSettings.push_back(std::move(fs));
    }

    return true;
}

static std::string joinRelativePath(const std::string &path1, const std::string &path2)
{
    if (!path1.empty() && !Path::isAbsolute(path2))
        return path1 + path2;
    return path2;
}

static std::list<std::string> readXmlStringList(const tinyxml2::XMLElement *node, const std::string &path, const char name[], const char attribute[])
{
    std::list<std::string> ret;
    for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
        if (strcmp(child->Name(), name) != 0)
            continue;
        const char *attr = attribute ? child->Attribute(attribute) : child->GetText();
        if (attr)
            ret.emplace_back(joinRelativePath(path, attr));
    }
    return ret;
}

static std::list<std::string> readXmlPathMatchList(const tinyxml2::XMLElement *node, const std::string &path, const char name[], const char attribute[])
{
    std::list<std::string> ret;
    for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
        if (strcmp(child->Name(), name) != 0)
            continue;
        const char *attr = attribute ? child->Attribute(attribute) : child->GetText();
        if (attr)
            ret.emplace_back(PathMatch::joinRelativePattern(path, attr));
    }
    return ret;
}

static std::string join(const std::list<std::string> &strlist, const char *sep)
{
    std::string ret;
    for (const std::string &s : strlist) {
        ret += (ret.empty() ? "" : sep) + s;
    }
    return ret;
}

static std::string istream_to_string(std::istream &istr)
{
    std::istreambuf_iterator<char> eos;
    return std::string(std::istreambuf_iterator<char>(istr), eos);
}

bool ImportProject::importCppcheckGuiProject(std::istream &istr, Settings &settings, Suppressions &supprs)
{
    tinyxml2::XMLDocument doc;
    const std::string xmldata = istream_to_string(istr);
    const tinyxml2::XMLError error = doc.Parse(xmldata.data(), xmldata.size());
    if (error != tinyxml2::XML_SUCCESS) {
        errors.emplace_back(std::string("Cppcheck GUI project file is not a valid XML - ") + tinyxml2::XMLDocument::ErrorIDToName(error));
        return false;
    }
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (rootnode == nullptr || strcmp(rootnode->Name(), CppcheckXml::ProjectElementName) != 0) {
        errors.emplace_back("Cppcheck GUI project file has no XML root node");
        return false;
    }

    const std::string &path = mPath;

    std::list<std::string> paths;
    std::list<SuppressionList::Suppression> suppressions;
    Settings temp;

    // default to --check-level=normal for import for now
    temp.setCheckLevel(Settings::CheckLevel::normal);

    // TODO: this should support all available command-line options
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const char* name = node->Name();
        if (strcmp(name, CppcheckXml::RootPathName) == 0) {
            const char* attr = node->Attribute(CppcheckXml::RootPathNameAttrib);
            if (attr) {
                temp.basePaths.push_back(Path::fromNativeSeparators(joinRelativePath(path, attr)));
                temp.relativePaths = true;
            }
        } else if (strcmp(name, CppcheckXml::BuildDirElementName) == 0)
            temp.buildDir = joinRelativePath(path, empty_if_null(node->GetText()));
        else if (strcmp(name, CppcheckXml::IncludeDirElementName) == 0)
            temp.includePaths = readXmlStringList(node, path, CppcheckXml::DirElementName, CppcheckXml::DirNameAttrib); // TODO: append instead of overwrite
        else if (strcmp(name, CppcheckXml::DefinesElementName) == 0)
            temp.userDefines = join(readXmlStringList(node, "", CppcheckXml::DefineName, CppcheckXml::DefineNameAttrib), ";"); // TODO: append instead of overwrite
        else if (strcmp(name, CppcheckXml::UndefinesElementName) == 0) {
            for (const std::string &u : readXmlStringList(node, "", CppcheckXml::UndefineName, nullptr))
                temp.userUndefs.insert(u);
        } else if (strcmp(name, CppcheckXml::UserIncludeElementName) == 0) {
            const char* i = node->GetText();
            if (i)
                temp.userIncludes.emplace_back(i);
        } else if (strcmp(name, CppcheckXml::ImportProjectElementName) == 0) {
            const std::string t_str = empty_if_null(node->GetText());
            if (!t_str.empty())
                guiProject.projectFile = path + t_str;
        }
        else if (strcmp(name, CppcheckXml::PathsElementName) == 0)
            paths = readXmlStringList(node, path, CppcheckXml::PathName, CppcheckXml::PathNameAttrib);
        else if (strcmp(name, CppcheckXml::ExcludeElementName) == 0)
            guiProject.excludedPaths = readXmlPathMatchList(node, path, CppcheckXml::ExcludePathName, CppcheckXml::ExcludePathNameAttrib); // TODO: append instead of overwrite
        else if (strcmp(name, CppcheckXml::FunctionContracts) == 0)
            ;
        else if (strcmp(name, CppcheckXml::VariableContractsElementName) == 0)
            ;
        else if (strcmp(name, CppcheckXml::IgnoreElementName) == 0)
            guiProject.excludedPaths = readXmlPathMatchList(node, path, CppcheckXml::IgnorePathName, CppcheckXml::IgnorePathNameAttrib); // TODO: append instead of overwrite
        else if (strcmp(name, CppcheckXml::LibrariesElementName) == 0)
            guiProject.libraries = readXmlStringList(node, "", CppcheckXml::LibraryElementName, nullptr); // TODO: append instead of overwrite
        else if (strcmp(name, CppcheckXml::SuppressionsElementName) == 0) {
            for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                if (strcmp(child->Name(), CppcheckXml::SuppressionElementName) != 0)
                    continue;
                SuppressionList::Suppression s;
                s.errorId = empty_if_null(child->GetText());
                s.fileName = empty_if_null(child->Attribute("fileName"));
                if (!s.fileName.empty())
                    s.fileName = joinRelativePath(path, s.fileName);
                s.lineNumber = child->IntAttribute("lineNumber", SuppressionList::Suppression::NO_LINE); // TODO: should not depend on Suppression
                s.symbolName = empty_if_null(child->Attribute("symbolName"));
                s.hash = strToInt<std::size_t>(default_if_null(child->Attribute("hash"), "0"));
                suppressions.push_back(std::move(s));
            }
        } else if (strcmp(name, CppcheckXml::VSConfigurationElementName) == 0)
            guiProject.checkVsConfigs = readXmlStringList(node, "", CppcheckXml::VSConfigurationName, nullptr);
        else if (strcmp(name, CppcheckXml::PlatformElementName) == 0)
            guiProject.platform = empty_if_null(node->GetText());
        else if (strcmp(name, CppcheckXml::AnalyzeAllVsConfigsElementName) == 0)
            temp.analyzeAllVsConfigs = std::string(empty_if_null(node->GetText())) != "false";
        else if (strcmp(name, CppcheckXml::Parser) == 0)
            temp.clang = true;
        else if (strcmp(name, CppcheckXml::AddonsElementName) == 0) {
            const auto& addons = readXmlStringList(node, "", CppcheckXml::AddonElementName, nullptr);
            temp.addons.insert(addons.cbegin(), addons.cend());
            if (settings.premium) {
                auto it = temp.addons.find("misra");
                if (it != temp.addons.end()) {
                    temp.addons.erase(it);
                    temp.premiumArgs += " --misra-c-2012";
                }
            }
        }
        else if (strcmp(name, CppcheckXml::TagsElementName) == 0)
            node->Attribute(CppcheckXml::TagElementName); // FIXME: Write some warning
        else if (strcmp(name, CppcheckXml::ToolsElementName) == 0) {
            const std::list<std::string> toolList = readXmlStringList(node, "", CppcheckXml::ToolElementName, nullptr);
            for (const std::string &toolName : toolList) {
                if (toolName == CppcheckXml::ClangTidy)
                    temp.clangTidy = true;
            }
        } else if (strcmp(name, CppcheckXml::CheckHeadersElementName) == 0)
            temp.checkHeaders = (strcmp(default_if_null(node->GetText(), ""), "true") == 0);
        else if (strcmp(name, CppcheckXml::CheckLevelReducedElementName) == 0)
            temp.setCheckLevel(Settings::CheckLevel::reduced);
        else if (strcmp(name, CppcheckXml::CheckLevelNormalElementName) == 0)
            temp.setCheckLevel(Settings::CheckLevel::normal);
        else if (strcmp(name, CppcheckXml::CheckLevelExhaustiveElementName) == 0)
            temp.setCheckLevel(Settings::CheckLevel::exhaustive);
        else if (strcmp(name, CppcheckXml::CheckUnusedTemplatesElementName) == 0)
            temp.checkUnusedTemplates = (strcmp(default_if_null(node->GetText(), ""), "true") == 0);
        else if (strcmp(name, CppcheckXml::InlineSuppression) == 0)
            temp.inlineSuppressions = (strcmp(default_if_null(node->GetText(), ""), "true") == 0);
        else if (strcmp(name, CppcheckXml::MaxCtuDepthElementName) == 0)
            temp.maxCtuDepth = strToInt<int>(default_if_null(node->GetText(), "2")); // TODO: bail out when missing?
        else if (strcmp(name, CppcheckXml::MaxTemplateRecursionElementName) == 0)
            temp.maxTemplateRecursion = strToInt<int>(default_if_null(node->GetText(), "100")); // TODO: bail out when missing?
        else if (strcmp(name, CppcheckXml::CheckUnknownFunctionReturn) == 0)
            ; // TODO
        else if (strcmp(name, Settings::SafeChecks::XmlRootName) == 0) {
            for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                const char* childname = child->Name();
                if (strcmp(childname, Settings::SafeChecks::XmlClasses) == 0)
                    temp.safeChecks.classes = true;
                else if (strcmp(childname, Settings::SafeChecks::XmlExternalFunctions) == 0)
                    temp.safeChecks.externalFunctions = true;
                else if (strcmp(childname, Settings::SafeChecks::XmlInternalFunctions) == 0)
                    temp.safeChecks.internalFunctions = true;
                else if (strcmp(childname, Settings::SafeChecks::XmlExternalVariables) == 0)
                    temp.safeChecks.externalVariables = true;
                else {
                    errors.emplace_back("Unknown '" + std::string(Settings::SafeChecks::XmlRootName) + "' element '" + childname + "' in Cppcheck GUI project file");
                    return false;
                }
            }
        } else if (strcmp(name, CppcheckXml::TagWarningsElementName) == 0)
            ; // TODO
        // Cppcheck Premium features
        else if (strcmp(name, CppcheckXml::BughuntingElementName) == 0)
            temp.premiumArgs += " --bughunting";
        else if (strcmp(name, CppcheckXml::CertIntPrecisionElementName) == 0)
            temp.premiumArgs += std::string(" --cert-c-int-precision=") + default_if_null(node->GetText(), "0");
        else if (strcmp(name, CppcheckXml::CodingStandardsElementName) == 0) {
            for (const tinyxml2::XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
                if (strcmp(child->Name(), CppcheckXml::CodingStandardElementName) == 0) {
                    const char* text = child->GetText();
                    if (text)
                        temp.premiumArgs += std::string(" --") + text;
                }
            }
        }
        else if (strcmp(name, CppcheckXml::ProjectNameElementName) == 0)
            ; // no-op
        else {
            errors.emplace_back("Unknown element '" + std::string(name) + "' in Cppcheck GUI project file");
            return false;
        }
    }
    settings.basePaths = temp.basePaths; // TODO: append instead of overwrite
    settings.relativePaths |= temp.relativePaths;
    settings.buildDir = temp.buildDir;
    settings.includePaths = temp.includePaths; // TODO: append instead of overwrite
    settings.userDefines = temp.userDefines; // TODO: append instead of overwrite
    settings.userUndefs = temp.userUndefs; // TODO: append instead of overwrite
    settings.userIncludes = temp.userIncludes; // TODO: append instead of overwrite
    for (const std::string &addon : temp.addons)
        settings.addons.emplace(addon);
    settings.clang = temp.clang;
    settings.clangTidy = temp.clangTidy;
    settings.analyzeAllVsConfigs = temp.analyzeAllVsConfigs;

    if (!settings.premiumArgs.empty())
        settings.premiumArgs += temp.premiumArgs;
    else if (!temp.premiumArgs.empty())
        settings.premiumArgs = temp.premiumArgs.substr(1);

    for (const std::string &p : paths)
        guiProject.pathNames.push_back(Path::fromNativeSeparators(p));

    bool ok = true;
    for (const auto &suppression : suppressions) {
        const std::string addError = supprs.nomsg.addSuppression(suppression);
        if (!addError.empty()) {
            errors.emplace_back(addError);
            ok = false;
        }
    }
    if (!ok)
        return false;

    settings.checkHeaders = temp.checkHeaders;
    settings.checkUnusedTemplates = temp.checkUnusedTemplates;
    settings.maxCtuDepth = temp.maxCtuDepth;
    settings.maxTemplateRecursion = temp.maxTemplateRecursion;
    settings.inlineSuppressions |= temp.inlineSuppressions;
    settings.safeChecks = temp.safeChecks;
    settings.setCheckLevel(temp.checkLevel);

    return true;
}

void ImportProject::selectOneVsConfig(Platform::Type platform)
{
    std::set<std::string> filenames;
    for (auto it = fileSettings.cbegin(); it != fileSettings.cend();) {
        if (it->cfg.empty()) {
            ++it;
            continue;
        }
        const FileSettings &fs = *it;
        bool remove = false;
        if (!startsWith(fs.cfg,"Debug"))
            remove = true;
        if (platform == Platform::Type::Win64 && fs.platformType != platform)
            remove = true;
        else if ((platform == Platform::Type::Win32A || platform == Platform::Type::Win32W) && fs.platformType == Platform::Type::Win64)
            remove = true;
        else if (filenames.find(fs.filename()) != filenames.end())
            remove = true;
        if (remove) {
            it = fileSettings.erase(it);
        } else {
            filenames.insert(fs.filename());
            ++it;
        }
    }
}

void ImportProject::selectVsConfigurations(Platform::Type platform, const std::vector<std::string> &configurations)
{
    for (auto it = fileSettings.cbegin(); it != fileSettings.cend();) {
        if (it->cfg.empty()) {
            ++it;
            continue;
        }
        const FileSettings &fs = *it;
        const auto config = fs.cfg.substr(0, fs.cfg.find('|'));
        bool remove = false;
        if (std::find(configurations.begin(), configurations.end(), config) == configurations.end())
            remove = true;
        if (platform == Platform::Type::Win64 && fs.platformType != platform)
            remove = true;
        else if ((platform == Platform::Type::Win32A || platform == Platform::Type::Win32W) && fs.platformType == Platform::Type::Win64)
            remove = true;
        if (remove) {
            it = fileSettings.erase(it);
        } else {
            ++it;
        }
    }
}

std::list<std::string> ImportProject::getVSConfigs()
{
    return std::list<std::string>(mAllVSConfigs.cbegin(), mAllVSConfigs.cend());
}

void ImportProject::setRelativePaths(const std::string &filename)
{
    if (Path::isAbsolute(filename))
        return;
    const std::vector<std::string> basePaths{Path::fromNativeSeparators(Path::getCurrentPath())};
    for (auto &fs: fileSettings) {
        fs.file.setPath(Path::getRelativePath(fs.filename(), basePaths));
        for (auto &includePath: fs.includePaths) {
            const std::string rel = Path::getRelativePath(includePath, basePaths);
            includePath = rel.empty() ? "." : rel;
        }
        for (auto &includePath: fs.systemIncludePaths) {
            const std::string rel = Path::getRelativePath(includePath, basePaths);
            includePath = rel.empty() ? "." : rel;
        }
        for (auto &forcedInclude: fs.forcedIncludes)
            forcedInclude = Path::getRelativePath(forcedInclude, basePaths);
    }
}

// only used by tests (testimportproject.cpp::testVcxprojConditions):
// cppcheck-suppress unusedFunction
bool cppcheck::testing::evaluateVcxprojCondition(const std::string& condition,
                                                 const std::string& configuration,
                                                 const std::string& platform)
{
    VariablesMap variables;
    variables["Platform"] = platform;
    variables["Configuration"] = configuration;
    return evalCondition(condition, variables);
}
