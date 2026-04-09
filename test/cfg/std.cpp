
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --library=std --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/std.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <bitset>
#include <cassert>
#include <cctype>
#include <cfenv>
#include <cinttypes>
#include <clocale>
#include <cmath>
#include <complex>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define __STDC_WANT_LIB_EXT1__ 1
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#ifndef __STDC_NO_THREADS__
    #include <threads.h>
#endif
#include <iomanip>
#include <ios> // IWYU pragma: keep
#include <iostream>
#include <istream> // IWYU pragma: keep
#include <iterator>
#include <list> // IWYU pragma: keep
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <streambuf> // IWYU pragma: keep
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <version> // IWYU pragma: keep
#ifdef __cpp_lib_span
#include <span>
#endif
#ifdef __cpp_lib_format
#include <format>
#endif

#if __cplusplus <= 201402L
void unreachableCode_std_unexpected(int &x)
{
    // cppcheck-suppress unexpectedCalled
    std::unexpected();
    // cppcheck-suppress unreachableCode
    x=42;
}
#endif

#ifdef __cpp_lib_clamp
int ignoredReturnValue_std_clamp(const int x)
{
    // cppcheck-suppress ignoredReturnValue
    std::clamp(x, 1, -1);
    return std::clamp(x, 1, -1);
}
void knownConditionTrueFalse_std_clamp(const int x)
{
    // cppcheck-suppress knownConditionTrueFalse
    if(std::clamp(-2, -1, 1) == -1){}
    // cppcheck-suppress knownConditionTrueFalse
    if(std::clamp(2, -1, 1) == 1){}
    // cppcheck-suppress knownConditionTrueFalse
    if(std::clamp(0, -1, 1) == 0){}
    if(std::clamp(x, 0, 2)){}
}
#endif // __cpp_lib_clamp

void unreachableCode_std_terminate(int &x)
{
    std::terminate();
    // cppcheck-suppress unreachableCode
    x=42;
}

bool ignoredReturnValue_std_filesystem_exists(const std::filesystem::path &path, std::error_code& ec)
{
    // cppcheck-suppress ignoredReturnValue
    std::filesystem::exists(path);
    // cppcheck-suppress ignoredReturnValue
    std::filesystem::exists(path, ec);
    const bool b {std::filesystem::exists(path)};
    return b && std::filesystem::exists(path, ec);
}

// https://en.cppreference.com/w/cpp/io/manip/quoted
void uninitvar_std_quoted(std::stringstream &ss, const std::string &input, const char delim, const char escape)
{
    ss << std::quoted(input);
    ss << std::quoted(input, delim);
    ss << std::quoted(input, delim, escape);
    char delimChar;
    // cppcheck-suppress uninitvar
    ss << std::quoted(input, delimChar);
    char escapeChar;
    // cppcheck-suppress uninitvar
    ss << std::quoted(input, delim, escapeChar);
}

