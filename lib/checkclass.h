/* -*- C++ -*-
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
#ifndef checkclassH
#define checkclassH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class ErrorLogger;
class Settings;
class Tokenizer;
class Token;
class Scope;
class SymbolDatabase;
class Function;
class Variable;
class Type;
enum class FunctionType : std::uint8_t;

/// @addtogroup Checks
/// @{


/** @brief %Check classes. Uninitialized member variables, non-conforming operators, missing virtual destructor, etc */
class CPPCHECKLIB CheckClass : public Check {
    friend class TestClass;
    friend class TestConstructors;
    friend class TestUnusedPrivateFunction;

public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckClass() : Check(myName()) {}

    /** @brief Set of the STL types whose operator[] is not const */
    static const std::set<std::string> stl_containers_not_const;

private:
    /** @brief This constructor is used when running checks. */
    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);

    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief %Check that all class constructors are ok */
    void constructors();

    /** @brief %Check that constructors with single parameter are explicit,
     *  if they has to be.*/
    void checkExplicitConstructors();

    /** @brief %Check that all private functions are called */
    void privateFunctions();

    /**
     * @brief %Check that the memsets are valid.
     * The 'memset' function can do dangerous things if used wrong. If it
     * is used on STL containers for instance it will clear all its data
     * and then the STL container may leak memory or worse have an invalid state.
     * It can also overwrite the virtual table.
     * Important: The checking doesn't work on simplified tokens list.
     */
    void checkMemset();
    void checkMemsetType(const Scope *start, const Token *tok, const Scope *type, bool allocation, std::set<const Scope *> parsedTypes);

    /** @brief 'operator=' should return reference to *this */
    void operatorEqRetRefThis();    // Warning upon no "return *this;"

    /** @brief 'operator=' should check for assignment to self */
    void operatorEqToSelf();    // Warning upon no check for assignment to self

    /** @brief The destructor in a base class should be virtual */
    void virtualDestructor();

    /** @brief warn for "this-x". The indented code may be "this->x"  */
    void thisSubtraction();

    /** @brief can member function be const? */
    void checkConst();

    /** @brief Check initializer list order */
    void initializerListOrder();

    /** @brief Suggest using initialization list */
    void initializationListUsage();

    /** @brief Check for initialization of a member with itself */
    void checkSelfInitialization();

    void copyconstructors();

    /** @brief call of virtual function in constructor/destructor */
    void checkVirtualFunctionCallInConstructor();

    /** @brief Check duplicated inherited members */
    void checkDuplInheritedMembers();

    /** @brief Check that copy constructor and operator defined together */
    void checkCopyCtorAndEqOperator();

    /** @brief Check that the override keyword is used when overriding virtual functions */
    void checkOverride();

    /** @brief Check that the overridden function is not identical to the base function */
    void checkUselessOverride();

    /** @brief Check that large members are returned by reference from getter function */
    void checkReturnByReference();

    /** @brief When "self pointer" is destroyed, 'this' might become invalid. */
    void checkThisUseAfterFree();

    /** @brief Unsafe class check - const reference member */
    void checkUnsafeClassRefMember();

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer &tokenizer, const Settings& /*settings*/, const std::string& currentConfig) const override;

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    const SymbolDatabase* mSymbolDatabase{};

    // Reporting errors..
    void noConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    void noExplicitConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    //void copyConstructorMallocError(const Token *cctor, const Token *alloc, const std::string& var_name);
    void copyConstructorShallowCopyError(const Token *tok, const std::string& varname);
    void noCopyConstructorError(const Scope *scope, bool isdefault, const Token *alloc, bool inconclusive);
    void noOperatorEqError(const Scope *scope, bool isdefault, const Token *alloc, bool inconclusive);
    void noDestructorError(const Scope *scope, bool isdefault, const Token *alloc);
    void uninitVarError(const Token *tok, bool isprivate, FunctionType functionType, const std::string &classname, const std::string &varname, bool derived, bool inconclusive);
    void uninitVarError(const Token *tok, const std::string &classname, const std::string &varname);
    void missingMemberCopyError(const Token *tok, FunctionType functionType, const std::string& classname, const std::string& varname);
    void operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive);
    void unusedPrivateFunctionError(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname);
    void memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type, bool isContainer = false);
    void memsetErrorReference(const Token *tok, const std::string &memfunc, const std::string &type);
    void memsetErrorFloat(const Token *tok, const std::string &type);
    void mallocOnClassError(const Token* tok, const std::string &memfunc, const Token* classTok, const std::string &classname);
    void mallocOnClassWarning(const Token* tok, const std::string &memfunc, const Token* classTok);
    void virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived, bool inconclusive);
    void thisSubtractionError(const Token *tok);
    void operatorEqRetRefThisError(const Token *tok);
    void operatorEqShouldBeLeftUnimplementedError(const Token *tok);
    void operatorEqMissingReturnStatementError(const Token *tok, bool error);
    void operatorEqToSelfError(const Token *tok);
    void checkConstError(const Token *tok, const std::string &classname, const std::string &funcname, bool suggestStatic, bool foundAllBaseClasses = true);
    void checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname, bool suggestStatic, bool foundAllBaseClasses = true);
    void initializerListError(const Token *tok1,const Token *tok2, const std::string & classname, const std::string &varname, const std::string& argname = {});
    void suggestInitializationList(const Token *tok, const std::string& varname);
    void selfInitializationError(const Token* tok, const std::string& varname);
    void pureVirtualFunctionCallInConstructorError(const Function * scopeFunction, const std::list<const Token *> & tokStack, const std::string &purefuncname);
    void virtualFunctionCallInConstructorError(const Function * scopeFunction, const std::list<const Token *> & tokStack, const std::string &funcname);
    void duplInheritedMembersError(const Token* tok1, const Token* tok2, const std::string &derivedName, const std::string &baseName, const std::string &memberName, bool derivedIsStruct, bool baseIsStruct, bool isFunction = false);
    void copyCtorAndEqOperatorError(const Token *tok, const std::string &classname, bool isStruct, bool hasCopyCtor);
    void overrideError(const Function *funcInBase, const Function *funcInDerived);
    void uselessOverrideError(const Function *funcInBase, const Function *funcInDerived, bool isSameCode = false);
    void returnByReferenceError(const Function *func, const Variable* var);
    void thisUseAfterFree(const Token *self, const Token *free, const Token *use);
    void unsafeClassRefMemberError(const Token *tok, const std::string &varname);
    void checkDuplInheritedMembersRecursive(const Type* typeCurrent, const Type* typeBase);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string myName() {
        return "Class";
    }

    std::string classInfo() const override {
        return "Check the code for each class.\n"
               "- Missing constructors and copy constructors\n"
               //"- Missing allocation of memory in copy constructor\n"
               "- Constructors which should be explicit\n"
               "- Are all variables initialized by the constructors?\n"
               "- Are all variables assigned by 'operator='?\n"
               "- Warn if memset, memcpy etc are used on a class\n"
               "- Warn if memory for classes is allocated with malloc()\n"
               "- If it's a base class, check that the destructor is virtual\n"
               "- Are there unused private functions?\n"
               "- 'operator=' should check for assignment to self\n"
               "- Constness for member functions\n"
               "- Order of initializations\n"
               "- Suggest usage of initialization list\n"
               "- Initialization of a member with itself\n"
               "- Suspicious subtraction from 'this'\n"
               "- Call of pure virtual function in constructor/destructor\n"
               "- Duplicated inherited data members\n"
               // disabled for now "- If 'copy constructor' defined, 'operator=' also should be defined and vice versa\n"
               "- Check that arbitrary usage of public interface does not result in division by zero\n"
               "- Delete \"self pointer\" and then access 'this'\n"
               "- Check that the 'override' keyword is used when overriding virtual functions\n"
               "- Check that the 'one definition rule' is not violated\n";
    }

    // operatorEqRetRefThis helper functions
    void checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last);
    void checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last, std::set<const Function*>& analyzedFunctions);

    // operatorEqToSelf helper functions
    bool hasAllocation(const Function *func, const Scope* scope) const;
    bool hasAllocation(const Function *func, const Scope* scope, const Token *start, const Token *end) const;
    bool hasAllocationInIfScope(const Function *func, const Scope* scope, const Token *ifStatementScopeStart) const;
    static bool hasAssignSelf(const Function *func, const Token *rhs, const Token *&out_ifStatementScopeStart);
    enum class Bool : std::uint8_t { True, False, Bailout };
    static Bool isInverted(const Token *tok, const Token *rhs);
    static const Token * getIfStmtBodyStart(const Token *tok, const Token *rhs);

    // checkConst helper functions
    bool isMemberVar(const Scope *scope, const Token *tok) const;
    static bool isMemberFunc(const Scope *scope, const Token *tok);
    static bool isConstMemberFunc(const Scope *scope, const Token *tok);
    enum class MemberAccess : std::uint8_t { NONE, SELF, MEMBER };
    bool checkConstFunc(const Scope *scope, const Function *func, MemberAccess& memberAccessed) const;

    // constructors helper function
    /** @brief Information about a member variable. Used when checking for uninitialized variables */
    struct Usage {
        explicit Usage(const Variable *var) : var(var) {}

        /** Variable that this usage is for */
        const Variable *var;

        /** @brief has this variable been assigned? */
        bool assign{};

        /** @brief has this variable been initialized? */
        bool init{};
    };

    static bool isBaseClassMutableMemberFunc(const Token *tok, const Scope *scope);

    /**
     * @brief Create usage list that contains all scope members and also members
     * of base classes without constructors.
     * @param scope current class scope
     */
    static std::vector<Usage> createUsageList(const Scope *scope);

    /**
     * @brief assign a variable in the varlist
     * @param usageList reference to usage vector
     * @param varid id of variable to mark assigned
     */
    static void assignVar(std::vector<Usage> &usageList, nonneg int varid);

    /**
     * @brief assign a variable in the varlist
     * @param usageList reference to usage vector
     * @param vartok variable token
     */
    static void assignVar(std::vector<Usage> &usageList, const Token *vartok);

    /**
     * @brief initialize a variable in the varlist
     * @param usageList reference to usage vector
     * @param varid id of variable to mark initialized
     */
    static void initVar(std::vector<Usage> &usageList, nonneg int varid);

    /**
     * @brief set all variables in list assigned
     * @param usageList reference to usage vector
     */
    static void assignAllVar(std::vector<Usage> &usageList);

    /**
     * @brief set all variable in list assigned, if visible from given scope
     * @param usageList reference to usage vector
     * @param scope scope from which usages must be visible
     */
    static void assignAllVarsVisibleFromScope(std::vector<Usage> &usageList, const Scope *scope);

    /**
     * @brief set all variables in list not assigned and not initialized
     * @param usageList reference to usage vector
     */
    static void clearAllVar(std::vector<Usage> &usageList);

    /**
     * @brief parse a scope for a constructor or member function and set the "init" flags in the provided varlist
     * @param func reference to the function that should be checked
     * @param callstack the function doesn't look into recursive function calls.
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    void initializeVarList(const Function &func, std::list<const Function *> &callstack, const Scope *scope, std::vector<Usage> &usage) const;

    /**
     * @brief gives a list of tokens where virtual functions are called directly or indirectly
     * @param function function to be checked
     * @param virtualFunctionCallsMap map of results for already checked functions
     * @return list of tokens where pure virtual functions are called
     */
    const std::list<const Token *> & getVirtualFunctionCalls(
        const Function & function,
        std::map<const Function *, std::list<const Token *>> & virtualFunctionCallsMap);

    /**
     * @brief looks for the first virtual function call stack
     * @param virtualFunctionCallsMap map of results obtained from getVirtualFunctionCalls
     * @param callToken token where pure virtual function is called directly or indirectly
     * @param[in,out] pureFuncStack list to append the stack
     */
    static void getFirstVirtualFunctionCallStack(
        std::map<const Function *, std::list<const Token *>> & virtualFunctionCallsMap,
        const Token *callToken,
        std::list<const Token *> & pureFuncStack);

    static bool canNotCopy(const Scope *scope);

    static bool canNotMove(const Scope *scope);

    /**
     * @brief Helper for checkThisUseAfterFree
     */
    bool checkThisUseAfterFreeRecursive(const Scope *classScope, const Function *func, const Variable *selfPointer, std::set<const Function *> callstack, const Token *&freeToken);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkclassH
