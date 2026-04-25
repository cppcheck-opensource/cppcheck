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


#include "preprocessor.h"

#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "utils.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <utility>

#include <simplecpp.h>

static bool sameline(const simplecpp::Token *tok1, const simplecpp::Token *tok2)
{
    return tok1 && tok2 && tok1->location.sameline(tok2->location);
}

Directive::Directive(const simplecpp::TokenList &tokens, const simplecpp::Location & _loc, std::string _str) :
    file(tokens.file(_loc)),
    linenr(_loc.line),
    str(std::move(_str))
{}

Directive::DirectiveToken::DirectiveToken(const simplecpp::Token & _tok) :
    line(_tok.location.line),
    column(_tok.location.col),
    tokStr(_tok.str())
{}

char Preprocessor::macroChar = char(1);

Preprocessor::Preprocessor(simplecpp::TokenList& tokens, const Settings& settings, ErrorLogger &errorLogger, Standards::Language lang)
    : mTokens(tokens)
    , mSettings(settings)
    , mErrorLogger(errorLogger)
    , mLang(lang)
{
    assert(mLang != Standards::Language::None);
}

namespace {
    struct BadInlineSuppression {
        BadInlineSuppression(const simplecpp::Location& loc, std::string msg) : location(loc), errmsg(std::move(msg)) {}
        simplecpp::Location location;
        std::string errmsg;
    };
}

static bool parseInlineSuppressionCommentToken(const simplecpp::Token *tok, std::list<SuppressionList::Suppression> &inlineSuppressions, std::list<BadInlineSuppression> &bad)
{
    const std::string cppchecksuppress("cppcheck-suppress");

    const std::string &comment = tok->str();
    if (comment.size() < cppchecksuppress.size())
        return false;
    const std::string::size_type pos1 = comment.find_first_not_of("/* \t");
    if (pos1 == std::string::npos)
        return false;
    if (comment.substr(pos1, cppchecksuppress.size()) != cppchecksuppress)
        return false;
    if (pos1 + cppchecksuppress.size() >= comment.size()) {
        bad.emplace_back(tok->location, "suppression without error ID");
        return false;
    }

    // check if it has a prefix
    const std::string::size_type posEndComment = comment.find_first_of(" [", pos1+cppchecksuppress.size());

    // skip spaces after "cppcheck-suppress" and its possible prefix
    const std::string::size_type pos2 = comment.find_first_not_of(' ', posEndComment);
    if (pos2 == std::string::npos) {
        bad.emplace_back(tok->location, "suppression without error ID");
        return false;
    }

    SuppressionList::Type errorType = SuppressionList::Type::unique;

    // determine prefix if specified
    if (posEndComment >= (pos1 + cppchecksuppress.size() + 1)) {
        const std::string suppressCmdString = comment.substr(pos1, pos2-pos1-1);
        if (comment.at(pos1 + cppchecksuppress.size()) != '-') {
            bad.emplace_back(tok->location, "unknown suppression type '" + suppressCmdString + "'");
            return false;
        }

        const unsigned int argumentLength =
            posEndComment - (pos1 + cppchecksuppress.size() + 1);

        const std::string suppressTypeString =
            comment.substr(pos1 + cppchecksuppress.size() + 1, argumentLength);

        if ("file" == suppressTypeString)
            errorType = SuppressionList::Type::file;
        else if ("begin" == suppressTypeString)
            errorType = SuppressionList::Type::blockBegin;
        else if ("end" == suppressTypeString)
            errorType = SuppressionList::Type::blockEnd;
        else if ("macro" == suppressTypeString)
            errorType = SuppressionList::Type::macro;
        else {
            bad.emplace_back(tok->location, "unknown suppression type '" + suppressCmdString + "'");
            return false;
        }
    }

    if (comment[pos2] == '[') {
        // multi suppress format
        std::string errmsg;
        std::vector<SuppressionList::Suppression> suppressions = SuppressionList::parseMultiSuppressComment(comment, &errmsg);

        for (SuppressionList::Suppression &s : suppressions) {
            s.isInline = true;
            s.type = errorType;
            s.lineNumber = tok->location.line;
            s.column = tok->location.col;
        }

        // TODO: return false?
        if (!errmsg.empty())
            bad.emplace_back(tok->location, std::move(errmsg));

        // TODO: report ones without ID - return false?
        std::copy_if(suppressions.cbegin(), suppressions.cend(), std::back_inserter(inlineSuppressions), [](const SuppressionList::Suppression& s) {
            return !s.errorId.empty();
        });
    } else {
        //single suppress format
        std::string errmsg;
        SuppressionList::Suppression s;
        if (!s.parseComment(comment, &errmsg))
            return false;

        s.isInline = true;
        s.type = errorType;
        s.lineNumber = tok->location.line;
        s.column = tok->location.col;

        // TODO: report when no ID - unreachable?
        if (!s.errorId.empty())
            inlineSuppressions.push_back(std::move(s));

        // TODO: unreachable?
        // TODO: return false?
        if (!errmsg.empty())
            bad.emplace_back(tok->location, std::move(errmsg));
    }

    return true;
}

static std::string getRelativeFilename(const simplecpp::TokenList &tokens, const simplecpp::Token* tok, const Settings &settings) {
    if (!tok)
        return "";
    std::string relativeFilename(tokens.file(tok->location));
    if (settings.relativePaths) {
        for (const std::string & basePath : settings.basePaths) {
            const std::string bp = basePath + "/";
            if (relativeFilename.compare(0,bp.size(),bp)==0) {
                relativeFilename = relativeFilename.substr(bp.size());
            }
        }
    }
    return Path::simplifyPath(std::move(relativeFilename));
}

static void addInlineSuppressions(const simplecpp::TokenList &tokens, const Settings &settings, SuppressionList &suppressions, std::list<BadInlineSuppression> &bad)
{
    std::list<SuppressionList::Suppression> inlineSuppressionsBlockBegin;

    bool onlyComments = true;

    polyspace::Parser polyspaceParser(settings);

    for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next) {
        if (!tok->comment) {
            onlyComments = false;
            continue;
        }

        std::list<SuppressionList::Suppression> inlineSuppressions;
        if (polyspace::isPolyspaceComment(tok->str())) {
            inlineSuppressions = polyspaceParser.parse(tok->str(), tok->location.line, getRelativeFilename(tokens, tok, settings));
        } else {
            if (!parseInlineSuppressionCommentToken(tok, inlineSuppressions, bad))
                continue;

            if (!sameline(tok->previous, tok)) {
                // find code after comment..
                if (tok->next) {
                    tok = tok->next;

                    while (tok->comment) {
                        parseInlineSuppressionCommentToken(tok, inlineSuppressions, bad);
                        if (tok->next) {
                            tok = tok->next;
                        } else {
                            break;
                        }
                    }
                }
            }
        }

        if (inlineSuppressions.empty())
            continue;

        // It should never happen
        if (!tok)
            continue;

        // Relative filename
        const std::string relativeFilename = getRelativeFilename(tokens, tok, settings);

        // Macro name
        std::string macroName;
        if (tok->str() == "#" && tok->next && tok->next->str() == "define") {
            const simplecpp::Token *macroNameTok = tok->next->next;
            if (sameline(tok, macroNameTok) && macroNameTok->name) {
                macroName = macroNameTok->str();
            }
        }

        // Add the suppressions.
        for (SuppressionList::Suppression &suppr : inlineSuppressions) {
            suppr.fileName = relativeFilename;
            suppr.fileIndex = tok->location.fileIndex;

            if (SuppressionList::Type::block == suppr.type) {
                suppressions.addSuppression(std::move(suppr));
            } else if (SuppressionList::Type::blockBegin == suppr.type) {
                inlineSuppressionsBlockBegin.push_back(std::move(suppr));
            } else if (SuppressionList::Type::blockEnd == suppr.type) {
                bool throwError = true;

                if (!inlineSuppressionsBlockBegin.empty()) {
                    const SuppressionList::Suppression lastBeginSuppression = inlineSuppressionsBlockBegin.back();

                    auto supprBegin = inlineSuppressionsBlockBegin.begin();
                    while (supprBegin != inlineSuppressionsBlockBegin.end())
                    {
                        if (lastBeginSuppression.lineNumber != supprBegin->lineNumber) {
                            ++supprBegin;
                            continue;
                        }

                        if (suppr.symbolName == supprBegin->symbolName && suppr.lineNumber > supprBegin->lineNumber) {
                            suppr.lineBegin = supprBegin->lineNumber;
                            suppr.lineEnd = suppr.lineNumber;
                            suppr.lineNumber = supprBegin->lineNumber;
                            suppr.column = supprBegin->column;
                            suppr.type = SuppressionList::Type::block;
                            inlineSuppressionsBlockBegin.erase(supprBegin);
                            suppressions.addSuppression(std::move(suppr)); // TODO: check result
                            throwError = false;
                            break;
                        }
                        ++supprBegin;
                    }
                }

                if (throwError) {
                    simplecpp::Location loc;
                    // NOLINTNEXTLINE(bugprone-use-after-move) - moved only when thrownError is false
                    loc.fileIndex = suppr.fileIndex;
                    loc.line = suppr.lineNumber;
                    loc.col = suppr.column;
                    bad.emplace_back(loc, "Suppress End: No matching begin");
                }
            } else if (SuppressionList::Type::unique == suppr.type || suppr.type == SuppressionList::Type::macro) {
                // special handling when suppressing { warnings for backwards compatibility
                const bool thisAndNextLine = tok->previous &&
                                             tok->previous->previous &&
                                             tok->next &&
                                             !sameline(tok->previous->previous, tok->previous) &&
                                             tok->location.line + 1 == tok->next->location.line &&
                                             tok->location.fileIndex == tok->next->location.fileIndex &&
                                             tok->previous->str() == "{";

                suppr.thisAndNextLine = thisAndNextLine;
                suppr.lineNumber = tok->location.line;
                suppr.column = tok->location.col;
                suppr.macroName = macroName;
                suppressions.addSuppression(std::move(suppr)); // TODO: check result
            } else if (SuppressionList::Type::file == suppr.type) {
                if (onlyComments)
                    suppressions.addSuppression(std::move(suppr)); // TODO: check result
                else {
                    simplecpp::Location loc;
                    loc.fileIndex = suppr.fileIndex;
                    loc.line = suppr.lineNumber;
                    loc.col = suppr.column;
                    bad.emplace_back(loc, "File suppression should be at the top of the file");
                }
            }
        }
    }

    for (const SuppressionList::Suppression & suppr: inlineSuppressionsBlockBegin) {
        simplecpp::Location loc;
        loc.fileIndex = suppr.fileIndex;
        loc.line = suppr.lineNumber;
        loc.col = suppr.column;
        bad.emplace_back(loc, "Suppress Begin: No matching end");
    }
}

void Preprocessor::inlineSuppressions(SuppressionList &suppressions)
{
    if (!mSettings.inlineSuppressions)
        return;
    std::list<BadInlineSuppression> err;
    ::addInlineSuppressions(mTokens, mSettings, suppressions, err);
    for (const auto &filedata : mFileCache) {
        ::addInlineSuppressions(filedata->tokens, mSettings, suppressions, err);
    }
    for (const BadInlineSuppression &bad : err) {
        invalidSuppression(bad.location, bad.errmsg);
    }
}

std::vector<RemarkComment> Preprocessor::getRemarkComments() const
{
    std::vector<RemarkComment> ret;
    addRemarkComments(mTokens, ret);
    for (const auto &filedata : mFileCache) {
        addRemarkComments(filedata->tokens, ret);
    }
    return ret;
}

std::list<Directive> Preprocessor::createDirectives() const
{
    // directive list..
    std::list<Directive> directives;

    std::vector<const simplecpp::TokenList *> list;
    list.reserve(1U + mFileCache.size());
    list.push_back(&mTokens);
    std::transform(mFileCache.cbegin(), mFileCache.cend(), std::back_inserter(list),
                   [](const std::unique_ptr<simplecpp::FileData> &filedata) {
        return &filedata->tokens;
    });

    for (const simplecpp::TokenList *tokenList : list) {
        for (const simplecpp::Token *tok = tokenList->cfront(); tok; tok = tok->next) {
            if ((tok->op != '#') || (tok->previous && tok->previous->location.line == tok->location.line))
                continue;
            if (tok->next && tok->next->str() == "endfile")
                continue;
            Directive directive(mTokens, tok->location, "");
            for (const simplecpp::Token *tok2 = tok; tok2 && tok2->location.line == directive.linenr; tok2 = tok2->next) {
                if (tok2->comment)
                    continue;
                if (!directive.str.empty() && (tok2->location.col > tok2->previous->location.col + tok2->previous->str().size()))
                    directive.str += ' ';
                if (directive.str == "#" && tok2->str() == "file")
                    directive.str += "include";
                else
                    directive.str += tok2->str();

                directive.strTokens.emplace_back(*tok2);
            }
            directives.push_back(std::move(directive));
        }
    }

    return directives;
}

static std::string readcondition(const simplecpp::Token *iftok, const std::set<std::string> &defined, const std::set<std::string> &undefined)
{
    const simplecpp::Token *cond = iftok->next;
    if (!sameline(iftok,cond))
        return "";

    const simplecpp::Token *next1 = cond->next;
    const simplecpp::Token *next2 = next1 ? next1->next : nullptr;
    const simplecpp::Token *next3 = next2 ? next2->next : nullptr;

    unsigned int len = 1;
    if (sameline(iftok,next1))
        len = 2;
    if (sameline(iftok,next2))
        len = 3;
    if (sameline(iftok,next3))
        len = 4;

    if (len == 1 && cond->str() == "0")
        return "0";

    if (len == 1 && cond->name) {
        if (defined.find(cond->str()) == defined.end())
            return cond->str();
    }

    if (len == 2 && cond->op == '!' && next1->name) {
        if (defined.find(next1->str()) == defined.end())
            return next1->str() + "=0";
    }

    if (len == 3 && cond->op == '(' && next1->name && next2->op == ')') {
        if (defined.find(next1->str()) == defined.end() && undefined.find(next1->str()) == undefined.end())
            return next1->str();
    }

    if (len == 3 && cond->name && (next1->str() == "==" || next1->str() == "<=" || next1->str() == ">=") && next2->number) {
        if (defined.find(cond->str()) == defined.end())
            return cond->str() + '=' + next1->next->str();
    }

    const auto lessGreaterThanConfig = [](const simplecpp::Token* dtok, const simplecpp::Token* ctok) {
        auto v = MathLib::toBigNumber(ctok->next->str());
        if (ctok->op == '<')
            v -= 1;
        else
            v += 1;
        return dtok->str() + '=' + std::to_string(v);
    };

    if (len == 3 && cond->name && (next1->op == '<' || next1->op == '>') && next2->number) {
        if (defined.find(cond->str()) == defined.end()) {
            return lessGreaterThanConfig(cond, next1);
        }
    }

    std::set<std::string> configset;
    bool isNotDefinedMacro = false;
    for (; sameline(iftok,cond); cond = cond->next) {
        if (cond->op == '!') {
            if (!sameline(iftok,cond->next) || !cond->next->name)
                break;
            if (cond->next->str() == "defined") {
                isNotDefinedMacro = true;
                continue;
            }
            configset.insert(cond->next->str() + "=0");
            continue;
        }
        if (cond->str() == "==" || cond->str() == "<=" || cond->str() == ">=") {
            if (cond->next->number) {
                const simplecpp::Token *dtok = cond->previous;
                if (sameline(iftok,dtok) && dtok->name && defined.find(dtok->str()) == defined.end() && undefined.find(dtok->str()) == undefined.end())
                    configset.insert(dtok->str() + '=' + cond->next->str());
            }
            continue;
        }
        if (cond->op == '<' || cond->op == '>') {
            if (cond->next->number) {
                const simplecpp::Token *dtok = cond->previous;
                if (sameline(iftok,dtok) && dtok->name && defined.find(dtok->str()) == defined.end() && undefined.find(dtok->str()) == undefined.end()) {
                    configset.insert(lessGreaterThanConfig(dtok, cond));
                }
            }
            continue;
        }
        if (cond->str() == "defined") {
            const simplecpp::Token *dtok = cond->next;
            if (!dtok)
                break;
            if (dtok->op == '(')
                dtok = dtok->next;

            if (sameline(iftok,dtok) && dtok->name && defined.find(dtok->str()) == defined.end() && undefined.find(dtok->str()) == undefined.end()) {
                if (!isNotDefinedMacro) {
                    configset.insert(dtok->str() + "=" + dtok->str()); // if defined is set to itself.
                } else {
                    configset.insert(dtok->str());
                }
                isNotDefinedMacro = false;
            }
            continue;
        }
    }
    std::string cfgStr;
    for (const std::string &s : configset) {
        if (!cfgStr.empty())
            cfgStr += ';';
        cfgStr += s;
    }
    return cfgStr;
}

static bool hasDefine(const std::string &userDefines, const std::string &cfg)
{
    if (cfg.empty()) {
        return false;
    }

    std::string::size_type pos = 0;
    const std::string cfgname = cfg.substr(0, cfg.find('='));
    while (pos < userDefines.size()) {
        pos = userDefines.find(cfgname, pos);
        if (pos == std::string::npos)
            break;
        const std::string::size_type pos2 = pos + cfgname.size();
        if ((pos == 0 || userDefines[pos-1U] == ';') && (pos2 == userDefines.size() || userDefines[pos2] == '='))
            return true;
        pos = pos2;
    }
    return false;
}

static std::string cfg(const std::vector<std::string> &configs, const std::string &userDefines)
{
    std::set<std::string> configs2(configs.cbegin(), configs.cend());
    std::string ret;
    for (const std::string &c : configs2) {
        if (c.empty())
            continue;
        if (c == "0")
            return "";
        if (hasDefine(userDefines, c))
            continue;
        if (!ret.empty())
            ret += ';';
        ret += c;
    }
    return ret;
}

static bool isUndefined(const std::string &cfg, const std::set<std::string> &undefined)
{
    for (std::string::size_type pos1 = 0U; pos1 < cfg.size();) {
        const std::string::size_type pos2 = cfg.find(';',pos1);
        const std::string def = (pos2 == std::string::npos) ? cfg.substr(pos1) : cfg.substr(pos1, pos2 - pos1);

        const std::string::size_type eq = def.find('=');
        if (eq == std::string::npos && undefined.find(def) != undefined.end())
            return true;
        if (eq != std::string::npos && undefined.find(def.substr(0,eq)) != undefined.end() && def.substr(eq) != "=0")
            return true;

        pos1 = (pos2 == std::string::npos) ? pos2 : pos2 + 1U;
    }
    return false;
}

static bool getConfigsElseIsFalse(const std::vector<std::string> &configs_if, const std::string &userDefines)
{
    return std::any_of(configs_if.cbegin(), configs_if.cend(),
                       [&](const std::string &cfg) {
        return hasDefine(userDefines, cfg);
    });
}

static const simplecpp::Token *gotoEndIf(const simplecpp::Token *cmdtok)
{
    int level = 0;
    while (nullptr != (cmdtok = cmdtok->next)) {
        if (cmdtok->op == '#' && !sameline(cmdtok->previous,cmdtok) && sameline(cmdtok, cmdtok->next)) {
            if (startsWith(cmdtok->next->str(),"if"))
                ++level;
            else if (cmdtok->next->str() == "endif") {
                --level;
                if (level < 0)
                    return cmdtok;
            }
        }
    }
    return nullptr;
}

static void getConfigs(const simplecpp::TokenList &tokens, std::set<std::string> &defined, const std::string &userDefines, const std::set<std::string> &undefined, std::set<std::string> &ret)
{
    std::vector<std::string> configs_if;
    std::vector<std::string> configs_ifndef;
    std::string elseError;

    for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next) {
        if (tok->op != '#' || sameline(tok->previous, tok))
            continue;
        const simplecpp::Token *cmdtok = tok->next;
        if (!sameline(tok, cmdtok))
            continue;
        if (cmdtok->str() == "ifdef" || cmdtok->str() == "ifndef" || cmdtok->str() == "if") {
            std::string config;
            if (cmdtok->str() == "ifdef" || cmdtok->str() == "ifndef") {
                const simplecpp::Token *expr1 = cmdtok->next;
                if (sameline(tok,expr1) && expr1->name && !sameline(tok,expr1->next))
                    config = expr1->str();
                if (defined.find(config) != defined.end()) {
                    config.clear();
                } else if ((cmdtok->str() == "ifdef") && sameline(cmdtok,expr1) && !config.empty()) {
                    config.append("=" + expr1->str()); //Set equal to itself if ifdef.
                }
            } else if (cmdtok->str() == "if") {
                config = readcondition(cmdtok, defined, undefined);
            }

            // skip undefined configurations..
            if (isUndefined(config, undefined))
                config.clear();

            bool ifndef = false;
            if (cmdtok->str() == "ifndef")
                ifndef = true;
            else {
                const std::array<std::string, 6> match{"if", "!", "defined", "(", config, ")"};
                std::size_t i = 0;
                ifndef = true;
                for (const simplecpp::Token *t = cmdtok; i < match.size(); t = t->next) {
                    if (!t || t->str() != match[i++]) {
                        ifndef = false;
                        break;
                    }
                }
            }

            // include guard..
            if (ifndef && tok->location.fileIndex > 0) {
                bool includeGuard = true;
                for (const simplecpp::Token *t = tok->previous; t; t = t->previous) {
                    if (t->location.fileIndex == tok->location.fileIndex) {
                        includeGuard = false;
                        break;
                    }
                }
                if (includeGuard) {
                    configs_if.emplace_back(/*std::string()*/);
                    configs_ifndef.emplace_back(/*std::string()*/);
                    continue;
                }
            }

            // check if config already exists in the ret set, but as a more general or more specific version
            if (cmdtok->str() != "ifndef")
            {
                const std::string::size_type eq = config.find('=');
                const std::string config2 = (eq != std::string::npos) ? config.substr(0, eq) : config + "=" + config;
                const std::set<std::string>::iterator it2 = ret.find(config2);
                if (it2 != ret.end()) {
                    if (eq == std::string::npos) {
                        // The instance in ret is more specific than the one in config (no =value), replace it with the one in config
                        ret.erase(it2);
                    } else {
                        // The instance in ret is more general than the one in config (have =value), keep the one in ret
                        config.clear();
                        continue;
                    }
                }
            }

            configs_if.push_back((cmdtok->str() == "ifndef") ? std::string() : config);
            configs_ifndef.push_back((cmdtok->str() == "ifndef") ? std::move(config) : std::string());
            ret.insert(cfg(configs_if,userDefines));
        } else if (cmdtok->str() == "elif" || cmdtok->str() == "else") {
            if (getConfigsElseIsFalse(configs_if,userDefines)) {
                tok = gotoEndIf(tok);
                if (!tok)
                    break;
                tok = tok->previous;
                continue;
            }
            if (cmdtok->str() == "else" &&
                cmdtok->next &&
                !sameline(cmdtok,cmdtok->next) &&
                sameline(cmdtok->next, cmdtok->next->next) &&
                cmdtok->next->op == '#' &&
                cmdtok->next->next->str() == "error") {
                const std::string &ifcfg = cfg(configs_if, userDefines);
                if (!ifcfg.empty()) {
                    if (!elseError.empty())
                        elseError += ';';
                    elseError += ifcfg;
                }
            }
            if (!configs_if.empty())
                configs_if.pop_back();
            if (cmdtok->str() == "elif") {
                std::string config = readcondition(cmdtok, defined, undefined);
                if (isUndefined(config,undefined))
                    config.clear();
                configs_if.push_back(std::move(config));
                ret.insert(cfg(configs_if, userDefines));
            } else if (!configs_ifndef.empty()) {
                //Check if ifndef already existing in ret as more general/specific version
                const std::string &confCandidate = configs_ifndef.back();
                if (ret.find(confCandidate) == ret.end()) {
                    // No instance of config_ifndef in ret. Check if a more specific version exists, in that case replace it
                    const std::set<std::string>::iterator it = ret.find(confCandidate + "=" + confCandidate);
                    if (it != ret.end()) {
                        // The instance in ret is more specific than the one in confCandidate (no =value), replace it with the one in confCandidate
                        ret.erase(it);
                    }
                    configs_if.push_back(configs_ifndef.back());
                    ret.insert(cfg(configs_if, userDefines));
                }
            }
        } else if (cmdtok->str() == "endif" && !sameline(tok, cmdtok->next)) {
            if (!configs_if.empty())
                configs_if.pop_back();
            if (!configs_ifndef.empty())
                configs_ifndef.pop_back();
        } else if (cmdtok->str() == "error") {
            if (!configs_ifndef.empty() && !configs_ifndef.back().empty()) {
                if (configs_ifndef.size() == 1U)
                    ret.erase("");
                std::vector<std::string> configs(configs_if);
                configs.push_back(configs_ifndef.back());
                ret.erase(cfg(configs, userDefines));
                std::set<std::string> temp;
                temp.swap(ret);
                for (const std::string &c: temp) {
                    if (c.find(configs_ifndef.back()) != std::string::npos)
                        ret.insert(c);
                    else if (c.empty())
                        ret.insert("");
                    else
                        ret.insert(c + ";" + configs_ifndef.back());
                }
                if (!elseError.empty())
                    elseError += ';';
                elseError += cfg(configs_ifndef, userDefines);
            }
            if (!configs_if.empty() && !configs_if.back().empty()) {
                const std::string &last = configs_if.back();
                if (last.size() > 2U && last.compare(last.size()-2U,2,"=0") == 0) {
                    std::vector<std::string> configs(configs_if);
                    ret.erase(cfg(configs, userDefines));
                    configs[configs.size() - 1U] = last.substr(0,last.size()-2U);
                    if (configs.size() == 1U)
                        ret.erase("");
                    if (!elseError.empty())
                        elseError += ';';
                    elseError += cfg(configs, userDefines);
                }
            }
        } else if (cmdtok->str() == "define" && sameline(tok, cmdtok->next) && cmdtok->next->name) {
            defined.insert(cmdtok->next->str());
        }
    }
    if (!elseError.empty())
        ret.insert(std::move(elseError));
}


std::set<std::string> Preprocessor::getConfigs() const
{
    std::set<std::string> ret = { "" };
    if (!mTokens.cfront())
        return ret;

    std::set<std::string> defined = { "__cplusplus" };

    // Insert library defines
    for (const auto &define : mSettings.library.defines()) {

        const std::string::size_type paren = define.find("(");
        const std::string::size_type space = define.find(" ");
        std::string::size_type end = space;

        if (paren != std::string::npos && paren < space)
            end = paren;

        defined.insert(define.substr(0, end));
    }

    ::getConfigs(mTokens, defined, mSettings.userDefines, mSettings.userUndefs, ret);

    for (const auto &filedata : mFileCache) {
        if (!mSettings.configurationExcluded(filedata->filename))
            ::getConfigs(filedata->tokens, defined, mSettings.userDefines, mSettings.userUndefs, ret);
    }

    return ret;
}

static void splitcfg(const std::string &cfg, std::list<std::string> &defines, const std::string &defaultValue)
{
    for (std::string::size_type defineStartPos = 0U; defineStartPos < cfg.size();) {
        const std::string::size_type defineEndPos = cfg.find(';', defineStartPos);
        std::string def = (defineEndPos == std::string::npos) ? cfg.substr(defineStartPos) : cfg.substr(defineStartPos, defineEndPos - defineStartPos);
        if (!defaultValue.empty() && def.find('=') == std::string::npos)
            def += '=' + defaultValue;
        defines.push_back(std::move(def));
        if (defineEndPos == std::string::npos)
            break;
        defineStartPos = defineEndPos + 1U;
    }
}

static simplecpp::DUI createDUI(const Settings &mSettings, const std::string &cfg, Standards::Language lang)
{
    // TODO: make it possible to specify platform-dependent sizes
    simplecpp::DUI dui;

    splitcfg(mSettings.userDefines, dui.defines, "1");
    if (!cfg.empty())
        splitcfg(cfg, dui.defines, "");

    for (const std::string &def : mSettings.library.defines()) {
        const std::string::size_type pos = def.find_first_of(" (");
        if (pos == std::string::npos) {
            dui.defines.push_back(def);
            continue;
        }
        std::string s = def;
        if (s[pos] == ' ') {
            s[pos] = '=';
        } else {
            s[s.find(')')+1] = '=';
        }
        dui.defines.push_back(std::move(s));
    }

    dui.undefined = mSettings.userUndefs; // -U
    dui.includePaths = mSettings.includePaths; // -I
    dui.includes = mSettings.userIncludes;  // --include
    if (lang == Standards::Language::CPP) {
        dui.std = mSettings.standards.stdValueCPP;
        if (dui.std.empty()) {
            dui.std = mSettings.standards.getCPP();
        }
        splitcfg(mSettings.platform.getLimitsDefines(Standards::getCPP(dui.std)), dui.defines, "");
    }
    else if (lang == Standards::Language::C) {
        dui.std = mSettings.standards.stdValueC;
        if (dui.std.empty()) {
            dui.std = mSettings.standards.getC();
        }
        splitcfg(mSettings.platform.getLimitsDefines(Standards::getC(dui.std)), dui.defines, "");
    }
    dui.clearIncludeCache = mSettings.clearIncludeCache;
    return dui;
}

const simplecpp::Output* Preprocessor::handleErrors(const simplecpp::OutputList& outputList)
{
    const bool showerror = (!mSettings.userDefines.empty() && !mSettings.force);
    return reportOutput(outputList, showerror);
}

bool Preprocessor::loadFiles(std::vector<std::string> &files)
{
    const simplecpp::DUI dui = createDUI(mSettings, "", mLang);

    simplecpp::OutputList outputList;
    mFileCache = simplecpp::load(mTokens, files, dui, &outputList);
    return !handleErrors(outputList);
}

void Preprocessor::removeComments()
{
    mTokens.removeComments();
    for (const auto &filedata : mFileCache) {
        filedata->tokens.removeComments();
    }
}

void Preprocessor::setPlatformInfo()
{
    mTokens.sizeOfType["bool"]          = mSettings.platform.sizeof_bool;
    mTokens.sizeOfType["short"]         = mSettings.platform.sizeof_short;
    mTokens.sizeOfType["int"]           = mSettings.platform.sizeof_int;
    mTokens.sizeOfType["long"]          = mSettings.platform.sizeof_long;
    mTokens.sizeOfType["long long"]     = mSettings.platform.sizeof_long_long;
    mTokens.sizeOfType["float"]         = mSettings.platform.sizeof_float;
    mTokens.sizeOfType["double"]        = mSettings.platform.sizeof_double;
    mTokens.sizeOfType["long double"]   = mSettings.platform.sizeof_long_double;
    mTokens.sizeOfType["bool *"]        = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["short *"]       = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["int *"]         = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["long *"]        = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["long long *"]   = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["float *"]       = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["double *"]      = mSettings.platform.sizeof_pointer;
    mTokens.sizeOfType["long double *"] = mSettings.platform.sizeof_pointer;
}

simplecpp::TokenList Preprocessor::preprocess(const std::string &cfg, std::vector<std::string> &files, simplecpp::OutputList& outputList)
{
    const simplecpp::DUI dui = createDUI(mSettings, cfg, mLang);

    std::list<simplecpp::MacroUsage> macroUsage;
    std::list<simplecpp::IfCond> ifCond;
    simplecpp::TokenList tokens2(files);
    simplecpp::preprocess(tokens2, mTokens, files, mFileCache, dui, &outputList, &macroUsage, &ifCond);
    mMacroUsage = std::move(macroUsage);
    mIfCond = std::move(ifCond);

    tokens2.removeComments();

    return tokens2;
}

std::string Preprocessor::getcode(const std::string &cfg, std::vector<std::string> &files, const bool writeLocations)
{
    simplecpp::OutputList outputList;
    simplecpp::TokenList tokens2 = preprocess(cfg, files, outputList);
    handleErrors(outputList);
    unsigned int prevfile = 0;
    unsigned int line = 1;
    std::ostringstream ret;
    for (const simplecpp::Token *tok = tokens2.cfront(); tok; tok = tok->next) {
        if (writeLocations && tok->location.fileIndex != prevfile) {
            ret << "\n#line " << tok->location.line << " \"" << mTokens.file(tok->location) << "\"\n";
            prevfile = tok->location.fileIndex;
            line = tok->location.line;
        }

        if (tok->previous && line >= tok->location.line) // #7912
            ret << ' ';
        while (tok->location.line > line) {
            ret << '\n';
            line++;
        }
        if (!tok->macro.empty())
            ret << Preprocessor::macroChar;
        ret << tok->str();
    }

    return ret.str();
}

const simplecpp::Output* Preprocessor::reportOutput(const simplecpp::OutputList &outputList, bool showerror)
{
    const simplecpp::Output* out_ret = nullptr;

    for (const simplecpp::Output &out : outputList) {
        switch (out.type) {
        case simplecpp::Output::ERROR:
            out_ret = &out;
            if (!startsWith(out.msg,"#error") || showerror)
                error(out.location, out.msg, out.type);
            break;
        case simplecpp::Output::WARNING:
        case simplecpp::Output::PORTABILITY_BACKSLASH:
            break;
        case simplecpp::Output::MISSING_HEADER: {
            // not considered an "error"
            const std::string::size_type pos1 = out.msg.find_first_of("<\"");
            const std::string::size_type pos2 = out.msg.find_first_of(">\"", pos1 + 1U);
            if (pos1 < pos2 && pos2 != std::string::npos)
                missingInclude(out.location, out.msg.substr(pos1+1, pos2-pos1-1), out.msg[pos1] == '\"' ? UserHeader : SystemHeader);
        }
        break;
        case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
        case simplecpp::Output::SYNTAX_ERROR:
        case simplecpp::Output::UNHANDLED_CHAR_ERROR:
            out_ret = &out;
            error(out.location, out.msg, out.type);
            break;
        case simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND:
        case simplecpp::Output::FILE_NOT_FOUND:
        case simplecpp::Output::DUI_ERROR:
            out_ret = &out;
            error({}, out.msg, out.type);
            break;
        }
    }

    return out_ret;
}

static std::string simplecppErrToId(simplecpp::Output::Type type)
{
    switch (type) {
    case simplecpp::Output::ERROR:
        return "preprocessorErrorDirective";
    case simplecpp::Output::SYNTAX_ERROR:
        return "syntaxError";
    case simplecpp::Output::UNHANDLED_CHAR_ERROR:
        return "unhandledChar";
    case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
        return "includeNestedTooDeeply";
    case simplecpp::Output::FILE_NOT_FOUND:
        return "missingFile";
    case simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND:
        return "missingIncludeExplicit";
    // should never occur
    case simplecpp::Output::DUI_ERROR:
    // handled separately
    case simplecpp::Output::MISSING_HEADER:
    // no handled at all (warnings)
    case simplecpp::Output::WARNING:
    case simplecpp::Output::PORTABILITY_BACKSLASH:
        throw std::runtime_error("unexpected simplecpp::Output type " + std::to_string(type));
    }

    cppcheck::unreachable();
}

void Preprocessor::error(const simplecpp::Location& loc, const std::string &msg, simplecpp::Output::Type type)
{
    error(loc, msg, simplecppErrToId(type));
}

void Preprocessor::error(const simplecpp::Location& loc, const std::string &msg, const std::string& id)
{
    std::list<ErrorMessage::FileLocation> locationList;
    if (!mTokens.file(loc).empty()) {
        std::string file = Path::fromNativeSeparators(mTokens.file(loc));
        if (mSettings.relativePaths)
            file = Path::getRelativePath(file, mSettings.basePaths);

        locationList.emplace_back(file, loc.line, loc.col);
    }
    mErrorLogger.reportErr(ErrorMessage(std::move(locationList),
                                        mFile0,
                                        Severity::error,
                                        msg,
                                        id,
                                        Certainty::normal));
}

// Report that include is missing
void Preprocessor::missingInclude(const simplecpp::Location& loc, const std::string &header, HeaderTypes headerType)
{
    if (!mSettings.checks.isEnabled(Checks::missingInclude))
        return;

    std::list<ErrorMessage::FileLocation> locationList;
    if (!mTokens.file(loc).empty()) {
        // TODO: add relative path handling?
        locationList.emplace_back(mTokens.file(loc), loc.line, loc.col);
    }
    ErrorMessage errmsg(std::move(locationList), mFile0, Severity::information,
                        (headerType==SystemHeader) ?
                        "Include file: <" + header + "> not found. Please note: Standard library headers do not need to be provided to get proper results." :
                        "Include file: \"" + header + "\" not found.",
                        (headerType==SystemHeader) ? "missingIncludeSystem" : "missingInclude",
                        Certainty::normal);
    mErrorLogger.reportErr(errmsg);
}

void Preprocessor::invalidSuppression(const simplecpp::Location& loc, const std::string &msg)
{
    error(loc, msg, "invalidSuppression");
}

void Preprocessor::getErrorMessages(ErrorLogger &errorLogger, const Settings &settings)
{
    std::vector<std::string> files;
    simplecpp::TokenList tokens(files);
    Preprocessor preprocessor(tokens, settings, errorLogger, Standards::Language::CPP);
    simplecpp::Location loc;
    loc.line = 1;
    loc.col = 2;
    preprocessor.missingInclude(loc, "", UserHeader);
    preprocessor.missingInclude(loc, "", SystemHeader);
    preprocessor.error(loc, "message", simplecpp::Output::ERROR);
    preprocessor.error(loc, "message", simplecpp::Output::SYNTAX_ERROR);
    preprocessor.error(loc, "message", simplecpp::Output::UNHANDLED_CHAR_ERROR);
    preprocessor.error(loc, "message", simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY);
    preprocessor.error(loc, "message", simplecpp::Output::FILE_NOT_FOUND);
    preprocessor.error(loc, "message", simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND);
    preprocessor.invalidSuppression(loc, "message");
}