int zerodiv_ldexp()
{
    int i = std::ldexp(0.0, 42.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_ilogb()
{
    int i = std::ilogb(1.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_hypot()
{
    int i = std::hypot(0.0, 0.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_fmod()
{
    int i = std::fmod(0.0, 42.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_fmin()
{
    int i = std::fmin(0.0, 0.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_fmax()
{
    int i = std::fmax(0.0, 0.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_floor()
{
    int i = std::floor(0.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_fabs()
{
    int i = std::fabs(-0.0) + std::fabs(+0.0) + std::fabs(0.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_fdim()
{
    int i = std::fdim(1.0, 1.0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_trunc()
{
    int i = std::trunc(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_ceil()
{
    int i = std::ceil(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_sqrt()
{
    int i = std::sqrt(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_cbrt()
{
    int i = std::cbrt(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_erf()
{
    int i = std::erf(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_erfc()
{
    int i = std::erfc(42);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_asin()
{
    int i = std::asin(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_acos()
{
    int i = std::acos(1);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_asinh()
{
    int i = std::asinh(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_acosh()
{
    int i = std::acosh(1);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_log1p()
{
    int i = std::log1p(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_nearbyint()
{
    int i = std::nearbyint(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_round()
{
    int i = std::round(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_sinh()
{
    int i = std::sinh(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_tanh()
{
    int i = std::tanh(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_atanh()
{
    int i = std::atanh(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_atan()
{
    int i = std::atan(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_sin()
{
    int i = std::sin(0)+std::sin(M_PI)+std::sin(2*M_PI);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int zerodiv_expm1()
{
    int i = std::expm1(0);
    // cppcheck-suppress zerodiv
    return 42 / i;
}

int moduloofone_cos()
{
    int i = std::cos(0);
    // cppcheck-suppress moduloofone
    return 42 % i;
}

int moduloofone_exp()
{
    int i = std::exp(0);
    // cppcheck-suppress moduloofone
    return 42 % i;
}

int moduloofone_exp2()
{
    int i = std::exp2(0);
    // cppcheck-suppress moduloofone
    return 42 % i;
}

int moduloofone_pow()
{
    int i = std::pow(2, 0);
    // cppcheck-suppress moduloofone
    return 42 % i;
}

char* invalidFunctionArgStr_strncpy(char * destination)
{
    // Copies the first num characters of source to destination.
    // If the end of the source C string (which is signaled by a null-character)
    // is found before num characters have been copied, destination
    // is padded with zeros until a total of num characters have been written to it.
    const char source = 'x';
    const std::size_t num = 1U;
    return strncpy(destination, &source, num);
}

void invalidFunctionArgStr_fprintf(FILE *stream, const char *format)
{
    const char formatBuf[] = {'%','d'};
    // cppcheck-suppress invalidFunctionArgStr
    (void)fprintf(stream,formatBuf,42);
    (void)fprintf(stream,format,42);
}

void invalidFunctionArgStr_fopen(const char * const fileName, const char * const mode)
{
    const char fileNameBuf[] = {'f','i','l','e'};
    const char modeBuf[] = {'r'};
    // cppcheck-suppress invalidFunctionArgStr
    FILE *fp = fopen(fileName, modeBuf);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
    // cppcheck-suppress invalidFunctionArgStr
    fp = fopen(fileNameBuf, mode);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
}

float invalidFunctionArg_remquo (float x, float y, int* quo )
{
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquo(x,0.0f,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquof(x,0.0f,quo);
    return std::remquo(x,y,quo);
}

double invalidFunctionArg_remquo (double x, double y, int* quo )
{
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquo(x,0.0,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquo(x,0.0f,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquo(x,0.0L,quo);
    return std::remquo(x,y,quo);
}

double invalidFunctionArg_remquo (long double x, long double y, int* quo )
{
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquo(x,0.0L,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) std::remquol(x,0.0L,quo);
    return std::remquo(x,y,quo);
}

void invalidFunctionArg_remainderl(long double f1, long double f2)
{
    // cppcheck-suppress invalidFunctionArg
    (void)std::remainderl(f1,0.0);
    // cppcheck-suppress invalidFunctionArg
    (void)std::remainderl(f1,0.0L);
    (void)std::remainderl(f1,f2);
}

void invalidFunctionArg_remainder(double f1, double f2)
{
    // cppcheck-suppress invalidFunctionArg
    (void)std::remainder(f1,0.0);
    (void)std::remainder(f1,f2);
}

void invalidFunctionArg_remainderf(float f1, float f2)
{
    // cppcheck-suppress invalidFunctionArg
    (void)std::remainderf(f1,0.0);
    // cppcheck-suppress invalidFunctionArg
    (void)std::remainderf(f1,0.0f);
    (void)std::remainderf(f1,f2);
}

void uninitvar_std_fstream_open(std::fstream &fs, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    std::string s;
    const char *ptr;
    std::ios_base::openmode m;

    fs.open(s, mode);
    // cppcheck-suppress uninitvar
    fs.open(ptr, mode);
    // TODO cppcheck-suppress uninitvar
    fs.open(filename, m);
    // TODO cppcheck-suppress uninitvar
    fs.open(strFileName, m);
    fs.open(s);
    // TODO cppcheck-suppress uninitvar
    fs.open(ptr);
}

void uninitvar_std_ifstream_open(std::ifstream &ifs, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    std::string s;
    const char *ptr;
    std::ios_base::openmode m;

    ifs.open(s, mode);
    // cppcheck-suppress uninitvar
    ifs.open(ptr, mode);
    // TODO cppcheck-suppress uninitvar
    ifs.open(filename, m);
    // TODO cppcheck-suppress uninitvar
    ifs.open(strFileName, m);
    ifs.open(s);
    // TODO cppcheck-suppress uninitvar
    ifs.open(ptr);
}

void uninitvar_std_ofstream_open(std::ofstream &os, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    std::string s;
    const char *ptr;
    std::ios_base::openmode m;

    os.open(s, mode);
    // cppcheck-suppress uninitvar
    os.open(ptr, mode);
    // TODO cppcheck-suppress uninitvar
    os.open(filename, m);
    // TODO cppcheck-suppress uninitvar
    os.open(strFileName, m);
    os.open(s);
    // TODO cppcheck-suppress uninitvar
    os.open(ptr);
}

void uninitvar_std_ofstream_precision(std::ofstream& os)
{
    std::streamsize s;
    // cppcheck-suppress uninitvar
    os.precision(s);
}

void nullPointer_std_filebuf_open(std::filebuf &fb, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    // cppcheck-suppress nullPointer
    (void)fb.open(nullptr, mode);
    (void)fb.open(filename, mode);
    (void)fb.open(strFileName, mode);
}

void nullPointer_std_ofstream_open(std::ofstream &os, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    // cppcheck-suppress nullPointer
    os.open(nullptr, mode);
    os.open(filename, mode);
    os.open(strFileName, mode);
    // cppcheck-suppress nullPointer
    os.open(nullptr);
    os.open(filename);
    os.open(strFileName);
}

void nullPointer_std_fstream_open(std::fstream &fs, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    // cppcheck-suppress nullPointer
    fs.open(nullptr, mode);
    fs.open(filename, mode);
    fs.open(strFileName, mode);
    // cppcheck-suppress nullPointer
    fs.open(nullptr);
    fs.open(filename);
    fs.open(strFileName);
}

void nullPointer_std_ifstream_open(std::ifstream &is, const std::string &strFileName, const char* filename, std::ios_base::openmode mode)
{
    // cppcheck-suppress nullPointer
    is.open(nullptr, mode);
    is.open(filename, mode);
    is.open(strFileName, mode);
    // cppcheck-suppress nullPointer
    is.open(nullptr);
    is.open(filename);
    is.open(strFileName);
}

void bufferAccessOutOfBounds_std_fstream_write(std::fstream &fs, const char* s, std::streamsize n)
{
    const char buf[42] = {0};
    (void)fs.write(buf,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)fs.write(buf,43);
    (void)fs.write(buf,n);
    (void)fs.write(s,n);
}

void bufferAccessOutOfBounds_std_ostream_write(std::ostream &os, const char* s, std::streamsize n)
{
    const char buf[42] = {0};
    (void)os.write(buf,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)os.write(buf,43);
    (void)os.write(buf,n);
    (void)os.write(s,n);
}

void bufferAccessOutOfBounds_std_ostringstream_write(std::ostringstream &oss, const char* s, std::streamsize n)
{
    const char buf[42] = {0};
    (void)oss.write(buf,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)oss.write(buf,43);
    (void)oss.write(buf,n);
    (void)oss.write(s,n);
}

void bufferAccessOutOfBounds_std_ofstream_write(std::ofstream &os, const char* s, std::streamsize n)
{
    const char buf[42] = {0};
    (void)os.write(buf,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)os.write(buf,43);
    (void)os.write(buf,n);
    (void)os.write(s,n);
}

// cppcheck-suppress constParameterReference // TODO: FP
void bufferAccessOutOfBounds_std_ifstream_get(std::ifstream& in, std::streambuf& sb)
{
    char cBuf[10];
    // cppcheck-suppress bufferAccessOutOfBounds
    in.getline(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.read(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.readsome(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.get(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.get(cBuf, 100, 'a');
    // cppcheck-suppress bufferAccessOutOfBounds
    in.getline(cBuf, 100, 'a');

    in.get(sb, 'a');

    in.close();
}

void invalidFunctionArg_fesetexceptflag(const fexcept_t* flagp, int excepts)
{
    (void)std::fesetexceptflag(flagp, excepts);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fesetexceptflag(flagp, 0);
    (void)std::fesetexceptflag(flagp, FE_DIVBYZERO);
    (void)std::fesetexceptflag(flagp, FE_INEXACT);
    (void)std::fesetexceptflag(flagp, FE_INVALID);
    (void)std::fesetexceptflag(flagp, FE_OVERFLOW);
    (void)std::fesetexceptflag(flagp, FE_UNDERFLOW);
    (void)std::fesetexceptflag(flagp, FE_ALL_EXCEPT);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fesetexceptflag(flagp, FE_ALL_EXCEPT+1);
}

void invalidFunctionArg_fetestexcept(int excepts)
{
    (void)std::fetestexcept(excepts);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fetestexcept(0);
    (void)std::fetestexcept(FE_DIVBYZERO);
    (void)std::fetestexcept(FE_INEXACT);
    (void)std::fetestexcept(FE_INVALID);
    (void)std::fetestexcept(FE_OVERFLOW);
    (void)std::fetestexcept(FE_UNDERFLOW);
    (void)std::fetestexcept(FE_ALL_EXCEPT);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fetestexcept(FE_ALL_EXCEPT+1);
}

void nullPointer_fprintf(FILE *Stream, const char *Format, int Argument)
{
    // cppcheck-suppress nullPointer
    (void)std::fprintf(Stream, nullptr, Argument);
    // no warning is expected
    (void)std::fprintf(Stream, Format, Argument);
}

void bufferAccessOutOfBounds_wcsftime(wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr)
{
    wchar_t buf[42];
    (void)std::wcsftime(buf, 42, format, timeptr);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    (void)std::wcsftime(buf, 43, format, timeptr);
    (void)std::wcsftime(ptr, maxsize, format, timeptr);
}

int qsort_cmpfunc (const void * a, const void * b) {
    return (*static_cast<const int*>(a) - *static_cast<const int*>(b));
}
void nullPointer_qsort(void *base, std::size_t n, std::size_t size, int (*cmp)(const void *, const void *))
{
    // cppcheck-suppress nullPointer
    std::qsort(nullptr, n, size, qsort_cmpfunc);
    // cppcheck-suppress nullPointer
    std::qsort(base, n, size, nullptr);
    std::qsort(base, n, size, qsort_cmpfunc);
}

void nullPointer_vfprintf(FILE *Stream, const char *Format, va_list Arg)
{
    // cppcheck-suppress nullPointer
    (void)std::vfprintf(Stream, nullptr, Arg);
    (void)std::vfprintf(Stream, Format, Arg);
}

void nullPointer_vfwprintf(FILE *Stream, const wchar_t *Format, va_list Arg)
{
    // cppcheck-suppress nullPointer
    (void)std::vfwprintf(Stream, nullptr, Arg);
    (void)std::vfwprintf(Stream, Format, Arg);
}

void *bufferAccessOutOfBounds_memchr(void *s, int c, size_t n)
{
    char buf[42]={0};
    (void)std::memchr(buf,c,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)std::memchr(buf,c,43);
    return std::memchr(s,c,n);
}

// As with all bounds-checked functions, localtime_s is only guaranteed to be available if __STDC_LIB_EXT1__ is defined by the implementation and if the user defines __STDC_WANT_LIB_EXT1__ to the integer constant 1 before including time.h.
#ifdef __STDC_LIB_EXT1__
void uninitvar_localtime_s(const std::time_t *restrict time, struct tm *restrict result)
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    const std::time_t *restrict Time;
    // TODO cppcheck-suppress uninitvar
    (void)std::localtime_s(Time, result);
    (void)std::localtime_s(time, result);
}

void nullPointer_localtime_s(const std::time_t *restrict time, struct tm *restrict result)
{
    // cppcheck-suppress nullPointer
    (void)std::localtime_s(NULL, result);
    // cppcheck-suppress nullPointer
    (void)std::localtime_s(time, NULL);
    (void)std::localtime_s(time, result);
}

void memleak_localtime_s(const std::time_t *restrict time, struct tm *restrict result) // #9258
{
    const time_t t = time(0);
    const struct tm* const now = new tm();
    if (localtime_s(now, &t) == 0) {
        // cppcheck-suppress valueFlowBailoutIncompleteVar
        std::cout << now->tm_mday << std::endl;
    }
    // cppcheck-suppress memleak
}
#endif // __STDC_LIB_EXT1__

size_t nullPointer_strftime(char *s, size_t max, const char *fmt, const struct tm *p)
{
    // cppcheck-suppress nullPointer
    (void) std::strftime(NULL,max,fmt,p);
    // cppcheck-suppress nullPointer
    (void) std::strftime(s,max,NULL,p);
    // cppcheck-suppress nullPointer
    (void) std::strftime(s,max,fmt,NULL);
    return std::strftime(s,max,fmt,p);
}

size_t bufferAccessOutOfBounds_wcsrtombs(char * dest, const wchar_t ** src, size_t len, mbstate_t * ps)
{
    char buf[42];
    (void)std::wcsrtombs(buf,src,42,ps);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)std::wcsrtombs(buf,src,43,ps);
    return std::wcsrtombs(dest,src,len,ps);
}

void deallocuse_wcrtomb(char *buff) { // #13119
    free(buff);
    // cppcheck-suppress deallocuse
    wcrtomb(buff, L'x', nullptr);
}

void invalidFunctionArg_std_string_substr(const std::string &str, std::size_t pos, std::size_t len) {
    // cppcheck-suppress invalidFunctionArg
    (void)str.substr(-1,len);
    // cppcheck-suppress invalidFunctionArg
    (void)str.substr(pos,-1);
    // no warning is expected for
    (void)str.substr(pos,len);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    (void)str.substr(pos, std::string::npos);
}

void invalidFunctionArg_std_wstring_substr(const std::wstring &str, std::size_t pos, std::size_t len) {
    // cppcheck-suppress invalidFunctionArg
    (void)str.substr(-1,len);
    // cppcheck-suppress invalidFunctionArg
    (void)str.substr(pos,-1);
    // no warning is expected for
    (void)str.substr(pos,len);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    (void)str.substr(pos, std::wstring::npos);
}

double invalidFunctionArg_log10(double d = 0.0) {
    // cppcheck-suppress invalidFunctionArg
    return log10(d);
}

void uninitvar_std_next(const std::vector<int> &v, int count)
{
    // No warning shall be shown:
    if (std::next(v.begin()) != v.end()) {}
    if (std::next(v.begin(), count) != v.end()) {}

    std::vector<int>::iterator it;
    // cppcheck-suppress uninitvar
    if (std::next(it) != v.end()) {}

    std::vector<int>::const_iterator const_it;
    // cppcheck-suppress uninitvar
    if (std::next(const_it) != v.end()) {}

    std::vector<int>::reverse_iterator rit;
    // cppcheck-suppress uninitvar
    if (std::next(rit) != v.rend()) {}

    std::vector<int>::const_reverse_iterator const_rit;
    // cppcheck-suppress uninitvar
    if (std::next(const_rit) != v.rend()) {}
}

void uninitvar_std_prev(const std::vector<int> &v, int count)
{
    // No warning shall be shown:
    if (std::prev(v.begin()) != v.end()) {}
    if (std::prev(v.begin(), count) != v.end()) {}

    std::vector<int>::iterator it;
    // cppcheck-suppress uninitvar
    if (std::prev(it) != v.end()) {}

    std::vector<int>::const_iterator const_it;
    // cppcheck-suppress uninitvar
    if (std::prev(const_it) != v.end()) {}

    std::vector<int>::reverse_iterator rit;
    // cppcheck-suppress uninitvar
    if (std::prev(rit) != v.rend()) {}

    std::vector<int>::const_reverse_iterator const_rit;
    // cppcheck-suppress uninitvar
    if (std::prev(const_rit) != v.rend()) {}
}

void overlappingWriteFunction_wcscat(wchar_t *src, wchar_t *dest)
{
    // No warning shall be shown:
    (void)wcscat(dest, src);
    // cppcheck-suppress overlappingWriteFunction
    (void)wcscat(src, src);
}

void overlappingWriteFunction_wcsxfrm(wchar_t *s1, const wchar_t *s2, size_t n)
{
    // No warning shall be shown:
    (void)wcsxfrm(s1, s2, n);
}

char * overlappingWriteFunction_strcat(char *src, char *dest)
{
    // No warning shall be shown:
    (void)strcat(dest, src);
    // cppcheck-suppress overlappingWriteFunction
    return strcat(src, src);
}

int nullPointer_wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void) std::wcsncmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void) std::wcsncmp(s1,NULL,n);
    return std::wcsncmp(s1,s2,n);
}

wchar_t* nullPointer_wcsncpy(wchar_t *s, const wchar_t *cs, size_t n)
{
    // cppcheck-suppress nullPointer
    (void) std::wcsncpy(NULL,cs,n);
    // cppcheck-suppress nullPointer
    (void) std::wcsncpy(s,NULL,n);
    return std::wcsncpy(s,cs,n);
}

char * overlappingWriteFunction_strncat(const char *src, char *dest, const std::size_t count)
{
    // No warning shall be shown:
    (void)strncat(dest, src, 42);
    (void)strncat(dest, src, count);
    (void)strncat(dest, dest, count);
    // cppcheck-suppress overlappingWriteFunction
    (void)strncat(dest, dest+1, 2);
    char buffer[] = "strncat";
    // cppcheck-suppress overlappingWriteFunction
    return strncat(buffer, buffer + 1, 3);
}

wchar_t * overlappingWriteFunction_wcsncat(const wchar_t *src, wchar_t *dest, const std::size_t count)
{
    // No warning shall be shown:
    (void)wcsncat(dest, src, 42);
    (void)wcsncat(dest, src, count);
    (void)wcsncat(dest, dest, count);
    // cppcheck-suppress overlappingWriteFunction
    (void)wcsncat(dest, dest+1, 2);
    wchar_t buffer[] = L"strncat";
    // cppcheck-suppress overlappingWriteFunction
    return wcsncat(buffer, buffer + 1, 3);
}

wchar_t * overlappingWriteFunction_wcscpy(wchar_t *src, wchar_t *dest)
{
    // No warning shall be shown:
    (void)wcscpy(dest, src);
    const wchar_t * destBuf = dest;
    // TODO-cppcheck-suppress overlappingWriteFunction  #10355
    (void)wcscpy(dest, destBuf);
    // cppcheck-suppress overlappingWriteFunction
    return wcscpy(src, src);
}

wchar_t * overlappingWriteFunction_wcsncpy(wchar_t *buf, const std::size_t count)
{
    // No warning shall be shown:
    (void)wcsncpy(&buf[0], &buf[3], count); // size is not known
    (void)wcsncpy(&buf[0], &buf[3], 3U);    // no-overlap
    // cppcheck-suppress overlappingWriteFunction
    return wcsncpy(&buf[0], &buf[3], 4U);
}

char * overlappingWriteFunction_strncpy(char *buf, const std::size_t count)
{
    // No warning shall be shown:
    (void)strncpy(&buf[0], &buf[3], count); // size is not known
    (void)strncpy(&buf[0], &buf[3], 3U);    // no-overlap
    // cppcheck-suppress overlappingWriteFunction
    return strncpy(&buf[0], &buf[3], 4U);
}

void * overlappingWriteFunction_memmove(void)
{
    // No warning shall be shown:
    char str[] = "memmove handles overlapping data well";
    return memmove(str,str+3,4);
}

std::bitset<10> std_bitset_test_ignoredReturnValue()
{
    std::bitset<10> b1("1111010000");
    // cppcheck-suppress ignoredReturnValue
    b1.test(2);
    return b1;
}

std::bitset<10> std_bitset_all_ignoredReturnValue()
{
    std::bitset<10> b1("1111010000");
    // cppcheck-suppress ignoredReturnValue
    b1.all();
    return b1;
}

std::bitset<10> std_bitset_none_ignoredReturnValue()
{
    std::bitset<10> b1("1111010000");
    // cppcheck-suppress ignoredReturnValue
    b1.none();
    return b1;
}

std::bitset<10> std_bitset_any_ignoredReturnValue()
{
    std::bitset<10> b1("1111010000");
    // cppcheck-suppress ignoredReturnValue
    b1.any();
    return b1;
}

std::bitset<10> std_bitset_size_ignoredReturnValue()
{
    std::bitset<10> b1("1111010000");
    // cppcheck-suppress ignoredReturnValue
    b1.size();
    return b1;
}

std::bitset<10> std_bitset_count_ignoredReturnValue()
{
    std::bitset<10> b1("1111010000");
    // cppcheck-suppress ignoredReturnValue
    b1.count();
    return b1;
}

void std_unordered_set_count_ignoredReturnValue(const std::unordered_set<int>& u)
{
    int i;
    // cppcheck-suppress [uninitvar, ignoredReturnValue]
    u.count(i);
}

void std_unordered_map_count_ignoredReturnValue(const std::unordered_map<int, int>& u)
{
    int i;
    // cppcheck-suppress [uninitvar, ignoredReturnValue]
    u.count(i);
}

void std_multimap_count_ignoredReturnValue(const std::multimap<int, int>& m)
{
    int i;
    // cppcheck-suppress [uninitvar, ignoredReturnValue]
    m.count(i);
}

void std_unordered_map_insert_unnitvar(std::unordered_set<int>& u)
{
    int i;
    // cppcheck-suppress uninitvar
    u.insert(i);
}

void std_unordered_map_emplace_unnitvar(std::unordered_set<int>& u)
{
    int i;
    // cppcheck-suppress uninitvar
    u.emplace(i);
}

int std_map_find_constref(std::map<int, int>& m) // #11857
{
    std::map<int, int>& r = m;
    std::map<int, int>::iterator it = r.find(42);
    int* p = &it->second;
    return ++*p;
}

void std_rotate_constref(std::vector<int>& v) // #13657
{
    std::rotate(v.begin(), v.begin() + 1, v.begin() + 2);
}

void std_queue_front_ignoredReturnValue(const std::queue<int>& q) {
    // cppcheck-suppress ignoredReturnValue
    q.front();
}

void std_priority_queue_top_ignoredReturnValue(const std::priority_queue<int>& pq) {
    // cppcheck-suppress ignoredReturnValue
    pq.top();
}

void std_tie_ignoredReturnValue(int a, int b)
{
    std::set<int> s;
    std::set<int>::iterator it;
    bool success;
    std::tie(it, success) = s.insert(1);
    // cppcheck-suppress ignoredReturnValue
    std::tie();
    // cppcheck-suppress ignoredReturnValue
    std::tie(a, b);
}

void std_exception_ignoredReturnValue(const std::exception& e)
{
    // cppcheck-suppress ignoredReturnValue
    e.what();
}

void valid_code()
{
    std::vector<int> vecInt{0, 1, 2};
    std::fill_n(vecInt.begin(), 2, 0);
    vecInt.push_back(1);
    vecInt.pop_back();
}

void returnValue_std_isgreater(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::isgreater(4,2) == 0) {}
    // @todo support floats
    if (std::isgreater(4.0f,2.0f) == 0) {}
}

void returnValue_std_isgreaterequal(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::isgreaterequal(4,2) == 0) {}
    // @todo support floats
    if (std::isgreaterequal(4.0f,2.0f) == 0) {}
}

void returnValue_std_isless(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::isless(4,2) == 0) {}
    // @todo support floats
    if (std::isless(4.0f,2.0f) == 0) {}
}

void returnValue_std_islessequal(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::islessequal(4,2) == 0) {}
    // @todo support floats
    if (std::islessequal(4.0f,2.0f) == 0) {}
}

void returnValue_std_islessgreater(void)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (std::islessgreater(4,2) == 0) {}
    // cppcheck-suppress knownConditionTrueFalse
    if (std::islessgreater(2,4) == 0) {}

    if (std::islessgreater(4.0f,2.0f) == 0) {} // @todo support floats
    if (std::islessgreater(2.0f,4.0f) == 0) {} // @todo support floats
}

void bufferAccessOutOfBounds(void)
{
    char a[5];
    std::strcpy(a,"abcd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    std::strcpy(a, "abcde");
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress terminateStrncpy
    std::strncpy(a,"abcde",5);
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    std::strncpy(a,"abcde",6);
}

void uninitvar_abs(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::abs(i);
}

void uninivar_imaxabs(void)
{
    intmax_t i1, i2;
    // cppcheck-suppress uninitvar
    (void)std::imaxabs(i1);
    // cppcheck-suppress uninitvar
    (void)imaxabs(i2);
}

void uninitvar_isalnum(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isalnum(i);
}

void uninitvar_isalpha(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isalpha(i);
}

void uninitvar_iscntrl(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::iscntrl(i);
}

void uninitvar_isdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isdigit(i);
}

void uninitvar_isgraph(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isgraph(i);
}

void uninitvar_islower(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::islower(i);
}

void uninitvar_isprint(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isprint(i);
}

void uninitvar_isspace(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isspace(i);
}

void uninitvar_isupper(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isupper(i);
}

void uninitvar_isxdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isxdigit(i);
}

void uninitvar_proj(void)
{
    double d;
    // cppcheck-suppress uninitvar
    const std::complex<double> dc(d,d);
    (void)std::proj(dc);
}

void uninitvar_acos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::acos(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::acos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::acos(ld);
}

void uninitvar_acosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::acoshf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::acosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::acoshl(ld);
}

void uninitvar_asctime(void)
{
    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress asctimeCalled
    (void)std::asctime(tm);
}

void uninitvar_sqrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::sqrt(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::sqrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::sqrt(ld);
}

void uninitvar_sinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::sinh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::sinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::sinh(ld);
}

void uninitvar_sin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::sin(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::sin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::sin(ld);
}

void uninitvar_asin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::asin(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::asin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::asin(ld);
}

void uninitvar_asinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::asinhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::asinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::asinhl(ld);
}

void uninitvar_wcsftime(wchar_t* ptr)
{
    size_t maxsize;
    const wchar_t* format;
    const struct tm* timeptr;
    // cppcheck-suppress uninitvar
    (void)std::wcsftime(ptr, maxsize, format, timeptr);
}

void uninitvar_tan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::tan(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tan(ld);
}

void uninitvar_tanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::tanh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tanh(ld);
}

void uninitvar_atan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::atan(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::atan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::atan(ld);
}

void uninitvar_tgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::tgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::tgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::tgammal(ld);
}

void uninitvar_trunc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::truncf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::trunc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::truncl(ld);
}

void uninitvar_atanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::atanhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::atanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::atanhl(ld);
}

void uninitvar_atan2(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::atan2(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::atan2(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::atan2(ld1,ld2);
}

void uninitvar_atof(void)
{
    const char * c;
    // cppcheck-suppress uninitvar
    (void)std::atof(c);
}

void uninitvar_atol(void)
{
    const char * c1, *c2, *c3;
    // cppcheck-suppress uninitvar
    (void)std::atoi(c1);

    // cppcheck-suppress uninitvar
    (void)std::atol(c2);

    // cppcheck-suppress uninitvar
    (void)std::atoll(c3);
}

void uninitvar_ceil(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::ceil(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::ceil(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::ceil(ld);
}

void uninitvar_copysign(void)
{
    float f1, f2;
    // cppcheck-suppress uninitvar
    (void)std::copysignf(f1, f2);

    double d1, d2;
    // cppcheck-suppress uninitvar
    (void)std::copysign(d1, d2);

    long double ld1, ld2;
    // cppcheck-suppress uninitvar
    (void)std::copysignl(ld1, ld2);
}

void uninitvar_cbrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cbrtf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cbrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cbrtl(ld);
}

void uninitvar_cos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cos(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cos(ld);
}

void uninitvar_clearerr(void)
{
    FILE * stream;
    // cppcheck-suppress uninitvar
    std::clearerr(stream);
}

void uninitvar_cosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::cosh(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::cosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::cosh(ld);
}

void uninitvar_feraiseexcept(void)
{
    int expects;
    // cppcheck-suppress uninitvar
    (void)std::feraiseexcept(expects);
}

void uninitvar_fesetexceptflag(const fexcept_t* flagp)
{
    int expects;
    // cppcheck-suppress uninitvar
    (void)std::fesetexceptflag(flagp, expects);
}

void uninitvar_feclearexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::feclearexcept(i);
}

void uninitvar_fesetenv(void)
{
    const fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)std::fesetenv(envp);
}

void uninitvar_fesetround(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fesetround(i);
}

void uninitvar_fetestexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fetestexcept(i);
}

void uninitvar_feupdateenv(void)
{
    const fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)std::feupdateenv(envp);
}

void uninitvar_ctime(void)
{
    const time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::ctime(tp);
}

void uninitvar_difftime(void)
{
    time_t t1,t2;
    // cppcheck-suppress uninitvar
    (void)std::difftime(t1, t2);
}

void uninitvar_div(void)
{
    int num;
    int denom;
    // cppcheck-suppress uninitvar
    (void)std::div(num,denom);
}

void uninitvar_imaxdiv(void)
{
    intmax_t numer1, numer2;
    intmax_t denom1, denom2;
    // cppcheck-suppress uninitvar
    (void)std::imaxdiv(numer1,denom1);
    // cppcheck-suppress uninitvar
    (void)imaxdiv(numer2,denom2);
}

void uninitvar_exit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    std::exit(i);
}

void uninitvar_erf(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::erff(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::erf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::erfl(ld);
}

void uninitvar_erfc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::erfcf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::erfc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::erfcl(ld);
}

void uninitvar_exp(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::exp(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::exp(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::exp(ld);
}

void uninitvar_exp2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::exp2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::exp2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::exp2l(ld);
}

void uninitvar_expm1(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::expm1f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::expm1(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::expm1l(ld);
}

void uninitvar_fabs(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::fabs(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::fabs(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::fabs(ld);
}

void uninitvar_fdim(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fdimf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fdim(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fdiml(ld1,ld2);
}

void uninitvar_fclose(void)
{
    // cppcheck-suppress unassignedVariable
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fclose(stream);
}

void uninitvar_ferror(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ferror(stream);
}

void uninitvar_feof(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::feof(stream);
}

void uninitvar_fflush(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fflush(stream);
}

void uninitvar_fgetc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgetc(stream);
}

void uninitvar_fgetwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgetwc(stream);
}

void uninitvar_fgetpos(void)
{
    FILE* stream;
    fpos_t *ptr;
    // cppcheck-suppress uninitvar
    (void)std::fgetpos(stream,ptr);
}

void uninitvar_floor(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::floor(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::floor(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::floor(ld);
}

void uninitvar_fma(void)
{
    float f1,f2,f3;
    // cppcheck-suppress uninitvar
    (void)std::fmaf(f1,f2,f3);

    double d1,d2,d3;
    // cppcheck-suppress uninitvar
    (void)std::fma(d1,d2,d3);

    long double ld1,ld2,ld3;
    // cppcheck-suppress uninitvar
    (void)std::fmal(ld1,ld2,ld3);
}

void uninitvar_fmax(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fmaxf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmax(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fmaxl(ld1,ld2);
}

void uninitvar_fmin(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fminf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmin(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fminl(ld1,ld2);
}

void uninitvar_fmod(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::fmod(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::fmod(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::fmod(ld1,ld2);
}

void uninitar_fopen(void)
{
    const char *filename;
    const char *mode;
    // cppcheck-suppress uninitvar
    FILE * fp = std::fopen(filename, mode);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
}

void uninitar_fprintf(FILE *Stream, const char *Format, int Argument)
{
    FILE *stream1, *stream2;
    const char *format1, *format2;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)std::fprintf(stream1, format1, argument1);
    // cppcheck-suppress uninitvar
    (void)std::fprintf(stream2, Format, Argument);
    // cppcheck-suppress uninitvar
    (void)std::fprintf(Stream, format2, Argument);
    // cppcheck-suppress uninitvar
    (void)std::fprintf(Stream, Format, argument2);

    // no warning is expected
    (void)std::fprintf(Stream, Format, Argument);
}

void uninitar_vfprintf(FILE *Stream, const char *Format, va_list Arg)
{
    FILE *stream1, *stream2;
    const char *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(stream1, format1, arg);
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(stream2, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)std::vfprintf(Stream, format2, Arg);

    // no warning is expected
    (void)std::vfprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vfprintf(Stream, Format, arg);
}

void uninitar_vfwprintf(FILE *Stream, const wchar_t *Format, va_list Arg)
{
    FILE *stream1, *stream2;
    const wchar_t *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(stream1, format1, arg);
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(stream2, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)std::vfwprintf(Stream, format2, Arg);

    // no warning is expected
    (void)std::vfwprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vfwprintf(Stream, Format, arg);
}

void uninitvar_fputc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputc(c,stream);
}

void uninitvar_fputwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputwc(c,stream);
}

void uninitvar_fputs(void)
{
    const char *string;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputs(string,stream);
}

void uninitvar_fputws(void)
{
    const wchar_t *string;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fputws(string,stream);
}

void uninitvar_fread(void)
{
    void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fread(ptr,size,nobj,stream);
}

void uninitvar_free(void)
{
    // cppcheck-suppress unassignedVariable
    void *block;
    // cppcheck-suppress uninitvar
    std::free(block);
}

void uninitvar_freopen(void)
{
    const char *filename;
    const char *mode;
    FILE *stream;
    // cppcheck-suppress uninitvar
    FILE * p = std::freopen(filename,mode,stream);
    std::fclose(p);
}

void uninitvar_frexp(void)
{
    float f1;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)std::frexp(f1,i1);

    double d1;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)std::frexp(d1,i2);

    long double ld1;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)std::frexp(ld1,i3);
}

void uninitvar_hypot(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::hypotf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::hypot(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::hypotl(ld1,ld2);
}

void uninitvar_fscanf(void)
{
    FILE *stream;
    const char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fscanf(stream,format,i);
}

void uninitvar_vfscanf(void)
{
    FILE *stream;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfscanf(stream,format,arg);
}

void uninitvar_vfwscanf(void)
{
    FILE *stream;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vfwscanf(stream,format,arg);
}

void uninitvar_fseek(void)
{
    FILE* stream;
    long int offset;
    int origin;
    // cppcheck-suppress uninitvar
    (void)std::fseek(stream,offset,origin);
}

void invalidFunctionArg_fseek(FILE* stream, long int offset, int origin)
{
    // cppcheck-suppress invalidFunctionArg
    (void)std::fseek(stream, offset, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fseek(stream, offset, 3);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fseek(stream, offset, 42+SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)std::fseek(stream, offset, SEEK_SET+42);
    // No warning is expected for
    (void)std::fseek(stream, offset, origin);
    (void)std::fseek(stream, offset, SEEK_SET);
    (void)std::fseek(stream, offset, SEEK_CUR);
    (void)std::fseek(stream, offset, SEEK_END);
}

void invalidFunctionArgBool_fseek(FILE* stream, long int offset, int origin)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::fseek(stream, offset, true);
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::fseek(stream, offset, false);
}

void uninitvar_fsetpos(void)
{
    FILE* stream;
    const fpos_t *ptr;
    // cppcheck-suppress uninitvar
    (void)std::fsetpos(stream,ptr);
}

wchar_t* nullPointer_fgetws(wchar_t* buffer, int n, FILE* stream)
{
    // cppcheck-suppress nullPointer
    (void)std::fgetws(NULL,n,stream);
    // cppcheck-suppress nullPointer
    (void)std::fgetws(buffer,n,NULL);
    // No warning is expected
    return std::fgetws(buffer, n, stream);
}

void nullPointer_wmemcmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::wmemcmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)std::wmemcmp(s1,NULL,n);
    (void)std::wmemcmp(s1,s2,n);
}


void nullPointer_memcmp(const void *s1, const void *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::memcmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)std::memcmp(s1,NULL,n);
    (void)std::memcmp(s1,s2,n);
}

void nullPointer_strncat(char *d, const char *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::strncat(NULL,s,n);
    // cppcheck-suppress nullPointer
    (void)std::strncat(d,NULL,n);
    // no warning is expected for
    (void)std::strncat(d,s,n);
}

void nullPointer_strcpy(char *dest, const char * const source)
{
    // cppcheck-suppress nullPointer
    (void)std::strcpy(NULL,source);
    // cppcheck-suppress nullPointer
    (void)std::strcpy(dest,NULL);

    // no warning shall be shown for
    (void)std::strcpy(dest,source);
}

void nullPointer_strcat(char *dest, const char * const source)
{
    // cppcheck-suppress nullPointer
    (void)std::strcat(NULL,source);
    // cppcheck-suppress nullPointer
    (void)std::strcat(dest,NULL);

    // no warning shall be shown for
    (void)std::strcat(dest,source);
}

void nullPointer_strncpy(char *d, const char *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::strncpy(NULL,s,n);
    // cppcheck-suppress nullPointer
    (void)std::strncpy(d,NULL,n);
    // no warning is expected for
    (void)std::strncpy(d,s,n);
}

void nullPointer_strncmp(const char *s1, const char *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::strncmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)std::strncmp(s1,NULL,n);
    (void)std::strncmp(s1,s2,n);
}

char* nullPointer_fgets(char *buffer, int n, FILE *stream)
{
    // cppcheck-suppress nullPointer
    (void)std::fgets(NULL,n,stream);
    // cppcheck-suppress nullPointer
    (void)std::fgets(buffer,n,NULL);
    // No warning is expected
    return std::fgets(buffer, n, stream);
}

void uninitvar_fgets(void)
{
    char *buffer;
    int n;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgets(buffer,n,stream);
}

void uninitvar_fgetws(void)
{
    wchar_t *buffer;
    int n;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fgetws(buffer,n,stream);
}

void uninitvar_ftell(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ftell(stream);
}

void uninitvar_fwide(void)
{
    FILE *stream;
    int mode;
    // cppcheck-suppress uninitvar
    (void)std::fwide(stream,mode);
}

void uninitvar_fwrite(void)
{
    const void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::fwrite(ptr,size,nobj,stream);
}

void uninitvar_mblen(void)
{
    const char *string;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::mblen(string,size);
}

void uninitvar_mbtowc(void)
{
    wchar_t* pwc;
    const char* pmb;
    size_t max;
    // cppcheck-suppress uninitvar
    (void)std::mbtowc(pwc,pmb,max);
}

void uninitvar_mbrlen(const char* p, size_t m, mbstate_t* s)
{
    const char* pmb1, *pmb2;
    size_t max1, max2;
    mbstate_t* ps1, *ps2;
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(pmb1,max1,ps1);
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(pmb2,m,s);
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(p,max2,s);
    // cppcheck-suppress uninitvar
    (void)std::mbrlen(p,m,ps2);
    // no warning is expected
    (void)std::mbrlen(p,m,s);
}

void nullPointer_mbrlen(const char* p, size_t m, mbstate_t* s)
{
    // no warning is expected: A call to the function with a null pointer as pmb resets the shift state (and ignores parameter max).
    (void)std::mbrlen(NULL,m,s);
    (void)std::mbrlen(NULL,0,s);
    // cppcheck-suppress nullPointer
    (void)std::mbrlen(p,m,NULL);
}

void uninitvar_btowc(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::btowc(c);
}

void uninitvar_mbsinit(void)
{
    const mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbsinit(ps);
}

void uninitvar_mbstowcs(void)
{
    wchar_t *ws;
    const char *s;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::mbstowcs(ws,s,n);
}

void uninitvar_mbsrtowcs(void)
{
    wchar_t* dest;
    const char* src;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbsrtowcs(dest,&src,max,ps);
}

void uninitvar_wctob(void)
{
    wint_t wc;
    // cppcheck-suppress uninitvar
    (void)std::wctob(wc);
}

void uninitvar_wctomb(void)
{
    char *s;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)std::wctomb(s,wc);
}

void uninitvar_wcstombs(void)
{
    char *mbstr;
    const wchar_t *wcstr;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcstombs(mbstr,wcstr,n);
}

void uninitvar_getc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::getc(stream);
}

void uninitvar_getwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::getwc(stream);
}

void uninitvar_ungetc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ungetc(c,stream);
}

void uninitvar_ungetwc(void)
{
    wint_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::ungetwc(c,stream);
}

void uninitvar_getenv(void)
{
    const char *name;
    // cppcheck-suppress uninitvar
    (void)std::getenv(name);
}

void uninitvar_gmtime(void)
{
    const time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::gmtime(tp);
}

void uninitvar_iswalnum(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswalnum(i);
}

void uninitvar_iswalpha(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswalpha(i);
}

void uninitvar_isblank(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::isblank(i);
}

void uninitvar_iswblank(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswblank(i);
}

void uninitvar_iswcntrl(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswcntrl(i);
}

void uninitvar_iswctype(void)
{
    wint_t c;
    wctype_t desc;
    // cppcheck-suppress uninitvar
    (void)std::iswctype(c,desc);
}

void uninitvar_iswdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswdigit(i);
}

void uninitvar_iswgraph(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswgraph(i);
}

void uninitvar_iswlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswlower(i);
}

void uninitvar_iswprint(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswprint(i);
}

void uninitvar_ispunct(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::ispunct(i);
}

void uninitvar_iswpunct(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswpunct(i);
}

void uninitvar_iswspace(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswspace(i);
}

void uninitvar_iswupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswupper(i);
}

void uninitvar_iswxdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::iswxdigit(i);
}

void uninitvar_towctrans(void)
{
    wint_t c;
    wctrans_t desc;
    // cppcheck-suppress uninitvar
    (void)std::towctrans(c,desc);
}

void uninitvar_towlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::towlower(i);
}

void uninitvar_towupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)std::towupper(i);
}

void uninitvar_wctrans(void)
{
    const char* property;
    // cppcheck-suppress uninitvar
    (void)std::wctrans(property);
}

void uninitvar_wctype(void)
{
    const char* property;
    // cppcheck-suppress uninitvar
    (void)std::wctype(property);
}

void uninitvar_labs(void)
{
    long int li;
    // cppcheck-suppress uninitvar
    (void)std::labs(li);

    long long int lli;
    // cppcheck-suppress uninitvar
    (void)std::llabs(lli);
}

void uninitvar_ldexp(void)
{
    float fd;
    int e1;
    // cppcheck-suppress uninitvar
    (void)std::ldexp(fd,e1);

    double dc;
    int e2;
    // cppcheck-suppress uninitvar
    (void)std::ldexp(dc,e2);

    long double ldc;
    int e3;
    // cppcheck-suppress uninitvar
    (void)std::ldexp(ldc,e3);
}

void invalidFunctionArg_lgamma(float f, double d, long double ld)
{
    (void)lgamma(d);
    // TODO cppcheck-suppress invalidFunctionArg
    (void)lgamma(-0.1);
    // cppcheck-suppress invalidFunctionArg
    (void)lgamma(0.0);
    (void)lgamma(0.1);

    (void)lgammaf(f);
    // TODO cppcheck-suppress invalidFunctionArg
    (void)lgammaf(-0.1f);
    // cppcheck-suppress invalidFunctionArg
    (void)lgammaf(0.0f);
    (void)lgammaf(0.1f);

    (void)lgammal(ld);
    // TODO cppcheck-suppress invalidFunctionArg
    (void)lgammal(-0.1L);
    // cppcheck-suppress invalidFunctionArg
    (void)lgammal(0.0L);
    (void)lgammal(0.1L);
}

void invalidFunctionArg_tgamma(float f, double d, long double ld)
{
    (void)tgamma(d);
    // TODO cppcheck-suppress invalidFunctionArg
    (void)tgamma(-0.1);
    // cppcheck-suppress invalidFunctionArg
    (void)tgamma(0.0);
    (void)tgamma(0.1);

    (void)tgammaf(f);
    // TODO cppcheck-suppress invalidFunctionArg
    (void)tgammaf(-0.1f);
    // cppcheck-suppress invalidFunctionArg
    (void)tgammaf(0.0f);
    (void)tgammaf(0.1f);

    (void)tgammal(ld);
    // TODO cppcheck-suppress invalidFunctionArg
    (void)tgammal(-0.1L);
    // cppcheck-suppress invalidFunctionArg
    (void)tgammal(0.0L);
    (void)tgammal(0.1L);
}

void uninitvar_lgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::lgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::lgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::lgammal(ld);
}

void uninitvar_rint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::rintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::rint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::rintl(ld);
}

void uninitvar_lrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::lrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::lrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::lrintl(ld);
}

void uninitvar_llrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::llrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::llrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::llrintl(ld);
}

void uninitvar_lround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::lroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::lround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::lroundl(ld);
}

void uninitvar_llround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::llroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::llround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::llroundl(ld);
}

void unusedScopedObject_std_monostate(void)
{
    // cppcheck-suppress unusedScopedObject
    std::monostate{};
}

void unusedScopedObject_std_logic_error()
{
    // cppcheck-suppress unusedScopedObject
    std::logic_error("abc");
}

void uninitvar_srand(void)
{
    unsigned int seed;
    // cppcheck-suppress uninitvar
    (void)std::srand(seed);
    // cppcheck-suppress ignoredReturnValue
    std::rand();
}

void uninitvar_ldiv(void)
{
    long int l1;
    long int l2;
    // cppcheck-suppress uninitvar
    (void)std::ldiv(l1,l2);

    long long int ll1;
    long long int ll2;
    // cppcheck-suppress uninitvar
    (void)std::lldiv(ll1,ll2);
}

void uninitvar_localtime(void)
{
    const time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::localtime(tp);
}

void uninitvar_log(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log(ld);
}

void uninitvar_fpclassify(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::fpclassify(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::fpclassify(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::fpclassify(ld);
}

void bool_isfinite(float f)
{
    // cppcheck-suppress compareBoolExpressionWithInt
    // cppcheck-suppress compareValueOutOfTypeRangeError
    if (std::isfinite(f)==123) {}
}

void uninitvar_isfinite(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::isfinite(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::isfinite(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::isfinite(ld);
}

void bool_isgreater(float f1, float f2)
{
    // cppcheck-suppress compareBoolExpressionWithInt
    // cppcheck-suppress compareValueOutOfTypeRangeError
    if (std::isgreater(f1,f2)==123) {}
}

void uninitvar_isgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::isgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::isgreater(ld1,ld2);
}

void bool_isgreaterequal(float f1, float f2)
{
    // cppcheck-suppress compareBoolExpressionWithInt
    // cppcheck-suppress compareValueOutOfTypeRangeError
    if (std::isgreaterequal(f1, f2)==123) {}
}

void uninitvar_isgreaterequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::isgreaterequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isgreaterequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::isgreaterequal(ld1,ld2);
}

void bool_isinf(float f)
{
    // cppcheck-suppress compareBoolExpressionWithInt
    // cppcheck-suppress compareValueOutOfTypeRangeError
    if (std::isinf(f)==123) {}
}

void uninitvar_isinf(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::isinf(d);
}

void uninitvar_logb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::logbf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::logb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::logbl(ld);
}

void uninitvar_isless(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::isless(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isless(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::isless(ld1,ld2);
}

void uninitvar_islessequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::islessequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::islessequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::islessequal(ld1,ld2);
}

void uninitvar_islessgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::islessgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::islessgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::islessgreater(ld1,ld2);
}

void uninitvar_nan(void)
{
    const char *tagp1, *tagp2, *tagp3;
    // cppcheck-suppress uninitvar
    (void)std::nanf(tagp1);
    // cppcheck-suppress uninitvar
    (void)std::nan(tagp2);
    // cppcheck-suppress uninitvar
    (void)std::nanl(tagp3);
}

void uninitvar_isnan(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::isnan(d);
}

void uninitvar_isnormal(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::isnormal(d);
}

void uninitvar_isunordered(void)
{
    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::isunordered(d1,d2);
}

void uninitvar_ilogb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::ilogb(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::ilogb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::ilogb(ld);
}

void uninitvar_log10(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log10(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log10(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log10(ld);
}

void uninitvar_log1p(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log1pf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log1p(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log1pl(ld);
}

void uninitvar_log2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::log2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::log2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::log2l(ld);
}

void uninitvar_nearbyint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::nearbyintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::nearbyint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::nearbyintl(ld);
}

void uninitvar_nextafter(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::nextafterf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::nextafter(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::nextafterl(ld1,ld2);
}

void uninitvar_nexttoward(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::nexttowardf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::nexttoward(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::nexttowardl(ld1,ld2);
}

void uninitvar_longjmp(void)
{
    jmp_buf env;
    int val;
    // cppcheck-suppress uninitvar
    (void)std::longjmp(env,val);
}

void uninitvar_malloc(void)
{
    size_t size;
    // cppcheck-suppress [uninitvar, cstyleCast, unusedAllocatedMemory]
    int *p = (int*)std::malloc(size);
    free(p);
}

void uninitvar_memchr(void)
{
    void *cs;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memchr(cs,c,n);
}

void uninitvar_wmemchr(void)
{
    wchar_t *cs;
    wchar_t c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemchr(cs,c,n);
}

void uninitvar_memcmp(void)
{
    const void *s1;
    const void *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memcmp(s1,s2,n);
}

void uninitvar_wmemcmp(void)
{
    const wchar_t *s1;
    const wchar_t *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemcmp(s1,s2,n);
}

void uninitvar_memcpy(void)
{
    void *ct;
    const void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memcpy(ct,cs,n);
}

void uninitvar_wmemcpy(void)
{
    wchar_t *cs;
    const wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemcpy(cs,c,n);
}

void uninitvar_memmove(void)
{
    void *ct;
    const void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memmove(ct,cs,n);
}

void uninitvar_wmemmove(void)
{
    wchar_t *cs;
    wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemmove(cs,c,n);
}

void uninitvar_memset(void)
{
    void *s;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::memset(s,c,n);
}

void uninitvar_wmemset(void)
{
    wchar_t *cs;
    wchar_t c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wmemset(cs,c,n);
}

void uninitvar_mktime(void)
{
    struct tm *tp;
    // cppcheck-suppress uninitvar
    (void)std::mktime(tp);
}

void uninivar_modf(void)
{
    float f1;
    float *f2;
    // cppcheck-suppress uninitvar
    (void)std::modf(f1,f2);

    double d1;
    double *d2;
    // cppcheck-suppress uninitvar
    (void)std::modf(d1,d2);

    long double ld1;
    long double *ld2;
    // cppcheck-suppress uninitvar
    (void)std::modf(ld1,ld2);
}

void uninivar_perror(void)
{
    const char *string;
    // cppcheck-suppress uninitvar
    (void)std::perror(string);
}

void uninitvar_pow(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::pow(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::pow(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::pow(ld1,ld2);
}

void uninitvar_remainder(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)std::remainderf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)std::remainder(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)std::remainderl(ld1,ld2);
}

void uninitvar_remquo(void)
{
    float f1,f2;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)std::remquof(f1,f2,i1);

    double d1,d2;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)std::remquo(d1,d2,i2);

    long double ld1,ld2;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)std::remquol(ld1,ld2,i3);
}

void uninivar_printf(const char *Format, int Argument)
{
    const char * format_1, * format_2, * format_3;
    int argument1, argument2;
    // no warning is expected
    (void)std::printf("x");
    // cppcheck-suppress uninitvar
    (void)std::printf(format_1,argument1);
    // cppcheck-suppress uninitvar
    (void)std::printf(Format,argument2);
    // cppcheck-suppress uninitvar
    (void)std::printf(format_2,Argument);
    // cppcheck-suppress uninitvar
    (void)std::printf(format_3,1);

    // no warning is expected
    (void)std::printf(Format,Argument);
}

void uninivar_vprintf(const char *Format, va_list Arg)
{
    const char * format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vprintf(format1,arg);
    // cppcheck-suppress uninitvar
    (void)std::vprintf(format2,Arg);

    // no warning is expected
    (void)std::vprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vprintf(Format,arg);
}

void uninivar_vwprintf(const wchar_t *Format, va_list Arg)
{
    const wchar_t * format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vwprintf(format1,arg);
    // cppcheck-suppress uninitvar
    (void)std::vwprintf(format2,Arg);

    // no warning is expected
    (void)std::vwprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vwprintf(Format,arg);
}

void uninivar_bsearch(void)
{
    const void* key;
    const void* base;
    size_t num;
    size_t size;
    // cppcheck-suppress [uninitvar, cstyleCast]
    (void)std::bsearch(key,base,num,size,(int (*)(const void*,const void*))strcmp);
}

void minsize_bsearch(const void* key, const void* base,
                     size_t num, size_t size,
                     int (*compar)(const void*,const void*))
{
    const int Base[3] = {42, 43, 44};

    (void)std::bsearch(key,Base,2,size,(int (*)(const void*,const void*))strcmp); // cppcheck-suppress cstyleCast
    (void)std::bsearch(key,Base,3,size,(int (*)(const void*,const void*))strcmp); // cppcheck-suppress cstyleCast
    (void)std::bsearch(key,Base,4,size,(int (*)(const void*,const void*))strcmp); // cppcheck-suppress cstyleCast

    (void)std::bsearch(key,base,2,size,(int (*)(const void*,const void*))strcmp); // cppcheck-suppress cstyleCast
}

void uninitvar_qsort(void)
{
    void *base;
    size_t n;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::qsort(base,n,size, (int (*)(const void*,const void*))strcmp); // cppcheck-suppress cstyleCast
}

void uninitvar_stable_sort(std::vector<int>& v)
{
    std::vector<int>::iterator end;
    // cppcheck-suppress uninitvar
    std::stable_sort(v.begin(), end);
}

void uninitvar_merge(const std::vector<int>& a, const std::vector<int>& b)
{
    std::vector<int>::iterator dst;
    // cppcheck-suppress uninitvar
    std::merge(a.begin(), a.end(), b.begin(), b.end(), dst);
}

void uninitvar_push_heap(std::vector<int>& v)
{
    std::vector<int>::iterator end;
    // cppcheck-suppress uninitvar
    std::push_heap(v.begin(), end);
}

void uninitvar_copy_n(const std::vector<int>& v)
{
    std::vector<int>::iterator dst;
    // cppcheck-suppress [uninitvar, invalidFunctionArg]
    std::copy_n(v.begin(), -1, dst);
}

void uninitvar_iota(std::vector<int>& v)
{
    int i;
    // cppcheck-suppress uninitvar
    std::iota(v.begin(), v.end(), i);
}

void uninitvar_putc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::putc(c,stream);
}

void uninitvar_putwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)std::putc(c,stream);
}

void uninitvar_putchar(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::putchar(c);
}

void uninitvar_putwchar(void)
{
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)std::putwchar(c);
}

void uninitvar_puts(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    (void)std::puts(s);
}

void uninitvar_realloc(void)
{
    void *block;
    size_t newsize;
    // cppcheck-suppress uninitvar
    void *p = std::realloc(block, newsize);
    free(p);
}

void uninitvar_remove(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    (void)std::remove(s);
}

void uninitvar_rename(void)
{
    const char *s1;
    const char *s2;
    // cppcheck-suppress uninitvar
    (void)std::rename(s1,s2);
}

void uninitvar_rewind(void)
{
    FILE *f;
    // cppcheck-suppress uninitvar
    (void)std::rewind(f);
}

void uninitvar_round(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)std::roundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)std::round(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::roundl(ld);
}

void uninivar_scalbn(void)
{
    float f;
    int i1;
    // cppcheck-suppress uninitvar
    (void)std::scalbnf(f,i1);

    double d;
    int i2;
    // cppcheck-suppress uninitvar
    (void)std::scalbn(d,i2);

    long double ld;
    int i3;
    // cppcheck-suppress uninitvar
    (void)std::scalbnl(ld,i3);
}

void uninivar_scalbln(void)
{
    float f;
    long int i1;
    // cppcheck-suppress uninitvar
    (void)std::scalblnf(f,i1);

    double d;
    long int i2;
    // cppcheck-suppress uninitvar
    (void)std::scalbln(d,i2);

    long double ld;
    long int i3;
    // cppcheck-suppress uninitvar
    (void)std::scalblnl(ld,i3);
}

void uninitvar_signbit(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)std::signbit(d);
}

void uninivar_signal(void)
{
    int i;
    // cppcheck-suppress uninitvar
    std::signal(i, exit);
}

void uninivar_raise(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::raise(i);
}

void uninivar_scanf(void)
{
    const char *format;
    char str[42];
    // cppcheck-suppress uninitvar
    (void)std::scanf(format, str);
}

void uninivar_vsscanf(void)
{
    const char *s;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vsscanf(s,format,arg);
}

void uninivar_vswscanf(void)
{
    const wchar_t *s;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vswscanf(s,format,arg);
}

void uninivar_vscanf(void)
{
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vscanf(format,arg);
}

void uninivar_vwscanf(void)
{
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vwscanf(format,arg);
}

void uninivar_setbuf(void)
{
    FILE *stream;
    char *buf;
    // cppcheck-suppress uninitvar
    (void)std::setbuf(stream,buf);
}

void nullPointer_setbuf(FILE *stream, char *buf)
{
    // cppcheck-suppress nullPointer
    std::setbuf(NULL,buf);
    std::setbuf(stream,NULL);
    std::setbuf(stream,buf);
}

int bufferAccessOutOfBounds_setvbuf(FILE* stream, int mode, size_t size)
{
    char buf[42]={0};
    // cppcheck-suppress bufferAccessOutOfBounds
    (void) std::setvbuf(stream, buf, mode, 43);
    return std::setvbuf(stream, buf, mode, 42);
}

int nullPointer_setvbuf(FILE* stream, char *buf, int mode, size_t size)
{
    // cppcheck-suppress nullPointer
    (void) std::setvbuf(NULL, buf, mode, size);
    (void) std::setvbuf(stream, NULL, mode, size);
    return std::setvbuf(stream, buf, mode, size);
}

void uninivar_setvbuf(void)
{
    FILE *stream;
    char *buf;
    int mode;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)std::setvbuf(stream,buf,mode,size);
}

void uninitvar_strcat(char *dest, const char * const source)
{
    char *deststr1, *deststr2;
    const char *srcstr1, *srcstr2;
    // cppcheck-suppress uninitvar
    (void)std::strcat(deststr1,srcstr1);
    // cppcheck-suppress uninitvar
    (void)std::strcat(dest,srcstr2);
    // cppcheck-suppress uninitvar
    (void)std::strcat(deststr2,source);

    // no warning shall be shown for
    (void)std::strcat(dest,source);
}

void uninitvar_wcscat(wchar_t *dest, const wchar_t * const source)
{
    wchar_t *deststr_1, *deststr_2;
    const wchar_t *srcstr1, *srcstr2;
    // cppcheck-suppress uninitvar
    (void)std::wcscat(deststr_1,srcstr1);
    // cppcheck-suppress uninitvar
    (void)std::wcscat(dest,srcstr2);
    // cppcheck-suppress uninitvar
    (void)std::wcscat(deststr_2,source);

    // no warning shall be shown for
    (void)std::wcscat(dest,source);
}

void uninivar_wcrtomb(void)
{
    char *s;
    wchar_t wc;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)std::wcrtomb(s,wc,ps);
}

void uninivar_strchr(void)
{
    const char *cs;
    int c;
    // cppcheck-suppress uninitvar
    (void)std::strchr(cs,c);
}

void uninivar_wcschr(void)
{
    const wchar_t *cs;
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)std::wcschr(cs,c);
}

void uninivar_strcmp(void)
{
    const char *str1;
    const char *str2;
    // cppcheck-suppress uninitvar
    (void)std::strcmp(str1,str2);
}

void uninivar_wcscmp(void)
{
    const wchar_t *str1;
    const wchar_t *str2;
    // cppcheck-suppress uninitvar
    (void)std::wcscmp(str1,str2);
}

void uninivar_strcpy(void)
{
    char *str1;
    const char *str2;
    // cppcheck-suppress uninitvar
    (void)std::strcpy(str1,str2);
}

void uninivar_wcscpy(void)
{
    wchar_t *str1;
    const wchar_t *str2;
    // cppcheck-suppress uninitvar
    (void)std::wcscpy(str1,str2);
}

void uninivar_strftime(void)
{
    char *s;
    size_t max;
    const char *fmt;
    const struct tm *p;
    // cppcheck-suppress uninitvar
    (void)std::strftime(s,max,fmt,p);
}

void uninivar_strlen(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    (void)std::strlen(s);
}

void uninivar_wcslen(void)
{
    const wchar_t *s;
    // cppcheck-suppress uninitvar
    (void)std::wcslen(s);
}

void uninivar_strncpy(void)
{
    char *s;
    const char *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::strncpy(s,ct,n);
}

void uninivar_strpbrk(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strpbrk(cs,ct);
}

void uninivar_strncat(char *Ct, const char *S, size_t N)
{
    char *ct_1, *ct_2;
    const char *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)std::strncat(ct_1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)std::strncat(ct_2,S,N);
    // cppcheck-suppress uninitvar
    (void)std::strncat(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)std::strncat(Ct,S,n2);

    // no warning is expected for
    (void)std::strncat(Ct,S,N);
}

void uninivar_wcsncat(wchar_t *Ct, const wchar_t *S, size_t N)
{
    wchar_t *ct_1, *ct_2;
    const wchar_t *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(ct_1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(ct_2,S,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncat(Ct,S,n2);

    // no warning is expected for
    (void)std::wcsncat(Ct,S,N);
}

void uninivar_strncmp(const char *Ct, const char *S, size_t N)
{
    const char *ct1, *ct2;
    const char *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)std::strncmp(ct1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)std::strncmp(ct2,S,N);
    // cppcheck-suppress uninitvar
    (void)std::strncmp(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)std::strncmp(Ct,S,n2);

    // no warning is expected for
    (void)std::strncmp(Ct,S,N);
}

void uninivar_wcsncmp(const wchar_t *Ct, const wchar_t *S, size_t N)
{
    const wchar_t *ct1, *ct2;
    const wchar_t *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(ct1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(ct2,S,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)std::wcsncmp(Ct,S,n2);

    // no warning is expected for
    (void)std::wcsncmp(Ct,S,N);
}

void uninivar_strstr(void)
{
    char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strstr(cs,ct);
}

void uninivar_wcsstr(void)
{
    wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcsstr(cs,ct);
}

void uninivar_strspn(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strspn(cs,ct);
}

void uninivar_strxfrm(void)
{
    char *ds;
    const char *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::strxfrm(ds,ss,n);
}

void uninivar_wcsxfrm(void)
{
    wchar_t *ds;
    const wchar_t *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcsxfrm(ds,ss,n);
}

void uninivar_wcsspn(void)
{
    const wchar_t *ds;
    const wchar_t *ss;
    // cppcheck-suppress uninitvar
    (void)std::wcsspn(ds,ss);
}

void uninivar_setlocale(void)
{
    int category;
    const char* locale;
    // cppcheck-suppress uninitvar
    (void)std::setlocale(category,locale);
}
void uninivar_strerror(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)std::strerror(i);
}

void uninivar_strcspn(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strcspn(cs,ct);
}

void uninivar_wcscspn(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcscspn(cs,ct);
}

void uninivar_wcspbrk(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcspbrk(cs,ct);
}

void uninivar_wcsncpy(void)
{
    wchar_t *cs;
    const wchar_t *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)std::wcsncpy(cs,ct,n);
}

void uninivar_strcoll(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strcoll(cs,ct);
}

void uninivar_wcscoll(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)std::wcscoll(cs,ct);
}

void uninivar_strrchr(void)
{
    const char * str;
    int c;
    // cppcheck-suppress uninitvar
    (void)std::strrchr(str,c);
}

void uninivar_wcsrchr(void)
{
    wchar_t* ws;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)std::wcsrchr(ws,wc);
}

void uninivar_wcsrtombs(void)
{
    char *dst;
    const wchar_t * p;;
    size_t len;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)std::wcsrtombs(dst,&p,len,ps);
}

void uninivar_strtok(void)
{
    char *s;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)std::strtok(s,ct);
}

void uninivar_strtoimax(void)
{
    const char *s1, *s2;
    char **endp1, **endp2;
    int base1, base2;
    // cppcheck-suppress uninitvar
    (void)std::strtoimax(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)std::strtoumax(s2,endp2,base2);
}

void uninivar_strtof(void)
{
    const char *s1, *s2, *s3;
    char **endp1, **endp2, **endp3;
    // cppcheck-suppress uninitvar
    (void)std::strtof(s1,endp1);
    // cppcheck-suppress uninitvar
    (void)std::strtod(s2,endp2);
    // cppcheck-suppress uninitvar
    (void)std::strtold(s3,endp3);
}

void uninivar_strtol(void)
{
    const char *s1,*s2,*s3,*s4,*s5,*s6,*s7,*s8;
    char **endp1, **endp2, **endp3, **endp4, **endp5, **endp6, **endp7, **endp8;
    int base1, base2, base3, base4, base5, base6, base7, base8;

    // cppcheck-suppress uninitvar
    (void)std::strtol(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)std::strtoll(s2,endp2,base2);
    // cppcheck-suppress uninitvar
    (void)std::strtoul(s3,endp3,base3);
    // cppcheck-suppress uninitvar
    (void)std::strtoull(s4,endp4,base4);
    // cppcheck-suppress uninitvar
    (void)std::strtoimax(s5,endp5,base5);
    // cppcheck-suppress uninitvar
    (void)strtoimax(s6,endp6,base6);
    // cppcheck-suppress uninitvar
    (void)std::strtoumax(s7,endp7,base7);
    // cppcheck-suppress uninitvar
    (void)strtoumax(s8,endp8,base8);
}

void uninitvar_time(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)std::time(tp);
}

void uninitvar_tmpnam(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)std::tmpnam(s);
}

void uninivar_tolower(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::tolower(c);
}

void uninivar_toupper(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)std::toupper(c);
}

void uninivar_wcstof(void)
{
    const wchar_t *s1, *s2, *s3;
    wchar_t **endp1, **endp2, **endp3;
    // cppcheck-suppress uninitvar
    (void)std::wcstof(s1,endp1);
    // cppcheck-suppress uninitvar
    (void)std::wcstod(s2,endp2);
    // cppcheck-suppress uninitvar
    (void)std::wcstold(s3,endp3);
}

void uninivar_stoX(void)
{
    std::string str;
    std::wstring wstr;
    size_t* idx1;
    size_t* idx2;
    size_t* idx3;
    size_t* idx4;
    size_t* idx5;
    size_t* idx6;
    size_t* idx7;
    size_t* idx8;
    size_t* idx9;
    size_t* idx10;
    size_t* idx11;
    size_t* idx12;
    size_t* idx13;
    size_t* idx14;
    size_t* idx15;
    size_t* idx16;
    // cppcheck-suppress uninitvar
    (void)std::stod(str,idx1);
    // cppcheck-suppress uninitvar
    (void)std::stod(wstr,idx2);
    // cppcheck-suppress uninitvar
    (void)std::stof(str,idx3);
    // cppcheck-suppress uninitvar
    (void)std::stof(wstr,idx4);
    // cppcheck-suppress uninitvar
    (void)std::stoi(str,idx5);
    // cppcheck-suppress uninitvar
    (void)std::stoi(wstr,idx6);
    // cppcheck-suppress uninitvar
    (void)std::stol(str,idx7);
    // cppcheck-suppress uninitvar
    (void)std::stol(wstr,idx8);
    // cppcheck-suppress uninitvar
    (void)std::stold(str,idx9);
    // cppcheck-suppress uninitvar
    (void)std::stold(wstr,idx10);
    // cppcheck-suppress uninitvar
    (void)std::stoll(str,idx11);
    // cppcheck-suppress uninitvar
    (void)std::stoll(wstr,idx12);
    // cppcheck-suppress uninitvar
    (void)std::stoul(str,idx13);
    // cppcheck-suppress uninitvar
    (void)std::stoul(wstr,idx14);
    // cppcheck-suppress uninitvar
    (void)std::stoull(str,idx15);
    // cppcheck-suppress uninitvar
    (void)std::stoull(wstr,idx16);
}

void uninivar_to_string(void)
{
    int i;
    long l;
    long long ll;
    unsigned u;
    unsigned long ul;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::to_string(i);
    // cppcheck-suppress uninitvar
    (void)std::to_string(l);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ll);
    // cppcheck-suppress uninitvar
    (void)std::to_string(u);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ul);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ull);
    // cppcheck-suppress uninitvar
    (void)std::to_string(f);
    // cppcheck-suppress uninitvar
    (void)std::to_string(d);
    // cppcheck-suppress uninitvar
    (void)std::to_string(ld);
}

void uninivar_to_wstring(void)
{
    int i;
    long l;
    long long ll;
    unsigned u;
    unsigned long ul;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(i);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(l);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ll);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(u);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ul);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ull);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(f);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(d);
    // cppcheck-suppress uninitvar
    (void)std::to_wstring(ld);
}

void uninivar_mbrtowc(void)
{
    wchar_t* pwc;
    const char* pmb;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)std::mbrtowc(pwc,pmb,max,ps);
}

void uninivar_wcstok(void)
{
    wchar_t *s;
    const wchar_t *ct;
    wchar_t **ptr;
    // cppcheck-suppress uninitvar
    (void)std::wcstok(s,ct,ptr);
}

void uninivar_wcstoimax(void)
{
    const wchar_t *s1, *s2;
    wchar_t ** endp1, **endp2;
    int base1, base2;
    // cppcheck-suppress uninitvar
    (void)std::wcstoimax(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)std::wcstoumax(s2,endp2,base2);
}

void uninivar_wcstol(void)
{
    const wchar_t *s1,*s2,*s3,*s4,*s5,*s6,*s7,*s8;
    wchar_t **endp1, **endp2, **endp3, **endp4, **endp5, **endp6, **endp7, **endp8;
    int base1, base2, base3, base4, base5, base6, base7, base8;
    // cppcheck-suppress uninitvar
    (void)std::wcstol(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)std::wcstoll(s2,endp2,base2);
    // cppcheck-suppress uninitvar
    (void)std::wcstoul(s3,endp3,base3);
    // cppcheck-suppress uninitvar
    (void)std::wcstoull(s4,endp4,base4);
    // cppcheck-suppress uninitvar
    (void)std::wcstoimax(s5,endp5,base5);
    // cppcheck-suppress uninitvar
    (void)wcstoimax(s6,endp6,base6);
    // cppcheck-suppress uninitvar
    (void)std::wcstoumax(s7,endp7,base7);
    // cppcheck-suppress uninitvar
    (void)wcstoumax(s8,endp8,base8);
}

void uninitvar_wprintf(const wchar_t *Format, int Argument)
{
    const wchar_t *format1, *format2, *format3;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)std::wprintf(format1,argument1);
    // cppcheck-suppress uninitvar
    (void)std::wprintf(format2);
    // cppcheck-suppress uninitvar
    (void)std::wprintf(Format,argument2);
    // cppcheck-suppress uninitvar
    (void)std::wprintf(format3,Argument);
    // no warning is expected
    (void)std::wprintf(Format,Argument);
    (void)std::wprintf(Format);
}

void uninitvar_sprintf(void)
{
    char *s;
    const char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::sprintf(s,format,i);
}

void uninitvar_swprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    // cppcheck-suppress uninitvar
    (void)std::swprintf(s,n,format);
}

void uninitvar_vsprintf(void)
{
    char *s;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vsprintf(s,format,arg);
}

void nullPointer_vsprintf(va_list arg,const char *format)
{
    char *s = NULL;
    (void)std::vsprintf(s,format,arg); // Its allowed to provide 's' as NULL pointer
    // cppcheck-suppress nullPointer
    (void)std::vsprintf(s,NULL,arg);
}

void uninitvar_vswprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vswprintf(s,n,format,arg);
}

void uninivar_fwprintf(void)
{
    FILE* stream;
    const wchar_t* format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fwprintf(stream,format,i);
}

void uninivar_snprintf(char *S, size_t N, const char *Format, int Int)
{
    size_t n1, n2;
    const char *format1, *format2;
    int i1, i2;
    char *s1, *s2;
    // cppcheck-suppress uninitvar
    (void)std::snprintf(s1,n1,format1,i1);
    // cppcheck-suppress uninitvar
    (void)std::snprintf(S,n2,Format,Int); // n is uninitialized
    // cppcheck-suppress uninitvar
    (void)std::snprintf(S,N,format2,Int); // format is uninitialized
    // cppcheck-suppress uninitvar
    (void)std::snprintf(S,N,Format,i2); // i is uninitialized
    // cppcheck-suppress uninitvar
    (void)std::snprintf(s2,N,Format,Int);

    // no warning is expected for
    (void)std::snprintf(S,N,Format,Int);
}

void uninivar_vsnprintf(char *S, size_t N, const char *Format, va_list Arg)
{
    char *s1, *s2;
    size_t n1, n2;
    const char *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(s1,n1,format1,arg);
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(s2,N,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(S,n2,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)std::vsnprintf(S,N,format2,Arg);

    // no warning is expected for
    (void)std::vsnprintf(S,N,Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)std::vsnprintf(S,N,Format,arg);
}

void uninivar_wscanf(void)
{
    const wchar_t *format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::wscanf(format1);
    // cppcheck-suppress uninitvar
    (void)std::wscanf(format2,&i);
}

void uninivar_sscanf(void)
{
    const char *string1, *string2;
    const char * format;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::sscanf(string1,format);
    // cppcheck-suppress uninitvar
    (void)std::sscanf(string2,format,&i);
}

void uninivar_fwscanf(void)
{
    FILE* stream;
    const wchar_t* format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::fwscanf(stream,format1);
    // cppcheck-suppress uninitvar
    (void)std::fwscanf(stream,format2,&i);
}

void uninivar_swscanf(void)
{
    const wchar_t* s;
    const wchar_t* format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)std::swscanf(s,format1);
    // cppcheck-suppress uninitvar
    (void)std::swscanf(s,format2,&i);
}

void uninitvar_system(void)
{
    const char *c;
    // cppcheck-suppress uninitvar
    (void)std::system(c);
}

#ifndef __STDC_NO_THREADS__

void nullPointer_mtx_destroy(mtx_t *mutex )
{
    // cppcheck-suppress nullPointer
    mtx_destroy(nullptr);
    mtx_destroy(mutex);
}

void nullPointer_mtx_lock( mtx_t *mutex )
{
    // cppcheck-suppress nullPointer
    mtx_lock(nullptr);
    mtx_lock(mutex);
}

void nullPointer_mtx_trylock( mtx_t *mutex )
{
    // cppcheck-suppress nullPointer
    mtx_trylock(nullptr);
    mtx_trylock(mutex);
}

int nullPointer_mtx_timedlock( mtx_t *mutex, const struct timespec *time_point )
{
    // cppcheck-suppress nullPointer
    (void) mtx_timedlock(nullptr, time_point);
    // cppcheck-suppress nullPointer
    (void) mtx_timedlock(mutex, nullptr);
    return mtx_timedlock(mutex, time_point);
}
#endif

void nullPointer_system(const char *c)
{
    // If a null pointer is given, command processor is checked for existence
    (void)std::system(NULL);
    (void)std::system(c);
}

void uninitvar_setw(void)
{
    int i;
    // cppcheck-suppress [uninitvar,valueFlowBailoutIncompleteVar]
    std::cout << std::setw(i);
}

void uninitvar_setiosflags(void)
{
    std::ios_base::fmtflags mask;
    // TODO cppcheck-suppress uninitvar
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    std::cout << std::setiosflags(mask); // #6987 - false negative
}

void uninitvar_resetiosflags(void)
{
    std::ios_base::fmtflags mask;
    // TODO cppcheck-suppress uninitvar
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    std::cout << std::resetiosflags(mask); // #6987 - false negative
}

void uninitvar_setfill(void)
{
    char c;
    // cppcheck-suppress [uninitvar,valueFlowBailoutIncompleteVar]
    std::cout << std::setfill(c);

    wchar_t wc;
    // cppcheck-suppress uninitvar
    std::wcout << std::setfill(wc);
}

void uninitvar_setprecision(void)
{
    int p;
    // cppcheck-suppress [uninitvar,valueFlowBailoutIncompleteVar]
    std::cout << std::setprecision(p);
}

void uninitvar_setbase(void)
{
    int p;
    // cppcheck-suppress [uninitvar,valueFlowBailoutIncompleteVar]
    std::cout << std::setbase(p);
}

// cppcheck-suppress passedByValue
void uninitvar_find(std::string s)
{
    // testing of size_t find (const string& str, size_t pos = 0)
    size_t pos1, pos2, pos3, pos4, pos5, pos6, pos7;
    // cppcheck-suppress uninitvar
    (void)s.find("find",pos1); // #6991

    // testing of  size_t find (const char* s, size_t pos = 0) const;
    char *pc, *pc2;
    // cppcheck-suppress uninitvar
    (void)s.find(pc,0);
    // cppcheck-suppress uninitvar
    (void)s.find(pc,pos2);
    // cppcheck-suppress uninitvar
    (void)s.find("test",pos3);

    // testing of size_t find (char c, size_t pos = 0) const;
    char c;
    // cppcheck-suppress uninitvar
    (void)s.find(c,pos4);

    // testing of size_t find (const char* pc, size_t pos, size_t n) const;
    size_t n1,n2,n3;
    // cppcheck-suppress uninitvar
    (void)s.find(pc,pos5,n1); // #6991
    // cppcheck-suppress uninitvar
    (void)s.find("test",pos6,n2);
    // cppcheck-suppress uninitvar
    (void)s.find("test",1,n3);
    // cppcheck-suppress uninitvar
    (void)s.find("test",pos7,1);
    // cppcheck-suppress uninitvar
    (void)s.find(pc2,1,1);
}

void uninivar_ifstream_read(std::ifstream &f)
{
    int size;
    char buffer[10];
    // cppcheck-suppress uninitvar
    f.read(buffer, size);
}

void uninivar_istream_read(std::istream &f)
{
    int size;
    char buffer[10];
    // cppcheck-suppress uninitvar
    f.read(buffer, size);
}

void uninitvar_string_compare(const std::string &teststr, const std::wstring &testwstr)
{
    const char *pStrUninit;
    // cppcheck-suppress uninitvar
    (void)teststr.compare(pStrUninit);

    const wchar_t *pWStrUninit;
    // cppcheck-suppress uninitvar
    (void)testwstr.compare(pWStrUninit);
}

void invalidFunctionArgBool_abs(bool b, double x, double y)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::abs(true); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::abs(b); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)std::abs(x<y); // #5635
}

void ignoredReturnValue_abs(int i)
{
    // cppcheck-suppress ignoredReturnValue
    std::abs(i);
    // cppcheck-suppress ignoredReturnValue
    std::abs(-199);
}

// cppcheck-suppress passedByValue
void ignoredReturnValue_string_compare(std::string teststr, std::wstring testwstr)
{
    // cppcheck-suppress ignoredReturnValue
    teststr.compare("test");
    // cppcheck-suppress ignoredReturnValue
    testwstr.compare(L"wtest");
}

// cppcheck-suppress constParameterReference
void ignoredReturnValue_container_access(std::string& s, std::string_view& sv, std::vector<int>& v)
{
    // cppcheck-suppress ignoredReturnValue
    s.begin();
    // cppcheck-suppress ignoredReturnValue
    v.end();
    // cppcheck-suppress ignoredReturnValue
    sv.front();
    // cppcheck-suppress ignoredReturnValue
    s.at(0);
}

void ignoredReturnValue_locale_global(const std::locale& loc)
{
    // no ignoredReturnValue shall be shown for
    std::locale::global(loc);
}

void ignoredReturnValue_make_pair()
{
    // cppcheck-suppress ignoredReturnValue
    std::make_pair(1, 2);
}

void nullPointer_ifstream_read(std::ifstream &f)
{
    // cppcheck-suppress nullPointer
    f.read(NULL, 10);
}

void nullPointer_istream_read(std::istream &f)
{
    // cppcheck-suppress nullPointer
    f.read(NULL, 10);
}

std::size_t nullPointer_strxfrm(char *dest, const char *src, std::size_t count)
{
    (void)strxfrm(dest, src, count);
    // In case the 3rd argument is 0, the 1st argument is permitted to be a null pointer. (#6306)
    (void)strxfrm(nullptr, src, 0);
    (void)strxfrm(nullptr, src, 1);
    (void)strxfrm(nullptr, src, count);
    // cppcheck-suppress nullPointer
    return strxfrm(dest, nullptr, count);
}

std::size_t nullPointer_wcsxfrm(wchar_t *dest, const wchar_t *src, std::size_t count)
{
    (void)wcsxfrm(dest, src, count);
    // In case the 3rd argument is 0, the 1st argument is permitted to be a null pointer. (#6306)
    (void)wcsxfrm(nullptr, src, 0);
    (void)wcsxfrm(nullptr, src, 1);
    (void)wcsxfrm(nullptr, src, count);
    // cppcheck-suppress nullPointer
    return wcsxfrm(dest, nullptr, count);
}

void nullPointer_asctime(void)
{
    const struct tm *tm = 0;
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)std::asctime(tm);
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)std::asctime(0);
}

void nullPointer_wcsftime(wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr)
{
    // cppcheck-suppress nullPointer
    (void)std::wcsftime(NULL, maxsize, format, timeptr);
    // cppcheck-suppress nullPointer
    (void)std::wcsftime(ptr, maxsize, NULL, timeptr);
    // cppcheck-suppress nullPointer
    (void)std::wcsftime(ptr, maxsize, format, NULL);
    (void)std::wcsftime(ptr, maxsize, format, timeptr);
}

void nullPointer_fegetenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fegetenv(envp);
    // cppcheck-suppress nullPointer
    (void)std::fegetenv(0);
}

void nullPointer_fegetexceptflag(int expects)
{
    fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fegetexceptflag(flagp,expects);
    // cppcheck-suppress nullPointer
    (void)std::fegetexceptflag(0,expects);
}

void nullPointer_feholdexcept(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::feholdexcept(envp);
    // cppcheck-suppress nullPointer
    (void)std::feholdexcept(0);
}

void nullPointer_fesetenv(void)
{
    const fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fesetenv(envp);
    // cppcheck-suppress nullPointer
    (void)std::fesetenv(0);
}

void nullPointer_fesetexceptflag(int expects)
{
    const fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)std::fesetexceptflag(flagp,expects);
    // cppcheck-suppress nullPointer
    (void)std::fesetexceptflag(0,expects);
}

void nullPointer_feupdateenv(void)
{
    const fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)std::feupdateenv(envp);
    // cppcheck-suppress nullPointer
    (void)std::feupdateenv(0);
}

void nullPointer_atexit(void)
{
    // cppcheck-suppress nullPointer
    (void)std::atexit(0);
}

void nullPointer_atof(void)
{
    const char * c = 0;
    // cppcheck-suppress nullPointer
    (void)std::atof(c);
    // cppcheck-suppress nullPointer
    (void)std::atof(0);
}

void nullPointer_memcpy(void *s1, const void *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::memcpy(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)std::memcpy(s1,NULL,n);
    (void)std::memcpy(s1,s2,n);
}

void nullPointer_memmove(void *s1, void *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::memmove(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)std::memmove(s1,NULL,n);
    (void)std::memmove(s1,s2,n);
}

void nullPointer_wmemmove(wchar_t* s1, const wchar_t* s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::wmemmove(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)std::wmemmove(s1,NULL,n);
    (void)std::wmemmove(s1,s2,n);
}

void nullPointer_wmemset(wchar_t* s, wchar_t c, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)std::wmemset(NULL,c,n);
    (void)std::wmemset(s,c,n);
}

///////////////////////////////////////////////////////////////////////
//  <algorithm>
///////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <list>

#define pred    [] (int i) {return i==0;}


void stdalgorithm(const std::list<int> &ints1, const std::list<int> &ints2)
{
    // <!-- InputIterator std::find(InputIterator first, InputIterator last, T val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::find(ints1.begin(), ints2.end(), 123);
    // cppcheck-suppress mismatchingContainers
    if (std::find(ints1.begin(), ints1.end(), 123) == ints2.end()) {}

    // #9455
    std::list<int>::const_iterator uninitItBegin;
    std::list<int>::const_iterator uninitItEnd;
    // cppcheck-suppress uninitvar
    if (std::find(uninitItBegin, uninitItEnd, 123) == uninitItEnd) {}

    // <!-- InputIterator std::find_if(InputIterator first, InputIterator last, UnaryPredicate val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::find_if(ints1.begin(), ints2.end(), pred);
    // cppcheck-suppress mismatchingContainers
    if (std::find_if(ints1.begin(), ints1.end(), pred) == ints2.end()) {}

    // <!-- InputIterator std::find_if_not(InputIterator first, InputIterator last, UnaryPredicate val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::find_if_not(ints1.begin(), ints2.end(), pred);
    // cppcheck-suppress mismatchingContainers
    if (std::find_if_not(ints1.begin(), ints1.end(), pred) == ints2.end()) {}

    // <!-- bool std::all_of(InputIterator first, InputIterator last, UnaryPredicate pred) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::all_of(ints1.begin(), ints2.end(), pred);

    // <!-- bool std::any_of(InputIterator first, InputIterator last, UnaryPredicate pred) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::any_of(ints1.begin(), ints2.end(), pred);

    // <!-- bool std::none_of(InputIterator first, InputIterator last, UnaryPredicate pred) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::none_of(ints1.begin(), ints2.end(), pred);

    // <!-- difference_type std::count(InputIterator first, InputIterator last, T val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::count(ints1.begin(), ints2.end(), 123);

    // <!-- difference_type std::count_if(InputIterator first, InputIterator last, UnaryPredicate val) -->
    // cppcheck-suppress mismatchingContainers
    // cppcheck-suppress ignoredReturnValue
    std::count_if(ints1.begin(), ints2.end(), pred);

    // <!-- Function std::for_each(InputIterator first, InputIterator last, Function func) -->
    // cppcheck-suppress mismatchingContainers
    std::for_each(ints1.begin(), ints2.end(), [](int i) {});
}

void getline()
{
    // #837
    std::ifstream in("test1.txt");

    char cBuf[10];
    // cppcheck-suppress bufferAccessOutOfBounds
    in.getline(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.read(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.readsome(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.get(cBuf, 100);
    // cppcheck-suppress bufferAccessOutOfBounds
    in.get(cBuf, 100, 'a');
    // cppcheck-suppress bufferAccessOutOfBounds
    in.getline(cBuf, 100, 'a');

    in.close();
}

// cppcheck-suppress passedByValue
void stream_write(std::ofstream& s, std::vector<char> v) {
    if (v.empty()) {}
    s.write(v.data(), v.size());
}

void stdstring()
{
    std::string s;
    // cppcheck-suppress ignoredReturnValue
    s.size();

    // valid
    s.assign("a");

#ifdef __cpp_lib_starts_ends_with
    // cppcheck-suppress ignoredReturnValue
    s.starts_with("abc");
#endif
}

void stdvector()
{
    int uninit1, uninit2, uninit3;
    std::vector<int> v;
    // cppcheck-suppress ignoredReturnValue
    v.size();
    // cppcheck-suppress ignoredReturnValue
    v.capacity();
    // cppcheck-suppress uselessCallsEmpty
    // cppcheck-suppress ignoredReturnValue
    v.empty();
    // cppcheck-suppress ignoredReturnValue
    v.max_size();
    // cppcheck-suppress uninitvar
    v.push_back(uninit1);
    // cppcheck-suppress uninitvar
    v.reserve(uninit2);
    // cppcheck-suppress invalidFunctionArg
    v.reserve(-1);
    // no warning is expected for capacity 0 as it simply has no effect
    v.reserve(0);
    // cppcheck-suppress uninitvar
    v.resize(uninit3);
    // cppcheck-suppress invalidFunctionArg
    v.resize(-1);

    v.clear();
    v.shrink_to_fit();

    // no warning is expected for pop_back()
    v.push_back(42);
    v.pop_back();

    v.push_back(42);
    // cppcheck-suppress ignoredReturnValue
    v.back();
    // cppcheck-suppress ignoredReturnValue
    v.front();
}

void stdbind_helper(int a)
{
    printf("%d", a);
}

void stdbind()
{
    using namespace std::placeholders;

    // TODO cppcheck-suppress ignoredReturnValue #9369
    std::bind(stdbind_helper, 1);

    // TODO cppcheck-suppress unreadVariable
    // cppcheck-suppress autoNoType
    auto f1 = std::bind(stdbind_helper, _1);
    // TODO cppcheck-suppress unreadVariable
    // cppcheck-suppress autoNoType
    auto f2 = std::bind(stdbind_helper, 10);
}

int stdexchange() {
    int i;
    // cppcheck-suppress uninitvar
    int j = std::exchange(i, 5);
    return j;
}

class A
{
    std::vector<std::string> m_str;

public:

    A() {}

    // cppcheck-suppress functionConst
    void begin_const_iterator(void)
    {
        for (std::vector<std::string>::const_iterator it = m_str.begin(); it != m_str.end(); ++it) {;}
    }
    // cppcheck-suppress functionConst
    void cbegin_const_iterator(void)
    {
        for (std::vector<std::string>::const_iterator it = m_str.cbegin(); it != m_str.cend(); ++it) {;}
    }
    // cppcheck-suppress functionConst
    void crbegin_const_iterator(void)
    {
        for (std::vector<std::string>::const_reverse_iterator it = m_str.crbegin(); it != m_str.crend(); ++it) {;}
    }
    // cppcheck-suppress functionConst
    void rbegin_const_iterator(void)
    {
        for (std::vector<std::string>::const_reverse_iterator it = m_str.rbegin(); it != m_str.rend(); ++it) {;}
    }
    // cppcheck-suppress functionConst
    void cbegin_auto(void)
    {
        for (auto it = m_str.cbegin(); it != m_str.cend(); ++it) {;}
    }
    void baz_begin_no_const_iterator(void)
    {
        for (std::vector<std::string>::iterator it = m_str.begin(); it != m_str.end(); ++it) {;}
    }
    void rbegin_no_const_iterator(void)
    {
        for (std::vector<std::string>::reverse_iterator it = m_str.rbegin(); it != m_str.rend(); ++it) {;}
    }
};

void addressof(int a)
{
    // cppcheck-suppress ignoredReturnValue
    std::addressof(a);
}

void string_view_unused(std::string_view v)
{
    // cppcheck-suppress ignoredReturnValue
    v.substr(1, 3);
}

// cppcheck-suppress passedByValue
void string_substr(std::string s)
{
    // cppcheck-suppress ignoredReturnValue
    s.substr(1, 3);
}

#ifndef __cpp_lib_span
#warning "This compiler does not support std::span"
#else
void stdspan()
{

    std::vector<int> vec{1,2,3,4};
    std::span spn{vec};
    // cppcheck-suppress unreadVariable
    std::span spn2 = spn;

    //cppcheck-suppress ignoredReturnValue
    spn.begin();
    //cppcheck-suppress ignoredReturnValue
    spn.end();
    //cppcheck-suppress ignoredReturnValue
    spn.rbegin();

    //cppcheck-suppress ignoredReturnValue
    spn.front();
    //cppcheck-suppress ignoredReturnValue
    spn.back();
    //cppcheck-suppress constStatement
    spn[0];
    //cppcheck-suppress ignoredReturnValue
    spn.data();
    //cppcheck-suppress ignoredReturnValue
    spn.size();
    //cppcheck-suppress ignoredReturnValue
    spn.size_bytes();
    //cppcheck-suppress ignoredReturnValue
    spn.empty();
    //cppcheck-suppress ignoredReturnValue
    spn.first(2);
    //cppcheck-suppress ignoredReturnValue
    spn.last(2);
    //cppcheck-suppress ignoredReturnValue
    spn.subspan(1, 2);
    spn.subspan<1>();

    static constexpr std::array<int, 2> arr{1, 2};
    constexpr std::span spn3{arr};
    spn3.first<1>();
    spn3.last<1>();
    spn3.subspan<1, 1>();
}

std::span<const int> returnDanglingLifetime_std_span0() {
    static int a[10]{};
    return a;
}

std::span<const int> returnDanglingLifetime_std_span1() {
    static std::vector<int> v;
    return v;
}
#endif

void beginEnd()
{
    std::vector<int> v;

    //cppcheck-suppress ignoredReturnValue
    std::begin(v);
    //cppcheck-suppress ignoredReturnValue
    std::rbegin(v);
    //cppcheck-suppress ignoredReturnValue
    std::cbegin(v);
    //cppcheck-suppress ignoredReturnValue
    std::crbegin(v);

    //cppcheck-suppress ignoredReturnValue
    std::end(v);
    //cppcheck-suppress ignoredReturnValue
    std::rend(v);
    //cppcheck-suppress ignoredReturnValue
    std::cend(v);
    //cppcheck-suppress ignoredReturnValue
    std::crend(v);

    // TODO cppcheck-suppress constVariable
    int arr[4];

    //cppcheck-suppress ignoredReturnValue
    std::begin(arr);
    //cppcheck-suppress ignoredReturnValue
    std::rbegin(arr);
    //cppcheck-suppress ignoredReturnValue
    std::cbegin(arr);
    //cppcheck-suppress ignoredReturnValue
    std::crbegin(arr);

    //cppcheck-suppress ignoredReturnValue
    std::end(arr);
    //cppcheck-suppress ignoredReturnValue
    std::rend(arr);
    //cppcheck-suppress ignoredReturnValue
    std::cend(arr);
    //cppcheck-suppress ignoredReturnValue
    std::crend(arr);
}

struct S_constParameter_std_begin { // #11617
    int a[2];
};

struct T_constParameter_std_begin {
    std::vector<int> v;
};

void f(S_constParameter_std_begin& s) {
    std::for_each(std::begin(s.a), std::end(s.a), [](int& i) { ++i; });
}

// cppcheck-suppress constParameterReference - FP
void f(T_constParameter_std_begin& t) {
    std::for_each(std::begin(t.v), std::end(t.v), [](int& i) { ++i; });
}

void g_constVariable_std_begin(int* p) { *p = 0; }

int f_constVariable_std_begin() {
    int arr[1];
    g_constVariable_std_begin(std::begin(arr));
    *std::begin(arr) = 1;
    return arr[0];
}

void smartPtr_get()
{
    std::unique_ptr<int> p;
    //cppcheck-suppress ignoredReturnValue
    p.get();
    //cppcheck-suppress nullPointer
    *p = 1;
}

void smartPtr_get2(std::vector<std::unique_ptr<int>>& v)
{
    // cppcheck-suppress autoNoType
    for (auto& u : v) {
        int* p = u.get();
        *p = 0;
    }
}

bool smartPtr_get3(size_t n, size_t i) { // #12748
    std::unique_ptr<int[]> buf = std::make_unique<int[]>(n);
    const int* p = buf.get() + i;
    return p != nullptr;
}

void smartPtr_reset()
{
    std::unique_ptr<int> p(new int());
    p.reset(nullptr);
    //cppcheck-suppress nullPointer
    *p = 1;
}

void smartPtr_release()
{
    std::unique_ptr<int> p{ new int() };
    //cppcheck-suppress ignoredReturnValue
    p.release();
    //cppcheck-suppress nullPointer
    *p = 1;
}

void std_vector_data_arithmetic()
{
    std::vector<char> buf;
    buf.resize(1);
    memcpy(buf.data() + 0, "", 1);
}

void memleak_std_malloc() // #12332
{
    //cppcheck-suppress [unreadVariable, constVariablePointer, unusedAllocatedMemory]
    void* p = std::malloc(1);
    //cppcheck-suppress memleak
}

void memleak_std_realloc(void* block, size_t newsize)
{
    //cppcheck-suppress [unreadVariable, constVariablePointer]
    void* p = std::realloc(block, newsize);
    //cppcheck-suppress memleak
}

void unusedAllocatedMemory_std_free()
{
    // cppcheck-suppress unusedAllocatedMemory
    void* p = std::malloc(1);
    std::free(p);
}

std::string global_scope_std() // #12355
{
    ::std::stringstream ss;
    return ss.str();
}

::std::size_t global_scope_std2() // #12378
{
    std::vector<::std::size_t> v;
    // cppcheck-suppress containerOutOfBounds
    return v.front();
}

void unique_lock_const_ref(std::mutex& m)
{
    std::unique_lock lock(m);
}

void eraseIteratorOutOfBounds_std_deque(std::deque<int>& x) // #8690
{
    // cppcheck-suppress eraseIteratorOutOfBounds
    x.erase(x.end());
}

void assertWithSideEffect_system()
{
    // cppcheck-suppress [assertWithSideEffect,checkLibraryNoReturn] // TODO: #8329
    assert(std::system("abc"));
}

void assertWithSideEffect_std_map_at(const std::map<int, int>& m) // #12695
{
    // cppcheck-suppress checkLibraryNoReturn
    assert(m.at(0));
}

void assertWithSideEffect_std_unique_ptr_get(std::unique_ptr<int>& p)
{
    // cppcheck-suppress checkLibraryNoReturn
    assert(p.get());
}

void assertWithSideEffect_std_begin(const std::vector<std::string>& v) {
    // cppcheck-suppress checkLibraryFunction // TODO
    assert(std::is_sorted(std::begin(v), std::end(v), [](const std::string& a, const std::string& b) {
        return a.size() < b.size();
    })); // cppcheck-suppress checkLibraryNoReturn
}

void assertWithSideEffect_std_prev_next(const std::vector<int>& v, std::vector<int>::const_iterator it) {
    assert(std::prev(it, 1) == v.begin());
    // cppcheck-suppress checkLibraryNoReturn
    assert(std::next(it, 1) == v.end());
}

std::vector<int> containerOutOfBounds_push_back() { // #12775
    std::vector<int> v;
    for (int i = 0; i < 4; ++i) {
        v.push_back(i);
        (void)v[i];
    }
    return v;
}

template<typename T>
void constVariablePointer_push_back(std::vector<T*>& d, const std::vector<T*>& s) {
    for (const auto& e : s) {
        T* newE = new T(*e);
        d.push_back(newE);
    }
}

struct S_constVariablePointer_wstring { // #14575
    std::wstring m;
    const std::wstring& get() const { return m; }
};

S_constVariablePointer_wstring* g_constVariablePointer_wstring();

void h_constVariablePointer_wstring(const wchar_t*);

void f_constVariablePointer_wstring() {
    S_constVariablePointer_wstring* s = g_constVariablePointer_wstring(); // cppcheck-suppress constVariablePointer
    h_constVariablePointer_wstring(s->get().c_str());
}

std::streampos constParameterPointer_istream_tellg(std::istream* p) { // #13801
    return p->tellg();
}

void constParameterPointer_unique_ptr_reset(std::unique_ptr<int>& u, int* p) {
    u.reset(p);
}

bool constParameterPointer_map_contains(const std::map<int*, int>& m, int* p) {
#if __cplusplus >= 202002L
    return m.contains(p);
#else
    return m.count(p) > 0;
#endif
}

// cppcheck-suppress constParameterReference
void constParameterReference_push_back(std::vector<std::string>& v, std::string& s) { // #12661
    v.push_back(s);
}

// cppcheck-suppress constParameterReference
void constParameterReference_assign(std::vector<int>& v, int& r) {
    v.assign(5, r);
}

// cppcheck-suppress constParameterReference
void constParameterReference_insert(std::list<int>& l, int& r) {
    l.insert(l.end(), r);
    l.insert(l.end(), 5, r);
}

const char* variableScope_cstr_dummy(const char* q); // #12812
std::size_t variableScope_cstr(const char* p) {
    std::string s;
    if (!p) {
        s = "abc";
        p = variableScope_cstr_dummy(s.c_str());
    }
    return std::strlen(p);
}

void unusedvar_stringstream(const char* p)
{
    // cppcheck-suppress unreadVariable
    std::istringstream istr(p);
    // cppcheck-suppress unreadVariable
    std::ostringstream ostr(p);
    // cppcheck-suppress unreadVariable
    std::stringstream sstr(p);
}

void unusedvar_stdcomplex()
{
    // cppcheck-suppress unusedVariable
    std::complex<double> z1;
    // cppcheck-suppress unreadVariable
    std::complex<double> z2(0.0, 0.0);
}

int passedByValue_std_array1(std::array<int, 2> a)
{
    return a[0] + a[1];
}

// cppcheck-suppress passedByValue
int passedByValue_std_array2(std::array<int, 200> a)
{
    return a[0] + a[1];
}

int passedByValue_std_bitset1(std::bitset<4> bs) // #12961
{
    return bs.size();
}

// cppcheck-suppress passedByValue
int passedByValue_std_bitset2(std::bitset<256> bs)
{
    return bs.size();
}

struct S_std_as_const { // #12974
    // cppcheck-suppress functionConst
    void f() {
        for (const int i : std::as_const(l)) {}
    }
    std::list<int> l;
};

#if __cpp_lib_format
void unreadVariable_std_format_error(char * c)
{
    // cppcheck-suppress unreadVariable
    std::format_error x(c);
}
#endif

void eraseIteratorOutOfBounds_std_list1()
{
    std::list<int> a;
    std::list<int>::iterator p = a.begin();
    // cppcheck-suppress eraseIteratorOutOfBounds
    a.erase(p);
}
int eraseIteratorOutOfBounds_std_list2()
{
    std::list<int> a;
    std::list<int>::iterator p = a.begin();
    std::list<int>::iterator q = p;
    // cppcheck-suppress eraseIteratorOutOfBounds
    a.erase(q);
    return *q;
}

void containerOutOfBounds_std_string(std::string &var) { // #11403
    std::string s0{"x"};
    // cppcheck-suppress containerOutOfBounds
    var+= s0[2];

    std::string s1{ R"(x)" };
    // cppcheck-suppress containerOutOfBounds
    var+= s1[2];

    std::string s2 = R"--(XYZ)--";
    // cppcheck-suppress containerOutOfBounds
    var+= s2[3];

    std::string s3 = {R"--(XYZ)--"};
    // cppcheck-suppress containerOutOfBounds
    var+= s3[3];

    const char *x = R"--(XYZ)--";
    std::string s4(x);
    // TODO cppcheck-suppress containerOutOfBounds
    var+= s4[3];

    std::string s5{x};
    // TODO cppcheck-suppress containerOutOfBounds
    var+= s5[3];
}

int containerOutOfBounds_std_initializer_list_access(const std::vector<int>& v) {
    // cppcheck-suppress containerOutOfBounds
    return v[2];
}

int containerOutOfBounds_std_initializer_list() { // #14340
    std::initializer_list<int> x{ 1, 2 };
    // cppcheck-suppress derefInvalidIterator
    int i = *x.end();
    return i + containerOutOfBounds_std_initializer_list_access(x);
}
