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
#ifndef checkexceptionsafetyH
#define checkexceptionsafetyH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <string>

class Settings;
class ErrorLogger;
class Token;
class Tokenizer;

/// @addtogroup Checks
/// @{


/**
 * @brief %Check exception safety (exceptions shouldn't cause leaks nor corrupt data)
 *
 * The problem with these checks is that Cppcheck can't determine what the valid
 * values are for variables. But in some cases (dead pointers) it can be determined
 * that certain variable values are corrupt.
 */

class CPPCHECKLIB CheckExceptionSafety : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckExceptionSafety() : Check("Exception Safety") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger& errorLogger) override;

    /** Generate all possible errors (for --errorlist) */
    void getErrorMessages(ErrorLogger& errorLogger, const Settings &settings) const override;

    /** wiki formatted description of the class (for --doc) */
    std::string classInfo() const override {
        return "Checking exception safety\n"
               "- Throwing exceptions in destructors\n"
               "- Throwing exception during invalid state\n"
               "- Throwing a copy of a caught exception instead of rethrowing the original exception\n"
               "- Exception caught by value instead of by reference\n"
               "- Throwing exception in noexcept, nothrow(), __attribute__((nothrow)) or __declspec(nothrow) function\n"
               "- Unhandled exception specification when calling function foo()\n"
               "- Rethrow without currently handled exception\n";
    }
};

class CPPCHECKLIB CheckExceptionSafetyImpl : public CheckImpl {
public:
    /** This constructor is used when running checks. */
    CheckExceptionSafetyImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger &errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** Don't throw exceptions in destructors */
    void destructors();

    /** deallocating memory and then throw (dead pointer) */
    void deallocThrow();

    /** Don't rethrow a copy of the caught exception; use a bare throw instead */
    void checkRethrowCopy();

    /** @brief %Check for exceptions that are caught by value instead of by reference */
    void checkCatchExceptionByValue();

    /** @brief %Check for functions that throw that shouldn't */
    void nothrowThrows();

    /** @brief %Check for unhandled exception specification */
    void unhandledExceptionSpecification();

    /** @brief %Check for rethrow not from catch scope */
    void rethrowNoCurrentException();

    /** Don't throw exceptions in destructors */
    void destructorsError(const Token * tok, const std::string &className);
    void deallocThrowError(const Token * tok, const std::string &varname);
    void rethrowCopyError(const Token * tok, const std::string &varname);
    void catchExceptionByValueError(const Token *tok);
    void noexceptThrowError(const Token * tok);
    void entryPointThrowError(const Token * tok);
    /** Missing exception specification */
    void unhandledExceptionSpecificationError(const Token * tok1, const Token * tok2, const std::string & funcname);
    /** Rethrow without currently handled exception */
    void rethrowNoCurrentExceptionError(const Token *tok);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkexceptionsafetyH
