#!/usr/bin/env python3
#
# MISRA C 2012 checkers (including amendment 1 and 2)
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python misra.py --rule-texts=<path-to-rule-texts> main.cpp.dump
#
# Limitations: This addon is released as open source. We are not allowed by
# MISRA to distribute rule texts openly.
#
# The MISRA standard documents may be obtained from https://www.misra.org.uk
#
# Total number of rules: 143

from __future__ import print_function

import cppcheckdata
import itertools
import json
import sys
import re
import os
import argparse
import string
import copy

try:
    from itertools import izip as zip
except ImportError:
    pass

import misra_9

def grouped(iterable, n):
    """s -> (s0,s1,s2,...sn-1), (sn,sn+1,sn+2,...s2n-1), (s2n,s2n+1,s2n+2,...s3n-1), ..."""
    return zip(*[iter(iterable)] * n)


INT_TYPES = ['bool', 'char', 'short', 'int', 'long', 'long long']


STDINT_TYPES = ['%s%d_t' % (n, v) for n, v in itertools.product(
        ['int', 'uint', 'int_least', 'uint_least', 'int_fast', 'uint_fast'],
        [8, 16, 32, 64])]

STDINT_H_DEFINES_MIN = ['%s%d_MIN' % (n, v) for n, v in itertools.product(
        ['INT', 'INT_LEAST', 'INT_FAST',],
        [8, 16, 32, 64])]

STDINT_H_DEFINES_MAX = ['%s%d_MAX' % (n, v) for n, v in itertools.product(
        ['INT', 'UINT','INT_LEAST','UINT_LEAST', 'INT_FAST', 'UINT_FAST',],
        [8, 16, 32, 64])]

STDINT_H_DEFINES_C = ['%s%d_C' % (n, v) for n, v in itertools.product(
        ['INT', 'UINT'],
        [8, 16, 32, 64])]


INTTYPES_H_DEFINES = ['%s%d' % (n, v) for n, v in itertools.product(
        ['PRId', 'PRIi', 'PRIo', 'PRIu', 'PRIx', 'PRIX', 'SCNd',
         'SCNi', 'SCNo', 'SCNu', 'SCNx', 'PRIdLEAST', 'PRIiLEAST',
         'PRIoLEAST', 'PRIuLEAST', 'PRIxLEAST', 'PRIXLEAST',
         'SCNdLEAST', 'SCNiLEAST', 'SCNoLEAST', 'SCNuLEAST', 
         'SCNxLEAST', 'PRIdFAST', 'PRIiFAST', 'PRIoFAST', 'PRIuFAST', 
         'PRIxFAST', 'PRIXFAST', 'SCNdFAST', 'SCNiFAST', 'SCNoFAST', 
         'SCNuFAST', 'SCNxFAST', ],
        [8, 16, 32, 64])]
typeBits = {
    'CHAR': None,
    'SHORT': None,
    'INT': None,
    'LONG': None,
    'LONG_LONG': None,
    'POINTER': None
}


def isUnsignedType(ty):
    return ty == 'unsigned' or ty.startswith('uint')


def simpleMatch(token, pattern):
    return cppcheckdata.simpleMatch(token, pattern)


def rawlink(rawtoken):
    if rawtoken.str == '}':
        indent = 0
        while rawtoken:
            if rawtoken.str == '}':
                indent = indent + 1
            elif rawtoken.str == '{':
                indent = indent - 1
                if indent == 0:
                    break
            rawtoken = rawtoken.previous
    else:
        rawtoken = None
    return rawtoken


# Identifiers described in Section 7 "Library" of C90 Standard
# Based on ISO/IEC9899:1990 Annex D -- Library summary and
# Annex E -- Implementation limits.
C90_STDLIB_IDENTIFIERS = {
    # D.1 Errors
    'errno.h': ['EDOM', 'ERANGE', 'errno'],
    # D.2 Common definitions
    'stddef.h': ['NULL', 'offsetof', 'ptrdiff_t', 'size_t', 'wchar_t'],
    # D.3 Diagnostics
    'assert.h': ['NDEBUG', 'assert'],
    # D.4 Character handling
    'ctype.h': [
        'isalnum', 'isalpha', 'isblank', 'iscntrl', 'isdigit',
        'isgraph', 'islower', 'isprint', 'ispunct', 'isspace',
        'isupper', 'isxdigit', 'tolower', 'toupper',
    ],
    # D.5 Localization
    'locale.h': [
        'LC_ALL', 'LC_COLLATE', 'LC_CTYPE', 'LC_MONETARY',
        'LC_NUMERIC', 'LC_TIME', 'NULL', 'lconv',
        'setlocale', 'localeconv',
    ],
    # D.6 Mathematics
    'math.h': [
        'HUGE_VAL', 'acos', 'asin' , 'atan2', 'cos', 'sin', 'tan', 'cosh',
        'sinh', 'tanh', 'exp', 'frexp', 'ldexp', 'log', 'loglO', 'modf',
        'pow', 'sqrt', 'ceil', 'fabs', 'floor', 'fmod',
    ],
    # D.7 Nonlocal jumps
    'setjmp.h': ['jmp_buf', 'setjmp', 'longjmp'],
    # D.8 Signal handling
    'signal.h': [
        'sig_atomic_t', 'SIG_DFL', 'SIG_ERR', 'SIG_IGN', 'SIGABRT', 'SIGFPE',
        'SIGILL', 'SIGINT', 'SIGSEGV', 'SIGTERM', 'signal', 'raise',
    ],
    # D.9 Variable arguments
    'stdarg.h': ['va_list', 'va_start', 'va_arg', 'va_end'],
    # D.10 Input/output
    'stdio.h': [
        '_IOFBF', '_IOLBF', '_IONBF', 'BUFSIZ', 'EOF', 'FILE', 'FILENAME_MAX',
        'FOPEN_MAX', 'fpos_t', 'L_tmpnam', 'NULL', 'SEEK_CUR', 'SEEK_END',
        'SEEK_SET', 'size_t', 'stderr', 'stdin', 'stdout', 'TMP_MAX',
        'remove', 'rename', 'tmpfile', 'tmpnam', 'fclose', 'fflush', 'fopen',
        'freopen', 'setbuf', 'setvbuf', 'fprintf', 'fscanf', 'printf',
        'scanf', 'sprintf', 'sscanf', 'vfprintf', 'vprintf', 'vsprintf',
        'fgetc', 'fgets', 'fputc', 'fputs', 'getc', 'getchar', 'gets', 'putc',
        'putchar', 'puts', 'ungetc', 'fread', 'fwrite', 'fgetpos', 'fseek',
        'fsetpos', 'rewind', 'clearerr', 'feof', 'ferror', 'perror',
    ],
    # D.11 General utilities
    'stdlib.h': [
        'EXIT_FAILURE', 'EXIT_SUCCESS', 'MB_CUR_MAX', 'NULL', 'RAND_MAX',
        'div_t', 'ldiv_t', 'wchar_t', 'atof', 'atoi', 'strtod', 'rand',
        'srand', 'calloc', 'free', 'malloc', 'realloc', 'abort', 'atexit',
        'exit', 'getenv', 'system', 'bsearch', 'qsort', 'abs', 'div', 'ldiv',
        'mblen', 'mbtowc', 'wctomb', 'mbstowcs', 'wcstombs',
    ],
    # D.12 String handling
    'string.h': [
        'NULL', 'size_t', 'memcpy', 'memmove', 'strcpy', 'strncpy', 'strcat',
        'strncat', 'memcmp', 'strcmp', 'strcoll', 'strncmp', 'strxfrm',
        'memchr', 'strchr', 'strcspn', 'strpbrk', 'strrchr', 'strspn',
        'strstr', 'strtok', 'memset', 'strerror', 'strlen',
    ],
    # D.13 Date and time
    'time.h': [
        'CLK_TCK', 'NULL', 'clock_t', 'time_t', 'size_t', 'tm', 'clock',
        'difftime', 'mktime', 'time', 'asctime', 'ctime', 'gmtime',
        'localtime', 'strftime',
    ],
    # Annex E: Implementation limits
    'limits.h': [
        'CHAR_BIT', 'SCHAR_MIN', 'SCHAR_MAX', 'UCHAR_MAX', 'CHAR_MIN',
        'CHAR_MAX', 'MB_LEN_MAX', 'SHRT_MIN', 'SHRT_MAX', 'USHRT_MAX',
        'INT_MIN', 'INT_MAX', 'UINT_MAX', 'LONG_MIN', 'LONG_MAX', 'ULONG_MAX',
        ],
    'float.h': [
        'FLT_ROUNDS', 'FLT_RADIX', 'FLT_MANT_DIG', 'DBL_MANT_DIG',
        'LDBL_MANT_DIG', 'DECIMAL_DIG', 'FLT_DIG', 'DBL_DIG', 'LDBL_DIG',
        'DBL_MIN_EXP', 'LDBL_MIN_EXP', 'FLT_MIN_10_EXP', 'DBL_MIN_10_EXP',
        'LDBL_MIN_10_EXP', 'FLT_MAX_EXP', 'DBL_MAX_EXP', 'LDBL_MAX_EXP',
        'FLT_MAX_10_EXP', 'DBL_MAX_10_EXP', 'LDBL_MAX_10_EXP', 'FLT_MAX',
        'DBL_MAX', 'LDBL_MAX', 'FLT_MIN', 'DBL_MIN', 'LDBL_MIN',
        'FLT_EPSILON', 'DBL_EPSILON', 'LDBL_EPSILON'
    ],
}


# Identifiers described in Section 7 "Library" of C99 Standard
# Based on ISO/IEC 9899:1999 (E) Annex B -- Library summary
# (https://www.dii.uchile.cl/~daespino/files/Iso_C_1999_definition.pdf)
C99_STDLIB_IDENTIFIERS = {
    # B.1 Diagnostics
    'assert.h': C90_STDLIB_IDENTIFIERS['assert.h'],
    # B.2 Complex
    'complex.h': [
        'complex', 'imaginary', 'I', '_Complex_I', '_Imaginary_I',
        'CX_LIMITED_RANGE',
        'cacos', 'cacosf', 'cacosl',
        'casin', 'casinf', 'casinl',
        'catan', 'catanf', 'catanl',
        'ccos', 'ccosf', 'ccosl',
        'csin', 'csinf', 'csinl',
        'ctan', 'ctanf', 'ctanl',
        'cacosh', 'cacoshf', 'cacoshl',
        'casinh', 'casinhf', 'casinhl',
        'catanh', 'catanhf', 'catanhl',
        'ccosh', 'ccoshf', 'ccoshl',
        'csinh', 'csinhf', 'csinhl',
        'ctanh', 'ctanhf', 'ctanhl',
        'cexp', 'cexpf', 'cexpl',
        'clog', 'clogf', 'clogl',
        'cabs', 'cabsf', 'cabsl',
        'cpow', 'cpowf', 'cpowl',
        'csqrt', 'csqrtf', 'csqrtl',
        'carg', 'cargf', 'cargl',
        'cimag', 'cimagf', 'cimagl',
        'conj', 'conjf', 'conjl',
        'cproj', 'cprojf', 'cprojl',
        'creal', 'crealf', 'creall',
    ],
    # B.3 Character handling
    'ctype.h': C90_STDLIB_IDENTIFIERS['ctype.h'],
    # B.4 Errors
    'errno.h': C90_STDLIB_IDENTIFIERS['errno.h'] + ['EILSEQ'],
    # B.5 Floating-point environment
    'fenv.h': [
        'fenv_t', 'FE_OVERFLOW', 'FE_TOWARDZERO',
        'fexcept_t', 'FE_UNDERFLOW', 'FE_UPWARD',
        'FE_DIVBYZERO', 'FE_ALL_EXCEPT', 'FE_DFL_ENV',
        'FE_INEXACT', 'FE_DOWNWARD',
        'FE_INVALID', 'FE_TONEAREST',
        'FENV_ACCESS',
        'feclearexcept', 'fegetexceptflag', 'fegetround',
        'fesetround', 'fegetenv', 'feholdexcept',
        'fesetenv', 'feupdateenv',
    ],
    # B.6 Characteristics of floating types
    'float.h': C90_STDLIB_IDENTIFIERS['float.h'] + ['FLT_EVAL_METHOD'],
    # B.7 Format conversion of integer types
    'inttypes.h': [
        'PRIdMAX', 'PRIiMAX', 'PRIoMAX', 'PRIuMAX', 'PRIxMAX', 'PRIXMAX',
        'SCNdMAX', 'SCNiMAX', 'SCNoMAX', 'SCNuMAX', 'SCNxMAX', 'PRIdPTR', 
        'PRIiPTR', 'PRIoPTR', 'PRIuPTR', 'PRIxPTR', 'PRIXPTR', 'SCNdPTR', 
        'SCNiPTR', 'SCNoPTR', 'SCNuPTR', 'SCNxPTR', 
        'imaxdiv_t', 'imaxabs', 'imaxdiv', 'strtoimax',
        'strtoumax', 'wcstoimax', 'wcstoumax',
    ] + INTTYPES_H_DEFINES,
    # B.8 Alternative spellings
    'iso646.h': [
        'and', 'and_eq', 'bitand', 'bitor', 'compl', 'not', 'not_eq',
        'or', 'or_eq', 'xor', 'xor_eq',
    ],
    # B.9 Size of integer types
    'limits.h': C90_STDLIB_IDENTIFIERS['limits.h'] +
    ['LLONG_MIN', 'LLONG_MAX', 'ULLONG_MAX'],
    # B.10 Localization
    'locale.h': C90_STDLIB_IDENTIFIERS['locale.h'],
    # B.11 Mathematics
    'math.h': C90_STDLIB_IDENTIFIERS['math.h'] + [
        'float_t', 'double_t', 'HUGE_VAL', 'HUGE_VALF', 'HUGE_VALL',
        'INFINITY', 'NAN', 'FP_INFINITE', 'FP_NAN', 'FP_NORMAL',
        'FP_SUBNORMAL', 'FP_ZERO', 'FP_FAST_FMA', 'FP_FAST_FMAF',
        'FP_FAST_FMAL', 'FP_ILOGB0', 'FP_ILOGBNAN', 'MATH_ERRNO',
        'MATH_ERREXCEPT', 'math_errhandling', 'FP_CONTRACT', 'fpclassify',
        'isfinite', 'isinf', 'isnan', 'isnormal', 'signbit', 'acosf', 'acosl',
        'asinf', 'asinl', 'atanf', 'atanl', 'atan2', 'atan2f', 'atan2l',
        'cosf', 'cosl', 'sinf', 'sinl', 'tanf', 'tanl', 'acosh', 'acoshf',
        'acoshl', 'asinh', 'asinhf', 'asinhl', 'atanh', 'atanhf', 'atanhl',
        'cosh', 'coshf', 'coshl', 'sinh', 'sinhf', 'sinhl', 'tanh', 'tanhf',
        'tanhl', 'expf', 'expl', 'exp2', 'exp2f', 'exp2l', 'expm1', 'expm1f',
        'expm1l', 'frexpf', 'frexpl', 'ilogb', 'ilogbf', 'ilogbl', 'ldexpf',
        'ldexpl', 'logf', 'logl', 'log10f', 'log10l', 'log1p', 'log1pf',
        'log1pl', 'log2', 'log2f', 'log2l', 'logb', 'logbf', 'logbl', 'modff',
        'modfl', 'scalbn', 'scalbnf', 'scalbnl', 'scalbln', 'scalblnf',
        'scalblnl','cbrt', 'cbrtf','cbrtl', 'fabs', 'fabsf', 'fabsl',
        'hypotl', 'hypotf', 'powf', 'powl', 'sqrtf', 'sqrtl', 'erf', 'erff',
        'erfl', 'erfc', 'erfcf', 'erfcl', 'lgamma', 'lgammaf', 'lgammal',
        'tgamma', 'tgammaf', 'tgammal', 'ceilf', 'ceill', 'floorf', 'floorl',
        'nearbyint', 'nearbyintf', 'nearbyintl', 'rint', 'rintf', 'rintl',
        'lrint', 'lrintf', 'lrintl', 'llrint', 'llrintf', 'llrintl', 'round',
        'roundf', 'roundl', 'lround', 'lroundf', 'lroundl', 'llround',
        'llroundf', 'llroundl', 'trunc', 'truncf', 'truncl', 'fmodf', 'fmodl',
        'remainder', 'remainderf', 'remainderl', 'remquo', 'remquof',
        'remquol', 'copysign', 'copysignf', 'copysignl', 'nan', 'nanf',
        'nanl', 'nextafter', 'nextafterf', 'nextafterl', 'nexttoward',
        'nexttowardf', 'nexttowardl', 'fdim', 'fdimf', 'fdiml', 'fmax',
        'fmaxf', 'fmaxl', 'fmin', 'fminf', 'fminl', 'fmaf','fmal', 'isgreater',
        'isgreaterequal', 'isless', 'islessequal', 'islessgreater',
        'isunordered',
    ],
    # B.12 Nonlocal jumps
    'setjmp.h': C90_STDLIB_IDENTIFIERS['setjmp.h'],
    # B.13 Signal handling
    'signal.h': C90_STDLIB_IDENTIFIERS['signal.h'],
    # B.14 Variable arguments
    'stdarg.h': C90_STDLIB_IDENTIFIERS['stdarg.h'] + ['va_copy'],
    # B.15 Boolean type and values
    'stdbool.h': ['bool', 'true', 'false', '__bool_true_false_are_defined'],
    # B.16 Common definitions
    'stddef.h': C90_STDLIB_IDENTIFIERS['stddef.h'],
    # B.17 Integer types
    'stdint.h': [
        'intptr_t', 'uintptr_t', 'intmax_t', 'uintmax_t', 'INTN_MIN',
        'INTN_MAX', 'UINTN_MAX', 'INT_LEASTN_MIN', 'INT_LEASTN_MAX',
        'UINT_LEASTN_MAX', 'INT_FASTN_MIN', 'INT_FASTN_MAX', 'UINT_FASTN_MAX',
        'INTPTR_MIN', 'INTPTR_MAX', 'UINTPTR_MAX', 'INTMAX_MIN', 'INTMAX_MAX',
        'UINTMAX_MAX', 'PTRDIFF_MIN', 'PTRDIFF_MAX', 'SIG_ATOMIC_MIN',
        'SIG_ATOMIC_MAX', 'SIZE_MAX', 'WCHAR_MIN', 'WCHAR_MAX', 'WINT_MIN',
        'WINT_MAX', 'INTN_C', 'UINTN_C', 'INTMAX_C', 'UINTMAX_C',
    ] + STDINT_TYPES + STDINT_H_DEFINES_MIN + STDINT_H_DEFINES_MAX + STDINT_H_DEFINES_C,
    # B.18 Input/output
    'stdio.h': C90_STDLIB_IDENTIFIERS['stdio.h'] + [
        'mode', 'restrict', 'snprintf', 'vfscanf', 'vscanf',
        'vsnprintf', 'vsscanf','ftell'
    ],
    # B.19 General utilities
    'stdlib.h': C90_STDLIB_IDENTIFIERS['stdlib.h'] + [
        '_Exit', 'labs', 'llabs', 'lldiv', 'lldiv_t', 'strtof', 'strtol',
        'strtold', 'strtoll', 'strtoul', 'strtoull'
    ],
    # B.20 String handling
    'string.h': C90_STDLIB_IDENTIFIERS['string.h'],
    # B.21 Type-generic math
    'tgmath.h': [
        'acos', 'asin', 'atan', 'acosh', 'asinh', 'atanh', 'cos', 'sin', 'tan',
        'cosh', 'sinh', 'tanh', 'exp', 'log', 'pow', 'sqrt', 'fabs', 'atan2',
        'cbrt', 'ceil', 'copysign', 'erf', 'erfc', 'exp2', 'expm1', 'fdim',
        'floor', 'fma', 'fmax', 'fmin', 'fmod', 'frexp', 'hypot', 'ilogb',
        'ldexp', 'lgamma', 'llrint', 'llround', 'log10', 'log1p', 'log2',
        'logb', 'lrint', 'lround', 'nearbyint', 'nextafter', 'nexttoward',
        'remainder', 'remquo', 'rint', 'round', 'scalbn', 'scalbln', 'tgamma',
        'trunc', 'carg', 'cimag', 'conj', 'cproj', 'creal',
    ],
    # B.22 Date and time
    'time.h': C90_STDLIB_IDENTIFIERS['time.h'] + ['CLOCKS_PER_SEC'],
    # B.23 Extended multibyte/wide character utilities
    'wchar.h': [
        'wchar_t', 'size_t', 'mbstate_t', 'wint_t', 'tm', 'NULL', 'WCHAR_MAX',
        'WCHAR_MIN', 'WEOF', 'fwprintf', 'fwscanf', 'swprintf', 'swscanf',
        'vfwprintf', 'vfwscanf', 'vswprintf', 'vswscanf', 'vwprintf',
        'vwscanf', 'wprintf', 'wscanf', 'fgetwc', 'fgetws', 'fputwc', 'fputws',
        'fwide', 'getwc', 'getwchar', 'putwc', 'putwchar', 'ungetwc', 'wcstod',
        'wcstof', 'wcstold', 'wcstol', 'wcstoll', 'wcstoul', 'wcstoull', 'wcscpy', 
        'wcsncpy', 'wmemcpy', 'wmemmove', 'wcscat', 'wcsncat', 'wcscmp', 'wcscoll',
        'wcsncmp', 'wcsxfrm', 'wmemcmp', 'wcschr', 'wcscspn', 'wcspbrk',
        'wcsrchr', 'wcsspn', 'wcsstr', 'wcstok', 'wmemchr', 'wcslen',
        'wmemset', 'wcsftime', 'btowc', 'wctob', 'mbsinit', 'mbrlen',
        'mbrtowc', 'wcrtomb', 'mbsrtowcs', 'wcsrtombs',
    ],
    # B.24 Wide character classification and mapping utilities
    'wctype.h': ['wint_t', 'wctrans_t', 'wctype_t', 'WEOF',
        'iswalnum', 'iswalpha', 'iswblank', 'iswcntrl', 'iswdigit',
        'iswgraph', 'iswlower', 'iswprint', 'iswpunct', 'iswspace', 'iswupper',
        'iswxdigit', 'iswctype', 'wctype', 'towlower', 'towupper', 'towctrans',
        'wctrans'],
}

# Identifiers described in Section 7 "Library" of C11 Standard
# Based on ISO/IEC 9899:201x N1570 (Draft 12.04.2011) Annex B -- Library summary
# (https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
C11_STDLIB_IDENTIFIERS = {
    # B.1 Diagnostics
    'assert.h': C99_STDLIB_IDENTIFIERS['assert.h']+ ['static_assert'],
    # B.2 Complex
    'complex.h': C99_STDLIB_IDENTIFIERS['complex.h']+['__STDC_NO_COMPLEX__','CMPLX','CMPLXF','CMPLXL'],
    # B.3 Character handling
    'ctype.h': C99_STDLIB_IDENTIFIERS['ctype.h'],
    # B.4 Errors
    'errno.h': C99_STDLIB_IDENTIFIERS['errno.h']+['__STDC_WANT_LIB_EXT1__', 'errno_t'],
    # B.5 Floating-point environment
    'fenv.h': C99_STDLIB_IDENTIFIERS['fenv.h'],
    # B.6 Characteristics of floating types
    'float.h': C99_STDLIB_IDENTIFIERS['float.h']+[
        'FLT_HAS_SUBNORM','DBL_HAS_SUBNORM','LDBL_HAS_SUBNORM',
        'FLT_DECIMAL_DIG','DBL_DECIMAL_DIG','LDBL_DECIMAL_DIG',
        'FLT_TRUE_MIN','DBL_TRUE_MIN','LDBL_TRUE_MIN'],
    # B.7 Format conversion of integer types
    'inttypes.h': C99_STDLIB_IDENTIFIERS["inttypes.h"],
    # B.8 Alternative spellings
    'iso646.h': C99_STDLIB_IDENTIFIERS["iso646.h"],
    # B.9 Size of integer types
    'limits.h': C99_STDLIB_IDENTIFIERS['limits.h'],
    # B.10 Localization
    'locale.h': C99_STDLIB_IDENTIFIERS['locale.h'],
    # B.11 Mathematics
    'math.h': C99_STDLIB_IDENTIFIERS['math.h'],
    # B.12 Nonlocal jumps
    'setjmp.h': C99_STDLIB_IDENTIFIERS['setjmp.h'],
    # B.13 Signal handling
    'signal.h': C99_STDLIB_IDENTIFIERS['signal.h'],
    # B.14 Alignment
    'stdalign.h': ['alignas','__alignas_is_defined'],
    # B.15 Variable arguments
    'stdarg.h': C99_STDLIB_IDENTIFIERS['stdarg.h'],
    # B.16 Atomics
    'stdatomic.h': ['ATOMIC_BOOL_LOCK_FREE', 'ATOMIC_CHAR_LOCK_FREE',
        'ATOMIC_CHAR16_T_LOCK_FREE', 'ATOMIC_CHAR32_T_LOCK_FREE', 'ATOMIC_WCHAR_T_LOCK_FREE', 
        'ATOMIC_SHORT_LOCK_FREE', 'ATOMIC_INT_LOCK_FREE', 'ATOMIC_LONG_LOCK_FREE', 
        'ATOMIC_LLONG_LOCK_FREE', 'ATOMIC_POINTER_LOCK_FREE', 'ATOMIC_FLAG_INIT', 
        'memory_order', 'atomic_flag', 'memory_order_relaxed', 'memory_order_consume', 
        'memory_order_acquire', 'memory_order_release', 'memory_order_acq_rel', 'memory_order_seq_cst',
        'atomic_bool', 'atomic_char', 'atomic_schar', 'atomic_uchar', 'atomic_short', 'atomic_ushort', 
        'atomic_int', 'atomic_uint', 'atomic_long', 'atomic_ulong', 'atomic_llong', 'atomic_ullong', 
        'atomic_char16_t', 'atomic_char32_t', 'atomic_wchar_t', 'atomic_int_least8_t',
        'atomic_uint_least8_t', 'atomic_int_least16_t', 'atomic_uint_least16_t', 
        'atomic_int_least32_t', 'atomic_uint_least32_t', 'atomic_int_least64_t', 
        'atomic_uint_least64_t', 'atomic_int_fast8_t', 'atomic_uint_fast8_t', 
        'atomic_int_fast16_t', 'atomic_uint_fast16_t', 'atomic_int_fast32_t', 
        'atomic_uint_fast32_t', 'atomic_int_fast64_t', 'atomic_uint_fast64_t', 
        'atomic_intptr_t', 'atomic_uintptr_t', 'atomic_size_t', 'atomic_ptrdiff_t', 
        'atomic_intmax_t', 'atomic_uintmax_t', 'ATOMIC_VAR_INIT', 'type kill_dependency',
        'atomic_thread_fence', 'atomic_signal_fence', 'atomic_is_lock_free', 
        'atomic_store', 'atomic_store_explicit', 'atomic_load', 'atomic_load_explicit',
        'atomic_exchange', 'atomic_exchange_explicit', 'atomic_compare_exchange_strong',
        'atomic_compare_exchange_strong_explicit', 'atomic_compare_exchange_weak',
        'atomic_compare_exchange_weak_explicit', 'atomic_fetch_key', 'atomic_fetch_key_explicit', 
        'atomic_flag_test_and_set', 'atomic_flag_test_and_set_explicit',
        'atomic_flag_clear', 'atomic_flag_clear_explicit', ],
    # B.17 Boolean type and values
    'stdbool.h': C99_STDLIB_IDENTIFIERS['stdbool.h'],
    # B.18 Common definitions
    'stddef.h': C99_STDLIB_IDENTIFIERS['stddef.h'] +
        ['max_align_t','__STDC_WANT_LIB_EXT1__', 'rsize_t'],
    # B.19 Integer types
    'stdint.h': C99_STDLIB_IDENTIFIERS['stdint.h']+
        ['__STDC_WANT_LIB_EXT1__', 'RSIZE_MAX'],
    # B.20 Input/output
    'stdio.h': C99_STDLIB_IDENTIFIERS['stdio.h'] + 
        ['__STDC_WANT_LIB_EXT1__', 'L_tmpnam_s', 'TMP_MAX_S', 'errno_t', 'rsize_t',
        'tmpfile_s', 'tmpnam_s', 'fopen_s', 'freopen_s', 'fprintf_s', 'fscanf_s',
        'printf_s','scanf_s','snprintf_s','sprintf_s','sscanf_s','vfprintf_s',
        'vfscanf_s', 'vsprintf_s', 'vsscanf_s', 'gets_s'
        ],
    # B.21 General utilities
    'stdlib.h': C99_STDLIB_IDENTIFIERS['stdlib.h'] + 
    ['constraint_handler_t', 'set_constraint_handler_s', 'abort_handler_s',
     'ignore_handler_s', 'getenv_s', 'bsearch_s', 'qsort_s', 'wctomb_s',
     'mbstowcs_s', 'wcstombs_s'],
    # B.22 Noretrun
    'stdnoreturn.h': ['noreturn'],
    # B.23 String handling
    'string.h': C99_STDLIB_IDENTIFIERS['string.h'] + 
    ['memcpy_s', 'memmoce_s', 'strcpy_s', 'strncpy_s','strcat_s',
     'strtok_s', 'memset_s', 'strerror_s', 'strerrorlen_s', 'strnlen_s'],
    # B.24 Type-generic math
    'tgmath.h': C99_STDLIB_IDENTIFIERS['tgmath.h'],
    # B.25 Threads
    'threads.h': ['thread_local', 'ONCE_FLAG_INIT', 'TSS_DTOR_ITERATIONS',
        'cnd_t', 'thrd_t', 'tss_t', 'mtx_t', 'tss_dtor_t', 'thrd_start_t', 
        'once_flag', 'mtx_plain', 'mtx_recursive', 'mtx_timed', 'thrd_timedout',
        'thrd_success', 'thrd_busy', 'thrd_error', 'thrd_nomem', 'call_once',
        'cnd_broadcast', 'cnd_destroy','cnd_init', 'cnd_signal', 'cnd_timedwait',
        'cnd_wait','mtx_destroy', 'mtx_init', 'mtx_lock', 'mtx_timedlock',
        'mtx_trylock', 'mtx_unlock', 'thrd_create', 'thrd_current',
        'thrd_detach', 'thrd_equal', 'thrd_exit', 'thrd_join', 'thrd_sleep',
        'thrd_yield', 'tss_create', 'tss_delete', 'tss_get', 'tss_set' ],
    # B.26 Date and time
    'time.h': C99_STDLIB_IDENTIFIERS['time.h'] + [
        'asctime_s', 'ctime_s', 'gmtime_s', 'localtime_s'
        ],
    # B.27 Unicode utilities
    'uchar.h': ['mbstate_t', 'size_t', 'char16_t', 'char32_t',
        'mbrtoc16', 'c16rtomb', 'mbrtoc32', 'c32rtomb'
        ],
    # B.28 Extended multibyte/wide character utilities
    'wchar.h': C99_STDLIB_IDENTIFIERS["wchar.h"]+[
        'fwprintf_s', 'fwscanf_s', 'snwprintf_s', 'swprintf_s', 'swscanf_s', 
        'vfwprintf_s', 'vfwscanf_s', 'vsnwprintf_s', 'vswprintf_s', 'vswscanf_s', 
        'vwprintf_s', 'vwscanf_s', 'wprintf_s', 'wscanf_s', 'wcscpy_s', 'wcsncpy_s', 
        'wmemcpy_s', 'wmemmove_s', 'wcscat_s', 'wcsncat_s', 'wcstok_s', 'wcsnlen_s', 
        'wcrtomb_s', 'mbsrtowcs_s', 'wcsrtombs_s', 
    ],
    # B.29 Wide character classification and mapping utilities
    'wctype.h': C99_STDLIB_IDENTIFIERS['wctype.h'],
}

def getStdLib(standard):
    if standard == 'c89':
        return C90_STDLIB_IDENTIFIERS
    if standard == 'c99':
        return C99_STDLIB_IDENTIFIERS
    return C11_STDLIB_IDENTIFIERS

def isStdLibId(id_, standard='c99'):
    id_lists = getStdLib(standard).values()
    for l in id_lists:
        if id_ in l:
            return True
    return False

# Reserved keywords defined in ISO/IEC9899:1990 -- ch 6.1.1
C90_KEYWORDS = {
    'auto', 'break', 'case', 'char', 'const', 'continue', 'default', 'do',
    'double', 'else', 'enum', 'extern', 'float', 'for', 'goto', 'if',
    'int', 'long', 'register', 'return', 'short', 'signed',
    'sizeof', 'static', 'struct', 'switch', 'typedef', 'union', 'unsigned',
    'void', 'volatile', 'while'
}


# Reserved keywords defined in Section 6.4.1 "Language" of C99 Standard
# Based on ISO/IEC 9899:1999 (E) 6.4.1 Keywords
# Adding the expanding macros from Section 7 too
# (https://www.dii.uchile.cl/~daespino/files/Iso_C_1999_definition.pdf)
C99_ADDED_KEYWORDS = {
    'inline', 'restrict', '_Bool', '_Complex', '_Imaginary',
    'bool', 'complex', 'imaginary'
}

# Reserved keywords defined in Section 6.4.1 "Language" of C11 Standard
# Based on ISO/IEC 9899:201x N1570 (Draft 12.04.2011) 6.4.1 Keywords
# Adding the expanding macros from Section 7 too
# (https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
C11_ADDED_KEYWORDS = {
    '_Alignas', '_Alignof', '_Atomic', '_Generic', '_Noreturn',
    '_Static_assert', '_Thread_local' ,
    'alignas', 'alignof', 'noreturn', 'static_assert','thread_local'
}

def isKeyword(keyword, standard='c99'):
    kw_set = {}
    if standard == 'c89':
        kw_set = C90_KEYWORDS
    elif standard == 'c99':
        kw_set = copy.copy(C90_KEYWORDS)
        kw_set.update(C99_ADDED_KEYWORDS)
    else:
        kw_set = copy.copy(C90_KEYWORDS)
        kw_set.update(C99_ADDED_KEYWORDS)
        kw_set.update(C11_ADDED_KEYWORDS)
    return keyword in kw_set


def is_source_file(file):
    return file.endswith('.c')


def is_header(file):
    return file.endswith('.h')


def is_errno_setting_function(function_name):
    return function_name and \
           function_name in ('ftell', 'fgetpos', 'fsetpos', 'fgetwc', 'fputwc'
                             'strtoimax', 'strtoumax', 'strtol', 'strtoul',
                             'strtoll', 'strtoull', 'strtof', 'strtod', 'strtold'
                             'wcstoimax', 'wcstoumax', 'wcstol', 'wcstoul',
                             'wcstoll', 'wcstoull', 'wcstof', 'wcstod', 'wcstold'
                             'wcrtomb', 'wcsrtombs', 'mbrtowc')


def get_type_conversion_to_from(token):
    def get_vartok(expr):
        while expr:
            if isCast(expr):
                if expr.astOperand2 is None:
                    expr = expr.astOperand1
                else:
                    expr = expr.astOperand2
            elif expr.str in ('.', '::'):
                expr = expr.astOperand2
            elif expr.str == '[':
                expr = expr.astOperand1
            else:
                break
        return expr if (expr and expr.variable) else None

    if isCast(token):
        vartok = get_vartok(token)
        if vartok:
            return (token.next, vartok.variable.typeStartToken)

    elif token.str == '=':
        lhs = get_vartok(token.astOperand1)
        rhs = get_vartok(token.astOperand2)
        if lhs and rhs:
            return (lhs.variable.typeStartToken, rhs.variable.typeStartToken)

    return None


