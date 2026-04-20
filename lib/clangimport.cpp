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

#include "clangimport.h"

#include "errortypes.h"
#include "mathlib.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

class Settings;

static const std::string AccessSpecDecl = "AccessSpecDecl";
static const std::string ArraySubscriptExpr = "ArraySubscriptExpr";
static const std::string BinaryOperator = "BinaryOperator";
static const std::string BreakStmt = "BreakStmt";
static const std::string CallExpr = "CallExpr";
static const std::string CaseStmt = "CaseStmt";
static const std::string CharacterLiteral = "CharacterLiteral";
static const std::string ClassTemplateDecl = "ClassTemplateDecl";
static const std::string ClassTemplateSpecializationDecl = "ClassTemplateSpecializationDecl";
static const std::string ConditionalOperator = "ConditionalOperator";
static const std::string ConstantExpr = "ConstantExpr";
static const std::string CompoundAssignOperator = "CompoundAssignOperator";
static const std::string CompoundStmt = "CompoundStmt";
static const std::string ContinueStmt = "ContinueStmt";
static const std::string CStyleCastExpr = "CStyleCastExpr";
static const std::string CXXBindTemporaryExpr = "CXXBindTemporaryExpr";
static const std::string CXXBoolLiteralExpr = "CXXBoolLiteralExpr";
static const std::string CXXConstructorDecl = "CXXConstructorDecl";
static const std::string CXXConstructExpr = "CXXConstructExpr";
static const std::string CXXDefaultArgExpr = "CXXDefaultArgExpr";
static const std::string CXXDeleteExpr = "CXXDeleteExpr";
static const std::string CXXDestructorDecl = "CXXDestructorDecl";
static const std::string CXXForRangeStmt = "CXXForRangeStmt";
static const std::string CXXFunctionalCastExpr = "CXXFunctionalCastExpr";
static const std::string CXXMemberCallExpr = "CXXMemberCallExpr";
static const std::string CXXMethodDecl = "CXXMethodDecl";
static const std::string CXXNewExpr = "CXXNewExpr";
static const std::string CXXNullPtrLiteralExpr = "CXXNullPtrLiteralExpr";
static const std::string CXXOperatorCallExpr = "CXXOperatorCallExpr";
static const std::string CXXRecordDecl = "CXXRecordDecl";
static const std::string CXXStaticCastExpr = "CXXStaticCastExpr";
static const std::string CXXStdInitializerListExpr = "CXXStdInitializerListExpr";
static const std::string CXXTemporaryObjectExpr = "CXXTemporaryObjectExpr";
static const std::string CXXThisExpr = "CXXThisExpr";
static const std::string CXXThrowExpr = "CXXThrowExpr";
static const std::string DeclRefExpr = "DeclRefExpr";
static const std::string DeclStmt = "DeclStmt";
static const std::string DefaultStmt = "DefaultStmt";
static const std::string DoStmt = "DoStmt";
static const std::string EnumConstantDecl = "EnumConstantDecl";
static const std::string EnumDecl = "EnumDecl";
static const std::string ExprWithCleanups = "ExprWithCleanups";
static const std::string FieldDecl = "FieldDecl";
static const std::string FloatingLiteral = "FloatingLiteral";
static const std::string ForStmt = "ForStmt";
static const std::string FunctionDecl = "FunctionDecl";
static const std::string FunctionTemplateDecl = "FunctionTemplateDecl";
static const std::string GotoStmt = "GotoStmt";
static const std::string IfStmt = "IfStmt";
static const std::string ImplicitCastExpr = "ImplicitCastExpr";
static const std::string InitListExpr = "InitListExpr";
static const std::string IntegerLiteral = "IntegerLiteral";
static const std::string LabelStmt = "LabelStmt";
static const std::string LinkageSpecDecl = "LinkageSpecDecl";
static const std::string MaterializeTemporaryExpr = "MaterializeTemporaryExpr";
static const std::string MemberExpr = "MemberExpr";
static const std::string NamespaceDecl = "NamespaceDecl";
static const std::string NullStmt = "NullStmt";
static const std::string ParenExpr = "ParenExpr";
static const std::string ParmVarDecl = "ParmVarDecl";
static const std::string RecordDecl = "RecordDecl";
static const std::string ReturnStmt = "ReturnStmt";
static const std::string StringLiteral = "StringLiteral";
static const std::string SwitchStmt = "SwitchStmt";
static const std::string TemplateArgument = "TemplateArgument";
static const std::string TypedefDecl = "TypedefDecl";
static const std::string UnaryOperator = "UnaryOperator";
static const std::string UnaryExprOrTypeTraitExpr = "UnaryExprOrTypeTraitExpr";
static const std::string VarDecl = "VarDecl";
static const std::string WhileStmt = "WhileStmt";

static std::string unquote(const std::string &s)
{
    return (s[0] == '\'') ? s.substr(1, s.size() - 2) : s;
}


static std::vector<std::string> splitString(const std::string &line)
{
    std::vector<std::string> ret;
    std::string::size_type pos1 = line.find_first_not_of(' ');
    while (pos1 < line.size()) {
        std::string::size_type pos2;
        if (std::strchr("*()", line[pos1])) {
            ret.push_back(line.substr(pos1,1));
            pos1 = line.find_first_not_of(' ', pos1 + 1);
            continue;
        }
        if (line[pos1] == '<')
            pos2 = line.find('>', pos1);
        else if (line[pos1] == '\"')
            pos2 = line.find('\"', pos1+1);
        else if (line[pos1] == '\'') {
            pos2 = line.find('\'', pos1+1);
            if (pos2 < static_cast<int>(line.size()) - 3 && line.compare(pos2, 3, "\':\'", 0, 3) == 0)
                pos2 = line.find('\'', pos2 + 3);
        } else {
            pos2 = pos1;
            while (pos2 < line.size() && (line[pos2] == '_' || line[pos2] == ':' || std::isalnum(static_cast<unsigned char>(line[pos2]))))
                ++pos2;
            if (pos2 > pos1 && pos2 < line.size() && line[pos2] == '<' && std::isalpha(line[pos1])) {
                int tlevel = 1;
                while (++pos2 < line.size() && tlevel > 0) {
                    if (line[pos2] == '<')
                        ++tlevel;
                    else if (line[pos2] == '>')
                        --tlevel;
                }
                if (tlevel == 0 && pos2 < line.size() && line[pos2] == ' ') {
                    ret.push_back(line.substr(pos1, pos2-pos1));
                    pos1 = pos2 + 1;
                    continue;
                }
            }

            pos2 = line.find(' ', pos1) - 1;
            if ((std::isalpha(line[pos1]) || line[pos1] == '_') &&
                line.find("::", pos1) < pos2 &&
                line.find("::", pos1) < line.find('<', pos1)) {
                pos2 = line.find("::", pos1);
                ret.push_back(line.substr(pos1, pos2-pos1));
                ret.emplace_back("::");
                pos1 = pos2 + 2;
                continue;
            }
            if ((std::isalpha(line[pos1]) || line[pos1] == '_') &&
                line.find('<', pos1) < pos2 &&
                line.find("<<",pos1) != line.find('<',pos1) &&
                line.find('>', pos1) != std::string::npos &&
                line.find('>', pos1) > pos2) {
                int level = 0;
                for (pos2 = pos1; pos2 < line.size(); ++pos2) {
                    if (line[pos2] == '<')
                        ++level;
                    else if (line[pos2] == '>') {
                        if (level <= 1)
                            break;
                        --level;
                    }
                }
                if (level > 1 && pos2 + 1 >= line.size())
                    return std::vector<std::string> {};
                pos2 = line.find(' ', pos2);
                if (pos2 != std::string::npos)
                    --pos2;
            }
        }
        if (pos2 == std::string::npos) {
            ret.push_back(line.substr(pos1));
            break;
        }
        ret.push_back(line.substr(pos1, pos2+1-pos1));
        pos1 = line.find_first_not_of(' ', pos2 + 1);
    }
    return ret;
}


namespace clangimport {
    struct Data {
        struct Decl {
            explicit Decl(Scope *scope) : scope(scope) {}
            Decl(Token *def, Variable *var) : def(def), var(var) {}
            Decl(Token *def, Function *function) : def(def), function(function) {}
            Decl(Token *def, Enumerator *enumerator) : def(def), enumerator(enumerator) {}
            void ref(Token *tok) const {
                if (enumerator)
                    tok->enumerator(enumerator);
                if (function)
                    tok->function(function);
                if (var) {
                    tok->variable(var);
                    tok->varId(var->declarationId());
                }
            }
            Token* def{};
            Enumerator* enumerator{};
            Function* function{};
            Scope* scope{};
            Variable* var{};
        };

        Data(const Settings& settings, SymbolDatabase& symbolDatabase)
            : mSettings(settings)
            , mSymbolDatabase(symbolDatabase)
        {}

        const Settings &mSettings;
        SymbolDatabase &mSymbolDatabase;

        int enumValue = 0;

        void enumDecl(const std::string &addr, Token *nameToken, Enumerator *enumerator) {
            Decl decl(nameToken, enumerator);
            mDeclMap.emplace(addr, decl);
            nameToken->enumerator(enumerator);
            notFound(addr);
        }

        void funcDecl(const std::string &addr, Token *nameToken, Function *function) {
            Decl decl(nameToken, function);
            mDeclMap.emplace(addr, decl);
            nameToken->function(function);
            notFound(addr);
        }

        void scopeDecl(const std::string &addr, Scope *scope) {
            Decl decl(scope);
            mDeclMap.emplace(addr, decl);
        }

        void varDecl(const std::string &addr, Token *def, Variable *var) {
            Decl decl(def, var);
            mDeclMap.emplace(addr, decl);
            def->varId(++mVarId);
            def->variable(var);
            if (def->valueType())
                var->setValueType(*def->valueType());
            notFound(addr);
        }

        void replaceVarDecl(const Variable *from, Variable *to) {
            for (auto &it: mDeclMap) {
                Decl &decl = it.second;
                if (decl.var == from)
                    decl.var = to;
            }
        }

        void ref(const std::string &addr, Token *tok) {
            auto it = mDeclMap.find(addr);
            if (it != mDeclMap.end())
                it->second.ref(tok);
            else
                mNotFound[addr].push_back(tok);
        }

        std::vector<const Variable *> getVariableList() const {
            std::vector<const Variable *> ret;
            ret.resize(mVarId + 1, nullptr);
            for (const auto& it: mDeclMap) {
                if (it.second.var)
                    ret[it.second.var->declarationId()] = it.second.var;
            }
            return ret;
        }

        bool hasDecl(const std::string &addr) const {
            return mDeclMap.find(addr) != mDeclMap.end();
        }

        const Scope *getScope(const std::string &addr) {
            auto it = mDeclMap.find(addr);
            return (it == mDeclMap.end() ? nullptr : it->second.scope);
        }

        // "}" tokens that are not end-of-scope
        std::set<Token *> mNotScope;

        std::map<const Scope *, AccessControl> scopeAccessControl;
    private:
        void notFound(const std::string &addr) {
            auto it = mNotFound.find(addr);
            if (it != mNotFound.end()) {
                for (Token *reftok: it->second)
                    ref(addr, reftok);
                mNotFound.erase(it);
            }
        }

        std::map<std::string, Decl> mDeclMap;
        std::map<std::string, std::vector<Token *>> mNotFound;
        int mVarId = 0;
    };

    class AstNode;
    using AstNodePtr = std::shared_ptr<AstNode>;

    class AstNode {
    public:
        AstNode(std::string nodeType, const std::string &ext, Data *data)
            : nodeType(std::move(nodeType)), mExtTokens(splitString(ext)), mData(data)
        {}
        std::string nodeType;
        std::vector<AstNodePtr> children;

        bool isPrologueTypedefDecl() const;
        /**
         * @throws InternalError thrown if AST location is invalid
         */
        void setLocations(TokenList &tokenList, int file, int line, int col);

        void dumpAst(int num = 0, int indent = 0) const;
        void createTokens1(TokenList &tokenList) {
            //dumpAst(); // TODO: reactivate or remove
            if (isPrologueTypedefDecl())
                return;
            if (!tokenList.back()) {
                setLocations(tokenList, 0, 1, 1);
            }
            else
                setLocations(tokenList, tokenList.back()->fileIndex(), tokenList.back()->linenr(), 1);
            createTokens(tokenList);
            if (nodeType == VarDecl || nodeType == RecordDecl || nodeType == TypedefDecl)
                addtoken(tokenList, ";");
            mData->mNotScope.clear();
        }

        /**
         * @throws InternalError thrown if index is out of bounds
         */
        AstNodePtr getChild(int c) {
            if (c >= children.size()) {
                std::ostringstream err;
                err << "ClangImport: AstNodePtr::getChild(" << c << ") out of bounds. children.size=" << children.size() << " " << nodeType;
                for (const std::string &s: mExtTokens)
                    err << " " << s;
                throw InternalError(nullptr, err.str());
            }
            return children[c];
        }
    private:
        /**
         * @throws InternalError thrown if CXXForRangeStmt cannot be imported
         */
        Token *createTokens(TokenList &tokenList);
        Token *addtoken(TokenList &tokenList, const std::string &str, bool valueType=true);
        const ::Type *addTypeTokens(TokenList &tokenList, const std::string &str, const Scope *scope = nullptr);
        void addFullScopeNameTokens(TokenList &tokenList, const Scope *recordScope);
        Scope *createScope(TokenList &tokenList, ScopeType scopeType, AstNodePtr astNode, const Token *def);
        Scope *createScope(TokenList &tokenList, ScopeType scopeType, const std::vector<AstNodePtr> &children2, const Token *def);
        RET_NONNULL Token *createTokensCall(TokenList &tokenList);
        void createTokensFunctionDecl(TokenList &tokenList);
        void createTokensForCXXRecord(TokenList &tokenList);
        Token *createTokensVarDecl(TokenList &tokenList);
        std::string getSpelling() const;
        std::string getType(int index = 0) const;
        std::string getFullType(int index = 0) const;
        bool isDefinition() const;
        std::string getTemplateParameters() const;
        const Scope *getNestedInScope(TokenList &tokenList);
        void setValueType(Token *tok);

        int mFile  = 0;
        int mLine  = 1;
        int mCol   = 1;
        std::vector<std::string> mExtTokens;
        Data *mData;
    };
}

std::string clangimport::AstNode::getSpelling() const
{
    if (nodeType == CompoundAssignOperator) {
        std::size_t typeIndex = 1;
        while (typeIndex < mExtTokens.size() && mExtTokens[typeIndex][0] != '\'')
            typeIndex++;
        // name is next quoted token
        std::size_t nameIndex = typeIndex + 1;
        while (nameIndex < mExtTokens.size() && mExtTokens[nameIndex][0] != '\'')
            nameIndex++;
        return (nameIndex < mExtTokens.size()) ? unquote(mExtTokens[nameIndex]) : "";
    }

    if (nodeType == UnaryExprOrTypeTraitExpr) {
        std::size_t typeIndex = 1;
        while (typeIndex < mExtTokens.size() && mExtTokens[typeIndex][0] != '\'')
            typeIndex++;
        const std::size_t nameIndex = typeIndex + 1;
        return (nameIndex < mExtTokens.size()) ? unquote(mExtTokens[nameIndex]) : "";
    }

    int typeIndex = mExtTokens.size() - 1;
    if (nodeType == FunctionDecl || nodeType == CXXConstructorDecl || nodeType == CXXMethodDecl) {
        while (typeIndex >= 0 && mExtTokens[typeIndex][0] != '\'')
            typeIndex--;
        if (typeIndex <= 0)
            return "";
    }
    if (nodeType == DeclRefExpr) {
        while (typeIndex > 0 && std::isalpha(mExtTokens[typeIndex][0]))
            typeIndex--;
        if (typeIndex <= 0)
            return "";
    }
    const std::string &str = mExtTokens[typeIndex - 1];
    if (startsWith(str,"col:"))
        return "";
    if (startsWith(str,"<invalid"))
        return "";
    if (nodeType == RecordDecl && str == "struct")
        return "";
    return str;
}

std::string clangimport::AstNode::getType(int index) const
{
    std::string type = getFullType(index);
    if (type.find(" (") != std::string::npos) {
        const std::string::size_type pos = type.find(" (");
        type[pos] = '\'';
        type.erase(pos+1);
    }
    if (type.find(" *(") != std::string::npos) {
        const std::string::size_type pos = type.find(" *(") + 2;
        type[pos] = '\'';
        type.erase(pos+1);
    }
    if (type.find(" &(") != std::string::npos) {
        const std::string::size_type pos = type.find(" &(") + 2;
        type[pos] = '\'';
        type.erase(pos+1);
    }
    return unquote(type);
}

std::string clangimport::AstNode::getFullType(int index) const
{
    std::size_t typeIndex = 1;
    while (typeIndex < mExtTokens.size() && mExtTokens[typeIndex][0] != '\'')
        typeIndex++;
    if (typeIndex >= mExtTokens.size())
        return "";
    std::string type = mExtTokens[typeIndex];
    if (type.find("\':\'") != std::string::npos) {
        if (index == 0)
            type.erase(type.find("\':\'") + 1);
        else
            type.erase(0, type.find("\':\'") + 2);
    }
    return type;
}

bool clangimport::AstNode::isDefinition() const
{
    return contains(mExtTokens, "definition");
}

std::string clangimport::AstNode::getTemplateParameters() const
{
    if (children.empty() || children[0]->nodeType != TemplateArgument)
        return "";
    std::string templateParameters;
    for (const AstNodePtr& child: children) {
        if (child->nodeType == TemplateArgument) {
            if (templateParameters.empty())
                templateParameters = "<";
            else
                templateParameters += ",";
            templateParameters += unquote(child->mExtTokens.back());
        }
    }
    return templateParameters + ">";
}

// cppcheck-suppress unusedFunction // only used in comment
void clangimport::AstNode::dumpAst(int num, int indent) const
{
    (void)num;
    std::cout << std::string(indent, ' ') << nodeType;
    for (const auto& tok: mExtTokens)
        std::cout << " " << tok;
    std::cout << std::endl;
    for (int c = 0; c < children.size(); ++c) {
        if (children[c])
            children[c]->dumpAst(c, indent + 2);
        else
            std::cout << std::string(indent + 2, ' ') << "<<<<NULL>>>>>" << std::endl;
    }
}

bool clangimport::AstNode::isPrologueTypedefDecl() const
{
    // these TypedefDecl are included in *any* AST dump and we should ignore them as they should not be of interest to us
    // see https://github.com/llvm/llvm-project/issues/120228#issuecomment-2549212109 for an explanation
    if (nodeType != TypedefDecl)
        return false;

    // TODO: use different values to indicate "<invalid sloc>"?
    if (mFile != 0 || mLine != 1 || mCol != 1)
        return false;

    // TODO: match without using children
    if (children.empty())
        return false;

    if (children[0].get()->mExtTokens.size() < 2)
        return false;

    const auto& type = children[0].get()->mExtTokens[1];
    if (type == "'__int128'" ||
        type == "'unsigned __int128'" ||
        type == "'struct __NSConstantString_tag'" ||
        type == "'char *'" ||
        type == "'struct __va_list_tag[1]'")
    {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        return true;
    }

    return false;
}

void clangimport::AstNode::setLocations(TokenList &tokenList, int file, int line, int col)
{
    if (mExtTokens.size() >= 2)
    {
        const std::string &ext = mExtTokens[1];
        if (startsWith(ext, "<col:"))
            col = strToInt<int>(ext.substr(5, ext.find_first_of(",>", 5) - 5));
        else if (startsWith(ext, "<line:")) {
            line = strToInt<int>(ext.substr(6, ext.find_first_of(":,>", 6) - 6));
            const auto pos = ext.find(", col:");
            if (pos != std::string::npos)
                col = strToInt<int>(ext.substr(pos+6, ext.find_first_of(":,>", pos+6) - (pos+6)));
        } else if (ext[0] == '<') {
            const std::string::size_type colon = ext.find(':');
            if (colon != std::string::npos) {
                const bool windowsPath = colon == 2 && ext.size() > 3 && ext[2] == ':';
                const std::string::size_type sep1 = windowsPath ? ext.find(':', 4) : colon;
                const std::string::size_type sep2 = ext.find(':', sep1 + 1);
                file = tokenList.appendFileIfNew(ext.substr(1, sep1 - 1));
                line = strToInt<int>(ext.substr(sep1 + 1, sep2 - sep1 - 1));
            }
            else {
                // "<invalid sloc>" are encountered in every AST dump by some built-in TypedefDecl
                // an completely empty location block was encountered with a CompoundStmt
                if (ext != "<<invalid sloc>" && ext != "<>")
                    throw InternalError(nullptr, "invalid AST location: " + ext, InternalError::AST);
            }
        }
    }
    mFile = file;
    mLine = line;
    mCol = col;
    for (const auto& child: children) {
        if (child)
            child->setLocations(tokenList, file, line, col);
    }
}

Token *clangimport::AstNode::addtoken(TokenList &tokenList, const std::string &str, bool valueType)
{
    const Scope *scope = getNestedInScope(tokenList);
    tokenList.addtoken(str, mLine, mCol, mFile);
    tokenList.back()->scope(scope);
    if (valueType)
        setValueType(tokenList.back());
    return tokenList.back();
}

const ::Type * clangimport::AstNode::addTypeTokens(TokenList &tokenList, const std::string &str, const Scope *scope)
{
    if (str.find("\':\'") != std::string::npos) {
        return addTypeTokens(tokenList, str.substr(0, str.find("\':\'") + 1), scope);
    }

    if (startsWith(str, "'enum (anonymous"))
        return nullptr;

    std::string type;
    if (str.find(" (") != std::string::npos) {
        if (str.find('<') != std::string::npos)
            type = str.substr(1, str.find('<')) + "...>";
        else
            type = str.substr(1,str.find(" (")-1);
    } else
        type = unquote(str);

    if (type.find("(*)(") != std::string::npos) {
        type.erase(type.find("(*)("));
        type += "*";
    }
    if (type.find('(') != std::string::npos)
        type.erase(type.find('('));

    // TODO: put in a helper?
    std::stack<Token *> lpar;
    for (const std::string &s: splitString(type)) {
        Token *tok = addtoken(tokenList, s, false);
        if (tok->str() == "(")
            lpar.push(tok);
        else if (tok->str() == ")") {
            Token::createMutualLinks(tok, lpar.top());
            lpar.pop();
        }
    }

    // Set Type
    if (!scope) {
        scope = tokenList.back() ? tokenList.back()->scope() : nullptr;
        if (!scope)
            return nullptr;
    }
    for (const Token *typeToken = tokenList.back(); Token::Match(typeToken, "&|*|%name%"); typeToken = typeToken->previous()) {
        if (!typeToken->isName())
            continue;
        const ::Type *recordType = scope->symdb.findVariableType(scope, typeToken);
        if (recordType) {
            const_cast<Token*>(typeToken)->type(recordType);
            return recordType;
        }
    }
    return nullptr;
}

void clangimport::AstNode::addFullScopeNameTokens(TokenList &tokenList, const Scope *recordScope)
{
    if (!recordScope)
        return;
    std::list<const Scope *> scopes;
    while (recordScope && recordScope != tokenList.back()->scope() && !recordScope->isExecutable()) {
        scopes.push_front(recordScope);
        recordScope = recordScope->nestedIn;
    }
    for (const Scope *s: scopes) {
        if (!s->className.empty()) {
            addtoken(tokenList, s->className);
            addtoken(tokenList, "::");
        }
    }
}

const Scope *clangimport::AstNode::getNestedInScope(TokenList &tokenList)
{
    if (!tokenList.back())
        return &mData->mSymbolDatabase.scopeList.front();
    if (tokenList.back()->str() == "}" && mData->mNotScope.find(tokenList.back()) == mData->mNotScope.end())
        return tokenList.back()->scope()->nestedIn;
    return tokenList.back()->scope();
}

void clangimport::AstNode::setValueType(Token *tok)
{
    for (int i = 0; i < 2; i++) {
        const std::string &type = getType(i);

        if (type.find('<') != std::string::npos)
            // TODO
            continue;

        TokenList decl(mData->mSettings, tok->isCpp() ? Standards::Language::CPP : Standards::Language::C);
        addTypeTokens(decl, type, tok->scope());
        if (!decl.front())
            break;

        ValueType valueType = ValueType::parseDecl(decl.front(), mData->mSettings);
        if (valueType.type != ValueType::Type::UNKNOWN_TYPE) {
            tok->setValueType(new ValueType(std::move(valueType)));
            break;
        }
    }
}

Scope *clangimport::AstNode::createScope(TokenList &tokenList, ScopeType scopeType, AstNodePtr astNode, const Token *def)
{
    std::vector<AstNodePtr> children2{std::move(astNode)};
    return createScope(tokenList, scopeType, children2, def);
}

Scope *clangimport::AstNode::createScope(TokenList &tokenList, ScopeType scopeType, const std::vector<AstNodePtr> & children2, const Token *def)
{
    SymbolDatabase &symbolDatabase = mData->mSymbolDatabase;

    auto *nestedIn = const_cast<Scope *>(getNestedInScope(tokenList));

    symbolDatabase.scopeList.emplace_back(nestedIn->symdb, nullptr, nestedIn);
    Scope *scope = &symbolDatabase.scopeList.back();
    if (scopeType == ScopeType::eEnum)
        scope->enumeratorList.reserve(children2.size());
    nestedIn->nestedList.push_back(scope);
    scope->type = scopeType;
    scope->classDef = def; // TODO: pass into ctor
    if (Token::Match(def, "if|for|while (")) {
        std::map<const Variable *, const Variable *> replaceVar;
        for (const Token *vartok = def->tokAt(2); vartok; vartok = vartok->next()) {
            if (!vartok->variable())
                continue;
            if (vartok->variable()->nameToken() == vartok) {
                const Variable *from = vartok->variable();
                scope->varlist.emplace_back(*from, scope);
                Variable *to = &scope->varlist.back();
                replaceVar[from] = to;
                mData->replaceVarDecl(from, to);
            }
            if (replaceVar.find(vartok->variable()) != replaceVar.end())
                const_cast<Token *>(vartok)->variable(replaceVar[vartok->variable()]);
        }
        std::list<Variable> &varlist = const_cast<Scope *>(def->scope())->varlist;
        for (auto var = varlist.cbegin(); var != varlist.cend();) {
            if (replaceVar.find(&(*var)) != replaceVar.end())
                var = varlist.erase(var);
            else
                ++var;
        }
    }
    scope->bodyStart = addtoken(tokenList, "{");
    tokenList.back()->scope(scope);
    mData->scopeAccessControl[scope] = scope->defaultAccess();
    if (!children2.empty()) {
        for (const AstNodePtr &astNode: children2) {
            if (astNode->nodeType == "VisibilityAttr")
                continue;
            if (astNode->nodeType == AccessSpecDecl) {
                if (contains(astNode->mExtTokens, "private"))
                    mData->scopeAccessControl[scope] = AccessControl::Private;
                else if (contains(astNode->mExtTokens, "protected"))
                    mData->scopeAccessControl[scope] = AccessControl::Protected;
                else if (contains(astNode->mExtTokens, "public"))
                    mData->scopeAccessControl[scope] = AccessControl::Public;
                continue;
            }
            astNode->createTokens(tokenList);
            if (scopeType == ScopeType::eEnum)
                astNode->addtoken(tokenList, ",");
            else if (!Token::Match(tokenList.back(), "[;{}]"))
                astNode->addtoken(tokenList, ";");
        }
    }
    scope->bodyEnd = addtoken(tokenList, "}");
    Token::createMutualLinks(const_cast<Token*>(scope->bodyStart), const_cast<Token*>(scope->bodyEnd));
    mData->scopeAccessControl.erase(scope);
    return scope;
}

Token *clangimport::AstNode::createTokens(TokenList &tokenList)
{
    if (nodeType == ArraySubscriptExpr) {
        Token *array = getChild(0)->createTokens(tokenList);
        Token *bracket1 = addtoken(tokenList, "[");
        Token *index = children[1]->createTokens(tokenList);
        Token *bracket2 = addtoken(tokenList, "]");
        bracket1->astOperand1(array);
        bracket1->astOperand2(index);
        bracket1->link(bracket2);
        bracket2->link(bracket1);
        return bracket1;
    }
    if (nodeType == BinaryOperator) {
        Token *tok1 = getChild(0)->createTokens(tokenList);
        Token *binop = addtoken(tokenList, unquote(mExtTokens.back()));
        Token *tok2 = children[1]->createTokens(tokenList);
        binop->astOperand1(tok1);
        binop->astOperand2(tok2);
        return binop;
    }
    if (nodeType == BreakStmt)
        return addtoken(tokenList, "break");
    if (nodeType == CharacterLiteral) {
        const int c = static_cast<int>(MathLib::toBigNumber(mExtTokens.back()));
        if (c == 0)
            return addtoken(tokenList, "\'\\0\'");
        if (c == '\r')
            return addtoken(tokenList, "\'\\r\'");
        if (c == '\n')
            return addtoken(tokenList, "\'\\n\'");
        if (c == '\t')
            return addtoken(tokenList, "\'\\t\'");
        if (c == '\\')
            return addtoken(tokenList, "\'\\\\\'");
        if (c < ' ' || c >= 0x80) {
            std::ostringstream hex;
            hex << std::hex << ((c>>4) & 0xf) << (c&0xf);
            return addtoken(tokenList, "\'\\x" + hex.str() + "\'");
        }
        return addtoken(tokenList, std::string("\'") + char(c) + std::string("\'"));
    }
    if (nodeType == CallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CaseStmt) {
        Token *caseToken = addtoken(tokenList, "case");
        Token *exprToken = getChild(0)->createTokens(tokenList);
        caseToken->astOperand1(exprToken);
        addtoken(tokenList, ":");
        children.back()->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == ClassTemplateDecl) {
        for (const AstNodePtr& child: children) {
            if (child->nodeType == ClassTemplateSpecializationDecl)
                child->createTokens(tokenList);
        }
        return nullptr;
    }
    if (nodeType == ClassTemplateSpecializationDecl) {
        createTokensForCXXRecord(tokenList);
        return nullptr;
    }
    if (nodeType == ConditionalOperator) {
        Token *expr1 = getChild(0)->createTokens(tokenList);
        Token *tok1 = addtoken(tokenList, "?");
        Token *expr2 = children[1]->createTokens(tokenList);
        Token *tok2 = addtoken(tokenList, ":");
        Token *expr3 = children[2]->createTokens(tokenList);
        tok2->astOperand1(expr2);
        tok2->astOperand2(expr3);
        tok1->astOperand1(expr1);
        tok1->astOperand2(tok2);
        return tok1;
    }
    if (nodeType == CompoundAssignOperator) {
        Token *lhs = getChild(0)->createTokens(tokenList);
        Token *assign = addtoken(tokenList, getSpelling());
        Token *rhs = children[1]->createTokens(tokenList);
        assign->astOperand1(lhs);
        assign->astOperand2(rhs);
        return assign;
    }
    if (nodeType == CompoundStmt) {
        for (const AstNodePtr& child: children) {
            child->createTokens(tokenList);
            if (!Token::Match(tokenList.back(), "[;{}]"))
                child->addtoken(tokenList, ";");
        }
        return nullptr;
    }
    if (nodeType == ConstantExpr)
        return children.back()->createTokens(tokenList);
    if (nodeType == ContinueStmt)
        return addtoken(tokenList, "continue");
    if (nodeType == CStyleCastExpr) {
        Token *par1 = addtoken(tokenList, "(");
        addTypeTokens(tokenList, '\'' + getType() + '\'');
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(getChild(0)->createTokens(tokenList));
        return par1;
    }
    if (nodeType == CXXBindTemporaryExpr)
        return getChild(0)->createTokens(tokenList);
    if (nodeType == CXXBoolLiteralExpr) {
        addtoken(tokenList, mExtTokens.back());
        tokenList.back()->setValueType(new ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0));
        return tokenList.back();
    }
    if (nodeType == CXXConstructExpr) {
        if (!children.empty())
            return getChild(0)->createTokens(tokenList);
        addTypeTokens(tokenList, '\'' + getType() + '\'');
        Token *type = tokenList.back();
        Token *par1 = addtoken(tokenList, "(");
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(type);
        return par1;
    }
    if (nodeType == CXXConstructorDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXDeleteExpr) {
        addtoken(tokenList, "delete");
        getChild(0)->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == CXXDestructorDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXForRangeStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");
        AstNodePtr varDecl;
        if (children[6]->nodeType == DeclStmt)
            varDecl = getChild(6)->getChild(0);
        else
            varDecl = getChild(5)->getChild(0);
        varDecl->mExtTokens.pop_back();
        varDecl->children.clear();
        Token *expr1 = varDecl->createTokens(tokenList);
        Token *colon = addtoken(tokenList, ":");

        auto it = std::find_if(children.cbegin(), children.cbegin() + 2, [&](const AstNodePtr& c) {
            return c && c->nodeType == DeclStmt && c->getChild(0)->nodeType == VarDecl;
        });
        if (it == children.cbegin() + 2)
            throw InternalError(tokenList.back(), "Failed to import CXXForRangeStmt. Range?");
        AstNodePtr range = (*it)->getChild(0)->getChild(0);

        Token *expr2 = range->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");

        par1->link(par2);
        par2->link(par1);

        colon->astOperand1(expr1);
        colon->astOperand2(expr2);
        par1->astOperand1(forToken);
        par1->astOperand2(colon);

        createScope(tokenList, ScopeType::eFor, children.back(), forToken);
        return nullptr;
    }
    if (nodeType == CXXMethodDecl) {
        for (std::size_t i = 0; i+1 < mExtTokens.size(); ++i) {
            if (mExtTokens[i] == "prev" && !mData->hasDecl(mExtTokens[i+1]))
                return nullptr;
        }
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXMemberCallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CXXNewExpr) {
        Token *newtok = addtoken(tokenList, "new");
        if (children.size() == 1 && getChild(0)->nodeType == CXXConstructExpr) {
            newtok->astOperand1(getChild(0)->createTokens(tokenList));
            return newtok;
        }
        std::string type = getType();
        if (type.find('*') != std::string::npos)
            type = type.erase(type.rfind('*'));
        addTypeTokens(tokenList, type);
        if (!children.empty()) {
            Token *bracket1 = addtoken(tokenList, "[");
            getChild(0)->createTokens(tokenList);
            Token *bracket2 = addtoken(tokenList, "]");
            bracket1->link(bracket2);
            bracket2->link(bracket1);
        }
        return newtok;
    }
    if (nodeType == CXXNullPtrLiteralExpr)
        return addtoken(tokenList, "nullptr");
    if (nodeType == CXXOperatorCallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CXXRecordDecl) {
        createTokensForCXXRecord(tokenList);
        return nullptr;
    }
    if (nodeType == CXXStaticCastExpr || nodeType == CXXFunctionalCastExpr) {
        Token *cast = addtoken(tokenList, getSpelling());
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = getChild(0)->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(cast);
        par1->astOperand2(expr);
        setValueType(par1);
        return par1;
    }
    if (nodeType == CXXStdInitializerListExpr)
        return getChild(0)->createTokens(tokenList);
    if (nodeType == CXXTemporaryObjectExpr && !children.empty())
        return getChild(0)->createTokens(tokenList);
    if (nodeType == CXXThisExpr)
        return addtoken(tokenList, "this");
    if (nodeType == CXXThrowExpr) {
        Token *t = addtoken(tokenList, "throw");
        t->astOperand1(getChild(0)->createTokens(tokenList));
        return t;
    }
    if (nodeType == DeclRefExpr) {
        int addrIndex = mExtTokens.size() - 1;
        while (addrIndex > 1 && !startsWith(mExtTokens[addrIndex],"0x"))
            --addrIndex;
        const std::string addr = mExtTokens[addrIndex];
        std::string name = unquote(getSpelling());
        Token *reftok = addtoken(tokenList, name.empty() ? "<NoName>" : name);
        mData->ref(addr, reftok);
        return reftok;
    }
    if (nodeType == DeclStmt)
        return getChild(0)->createTokens(tokenList);
    if (nodeType == DefaultStmt) {
        addtoken(tokenList, "default");
        addtoken(tokenList, ":");
        children.back()->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == DoStmt) {
        addtoken(tokenList, "do");
        createScope(tokenList, ScopeType::eDo, getChild(0), tokenList.back());
        Token *tok1 = addtoken(tokenList, "while");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[1]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(expr);
        return nullptr;
    }
    if (nodeType == EnumConstantDecl) {
        Token *nameToken = addtoken(tokenList, getSpelling());
        auto *scope = const_cast<Scope *>(nameToken->scope());
        scope->enumeratorList.emplace_back(nameToken->scope());
        Enumerator *e = &scope->enumeratorList.back();
        e->name = nameToken;
        e->value = mData->enumValue++;
        e->value_known = true;
        mData->enumDecl(mExtTokens.front(), nameToken, e);
        return nameToken;
    }
    if (nodeType == EnumDecl) {
        int colIndex = mExtTokens.size() - 1;
        while (colIndex > 0 && !startsWith(mExtTokens[colIndex],"col:") && !startsWith(mExtTokens[colIndex],"line:"))
            --colIndex;
        if (colIndex == 0)
            return nullptr;

        mData->enumValue = 0;
        Token *enumtok = addtoken(tokenList, "enum");
        const Token *nametok = nullptr;
        {
            int nameIndex = mExtTokens.size() - 1;
            while (nameIndex > colIndex && mExtTokens[nameIndex][0] == '\'')
                --nameIndex;
            if (nameIndex > colIndex)
                nametok = addtoken(tokenList, mExtTokens[nameIndex]);
            if (mExtTokens.back()[0] == '\'') {
                addtoken(tokenList, ":");
                addTypeTokens(tokenList, mExtTokens.back());
            }
        }
        Scope *enumscope = createScope(tokenList, ScopeType::eEnum, children, enumtok);
        if (nametok)
            enumscope->className = nametok->str();
        if (enumscope->bodyEnd && Token::simpleMatch(enumscope->bodyEnd->previous(), ", }"))
            const_cast<Token *>(enumscope->bodyEnd)->deletePrevious();

        // Create enum type
        mData->mSymbolDatabase.typeList.emplace_back(enumtok, enumscope, enumtok->scope());
        enumscope->definedType = &mData->mSymbolDatabase.typeList.back();
        if (nametok)
            const_cast<Scope *>(enumtok->scope())->definedTypesMap[nametok->str()] = enumscope->definedType;

        return nullptr;
    }
    if (nodeType == ExprWithCleanups)
        return getChild(0)->createTokens(tokenList);
    if (nodeType == FieldDecl)
        return createTokensVarDecl(tokenList);
    if (nodeType == FloatingLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == ForStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr1 = getChild(0) ? children[0]->createTokens(tokenList) : nullptr;
        Token *sep1 = addtoken(tokenList, ";");
        Token *expr2 = children[2] ? children[2]->createTokens(tokenList) : nullptr;
        Token *sep2 = addtoken(tokenList, ";");
        Token *expr3 = children[3] ? children[3]->createTokens(tokenList) : nullptr;
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(forToken);
        par1->astOperand2(sep1);
        sep1->astOperand1(expr1);
        sep1->astOperand2(sep2);
        sep2->astOperand1(expr2);
        sep2->astOperand2(expr3);
        createScope(tokenList, ScopeType::eFor, children[4], forToken);
        return nullptr;
    }
    if (nodeType == FunctionDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == FunctionTemplateDecl) {
        bool first = true;
        for (const AstNodePtr& child: children) {
            if (child->nodeType == FunctionDecl) {
                if (!first)
                    child->createTokens(tokenList);
                first = false;
            }
        }
        return nullptr;
    }
    if (nodeType == GotoStmt) {
        addtoken(tokenList, "goto");
        addtoken(tokenList, unquote(mExtTokens[mExtTokens.size() - 2]));
        addtoken(tokenList, ";");
        return nullptr;
    }
    if (nodeType == IfStmt) {
        AstNodePtr cond;
        AstNodePtr thenCode;
        AstNodePtr elseCode;
        if (children.size() == 2) {
            cond = children[children.size() - 2];
            thenCode = children[children.size() - 1];
        } else {
            cond = children[children.size() - 3];
            thenCode = children[children.size() - 2];
            elseCode = children[children.size() - 1];
        }

        Token *iftok = addtoken(tokenList, "if");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(iftok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        createScope(tokenList, ScopeType::eIf, std::move(thenCode), iftok);
        if (elseCode) {
            elseCode->addtoken(tokenList, "else");
            createScope(tokenList, ScopeType::eElse, std::move(elseCode), tokenList.back());
        }
        return nullptr;
    }
    if (nodeType == ImplicitCastExpr) {
        Token *expr = getChild(0)->createTokens(tokenList);
        if (!expr->valueType() || contains(mExtTokens, "<ArrayToPointerDecay>"))
            setValueType(expr);
        return expr;
    }
    if (nodeType == InitListExpr) {
        const Scope *scope = tokenList.back()->scope();
        Token *start = addtoken(tokenList, "{");
        start->scope(scope);
        for (const AstNodePtr& child: children) {
            if (tokenList.back()->str() != "{")
                addtoken(tokenList, ",");
            child->createTokens(tokenList);
        }
        Token *end = addtoken(tokenList, "}");
        end->scope(scope);
        start->link(end);
        end->link(start);
        mData->mNotScope.insert(end);
        return start;
    }
    if (nodeType == IntegerLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == LabelStmt) {
        addtoken(tokenList, unquote(mExtTokens.back()));
        addtoken(tokenList, ":");
        for (const auto& child: children)
            child->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == LinkageSpecDecl)
        return nullptr;
    if (nodeType == MaterializeTemporaryExpr)
        return getChild(0)->createTokens(tokenList);
    if (nodeType == MemberExpr) {
        Token *s = getChild(0)->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        std::string memberName = getSpelling();
        if (startsWith(memberName, "->")) {
            dot->originalName("->");
            memberName = memberName.substr(2);
        } else if (startsWith(memberName, ".")) {
            memberName = memberName.substr(1);
        }
        if (memberName.empty())
            memberName = "<unknown>";
        Token *member = addtoken(tokenList, memberName);
        mData->ref(mExtTokens.back(), member);
        dot->astOperand1(s);
        dot->astOperand2(member);
        return dot;
    }
    if (nodeType == NamespaceDecl) {
        if (children.empty())
            return nullptr;
        const Token *defToken = addtoken(tokenList, "namespace");
        const std::string &s = mExtTokens[mExtTokens.size() - 2];
        const Token* nameToken = (startsWith(s, "col:") || startsWith(s, "line:")) ?
                                 addtoken(tokenList, mExtTokens.back()) : nullptr;
        Scope *scope = createScope(tokenList, ScopeType::eNamespace, children, defToken);
        if (nameToken)
            scope->className = nameToken->str();
        return nullptr;
    }
    if (nodeType == NullStmt)
        return addtoken(tokenList, ";");
    if (nodeType == ParenExpr) {
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = getChild(0)->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return expr;
    }
    if (nodeType == RecordDecl) {
        const Token *classDef = addtoken(tokenList, "struct");
        const std::string &recordName = getSpelling();
        if (!recordName.empty())
            addtoken(tokenList, getSpelling());
        if (!isDefinition()) {
            addtoken(tokenList, ";");
            return nullptr;
        }

        Scope *recordScope = createScope(tokenList, ScopeType::eStruct, children, classDef);
        mData->mSymbolDatabase.typeList.emplace_back(classDef, recordScope, classDef->scope());
        recordScope->definedType = &mData->mSymbolDatabase.typeList.back();
        if (!recordName.empty()) {
            recordScope->className = recordName;
            const_cast<Scope *>(classDef->scope())->definedTypesMap[recordName] = recordScope->definedType;
        }

        return nullptr;
    }
    if (nodeType == ReturnStmt) {
        Token *tok1 = addtoken(tokenList, "return");
        if (!children.empty()) {
            getChild(0)->setValueType(tok1);
            tok1->astOperand1(getChild(0)->createTokens(tokenList));
        }
        return tok1;
    }
    if (nodeType == StringLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == SwitchStmt) {
        Token *tok1 = addtoken(tokenList, "switch");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[children.size() - 2]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(expr);
        createScope(tokenList, ScopeType::eSwitch, children.back(), tok1);
        return nullptr;
    }
    if (nodeType == TypedefDecl) {
        addtoken(tokenList, "typedef");
        addTypeTokens(tokenList, getType());
        return addtoken(tokenList, getSpelling());
    }
    if (nodeType == UnaryOperator) {
        int index = static_cast<int>(mExtTokens.size()) - 1;
        while (index > 0 && mExtTokens[index][0] != '\'')
            --index;
        Token *unop = addtoken(tokenList, unquote(mExtTokens[index]));
        unop->astOperand1(getChild(0)->createTokens(tokenList));
        return unop;
    }
    if (nodeType == UnaryExprOrTypeTraitExpr) {
        Token *tok1 = addtoken(tokenList, getSpelling());
        Token *par1 = addtoken(tokenList, "(");
        if (children.empty())
            addTypeTokens(tokenList, mExtTokens.back());
        else {
            AstNodePtr child = getChild(0);
            if (child && child->nodeType == ParenExpr)
                child = child->getChild(0);
            Token *expr = child->createTokens(tokenList);
            child->setValueType(expr);
            par1->astOperand2(expr);
        }
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(par1->next());
        setValueType(par1);
        return par1;
    }
    if (nodeType == VarDecl)
        return createTokensVarDecl(tokenList);
    if (nodeType == WhileStmt) {
        AstNodePtr cond = children[children.size() - 2];
        AstNodePtr body = children.back();
        Token *whiletok = addtoken(tokenList, "while");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(whiletok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        createScope(tokenList, ScopeType::eWhile, std::move(body), whiletok);
        return nullptr;
    }
    return addtoken(tokenList, "?" + nodeType + "?");
}

Token * clangimport::AstNode::createTokensCall(TokenList &tokenList)
{
    int firstParam;
    Token *f;
    if (nodeType == CXXOperatorCallExpr) {
        firstParam = 2;
        Token *obj = getChild(1)->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        Token *op = getChild(0)->createTokens(tokenList);
        dot->astOperand1(obj);
        dot->astOperand2(op);
        f = dot;
    } else {
        firstParam = 1;
        f = getChild(0)->createTokens(tokenList);
    }
    f->setValueType(nullptr);
    Token *par1 = addtoken(tokenList, "(");
    par1->astOperand1(f);
    std::size_t args = 0;
    while (args < children.size() && children[args]->nodeType != CXXDefaultArgExpr)
        args++;
    Token *child = nullptr;
    for (std::size_t c = firstParam; c < args; ++c) {
        if (child) {
            Token *comma = addtoken(tokenList, ",");
            comma->setValueType(nullptr);
            comma->astOperand1(child);
            comma->astOperand2(children[c]->createTokens(tokenList));
            child = comma;
        } else {
            child = children[c]->createTokens(tokenList);
        }
    }
    par1->astOperand2(child);
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);
    return par1;
}

void clangimport::AstNode::createTokensFunctionDecl(TokenList &tokenList)
{
    const bool prev = contains(mExtTokens, "prev");
    const bool hasBody = !children.empty() && children.back()->nodeType == CompoundStmt;
    const bool isStatic = contains(mExtTokens, "static");
    const bool isInline = contains(mExtTokens, "inline");

    const Token *startToken = nullptr;

    SymbolDatabase &symbolDatabase = mData->mSymbolDatabase;
    if (nodeType != CXXConstructorDecl && nodeType != CXXDestructorDecl) {
        if (isStatic)
            addtoken(tokenList, "static");
        if (isInline)
            addtoken(tokenList, "inline");
        const Token * const before = tokenList.back();
        addTypeTokens(tokenList, '\'' + getType() + '\'');
        startToken = before ? before->next() : tokenList.front();
    }

    if (mExtTokens.size() > 4 && mExtTokens[1] == "parent")
        addFullScopeNameTokens(tokenList, mData->getScope(mExtTokens[2]));

    Token *nameToken = addtoken(tokenList, getSpelling() + getTemplateParameters());
    auto *nestedIn = const_cast<Scope *>(nameToken->scope());

    if (prev) {
        const std::string addr = *(std::find(mExtTokens.cbegin(), mExtTokens.cend(), "prev") + 1);
        mData->ref(addr, nameToken);
    }
    if (!nameToken->function()) {
        nestedIn->functionList.emplace_back(nameToken, unquote(getFullType()));
        mData->funcDecl(mExtTokens.front(), nameToken, &nestedIn->functionList.back());
        if (nodeType == CXXConstructorDecl)
            nestedIn->functionList.back().type = FunctionType::eConstructor;
        else if (nodeType == CXXDestructorDecl)
            nestedIn->functionList.back().type = FunctionType::eDestructor;
        else
            nestedIn->functionList.back().retDef = startToken;
    }

    auto * const function = const_cast<Function*>(nameToken->function());

    if (!prev) {
        auto accessControl = mData->scopeAccessControl.find(tokenList.back()->scope());
        if (accessControl != mData->scopeAccessControl.end())
            function->access = accessControl->second;
    }

    Scope *scope = nullptr;
    if (hasBody) {
        symbolDatabase.scopeList.emplace_back(symbolDatabase, nullptr, nestedIn);
        scope = &symbolDatabase.scopeList.back();
        scope->function = function;
        scope->classDef = nameToken; // TODO: pass into ctor
        scope->type = ScopeType::eFunction;
        scope->className = nameToken->str();
        nestedIn->nestedList.push_back(scope);
        function->hasBody(true);
        function->functionScope = scope;
    }

    Token *par1 = addtoken(tokenList, "(");
    if (!function->arg)
        function->arg = par1;
    function->token = nameToken;
    if (!function->nestedIn)
        function->nestedIn = nestedIn;
    function->argDef = par1;
    // Function arguments
    for (int i = 0; i < children.size(); ++i) {
        AstNodePtr child = children[i];
        if (child->nodeType != ParmVarDecl)
            continue;
        if (tokenList.back() != par1)
            addtoken(tokenList, ",");
        const Type *recordType = addTypeTokens(tokenList, child->mExtTokens.back(), nestedIn);
        const Token *typeEndToken = tokenList.back();
        const std::string spelling = child->getSpelling();
        Token *vartok = nullptr;
        if (!spelling.empty())
            vartok = child->addtoken(tokenList, spelling);
        if (!prev) {
            function->argumentList.emplace_back(vartok, child->getType(), nullptr, typeEndToken, i, AccessControl::Argument, recordType, scope);
            if (vartok) {
                const std::string addr = child->mExtTokens[0];
                mData->varDecl(addr, vartok, &function->argumentList.back());
            }
        } else if (vartok) {
            const std::string addr = child->mExtTokens[0];
            mData->ref(addr, vartok);
        }
    }
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);

    if (function->isConst())
        addtoken(tokenList, "const");

    // Function body
    if (hasBody) {
        symbolDatabase.functionScopes.push_back(scope);
        Token *bodyStart = addtoken(tokenList, "{");
        bodyStart->scope(scope);
        children.back()->createTokens(tokenList);
        Token *bodyEnd = addtoken(tokenList, "}");
        scope->bodyStart = bodyStart;
        scope->bodyEnd = bodyEnd;
        bodyStart->link(bodyEnd);
        bodyEnd->link(bodyStart);
    } else {
        if (nodeType == CXXConstructorDecl && contains(mExtTokens, "default")) {
            addtoken(tokenList, "=");
            addtoken(tokenList, "default");
        }

        addtoken(tokenList, ";");
    }
}

void clangimport::AstNode::createTokensForCXXRecord(TokenList &tokenList)
{
    const bool isStruct = contains(mExtTokens, "struct");
    Token * const classToken = addtoken(tokenList, isStruct ? "struct" : "class");
    std::string className;
    if (mExtTokens[mExtTokens.size() - 2] == (isStruct?"struct":"class"))
        className = mExtTokens.back();
    else
        className = mExtTokens[mExtTokens.size() - 2];
    className += getTemplateParameters();
    /*Token *nameToken =*/ addtoken(tokenList, className);
    // base classes
    bool firstBase = true;
    for (const AstNodePtr &child: children) {
        if (child->nodeType == "public" || child->nodeType == "protected" || child->nodeType == "private") {
            addtoken(tokenList, firstBase ? ":" : ",");
            addtoken(tokenList, child->nodeType);
            addtoken(tokenList, unquote(child->mExtTokens.back()));
            firstBase = false;
        }
    }
    // definition
    if (isDefinition()) {
        std::vector<AstNodePtr> children2;
        std::copy_if(children.cbegin(), children.cend(), std::back_inserter(children2), [](const AstNodePtr& child) {
            return child->nodeType == CXXConstructorDecl ||
                   child->nodeType == CXXDestructorDecl ||
                   child->nodeType == CXXMethodDecl ||
                   child->nodeType == FieldDecl ||
                   child->nodeType == VarDecl ||
                   child->nodeType == AccessSpecDecl ||
                   child->nodeType == TypedefDecl;
        });
        Scope *scope = createScope(tokenList, isStruct ? ScopeType::eStruct : ScopeType::eClass, children2, classToken);
        const std::string addr = mExtTokens[0];
        mData->scopeDecl(addr, scope);
        scope->className = className;
        mData->mSymbolDatabase.typeList.emplace_back(classToken, scope, classToken->scope());
        scope->definedType = &mData->mSymbolDatabase.typeList.back();
        const_cast<Scope *>(classToken->scope())->definedTypesMap[className] = scope->definedType;
    }
    addtoken(tokenList, ";");
    tokenList.back()->scope(classToken->scope());
}