void Preprocessor::dump(std::ostream &out) const
{
    // Create a xml dump.

    if (!mMacroUsage.empty()) {
        out << "  <macro-usage>" << std::endl;
        for (const simplecpp::MacroUsage &macroUsage: mMacroUsage) {
            out << "    <macro"
                << " name=\"" << macroUsage.macroName << "\""
                << " file=\"" << ErrorLogger::toxml(mTokens.file(macroUsage.macroLocation)) << "\""
                << " line=\"" << macroUsage.macroLocation.line << "\""
                << " column=\"" << macroUsage.macroLocation.col << "\""
                << " usefile=\"" << ErrorLogger::toxml(mTokens.file(macroUsage.useLocation)) << "\""
                << " useline=\"" << macroUsage.useLocation.line << "\""
                << " usecolumn=\"" << macroUsage.useLocation.col << "\""
                << " is-known-value=\"" << bool_to_string(macroUsage.macroValueKnown) << "\""
                << "/>" << std::endl;
        }
        out << "  </macro-usage>" << std::endl;
    }

    if (!mIfCond.empty()) {
        out << "  <simplecpp-if-cond>" << std::endl;
        for (const simplecpp::IfCond &ifCond: mIfCond) {
            out << "    <if-cond"
                << " file=\"" << ErrorLogger::toxml(mTokens.file(ifCond.location)) << "\""
                << " line=\"" << ifCond.location.line << "\""
                << " column=\"" << ifCond.location.col << "\""
                << " E=\"" << ErrorLogger::toxml(ifCond.E) << "\""
                << " result=\"" << ifCond.result << "\""
                << "/>" << std::endl;
        }
        out << "  </simplecpp-if-cond>" << std::endl;
    }
}

std::size_t Preprocessor::calculateHash(const std::string &toolinfo) const
{
    std::string hashData = toolinfo;
    for (const simplecpp::Token *tok = mTokens.cfront(); tok; tok = tok->next) {
        if (!tok->comment) {
            hashData += tok->str();
            hashData += static_cast<char>(tok->location.line);
            hashData += static_cast<char>(tok->location.col);
        }
    }
    for (const auto &filedata : mFileCache) {
        for (const simplecpp::Token *tok = filedata->tokens.cfront(); tok; tok = tok->next) {
            if (!tok->comment) {
                hashData += tok->str();
                hashData += static_cast<char>(tok->location.line);
                hashData += static_cast<char>(tok->location.col);
            }
        }
    }
    return (std::hash<std::string>{})(hashData);
}

void Preprocessor::simplifyPragmaAsm()
{
    Preprocessor::simplifyPragmaAsmPrivate(mTokens);
    for (const auto &filedata : mFileCache) {
        Preprocessor::simplifyPragmaAsmPrivate(filedata->tokens);
    }
}

void Preprocessor::simplifyPragmaAsmPrivate(simplecpp::TokenList &tokenList)
{
    // assembler code..
    for (simplecpp::Token *tok = tokenList.front(); tok; tok = tok->next) {
        if (tok->op != '#')
            continue;
        if (sameline(tok, tok->previousSkipComments()))
            continue;

        const simplecpp::Token * const tok2 = tok->nextSkipComments();
        if (!tok2 || !sameline(tok, tok2) || tok2->str() != "pragma")
            continue;

        const simplecpp::Token * const tok3 = tok2->nextSkipComments();
        if (!tok3 || !sameline(tok, tok3) || tok3->str() != "asm")
            continue;

        const simplecpp::Token *endasm = tok3;
        bool foundEndasm = false;
        while ((endasm = endasm->next) != nullptr) {
            if (endasm->op != '#' || sameline(endasm,endasm->previousSkipComments()))
                continue;
            const simplecpp::Token * const endasm2 = endasm->nextSkipComments();
            if (!endasm2 || !sameline(endasm, endasm2) || endasm2->str() != "pragma")
                continue;
            const simplecpp::Token * const endasm3 = endasm2->nextSkipComments();
            if (!endasm3 || !sameline(endasm2, endasm3) || endasm3->str() != "endasm")
                continue;
            while (sameline(endasm,endasm3))
                endasm = endasm->next;
            foundEndasm = true;
            break;
        }

        if (!foundEndasm)
            throw InternalError(nullptr, "syntax error: missing #pragma endasm", InternalError::SYNTAX);

        const simplecpp::Token * const tok4 = tok3->next;
        tok->setstr("asm");
        const_cast<simplecpp::Token *>(tok2)->setstr("(");
        const_cast<simplecpp::Token *>(tok3)->setstr(")");
        const_cast<simplecpp::Token *>(tok4)->setstr(";");
        while (tok4->next != endasm)
            tokenList.deleteToken(tok4->next);
    }
}


void Preprocessor::addRemarkComments(const simplecpp::TokenList &tokens, std::vector<RemarkComment> &remarkComments) const
{
    for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next) {
        if (!tok->comment)
            continue;

        const std::string& comment = tok->str();

        // is it a remark comment?
        const std::string::size_type pos1 = comment.find_first_not_of("/* \t");
        if (pos1 == std::string::npos)
            continue;
        const std::string::size_type pos2 = comment.find_first_of(": \t", pos1);
        if (pos2 != pos1 + 6 || comment.compare(pos1, 6, "REMARK") != 0)
            continue;
        const std::string::size_type pos3 = comment.find_first_not_of(": \t", pos2);
        if (pos3 == std::string::npos)
            continue;
        if (comment.compare(0,2,"/*") == 0 && pos3 + 2 >= tok->str().size())
            continue;

        const std::string::size_type pos4 = (comment.compare(0,2,"/*") == 0) ? comment.size()-2 : comment.size();
        const std::string remarkText = comment.substr(pos3, pos4-pos3);

        // Get remarked token
        const simplecpp::Token* remarkedToken = nullptr;
        for (const simplecpp::Token* after = tok->next; after; after = after->next) {
            if (after->comment)
                continue;
            remarkedToken = after;
            break;
        }
        for (const simplecpp::Token* prev = tok->previous; prev; prev = prev->previous) {
            if (prev->comment)
                continue;
            if (sameline(prev, tok))
                remarkedToken = prev;
            break;
        }
        if (!remarkedToken)
            continue;

        // Relative filename
        const std::string relativeFilename = getRelativeFilename(tokens, remarkedToken, mSettings);

        // Add the suppressions.
        remarkComments.emplace_back(relativeFilename, remarkedToken->location.line, remarkText);
    }
}