def is_composite_expr(expr, composite_operator=False):
    """MISRA C 2012, section 8.10.3"""
    if expr is None:
        return False

    if not composite_operator:
        if expr.str == '?' and simpleMatch(expr.astOperand2, ':'):
            colon = expr.astOperand2
            return is_composite_expr(colon.astOperand1,True) or is_composite_expr(colon.astOperand2, True)
        if (expr.str in ('+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":", '~')):
            return is_composite_expr(expr.astOperand1,True) or is_composite_expr(expr.astOperand2, True)
        return False

    # non constant expression?
    if expr.isNumber:
        return False
    if expr.astOperand1 or expr.astOperand2:
        return is_composite_expr(expr.astOperand1,True) or is_composite_expr(expr.astOperand2, True)
    return True


def getEssentialTypeCategory(expr):
    if not expr:
        return None
    if expr.str == ',':
        return getEssentialTypeCategory(expr.astOperand2)
    if expr.str in ('<', '<=', '==', '!=', '>=', '>', '&&', '||', '!'):
        return 'bool'
    if expr.str in ('<<', '>>'):
        # TODO this is incomplete
        return getEssentialTypeCategory(expr.astOperand1)
    if len(expr.str) == 1 and expr.str in '+-*/%&|^':
        # TODO this is incomplete
        e1 = getEssentialTypeCategory(expr.astOperand1)
        e2 = getEssentialTypeCategory(expr.astOperand2)
        # print('{0}: {1} {2}'.format(expr.str, e1, e2))
        if e1 and e2 and e1 == e2:
            return e1
        if expr.valueType:
            return expr.valueType.sign
    if expr.valueType and expr.valueType.typeScope and expr.valueType.typeScope.className:
        return "enum<" + expr.valueType.typeScope.className + ">"
    # Unwrap membership, dereferences and array indexing
    vartok = expr
    while True:
        if simpleMatch(vartok, '[') or (vartok and vartok.str == '*' and vartok.astOperand2 is None):
            vartok = vartok.astOperand1
        elif simpleMatch(vartok, '.'):
            vartok = vartok.astOperand2
        else:
            break
    if vartok and vartok.variable:
        typeToken = vartok.variable.typeStartToken
        while typeToken and typeToken.isName:
            if typeToken.str == 'char' and not typeToken.isSigned and not typeToken.isUnsigned:
                return 'char'
            if typeToken.valueType:
                if typeToken.valueType.type == 'bool':
                    return typeToken.valueType.type
                if typeToken.valueType.type in ('float', 'double', 'long double'):
                    return "float"
                if typeToken.valueType.sign:
                    return typeToken.valueType.sign
            typeToken = typeToken.next

    # See Appendix D, section D.6, Character constants
    if expr.str[0] == "'" and expr.str[-1] == "'":
        if len(expr.str) == 3 or (len(expr.str) == 4 and expr.str[1] == '\\'):
            return 'char'
        return expr.valueType.sign

    if (expr.isCast and expr.str == "("):
        castTok = expr.next
        while castTok.isName or castTok.str == "*":
            if castTok.str == 'char' and not castTok.isSigned and not castTok.isUnsigned:
                return 'char'
            castTok = castTok.next

    if expr.valueType:
        return expr.valueType.sign
    return None


def getEssentialCategorylist(operand1, operand2):
    if not operand1 or not operand2:
        return None, None
    if (operand1.str in ('++', '--') or
            operand2.str in ('++', '--')):
        return None, None
    if ((operand1.valueType and operand1.valueType.pointer) or
            (operand2.valueType and operand2.valueType.pointer)):
        return None, None
    e1 = getEssentialTypeCategory(operand1)
    e2 = getEssentialTypeCategory(operand2)
    return e1, e2


def get_essential_type_from_value(value, is_signed):
    if value is None:
        return None
    for t in ('char', 'short', 'int', 'long', 'long long'):
        bits = bitsOfEssentialType(t)
        if bits >= 64:
            continue
        if is_signed:
            range_min = -(1 << (bits - 1))
            range_max = (1 << (bits - 1)) - 1
        else:
            range_min = 0
            range_max = (1 << bits) - 1
        sign = 'signed' if is_signed else 'unsigned'
        if is_signed and value < 0 and value >= range_min:
            return '%s %s' % (sign, t)
        if value >= 0 and value <= range_max:
            return '%s %s' % (sign, t)
    return None

def getEssentialType(expr):
    if not expr:
        return None

    # See Appendix D, section D.6, Character constants
    if expr.str[0] == "'" and expr.str[-1] == "'":
        if len(expr.str) == 3 or (len(expr.str) == 4 and expr.str[1] == '\\'):
            return 'char'
        return '%s %s' % (expr.valueType.sign, expr.valueType.type)

    if expr.variable or isCast(expr):
        typeToken = expr.variable.typeStartToken if expr.variable else expr.next
        while typeToken and typeToken.isName:
            if typeToken.str == 'char' and not typeToken.isSigned and not typeToken.isUnsigned:
                return 'char'
            typeToken = typeToken.next
        if expr.valueType:
            if expr.valueType.type == 'bool':
                return 'bool'
            if expr.valueType.isFloat():
                return expr.valueType.type
            if expr.valueType.isIntegral():
                if (expr.valueType.sign is None) and expr.valueType.type == 'char':
                    return 'char'
                return '%s %s' % (expr.valueType.sign, expr.valueType.type)

    elif expr.isNumber:
        # Appendix D, D.6 The essential type of literal constants
        # Integer constants
        if expr.valueType.type == 'bool':
            return 'bool'
        if expr.valueType.isFloat():
            return expr.valueType.type
        if expr.valueType.isIntegral():
            if expr.valueType.type != 'int':
                return '%s %s' % (expr.valueType.sign, expr.valueType.type)
            return get_essential_type_from_value(expr.getKnownIntValue(), expr.valueType.sign == 'signed')

    elif expr.str in ('<', '<=', '>=', '>', '==', '!=', '&&', '||', '!'):
        return 'bool'

    elif expr.astOperand1 and expr.astOperand2 and expr.str in (
    '+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":"):
        if expr.astOperand1.valueType and expr.astOperand1.valueType.pointer > 0:
            return None
        if expr.astOperand2.valueType and expr.astOperand2.valueType.pointer > 0:
            return None
        e1 = getEssentialType(expr.astOperand1)
        e2 = getEssentialType(expr.astOperand2)
        if e1 is None or e2 is None:
            return None
        if is_constant_integer_expression(expr):
            sign1 = e1.split(' ')[0]
            sign2 = e2.split(' ')[0]
            if sign1 == sign2 and sign1 in ('signed', 'unsigned'):
                e = get_essential_type_from_value(expr.getKnownIntValue(), sign1 == 'signed')
                if e:
                    return e
        if bitsOfEssentialType(e2) >= bitsOfEssentialType(e1):
            return e2
        return e1

    elif expr.str == "~":
        e1 = getEssentialType(expr.astOperand1)
        return e1

    return None


def bitsOfEssentialType(ty):
    if ty is None:
        return 0
    last_type = ty.split(' ')[-1]
    if last_type == 'Boolean':
        return 1
    if last_type == 'char':
        return typeBits['CHAR']
    if last_type == 'short':
        return typeBits['SHORT']
    if last_type == 'int':
        return typeBits['INT']
    if ty.endswith('long long'):
        return typeBits['LONG_LONG']
    if last_type == 'long':
        return typeBits['LONG']
    for sty in STDINT_TYPES:
        if ty == sty:
            return int(''.join(filter(str.isdigit, sty)))
    return 0


def get_function_pointer_type(tok):
    ret = ''
    while tok and (tok.isName or tok.str == '*'):
        ret += ' ' + tok.str
        tok = tok.next
    if tok is None or tok.str != '(':
        return None
    tok = tok.link
    if not simpleMatch(tok, ') ('):
        return None
    ret += '('
    tok = tok.next.next
    while tok and (tok.str not in '()'):
        if tok.varId is None:
            ret += ' ' + tok.str
        tok = tok.next
    if (tok is None) or tok.str != ')':
        return None
    return ret[1:] + ')'

def isCast(expr):
    if not expr or expr.str != '(' or not expr.astOperand1 or expr.astOperand2:
        return False
    if simpleMatch(expr, '( )'):
        return False
    return True

def is_constant_integer_expression(expr):
    if expr is None:
        return False
    if expr.isInt:
        return True
    if not expr.isArithmeticalOp:
        return False
    if expr.astOperand1 and not is_constant_integer_expression(expr.astOperand1):
        return False
    if expr.astOperand2 and not is_constant_integer_expression(expr.astOperand2):
        return False
    return True

def isFunctionCall(expr, std='c99'):
    if not expr:
        return False
    if expr.str != '(' or not expr.astOperand1:
        return False
    if expr.astOperand1 != expr.previous:
        return False
    if isKeyword(expr.astOperand1.str, std):
        return False
    return True


def hasExternalLinkage(var):
    return var.isGlobal and not var.isStatic


def countSideEffects(expr):
    if not expr or expr.str in (',', ';'):
        return 0
    ret = 0
    if expr.str in ('++', '--', '='):
        ret = 1
    return ret + countSideEffects(expr.astOperand1) + countSideEffects(expr.astOperand2)


def getForLoopExpressions(forToken):
    if not forToken or forToken.str != 'for':
        return None
    lpar = forToken.next
    if not lpar or lpar.str != '(':
        return None
    if not lpar.astOperand2 or lpar.astOperand2.str != ';':
        return None
    if not lpar.astOperand2.astOperand2 or lpar.astOperand2.astOperand2.str != ';':
        return None
    return [lpar.astOperand2.astOperand1,
            lpar.astOperand2.astOperand2.astOperand1,
            lpar.astOperand2.astOperand2.astOperand2]


def get_function_scope(cfg, func):
    if func:
        for scope in cfg.scopes:
            if scope.function == func:
                return scope
    return None


def is_variable_changed(start_token, end_token, var):
    """Check if variable is updated between body_start and body_end"""
    tok = start_token
    while tok != end_token:
        if tok.isAssignmentOp:
            vartok = tok.astOperand1
            while vartok.astOperand1:
                vartok = vartok.astOperand1
            if vartok and vartok.variable == var:
                return True
        tok = tok.next
    return False

def getForLoopCounterVariables(forToken, cfg):
    """ Return a set of Variable objects defined in ``for`` statement and
    satisfy requirements to loop counter term from section 8.14 of MISRA
    document.
    """
    if not forToken or forToken.str != 'for':
        return None
    tn = forToken.next
    if not tn or tn.str != '(':
        return None
    vars_defined = set()
    vars_initialized = set()
    vars_exit = set()
    vars_modified = set()
    cur_clause = 1
    te = tn.link
    while tn and tn != te:
        if tn.variable:
            if cur_clause == 1 and tn.variable.nameToken == tn:
                vars_defined.add(tn.variable)
            elif cur_clause == 2:
                vars_exit.add(tn.variable)
            elif cur_clause == 3:
                if tn.next and countSideEffectsRecursive(tn.next) > 0:
                    vars_modified.add(tn.variable)
                elif tn.previous and tn.previous.str in ('++', '--'):
                    tn_ast = tn.astParent
                    if tn_ast and tn_ast == tn.previous:
                        vars_modified.add(tn.variable)
                    elif tn_ast and tn_ast.str == '.' and tn_ast.astOperand2 and tn_ast.astOperand2.variable:
                        vars_modified.add(tn_ast.astOperand2.variable)
        if cur_clause == 1 and tn.isAssignmentOp:
            var_token = tn.astOperand1
            while var_token and var_token.str == '.':
                var_token = var_token.astOperand2
            if var_token and var_token.variable:
                vars_initialized.add(var_token.variable)
        if cur_clause == 1 and tn.isName and tn.next.str == '(':
            function_args_in_init = getArguments(tn.next)
            function_scope = get_function_scope(cfg, tn.function)
            for arg_nr in range(len(function_args_in_init)):
                init_arg = function_args_in_init[arg_nr]
                if init_arg is None or not init_arg.isUnaryOp('&'):
                    continue
                var_token = init_arg.astOperand1
                while var_token and var_token.str == '.':
                    var_token = var_token.astOperand2
                if var_token is None or var_token.variable is None:
                    continue
                changed = False
                if function_scope is None:
                    changed = True
                elif tn.function is None:
                    changed = True
                else:
                    function_body_start = function_scope.bodyStart
                    function_body_end = function_scope.bodyEnd
                    args = tn.function.argument[arg_nr + 1]
                    if function_scope is None or is_variable_changed(function_body_start, function_body_end, args):
                        changed = True
                if changed:
                    vars_initialized.add(var_token.variable)

        if tn.str == ';':
            cur_clause += 1
        tn = tn.next
    return vars_defined | vars_initialized, vars_exit & vars_modified


def findCounterTokens(cond):
    if not cond:
        return []
    if cond.str in ['&&', '||']:
        c = findCounterTokens(cond.astOperand1)
        c.extend(findCounterTokens(cond.astOperand2))
        return c
    ret = []
    if ((cond.isArithmeticalOp and cond.astOperand1 and cond.astOperand2) or
            (cond.isComparisonOp and cond.astOperand1 and cond.astOperand2)):
        if cond.astOperand1.isName:
            ret.append(cond.astOperand1)
        if cond.astOperand2.isName:
            ret.append(cond.astOperand2)
        if cond.astOperand1.isOp:
            ret.extend(findCounterTokens(cond.astOperand1))
        if cond.astOperand2.isOp:
            ret.extend(findCounterTokens(cond.astOperand2))
    return ret


def isFloatCounterInWhileLoop(whileToken):
    if not simpleMatch(whileToken, 'while ('):
        return False
    lpar = whileToken.next
    rpar = lpar.link
    counterTokens = findCounterTokens(lpar.astOperand2)
    tok_varid = tuple(tok.varId for tok in counterTokens if tok.varId)
    whileBodyStart = None
    if simpleMatch(rpar, ') {'):
        whileBodyStart = rpar.next
    elif simpleMatch(whileToken.previous, '} while') and simpleMatch(whileToken.previous.link.previous, 'do {'):
        whileBodyStart = whileToken.previous.link
    else:
        return False
    token = whileBodyStart
    while token != whileBodyStart.link:
        token = token.next
        if not token.varId:
            continue
        if token.varId not in tok_varid:
            continue
        if not token.astParent or not token.valueType or not token.valueType.isFloat():
            continue
        parent = token.astParent
        if parent.str in ('++', '--'):
            return True
        while parent:
            if parent.isAssignmentOp and parent.str != "=" and parent.astOperand1 == token:
                return True
            if parent.str == "=" and parent.astOperand1.str == token.str and parent.astOperand1 != token:
                return True
            parent = parent.astParent
    return False


def countSideEffectsRecursive(expr):
    if not expr or expr.str == ';':
        return 0
    if expr.str == '=' and expr.astOperand1 and expr.astOperand1.str == '[':
        prev = expr.astOperand1.previous
        if prev and (prev.str == '{' or prev.str == '{'):
            return countSideEffectsRecursive(expr.astOperand2)
    if expr.str == '=' and expr.astOperand1 and expr.astOperand1.str == '.':
        e = expr.astOperand1
        while e and e.str == '.' and e.astOperand2:
            e = e.astOperand1
        if e and e.str == '.':
            return 0
    if expr.isAssignmentOp or expr.str in {'++', '--'}:
        return 1
    # Todo: Check function calls
    return countSideEffectsRecursive(expr.astOperand1) + countSideEffectsRecursive(expr.astOperand2)


def isBoolExpression(expr):
    if not expr:
        return False
    if expr.valueType and (expr.valueType.type == 'bool' or expr.valueType.bits == 1):
        return True
    return expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||', '0', '1', 'true', 'false']


def isEnumConstant(expr):
    if not expr or not expr.values:
        return False
    values = expr.values
    return len(values) == 1 and values[0].valueKind == 'known'


def isConstantExpression(expr):
    if expr.isNumber:
        return True
    if expr.isName and not isEnumConstant(expr):
        return False
    if simpleMatch(expr.previous, 'sizeof ('):
        return True
    if expr.astOperand1 and not isConstantExpression(expr.astOperand1):
        return False
    if expr.astOperand2 and not isConstantExpression(expr.astOperand2):
        return False
    return True

def isUnknownConstantExpression(expr):
    if expr.isName and not isEnumConstant(expr) and expr.variable is None:
        return True
    if expr.astOperand1 and isUnknownConstantExpression(expr.astOperand1):
        return True
    if expr.astOperand2 and isUnknownConstantExpression(expr.astOperand2):
        return True
    return False

def isUnsignedInt(expr):
    return expr and expr.valueType and expr.valueType.type in ('short', 'int') and expr.valueType.sign == 'unsigned'


def getPrecedence(expr):
    if not expr:
        return 16
    if not expr.astOperand1 or not expr.astOperand2:
        return 16
    if expr.str in ('*', '/', '%'):
        return 12
    if expr.str in ('+', '-'):
        return 11
    if expr.str in ('<<', '>>'):
        return 10
    if expr.str in ('<', '>', '<=', '>='):
        return 9
    if expr.str in ('==', '!='):
        return 8
    if expr.str == '&':
        return 7
    if expr.str == '^':
        return 6
    if expr.str == '|':
        return 5
    if expr.str == '&&':
        return 4
    if expr.str == '||':
        return 3
    if expr.str in ('?', ':'):
        return 2
    if expr.isAssignmentOp:
        return 1
    if expr.str == ',':
        return 0
    return -1


def findRawLink(token):
    tok1 = None
    tok2 = None
    forward = False

    if token.str in '{([':
        tok1 = token.str
        tok2 = '})]'['{(['.find(token.str)]
        forward = True
    elif token.str in '})]':
        tok1 = token.str
        tok2 = '{(['['})]'.find(token.str)]
        forward = False
    else:
        return None

    # try to find link
    indent = 0
    while token:
        if token.str == tok1:
            indent = indent + 1
        elif token.str == tok2:
            if indent <= 1:
                return token
            indent = indent - 1
        if forward is True:
            token = token.next
        else:
            token = token.previous

    # raw link not found
    return None


def numberOfParentheses(tok1, tok2):
    while tok1 and tok1 != tok2:
        if tok1.str == '(' or tok1.str == ')':
            return False
        tok1 = tok1.next
    return tok1 == tok2


def findGotoLabel(gotoToken):
    label = gotoToken.next.str
    tok = gotoToken.next.next
    while tok:
        if tok.str == '}' and tok.scope.type == 'Function':
            break
        if tok.str == label and tok.next.str == ':':
            return tok
        tok = tok.next
    return None


def findInclude(directives, header):
    for directive in directives:
        if directive.str == '#include ' + header:
            return directive
    return None


# Get function arguments
def getArgumentsRecursive(tok, arguments):
    if tok is None:
        return
    if tok.str == ',':
        getArgumentsRecursive(tok.astOperand1, arguments)
        getArgumentsRecursive(tok.astOperand2, arguments)
    else:
        arguments.append(tok)


def getArguments(ftok):
    arguments = []
    getArgumentsRecursive(ftok.astOperand2, arguments)
    return arguments


def isalnum(c):
    return c in string.digits or c in string.ascii_letters


def isHexEscapeSequence(symbols):
    """Checks that given symbols are valid hex escape sequence.

    hexadecimal-escape-sequence:
            \\x hexadecimal-digit
            hexadecimal-escape-sequence hexadecimal-digit

    Reference: n1570 6.4.4.4"""
    if len(symbols) < 3 or symbols[:2] != '\\x':
        return False
    return all([s in string.hexdigits for s in symbols[2:]])


def isOctalEscapeSequence(symbols):
    r"""Checks that given symbols are valid octal escape sequence:

     octal-escape-sequence:
             \ octal-digit
             \ octal-digit octal-digit
             \ octal-digit octal-digit octal-digit

    Reference: n1570 6.4.4.4"""
    if len(symbols) not in range(2, 5) or symbols[0] != '\\':
        return False
    return all([s in string.octdigits for s in symbols[1:]])


def isSimpleEscapeSequence(symbols):
    """Checks that given symbols are simple escape sequence.
    Reference: n1570 6.4.4.4"""
    if len(symbols) != 2 or symbols[0] != '\\':
        return False
    return symbols[1] in ("'", '"', '?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v')


def isTernaryOperator(token):
    if not token:
        return False
    if not token.astOperand2:
        return False
    return token.str == '?' and token.astOperand2.str == ':'


def getTernaryOperandsRecursive(token):
    """Returns list of ternary operands including nested ones."""
    if not isTernaryOperator(token):
        return []
    result = []
    result += getTernaryOperandsRecursive(token.astOperand2.astOperand1)
    if token.astOperand2.astOperand1 and not isTernaryOperator(token.astOperand2.astOperand1):
        result += [token.astOperand2.astOperand1]
    result += getTernaryOperandsRecursive(token.astOperand2.astOperand2)
    if token.astOperand2.astOperand2 and not isTernaryOperator(token.astOperand2.astOperand2):
        result += [token.astOperand2.astOperand2]
    return result


def hasNumericEscapeSequence(symbols):
    """Check that given string contains octal or hexadecimal escape sequences."""
    if '\\' not in symbols:
        return False
    for c, cn in grouped(symbols, 2):
        if c == '\\' and cn in ('x' + string.octdigits):
            return True
    return False


def isNoReturnScope(tok):
    if tok is None or tok.str != '}':
        return False
    if tok.previous is None or tok.previous.str != ';':
        return False
    if simpleMatch(tok.previous.previous, 'break ;'):
        return True
    prev = tok.previous.previous
    while prev and prev.str not in ';{}':
        if prev.str in '])':
            prev = prev.link
        prev = prev.previous
    if prev and prev.next.str in ['throw', 'return']:
        return True
    return False


# Return the token which the value is assigned to
def getAssignedVariableToken(vartok):
    if not vartok:
        return None
    parent = vartok.astParent
    while parent and parent.isArithmeticalOp:
        parent = parent.astParent
    if parent and parent.isAssignmentOp:
        return parent.astOperand1
    return None

# If the value is used as a return value, return the function definition
def getFunctionUsingReturnValue(valueToken):
    if not valueToken:
        return None
    if not valueToken.astParent:
        return None
    operator = valueToken.astParent
    if operator.str == 'return':
        return operator.scope.function
    if operator.isArithmeticalOp:
        return getFunctionUsingReturnValue(operator)
    return None

# Return true if the token follows a specific sequence of token str values
def tokenFollowsSequence(token, sequence):
    if not token:
        return False
    for i in reversed(sequence):
        prev = token.previous
        if not prev:
            return False
        if prev.str != i:
            return False
        token = prev
    return True

class Define:
    def __init__(self, directive):
        self.name = ''
        self.args = []
        self.expansionList = ''

        res = re.match(r'#define ([A-Za-z0-9_]+)\(([A-Za-z0-9_, ]+)\)[ ]+(.*)', directive.str)
        if res:
            self.name = res.group(1)
            self.args = res.group(2).strip().split(',')
            self.expansionList = res.group(3)
        else:
            res = re.match(r'#define ([A-Za-z0-9_]+)[ ]+(.*)', directive.str)
            if res:
                self.name = res.group(1)
                self.expansionList = res.group(2)

    def __repr__(self):
        attrs = ["name", "args", "expansionList"]
        return "{}({})".format(
            "Define",
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )


def getAddonRules():
    """Returns dict of MISRA rules handled by this addon."""
    addon_rules = []
    compiled = re.compile(r'.*def[ ]+misra_([0-9]+)_([0-9]+)[(].*')
    with open(__file__) as f:
        for line in f:
            res = compiled.match(line)
            if res is None:
                continue
            addon_rules.append(res.group(1) + '.' + res.group(2))
    return addon_rules


def getCppcheckRules():
    """Returns list of rules handled by cppcheck."""
    return ['1.3', # <most "error">
            '2.1', # alwaysFalse, duplicateBreak
            '2.2', # alwaysTrue, redundantCondition, redundantAssignment, redundantAssignInSwitch, unreadVariable
            '2.6', # unusedLabel
            '5.3', # shadowVariable
            '8.3', # funcArgNamesDifferent
            '8.13', # constPointer
            '9.1', # uninitvar
            '14.3', # alwaysTrue, alwaysFalse, compareValueOutOfTypeRangeError
            '13.2', # unknownEvaluationOrder
            '13.6', # sizeofCalculation
            '17.4', # missingReturn
            '17.5', # argumentSize
            '18.1', # pointerOutOfBounds
            '18.2', # comparePointers
            '18.3', # comparePointers
            '18.6', # danglingLifetime
            '19.1', # overlappingWriteUnion, overlappingWriteFunction
            '20.6', # preprocessorErrorDirective
            '21.13', # invalidFunctionArg
            '21.17', # bufferAccessOutOfBounds
            '21.18', # bufferAccessOutOfBounds
            '22.1', # memleak, resourceLeak, memleakOnRealloc, leakReturnValNotUsed, leakNoVarFunctionCall
            '22.2', # autovarInvalidDeallocation
            '22.3', # incompatibleFileOpen
            '22.4', # writeReadOnlyFile
            '22.6' # useClosedFile
           ]


def generateTable():
    # print table
    numberOfRules = {}
    numberOfRules[1] = 3
    numberOfRules[2] = 7
    numberOfRules[3] = 2
    numberOfRules[4] = 2
    numberOfRules[5] = 9
    numberOfRules[6] = 2
    numberOfRules[7] = 4
    numberOfRules[8] = 14
    numberOfRules[9] = 5
    numberOfRules[10] = 8
    numberOfRules[11] = 9
    numberOfRules[12] = 4
    numberOfRules[13] = 6
    numberOfRules[14] = 4
    numberOfRules[15] = 7
    numberOfRules[16] = 7
    numberOfRules[17] = 8
    numberOfRules[18] = 8
    numberOfRules[19] = 2
    numberOfRules[20] = 14
    numberOfRules[21] = 21
    numberOfRules[22] = 10

    # Rules that can be checked with compilers:
    # compiler = ['1.1', '1.2']

    addon = getAddonRules()
    cppcheck = getCppcheckRules()
    for i1 in range(1, 23):
        for i2 in range(1, numberOfRules[i1] + 1):
            num = str(i1) + '.' + str(i2)
            s = ''
            if num in addon:
                s = 'X (Addon)'
            elif num in cppcheck:
                s = 'X (Cppcheck)'
            num = num + '       '
            print(num[:8] + s)


def remove_file_prefix(file_path, prefix):
    """
    Remove a file path prefix from a give path.  leftover
    directory separators at the beginning of a file
    after the removal are also stripped.

    Example:
        '/remove/this/path/file.c'
    with a prefix of:
        '/remove/this/path'
    becomes:
        file.c
    """
    result = None
    if file_path.startswith(prefix):
        result = file_path[len(prefix):]
        # Remove any leftover directory separators at the
        # beginning
        result = result.lstrip('\\/')
    else:
        result = file_path
    return result


class Rule():
    """Class to keep rule text and metadata"""

    MISRA_SEVERITY_LEVELS = ['Required', 'Mandatory', 'Advisory']

    def __init__(self, num1, num2):
        self.num1 = num1
        self.num2 = num2
        self.text = ''
        self.misra_severity = ''

    @property
    def num(self):
        return self.num1 * 100 + self.num2

    @property
    def misra_severity(self):
        return self._misra_severity

    @misra_severity.setter
    def misra_severity(self, val):
        if val in self.MISRA_SEVERITY_LEVELS:
            self._misra_severity = val
        else:
            self._misra_severity = ''

    @property
    def cppcheck_severity(self):
        return 'style'

    def __repr__(self):
        return "%d.%d (%s)" % (self.num1, self.num2, self.misra_severity)


class MisraSettings():
    """Hold settings for misra.py script."""

    __slots__ = ["verify", "quiet", "show_summary"]

    def __init__(self, args):
        """
        :param args: Arguments given by argparse.
        """
        self.verify = False
        self.quiet = False
        self.show_summary = True

        if args.verify:
            self.verify = True
        if args.cli:
            self.quiet = True
            self.show_summary = False
        if args.quiet:
            self.quiet = True
        if args.no_summary:
            self.show_summary = False

    def __repr__(self):
        attrs = ["verify", "quiet", "show_summary", "verify"]
        return "{}({})".format(
            "MisraSettings",
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )


class MisraChecker:

    def __init__(self, settings, stdversion="c89"):
        """
        :param settings: misra.py script settings.
        """

        self.settings = settings

        # Test validation rules lists
        self.verify_expected = []
        self.verify_actual = []

        # List of formatted violation messages
        self.violations = {}

        # if --rule-texts is specified this dictionary
        # is loaded with descriptions of each rule
        # by rule number (in hundreds).
        # ie rule 1.2 becomes 102
        self.ruleTexts = {}
        self.ruleText_filename = None

        # Dictionary of dictionaries for rules to suppress
        # Dict1 is keyed by rule number in the hundreds format of
        # Major *  100 + minor. ie Rule 5.2 = (5*100) + 2
        # Dict 2 is keyed by filename.  An entry of None means suppress globally.
        # Each file name entry contains a list of tuples of (lineNumber, symbolName)
        # or an item of None which indicates suppress rule for the entire file.
        # The line and symbol name tuple may have None as either of its elements but
        # should not be None for both.
        self.suppressedRules = {}

        # Prefix to ignore when matching suppression files.
        self.filePrefix = None

        # Number of all violations suppressed per rule
        self.suppressionStats = {}

        self.stdversion = stdversion

        self.severity = None

        self.existing_violations = set()

        self._ctu_summary_typedefs = False
        self._ctu_summary_tagnames = False
        self._ctu_summary_identifiers = False
        self._ctu_summary_usage = False

        self.path_premium_addon = None

    def __repr__(self):
        attrs = ["settings", "verify_expected", "verify_actual", "violations",
                 "ruleTexts", "suppressedRules", "filePrefix",
                 "suppressionStats", "stdversion", "severity"]
        return "{}({})".format(
            "MisraChecker",
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )

    def get_num_significant_naming_chars(self, cfg):
        if cfg.standards and cfg.standards.c == "c89":
            return 31
        return 63

    def _save_ctu_summary_typedefs(self, dumpfile, typedef_info):
        if self._ctu_summary_typedefs:
            return

        self._ctu_summary_typedefs = True

        summary = []
        for ti in typedef_info:
            summary.append({ 'name': ti.name, 'file': ti.file, 'line': ti.linenr, 'column': ti.column, 'used': ti.used })
        if len(summary) > 0:
            cppcheckdata.reportSummary(dumpfile, 'MisraTypedefInfo', summary)

    def _save_ctu_summary_tagnames(self, dumpfile, cfg):
        if self._ctu_summary_tagnames:
            return

        self._ctu_summary_tagnames = True

        summary = []
        # structs/enums
        for scope in cfg.scopes:
            if scope.className is None:
                continue
            if scope.className.startswith('Anonymous'):
                continue
            if scope.type not in ('Struct', 'Enum'):
                continue
            used = False
            tok = scope.bodyEnd
            while tok:
                if tok.str == scope.className:
                    used = True
                    break
                tok = tok.next
            summary.append({'name': scope.className, 'used':used, 'file': scope.bodyStart.file, 'line': scope.bodyStart.linenr, 'column': scope.bodyStart.column})
        if len(summary) > 0:
            cppcheckdata.reportSummary(dumpfile, 'MisraTagName', summary)

    def _save_ctu_summary_identifiers(self, dumpfile, cfg):
        if self._ctu_summary_identifiers:
            return
        self._ctu_summary_identifiers = True

        external_identifiers = []
        internal_identifiers = []
        local_identifiers = []

        def identifier(nameToken):
            return {'name':nameToken.str, 'file':nameToken.file, 'line':nameToken.linenr, 'column':nameToken.column}

        names = []

        for var in cfg.variables:
            if var.nameToken is None:
                continue
            if var.access != 'Global':
                if var.nameToken.str in names:
                    continue
                names.append(var.nameToken.str)
                local_identifiers.append(identifier(var.nameToken))
            elif var.isStatic:
                names.append(var.nameToken.str)
                i = identifier(var.nameToken)
                i['inlinefunc'] = False
                internal_identifiers.append(i)
            else:
                names.append(var.nameToken.str)
                i = identifier(var.nameToken)
                i['decl'] = var.isExtern
                external_identifiers.append(i)

        for func in cfg.functions:
            if func.tokenDef is None:
                continue
            if func.isStatic:
                i = identifier(func.tokenDef)
                i['inlinefunc'] = func.isInlineKeyword
                internal_identifiers.append(i)
            else:
                if func.token is None:
                    i = identifier(func.tokenDef)
                else:
                    i = identifier(func.token)
                i['decl'] = func.token is None
                external_identifiers.append(i)

        cppcheckdata.reportSummary(dumpfile, 'MisraExternalIdentifiers', external_identifiers)
        cppcheckdata.reportSummary(dumpfile, 'MisraInternalIdentifiers', internal_identifiers)
        cppcheckdata.reportSummary(dumpfile, 'MisraLocalIdentifiers', local_identifiers)

    def _save_ctu_summary_usage(self, dumpfile, cfg):
        if self._ctu_summary_usage:
            return
        self._ctu_summary_usage = True

        names = []
        for token in cfg.tokenlist:
            if not token.isName:
                continue
            if token.function and token != token.function.tokenDef:
                if (not token.function.isStatic) and (token.str not in names):
                    names.append({'name': token.str, 'file': token.file})
            elif token.variable:
                if token == token.variable.nameToken:
                    continue
                if token.variable.access == 'Global' and (not token.variable.isStatic) and (token.str not in names):
                    names.append({'name': token.str, 'file': token.file})

        if len(names) > 0:
            cppcheckdata.reportSummary(dumpfile, 'MisraUsage', names)


    def misra_1_2(self, cfg):
        # gcc language extensions: https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html
        for token in cfg.tokenlist:
            if simpleMatch(token, '? :'):
                self.reportError(token, 1, 2)
            elif simpleMatch(token, '( {') and simpleMatch(token.next.link.previous, '; } )'):
                self.reportError(token, 1, 2)


    def misra_1_4(self, cfg):
        for token in cfg.tokenlist:
            if token.str in ('_Atomic', '_Noreturn', '_Generic', '_Thread_local', '_Alignas', '_Alignof'):
                self.reportError(token, 1, 4)
            if token.str.endswith('_s') and isFunctionCall(token.next, cfg.standards.c):
                # See C specification C11 - Annex K, page 578
                if token.str in ('tmpfile_s', 'tmpnam_s', 'fopen_s', 'freopen_s', 'fprintf_s', 'fscanf_s', 'printf_s', 'scanf_s',
                                 'snprintf_s', 'sprintf_s', 'sscanf_s', 'vfprintf_s', 'vfscanf_s', 'vprintf_s', 'vscanf_s',
                                 'vsnprintf_s', 'vsprintf_s', 'vsscanf_s', 'gets_s', 'set_constraint_handler_s', 'abort_handler_s',
                                 'ignore_handler_s', 'getenv_s', 'bsearch_s', 'qsort_s', 'wctomb_s', 'mbstowcs_s', 'wcstombs_s',
                                 'memcpy_s', 'memmove_s', 'strcpy_s', 'strncpy_s', 'strcat_s', 'strncat_s', 'strtok_s', 'memset_s',
                                 'strerror_s', 'strerrorlen_s', 'strnlen_s', 'asctime_s', 'ctime_s', 'gmtime_s', 'localtime_s',
                                 'fwprintf_s', 'fwscanf_s', 'snwprintf_s', 'swprintf_s', 'swscanf_s', 'vfwprintf_s', 'vfwscanf_s',
                                 'vsnwprintf_s', 'vswprintf_s', 'vswscanf_s', 'vwprintf_s', 'vwscanf_s', 'wprintf_s', 'wscanf_s',
                                 'wcscpy_s', 'wcsncpy_s', 'wmemcpy_s', 'wmemmove_s', 'wcscat_s', 'wcsncat_s', 'wcstok_s', 'wcsnlen_s',
                                 'wcrtomb_s', 'mbsrtowcs_s', 'wcsrtombs_s'):
                    self.reportError(token, 1, 4)

    def misra_2_2(self, cfg):
        for token in cfg.tokenlist:
            if token.isExpandedMacro:
                continue
            if (token.str in '+-') and token.astOperand2:
                if simpleMatch(token.astOperand1, '0'):
                    self.reportError(token.astOperand1, 2, 2)
                elif simpleMatch(token.astOperand2, '0'):
                    self.reportError(token.astOperand2, 2, 2)
            if token.str == '*' and token.astOperand2:
                if simpleMatch(token.astOperand2, '0'):
                    self.reportError(token.astOperand1, 2, 2)
                elif simpleMatch(token.astOperand1, '0'):
                    self.reportError(token.astOperand2, 2, 2)
                elif simpleMatch(token.astOperand1, '1'):
                    self.reportError(token.astOperand1, 2, 2)
                elif simpleMatch(token.astOperand2, '1'):
                    self.reportError(token.astOperand2, 2, 2)

    def misra_2_3(self, dumpfile, typedefInfo):
        self._save_ctu_summary_typedefs(dumpfile, typedefInfo)

    def misra_2_4(self, dumpfile, cfg):
        self._save_ctu_summary_tagnames(dumpfile, cfg)

    def misra_2_5(self, dumpfile, cfg):
        used_macros = []
        unused_macro = {}
        for m in cfg.macro_usage:
            used_macros.append(m.name)
        for directive in cfg.directives:
            res_define = re.match(r'#define[ \t]+([a-zA-Z_][a-zA-Z_0-9]*).*', directive.str)
            res_undef = re.match(r'#undef[ \t]+([a-zA-Z_][a-zA-Z_0-9]*).*', directive.str)
            if res_define:
                macro_name = res_define.group(1)
                unused_macro[macro_name] = {'name': macro_name, 'used': (macro_name in used_macros),
                                            'file': directive.file, 'line': directive.linenr, 'column': directive.column}
            elif res_undef:
                macro_name = res_undef.group(1)
                # assuming that if we have #undef, we also have #define somewhere
                if macro_name in unused_macro:
                    unused_macro[macro_name]['used'] = True
                else:
                    unused_macro[macro_name] = {'name': macro_name, 'used': True, 'file': directive.file,
                                                'line': directive.linenr, 'column': directive.column}
                    used_macros.append(macro_name)

        if unused_macro:
            cppcheckdata.reportSummary(dumpfile, 'MisraMacro', list(unused_macro.values()))

    def misra_2_7(self, data):
        for func in data.functions:
            # Skip function with no parameter
            if len(func.argument) == 0:
                continue
            # Setup list of function parameters
            func_param_list = []
            for arg in func.argument:
                func_arg = func.argument[arg]
                if func_arg.typeStartToken and func_arg.typeStartToken.str == '...':
                    continue
                func_param_list.append(func_arg)
            # Search for scope of current function
            for scope in data.scopes:
                if (scope.type == "Function") and (scope.function == func):
                    # Search function body: remove referenced function parameter from list
                    token = scope.bodyStart
                    while token.next is not None and token != scope.bodyEnd and len(func_param_list) > 0:
                        if token.variable is not None and token.variable in func_param_list:
                            func_param_list.remove(token.variable)
                        token = token.next
                    # Emit a warning for each unused variable, but no more that one warning per line
                    reported_linenrs = set()
                    for func_param in func_param_list:
                        if func_param.nameToken:
                            linenr = func_param.nameToken
                            if linenr not in reported_linenrs:
                                self.reportError(func_param.nameToken, 2, 7)
                                reported_linenrs.add(linenr)
                        else:
                            linenr = func.tokenDef.linenr
                            if linenr not in reported_linenrs:
                                self.reportError(func.tokenDef, 2, 7)
                                reported_linenrs.add(linenr)

    def misra_3_1(self, rawTokens):
        for token in rawTokens:
            starts_with_double_slash = token.str.startswith('//')
            starts_with_block_comment = token.str.startswith("/*")
            s = token.str.lstrip('/')
            if (starts_with_double_slash or starts_with_block_comment) and "/*" in s:
                # Block comment inside of regular comment, violation
                self.reportError(token, 3, 1)
            elif starts_with_block_comment and "//" in s:
                # "//" in block comment, check if it's a uri
                while "//" in s:
                    possible_uri, s = s.split("//", 1)
                    if not re.search(r"\w+:$", possible_uri):
                        # Violation if no uri was found
                        self.reportError(token, 3, 1)
                        break

    def misra_4_1(self, rawTokens):
        for token in rawTokens:
            if (token.str[0] != '"') and (token.str[0] != '\''):
                continue
            if len(token.str) < 3:
                continue

            delimiter = token.str[0]
            symbols = token.str[1:-1]

            # No closing delimiter. This will not compile.
            if token.str[-1] != delimiter:
                continue

            if len(symbols) < 2:
                continue

            if not hasNumericEscapeSequence(symbols):
                continue

            # String literals that contains one or more escape sequences. All of them should be
            # terminated.
            for sequence in ['\\' + t for t in symbols.split('\\')][1:]:
                if (isHexEscapeSequence(sequence) or isOctalEscapeSequence(sequence) or
                        isSimpleEscapeSequence(sequence)):
                    continue
                self.reportError(token, 4, 1)

    def misra_4_2(self, rawTokens):
        for token in rawTokens:
            if (token.str[0] != '"') or (token.str[-1] != '"'):
                continue
            # Check for trigraph sequence as defined by ISO/IEC 9899:1999
            for sequence in ['??=', '??(', '??/', '??)', '??\'', '??<', '??!', '??>', '??-']:
                if sequence in token.str[1:-1]:
                    # First trigraph sequence match, report error and leave loop.
                    self.reportError(token, 4, 2)
                    break

    def misra_5_1(self, data):
        long_vars = {}
        num_sign_chars = self.get_num_significant_naming_chars(data)
        for var in data.variables:
            if var.nameToken is None:
                continue
            if len(var.nameToken.str) <= num_sign_chars:
                continue
            if not hasExternalLinkage(var):
                continue
            long_vars.setdefault(var.nameToken.str[:num_sign_chars], []).append(var.nameToken)
        for name_prefix in long_vars:
            tokens = long_vars[name_prefix]
            if len(tokens) < 2:
                continue
            for tok in sorted(tokens, key=lambda t: (t.linenr, t.column))[1:]:
                self.reportError(tok, 5, 1)

    def misra_5_2(self, data):
        scopeVars = {}
        num_sign_chars = self.get_num_significant_naming_chars(data)
        for var in data.variables:
            if var.nameToken is None:
                continue
            if len(var.nameToken.str) <= num_sign_chars:
                continue
            if var.nameToken.scope not in scopeVars:
                scopeVars.setdefault(var.nameToken.scope, {})["varlist"] = []
                scopeVars.setdefault(var.nameToken.scope, {})["scopelist"] = []
            scopeVars[var.nameToken.scope]["varlist"].append(var)
        for scope in data.scopes:
            if scope.nestedIn and scope.className:
                if scope.nestedIn not in scopeVars:
                    scopeVars.setdefault(scope.nestedIn, {})["varlist"] = []
                    scopeVars.setdefault(scope.nestedIn, {})["scopelist"] = []
                scopeVars[scope.nestedIn]["scopelist"].append(scope)
        for scope in scopeVars:
            if len(scopeVars[scope]["varlist"]) <= 1:
                continue
            for i, variable1 in enumerate(scopeVars[scope]["varlist"]):
                for variable2 in scopeVars[scope]["varlist"][i + 1:]:
                    if variable1.isArgument and variable2.isArgument:
                        continue
                    if hasExternalLinkage(variable1) or hasExternalLinkage(variable2):
                        continue
                    if (variable1.nameToken.str[:num_sign_chars] == variable2.nameToken.str[:num_sign_chars] and
                            variable1 is not variable2):
                        if int(variable1.nameToken.linenr) > int(variable2.nameToken.linenr):
                            self.reportError(variable1.nameToken, 5, 2)
                        else:
                            self.reportError(variable2.nameToken, 5, 2)
                for innerscope in scopeVars[scope]["scopelist"]:
                    if variable1.nameToken.str[:num_sign_chars] == innerscope.className[:num_sign_chars]:
                        if int(variable1.nameToken.linenr) > int(innerscope.bodyStart.linenr):
                            self.reportError(variable1.nameToken, 5, 2)
                        else:
                            self.reportError(innerscope.bodyStart, 5, 2)
            if len(scopeVars[scope]["scopelist"]) <= 1:
                continue
            for i, scopename1 in enumerate(scopeVars[scope]["scopelist"]):
                for scopename2 in scopeVars[scope]["scopelist"][i + 1:]:
                    if scopename1.className[:num_sign_chars] == scopename2.className[:num_sign_chars]:
                        if int(scopename1.bodyStart.linenr) > int(scopename2.bodyStart.linenr):
                            self.reportError(scopename1.bodyStart, 5, 2)
                        else:
                            self.reportError(scopename2.bodyStart, 5, 2)

    def misra_5_4(self, data):
        num_sign_chars = self.get_num_significant_naming_chars(data)
        macro = {}
        compile_name = re.compile(r'#define ([a-zA-Z0-9_]+)')
        compile_param = re.compile(r'#define ([a-zA-Z0-9_]+)[(]([a-zA-Z0-9_, ]+)[)]')
        short_names = {}
        macro_w_arg = []
        for dir in data.directives:
            res1 = compile_name.match(dir.str)
            if res1:
                if dir not in macro:
                    macro.setdefault(dir, {})["name"] = []
                    macro.setdefault(dir, {})["params"] = []
                full_name = res1.group(1)
                macro[dir]["name"] = full_name
                short_name = full_name[:num_sign_chars]
                if short_name in short_names:
                    _dir = short_names[short_name]
                    if full_name != macro[_dir]["name"]:
                        self.reportError(dir, 5, 4)
                else:
                    short_names[short_name] = dir
            res2 = compile_param.match(dir.str)
            if res2:
                res_gp2 = res2.group(2).split(",")
                res_gp2 = [macroname.replace(" ", "") for macroname in res_gp2]
                macro[dir]["params"].extend(res_gp2)
                macro_w_arg.append(dir)
        for mvar in macro_w_arg:
            for i, macroparam1 in enumerate(macro[mvar]["params"]):
                for j, macroparam2 in enumerate(macro[mvar]["params"]):
                    if j > i and macroparam1[:num_sign_chars] == macroparam2[:num_sign_chars]:
                        self.reportError(mvar, 5, 4)
                param = macroparam1
                if param[:num_sign_chars] in short_names:
                    m_var1 = short_names[param[:num_sign_chars]]
                    if m_var1.linenr > mvar.linenr:
                        self.reportError(m_var1, 5, 4)
                    else:
                        self.reportError(mvar, 5, 4)

    def misra_5_5(self, data):
        num_sign_chars = self.get_num_significant_naming_chars(data)
        macroNames = {}
        compiled = re.compile(r'#define ([A-Za-z0-9_]+)')
        for dir in data.directives:
            res = compiled.match(dir.str)
            if res:
                macroNames[res.group(1)[:num_sign_chars]] = dir
        for var in data.variables:
            if var.nameToken and var.nameToken.str[:num_sign_chars] in macroNames:
                self.reportError(var.nameToken, 5, 5)
        for scope in data.scopes:
            if scope.className and scope.className[:num_sign_chars] in macroNames:
                self.reportError(scope.bodyStart, 5, 5)


    def misra_5_6(self, dumpfile, typedefInfo):
        self._save_ctu_summary_typedefs(dumpfile, typedefInfo)

    def misra_5_7(self, dumpfile, cfg):
        self._save_ctu_summary_tagnames(dumpfile, cfg)

    def misra_5_8(self, dumpfile, cfg):
        self._save_ctu_summary_identifiers(dumpfile, cfg)

    def misra_5_9(self, dumpfile, cfg):
        self._save_ctu_summary_identifiers(dumpfile, cfg)

    def misra_6_1(self, data):
        # Bitfield type must be bool or explicitly signed/unsigned int
        for token in data.tokenlist:
            if not token.valueType:
                continue
            if token.valueType.bits is None:
                continue
            if not token.variable:
                continue
            if not token.scope:
                continue
            if token.scope.type not in 'Struct':
                continue

            if data.standards.c == 'c89':
                if token.valueType.type != 'int' and  not isUnsignedType(token.variable.typeStartToken.str):
                    self.reportError(token, 6, 1)
            elif data.standards.c in ('c99', 'c11', 'c17', 'c18'):
                if token.valueType.type == 'bool':
                    continue

            isExplicitlySignedOrUnsigned = False
            typeToken = token.variable.typeStartToken
            while typeToken:
                if typeToken.isUnsigned or typeToken.isSigned or isUnsignedType(typeToken.str):
                    isExplicitlySignedOrUnsigned = True
                    break

                if typeToken is token.variable.typeEndToken:
                    break

                typeToken = typeToken.next

            if not isExplicitlySignedOrUnsigned:
                self.reportError(token, 6, 1)


    def misra_6_2(self, data):
        # Bitfields of size 1 can not be signed
        for token in data.tokenlist:
            if not token.valueType:
                continue
            if not token.scope:
                continue
            if token.scope.type not in 'Struct':
                continue
            if token.valueType.bits == 1 and token.valueType.sign == 'signed':
                self.reportError(token, 6, 2)


    def misra_7_1(self, rawTokens):
        compiled = re.compile(r'^0[0-7]+$')
        for tok in rawTokens:
            if compiled.match(tok.str):
                self.reportError(tok, 7, 1)

    def misra_7_2(self, data):
        for token in data.tokenlist:
            if token.isInt and ('U' not in token.str.upper()) and token.valueType and token.valueType.sign == 'unsigned':
                self.reportError(token, 7, 2)

    def misra_7_3(self, rawTokens):
        # Match decimal digits, hex digits, decimal point, and e/E p/P floating
        # point constant exponent separators.
        compiled = re.compile(r'^(0[xX])?[0-9a-fA-FpP.]+[Uu]*l+[Uu]*$')
        for tok in rawTokens:
            if compiled.match(tok.str):
                self.reportError(tok, 7, 3)

    def misra_7_4(self, data):
        # A string literal shall not be assigned to an object unless the object's type
        # is constant.
        def reportErrorIfVariableIsNotConst(variable, stringLiteral):
            if variable.valueType:
                if (variable.valueType.constness % 2) != 1:
                    self.reportError(stringLiteral, 7, 4)

        for token in data.tokenlist:
            if token.isString:
                # Check normal variable assignment
                variable = getAssignedVariableToken(token)
                if variable:
                    reportErrorIfVariableIsNotConst(variable, token)

                # Check use as return value
                function = getFunctionUsingReturnValue(token)
                if function:
                    # "Primitive" test since there is no info available on return value type
                    if not tokenFollowsSequence(function.tokenDef, ['const', 'char', '*']):
                        self.reportError(token, 7, 4)

            # Check use as function parameter
            if isFunctionCall(token, data.standards.c) and token.astOperand1 and token.astOperand1.function:
                functionDeclaration = token.astOperand1.function

                if functionDeclaration.tokenDef:
                    if functionDeclaration.tokenDef is token.astOperand1:
                        # Token is not a function call, but it is the definition of the function
                        continue

                    parametersUsed = getArguments(token)
                    for i in range(len(parametersUsed)):
                        usedParameter = parametersUsed[i]
                        parameterDefinition = functionDeclaration.argument.get(i+1)

                        if usedParameter.isString and parameterDefinition and parameterDefinition.nameToken:
                            reportErrorIfVariableIsNotConst(parameterDefinition.nameToken, usedParameter)

    def misra_8_1(self, cfg):
        for token in cfg.tokenlist:
            if token.isImplicitInt and not token.isUnsigned and not token.isSigned:
                self.reportError(token, 8, 1)

    def misra_8_2(self, data, rawTokens):
        def getFollowingRawTokens(rawTokens, token, count):
            following =[]
            for rawToken in rawTokens:
                if (rawToken.file == token.file and
                        rawToken.linenr == token.linenr and
                        rawToken.column == token.column):
                    for _ in range(count):
                        rawToken = rawToken.next
                        # Skip comments
                        while rawToken and (rawToken.str.startswith('/*') or rawToken.str.startswith('//')):
                            rawToken = rawToken.next
                        if rawToken is None:
                            break
                        following.append(rawToken)
            return following

        # Zero arguments should be in form ( void )
        def checkZeroArguments(func, startCall, endCall):
            if not startCall.isRemovedVoidParameter and len(func.argument) == 0:
                if func.tokenDef.next:
                    self.reportError(func.tokenDef.next, 8, 2)
                else:
                    self.reportError(func.tokenDef, 8, 2)

        def checkDeclarationArgumentsViolations(func, startCall, endCall):
            # Collect the tokens for the arguments in function definition
            argNameTokens = set()
            for arg in func.argument:
                argument = func.argument[arg]
                typeStartToken = argument.typeStartToken
                if typeStartToken is None:
                    continue
                nameToken = argument.nameToken
                if nameToken is None:
                    continue
                argNameTokens.add(nameToken)

            # Check if we have the same number of variables in both the
            # declaration and the definition.
            #
            # TODO: We actually need to check if the names of the arguments are
            # the same. But we can't do this because we have no links to
            # variables in the arguments in function definition in the dump file.
            foundVariables = 0
            while startCall and startCall != endCall:
                if startCall.varId:
                    foundVariables += 1
                startCall = startCall.next

            if len(argNameTokens) != foundVariables:
                if func.tokenDef.next:
                    self.reportError(func.tokenDef.next, 8, 2)
                else:
                    self.reportError(func.tokenDef, 8, 2)

        def checkDefinitionArgumentsViolations(func, startCall, endCall):
            for arg in func.argument:
                argument = func.argument[arg]
                typeStartToken = argument.typeStartToken
                if typeStartToken is None:
                    continue

                # Arguments should have a name unless variable length arg
                nameToken = argument.nameToken
                if nameToken is None and typeStartToken.str != '...':
                    self.reportError(typeStartToken, 8, 2)

                # Type declaration on next line (old style declaration list) is not allowed
                if typeStartToken.linenr > endCall.linenr:
                    self.reportError(typeStartToken, 8, 2)

        # Check arguments in function declaration
        for func in data.functions:

            # Check arguments in function definition
            tokenImpl = func.token
            if tokenImpl:
                startCall = tokenImpl.next
                if startCall is None or startCall.str != '(':
                    continue
                endCall = startCall.link
                if endCall is None or endCall.str != ')':
                    continue
                checkZeroArguments(func, startCall, endCall)
                checkDefinitionArgumentsViolations(func, startCall, endCall)

            # Check arguments in function declaration
            tokenDef = func.tokenDef
            if tokenDef:
                startCall = func.tokenDef.next
                if startCall is None or startCall.str != '(':
                    continue
                endCall = startCall.link
                if endCall is None or endCall.str != ')':
                    continue
                checkZeroArguments(func, startCall, endCall)
                if tokenImpl:
                    checkDeclarationArgumentsViolations(func, startCall, endCall)
                else:
                    # When there is no function definition, we should execute
                    # its checks for the declaration token. The point is that without
                    # a known definition we have no Function.argument list required
                    # for declaration check.
                    checkDefinitionArgumentsViolations(func, startCall, endCall)

        # Check arguments in pointer declarations
        for var in data.variables:
            if not var.isPointer:
                continue

            if var.nameToken is None:
                continue

            rawTokensFollowingPtr = getFollowingRawTokens(rawTokens, var.nameToken, 3)
            if len(rawTokensFollowingPtr) != 3:
                continue

            # Compliant:           returnType (*ptrName) ( ArgType )
            # Non-compliant:       returnType (*ptrName) ( )
            if (rawTokensFollowingPtr[0].str == ')' and
                    rawTokensFollowingPtr[1].str == '(' and
                    rawTokensFollowingPtr[2].str == ')'):
                self.reportError(var.nameToken, 8, 2)

    def insert_in_dict(self, dict_name,key, value):
        if key not in dict_name:
            dict_name[key] = []
        dict_name[key].append(value)
    def misra_8_4(self, cfg):
        for func in cfg.functions:
            if func.isStatic:
                continue
            if func.token is None:
                continue
            if not is_source_file(func.token.file):
                continue
            if func.token != func.tokenDef:
                continue
            if func.tokenDef.str == 'main':
                continue
            self.reportError(func.tokenDef, 8, 4)
        extern_var_with_def = {}
        extern_var_without_def = {}
        for var in cfg.variables:
            if not var.isGlobal:
                continue
            if var.isStatic:
                continue
            if var.nameToken is None:
                continue
            tok = var.nameToken
            if tok.next.str == ";":
                if tok.next.isSplittedVarDeclEq:
                    self.insert_in_dict(extern_var_with_def, tok.str, tok)
                else:
                    self.insert_in_dict(extern_var_without_def, tok.str, tok)
            else:
                self.insert_in_dict(extern_var_without_def, var.nameToken.str, var.nameToken)

        for var in extern_var_with_def:
            if var not in extern_var_without_def:
                for t in extern_var_with_def[var]:
                    self.reportError(t, 8, 4)

        for var_str, var_tok in extern_var_without_def.items():
            warn = True
            if var_str not in extern_var_with_def:
                for t in var_tok:
                    if t.variable.isExtern:
                        warn = False
                        break
                if warn:
                    for t in var_tok:
                        self.reportError(t, 8, 4)

    def misra_8_5(self, dumpfile, cfg):
        self._save_ctu_summary_identifiers(dumpfile, cfg)

    def misra_8_6(self, dumpfile, cfg):
        self._save_ctu_summary_identifiers(dumpfile, cfg)

    def misra_8_7(self, dumpfile, cfg):
        self._save_ctu_summary_usage(dumpfile, cfg)

    def misra_8_8(self, cfg):
        vars = {}
        for var in cfg.variables:
            if var.access != 'Global':
                continue
            if var.nameToken is None:
                continue
            varname = var.nameToken.str
            if varname in vars:
                vars[varname].append(var)
            else:
                vars[varname] = [var]
        for varname, varlist in vars.items():
            static_var = None
            extern_var = None
            for var in varlist:
                if var.isStatic:
                    static_var = var
                elif var.isExtern:
                    extern_var = var
            if static_var and extern_var:
                self.reportError(extern_var.nameToken, 8, 8)

    def misra_8_9(self, cfg):
        variables = {}
        for scope in cfg.scopes:
            if scope.type != 'Function':
                continue
            variables_used_in_scope = []
            tok = scope.bodyStart
            while tok != scope.bodyEnd:
                if tok.variable and tok.variable.access == 'Global' and tok.variable.isStatic:
                    if tok.variable not in variables_used_in_scope:
                        variables_used_in_scope.append(tok.variable)
                tok = tok.next
            for var in variables_used_in_scope:
                if var in variables:
                    variables[var] += 1
                else:
                    variables[var] = 1
        for var, count in variables.items():
            if count == 1:
                self.reportError(var.nameToken, 8, 9)


    def misra_8_10(self, cfg):
        for func in cfg.functions:
            if func.isInlineKeyword and not func.isStatic:
                self.reportError(func.tokenDef, 8, 10)

    def misra_8_11(self, data):
        for var in data.variables:
            if var.isExtern and simpleMatch(var.nameToken.next, '[ ]') and var.nameToken.scope.type == 'Global':
                self.reportError(var.nameToken, 8, 11)

    def misra_8_12(self, data):
        for scope in data.scopes:
            if scope.type != 'Enum':
                continue
            enum_values = []
            implicit_enum_values = []
            e_token = scope.bodyStart.next
            while e_token != scope.bodyEnd:
                if e_token.str == '(':
                    e_token = e_token.link
                    continue
                if e_token.previous.str not in ',{':
                    e_token = e_token.next
                    continue
                if e_token.isName and e_token.values and e_token.valueType and e_token.valueType.typeScope == scope:
                    token_values = [v.intvalue for v in e_token.values]
                    enum_values += token_values
                    if e_token.next.str != "=":
                        implicit_enum_values += token_values
                e_token = e_token.next
            for implicit_enum_value in implicit_enum_values:
                if enum_values.count(implicit_enum_value) != 1:
                    self.reportError(scope.bodyStart, 8, 12)

    def misra_8_14(self, rawTokens):
        for token in rawTokens:
            if token.str == 'restrict':
                self.reportError(token, 8, 14)

    def misra_9_2(self, data):
        misra_9.misra_9_x(self, data, 902)

    def misra_9_3(self, data):
        misra_9.misra_9_x(self, data, 903)

    def misra_9_4(self, data):
        misra_9.misra_9_x(self, data, 904)

    def misra_9_5(self, data, rawTokens):
        misra_9.misra_9_x(self, data, 905, rawTokens)
        #for token in rawTokens:
        #    if simpleMatch(token, '[ ] = { ['):
        #        self.reportError(token, 9, 5)

    def misra_10_1(self, data):
        for token in data.tokenlist:
            if not token.isOp:
                continue

            for t1, t2 in itertools.product(
                    list(getTernaryOperandsRecursive(token.astOperand1) or [token.astOperand1]),
                    list(getTernaryOperandsRecursive(token.astOperand2) or [token.astOperand2]),
            ):
                e1 = getEssentialTypeCategory(t1)
                e2 = getEssentialTypeCategory(t2)
                if not e1 or not e2:
                    continue
                if token.str in ('<<', '>>'):
                    if not isUnsignedType(e1):
                        self.reportError(token, 10, 1)
                    elif not isUnsignedType(e2) and not token.astOperand2.isNumber:
                        self.reportError(token, 10, 1)
                elif token.str in ('~', '&', '|', '^'):
                    e1_et = getEssentialType(token.astOperand1)
                    e2_et = getEssentialType(token.astOperand2)
                    if e1_et == 'char' or e2_et == 'char':
                        self.reportError(token, 10, 1)

    def misra_10_2(self, data):
        def isEssentiallySignedOrUnsigned(op):
            e = getEssentialType(op)
            return e and (e.split(' ')[0] in ('unsigned', 'signed'))

        def isEssentiallyChar(op):
            if op is None:
                return False
            if op.str == '+':
                return isEssentiallyChar(op.astOperand1) or isEssentiallyChar(op.astOperand2)
            return op.isChar

        for token in data.tokenlist:
            if token.str not in ('+', '-'):
                continue

            if (not isEssentiallyChar(token.astOperand1)) and (not isEssentiallyChar(token.astOperand2)):
                continue

            if token.str == '+':
                if isEssentiallyChar(token.astOperand1) and not isEssentiallySignedOrUnsigned(token.astOperand2):
                    self.reportError(token, 10, 2)
                if isEssentiallyChar(token.astOperand2) and not isEssentiallySignedOrUnsigned(token.astOperand1):
                    self.reportError(token, 10, 2)

            if token.str == '-':
                e1 = getEssentialType(token.astOperand1)
                if e1 and e1.split(' ')[-1] != 'char':
                    self.reportError(token, 10, 2)
                if not isEssentiallyChar(token.astOperand2) and not isEssentiallySignedOrUnsigned(token.astOperand2):
                    self.reportError(token, 10, 2)

    def misra_10_3(self, cfg):
        def get_category(essential_type):
            if essential_type:
                if essential_type in ('bool', 'char'):
                    return essential_type
                if essential_type.split(' ')[-1] in ('float', 'double'):
                    return 'floating'
                if essential_type.split(' ')[0] in ('unsigned', 'signed'):
                    return essential_type.split(' ')[0]
            return None

        for tok in cfg.tokenlist:
            if not tok.isAssignmentOp:
                continue

            lhs = getEssentialType(tok.astOperand1)
            rhs = getEssentialType(tok.astOperand2)
            if lhs is None or rhs is None:
                continue

            find_std = cfg.standards.c if cfg.standards and cfg.standards.c else self.stdversion

            rhs_tok = tok.astOperand2
            rhs_macro_name = rhs_tok.macroName if rhs_tok else None
            rhs_spelling = rhs_macro_name if rhs_macro_name in ('true', 'false') else rhs_tok.str

            rhs_is_source_bool_literal = rhs_spelling in ('true', 'false')
            rhs_is_source_int_literal_0_1 = rhs_spelling in ('0', '1')

            if lhs == 'bool':
                if rhs_is_source_bool_literal:
                    continue
                if find_std == 'c89' and rhs_is_source_int_literal_0_1:
                    continue

            lhs_category = get_category(lhs)
            rhs_category = get_category(rhs)
            if lhs_category and rhs_category and lhs_category != rhs_category and rhs_category not in ('signed', 'unsigned'):
                self.reportError(tok, 10, 3)

            if bitsOfEssentialType(lhs) < bitsOfEssentialType(rhs):
                self.reportError(tok, 10, 3)

    def misra_10_4(self, data):
        op = {'+', '-', '*', '/', '%', '&', '|', '^', '+=', '-=', ':'}
        for token in data.tokenlist:
            if token.str not in op and not token.isComparisonOp:
                continue
            if not token.astOperand1 or not token.astOperand2:
                continue
            if not token.astOperand1.valueType or not token.astOperand2.valueType:
                continue
            if ((token.astOperand1.str in op or token.astOperand1.isComparisonOp) and
                (token.astOperand2.str in op or token.astOperand2.isComparisonOp)):
                e1, e2 = getEssentialCategorylist(token.astOperand1.astOperand2, token.astOperand2.astOperand1)
            elif token.astOperand1.str in op or token.astOperand1.isComparisonOp:
                e1, e2 = getEssentialCategorylist(token.astOperand1.astOperand2, token.astOperand2)
            elif token.astOperand2.str in op or token.astOperand2.isComparisonOp:
                e1, e2 = getEssentialCategorylist(token.astOperand1, token.astOperand2.astOperand1)
            else:
                e1, e2 = getEssentialCategorylist(token.astOperand1, token.astOperand2)
            if token.str == "+=" or token.str == "+":
                if e1 == "char" and (e2 == "signed" or e2 == "unsigned"):
                    continue
                if e2 == "char" and (e1 == "signed" or e1 == "unsigned"):
                    continue
            if token.str == "-=" or token.str == "-":
                if e1 == "char" and (e2 == "signed" or e2 == "unsigned"):
                    continue
            if e1 and e2 and (e1.find('Anonymous') != -1 and (e2 == "signed" or e2 == "unsigned")):
                continue
            if e1 and e2 and (e2.find('Anonymous') != -1 and (e1 == "signed" or e1 == "unsigned")):
                continue
            if e1 and e2 and e1 != e2:
                self.reportError(token, 10, 4)

    def misra_10_5(self, cfg):
        def _get_essential_category(token):
            essential_type = getEssentialType(token)
            #print(essential_type)
            if essential_type:
                if essential_type in ('bool', 'char'):
                    return essential_type
                if essential_type.split(' ')[-1] in ('float', 'double'):
                    return 'floating'
                if essential_type.split(' ')[0] in ('unsigned', 'signed'):
                    return essential_type.split(' ')[0]
            return None
        for token in cfg.tokenlist:
            if not isCast(token):
                continue
            to_type = _get_essential_category(token)
            #print(to_type)
            if to_type is None:
                continue
            from_type = _get_essential_category(token.astOperand1)
            #print(from_type)
            if from_type is None:
                continue
            if to_type == from_type:
                continue
            if to_type == 'bool' or from_type == 'bool':
                if token.astOperand1.isInt and token.astOperand1.getKnownIntValue() == 1:
                    # Exception
                    continue
                self.reportError(token, 10, 5)
                continue
            if to_type == 'enum':
                self.reportError(token, 10, 5)
                continue
            if from_type == 'float' and to_type == 'char':
                self.reportError(token, 10, 5)
                continue
            if from_type == 'char' and to_type == 'float':
                self.reportError(token, 10, 5)
                continue

    def misra_10_6(self, data):
        for token in data.tokenlist:
            if token.str != '=' or not token.astOperand1 or not token.astOperand2:
                continue
            if not is_composite_expr(token.astOperand2):
                continue
            vt1 = token.astOperand1.valueType
            vt2 = token.astOperand2.valueType
            if not vt1 or vt1.pointer > 0:
                continue
            if not vt2 or vt2.pointer > 0:
                continue
            try:
                if isCast(token.astOperand2):
                    e = vt2.type
                else:
                    e = getEssentialType(token.astOperand2)
                if not e:
                    continue
                if e == "char" and vt1.type == "int":
                    # When arithmetic operations are performed on char values, they are usually promoted to int
                    continue
                lhsbits = vt1.bits if vt1.bits else bitsOfEssentialType(vt1.type)
                if lhsbits > bitsOfEssentialType(e):
                    self.reportError(token, 10, 6)
            except ValueError:
                pass

    def misra_10_7(self, cfg):
        for token in cfg.tokenlist:
            if token.astOperand1 is None or token.astOperand2 is None:
                continue
            if not token.isArithmeticalOp:
                continue
            if not is_composite_expr(token):
                continue
            parent = token.astParent
            if parent is None:
                continue
            if not parent.isArithmeticalOp:
                if not parent.isAssignmentOp:
                    continue
                if parent.str == '=':
                    continue
            token_type = getEssentialType(token)
            if token_type is None:
                continue
            sibling = parent.astOperand1 if (token == parent.astOperand2) else parent.astOperand2
            sibling_type = getEssentialType(sibling)
            if sibling_type is None:
                continue
            b1 = bitsOfEssentialType(token_type)
            b2 = bitsOfEssentialType(sibling_type)
            if b1 > 0 and b1 < b2:
                self.reportError(token, 10, 7)

    def misra_10_8(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            if not token.valueType or token.valueType.pointer > 0:
                continue
            if not token.astOperand1.valueType or token.astOperand1.valueType.pointer > 0:
                continue
            if not token.astOperand1.astOperand1:
                continue
            if token.astOperand1.str not in ('+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":", '~'):
                continue
            if token.astOperand1.str != '~' and not token.astOperand1.astOperand2:
                continue
            if token.astOperand1.str == '~':
                e2 = getEssentialTypeCategory(token.astOperand1.astOperand1)
            else:
                e2, e3 = getEssentialCategorylist(token.astOperand1.astOperand1, token.astOperand1.astOperand2)
                if e2 != e3:
                    continue
            e1 = getEssentialTypeCategory(token)
            if e1 != e2:
                self.reportError(token, 10, 8)
            else:
                try:
                    e = getEssentialType(token.astOperand1)
                    if not e:
                        continue
                    if bitsOfEssentialType(token.valueType.type) > bitsOfEssentialType(e):
                        self.reportError(token, 10, 8)
                except ValueError:
                    pass

    def misra_11_1(self, data):
        for token in data.tokenlist:
            to_from = get_type_conversion_to_from(token)
            if to_from is None:
                continue
            from_type = get_function_pointer_type(to_from[1])
            if from_type is None:
                continue
            to_type = get_function_pointer_type(to_from[0])
            if to_type is None or to_type != from_type:
                self.reportError(token, 11, 1)

    def misra_11_2(self, data):
        def get_pointer_type(type_token):
            while type_token and (type_token.str in ('const', 'struct')):
                type_token = type_token.next
            if type_token is None:
                return None
            if not type_token.isName:
                return None
            return type_token if (type_token.next and type_token.next.str == '*') else None

        incomplete_types = []

        for token in data.tokenlist:
            if token.str == 'struct' and token.next and token.next.next and token.next.isName and token.next.next.str == ';':
                incomplete_types.append(token.next.str)
            to_from = get_type_conversion_to_from(token)
            if to_from is None:
                continue
            to_pointer_type_token = get_pointer_type(to_from[0])
            if to_pointer_type_token is None:
                continue
            from_pointer_type_token = get_pointer_type(to_from[1])
            if from_pointer_type_token is None:
                continue
            if to_pointer_type_token.str == from_pointer_type_token.str:
                continue
            if from_pointer_type_token.typeScope is None and (from_pointer_type_token.str in incomplete_types):
                self.reportError(token, 11, 2)
            elif to_pointer_type_token.typeScope is None and (to_pointer_type_token.str in incomplete_types):
                self.reportError(token, 11, 2)

    def misra_11_3(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt1.type == 'void' or vt2.type == 'void':
                continue
            if (vt1.pointer > 0 and vt1.type == 'record' and
                    vt2.pointer > 0 and vt2.type == 'record' and
                    vt1.typeScopeId != vt2.typeScopeId):
                self.reportError(token, 11, 3)
            elif (vt1.pointer == vt2.pointer and vt1.pointer > 0 and
                  vt1.type != vt2.type and vt1.type != 'char'):
                self.reportError(token, 11, 3)

    def misra_11_4(self, data):
        # Get list of macro definitions
        macros = {}
        for directive in data.directives:
            #define X ((peripheral_t *)0x40000U)
            res = re.match(r'#define ([A-Za-z0-9_]+).*', directive.str)
            if res:
                if res.group(1) in macros:
                    macros[res.group(1)].append(directive)
                else:
                    macros[res.group(1)] = [directive]

        # If macro definition is non-compliant then warn about the macro definition instead of
        # the macro usages. To reduce diagnostics for a non-compliant macro.
        bad_macros = []
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt2.pointer > 0 and vt1.pointer == 0 and (vt1.isIntegral() or vt1.isEnum()) and vt2.type != 'void':
                self.reportError(token, 11, 4)
            elif vt1.pointer > 0 and vt2.pointer == 0 and (vt2.isIntegral() or vt2.isEnum()) and vt1.type != 'void':
                if token.macroName is not None and \
                   token.macroName == token.astOperand1.macroName and \
                   token.astOperand1.isInt and \
                   token.link.previous.str == '*' and \
                   token.macroName == token.link.previous.macroName and \
                   token.macroName in macros and \
                   len(macros[token.macroName]) == 1:
                    if token.macroName not in bad_macros:
                        bad_macros.append(token.macroName)
                        self.reportError(macros[token.macroName][0], 11, 4)
                    continue
                self.reportError(token, 11, 4)

    def misra_11_5(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                if token.astOperand1 and token.astOperand2 and token.str == "=" and token.next.str != "(":
                    vt1 = token.astOperand1.valueType
                    vt2 = token.astOperand2.valueType
                    if not vt1 or not vt2:
                        continue
                    if vt1.pointer > 0 and vt1.type != 'void' and vt2.pointer == vt1.pointer and vt2.type == 'void':
                        self.reportError(token, 11, 5)
                continue
            if token.astOperand1.astOperand1 and token.astOperand1.astOperand1.str in (
            'malloc', 'calloc', 'realloc', 'free'):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt1.pointer > 0 and vt1.type != 'void' and vt2.pointer == vt1.pointer and vt2.type == 'void':
                self.reportError(token, 11, 5)

    def misra_11_6(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt1.pointer == 1 and vt1.type == 'void' and vt2.pointer == 0 and token.astOperand1.getKnownIntValue() != 0:
                self.reportError(token, 11, 6)
            elif vt1.pointer == 0 and vt1.type != 'void' and vt2.pointer == 1 and vt2.type == 'void':
                self.reportError(token, 11, 6)

    def misra_11_7(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if token.astOperand1.astOperand1:
                continue
            if (vt2.pointer > 0 and vt1.pointer == 0 and
                    not vt1.isIntegral() and not vt1.isEnum() and
                    vt1.type != 'void'):
                self.reportError(token, 11, 7)
            elif (vt1.pointer > 0 and vt2.pointer == 0 and
                  not vt2.isIntegral() and not vt2.isEnum() and
                  vt1.type != 'void'):
                self.reportError(token, 11, 7)

    def misra_11_8(self, data):
        # TODO: reuse code in CERT-EXP05
        for token in data.tokenlist:
            if isCast(token):
                # C-style cast
                if not token.valueType:
                    continue
                if not token.astOperand1.valueType:
                    continue
                if token.valueType.pointer == 0:
                    continue
                if token.astOperand1.valueType.pointer == 0:
                    continue
                const1 = token.valueType.constness
                const2 = token.astOperand1.valueType.constness
                if (const1 % 2) < (const2 % 2):
                    self.reportError(token, 11, 8)

            elif token.str == '(' and token.astOperand1 and token.astOperand2 and token.astOperand1.function:
                # Function call
                function = token.astOperand1.function
                arguments = getArguments(token)
                for argnr, argvar in function.argument.items():
                    if argnr < 1 or argnr > len(arguments):
                        continue
                    if not argvar.isPointer:
                        continue
                    argtok = arguments[argnr - 1]
                    if not argtok.valueType:
                        continue
                    if argtok.valueType.pointer == 0:
                        continue
                    const1 = argvar.constness
                    const2 = arguments[argnr - 1].valueType.constness
                    if (const1 % 2) < (const2 % 2):
                        self.reportError(token, 11, 8)

    def misra_11_9(self, data):
        for token in data.tokenlist:
            if token.astOperand1 and token.astOperand2 and token.str in ["=", "==", "!=", "?", ":"]:
                vt1 = token.astOperand1.valueType
                vt2 = token.astOperand2.valueType
                if not vt1 or not vt2:
                    continue
                if vt1.pointer > 0 and vt2.pointer == 0 and token.astOperand2.str == "NULL":
                    continue
                if (token.astOperand2.values and vt1.pointer > 0 and
                        vt2.pointer == 0 and token.astOperand2.values):
                    if token.astOperand2.getValue(0):
                        self.reportError(token, 11, 9)

    def misra_12_1_sizeof(self, rawTokens):
        state = 0
        compiled = re.compile(r'^[a-zA-Z_]')
        for tok in rawTokens:
            if tok.str.startswith('//') or tok.str.startswith('/*'):
                continue
            if tok.str == 'sizeof':
                state = 1
            elif state == 1:
                if compiled.match(tok.str):
                    state = 2
                else:
                    state = 0
            elif state == 2:
                if tok.str in ('+', '-', '*', '/', '%'):
                    self.reportError(tok, 12, 1)
                else:
                    state = 0

    def misra_12_1(self, data):
        for token in data.tokenlist:
            p = getPrecedence(token)
            if p < 2 or p > 12:
                continue
            p1 = getPrecedence(token.astOperand1)
            if p < p1 <= 12 and numberOfParentheses(token.astOperand1, token):
                self.reportError(token, 12, 1)
                continue
            p2 = getPrecedence(token.astOperand2)
            if p < p2 <= 12 and numberOfParentheses(token, token.astOperand2):
                self.reportError(token, 12, 1)
                continue

    def misra_12_2(self, data):
        for token in data.tokenlist:
            if not (token.str in ('<<', '>>')):
                continue
            if (not token.astOperand2) or (not token.astOperand2.values):
                continue
            maxval = 0
            for val in token.astOperand2.values:
                if val.intvalue and val.intvalue > maxval:
                    maxval = val.intvalue
            if maxval == 0:
                continue
            sz = bitsOfEssentialType(getEssentialType(token.astOperand1))
            if sz <= 0:
                continue
            if maxval >= sz:
                self.reportError(token, 12, 2)

    def misra_12_3(self, data):
        for token in data.tokenlist:
            if token.str == ';' and (token.isSplittedVarDeclComma is True):
                self.reportError(token, 12, 3)
            if token.str == ',' and token.astParent and token.astParent.str == ';':
                self.reportError(token, 12, 3)
            if token.str == ',' and token.astParent is None:
                if token.scope.type in ('Class', 'Struct'):
                    # Is this initlist..
                    tok = token
                    while tok and tok.str == ',':
                        tok = tok.next
                        if tok and tok.next and tok.isName and tok.next.str == '(':
                            tok = tok.next.link.next
                    if tok.str == '{':
                        # This comma is used in initlist, do not warn
                        continue
                prev = token.previous
                while prev:
                    if prev.str == ';':
                        self.reportError(token, 12, 3)
                        break
                    if prev.str in '({[':
                        break
                    if prev.str in ')}]':
                        prev = prev.link
                    prev = prev.previous

    def misra_12_4_check_expr(self, expr):
        if not expr.astOperand2 or not expr.astOperand1:
            return
        if expr.valueType is None:
            return
        if expr.valueType.sign is None or expr.valueType.sign != 'unsigned':
            return
        if expr.valueType.pointer > 0:
            return
        if not expr.valueType.isIntegral():
            return
        op1 = expr.astOperand1.getKnownIntValue()
        if op1 is None:
            return
        op2 = expr.astOperand2.getKnownIntValue()
        if op2 is None:
            return
        bits = bitsOfEssentialType('unsigned ' + expr.valueType.type)
        if bits <= 0 or bits >= 64:
            return
        max_value = (1 << bits) - 1
        if not is_constant_integer_expression(expr):
            return
        if expr.str == '+' and op1 + op2 > max_value:
            self.reportError(expr, 12, 4)
        elif expr.str == '-' and op1 - op2 < 0:
            self.reportError(expr, 12, 4)
        elif expr.str == '*' and op1 * op2 > max_value:
            self.reportError(expr, 12, 4)
    def misra_12_4(self, cfg):
        if not cfg.tokenlist:
            return
        expr = cfg.tokenlist[0]
        while expr.next:
            expr = expr.next
            if expr.str == "?" and expr.astOperand2.str == ":":
                known_value = expr.astOperand1.getKnownIntValue()
                if known_value == 1:
                    tok = expr
                    while tok != expr.astOperand2:
                        self.misra_12_4_check_expr(tok)
                        tok = tok.next
                    expr = tok
                    while expr.str not in (";", "{", "}"):
                        expr = expr.next
                    continue
                if known_value == 0:
                    expr = expr.astOperand2
            self.misra_12_4_check_expr(expr)


    def misra_13_1(self, data):
        for token in data.tokenlist:
            if simpleMatch(token, ") {") and token.next.astParent == token.link:
                pass
            elif not simpleMatch(token, '= {'):
                continue
            init = token.next
            end = init.link
            if not end:
                continue  # syntax is broken

            tn = init
            while tn and tn != end:
                if tn.str == '[' and tn.link:
                    tn = tn.link
                    if tn and tn.next and tn.next.str == '=':
                        tn = tn.next.next
                        continue
                    break
                if tn.str == '.' and tn.next and tn.next.isName:
                    tn = tn.next
                    if tn.next and tn.next.str == '=':
                        tn = tn.next.next
                    continue
                if tn.str in {'++', '--'} or tn.isAssignmentOp:
                    self.reportError(init, 13, 1)
                tn = tn.next

    def misra_13_3(self, data):
        for token in data.tokenlist:
            if token.str not in ('++', '--'):
                continue
            astTop = token
            while astTop.astParent and astTop.astParent.str not in (',', ';'):
                astTop = astTop.astParent
            if countSideEffects(astTop) >= 2:
                self.reportError(astTop, 13, 3)

    def misra_13_4(self, data):
        for token in data.tokenlist:
            if token.str != '=':
                continue
            if not token.astParent:
                continue
            if (token.astOperand1 is None) or (token.astOperand2 is None):
                continue
            if token.astOperand1.str == '[' and token.astOperand1.previous.str in ('{', ','):
                continue
            if not (token.astParent.str in [',', ';', '{']):
                self.reportError(token, 13, 4)

    def misra_13_5(self, data):
        for token in data.tokenlist:
            if token.isLogicalOp and countSideEffectsRecursive(token.astOperand2) > 0:
                self.reportError(token, 13, 5)

    def misra_13_6(self, data):
        for token in data.tokenlist:
            if token.str == 'sizeof' and countSideEffectsRecursive(token.next) > 0:
                self.reportError(token, 13, 6)

    def misra_14_1(self, data):
        for token in data.tokenlist:
            if token.str == 'for':
                exprs = getForLoopExpressions(token)
                if not exprs:
                    continue
                for counter in findCounterTokens(exprs[1]):
                    if counter.valueType and counter.valueType.isFloat():
                        self.reportError(token, 14, 1)
            elif token.str == 'while':
                if isFloatCounterInWhileLoop(token):
                    self.reportError(token, 14, 1)

    def misra_14_2(self, data):
        for token in data.tokenlist:
            if token.str == 'for':
                expressions = getForLoopExpressions(token)
                if not expressions:
                    continue
                if expressions[0] and not expressions[0].isAssignmentOp:
                    if expressions[0].str != "(" or not expressions[0].previous.isName:
                        self.reportError(token, 14, 2)
                if countSideEffectsRecursive(expressions[1]) > 0:
                    self.reportError(token, 14, 2)
                if countSideEffectsRecursive(expressions[2]) > 1:
                    self.reportError(token, 14, 2)

                counter_vars_first_clause, counter_vars_exit_modified = getForLoopCounterVariables(token, data)
                if len(counter_vars_exit_modified) == 0:
                    # if it's not possible to identify a loop counter, all 3 clauses must be empty
                    for idx in range(len(expressions)):
                        if expressions[idx]:
                            self.reportError(token, 14, 2)
                            break
                elif len(counter_vars_exit_modified) > 1:
                    # there shall be a single loop counter
                    self.reportError(token, 14, 2)
                else: # len(counter_vars_exit_modified) == 1:
                    loop_counter = counter_vars_exit_modified.pop()
                    # if the first clause is not empty, then it shall (declare and) initialize the loop counter
                    if expressions[0] is not None and loop_counter not in counter_vars_first_clause:
                        self.reportError(token, 14, 2)

                    # Inspect modification of loop counter in loop body
                    body_scope = token.next.link.next.scope
                    if not body_scope:
                        continue
                    tn = body_scope.bodyStart
                    while tn and tn != body_scope.bodyEnd:
                        if tn.variable == loop_counter:
                            if tn.next:
                                # TODO: Check modifications in function calls
                                if countSideEffectsRecursive(tn.next) > 0:
                                    self.reportError(tn, 14, 2)
                        tn = tn.next

    def misra_14_4(self, data):
        for token in data.tokenlist:
            if token.str != '(':
                continue
            if not token.astOperand1 or not (token.astOperand1.str in ['if', 'while']):
                continue
            if isBoolExpression(token.astOperand2):
                continue
            if token.astOperand2.valueType:
                self.reportError(token, 14, 4)

    def misra_15_1(self, data):
        for token in data.tokenlist:
            if token.str == "goto":
                self.reportError(token, 15, 1)

    def misra_15_2(self, data):
        for token in data.tokenlist:
            if token.str != 'goto':
                continue
            if (not token.next) or (not token.next.isName):
                continue
            if not findGotoLabel(token):
                self.reportError(token, 15, 2)

    def misra_15_3(self, data):
        for token in data.tokenlist:
            if token.str != 'goto':
                continue
            if (not token.next) or (not token.next.isName):
                continue
            tok = findGotoLabel(token)
            if not tok:
                continue
            scope = token.scope
            while scope and scope != tok.scope:
                scope = scope.nestedIn
            if not scope:
                self.reportError(token, 15, 3)
            # Jump crosses from one switch-clause to another is non-compliant
            elif scope.type == 'Switch':
                # Search for start of a current case block
                tcase_start = token
                while tcase_start and tcase_start.str not in ('case', 'default'):
                    tcase_start = tcase_start.previous
                # Make sure that goto label doesn't occurs in the other
                # switch-clauses
                if tcase_start:
                    t = scope.bodyStart
                    in_this_case = False
                    while t and t != scope.bodyEnd:
                        if t == tcase_start:
                            in_this_case = True
                        if in_this_case and t.str not in ('case', 'default'):
                            in_this_case = False
                        if t == tok and not in_this_case:
                            self.reportError(token, 15, 3)
                            break
                        t = t.next

    def misra_15_4(self, data):
        # Return a list of scopes affected by a break or goto
        def getLoopsAffectedByBreak(knownLoops, scope, isGoto):
            if scope and scope.type and scope.type not in ['Global', 'Function']:
                if not isGoto and scope.type == 'Switch':
                    return
                if scope.type in ['For', 'While', 'Do']:
                    knownLoops.append(scope)
                    if not isGoto:
                        return
                getLoopsAffectedByBreak(knownLoops, scope.nestedIn, isGoto)

        loopWithBreaks = {}
        for token in data.tokenlist:
            if token.str not in ['break', 'goto']:
                continue

            affectedLoopScopes = []
            getLoopsAffectedByBreak(affectedLoopScopes, token.scope, token.str == 'goto')
            for scope in affectedLoopScopes:
                if scope in loopWithBreaks:
                    loopWithBreaks[scope] += 1
                else:
                    loopWithBreaks[scope] = 1

        for scope, breakCount in loopWithBreaks.items():
            if breakCount > 1:
                self.reportError(scope.bodyStart, 15, 4)

    def misra_15_5(self, data):
        for token in data.tokenlist:
            if token.str == 'return' and token.scope.type != 'Function':
                self.reportError(token, 15, 5)

    def misra_15_6(self, rawTokens):
        state = 0
        indent = 0
        tok1 = None
        def tokAt(tok,i):
            while i < 0 and tok:
                tok = tok.previous
                if tok.str.startswith('//') or tok.str.startswith('/*'):
                    continue
                i += 1
            while i > 0 and tok:
                tok = tok.next
                if tok.str.startswith('//') or tok.str.startswith('/*'):
                    continue
                i -= 1
            return tok

        def strtokens(tok, i1, i2):
            tok1 = tokAt(tok, i1)
            tok2 = tokAt(tok, i2)
            tok = tok1
            s = ''
            while tok != tok2:
                if tok.str.startswith('//') or tok.str.startswith('/*'):
                    tok = tok.next
                    continue
                s += ' ' + tok.str
                tok = tok.next
            s += ' ' + tok.str
            return s[1:]

        for token in rawTokens:
            if token.str in ['if', 'for', 'while']:
                if strtokens(token,-1,0) == '# if':
                    continue
                if strtokens(token,-1,0) == "} while":
                    # is there a 'do { .. } while'?
                    start = rawlink(tokAt(token,-1))
                    if start and strtokens(start, -1, 0) == 'do {':
                        continue
                if state == 2:
                    self.reportError(tok1, 15, 6)
                state = 1
                indent = 0
                tok1 = token
            elif token.str == 'else':
                if strtokens(token,-1,0) == '# else':
                    continue
                if strtokens(token,0,1) == 'else if':
                    continue
                if state == 2:
                    self.reportError(tok1, 15, 6)
                state = 2
                indent = 0
                tok1 = token
            elif state == 1:
                if indent == 0 and token.str != '(':
                    state = 0
                    continue
                if token.str == '(':
                    indent = indent + 1
                elif token.str == ')':
                    if indent == 0:
                        state = 0
                    elif indent == 1:
                        state = 2
                    indent = indent - 1
            elif state == 2:
                if token.str.startswith('//') or token.str.startswith('/*'):
                    continue
                state = 0
                if token.str not in ('{', '#'):
                    self.reportError(tok1, 15, 6)

    def misra_15_7(self, data):
        for scope in data.scopes:
            if scope.type != 'Else':
                continue
            if not simpleMatch(scope.bodyStart, '{ if ('):
                continue
            if scope.bodyStart.column > 0:
                continue
            tok = scope.bodyStart.next.next.link
            if not simpleMatch(tok, ') {'):
                continue
            tok = tok.next.link
            if not simpleMatch(tok, '} else'):
                self.reportError(tok, 15, 7)

    def misra_16_1(self, cfg):
        for scope in cfg.scopes:
            if scope.type != 'Switch':
                continue
            in_case_or_default = False
            tok = scope.bodyStart.next
            while tok != scope.bodyEnd:
                if not in_case_or_default:
                    if tok.str not in ('case', 'default'):
                        self.reportError(tok, 16, 1)
                    else:
                        in_case_or_default = True
                else:
                    if simpleMatch(tok, 'break ;'):
                        in_case_or_default = False
                        tok = tok.next
                if tok.str == '{':
                    tok = tok.link
                    if tok.scope.type == 'Unconditional' and simpleMatch(tok.previous.previous, 'break ;'):
                        in_case_or_default = False
                tok = tok.next

    def misra_16_2(self, data):
        for token in data.tokenlist:
            if token.str == 'case' and token.scope.type != 'Switch':
                self.reportError(token, 16, 2)

    def misra_16_3(self, rawTokens):
        STATE_NONE = 0  # default state, not in switch case/default block
        STATE_BREAK = 1  # break/comment is seen but not its ';'
        STATE_OK = 2  # a case/default is allowed (we have seen 'break;'/'comment'/'{'/attribute)
        STATE_SWITCH = 3  # walking through switch statement scope

        directive = None
        state = STATE_NONE
        end_switch_token = None  # end '}' for the switch scope
        for token in rawTokens:
            if simpleMatch(token, '# define') or simpleMatch(token, '# pragma'):
                directive = token
            if directive:
                if token.linenr != directive.linenr:
                    directive = None
                else:
                    continue

            # Find switch scope borders
            if token.str == 'switch':
                state = STATE_SWITCH
            if state == STATE_SWITCH:
                if token.str == '{':
                    end_switch_token = findRawLink(token)
                else:
                    continue

            if token.str == 'break' or token.str == 'return' or token.str == 'throw':
                state = STATE_BREAK
            elif token.str == ';':
                if state == STATE_BREAK:
                    state = STATE_OK
                elif token.next and token.next == end_switch_token:
                    self.reportError(token.next, 16, 3)
                else:
                    state = STATE_NONE
            elif token.str.startswith('/*') or token.str.startswith('//'):
                if 'fallthrough' in token.str.lower():
                    state = STATE_OK
            elif simpleMatch(token, '[ [ fallthrough ] ] ;'):
                state = STATE_BREAK
            elif token.str == '{':
                state = STATE_OK
            elif token.str == '}' and state == STATE_OK:
                # is this {} an unconditional block of code?
                prev = findRawLink(token)
                if prev:
                    prev = prev.previous
                    while prev and prev.str[:2] in ('//', '/*'):
                        prev = prev.previous
                if (prev is None) or (prev.str not in ':;{}'):
                    state = STATE_NONE
            elif token.str == 'case' or token.str == 'default':
                if state != STATE_OK:
                    self.reportError(token, 16, 3)
                state = STATE_OK

    def misra_16_4(self, data):
        for token in data.tokenlist:
            if token.str != 'switch':
                continue
            if not simpleMatch(token, 'switch ('):
                continue
            if not simpleMatch(token.next.link, ') {'):
                continue
            startTok = token.next.link.next
            tok = startTok.next
            while tok and tok.str != '}':
                if tok.str == '{':
                    tok = tok.link
                elif tok.str == 'default':
                    break
                tok = tok.next
            if tok and tok.str != 'default':
                self.reportError(token, 16, 4)

    def misra_16_5(self, data):
        for token in data.tokenlist:
            if token.str != 'default':
                continue
            if token.previous and (token.previous.str == '{'):
                continue
            tok2 = token
            while tok2:
                if tok2.str in ('}', 'case'):
                    break
                if tok2.str == '{':
                    tok2 = tok2.link
                tok2 = tok2.next
            if tok2 and tok2.str == 'case':
                self.reportError(token, 16, 5)

    def misra_16_6(self, data):
        for token in data.tokenlist:
            if not (simpleMatch(token, 'switch (') and simpleMatch(token.next.link, ') {')):
                continue
            tok = token.next.link.next.next
            count = 0
            while tok:
                if tok.str in ['break', 'return', 'throw']:
                    count = count + 1
                elif tok.str == '{':
                    tok = tok.link
                    if isNoReturnScope(tok):
                        count = count + 1
                elif tok.str == '}':
                    break
                tok = tok.next
            if count < 2:
                self.reportError(token, 16, 6)

    def misra_16_7(self, data):
        for token in data.tokenlist:
            if simpleMatch(token, 'switch (') and isBoolExpression(token.next.astOperand2):
                self.reportError(token, 16, 7)

    def misra_17_1(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token, data.standards.c) and token.astOperand1.str in (
            'va_list', 'va_arg', 'va_start', 'va_end', 'va_copy'):
                self.reportError(token, 17, 1)
            elif token.str == 'va_list':
                self.reportError(token, 17, 1)

    def misra_17_2(self, data):
        # find recursions..
        def find_recursive_call(search_for_function, direct_call, calls_map, visited=None):
            if visited is None:
                visited = set()
            if direct_call == search_for_function:
                return True
            for indirect_call in calls_map.get(direct_call, []):
                if indirect_call == search_for_function:
                    return True
                if indirect_call in visited:
                    # This has already been handled
                    continue
                visited.add(indirect_call)
                if find_recursive_call(search_for_function, indirect_call, calls_map, visited):
                    return True
            return False

        # List functions called in each function
        function_calls = {}
        for scope in data.scopes:
            if scope.type != 'Function':
                continue
            calls = []
            tok = scope.bodyStart
            while tok != scope.bodyEnd:
                tok = tok.next
                if not isFunctionCall(tok, data.standards.c):
                    continue
                f = tok.astOperand1.function
                if f is not None and f not in calls:
                    calls.append(f)
            function_calls[scope.function] = calls

        # Report warnings for all recursions..
        for func in function_calls:
            for call in function_calls[func]:
                if not find_recursive_call(func, call, function_calls):
                    # Function call is not recursive
                    continue
                # Warn about all functions calls..
                for scope in data.scopes:
                    if scope.type != 'Function' or scope.function != func:
                        continue
                    tok = scope.bodyStart
                    while tok != scope.bodyEnd:
                        if tok.function and tok.function == call:
                            self.reportError(tok, 17, 2)
                        tok = tok.next

    def misra_17_3(self, cfg):
        # Check for Clang warnings related to implicit function declarations
        for w in cfg.clang_warnings:
            if w['message'].endswith('[-Wimplicit-function-declaration]'):
                self.reportError(cppcheckdata.Location(w), 17, 3)

        # Additional check for implicit function calls in expressions
        for token in cfg.tokenlist:
            if token.isName and token.function is None and token.valueType is None:
                if token.next and token.next.str == "(" and token.next.valueType is None:
                    if token.next.next.str == "*" and \
                        token.next.next.next.isName and token.next.next.next.valueType is not None and \
                        token.next.next.next.valueType.pointer > 0 :
                        # this is a function pointer definition the tokens look like this int16_t ( * misra_8_2_p_a ) ()
                        # and the int16_t causes the detection as the '(' follows
                        continue
                    if not isKeyword(token.str,cfg.standards.c) and not isStdLibId(token.str,cfg.standards.c):
                        self.reportError(token, 17, 3)

    def misra_config(self, data):
        for var in data.variables:
            if not var.isArray or var.nameToken is None or not cppcheckdata.simpleMatch(var.nameToken.next, '['):
                continue
            tok = var.nameToken.next
            while tok.str == '[':
                sz = tok.astOperand2
                if sz and sz.getKnownIntValue() is None:
                    has_var = False
                    unknown_constant = False
                    tokens = [sz]
                    while len(tokens) > 0:
                        t = tokens[-1]
                        tokens = tokens[:-1]
                        if t:
                            if t.isName and t.getKnownIntValue() is None:
                                if t.varId or t.variable:
                                    has_var = True
                                    continue
                                unknown_constant = True
                                self.report_config_error(tok, 'Unknown constant {}, please review configuration'.format(t.str))
                            if t.isArithmeticalOp:
                                tokens += [t.astOperand1, t.astOperand2]
                    if not unknown_constant and not has_var:
                        self.report_config_error(tok, 'Unknown array size, please review configuration')
                tok = tok.link.next

        for token in data.tokenlist:
            if token.str not in ("while", "if"):
                continue
            tok = token.next
            if token is None or tok.str != "(":
                continue
            end_token = tok.link
            while tok != end_token:
                tok = tok.next
                if tok.str == 'sizeof' and tok.next.str == '(':
                    tok = tok.next.link
                    continue
                if tok.str == "(" and tok.isCast:
                    tok = tok.link
                    continue
                if not tok.isName:
                    continue
                if tok.function or tok.variable or tok.varId or tok.valueType or tok.typeScope:
                    continue
                if tok.next.str == "(" or tok.str in ["EOF"]:
                    continue
                if isKeyword(tok.str, data.standards.c) or isStdLibId(tok.str, data.standards.c):
                    continue
                if tok.astParent is None:
                    continue
                if tok.astParent.str == "." and tok.astParent.valueType:
                    continue
                self.report_config_error(tok, "Variable '%s' is unknown" % tok.str)

    def misra_17_6(self, rawTokens):
        for token in rawTokens:
            if simpleMatch(token, '[ static'):
                self.reportError(token, 17, 6)

    def misra_17_7(self, data):
        for token in data.tokenlist:
            if not token.scope.isExecutable:
                continue
            if token.str != '(' or token.astParent:
                continue
            if token.astOperand1 is None or not token.astOperand1.isName:
                continue
            if token.astOperand1.varId and (token.astOperand1.variable is None or get_function_pointer_type(token.astOperand1.variable.typeStartToken) is None):
                continue
            if token.valueType is None:
                continue
            if token.valueType.type == 'void' and token.valueType.pointer == 0:
                continue
            self.reportError(token, 17, 7)

    def misra_17_8(self, data):
        for token in data.tokenlist:
            if not (token.isAssignmentOp or (token.str in ('++', '--'))):
                continue
            if not token.astOperand1:
                continue
            var = token.astOperand1.variable
            if var and var.isArgument:
                self.reportError(token, 17, 8)

    def misra_18_4(self, data):
        for token in data.tokenlist:
            if token.str not in ('+', '-', '+=', '-='):
                continue
            if token.astOperand1 is None or token.astOperand2 is None:
                continue
            vt1 = token.astOperand1.valueType
            vt2 = token.astOperand2.valueType
            if vt1 and vt1.pointer > 0:
                self.reportError(token, 18, 4)
            elif vt2 and vt2.pointer > 0:
                self.reportError(token, 18, 4)

    def misra_18_5(self, data):
        for var in data.variables:
            if not var.isPointer:
                continue
            typetok = var.nameToken
            count = 0
            while typetok:
                if typetok.str == '*':
                    count = count + 1
                elif not typetok.isName:
                    break
                typetok = typetok.previous
            if count > 2:
                self.reportError(var.nameToken, 18, 5)

    def misra_18_7(self, data):
        for scope in data.scopes:
            if scope.type != 'Struct':
                continue

            token = scope.bodyStart.next
            while token != scope.bodyEnd and token is not None:
                # Handle nested structures to not duplicate an error.
                if token.str == '{':
                    token = token.link

                # skip function pointer parameter types
                if token.astOperand1 is None:
                    pass
                elif cppcheckdata.simpleMatch(token, "[ ]"):
                    self.reportError(token, 18, 7)
                    break
                token = token.next

    def misra_18_8(self, data):
        for var in data.variables:
            if not var.isArray or not var.isLocal:
                continue
            # TODO Array dimensions are not available in dump, must look in tokens
            typetok = var.nameToken.next
            if not typetok or typetok.str != '[':
                continue
            # Unknown define or syntax error
            if not typetok.astOperand2:
                continue
            if not isConstantExpression(typetok.astOperand2) and not isUnknownConstantExpression(typetok.astOperand2):
                self.reportError(var.nameToken, 18, 8)

    def misra_19_2(self, data):
        for token in data.tokenlist:
            if token.str == 'union':
                self.reportError(token, 19, 2)

    def misra_20_1(self, data):
        token_in_file = {}
        for token in data.tokenlist:
            if token.file not in token_in_file:
                token_in_file[token.file] = int(token.linenr)
            else:
                token_in_file[token.file] = min(token_in_file[token.file], int(token.linenr))

        for directive in data.directives:
            if not directive.str.startswith('#include'):
                continue
            if directive.file not in token_in_file:
                continue
            if token_in_file[directive.file] < int(directive.linenr):
                self.reportError(directive, 20, 1)

    def misra_20_2(self, data):
        for directive in data.directives:
            if not directive.str.startswith('#include '):
                continue
            for pattern in ('\\', '//', '/*', ',', "'"):
                if pattern in directive.str:
                    self.reportError(directive, 20, 2)
                    break

    def misra_20_3(self, data):
        for directive in data.directives:
            if not directive.str.startswith('#include '):
                continue

            words = directive.str.split(' ')

            # If include directive contains more than two words, here would be
            # violation anyway.
            if len(words) > 2:
                self.reportError(directive, 20, 3)

            # Handle include directives with not quoted argument
            elif len(words) > 1:
                filename = words[1]
                if not ((filename.startswith('"') and
                         filename.endswith('"')) or
                        (filename.startswith('<') and
                         filename.endswith('>'))):
                    # We are handle only directly included files in the
                    # following format: #include file.h
                    # Cases with macro expansion provided by MISRA document are
                    # skipped because we don't always have access to directive
                    # definition.
                    if '.' in filename:
                        self.reportError(directive, 20, 3)

    def misra_20_4(self, data):
        for directive in data.directives:
            res = re.search(r'#define ([a-z][a-z0-9_]+)', directive.str)
            if res and isKeyword(res.group(1), data.standards.c):
                self.reportError(directive, 20, 4)

    def misra_20_5(self, data):
        for directive in data.directives:
            if directive.str.startswith('#undef '):
                self.reportError(directive, 20, 5)

    def misra_20_7(self, data):
        def find_string_concat(exp, arg, directive_args):
            # Handle concatenation of string literals, e.g.:
            # #define MACRO(A, B) (A " " B)
            # Addon should not report errors for both macro arguments.
            arg_pos = exp.find(arg, 0)
            need_check = False
            skip_next = False
            state_in_string = False
            pos_search = arg_pos + 1
            directive_args = [a.strip() for a in directive_args if a != arg]
            arg = arg.strip()
            while pos_search < len(exp):
                if exp[pos_search] == '"':
                    if state_in_string:
                        state_in_string = False
                    else:
                        state_in_string = True
                    pos_search += 1
                elif exp[pos_search].isalnum():
                    word = ""
                    while pos_search < len(exp) and exp[pos_search].isalnum():
                        word += exp[pos_search]
                        pos_search += 1
                    if word == arg:
                        pos_search += 1
                    elif word in directive_args:
                        skip_next = True
                        break
                elif exp[pos_search] == ' ':
                    pos_search += 1
                elif state_in_string:
                    pos_search += 1
                else:
                    need_check = True
                    break
            return need_check, skip_next

        for directive in data.directives:
            d = Define(directive)
            exp = '(' + d.expansionList + ')'
            skip_next = False
            for arg in d.args:
                if skip_next:
                    _, skip_next = find_string_concat(exp, arg, d.args)
                    continue
                need_check, skip_next = find_string_concat(exp, arg, d.args)
                if not need_check:
                    continue

                pos = 0
                while pos < len(exp):
                    pos = exp.find(arg, pos)
                    if pos < 0:
                        break
                    # is 'arg' used at position pos
                    pos1 = pos - 1
                    pos2 = pos + len(arg)
                    pos = pos2
                    if pos1 >= 0 and (isalnum(exp[pos1]) or exp[pos1] == '_'):
                        continue
                    if pos2 < len(exp) and (isalnum(exp[pos2]) or exp[pos2] == '_'):
                        continue

                    while pos1 >= 0 and exp[pos1] == ' ':
                        pos1 -= 1
                    if exp[pos1] == '#':
                        continue
                    if exp[pos1] not in '([,.':
                        self.reportError(directive, 20, 7)
                        break
                    while pos2 < len(exp) and exp[pos2] == ' ':
                        pos2 += 1
                    if pos2 < len(exp) and exp[pos2] not in ')]#,':
                        self.reportError(directive, 20, 7)
                        break

    def misra_20_8(self, cfg):
        for cond in cfg.preprocessor_if_conditions:
            #print(cond)
            if cond.result and cond.result not in (0,1):
                self.reportError(cond, 20, 8)

    def misra_20_9(self, cfg):
        for cond in cfg.preprocessor_if_conditions:
            if cond.E is None:
                continue
            defined = []
            for directive in cfg.directives:
                if directive.file == cond.file and directive.linenr == cond.linenr:
                    for name in re.findall(r'[^_a-zA-Z0-9]defined[ ]*\([ ]*([_a-zA-Z0-9]+)[ ]*\)', directive.str):
                        defined.append(name)
                    for name in re.findall(r'[^_a-zA-Z0-9]defined[ ]*([_a-zA-Z0-9]+)', directive.str):
                        defined.append(name)
                    break
            for s in cond.E.split(' '):
                if (s[0] >= 'A' and s[0] <= 'Z') or (s[0] >= 'a' and s[0] <= 'z'):
                    if isKeyword(s, cfg.standards.c):
                        continue
                    if s in defined:
                        continue
                    self.reportError(cond, 20, 9)

    def misra_20_10(self, data):
        for directive in data.directives:
            d = Define(directive)
            if d.expansionList.find('#') >= 0:
                self.reportError(directive, 20, 10)

    def misra_20_11(self, cfg):
        for directive in cfg.directives:
            d = Define(directive)
            for arg in d.args:
                res = re.search(r'[^#]#[ ]*%s[ ]*##' % arg, ' ' + d.expansionList)
                if res:
                    self.reportError(directive, 20, 11)

    def misra_20_12(self, cfg):
        def _is_hash_hash_op(expansion_list, arg):
            return re.search(r'##[ ]*%s[^a-zA-Z0-9_]' % arg, expansion_list) or \
                   re.search(r'[^a-zA-Z0-9_]%s[ ]*##' % arg, expansion_list)

        def _is_other_op(expansion_list, arg):
            pos = expansion_list.find(arg)
            while pos >= 0:
                pos1 = pos - 1
                pos2 = pos + len(arg)
                pos = expansion_list.find(arg, pos2)
                if isalnum(expansion_list[pos1]) or expansion_list[pos1] == '_':
                    continue
                if isalnum(expansion_list[pos2]) or expansion_list[pos2] == '_':
                    continue
                while expansion_list[pos1] == ' ':
                    pos1 = pos1 - 1
                if expansion_list[pos1] == '#':
                    continue
                while expansion_list[pos2] == ' ':
                    pos2 = pos2 + 1
                if expansion_list[pos2] == '#':
                    continue
                return True
            return False

        def _is_arg_macro_usage(directive, arg):
            for macro_usage in cfg.macro_usage:
                if macro_usage.file == directive.file and macro_usage.linenr == directive.linenr:
                    for macro_usage_arg in cfg.macro_usage:
                        if macro_usage_arg == macro_usage:
                            continue
                        if (macro_usage.usefile == macro_usage_arg.usefile and
                            macro_usage.uselinenr == macro_usage_arg.uselinenr and
                            macro_usage.usecolumn == macro_usage_arg.usecolumn):
                            # TODO: check arg better
                            return True
            return False

        for directive in cfg.directives:
            define = Define(directive)
            expansion_list = '(%s)' % define.expansionList
            for arg in define.args:
                if not _is_hash_hash_op(expansion_list, arg):
                    continue
                if not _is_other_op(expansion_list, arg):
                    continue
                if _is_arg_macro_usage(directive, arg):
                    self.reportError(directive, 20, 12)
                    break

    def misra_20_13(self, data):
        dir_pattern = re.compile(r'#[ ]*([^ (<]*)')
        for directive in data.directives:
            dir = directive.str
            mo = dir_pattern.match(dir)
            if mo:
                dir = mo.group(1)
            if dir not in ['define', 'elif', 'else', 'endif', 'error', 'if', 'ifdef', 'ifndef', 'include',
                           'pragma', 'undef', 'warning']:
                self.reportError(directive, 20, 13)

    def misra_20_14(self, data):
        # stack for #if blocks. contains the #if directive until the corresponding #endif is seen.
        # the size increases when there are inner #if directives.
        ifStack = []
        for directive in data.directives:
            if directive.str.startswith('#if ') or directive.str.startswith('#ifdef ') or directive.str.startswith(
                    '#ifndef '):
                ifStack.append(directive)
            elif directive.str == '#else' or directive.str.startswith('#elif '):
                if len(ifStack) == 0:
                    self.reportError(directive, 20, 14)
                    ifStack.append(directive)
                elif directive.file != ifStack[-1].file:
                    self.reportError(directive, 20, 14)
            elif directive.str == '#endif':
                if len(ifStack) == 0:
                    self.reportError(directive, 20, 14)
                elif directive.file != ifStack[-1].file:
                    self.reportError(directive, 20, 14)
                    ifStack.pop()

    def misra_21_1(self, data):
        re_forbidden_macro = re.compile(r'#(?:define|undef) _[_A-Z]+')
        re_macro_name = re.compile(r'#(?:define|undef) (.+)[ $]')

        for d in data.directives:
            # Search for forbidden identifiers
            m = re.search(re_forbidden_macro, d.str)
            if m:
                self.reportError(d, 21, 1)
                continue

            # Search standard library identifiers in macro names
            m = re.search(re_macro_name, d.str)
            if not m:
                continue
            name = m.group(1)
            if isStdLibId(name, data.standards.c):
                self.reportError(d, 21, 1)

    def misra_21_2(self, cfg):
        for directive in cfg.directives:
            define = Define(directive)
            if re.match(r'_+BUILTIN_.*', define.name.upper()):
                self.reportError(directive, 21, 2)
        for func in cfg.functions:
            if isStdLibId(func.name, cfg.standards.c):
                tok = func.tokenDef if func.tokenDef else func.token
                self.reportError(tok, 21, 2)

    def misra_21_3(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token, data.standards.c) and (token.astOperand1.str in ('malloc', 'calloc', 'realloc', 'free')):
                self.reportError(token, 21, 3)

    def misra_21_4(self, data):
        directive = findInclude(data.directives, '<setjmp.h>')
        if directive:
            self.reportError(directive, 21, 4)

    def misra_21_5(self, data):
        directive = findInclude(data.directives, '<signal.h>')
        if directive:
            self.reportError(directive, 21, 5)

    def misra_21_6(self, data):
        for token in data.tokenlist:
            if not isFunctionCall(token) or token.previous.function:
                continue
            standard_id = getStdLib(data.standards.c)
            funcname = token.previous.str
            if funcname in standard_id.get("stdio.h", []) or funcname in standard_id.get("wchar.h", []):
                self.reportError(token, 21, 6)

    def misra_21_7(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token, data.standards.c) and (token.astOperand1.str in ('atof', 'atoi', 'atol', 'atoll')):
                self.reportError(token, 21, 7)

    def misra_21_8(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token, data.standards.c) and (token.astOperand1.str in ('abort', 'exit', 'getenv')):
                self.reportError(token, 21, 8)

    def misra_21_9(self, data):
        for token in data.tokenlist:
            if (token.str in ('bsearch', 'qsort')) and token.next and token.next.str == '(':
                self.reportError(token, 21, 9)

    def misra_21_10(self, data):
        directive = findInclude(data.directives, '<time.h>')
        if directive:
            self.reportError(directive, 21, 10)

        for token in data.tokenlist:
            if (token.str == 'wcsftime') and token.next and token.next.str == '(':
                self.reportError(token, 21, 10)

    def misra_21_11(self, data):
        directive = findInclude(data.directives, '<tgmath.h>')
        if directive:
            self.reportError(directive, 21, 11)

    def misra_21_12(self, data):
        if findInclude(data.directives, '<fenv.h>'):
            for token in data.tokenlist:
                if token.str == 'fexcept_t' and token.isName:
                    self.reportError(token, 21, 12)
                if isFunctionCall(token, data.standards.c) and (token.astOperand1.str in (
                        'feclearexcept',
                        'fegetexceptflag',
                        'feraiseexcept',
                        'fesetexceptflag',
                        'fetestexcept')):
                    self.reportError(token, 21, 12)

    def misra_21_14(self, data):
        # buffers used in strcpy/strlen/etc function calls
        string_buffers = []
        for token in data.tokenlist:
            if token.str[0] == 's' and isFunctionCall(token.next, data.standards.c):
                name, args = cppcheckdata.get_function_call_name_args(token)
                if name is None:
                    continue
                def _get_string_buffers(match, args, argnum):
                    if not match:
                        return []
                    ret = []
                    for a in argnum:
                        if a < len(args):
                            arg = args[a]
                            while arg and arg.str in ('.', '::'):
                                arg = arg.astOperand2
                            if arg and arg.varId != 0 and arg.varId not in ret:
                                ret.append(arg.varId)
                    return ret
                string_buffers += _get_string_buffers(name == 'strcpy', args, [0, 1])
                string_buffers += _get_string_buffers(name == 'strncpy', args, [0, 1])
                string_buffers += _get_string_buffers(name == 'strlen', args, [0])
                string_buffers += _get_string_buffers(name == 'strcmp', args, [0, 1])
                string_buffers += _get_string_buffers(name == 'sprintf', args, [0])
                string_buffers += _get_string_buffers(name == 'snprintf', args, [0, 3])

        for token in data.tokenlist:
            if token.str != 'memcmp':
                continue
            name, args = cppcheckdata.get_function_call_name_args(token)
            if name is None:
                continue
            if len(args) != 3:
                continue
            for arg in args[:2]:
                if arg.str[-1] == '\"':
                    self.reportError(arg, 21, 14)
                    continue
                while arg and arg.str in ('.', '::'):
                    arg = arg.astOperand2
                if arg and arg.varId and arg.varId in string_buffers:
                    self.reportError(arg, 21, 14)

    def misra_21_15(self, data):
        for token in data.tokenlist:
            if token.str not in ('memcpy', 'memmove', 'memcmp'):
                continue
            name, args = cppcheckdata.get_function_call_name_args(token)
            if name is None:
                continue
            if len(args) != 3:
                continue
            if args[0].valueType is None or args[1].valueType is None:
                continue
            if args[0].valueType.type == args[1].valueType.type:
                continue
            if args[0].valueType.type == 'void' or args[1].valueType.type == 'void':
                continue
            self.reportError(token, 21, 15)

    def misra_21_16(self, cfg):
        for token in cfg.tokenlist:
            if token.str != 'memcmp':
                continue
            name, args = cppcheckdata.get_function_call_name_args(token)
            if name is None:
                continue
            if len(args) != 3:
                continue
            for arg in args[:2]:
                if arg.valueType is None:
                    continue
                if arg.valueType.pointer > 1:
                    continue
                if getEssentialTypeCategory(arg) in ('unsigned', 'signed', 'bool'):
                    continue
                if arg.valueType.isEnum():
                    continue
                self.reportError(token, 21, 16)

    def misra_21_19(self, cfg):
        for token in cfg.tokenlist:
            if token.str in ('localeconv', 'getenv', 'setlocale', 'strerror') and simpleMatch(token.next, '('):
                name, _ = cppcheckdata.get_function_call_name_args(token)
                if name is None or name != token.str:
                    continue
                parent = token.next
                while simpleMatch(parent.astParent, '+'):
                    parent = parent.astParent
                # x = f()
                if simpleMatch(parent.astParent, '=') and parent == parent.astParent.astOperand2:
                    lhs = parent.astParent.astOperand1
                    if lhs and lhs.valueType and lhs.valueType.pointer > 0 and lhs.valueType.constness == 0:
                        self.reportError(token, 21, 19)
            if token.str == '=':
                lhs = token.astOperand1
                while simpleMatch(lhs, '*') and lhs.astOperand2 is None:
                    lhs = lhs.astOperand1
                if not simpleMatch(lhs, '.'):
                    continue
                while simpleMatch(lhs, '.'):
                    lhs = lhs.astOperand1
                if lhs and lhs.variable and simpleMatch(lhs.variable.typeStartToken, 'lconv'):
                    self.reportError(token, 21, 19)

    def misra_21_20(self, cfg):
        assigned = {}
        invalid = []
        for token in cfg.tokenlist:
            # No sophisticated data flow analysis, bail out if control flow is "interrupted"
            if token.str in ('{', '}', 'break', 'continue', 'return'):
                assigned = {}
                invalid = []
                continue

            # When pointer is assigned, remove it from 'assigned' and 'invalid'
            if token.varId and token.varId > 0 and simpleMatch(token.next, '='):
                for name in assigned.keys():
                    while token.varId in assigned[name]:
                        assigned[name].remove(token.varId)
                while token.varId in invalid:
                    invalid.remove(token.varId)
                continue

            # Calling dangerous function
            if token.str in ('asctime', 'ctime', 'gmtime', 'localtime', 'localeconv', 'getenv', 'setlocale', 'strerror'):
                name, _ = cppcheckdata.get_function_call_name_args(token)
                if name and name == token.str:
                    # make assigned pointers invalid
                    for varId in assigned.get(name, ()):
                        if varId not in invalid:
                            invalid.append(varId)

                    # assign pointer
                    parent = token.next
                    while parent.astParent and (parent.astParent.str == '+' or isCast(parent.astParent)):
                        parent = parent.astParent
                    if simpleMatch(parent.astParent, '='):
                        eq = parent.astParent
                        vartok = eq.previous
                        if vartok and vartok.varId and vartok.varId > 0:
                            if name not in assigned:
                                assigned[name] = [vartok.varId]
                            elif vartok.varId not in assigned[name]:
                                assigned[name].append(vartok.varId)
                continue

            # taking value of invalid pointer..
            if token.astParent and token.varId:
                if token.varId in invalid:
                    self.reportError(token, 21, 20)

    def misra_21_21(self, cfg):
        for token in cfg.tokenlist:
            if token.str == 'system':
                name, args = cppcheckdata.get_function_call_name_args(token)
                if name == 'system' and len(args) == 1:
                    self.reportError(token, 21, 21)

    def misra_22_5(self, cfg):
        for token in cfg.tokenlist:
            if token.isUnaryOp("*") or (token.isBinaryOp() and token.str == '.'):
                fileptr = token.astOperand1
                if fileptr.variable and cppcheckdata.simpleMatch(fileptr.variable.typeStartToken, 'FILE *'):
                    self.reportError(token, 22, 5)

    def misra_22_7(self, cfg):
        for eofToken in cfg.tokenlist:
            if eofToken.str != 'EOF':
                continue
            if eofToken.astParent is None or not eofToken.astParent.isComparisonOp:
                continue
            if eofToken.astParent.astOperand1 == eofToken:
                eofTokenSibling = eofToken.astParent.astOperand2
            else:
                eofTokenSibling = eofToken.astParent.astOperand1
            while isCast(eofTokenSibling) and eofTokenSibling.valueType and eofTokenSibling.valueType.type and eofTokenSibling.valueType.type == 'int':
                eofTokenSibling = eofTokenSibling.astOperand2 if eofTokenSibling.astOperand2 else eofTokenSibling.astOperand1
            if eofTokenSibling is not None and eofTokenSibling.valueType and eofTokenSibling.valueType and eofTokenSibling.valueType.type in ('bool', 'char', 'short'):
                self.reportError(eofToken, 22, 7)

    def misra_22_8(self, cfg):
        is_zero = False
        for token in cfg.tokenlist:
            if simpleMatch(token, 'errno = 0'):
                is_zero = True
            if token.str == '(' and not simpleMatch(token.link, ') {'):
                name, _ = cppcheckdata.get_function_call_name_args(token.previous)
                if name is None:
                    continue
                if is_errno_setting_function(name):
                    if not is_zero:
                        self.reportError(token, 22, 8)
                else:
                    is_zero = False

    def misra_22_9(self, cfg):
        errno_is_set = False
        for token in cfg.tokenlist:
            if token.str == '(' and not simpleMatch(token.link, ') {'):
                name, _ = cppcheckdata.get_function_call_name_args(token.previous)
                if name is None:
                    continue
                errno_is_set = is_errno_setting_function(name)
            if errno_is_set and token.str in '{};':
                errno_is_set = False
                tok = token.next
                while tok and tok.str not in ('{','}',';','errno'):
                    tok = tok.next
                if tok is None or tok.str != 'errno':
                    self.reportError(token, 22, 9)
                elif (tok.astParent is None) or (not tok.astParent.isComparisonOp):
                    self.reportError(token, 22, 9)

    def misra_22_10(self, cfg):
        last_function_call = None
        for token in cfg.tokenlist:
            if token.isName and token.next and token.next.str == '(' and not simpleMatch(token.next.link, ') {'):
                name, _ = cppcheckdata.get_function_call_name_args(token)
                last_function_call = name
            if token.str == '}':
                last_function_call = None
            if token.str == 'errno' and token.astParent and token.astParent.isComparisonOp:
                if last_function_call is None:
                    self.reportError(token, 22, 10)
                elif not is_errno_setting_function(last_function_call):
                    self.reportError(token, 22, 10)


    def get_verify_expected(self):
        """Return the list of expected violations in the verify test"""
        return self.verify_expected

    def get_verify_actual(self):
        """Return the list of actual violations in for the verify test"""
        return self.verify_actual

    def get_violations(self, violation_type=None):
        """Return the list of violations for a normal checker run"""
        if violation_type is None:
            return self.violations.items()
        return self.violations[violation_type]

    def get_violation_types(self):
        """Return the list of violations for a normal checker run"""
        return self.violations.keys()

    def addSuppressedRule(self, ruleNum,
                          fileName=None,
                          lineNumber=None,
                          symbolName=None):
        """
        Add a suppression to the suppressions data structure

        Suppressions are stored in a dictionary of dictionaries that
        contains a list of tuples.

        The first dictionary is keyed by the MISRA rule in hundreds
        format. The value of that dictionary is a dictionary of filenames.
        If the value is None then the rule is assumed to be suppressed for
        all files.
        If the filename exists then the value of that dictionary contains a list
        with the scope of the suppression.  If the list contains an item of None
        then the rule is assumed to be suppressed for the entire file. Otherwise
        the list contains line number, symbol name tuples.
        For each tuple either line number or symbol name can can be none.

        """
        normalized_filename = None

        if fileName is not None:
            normalized_filename = os.path.expanduser(fileName)
            normalized_filename = os.path.normpath(normalized_filename)

        if lineNumber is not None or symbolName is not None:
            line_symbol = (lineNumber, symbolName)
        else:
            line_symbol = None

        # If the rule is not in the dict already then add it
        if ruleNum not in self.suppressedRules:
            ruleItemList = []
            ruleItemList.append(line_symbol)

            fileDict = {}
            fileDict[normalized_filename] = ruleItemList

            self.suppressedRules[ruleNum] = fileDict

            # Rule is added.  Done.
            return

        # Rule existed in the dictionary. Check for
        # filename entries.

        # Get the dictionary for the rule number
        fileDict = self.suppressedRules[ruleNum]

        # If the filename is not in the dict already add it
        if normalized_filename not in fileDict:
            ruleItemList = []
            ruleItemList.append(line_symbol)

            fileDict[normalized_filename] = ruleItemList

            # Rule is added with a file scope. Done
            return

        # Rule has a matching filename. Get the rule item list.

        # Check the lists of rule items
        # to see if this (lineNumber, symbolName) combination
        # or None already exists.
        ruleItemList = fileDict[normalized_filename]

        if line_symbol is None:
            # is it already in the list?
            if line_symbol not in ruleItemList:
                ruleItemList.append(line_symbol)
        else:
            # Check the list looking for matches
            matched = False
            for each in ruleItemList:
                if each is not None:
                    if (each[0] == line_symbol[0]) and (each[1] == line_symbol[1]):
                        matched = True

            # Append the rule item if it was not already found
            if not matched:
                ruleItemList.append(line_symbol)

    def isRuleSuppressed(self, file_path, linenr, ruleNum):
        """
        Check to see if a rule is suppressed.

        :param ruleNum: is the rule number in hundreds format
        :param file_path: File path of checked location
        :param linenr: Line number of checked location

        If the rule exists in the dict then check for a filename
        If the filename is None then rule is suppressed globally
        for all files.
        If the filename exists then look for list of
        line number, symbol name tuples.  If the list is None then
        the rule is suppressed for the entire file
        If the list of tuples exists then search the list looking for
        matching line numbers.  Symbol names are currently ignored
        because they can include regular expressions.
        TODO: Support symbol names and expression matching.

        """
        ruleIsSuppressed = False

        # Remove any prefix listed in command arguments from the filename.
        filename = None
        if file_path is not None:
            if self.filePrefix is not None:
                filename = remove_file_prefix(file_path, self.filePrefix)
            else:
                filename = os.path.basename(file_path)

        if ruleNum in self.suppressedRules:
            fileDict = self.suppressedRules[ruleNum]

            # a file name entry of None means that the rule is suppressed
            # globally
            if None in fileDict:
                ruleIsSuppressed = True
            else:
                # Does the filename match one of the names in
                # the file list
                if filename in fileDict:
                    # Get the list of ruleItems
                    ruleItemList = fileDict[filename]

                    if None in ruleItemList:
                        # Entry of None in the ruleItemList means the rule is
                        # suppressed for all lines in the filename
                        ruleIsSuppressed = True
                    else:
                        # Iterate though the the list of line numbers
                        # and symbols looking for a match of the line
                        # number.  Matching the symbol is a TODO:
                        for each in ruleItemList:
                            if each is not None:
                                if each[0] == linenr:
                                    ruleIsSuppressed = True

        return ruleIsSuppressed

    def isRuleGloballySuppressed(self, rule_num):
        """
        Check to see if a rule is globally suppressed.
        :param rule_num: is the rule number in hundreds format
        """
        if rule_num not in self.suppressedRules:
            return False
        return None in self.suppressedRules[rule_num]

    def showSuppressedRules(self):
        """
        Print out rules in suppression list sorted by Rule Number
        """
        print("Suppressed Rules List:")
        outlist = []

        for ruleNum in self.suppressedRules:
            fileDict = self.suppressedRules[ruleNum]

            for fname in fileDict:
                ruleItemList = fileDict[fname]

                for item in ruleItemList:
                    if item is None:
                        item_str = "None"
                    else:
                        item_str = str(item[0])

                    outlist.append("%s: %s: %s (%d locations suppressed)" % (
                    float(ruleNum) / 100, fname, item_str, self.suppressionStats.get(ruleNum, 0)))

        for line in sorted(outlist, reverse=True):
            print("  %s" % line)

    def setFilePrefix(self, prefix):
        """
        Set the file prefix to ignore from files when matching
        suppression files
        """
        self.filePrefix = prefix

    def setSeverity(self, severity):
        """
        Set the severity for all errors.
        """
        self.severity = severity

    def setSuppressionList(self, suppressionlist):
        num1 = 0
        num2 = 0
        rule_pattern = re.compile(r'([0-9]+).([0-9]+)')
        strlist = suppressionlist.split(",")

        # build ignore list
        for item in strlist:
            res = rule_pattern.match(item)
            if res:
                num1 = int(res.group(1))
                num2 = int(res.group(2))
                ruleNum = (num1 * 100) + num2

                self.addSuppressedRule(ruleNum)

    def report_config_error(self, location, errmsg):
        errmsg = 'Because of missing configuration, misra checking is incomplete. There can be false negatives! ' + errmsg
        cppcheck_severity = 'error'
        error_id = 'config'
        if self.settings.verify:
            self.verify_actual.append('%s:%d %s' % (location.file, location.linenr, error_id))
        else:
            cppcheckdata.reportError(location, cppcheck_severity, errmsg, 'misra', error_id)

    def reportError(self, location, num1, num2):
        ruleNum = num1 * 100 + num2

        if self.isRuleGloballySuppressed(ruleNum):
            return

        if self.settings.verify:
            self.verify_actual.append('%s:%d %d.%d' % (location.file, location.linenr, num1, num2))
        elif self.isRuleSuppressed(location.file, location.linenr, ruleNum):
            # Error is suppressed. Ignore
            self.suppressionStats.setdefault(ruleNum, 0)
            self.suppressionStats[ruleNum] += 1
            return
        else:
            errorId = 'c2012-' + str(num1) + '.' + str(num2)
            misra_severity = 'Undefined'
            cppcheck_severity = 'style'
            if ruleNum in self.ruleTexts:
                errmsg = self.ruleTexts[ruleNum].text
                if self.ruleTexts[ruleNum].misra_severity:
                    misra_severity = self.ruleTexts[ruleNum].misra_severity
                cppcheck_severity = self.ruleTexts[ruleNum].cppcheck_severity
            elif len(self.ruleTexts) == 0:
                if self.ruleText_filename is None:
                    errmsg = 'misra violation (use --rule-texts=<file> to get proper output)'
                else:
                    errmsg = 'misra violation (rule-texts-file not found: ' + self.ruleText_filename + ')'
            else:
                errmsg = 'misra violation %s with no text in the supplied rule-texts-file' % (ruleNum)

            if self.severity:
                cppcheck_severity = self.severity

            this_violation = '{}-{}-{}-{}'.format(location.file, location.linenr, location.column, ruleNum)

            # If this is new violation then record it and show it. If not then
            # skip it since it has already been displayed.
            if this_violation not in self.existing_violations:
                self.existing_violations.add(this_violation)
                cppcheckdata.reportError(location, cppcheck_severity, errmsg, 'misra', errorId, misra_severity)

                if misra_severity not in self.violations:
                    self.violations[misra_severity] = []
                self.violations[misra_severity].append('misra-' + errorId)

    def loadRuleTexts(self, filename):
        num1 = 0
        num2 = 0
        appendixA = False

        Rule_pattern = re.compile(r'^Rule ([0-9]+)\.([0-9]+)')
        severity_pattern = re.compile(r'.*[ ]*(Advisory|Required|Mandatory)$')
        xA_Z_pattern = re.compile(r'^[#A-Z].*')
        a_z_pattern = re.compile(r'^[a-z].*')
        # Try to detect the file encoding
        file_stream = None
        encodings = ['ascii', 'utf-8', 'windows-1250', 'windows-1252']
        for e in encodings:
            try:
                file_stream = open(filename, 'r', encoding=e)
                file_stream.readlines()
                file_stream.seek(0)
            except UnicodeDecodeError:
                file_stream.close()
                file_stream = None
            else:
                break
        if not file_stream:
            print('Could not find a suitable codec for "' + filename + '".')
            print('If you know the codec please report it to the developers so the list can be enhanced.')
            print('Trying with default codec now and ignoring errors if possible ...')
            try:
                file_stream = open(filename, 'rt', errors='ignore')
            except TypeError:
                # Python 2 does not support the errors parameter
                file_stream = open(filename, 'rt')

        rule = None
        rule_line_number = 0

        for line in file_stream:

            line = line.strip()
            if len(line) == 0:
                continue

            if not appendixA:
                if line.find('Appendix A') >= 0 and line.find('Summary of guidelines') >= 10:
                    appendixA = True
                continue
            if line.find('Appendix B') >= 0:
                break

            # Parse rule declaration.
            res = Rule_pattern.match(line)

            if res:
                rule_line_number = 0
                num1 = int(res.group(1))
                num2 = int(res.group(2))
                rule = Rule(num1, num2)

                res = severity_pattern.match(line)
                if res:
                    rule.misra_severity = res.group(1)
                    rule_line_number = 1
                continue

            if rule is None:
                continue

            rule_line_number += 1

            if rule_line_number == 1:
                res = severity_pattern.match(line)

                if res:
                    rule.misra_severity = res.group(1)
                    continue

                rule_line_number = 2

            # Parse beginning of rule text.
            if not rule.text and xA_Z_pattern.match(line):
                rule.text = line.strip()
                self.ruleTexts[rule.num] = rule
                continue

            # Parse continuing of rule text.
            if a_z_pattern.match(line):
                self.ruleTexts[rule.num].text += ' ' + line.strip()
                continue

            rule = None

        file_stream.close()

    def verifyRuleTexts(self):
        """Prints rule numbers without rule text."""
        rule_texts_rules = []
        for rule_num in self.ruleTexts:
            rule = self.ruleTexts[rule_num]
            rule_texts_rules.append(str(rule.num1) + '.' + str(rule.num2))

        all_rules = list(getAddonRules() + getCppcheckRules())

        missing_rules = list(set(all_rules) - set(rule_texts_rules))
        if len(missing_rules) == 0:
            print("Rule texts are correct.")
        else:
            print("Missing rule texts: " + ', '.join(missing_rules))

    def printStatus(self, *args, **kwargs):
        if not self.settings.quiet:
            print(*args, **kwargs)

    def executeCheck(self, rule_num, check_function, *args):
        """Execute check function for a single MISRA rule.

        :param rule_num: Number of rule in hundreds format
        :param check_function: Check function to execute
        :param args: Check function arguments
        """
        if not self.isRuleGloballySuppressed(rule_num):
            misra_cpp = (
                202, # misra-c2012-2.3 : misra c++2008 0-1-9
                203, # misra-c2012-2.3 : misra c++2008 0-1-5
                402, # misra-c2012-4.2 : misra c++2008 2-3-1
                701, # misra-c2012-7.1 : misra c++2008 2-3-1
                702, # misra-c2012-7.2 : misra c++2008 2-13-2
                1203, # misra-c2012-12.3 : misra c++2008 5-14-1
                1204, # misra-c2012-12.4 : misra c++2008 5-18-1
                1305, # misra-c2012-13.5 : misra c++2008 5-19-1
                1702, # misra-c2012-17.2 : misra c++2008 7-5-4
                1901) # misra-c2012-19.1 : misra c++2008 2-13-3

            if (not self.is_cpp) or rule_num in misra_cpp:
                # log checker
                errmsg = 'Misra C: %i.%i' % (rule_num // 100, rule_num % 100)
                cppcheckdata.log_checker(errmsg, 'misra')

                check_function(*args)

    def parseDump(self, dumpfile, path_premium_addon=None):
        def fillVerifyExpected(verify_expected, tok):
            """Add expected suppressions to verify_expected list."""
            rule_re = re.compile(r'[0-9]+\.[0-9]+')
            if tok.str.startswith('//') and 'TODO' not in tok.str:
                for word in tok.str[2:].split(' '):
                    if rule_re.match(word) or word == "config":
                        verify_expected.append('%s:%d %s' % (tok.file, tok.linenr, word))

        data = cppcheckdata.parsedump(dumpfile)
        typeBits['CHAR'] = data.platform.char_bit
        typeBits['SHORT'] = data.platform.short_bit
        typeBits['INT'] = data.platform.int_bit
        typeBits['LONG'] = data.platform.long_bit
        typeBits['LONG_LONG'] = data.platform.long_long_bit
        typeBits['POINTER'] = data.platform.pointer_bit

        if self.settings.verify:
            # Add suppressions from the current file
            for tok in data.rawTokens:
                fillVerifyExpected(self.verify_expected, tok)
            # Add suppressions from the included headers
            include_re = re.compile(r'^#include [<"]([a-zA-Z0-9]+[a-zA-Z\-_./\\0-9]*)[">]$')
            dump_dir = os.path.dirname(data.filename)
            for conf in data.configurations:
                for directive in conf.directives:
                    m = re.match(include_re, directive.str)
                    if not m:
                        continue
                    header_dump_path = os.path.join(dump_dir, m.group(1) + '.dump')
                    if not os.path.exists(header_dump_path):
                        continue
                    header_data = cppcheckdata.parsedump(header_dump_path)
                    for tok in header_data.rawTokens:
                        fillVerifyExpected(self.verify_expected, tok)
        else:
            self.printStatus('Checking ' + dumpfile + '...')

        self.is_cpp = data.language == 'cpp'

        for cfgNumber, cfg in enumerate(data.iterconfigurations()):
            if not self.settings.quiet:
                self.printStatus('Checking %s, config %s...' % (dumpfile, cfg.name))

            self.executeCheck(102, self.misra_1_2, cfg)
            if not path_premium_addon:
                self.executeCheck(104, self.misra_1_4, cfg)
            self.executeCheck(202, self.misra_2_2, cfg)
            self.executeCheck(203, self.misra_2_3, dumpfile, cfg.typedefInfo)
            self.executeCheck(204, self.misra_2_4, dumpfile, cfg)
            self.executeCheck(205, self.misra_2_5, dumpfile, cfg)
            self.executeCheck(207, self.misra_2_7, cfg)
            # data.rawTokens is same for all configurations
            if cfgNumber == 0:
                self.executeCheck(301, self.misra_3_1, data.rawTokens)
                #self.executeCheck(302, self.misra_3_2, data.rawTokens)
                self.executeCheck(401, self.misra_4_1, data.rawTokens)
                self.executeCheck(402, self.misra_4_2, data.rawTokens)
            self.executeCheck(501, self.misra_5_1, cfg)
            self.executeCheck(502, self.misra_5_2, cfg)
            self.executeCheck(504, self.misra_5_4, cfg)
            self.executeCheck(505, self.misra_5_5, cfg)
            self.executeCheck(506, self.misra_5_6, dumpfile, cfg.typedefInfo)
            self.executeCheck(507, self.misra_5_7, dumpfile, cfg)
            self.executeCheck(508, self.misra_5_8, dumpfile, cfg)
            self.executeCheck(509, self.misra_5_9, dumpfile, cfg)
            self.executeCheck(601, self.misra_6_1, cfg)
            self.executeCheck(602, self.misra_6_2, cfg)
            if cfgNumber == 0:
                self.executeCheck(701, self.misra_7_1, data.rawTokens)
            self.executeCheck(702, self.misra_7_2, cfg)
            if cfgNumber == 0:
                self.executeCheck(703, self.misra_7_3, data.rawTokens)
            self.executeCheck(704, self.misra_7_4, cfg)
            self.executeCheck(801, self.misra_8_1, cfg)
            if cfgNumber == 0:
                self.executeCheck(802, self.misra_8_2, cfg, data.rawTokens)
            self.executeCheck(804, self.misra_8_4, cfg)
            self.executeCheck(805, self.misra_8_5, dumpfile, cfg)
            self.executeCheck(806, self.misra_8_6, dumpfile, cfg)
            self.executeCheck(807, self.misra_8_7, dumpfile, cfg)
            self.executeCheck(808, self.misra_8_8, cfg)
            self.executeCheck(809, self.misra_8_9, cfg)
            self.executeCheck(810, self.misra_8_10, cfg)
            self.executeCheck(811, self.misra_8_11, cfg)
            self.executeCheck(812, self.misra_8_12, cfg)
            if cfgNumber == 0:
                self.executeCheck(814, self.misra_8_14, data.rawTokens)
            self.executeCheck(902, self.misra_9_2, cfg)
            self.executeCheck(903, self.misra_9_3, cfg)
            self.executeCheck(904, self.misra_9_4, cfg)
            if cfgNumber == 0:
                self.executeCheck(905, self.misra_9_5, cfg, data.rawTokens)
            if not path_premium_addon:
                self.executeCheck(1001, self.misra_10_1, cfg)
                self.executeCheck(1002, self.misra_10_2, cfg)
                self.executeCheck(1003, self.misra_10_3, cfg)
                self.executeCheck(1004, self.misra_10_4, cfg)
                self.executeCheck(1005, self.misra_10_5, cfg)
                self.executeCheck(1006, self.misra_10_6, cfg)
                self.executeCheck(1007, self.misra_10_7, cfg)
                self.executeCheck(1008, self.misra_10_8, cfg)
            self.executeCheck(1101, self.misra_11_1, cfg)
            self.executeCheck(1102, self.misra_11_2, cfg)
            self.executeCheck(1103, self.misra_11_3, cfg)
            self.executeCheck(1104, self.misra_11_4, cfg)
            self.executeCheck(1105, self.misra_11_5, cfg)
            self.executeCheck(1106, self.misra_11_6, cfg)
            self.executeCheck(1107, self.misra_11_7, cfg)
            self.executeCheck(1108, self.misra_11_8, cfg)
            self.executeCheck(1109, self.misra_11_9, cfg)
            if cfgNumber == 0:
                self.executeCheck(1201, self.misra_12_1_sizeof, data.rawTokens)
            self.executeCheck(1201, self.misra_12_1, cfg)
            self.executeCheck(1202, self.misra_12_2, cfg)
            self.executeCheck(1203, self.misra_12_3, cfg)
            self.executeCheck(1204, self.misra_12_4, cfg)
            self.executeCheck(1301, self.misra_13_1, cfg)
            self.executeCheck(1303, self.misra_13_3, cfg)
            self.executeCheck(1304, self.misra_13_4, cfg)
            self.executeCheck(1305, self.misra_13_5, cfg)
            self.executeCheck(1306, self.misra_13_6, cfg)
            self.executeCheck(1401, self.misra_14_1, cfg)
            self.executeCheck(1402, self.misra_14_2, cfg)
            self.executeCheck(1404, self.misra_14_4, cfg)
            self.executeCheck(1501, self.misra_15_1, cfg)
            self.executeCheck(1502, self.misra_15_2, cfg)
            self.executeCheck(1503, self.misra_15_3, cfg)
            self.executeCheck(1504, self.misra_15_4, cfg)
            self.executeCheck(1505, self.misra_15_5, cfg)
            if cfgNumber == 0:
                self.executeCheck(1506, self.misra_15_6, data.rawTokens)
            self.executeCheck(1507, self.misra_15_7, cfg)
            self.executeCheck(1601, self.misra_16_1, cfg)
            self.executeCheck(1602, self.misra_16_2, cfg)
            if cfgNumber == 0:
                self.executeCheck(1603, self.misra_16_3, data.rawTokens)
            self.executeCheck(1604, self.misra_16_4, cfg)
            self.executeCheck(1605, self.misra_16_5, cfg)
            self.executeCheck(1606, self.misra_16_6, cfg)
            self.executeCheck(1607, self.misra_16_7, cfg)
            self.executeCheck(1701, self.misra_17_1, cfg)
            self.executeCheck(1702, self.misra_17_2, cfg)
            self.executeCheck(1703, self.misra_17_3, cfg)
            self.misra_config(cfg)
            if cfgNumber == 0:
                self.executeCheck(1706, self.misra_17_6, data.rawTokens)
            self.executeCheck(1707, self.misra_17_7, cfg)
            self.executeCheck(1708, self.misra_17_8, cfg)
            self.executeCheck(1804, self.misra_18_4, cfg)
            self.executeCheck(1805, self.misra_18_5, cfg)
            self.executeCheck(1807, self.misra_18_7, cfg)
            self.executeCheck(1808, self.misra_18_8, cfg)
            self.executeCheck(1902, self.misra_19_2, cfg)
            self.executeCheck(2001, self.misra_20_1, cfg)
            self.executeCheck(2002, self.misra_20_2, cfg)
            self.executeCheck(2003, self.misra_20_3, cfg)
            self.executeCheck(2004, self.misra_20_4, cfg)
            self.executeCheck(2005, self.misra_20_5, cfg)
            self.executeCheck(2007, self.misra_20_7, cfg)
            self.executeCheck(2008, self.misra_20_8, cfg)
            self.executeCheck(2009, self.misra_20_9, cfg)
            self.executeCheck(2010, self.misra_20_10, cfg)
            self.executeCheck(2011, self.misra_20_11, cfg)
            self.executeCheck(2012, self.misra_20_12, cfg)
            self.executeCheck(2013, self.misra_20_13, cfg)
            self.executeCheck(2014, self.misra_20_14, cfg)
            self.executeCheck(2101, self.misra_21_1, cfg)
            self.executeCheck(2102, self.misra_21_2, cfg)
            self.executeCheck(2103, self.misra_21_3, cfg)
            self.executeCheck(2104, self.misra_21_4, cfg)
            self.executeCheck(2105, self.misra_21_5, cfg)
            self.executeCheck(2106, self.misra_21_6, cfg)
            self.executeCheck(2107, self.misra_21_7, cfg)
            self.executeCheck(2108, self.misra_21_8, cfg)
            self.executeCheck(2109, self.misra_21_9, cfg)
            self.executeCheck(2110, self.misra_21_10, cfg)
            self.executeCheck(2111, self.misra_21_11, cfg)
            self.executeCheck(2112, self.misra_21_12, cfg)
            self.executeCheck(2114, self.misra_21_14, cfg)
            self.executeCheck(2115, self.misra_21_15, cfg)
            self.executeCheck(2116, self.misra_21_16, cfg)
            self.executeCheck(2119, self.misra_21_19, cfg)
            self.executeCheck(2120, self.misra_21_20, cfg)
            self.executeCheck(2121, self.misra_21_21, cfg)
            # 22.4 is already covered by Cppcheck writeReadOnlyFile
            self.executeCheck(2205, self.misra_22_5, cfg)
            self.executeCheck(2207, self.misra_22_7, cfg)
            self.executeCheck(2208, self.misra_22_8, cfg)
            self.executeCheck(2209, self.misra_22_9, cfg)
            self.executeCheck(2210, self.misra_22_10, cfg)

    def read_ctu_info_line(self, line):
        if not line.startswith('{'):
            return None
        try:
            ctu_info = json.loads(line)
        except json.decoder.JSONDecodeError:
            return None
        if 'summary' not in ctu_info:
            return None
        if 'data' not in ctu_info:
            return None
        return ctu_info

    def analyse_ctu_info(self, ctu_info_files):
        all_typedef_info = {}
        all_tagname_info = {}
        all_macro_info = {}
        all_external_identifiers_decl = {}
        all_external_identifiers_def = {}
        all_internal_identifiers = {}
        all_local_identifiers = {}
        all_usage_files = {}

        from cppcheckdata import Location

        def is_different_location(loc1, loc2):
            return loc1['file'] != loc2['file'] or loc1['line'] != loc2['line']

        def is_different_file(loc1, loc2):
            return loc1['file'] != loc2['file']

        try:
            for filename in ctu_info_files:
                for line in open(filename, 'rt'):
                    s = self.read_ctu_info_line(line)
                    if s is None:
                        continue
                    summary_type = s.get('summary', '')
                    summary_data = s.get('data', None)

                    if summary_type == 'MisraTypedefInfo':
                        for new_typedef_info in summary_data:
                            key = new_typedef_info['name']
                            existing_typedef_info = all_typedef_info.get(key, None)
                            if existing_typedef_info:
                                if is_different_location(existing_typedef_info, new_typedef_info):
                                    self.reportError(Location(existing_typedef_info), 5, 6)
                                    self.reportError(Location(new_typedef_info), 5, 6)
                                else:
                                    existing_typedef_info['used'] = existing_typedef_info['used'] or new_typedef_info['used']
                            else:
                                all_typedef_info[key] = new_typedef_info

                    if summary_type == 'MisraTagName':
                        for new_tagname_info in summary_data:
                            key = new_tagname_info['name']
                            existing_tagname_info = all_tagname_info.get(key, None)
                            if existing_tagname_info:
                                if is_different_location(existing_tagname_info, new_tagname_info):
                                    self.reportError(Location(existing_tagname_info), 5, 7)
                                    self.reportError(Location(new_tagname_info), 5, 7)
                                else:
                                    existing_tagname_info['used'] = existing_tagname_info['used'] or new_tagname_info['used']
                            else:
                                all_tagname_info[key] = new_tagname_info

                    if summary_type == 'MisraMacro':
                        for new_macro in summary_data:
                            key = new_macro['name']
                            existing_macro = all_macro_info.get(key, None)
                            if existing_macro:
                                existing_macro['used'] = existing_macro['used'] or new_macro['used']
                            else:
                                all_macro_info[key] = new_macro

                    if summary_type == 'MisraExternalIdentifiers':
                        for s in sorted(summary_data, key=lambda d: "%s %s %s" %(d['file'],d['line'], d['column'] )):
                            is_declaration = s['decl']
                            if is_declaration:
                                all_external_identifiers = all_external_identifiers_decl
                            else:
                                all_external_identifiers = all_external_identifiers_def

                            name = s['name']
                            if name in all_external_identifiers:
                                if is_declaration and is_different_location(s, all_external_identifiers[name]):
                                    self.reportError(Location(s), 8, 5)
                                    self.reportError(Location(all_external_identifiers[name]), 8, 5)
                                elif is_different_file(s, all_external_identifiers[name]):
                                    self.reportError(Location(s), 8, 6)
                                    self.reportError(Location(all_external_identifiers[name]), 8, 6)
                            all_external_identifiers[name] = s

                    if summary_type == 'MisraInternalIdentifiers':
                        for s in summary_data:
                            if s['name'] in all_internal_identifiers:
                                if not s['inlinefunc'] or s['file'] != all_internal_identifiers[s['name']]['file']:
                                    self.reportError(Location(s), 5, 9)
                                    self.reportError(Location(all_internal_identifiers[s['name']]), 5, 9)
                            all_internal_identifiers[s['name']] = s

                    if summary_type == 'MisraLocalIdentifiers':
                        for s in summary_data:
                            all_local_identifiers[s['name']] = s

                    if summary_type == 'MisraUsage':
                        for s in summary_data:
                            if s['name'] in all_usage_files:
                                all_usage_files[s['name']].append(s['file'])
                            else:
                                all_usage_files[s['name']] = [s['file']]

        except FileNotFoundError:
            return

        unused_typedefs = [tdi for tdi in all_typedef_info.values() if not tdi['used']]
        for tdi in unused_typedefs:
            self.reportError(Location(tdi), 2, 3)

        unused_tags = [tag for tag in all_tagname_info.values() if not tag['used']]
        for tag in unused_tags:
            self.reportError(Location(tag), 2, 4)

        unused_macros = [m for m in all_macro_info.values() if not m['used']]
        for m in unused_macros:
            self.reportError(Location(m), 2, 5)

        all_external_identifiers = all_external_identifiers_decl
        all_external_identifiers.update(all_external_identifiers_def)
        for name, external_identifier in all_external_identifiers.items():
            internal_identifier = all_internal_identifiers.get(name)
            if internal_identifier:
                self.reportError(Location(internal_identifier), 5, 8)
                self.reportError(Location(external_identifier), 5, 8)

            local_identifier = all_local_identifiers.get(name)
            if local_identifier:
                self.reportError(Location(local_identifier), 5, 8)
                self.reportError(Location(external_identifier), 5, 8)

        for name, files in all_usage_files.items():
            #print('%s:%i' % (name, count))
            count = len(files)
            if count != 1 or name not in all_external_identifiers_def:
                continue
            if files[0] != Location(all_external_identifiers_def[name]).file:
                continue
            if name in all_external_identifiers:
                self.reportError(Location(all_external_identifiers[name]), 8, 7)

RULE_TEXTS_HELP = '''Path to text file of MISRA rules

If you have the tool 'pdftotext' you might be able
to generate this textfile with such command:

    pdftotext MISRA_C_2012.pdf MISRA_C_2012.txt

Otherwise you can more or less copy/paste the chapter
Appendix A Summary of guidelines
from the MISRA pdf. You can buy the MISRA pdf from
http://www.misra.org.uk/

Format:

<..arbitrary text..>
Appendix A Summary of guidelines
Rule 1.1 Required
Rule text for 1.1
continuation of rule text for 1.1
Rule 1.2 Mandatory
Rule text for 1.2
continuation of rule text for 1.2
<...>

'''

SUPPRESS_RULES_HELP = '''MISRA rules to suppress (comma-separated)

For example, if you'd like to suppress rules 15.1, 11.3,
and 20.13, run:

    python misra.py --suppress-rules 15.1,11.3,20.13 ...

'''


def get_args_parser():
    """Generates list of command-line arguments acceptable by misra.py script."""
    parser = cppcheckdata.ArgumentParser()
    parser.add_argument("--rule-texts", type=str, help=RULE_TEXTS_HELP)
    parser.add_argument("--verify-rule-texts",
                        help="Verify that all supported rules texts are present in given file and exit.",
                        action="store_true")
    parser.add_argument("--suppress-rules", type=str, help=SUPPRESS_RULES_HELP)
    parser.add_argument("--no-summary", help="Hide summary of violations", action="store_true")
    parser.add_argument("--show-suppressed-rules", help="Print rule suppression list", action="store_true")
    parser.add_argument("-P", "--file-prefix", type=str, help="Prefix to strip when matching suppression file rules")
    parser.add_argument("-generate-table", help=argparse.SUPPRESS, action="store_true")
    parser.add_argument("-verify", help=argparse.SUPPRESS, action="store_true")
    parser.add_argument("--severity", type=str, help="Set a custom severity string, for example 'error' or 'warning'. ")
    return parser


def main():
    parser = get_args_parser()
    args = parser.parse_args()
    settings = MisraSettings(args)
    checker = MisraChecker(settings)

    checker.path_premium_addon = cppcheckdata.get_path_premium_addon()

    if args.generate_table:
        generateTable()
        sys.exit(0)

    if args.rule_texts:
        filename = os.path.expanduser(args.rule_texts)
        filename = os.path.normpath(filename)
        checker.ruleText_filename = filename
        if os.path.isfile(filename):
            checker.loadRuleTexts(filename)
            if args.verify_rule_texts:
                checker.verifyRuleTexts()
                sys.exit(0)
        else:
            if args.verify_rule_texts:
                print('Fatal error: file is not found: ' + filename)
                sys.exit(1)


    if args.verify_rule_texts and not args.rule_texts:
        print("Error: Please specify rule texts file with --rule-texts=<file>")
        sys.exit(1)

    if args.suppress_rules:
        checker.setSuppressionList(args.suppress_rules)

    if args.file_prefix:
        checker.setFilePrefix(args.file_prefix)

    dump_files, ctu_info_files = cppcheckdata.get_files(args)

    if (not dump_files) and (not ctu_info_files):
        if not args.quiet:
            print("No input files.")
        sys.exit(0)

    if args.severity:
        checker.setSeverity(args.severity)

    for item in dump_files:
        checker.parseDump(item,checker.path_premium_addon)

        if settings.verify:
            verify_expected = checker.get_verify_expected()
            verify_actual = checker.get_verify_actual()

            exitCode = 0
            for expected in verify_expected:
                if expected not in verify_actual:
                    print('Expected but not seen: ' + expected)
                    exitCode = 1
            for actual in verify_actual:
                if actual not in verify_expected:
                    print('Not expected: ' + actual)
                    exitCode = 1

            # Existing behavior of verify mode is to exit
            # on the first un-expected output.
            # TODO: Is this required? or can it be moved to after
            # all input files have been processed
            if exitCode != 0:
                sys.exit(exitCode)

    checker.analyse_ctu_info(ctu_info_files)

    if settings.verify:
        sys.exit(exitCode)

    number_of_violations = len(checker.get_violations())
    if number_of_violations > 0:
        if settings.show_summary:
            print("\nMISRA rules violations found:\n\t%s\n" % (
                "\n\t".join(["%s: %d" % (viol, len(checker.get_violations(viol))) for viol in
                             checker.get_violation_types()])))

            rules_violated = {}
            for severity, ids in checker.get_violations():
                for misra_id in ids:
                    rules_violated[misra_id] = rules_violated.get(misra_id, 0) + 1
            print("MISRA rules violated:")
            convert = lambda text: int(text) if text.isdigit() else 0
            misra_sort = lambda key: [convert(c) for c in re.split(r'[\.-]([0-9]*)', key)]
            for misra_id in sorted(rules_violated.keys(), key=misra_sort):
                res = re.match(r'misra-c2012-([0-9]+)\\.([0-9]+)', misra_id)
                if res is None:
                    num = 0
                else:
                    num = int(res.group(1)) * 100 + int(res.group(2))
                severity = '-'
                if num in checker.ruleTexts:
                    severity = checker.ruleTexts[num].cppcheck_severity
                print("\t%15s (%s): %d" % (misra_id, severity, rules_violated[misra_id]))

    if args.show_suppressed_rules:
        checker.showSuppressedRules()


if __name__ == '__main__':
    main()
    sys.exit(cppcheckdata.EXIT_CODE)