Token * clangimport::AstNode::createTokensVarDecl(TokenList &tokenList)
{
    const std::string addr = mExtTokens.front();
    if (contains(mExtTokens, "static"))
        addtoken(tokenList, "static");
    int typeIndex = mExtTokens.size() - 1;
    while (typeIndex > 1 && std::isalpha(mExtTokens[typeIndex][0]))
        typeIndex--;
    const std::string type = mExtTokens[typeIndex];
    const std::string name = mExtTokens[typeIndex - 1];
    const Token *startToken = tokenList.back();
    const ::Type *recordType = addTypeTokens(tokenList, type);
    if (!startToken)
        startToken = tokenList.front();
    else if (startToken->str() != "static")
        startToken = startToken->next();
    Token *vartok1 = addtoken(tokenList, name);
    auto *scope = const_cast<Scope *>(tokenList.back()->scope());
    scope->varlist.emplace_back(vartok1, unquote(type), startToken, vartok1->previous(), 0, scope->defaultAccess(), recordType, scope);
    mData->varDecl(addr, vartok1, &scope->varlist.back());
    if (mExtTokens.back() == "cinit" && !children.empty()) {
        Token *eq = addtoken(tokenList, "=");
        eq->astOperand1(vartok1);
        eq->astOperand2(children.back()->createTokens(tokenList));
        return eq;
    }
    if (mExtTokens.back() == "callinit") {
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(vartok1);
        par1->astOperand2(getChild(0)->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return par1;
    }
    if (mExtTokens.back() == "listinit") {
        return getChild(0)->createTokens(tokenList);
    }
    return vartok1;
}

static void setTypes(TokenList &tokenList)
{
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            for (Token *typeToken = tok->tokAt(2); typeToken->str() != ")"; typeToken = typeToken->next()) {
                if (typeToken->type())
                    continue;
                typeToken->type(typeToken->scope()->findType(typeToken->str()));
            }
        }
    }
}

static void setValues(const Tokenizer &tokenizer, const SymbolDatabase *symbolDatabase)
{
    const Settings & settings = tokenizer.getSettings();

    for (const Scope& scope : symbolDatabase->scopeList) {
        if (!scope.definedType)
            continue;

        MathLib::bigint typeSize = 0;
        for (const Variable &var: scope.varlist) {
            const MathLib::bigint mul = std::accumulate(var.dimensions().cbegin(), var.dimensions().cend(), MathLib::bigint(1), [](MathLib::bigint v, const Dimension& dim) {
                return v * dim.num;
            });
            if (var.valueType())
                typeSize += mul * var.valueType()->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
        }
        scope.definedType->sizeOf = typeSize;
    }

    for (auto *tok = const_cast<Token*>(tokenizer.tokens()); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            ValueType vt = ValueType::parseDecl(tok->tokAt(2), settings);
            const size_t sz = vt.getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
            if (sz == 0)
                continue;
            long long mul = 1;
            for (const Token *arrtok = tok->linkAt(1)->previous(); arrtok; arrtok = arrtok->previous()) {
                const std::string &a = arrtok->str();
                if (a.size() > 2 && a[0] == '[' && a.back() == ']')
                    mul *= strToInt<long long>(a.substr(1));
                else
                    break;
            }
            ValueFlow::Value v(mul * sz);
            v.setKnown();
            tok->next()->addValue(v);
        }
    }
}

void clangimport::parseClangAstDump(Tokenizer &tokenizer, std::istream &f)
{
    TokenList &tokenList = tokenizer.list;

    tokenizer.createSymbolDatabase();
    auto *symbolDatabase = const_cast<SymbolDatabase *>(tokenizer.getSymbolDatabase());
    symbolDatabase->scopeList.emplace_back(*symbolDatabase, nullptr, nullptr);
    symbolDatabase->scopeList.back().type = ScopeType::eGlobal;

    clangimport::Data data(tokenizer.getSettings(), *symbolDatabase);
    std::string line;
    std::vector<AstNodePtr> tree;
    while (std::getline(f,line)) {
        const std::string::size_type pos1 = line.find('-');
        if (pos1 == std::string::npos)
            continue;
        if (!tree.empty() && line.substr(pos1) == "-<<<NULL>>>") {
            const int level = (pos1 - 1) / 2;
            tree[level - 1]->children.push_back(nullptr);
            continue;
        }
        const std::string::size_type pos2 = line.find(' ', pos1);
        if (pos2 < pos1 + 4 || pos2 == std::string::npos)
            continue;
        const std::string nodeType = line.substr(pos1+1, pos2 - pos1 - 1);
        const std::string ext = line.substr(pos2);

        if (pos1 == 1 && endsWith(nodeType, "Decl")) {
            if (!tree.empty())
                tree[0]->createTokens1(tokenList);
            tree.clear();
            tree.push_back(std::make_shared<AstNode>(nodeType, ext, &data));
            continue;
        }

        const int level = (pos1 - 1) / 2;
        if (level == 0 || level > tree.size())
            continue;

        AstNodePtr newNode = std::make_shared<AstNode>(nodeType, ext, &data);
        tree[level - 1]->children.push_back(newNode);
        if (level >= tree.size())
            tree.push_back(std::move(newNode));
        else
            tree[level] = std::move(newNode);
    }

    if (!tree.empty())
        tree[0]->createTokens1(tokenList);

    // Validation
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "(|)|[|]|{|}") && !tok->link())
            throw InternalError(tok, "Token::link() is not set properly");
    }

    if (tokenList.front())
        tokenList.front()->assignIndexes();
    symbolDatabase->clangSetVariables(data.getVariableList());
    symbolDatabase->createSymbolDatabaseExprIds();
    tokenList.clangSetOrigFiles();
    setTypes(tokenList);
    setValues(tokenizer, symbolDatabase);
}

