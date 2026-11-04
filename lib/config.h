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

#ifndef configH
#define configH

#ifdef MAXTIME
#error "MAXTIME is no longer supported - please use command-line options --checks-max-time=, --template-max-time= and --typedef-max-time= instead"
#endif

#ifdef _WIN32
#  ifdef CPPCHECKLIB_EXPORT
#    define CPPCHECKLIB __declspec(dllexport)
#  elif defined(CPPCHECKLIB_IMPORT)
#    define CPPCHECKLIB __declspec(dllimport)
#  else
#    define CPPCHECKLIB
#  endif
#else
#  define CPPCHECKLIB
#endif

// compatibility macros
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef __has_include
#define __has_include(x) 0
#endif

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

// C++11 noreturn
#if __has_cpp_attribute (noreturn) \
    || (defined(__GNUC__) && (__GNUC__ >= 5)) \
    || defined(__clang__) \
    || defined(__CPPCHECK__)
#  define NORETURN [[noreturn]]
#elif defined(__GNUC__)
#  define NORETURN __attribute__((noreturn))
#else
#  define NORETURN
#endif

// fallthrough
#if __cplusplus >= 201703L && __has_cpp_attribute (fallthrough)
#  define FALLTHROUGH [[fallthrough]]
#elif defined(__clang__)
#  define FALLTHROUGH [[clang::fallthrough]]
#elif (defined(__GNUC__) && (__GNUC__ >= 7))
#  define FALLTHROUGH __attribute__((fallthrough))
#else
#  define FALLTHROUGH
#endif

// unused
#if __cplusplus >= 201703L && __has_cpp_attribute (maybe_unused)
#  define UNUSED [[maybe_unused]]
#elif defined(__GNUC__) \
    || defined(__clang__) \
    || defined(__CPPCHECK__)
#  define UNUSED __attribute__((unused))
#else
#  define UNUSED
#endif

// TODO: AppleClang versions do not align with Clang versions - add check for proper version
// warn_unused
#if __has_cpp_attribute (gnu::warn_unused) || \
    (defined(__clang__) && (__clang_major__ >= 15))
#  define WARN_UNUSED [[gnu::warn_unused]]
#else
#  define WARN_UNUSED
#endif

// deprecated
#if defined(__GNUC__) \
    || defined(__clang__) \
    || defined(__CPPCHECK__)
#  define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#  define DEPRECATED __declspec(deprecated)
#else
#  define DEPRECATED
#endif

// TODO: AppleClang versions do not align with Clang versions - add check for proper version
// returns_nonnull
#if __has_cpp_attribute (gnu::returns_nonnull)
#  define RET_NONNULL [[gnu::returns_nonnull]]
#elif (defined(__clang__) && ((__clang_major__ > 3) || ((__clang_major__ == 3) && (__clang_minor__ >= 7)))) \
    || (defined(__GNUC__) && (__GNUC__ >= 9))
#  define RET_NONNULL __attribute__((returns_nonnull))
#else
#  define RET_NONNULL
#endif

#define REQUIRES(msg, ...) class=typename std::enable_if<__VA_ARGS__::value>::type

// Use the nonneg macro when you want to assert that a variable/argument is not negative
#ifdef __CPPCHECK__
#define nonneg   __cppcheck_low__(0)
#elif defined(NONNEG)
// Enable non-negative values checking
// TODO : investigate using annotations/contracts for stronger value checking
#define nonneg   unsigned
#else
// Disable non-negative values checking
#define nonneg
#endif

#if __has_feature(address_sanitizer)
#define ASAN 1
#endif

#ifndef ASAN
#ifdef  __SANITIZE_ADDRESS__
#define ASAN 1
#else
#define ASAN 0
#endif
#endif

#if defined(_WIN32)
#define HAS_THREADING_MODEL_THREAD
#define STDCALL __stdcall
#elif ((defined(__GNUC__) || defined(__sun)) && !defined(__MINGW32__)) || defined(__CPPCHECK__)
#if !defined(DISALLOW_PROCESS_EXECUTOR)
#define HAS_THREADING_MODEL_FORK
#endif
#if !defined(DISALLOW_THREAD_EXECUTOR)
#define HAS_THREADING_MODEL_THREAD
#endif
#define STDCALL
#endif

#if !defined(HAS_THREADING_MODEL_FORK) && !defined(HAS_THREADING_MODEL_THREAD)
#error "No threading model defined"
#endif

#define STRINGISIZE(...) #__VA_ARGS__

#ifdef __clang__
#define SUPPRESS_WARNING_PUSH(warning) _Pragma("clang diagnostic push") _Pragma(STRINGISIZE(clang diagnostic ignored warning))
#define SUPPRESS_WARNING_POP _Pragma("clang diagnostic pop")
#define SUPPRESS_WARNING_GCC_PUSH(warning)
#define SUPPRESS_WARNING_GCC_POP
#define SUPPRESS_WARNING_CLANG_PUSH(warning) SUPPRESS_WARNING_PUSH(warning)
#define SUPPRESS_WARNING_CLANG_POP SUPPRESS_WARNING_POP
#define FORCE_WARNING_PUSH(warn) _Pragma("clang diagnostic push") _Pragma(STRINGISIZE(clang diagnostic warning warn))
#define FORCE_WARNING_CLANG_PUSH(warning) FORCE_WARNING_PUSH(warning)
#define FORCE_WARNING_CLANG_POP SUPPRESS_WARNING_POP
#elif defined(__GNUC__)
#define SUPPRESS_WARNING_PUSH(warning) _Pragma("GCC diagnostic push") _Pragma(STRINGISIZE(GCC diagnostic ignored warning))
#define SUPPRESS_WARNING_POP _Pragma("GCC diagnostic pop")
#define SUPPRESS_WARNING_GCC_PUSH(warning) SUPPRESS_WARNING_PUSH(warning)
#define SUPPRESS_WARNING_GCC_POP SUPPRESS_WARNING_POP
#define SUPPRESS_WARNING_CLANG_PUSH(warning)
#define SUPPRESS_WARNING_CLANG_POP
#define FORCE_WARNING_PUSH(warning)
#define FORCE_WARNING_CLANG_PUSH(warning)
#define FORCE_WARNING_CLANG_POP
#else
#define SUPPRESS_WARNING_PUSH(warning)
#define SUPPRESS_WARNING_POP
#define SUPPRESS_WARNING_GCC_PUSH(warning)
#define SUPPRESS_WARNING_GCC_POP
#define SUPPRESS_WARNING_CLANG_PUSH(warning)
#define SUPPRESS_WARNING_CLANG_POP
#define FORCE_WARNING_PUSH(warning)
#define FORCE_WARNING_CLANG_PUSH(warning)
#define FORCE_WARNING_CLANG_POP
#endif

#if !defined(NO_WINDOWS_SEH) && defined(_WIN32) && defined(_MSC_VER)
#define USE_WINDOWS_SEH
#endif

#if !defined(NO_UNIX_BACKTRACE_SUPPORT)
#if defined(HAVE_EXECINFO_H)
#if HAVE_EXECINFO_H
#define USE_UNIX_BACKTRACE_SUPPORT
#endif
#elif __has_include(<execinfo.h>)
#define USE_UNIX_BACKTRACE_SUPPORT
#endif
#endif

#if !defined(NO_UNIX_SIGNAL_HANDLING) && defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OS2__)
#define USE_UNIX_SIGNAL_HANDLING
#endif

#endif // configH
