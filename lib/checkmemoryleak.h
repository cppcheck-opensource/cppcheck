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
#ifndef checkmemoryleakH
#define checkmemoryleakH
//---------------------------------------------------------------------------

/**
 * @file
 *
 * %Check for memory leaks
 *
 * The checking is split up into three specialized classes.
 * - CheckMemoryLeakInFunction can detect when a function variable is allocated but not deallocated properly.
 * - CheckMemoryLeakInClass can detect when a class variable is allocated but not deallocated properly.
 * - CheckMemoryLeakStructMember checks allocation/deallocation of structs and struct members
 */

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <cstdint>
#include <list>
#include <string>

class Function;
class Scope;
class Settings;
class Token;
class Variable;
class ErrorLogger;
struct CWE;
class Tokenizer;
enum class Severity : std::uint8_t;

/// @addtogroup Core
/// @{


/**
 * @brief %CheckMemoryLeakInFunction detects when a function variable is allocated but not deallocated properly.
 *
 * The checking is done by looking at each function variable separately. By repeating these 4 steps over and over:
 * -# locate a function variable
 * -# create a simple token list that describes the usage of the function variable.
 * -# simplify the token list.
 * -# finally, check if the simplified token list contain any leaks.
 */

class CPPCHECKLIB CheckMemoryLeakInFunction : public Check {
    friend class TestMemleakInFunction;

public:
    /** @brief This constructor is used when registering this class */
    CheckMemoryLeakInFunction() : Check("Memory leaks (function variables)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** Report all possible errors (for the --errorlist) */
    void getErrorMessages(ErrorLogger *e, const Settings &settings) const override;

    /**
     * Get class information (--doc)
     * @return Wiki formatted information about this class
     */
    std::string classInfo() const override {
        return "Is there any allocated memory when a function goes out of scope\n";
    }
};


/**
 * @brief %Check class variables, variables that are allocated in the constructor should be deallocated in the destructor
 */

class CPPCHECKLIB CheckMemoryLeakInClass : public Check {
    friend class TestMemleakInClass;

public:
    CheckMemoryLeakInClass() : Check("Memory leaks (class variables)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *e, const Settings &settings) const override;

    std::string classInfo() const override {
        return "If the constructor allocate memory then the destructor must deallocate it.\n";
    }
};



/** @brief detect simple memory leaks for struct members */

class CPPCHECKLIB CheckMemoryLeakStructMember : public Check {
    friend class TestMemleakStructMember;

public:
    CheckMemoryLeakStructMember() : Check("Memory leaks (struct members)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger * errorLogger, const Settings & settings) const override;

    std::string classInfo() const override {
        return "Don't forget to deallocate struct members\n";
    }
};



/** @brief detect simple memory leaks (address not taken) */

class CPPCHECKLIB CheckMemoryLeakNoVar : public Check {
    friend class TestMemleakNoVar;

public:
    CheckMemoryLeakNoVar() : Check("Memory leaks (address not taken)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *e, const Settings &settings) const override;

    std::string classInfo() const override {
        return "Not taking the address to allocated memory\n";
    }
};

/** @brief Base class for memory leaks checking */
class CPPCHECKLIB CheckMemoryLeakImpl : public CheckImpl {
private:
    /**
     * Report error. Similar with the function Check::reportError
     * @param tok the token where the error occurs
     * @param severity the severity of the bug
     * @param id type of message
     * @param msg text
     * @param cwe cwe number
     */
    void reportErr(const Token *tok, Severity severity, const std::string &id, const std::string &msg, const CWE &cwe) const;

    /**
     * Report error. Similar with the function Check::reportError
     * @param callstack callstack of error
     * @param severity the severity of the bug
     * @param id type of message
     * @param msg text
     * @param cwe cwe number
     */
    void reportErr(const std::list<const Token *> &callstack, Severity severity, const std::string &id, const std::string &msg, const CWE &cwe) const;

public:
    CheckMemoryLeakImpl() = delete;
    CheckMemoryLeakImpl(const CheckMemoryLeakImpl &) = delete;
    CheckMemoryLeakImpl& operator=(const CheckMemoryLeakImpl &) = delete;

    CheckMemoryLeakImpl(const Tokenizer *t, const Settings &s, ErrorLogger *e)
        : CheckImpl(t, s, e) {}

    /** @brief What type of allocation are used.. the "Many" means that several types of allocation and deallocation are used */
    enum AllocType : std::uint8_t { No, Malloc, New, NewArray, File, Fd, Pipe, OtherMem, OtherRes, Many };

    void memoryLeak(const Token *tok, const std::string &varname, AllocType alloctype) const;

    /**
     * @brief Get type of deallocation at given position
     * @param tok position
     * @param varid variable id
     * @return type of deallocation
     */
    AllocType getDeallocationType(const Token *tok, nonneg int varid) const;

    /**
     * @brief Get type of allocation at given position
     */
    AllocType getAllocationType(const Token *tok2, nonneg int varid, std::list<const Function*> *callstack = nullptr) const;

    /**
     * @brief Get type of reallocation at given position
     */
    AllocType getReallocationType(const Token *tok2, nonneg int varid) const;

    /**
     * Check if token reopens a standard stream
     * @param tok token to check
     */
    bool isReopenStandardStream(const Token *tok) const;
    /**
     * Check if token opens /dev/null
     * @param tok token to check
     */
    bool isOpenDevNull(const Token *tok) const;
    /**
     * Report that there is a memory leak (new/malloc/etc)
     * @param tok token where memory is leaked
     * @param varname name of variable
     */
    void memleakError(const Token *tok, const std::string &varname) const;

    /**
     * Report that there is a resource leak (fopen/popen/etc)
     * @param tok token where resource is leaked
     * @param varname name of variable
     */
    void resourceLeakError(const Token *tok, const std::string &varname) const;

    void deallocuseError(const Token *tok, const std::string &varname) const;
    void mismatchAllocDealloc(const std::list<const Token *> &callstack, const std::string &varname) const;
    void memleakUponReallocFailureError(const Token *tok, const std::string &reallocfunction, const std::string &varname) const;

    /** What type of allocated memory does the given function return? */
    AllocType functionReturnType(const Function* func, std::list<const Function*> *callstack = nullptr) const;
};

/// @}



/// @addtogroup Checks
/// @{


/**
 * @brief %CheckMemoryLeakInFunction detects when a function variable is allocated but not deallocated properly.
 *
 * The checking is done by looking at each function variable separately. By repeating these 4 steps over and over:
 * -# locate a function variable
 * -# create a simple token list that describes the usage of the function variable.
 * -# simplify the token list.
 * -# finally, check if the simplified token list contain any leaks.
 */

class CPPCHECKLIB CheckMemoryLeakInFunctionImpl : public CheckMemoryLeakImpl {
public:
    /** @brief This constructor is used when running checks */
    CheckMemoryLeakInFunctionImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckMemoryLeakImpl(tokenizer, settings, errorLogger) {}

    /**
     * Checking for a memory leak caused by improper realloc usage.
     */
    void checkReallocUsage();
};



/**
 * @brief %Check class variables, variables that are allocated in the constructor should be deallocated in the destructor
 */

class CPPCHECKLIB CheckMemoryLeakInClassImpl : private CheckMemoryLeakImpl {
public:
    CheckMemoryLeakInClassImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckMemoryLeakImpl(tokenizer, settings, errorLogger) {}

    void check();

    void variable(const Scope *scope, const Token *tokVarname);

    /** Public functions: possible double-allocation */
    void checkPublicFunctions(const Scope *scope, const Token *classtok);
    void publicAllocationError(const Token *tok, const std::string &varname);

    void unsafeClassError(const Token *tok, const std::string &classname, const std::string &varname);
};



/** @brief detect simple memory leaks for struct members */

class CPPCHECKLIB CheckMemoryLeakStructMemberImpl : private CheckMemoryLeakImpl {
public:

    CheckMemoryLeakStructMemberImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckMemoryLeakImpl(tokenizer, settings, errorLogger) {}

    void check();

    /** Is local variable allocated with malloc? */
    bool isMalloc(const Variable *variable) const;

    void checkStructVariable(const Variable*  variable) const;
};



/** @brief detect simple memory leaks (address not taken) */

class CPPCHECKLIB CheckMemoryLeakNoVarImpl : private CheckMemoryLeakImpl {
public:
    CheckMemoryLeakNoVarImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckMemoryLeakImpl(tokenizer, settings, errorLogger) {}

    void check();

    /**
     * @brief %Check if an input argument to a function is the return value of an allocation function
     * like malloc(), and the function does not release it.
     * @param scope     The scope of the function to check.
     */
    void checkForUnreleasedInputArgument(const Scope *scope);

    /**
     * @brief %Check if a call to an allocation function like malloc() is made and its return value is not assigned.
     * @param scope     The scope of the function to check.
     */
    void checkForUnusedReturnValue(const Scope *scope);

    /**
     * @brief %Check if an exception could cause a leak in an argument constructed with shared_ptr/unique_ptr.
     * @param scope     The scope of the function to check.
     */
    void checkForUnsafeArgAlloc(const Scope *scope);

    void functionCallLeak(const Token *loc, const std::string &alloc, const std::string &functionCall);
    void returnValueNotUsedError(const Token* tok, const std::string &alloc);
    void unsafeArgAllocError(const Token *tok, const std::string &funcName, const std::string &ptrType, const std::string &objType);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkmemoryleakH
