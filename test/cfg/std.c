
// Test library configuration for std.cfg
//
// Usage:
// $ cppcheck --check-library --library=std --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/std.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file [valueFlowBailout,purgedConfiguration]

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h> // frexp
#include <wchar.h>
#if  defined(__STD_UTF_16__) || defined(__STD_UTF_32__)
#include <uchar.h>
#endif
#include <ctype.h>
#include <wctype.h>
#include <fenv.h>
#include <setjmp.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include <stdbool.h>
#ifndef __STDC_NO_THREADS__
#include <threads.h>
#endif
#include <inttypes.h>
#include <float.h>
#include <stdarg.h>
#include <assert.h>
#include <alloca.h>
#include <locale.h>
#include <signal.h>
#include <complex.h>
#include <math.h>
#include <stddef.h>

void test_int32_min() {
    // cppcheck-suppress shiftNegativeLHS
    // cppcheck-suppress shiftTooManyBits ; tests that INT32_MIN is a 32-bit expression (not 64-bit expression)
    (void)(INT32_MIN >> 40);
}

void test_int64_min() {
    // cppcheck-suppress compareValueOutOfTypeRangeError ; tests that INT64_MIN is a signed expression
    if (INT64_MIN < 0) {}
}

size_t invalidFunctionArgStr_wcslen(void)
{
    const wchar_t terminated0[] = L"ABCDEF49620910";
    const wchar_t terminated1[3] = { L'a', L'b', L'\0' };
    const wchar_t notTerminated[3] = { L'a', L'b', L'c' };
    // cppcheck-suppress invalidFunctionArgStr
    (void) wcslen(notTerminated);
    (void) wcslen(terminated0);
    return wcslen(terminated1);
}

int invalidFunctionArgStr_strcpn(void)
{
    const char str1[] = "ABCDEF49620910";
    const char str2[] = "42";
    const char pattern[3] = { -42, -43, -44 };
    // cppcheck-suppress invalidFunctionArgStr
    (void) strcspn(str1, pattern);
    return strcspn(str1, str2);
}

void invalidFunctionArgStr_strncat(void)
{
    char str1[20];
    strcpy (str1,"test");
    const char src = '/';
    // No warning is expected for
    strncat (str1, &src, 1);
    puts (str1);
}

char * invalidFunctionArgStr_strpbrk( const char *p )
{
    const char search[] = { -42, -43, -44 };
    const char pattern[3] = { -42, -43, -44 };
    (void) strpbrk( "abc42", "42" );
    // cppcheck-suppress invalidFunctionArgStr
    (void) strpbrk( search, "42" );
    // cppcheck-suppress invalidFunctionArgStr
    (void) strpbrk( search, pattern );
    // cppcheck-suppress invalidFunctionArgStr
    return strpbrk( p, pattern );
}

int invalidFunctionArgStr_strncmp( const char *p )
{
    const char string[] = "foo";
    char other[5] = { 0 };
    memcpy(other, "foo", 4);
    if (strncmp(other, string, 5) != 0) {}

    // No warning is expected for:
    const char emdash[3] = { -42, -43, -44 };
    return strncmp( p, emdash, 3 );
}

float invalidFunctionArg_float_remquo (float x, float y, int* quo )
{
    // cppcheck-suppress invalidFunctionArg
    (void) remquo(x,0.0f,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) remquof(x,0.0f,quo);
    return remquo(x,y,quo);
}

double invalidFunctionArg_double_remquo (double x, double y, int* quo )
{
    // cppcheck-suppress invalidFunctionArg
    (void) remquo(x,0.0,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) remquo(x,0.0f,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) remquo(x,0.0L,quo);
    return remquo(x,y,quo);
}

double invalidFunctionArg_long_double_remquo (long double x, long double y, int* quo )
{
    // cppcheck-suppress invalidFunctionArg
    (void) remquo(x,0.0L,quo);
    // cppcheck-suppress invalidFunctionArg
    (void) remquol(x,0.0L,quo);
    return remquo(x,y,quo);
}

void invalidFunctionArg_remainderl(long double f1, long double f2)
{
    // cppcheck-suppress invalidFunctionArg
    (void)remainderl(f1,0.0);
    // cppcheck-suppress invalidFunctionArg
    (void)remainderl(f1,0.0L);
    (void)remainderl(f1,f2);
}

void invalidFunctionArg_remainder(double f1, double f2)
{
    // cppcheck-suppress invalidFunctionArg
    (void)remainder(f1,0.0);
    (void)remainder(f1,f2);
}

void invalidFunctionArg_remainderf(float f1, float f2)
{
    // cppcheck-suppress invalidFunctionArg
    (void)remainderf(f1,0.0);
    // cppcheck-suppress invalidFunctionArg
    (void)remainderf(f1,0.0f);
    (void)remainderf(f1,f2);
}

int qsort_cmpfunc (const void * a, const void * b) {
    return (*(int*)a - *(int*)b);
}
void nullPointer_qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    // cppcheck-suppress nullPointer
    qsort(NULL, n, size, qsort_cmpfunc);
    // cppcheck-suppress nullPointer
    qsort(base, n, size, NULL);
    qsort(base, n, size, qsort_cmpfunc);
}

// As with all bounds-checked functions, localtime_s is only guaranteed to be available if __STDC_LIB_EXT1__ is defined by the implementation and if the user defines __STDC_WANT_LIB_EXT1__ to the integer constant 1 before including time.h.
#ifdef __STDC_LIB_EXT1__
void uninitvar_localtime_s(const time_t *restrict time, struct tm *restrict result)
{
    const time_t *restrict Time;
    // cppcheck-suppress uninitvar
    (void)localtime_s(Time, result);
    (void)localtime_s(time, result);
}

void nullPointer_localtime_s(const time_t *restrict time, struct tm *restrict result)
{
    // cppcheck-suppress nullPointer
    (void)localtime_s(NULL, result);
    // cppcheck-suppress nullPointer
    (void)localtime_s(time, NULL);
    (void)localtime_s(time, result);
}
#endif // __STDC_LIB_EXT1__

size_t bufferAccessOutOfBounds_wcsrtombs(char * dest, const wchar_t ** src, size_t len, mbstate_t * ps)
{
    char buf[42];
    (void)wcsrtombs(buf,src,42,ps);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)wcsrtombs(buf,src,43,ps);
    return wcsrtombs(dest,src,len,ps);
}

void bufferAccessOutOfBounds(void)
{
    char a[5];
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    fgets(a,5,stdin);
    // cppcheck-suppress bufferAccessOutOfBounds
    fgets(a,6,stdin);
    sprintf(a, "ab%s", "cd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    sprintf(a, "ab%s", "cde");
    // TODO cppcheck-suppress redundantCopy
    snprintf(a, 5, "abcde%i", 1);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    snprintf(a, 6, "abcde%i", 1);
    // TODO cppcheck-suppress redundantCopy
    strcpy(a,"abcd");
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    strcpy(a, "abcde");
#ifdef __STDC_LIB_EXT1__
    // cppcheck-suppress bufferAccessOutOfBounds
    strcpy_s(a, 10, "abcdefghij");
#endif
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress terminateStrncpy
    strncpy(a,"abcde",5);
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"abcde",6);
    // cppcheck-suppress bufferAccessOutOfBounds
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"a",6);
    // TODO cppcheck-suppress redundantCopy
    strncpy(a,"abcdefgh",4);
#ifdef __STDC_LIB_EXT1__
    // valid call
    strncpy_s(a,5,"abcd",5);
    // string will be truncated, error is returned, but no buffer overflow
    strncpy_s(a,5,"abcde",6);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    strncpy_s(a,5,"a",6);
    strncpy_s(a,5,"abcdefgh",4);
    // valid call
    strncat_s(a,5,"1",2);
    // cppcheck-suppress bufferAccessOutOfBounds
    strncat_s(a,10,"1",2);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    strncat_s(a,5,"1",5);
#endif
    fread(a,1,5,stdin);
    // cppcheck-suppress bufferAccessOutOfBounds
    fread(a,1,6,stdin);
    fwrite(a,1,5,stdout);
    // cppcheck-suppress bufferAccessOutOfBounds
    fread(a,1,6,stdout);

    char * pAlloc1 = aligned_alloc(8, 16);
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc1, 0, 16);
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc1, 0, 17);
    free(pAlloc1);
}

wchar_t* nullPointer_fgetws(wchar_t* buffer, int n, FILE* stream)
{
    // cppcheck-suppress nullPointer
    (void)fgetws(NULL,n,stream);
    // cppcheck-suppress nullPointer
    (void)fgetws(buffer,n,NULL);
    // No warning is expected
    return fgetws(buffer, n, stream);
}

char* nullPointer_fgets(char *buffer, int n, FILE *stream)
{
    // cppcheck-suppress nullPointer
    (void)fgets(NULL,n,stream);
    // cppcheck-suppress nullPointer
    (void)fgets(buffer,n,NULL);
    // No warning is expected
    return fgets(buffer, n, stream);
}

void memleak_aligned_alloc(void)
{
    // cppcheck-suppress [unusedAllocatedMemory, unreadVariable, constVariablePointer]
    char * alignedBuf = aligned_alloc(8, 16);
    // cppcheck-suppress memleak
}

void pointerLessThanZero_aligned_alloc(void)
{
    char * alignedBuf = aligned_alloc(8, 16);
    // cppcheck-suppress pointerLessThanZero
    if (alignedBuf < 0) return;
    free(alignedBuf);

    // no warning is expected for
    alignedBuf = aligned_alloc(8, 16);
    if (alignedBuf == 0) return;
    free(alignedBuf);

    // no warning is expected for
    alignedBuf = aligned_alloc(8, 16);
    if (alignedBuf) free(alignedBuf);
}

void unusedRetVal_aligned_alloc(void)
{
    // cppcheck-suppress leakReturnValNotUsed
    aligned_alloc(8, 16);
}

void uninitvar_aligned_alloc(size_t alignment, size_t size)
{
    size_t uninitVar1, uninitVar2, uninitVar3;
    // cppcheck-suppress uninitvar
    free(aligned_alloc(uninitVar1, size));
    // cppcheck-suppress uninitvar
    free(aligned_alloc(alignment, uninitVar2));
    // cppcheck-suppress uninitvar
    free(aligned_alloc(uninitVar3, uninitVar3));
    // no warning is expected
    free(aligned_alloc(alignment, size));
}

void bufferAccessOutOfBounds_libraryDirectionConfiguration(void)
{
    // This tests whether the argument to isdigit() is configured with direction "in". This allows
    // Cppcheck to report the error without marking it as inconclusive.
    char arr[10];
    char c = 'A';
    (void)isdigit(c);
    // cppcheck-suppress arrayIndexOutOfBounds
    // cppcheck-suppress unreadVariable
    arr[c] = 'x';
}

void internalError_libraryDirectionConfiguration(char* str) { // #12824
    const char* s = str;
    char* end = str;
    if (1) {
        // cppcheck-suppress unreadVariable
        unsigned long val = strtoul(&s[1], &end, 10);
    }
}

void arrayIndexOutOfBounds()
{
    char * pAlloc1 = aligned_alloc(8, 16);
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc1[15] = '\0';
    // cppcheck-suppress arrayIndexOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc1[16] = '1';
    free(pAlloc1);

    char * pAlloc2 = malloc(9);
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc2[8] = 'a';
    // cppcheck-suppress arrayIndexOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc2[9] = 'a';

    // #1379
    // cppcheck-suppress memleakOnRealloc
    pAlloc2 = realloc(pAlloc2, 8);
    pAlloc2[7] = 'b';
    // cppcheck-suppress arrayIndexOutOfBounds
    pAlloc2[8] = 0;
    // cppcheck-suppress memleakOnRealloc
    pAlloc2 = realloc(pAlloc2, 20);
    pAlloc2[19] = 'b';
    // cppcheck-suppress arrayIndexOutOfBounds
    pAlloc2[20] = 0;
    free(pAlloc2);

    char * pAlloc3 = calloc(2,3);
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc3[5] = 'a';
    // cppcheck-suppress arrayIndexOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc3[6] = 1;
    free(pAlloc3);
}

void resourceLeak_tmpfile(void)
{
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    FILE * fp = tmpfile();
    // cppcheck-suppress resourceLeak
}

// memory leak

void ignoreleak(void)
{
    char *p = (char *)malloc(10);
    memset(&(p[0]), 0, 10);
    // cppcheck-suppress memleak
}

// null pointer

void nullpointer(int value)
{
    int res = 0;
    FILE *fp;
    wchar_t *pWcsUninit;

#ifndef __CYGWIN__
    // cppcheck-suppress nullPointer
    clearerr(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    feof(0);
#endif
    // cppcheck-suppress nullPointer
    (void)fgetc(0);
    // cppcheck-suppress nullPointer
    fclose(0);
#ifndef __CYGWIN__
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    ferror(0);
#endif
    // cppcheck-suppress nullPointer
    (void)ftell(0);
    // cppcheck-suppress nullPointer
    puts(0);
    // cppcheck-suppress nullPointer
    fp=fopen(0,0);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
    fp = 0;
    // No FP
    fflush(0); // If stream is a null pointer, all streams are flushed.
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    fp = freopen(0,"abc",stdin);
    fclose(fp);
    fp = NULL;
    // cppcheck-suppress nullPointer
    fputc(0,0);
    // cppcheck-suppress nullPointer
    fputs(0,0);
    // cppcheck-suppress nullPointer
    fgetpos(0,0);
    // cppcheck-suppress nullPointer
    frexp(1.0,0);
    // cppcheck-suppress nullPointer
    fsetpos(0,0);
#ifdef _WIN32 // not available on non-Windows compilers
    // cppcheck-suppress nullPointer
    itoa(123,0,10);
#endif
    putchar(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strchr(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcschr(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strlen(0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcslen(0);
    // cppcheck-suppress nullPointer
    strcpy(0,0);
    // cppcheck-suppress nullPointer
    wcscpy(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcsspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcscspn(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcoll(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcscoll(0,0);
    // cppcheck-suppress nullPointer
    strcat(0,0);
    // cppcheck-suppress nullPointer
    wcscat(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcmp(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcscmp(0,0);
#ifdef __STDC_LIB_EXT1__
    // cppcheck-suppress nullPointer
    strcpy_s(0,1,1);
    // cppcheck-suppress nullPointer
    strcpy_s(1,1,0);
#endif
    // cppcheck-suppress nullPointer
    strncpy(0,0,1);
#ifdef __STDC_LIB_EXT1__
    // cppcheck-suppress nullPointer
    strncpy_s(0,1,1,1);
    // cppcheck-suppress nullPointer
    strncpy_s(1,1,0,1);
#endif
    // cppcheck-suppress nullPointer
    wcsncpy(0,0,1);
    // cppcheck-suppress nullPointer
    strncat(0,0,1);
#ifdef __STDC_LIB_EXT1__
    // cppcheck-suppress nullPointer
    strncat_s(0,1,1,1);
    // cppcheck-suppress nullPointer
    strncat_s(1,1,0,1);
#endif
    // cppcheck-suppress nullPointer
    wcsncat(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strncmp(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcsncmp(0,0,1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strstr(0,0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    wcsstr(0,0);
    // cppcheck-suppress nullPointer
    strtoul(0,0,0);
    // cppcheck-suppress nullPointer
    wcstoul(0,0,0);
    // cppcheck-suppress nullPointer
    strtoull(0,0,0);
    // cppcheck-suppress nullPointer
    wcstoull(0,0,0);
    // cppcheck-suppress nullPointer
    strtol(0,0,0);
    // cppcheck-suppress nullPointer
    wcstol(0,0,0);

    // #6100 False positive nullPointer - calling mbstowcs(NULL,)
    res += mbstowcs(0,"",0);
    res += wcstombs(0,L"",0);

    strtok(NULL,"xyz");
    wcstok(NULL,L"xyz",&pWcsUninit);

    strxfrm(0,"foo",0);
    // TODO: error message (#6306 and http://trac.cppcheck.net/changeset/d11eb4931aea51cf2cb74faccdcd2a3289b818d6/)
    strxfrm(0,"foo",42);
    wcsxfrm(0,L"foo",0);
    // TODO: error message when arg1==NULL and arg3!=0 #6306: https://trac.cppcheck.net/ticket/6306#comment:2
    wcsxfrm(0,L"foo",42);

    snprintf(NULL, 0, "someformatstring"); // legal
    // cppcheck-suppress nullPointer
    snprintf(NULL, 42, "someformatstring"); // not legal

    scanf("%i", &res);
    // cppcheck-suppress nullPointer
    scanf("%i", NULL);
    wscanf(L"%i", &res);
    // cppcheck-suppress nullPointer
    wscanf(L"%i", NULL);
}

void nullPointer_wcsftime(wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr)
{
    // cppcheck-suppress nullPointer
    (void)wcsftime(NULL, maxsize, format, timeptr);
    // cppcheck-suppress nullPointer
    (void)wcsftime(ptr, maxsize, NULL, timeptr);
    // cppcheck-suppress nullPointer
    (void)wcsftime(ptr, maxsize, format, NULL);
    (void)wcsftime(ptr, maxsize, format, timeptr);
}

void bufferAccessOutOfBounds_wcsftime(wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr)
{
    wchar_t buf[42];
    (void)wcsftime(buf, 42, format, timeptr);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    (void)wcsftime(buf, 43, format, timeptr);
    (void)wcsftime(ptr, maxsize, format, timeptr);
}

void bufferAccessOutOfBounds_wcsncpy()
{
    wchar_t s[16];
    wcsncpy(s, L"abc", 16);
    // cppcheck-suppress bufferAccessOutOfBounds
    wcsncpy(s, L"abc", 17);
}

int nullPointer_wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void) wcsncmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void) wcsncmp(s1,NULL,n);
    return wcsncmp(s1,s2,n);
}

wchar_t* nullPointer_wcsncpy(wchar_t *s, const wchar_t *cs, size_t n)
{
    // cppcheck-suppress nullPointer
    (void) wcsncpy(NULL,cs,n);
    // cppcheck-suppress nullPointer
    (void) wcsncpy(s,NULL,n);
    return wcsncpy(s,cs,n);
}

size_t nullPointer_strlen(const char *s)
{
    // cppcheck-suppress nullPointer
    (void) strlen(NULL);
    return strlen(s);
}

void nullpointerMemchr1(char *p, const char *s)
{
    p = memchr(s, 'p', strlen(s));
    (void)p;
}

void nullpointerMemchr2(char *p, const char *s)
{
    p = memchr(s, 0, strlen(s));
    (void)p;
}

void nullPointer_memchr(char *p)
{
    const char *s = 0;
    // cppcheck-suppress nullPointer
    p = memchr(s, 0, strlen(s));
    (void)p;
}

void nullPointer_vsnprintf(const char * format, ...)
{
    va_list args;
    // valid
    char buffer[256];
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    printf("%s", buffer);
    va_end(args);
    // valid
    va_start(args, format);
    vsnprintf(NULL, 0, format, args);
    va_end(args);
    // invalid
    va_start(args, format);
    // TODO #9410 cppcheck-suppress nullPointer
    vsnprintf(NULL, 10, format, args);
    va_end(args);
}

// uninit pointers

void uninitvar_abs(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)abs(i);
}

void uninitvar_clearerr(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    clearerr(fp);
}

void uninitvar_fclose(void)
{
    // cppcheck-suppress unassignedVariable
    FILE *fp;
    // cppcheck-suppress uninitvar
    fclose(fp);
}

void uninitvar_fopen(void)
{
    const char *filename, *mode;
    FILE *fp;
    // cppcheck-suppress uninitvar
    fp = fopen(filename, "rt");
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
    // cppcheck-suppress uninitvar
    fp = fopen("filename.txt", mode);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
}

void uninitvar_feof(void)
{
    FILE *fp1, *fp2;
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    feof(fp1);

    // cppcheck-suppress uninitvar
    (void)feof(fp2);
}

void uninitvar_ferror(void)
{
    FILE *fp1, *fp2;
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress uninitvar
    ferror(fp1);

    // cppcheck-suppress uninitvar
    (void)ferror(fp2);
}

void uninitvar_fflush(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    fflush(fp);
}

void uninitvar_fgetc(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    (void)fgetc(fp);
}

void uninitvar_fgetpos(void)
{
    FILE *fp;
    fpos_t pos;
    fpos_t *ppos;
    // cppcheck-suppress uninitvar
    fgetpos(fp,&pos);

    fp = fopen("filename","rt");
    // cppcheck-suppress uninitvar
    // cppcheck-suppress nullPointerOutOfResources
    fgetpos(fp,ppos);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
}

void uninitvar_fsetpos(void)
{
    FILE *fp;
    fpos_t pos;
    const fpos_t *ppos;
    // cppcheck-suppress uninitvar
    fsetpos(fp,&pos);

    fp = fopen("filename","rt");
    // cppcheck-suppress uninitvar
    // cppcheck-suppress nullPointerOutOfResources
    fsetpos(fp,ppos);
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fp);
}

void uninitvar_fgets(void)
{
    FILE *fp;
    char buf[10];
    char *str;
    int n;

    // cppcheck-suppress valueFlowBailoutIncompleteVar
    fgets(buf,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(str,10,stdin);

    // cppcheck-suppress uninitvar
    fgets(buf,10,fp);

    // cppcheck-suppress uninitvar
    fgets(buf,n,stdin);
}

void uninitvar_fputc(void)
{
    int i;
    FILE *fp;

    // cppcheck-suppress valueFlowBailoutIncompleteVar
    fputc('a', stdout);

    // cppcheck-suppress uninitvar
    fputc(i, stdout);

    // cppcheck-suppress uninitvar
    fputc('a', fp);
}

void uninitvar_fputs(void)
{
    const char *s;
    FILE *fp;

    // cppcheck-suppress valueFlowBailoutIncompleteVar
    fputs("a", stdout);

    // cppcheck-suppress uninitvar
    fputs(s, stdout);

    // cppcheck-suppress uninitvar
    fputs("a", fp);
}

void uninitvar_ftell(void)
{
    FILE *fp;
    // cppcheck-suppress uninitvar
    (void)ftell(fp);
}

void uninitvar_puts(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    puts(s);
}

void uninitvar_putchar(void)
{
    char c;
    // cppcheck-suppress uninitvar
    putchar(c);
}

void uninitvar_cproj(void) // #6939
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)cprojf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cproj(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cprojl(ldc);
}

void uninitvar_creal(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)crealf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)creal(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)creall(ldc);
}

void uninitvar_acos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)acosf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)acos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)acosl(ld);
}

void uninitvar_acosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)acoshf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)acosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)acoshl(ld);
}

void invalidFunctionArg_acosh(void)
{
    float f = .999f;
    // cppcheck-suppress invalidFunctionArg
    (void)acoshf(f);
    f = 1.0f;
    (void)acoshf(f);

    double d = .999;
    // cppcheck-suppress invalidFunctionArg
    (void)acosh(d);
    d = 1.0;
    (void)acosh(d);

    long double ld = .999L;
    // cppcheck-suppress invalidFunctionArg
    (void)acoshl(ld);
    ld = 1.0;
    (void)acoshl(ld);
}

void invalidFunctionArg_atanh(void)
{
    float f = 1.00001f;
    // cppcheck-suppress invalidFunctionArg
    (void)atanhf(f);
    f = 1.0f;
    (void)atanhf(f);
    f = -1.0f;
    (void)atanhf(f);
    f = -1.00001f;
    // cppcheck-suppress invalidFunctionArg
    (void)atanhf(f);

    double d = 1.00001;
    // cppcheck-suppress invalidFunctionArg
    (void)atanh(d);
    d = 1.0;
    (void)atanh(d);
    d = -1.0;
    (void)atanh(d);
    d = -1.00001;
    // cppcheck-suppress invalidFunctionArg
    (void)atanh(d);

    long double ld = 1.00001L;
    // cppcheck-suppress invalidFunctionArg
    (void)atanhl(ld);
    ld = 1.0L;
    (void)atanhl(ld);
    ld = -1.0L;
    (void)atanhl(ld);
    ld = -1.00001L;
    // cppcheck-suppress invalidFunctionArg
    (void)atanhl(ld);
}

void uninitvar_asctime(void)
{
    const struct tm *tm;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress asctimeCalled
    (void)asctime(tm);
}

#ifdef __STDC_LIB_EXT1__
void uninitvar_asctime_s(void)
{
    const struct tm *tm;
    char buf[26];
    // cppcheck-suppress uninitvar
    // cppcheck-suppress asctime_sCalled
    asctime_s(buf, sizeof(buf), tm);
}
#endif

void uninitvar_assert(void)
{
    int i;
    // cppcheck-suppress checkLibraryNoReturn
    // cppcheck-suppress uninitvar
    assert(i);
}

void uninitvar_sqrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)sqrtf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)sqrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)sqrtl(ld);
}

void uninitvar_csqrt(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)csqrtf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)csqrt(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)csqrtl(ldc);
}

void uninitvar_sinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)sinhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)sinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)sinhl(ld);
}

void uninitvar_sin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)sinf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)sin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)sinl(ld);
}

void uninitvar_csin(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)csinf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)csin(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)csinl(ldc);
}

void uninitvar_csinh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)csinhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)csinh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)csinhl(ldc);
}

void uninitvar_asin(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)asinf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)asin(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)asinl(ld);
}

void uninitvar_casin(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)casinf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)casin(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)casinl(ldc);
}

void uninitvar_asinh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)asinhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)asinh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)asinhl(ld);
}

void uninitvar_casinh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)casinhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)casinh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)casinhl(ldc);
}

void uninitvar_wcsftime(wchar_t* ptr)
{
    size_t maxsize;
    const wchar_t* format;
    const struct tm* timeptr;
    // cppcheck-suppress uninitvar
    (void)wcsftime(ptr, maxsize, format, timeptr);
}

void uninitvar_tan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)tanf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)tan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)tanl(ld);
}

void uninitvar_ctan(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ctanf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ctan(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ctanl(ldc);
}

void uninitvar_tanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)tanhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)tanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)tanhl(ld);
}

void uninitvar_ctanh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ctanhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ctanh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ctanhl(ldc);
}

void uninitvar_feclearexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)feclearexcept(i);
}

void uninitvar_fegetexceptflag(fexcept_t* flagp)
{
    int excepts;
    // cppcheck-suppress uninitvar
    (void)fegetexceptflag(flagp, excepts);
}

void uninitvar_feraiseexcept(void)
{
    int excepts;
    // cppcheck-suppress uninitvar
    (void)feraiseexcept(excepts);
}

void uninitvar_fesetenv(void)
{
    const fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)fesetenv(envp);
}

void uninitvar_fesetround(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)fesetround(i);
}

void uninitvar_fetestexcept(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)fetestexcept(i);
}

void uninitvar_feupdateenv(void)
{
    const fenv_t* envp;
    // cppcheck-suppress uninitvar
    (void)feupdateenv(envp);
}

void uninitvar_atan(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)atanf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)atan(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)atanl(ld);
}

void uninitvar_catan(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)catanf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)catan(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)catanl(ldc);
}

void uninitvar_tgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)tgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)tgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)tgammal(ld);
}

void uninitvar_trunc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)truncf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)trunc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)truncl(ld);
}

void uninitvar_atanh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)atanhf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)atanh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)atanhl(ld);
}

void uninitvar_catanh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)catanhf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)catanh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)catanhl(ldc);
}

void uninitvar_atan2(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)atan2f(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)atan2(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)atan2l(ld1,ld2);
}

void uninitvar_atof(void)
{
    const char * c;
    // cppcheck-suppress uninitvar
    (void)atof(c);
}

void uninitvar_atol(void)
{
    const char * c1, *c2, *c3;
    // cppcheck-suppress uninitvar
    (void)atoi(c1);

    // cppcheck-suppress uninitvar
    (void)atol(c2);

    // cppcheck-suppress uninitvar
    (void)atoll(c3);
}

void uninitvar_calloc(void)
{
    size_t nitems;
    size_t size;
    // cppcheck-suppress unusedAllocatedMemory
    // cppcheck-suppress uninitvar
    int * p = (int*) calloc(nitems, size);
    free(p);
}

void uninitvar_ceil(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)ceilf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)ceil(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)ceill(ld);
}

void uninitvar_copysign(void)
{
    float f1, f2;
    // cppcheck-suppress uninitvar
    (void)copysignf(f1, f2);

    double d1, d2;
    // cppcheck-suppress uninitvar
    (void)copysign(d1, d2);

    long double ld1, ld2;
    // cppcheck-suppress uninitvar
    (void)copysignl(ld1, ld2);
}

void uninitvar_cbrt(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)cbrtf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)cbrt(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)cbrtl(ld);
}

void uninitvar_cos(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)cosf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)cos(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)cosl(ld);
}

void uninitvar_ccos(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ccosf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ccos(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ccosl(ldc);
}

void uninitvar_cosh(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)coshf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)cosh(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)coshl(ld);
}

void uninitvar_ccosh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)ccoshf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)ccosh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)ccoshl(ldc);
}

void uninitvar_ctime(void)
{
    const time_t *tp;
    // cppcheck-suppress uninitvar
    (void)ctime(tp);
}

void uninitvar_difftime(void)
{
    time_t t1,t2;
    // cppcheck-suppress uninitvar
    (void)difftime(t1, t2);
}

void uninitvar_div(void)
{
    int num;
    int denom;
    // cppcheck-suppress uninitvar
    (void)div(num,denom);
}

void uninitvar_exit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    exit(i);
}

void uninitvar_erf(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)erff(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)erf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)erfl(ld);
}

void uninitvar_erfc(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)erfcf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)erfc(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)erfcl(ld);
}

void uninitvar_carg(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cargf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)carg(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cargl(ldc);
}

void uninitvar_exp(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)expf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)exp(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)expl(ld);
}

void uninitvar_cexp(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cexpf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cexp(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cexpl(ldc);
}

void uninitvar_cimag(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cimagf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cimag(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cimagl(ldc);
}

void uninitvar_exp2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)exp2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)exp2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)exp2l(ld);
}

void uninitvar_expm1(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)expm1f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)expm1(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)expm1l(ld);
}

void uninitvar_fabs(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)fabsf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)fabs(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)fabsl(ld);
}

void uninitvar_fdim(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fdimf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fdim(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fdiml(ld1,ld2);
}

void uninitvar_fgetwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fgetwc(stream);
}

void uninitvar_floor(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)floorf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)floor(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)floorl(ld);
}

void uninitvar_fma(void)
{
    float f1,f2,f3;
    // cppcheck-suppress uninitvar
    (void)fmaf(f1,f2,f3);

    double d1,d2,d3;
    // cppcheck-suppress uninitvar
    (void)fma(d1,d2,d3);

    long double ld1,ld2,ld3;
    // cppcheck-suppress uninitvar
    (void)fmal(ld1,ld2,ld3);
}

void uninitvar_fmax(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fmaxf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fmax(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fmaxl(ld1,ld2);
}

void uninitvar_fmin(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fminf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fmin(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fminl(ld1,ld2);
}

void uninitvar_fmod(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)fmodf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)fmod(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)fmodl(ld1,ld2);
}

void nullPointer_fprintf(FILE *Stream, const char *Format, int Argument)
{
    // cppcheck-suppress nullPointer
    (void)fprintf(Stream, NULL, Argument);
    // no warning is expected
    (void)fprintf(Stream, Format, Argument);
}

void uninitvar_fprintf(FILE *Stream, const char *Format, int Argument)
{
    FILE *stream1, *stream2;
    const char *format1, *format2;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)fprintf(stream1, format1, argument1);
    // cppcheck-suppress uninitvar
    (void)fprintf(stream2, Format, Argument);
    // cppcheck-suppress uninitvar
    (void)fprintf(Stream, format2, Argument);
    // cppcheck-suppress uninitvar
    (void)fprintf(Stream, Format, argument2);

    // no warning is expected
    (void)fprintf(Stream, Format, Argument);
}

void nullPointer_vfprintf(FILE *Stream, const char *Format, va_list Arg)
{
    // cppcheck-suppress nullPointer
    (void)vfprintf(Stream, NULL, Arg);
    (void)vfprintf(Stream, Format, Arg);
}

void uninitvar_vfprintf(FILE *Stream, const char *Format, va_list Arg)
{
    FILE *stream1, *stream2;
    const char *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfprintf(stream1, format1, arg);
    // cppcheck-suppress uninitvar
    (void)vfprintf(stream2, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)vfprintf(Stream, format2, Arg);

    // no warning is expected
    (void)vfprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vfprintf(Stream, Format, arg);
}

void nullPointer_vfwprintf(FILE *Stream, const wchar_t *Format, va_list Arg)
{
    // cppcheck-suppress nullPointer
    (void)vfwprintf(Stream, NULL, Arg);
    (void)vfwprintf(Stream, Format, Arg);
}

void uninitvar_vfwprintf(FILE *Stream, const wchar_t *Format, va_list Arg)
{
    FILE *stream1, *stream2;
    const wchar_t *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfwprintf(stream1, format1, arg);
    // cppcheck-suppress uninitvar
    (void)vfwprintf(stream2, Format, Arg);
    // cppcheck-suppress uninitvar
    (void)vfwprintf(Stream, format2, Arg);

    // no warning is expected
    (void)vfwprintf(Stream, Format, Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vfwprintf(Stream, Format, arg);
}

void uninitvar_fputwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fputwc(c,stream);
}

void uninitvar_fputws(void)
{
    const wchar_t *string;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fputws(string,stream);
}

void uninitvar_fread(void)
{
    void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fread(ptr,size,nobj,stream);
}

void uninitvar_free(void)
{
    // cppcheck-suppress unassignedVariable
    const void *block;
    // cppcheck-suppress uninitvar
    free(block);
}

void uninitvar_freopen(void)
{
    const char *filename;
    const char *mode;
    FILE *stream;
    // cppcheck-suppress uninitvar
    FILE * p = freopen(filename,mode,stream);
    fclose(p);
}

void uninitvar_frexp(void)
{
    float f1;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)frexpf(f1,i1);

    double d1;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)frexp(d1,i2);

    long double ld1;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)frexpl(ld1,i3);
}

void uninitvar_hypot(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)hypotf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)hypot(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)hypotl(ld1,ld2);
}

void uninitvar_fscanf(void)
{
    FILE *stream;
    const char *format;
    int i;
    // cppcheck-suppress uninitvar
    (void)fscanf(stream,format,i);
}

void uninitvar_vfscanf(void)
{
    FILE *stream;
    const char * format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfscanf(stream,format,arg);
}

void uninitvar_vfwscanf(void)
{
    FILE *stream;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vfwscanf(stream,format,arg);
}

void uninitvar_fseek(void)
{
    FILE* stream;
    long int offset;
    int origin;
    // cppcheck-suppress uninitvar
    (void)fseek(stream,offset,origin);
}

void uninitvar_fgetws(void)
{
    wchar_t *buffer;
    int n;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fgetws(buffer,n,stream);
}

void uninitvar_fwide(void)
{
    FILE *stream;
    int mode;
    // cppcheck-suppress uninitvar
    (void)fwide(stream,mode);
}

void uninitvar_fwrite(void)
{
    const void *ptr;
    size_t size;
    size_t nobj;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)fwrite(ptr,size,nobj,stream);
}

void uninitvar_mblen(void)
{
    const char *string;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)mblen(string,size);
}

void uninitvar_mbtowc(void)
{
    wchar_t* pwc;
    const char* pmb;
    size_t max;
    // cppcheck-suppress uninitvar
    (void)mbtowc(pwc,pmb,max);
}

void uninitvar_mbrlen(const char* p, size_t m, mbstate_t* s)
{
    const char* pmb1, *pmb2;
    size_t max1, max2;
    mbstate_t* ps1, *ps2;
    // cppcheck-suppress uninitvar
    (void)mbrlen(pmb1,max1,ps1);
    // cppcheck-suppress uninitvar
    (void)mbrlen(pmb2,m,s);
    // cppcheck-suppress uninitvar
    (void)mbrlen(p,max2,s);
    // cppcheck-suppress uninitvar
    (void)mbrlen(p,m,ps2);
    // no warning is expected
    (void)mbrlen(p,m,s);
}

void nullPointer_mbrlen(const char* p, size_t m, mbstate_t* s)
{
    /* no warning is expected: A call to the function with a null pointer as pmb resets the shift state (and ignores parameter max). */
    (void)mbrlen(NULL,m,s);
    (void)mbrlen(NULL,0,s);
    /* cppcheck-suppress nullPointer */
    (void)mbrlen(p,m,NULL);
}

void uninitvar_btowc(void)
{
    int c;
    // cppcheck-suppress uninitvar
    (void)btowc(c);
}

void uninitvar_mbsinit(void)
{
    const mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)mbsinit(ps);
}

void uninitvar_mbstowcs(wchar_t* d, const char* s, size_t m)
{
    wchar_t *dest;
    const char *src;
    size_t max;

    // cppcheck-suppress uninitvar
    (void)mbstowcs(dest,s,m);
    // cppcheck-suppress uninitvar
    (void)mbstowcs(d,src,m);
    // cppcheck-suppress uninitvar
    (void)mbstowcs(d,s,max);

    // No warning is expected
    (void)mbstowcs(d,s,m);

    wchar_t buf[100];
    (void)mbstowcs(buf,s,100);
}

void uninitvar_mbsrtowcs(wchar_t* d, const char** s, size_t m, mbstate_t *p)
{
    wchar_t* dest;
    const char* src;
    size_t max;
    mbstate_t* ps;

    // cppcheck-suppress uninitvar
    (void)mbsrtowcs(dest,s,m,p);
    // cppcheck-suppress uninitvar
    (void)mbsrtowcs(d,&src,m,p);
    // cppcheck-suppress uninitvar
    (void)mbsrtowcs(d,s,max,p);
    // cppcheck-suppress uninitvar
    (void)mbsrtowcs(d,s,m,ps);

    // No warning is expected
    (void)mbsrtowcs(d,s,m,p);
}

void uninitvar_wctob(void)
{
    wint_t wc;
    // cppcheck-suppress uninitvar
    (void)wctob(wc);
}

void uninitvar_wctomb(void)
{
    char *s;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)wctomb(s,wc);
}

void uninitvar_wcstombs(void)
{
    char *mbstr;
    const wchar_t *wcstr;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wcstombs(mbstr,wcstr,n);
}

void uninitvar_getc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)getc(stream);
}

void uninitvar_getwc(void)
{
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)getwc(stream);
}

void uninitvar_ungetc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)ungetc(c,stream);
}

void uninitvar_ungetwc(void)
{
    wint_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)ungetwc(c,stream);
}

void uninitvar_getenv(void)
{
    const char *name;
    // cppcheck-suppress uninitvar
    (void)getenv(name);
}

#if defined(_WIN32) || (__STDC_VERSION__ < 201112L)
// since glibc 2.16 gets() is no longer available starting when >= C11
void uninitvar_gets(void)
{
    char *buffer;
    // cppcheck-suppress getsCalled
    // cppcheck-suppress uninitvar
    (void)gets(buffer);
}
#endif

void uninitvar_gmtime(void)
{
    const time_t *tp;
    // cppcheck-suppress uninitvar
    (void)gmtime(tp);
}

void uninitvar_isalnum(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isalnum(i);
}

void uninitvar_iswalnum(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswalnum(i);
}

void uninitvar_isalpha(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isalpha(i);
}

void uninitvar_iswalpha(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswalpha(i);
}

void uninitvar_isblank(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isblank(i);
}

void uninitvar_iswblank(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswblank(i);
}

void uninitvar_iscntrl(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)iscntrl(i);
}

void uninitvar_iswcntrl(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswcntrl(i);
}

void uninitvar_iswctype(void)
{
    wint_t c;
    wctype_t desc;
    // cppcheck-suppress uninitvar
    (void)iswctype(c,desc);
}

void uninitvar_isdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isdigit(i);
}

void uninitvar_iswdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswdigit(i);
}

void uninitvar_isgraph(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isgraph(i);
}

void uninitvar_iswgraph(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswgraph(i);
}

void uninitvar_islower(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)islower(i);
}

void uninitvar_iswlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswlower(i);
}

void uninitvar_isprint(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isprint(i);
}

void uninitvar_iswprint(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswprint(i);
}

void uninitvar_ispunct(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)ispunct(i);
}

void uninitvar_iswpunct(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswpunct(i);
}

void uninitvar_isspace(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isspace(i);
}

void uninitvar_iswspace(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswspace(i);
}

void uninitvar_isupper(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isupper(i);
}

void uninitvar_iswupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswupper(i);
}

void uninitvar_isxdigit(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)isxdigit(i);
}

void uninitvar_iswxdigit(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)iswxdigit(i);
}

void uninitvar_towctrans(void)
{
    wint_t c;
    wctrans_t desc;
    // cppcheck-suppress uninitvar
    (void)towctrans(c,desc);
}

void uninitvar_towlower(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)towlower(i);
}

void uninitvar_towupper(void)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)towupper(i);
}

void uninitvar_wctrans(void)
{
    const char* property;
    // cppcheck-suppress uninitvar
    (void)wctrans(property);
}

void uninitvar_wctype(void)
{
    const char* property;
    // cppcheck-suppress uninitvar
    (void)wctype(property);
}

void ignorereturn(void)
{
    const char szNumbers[] = "2001 60c0c0 -1101110100110100100000 0x6fffff";
    char * pEnd;
    strtol(szNumbers,&pEnd,10);
}

void uninitvar_cabs(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cabsf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cabs(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cabsl(ldc);
}

void uninitvar_cacos(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cacosf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cacos(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cacosl(ldc);
}

void uninitvar_cacosh(void)
{
    float complex fd;
    // cppcheck-suppress uninitvar
    (void)cacoshf(fd);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)cacosh(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)cacoshl(ldc);
}

void uninitvar_labs(void)
{
    long int li;
    // cppcheck-suppress uninitvar
    (void)labs(li);

    long long int lli;
    // cppcheck-suppress uninitvar
    (void)llabs(lli);
}

void uninitvar_ldexp(void)
{
    float f;
    int e1;
    // cppcheck-suppress uninitvar
    (void)ldexpf(f,e1);

    double d;
    int e2;
    // cppcheck-suppress uninitvar
    (void)ldexp(d,e2);

    long double ld;
    int e3;
    // cppcheck-suppress uninitvar
    (void)ldexpl(ld,e3);
}

void uninitvar_lgamma(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)lgammaf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)lgamma(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)lgammal(ld);
}

void uninitvar_rint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)rintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)rint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)rintl(ld);
}

void uninitvar_lrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)lrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)lrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)lrintl(ld);
}

void uninitvar_llrint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)llrintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)llrint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)llrintl(ld);
}

void uninitvar_lround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)lroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)lround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)lroundl(ld);
}

void uninitvar_llround(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)llroundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)llround(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)llroundl(ld);
}

void uninitvar_srand(void)
{
    unsigned int seed;
    // cppcheck-suppress uninitvar
    (void)srand(seed);
}

void uninitvar_ldiv(void)
{
    long int l1;
    long int l2;
    // cppcheck-suppress uninitvar
    (void)ldiv(l1,l2);

    long long int ll1;
    long long int ll2;
    // cppcheck-suppress uninitvar
    (void)lldiv(ll1,ll2);
}

void uninitvar_localtime(void)
{
    const time_t *tp;
    // cppcheck-suppress uninitvar
    (void)localtime(tp);
}

void uninitvar_log(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)logf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)logl(ld);
}

void uninitvar_clog(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)clogf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)clog(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)clogl(ldc);
}

void uninitvar_conj(void)
{
    float complex fc;
    // cppcheck-suppress uninitvar
    (void)conjf(fc);

    double complex dc;
    // cppcheck-suppress uninitvar
    (void)conj(dc);

    long double complex ldc;
    // cppcheck-suppress uninitvar
    (void)conjl(ldc);
}

void uninitvar_fpclassify(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)fpclassify(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)fpclassify(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)fpclassify(ld);
}

void uninitvar_isfinite(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)isfinite(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)isfinite(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)isfinite(ld);
}

void uninitvar_isgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)isgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)isgreater(ld1,ld2);
}

void uninitvar_isgreaterequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)isgreaterequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isgreaterequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)isgreaterequal(ld1,ld2);
}

void uninitvar_isinf(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)isinf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)isinf(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)isinf(ld);
}

void uninitvar_logb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)logbf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)logb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)logbl(ld);
}

void uninitvar_isless(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)isless(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isless(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)isless(ld1,ld2);
}

void uninitvar_islessequal(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)islessequal(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)islessequal(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)islessequal(ld1,ld2);
}

void uninitvar_islessgreater(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)islessgreater(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)islessgreater(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)islessgreater(ld1,ld2);
}

void uninitvar_nan(void)
{
    const char *tagp1, *tagp2, *tagp3;
    // cppcheck-suppress uninitvar
    (void)nanf(tagp1);
    // cppcheck-suppress uninitvar
    (void)nan(tagp2);
    // cppcheck-suppress uninitvar
    (void)nanl(tagp3);
}

void uninitvar_isnan(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)isnan(d);
}

void uninitvar_isnormal(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)isnormal(d);
}

void uninitvar_isunordered(void)
{
    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)isunordered(d1,d2);
}

void uninitvar_ilogb(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)ilogbf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)ilogb(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)ilogbl(ld);
}

void uninitvar_log10(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)log10f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log10(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)log10l(ld);
}

void uninitvar_log1p(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)log1pf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log1p(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)log1pl(ld);
}

void uninitvar_log2(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)log2f(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)log2(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)log2l(ld);
}

void uninitvar_nearbyint(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)nearbyintf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)nearbyint(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)nearbyintl(ld);
}

void uninitvar_nextafter(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)nextafterf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)nextafter(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)nextafterl(ld1,ld2);
}

void uninitvar_nexttoward(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)nexttowardf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)nexttoward(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)nexttowardl(ld1,ld2);
}

void uninitvar_longjmp(void)
{
    jmp_buf env;
    int val;
    // cppcheck-suppress uninitvar
    (void)longjmp(env,val);
}

void uninitvar_malloc(void)
{
    size_t size;
    // cppcheck-suppress unusedAllocatedMemory
    // cppcheck-suppress uninitvar
    int *p = (int*)malloc(size);
    free(p);
}

void uninitvar_alloca(void)
{
    size_t size;
    // cppcheck-suppress allocaCalled
    // cppcheck-suppress uninitvar
    (void)alloca(size);
}

void uninitvar_memchr(void)
{
    const void *cs;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memchr(cs,c,n);
}

void *bufferAccessOutOfBounds_memchr(const void *s, int c, size_t n)
{
    const char buf[42]={0};
    (void)memchr(buf,c,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)memchr(buf,c,43);
    return memchr(s,c,n);
}

void uninitvar_wmemchr(void)
{
    const wchar_t *cs;
    wchar_t c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemchr(cs,c,n);
}

void uninitvar_memcmp(void)
{
    const void *s1;
    const void *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memcmp(s1,s2,n);
}

void uninitvar_wmemcmp(void)
{
    const wchar_t *s1;
    const wchar_t *s2;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemcmp(s1,s2,n);
}

void uninitvar_memcpy(void)
{
    void *ct;
    const void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memcpy(ct,cs,n);
}

void uninitvar_wmemcpy(void)
{
    wchar_t *cs;
    const wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemcpy(cs,c,n);
}

void uninitvar_memmove(void)
{
    void *ct;
    void *cs;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memmove(ct,cs,n);
}

void uninitvar_wmemmove(void)
{
    wchar_t *cs;
    wchar_t *c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemmove(cs,c,n);
}

void uninitvar_memset(void)
{
    void *s;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)memset(s,c,n);
}

void uninitvar_wmemset(void)
{
    wchar_t *cs;
    wchar_t c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wmemset(cs,c,n);
}

void uninitvar_mktime(void)
{
    struct tm *tp;
    // cppcheck-suppress uninitvar
    (void)mktime(tp);
}

void uninitvar_modf(void)
{
    float f1;
    float *f2;
    // cppcheck-suppress uninitvar
    (void)modff(f1,f2);

    double d1;
    double *d2;
    // cppcheck-suppress uninitvar
    (void)modf(d1,d2);

    long double ld1;
    long double *ld2;
    // cppcheck-suppress uninitvar
    (void)modfl(ld1,ld2);
}

void uninitvar_perror(void)
{
    const char *string;
    // cppcheck-suppress uninitvar
    (void)perror(string);
}

void uninitvar_pow(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)powf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)pow(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)powl(ld1,ld2);
}

void uninitvar_cpow(void)
{
    float complex f1,f2;
    // cppcheck-suppress uninitvar
    (void)cpowf(f1,f2);

    double complex d1,d2;
    // cppcheck-suppress uninitvar
    (void)cpow(d1,d2);

    long double complex ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)cpowl(ld1,ld2);
}

void uninitvar_remainder(void)
{
    float f1,f2;
    // cppcheck-suppress uninitvar
    (void)remainderf(f1,f2);

    double d1,d2;
    // cppcheck-suppress uninitvar
    (void)remainder(d1,d2);

    long double ld1,ld2;
    // cppcheck-suppress uninitvar
    (void)remainderl(ld1,ld2);
}

void uninitvar_remquo(void)
{
    float f1,f2;
    int *i1;
    // cppcheck-suppress uninitvar
    (void)remquof(f1,f2,i1);

    double d1,d2;
    int *i2;
    // cppcheck-suppress uninitvar
    (void)remquo(d1,d2,i2);

    long double ld1,ld2;
    int *i3;
    // cppcheck-suppress uninitvar
    (void)remquol(ld1,ld2,i3);
}

void uninitvar_printf(const char *Format, int Argument)
{
    const char * format_1, * format_2, * format_3;
    int argument1, argument2;
    // no warning is expected
    (void)printf("x");
    // cppcheck-suppress uninitvar
    (void)printf(format_1,argument1);
    // cppcheck-suppress uninitvar
    (void)printf(Format,argument2);
    // cppcheck-suppress uninitvar
    (void)printf(format_2,Argument);
    // cppcheck-suppress uninitvar
    (void)printf(format_3,1);

    // no warning is expected
    (void)printf(Format,Argument);
}

void uninitvar_vprintf(const char *Format, va_list Arg)
{
    const char * format1, *format2;
    va_list arg1, arg2;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vprintf(format1,arg1);
    // cppcheck-suppress uninitvar
    (void)vprintf(format2,Arg);

    // no warning is expected
    (void)vprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vprintf(Format,arg2);
}

#if defined(_WIN32) || (__STDC_VERSION__ < 199901L)
// strdup() is POSIX since glibc 2.10 - it is also not available on Linux compiler starting with C99
void memleak_strdup (const char *s) // #9328
{
    const char *s1 = strdup(s);
    // cppcheck-suppress nullPointerOutOfMemory
    printf("%s",s1);
    free(s);     // s1 is not freed
    // cppcheck-suppress memleak
}
#endif

void uninitvar_vwprintf(const wchar_t *Format, va_list Arg)
{
    const wchar_t * format1, * format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vwprintf(format1,arg);
    // cppcheck-suppress uninitvar
    (void)vwprintf(format2,Arg);

    // no warning is expected
    (void)vwprintf(Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vwprintf(Format,arg);
}

void nullPointer_bsearch(const void* key, const void* base, size_t num, size_t size)
{
    // cppcheck-suppress nullPointer
    (void)bsearch(NULL,base,num,size,(int (*)(const void*,const void*))strcmp);
    // cppcheck-suppress nullPointer
    (void)bsearch(key,NULL,num,size,(int (*)(const void*,const void*))strcmp);
    // No warning is expected
    (void)bsearch(key,base,num,size,(int (*)(const void*,const void*))strcmp);
}

void uninitvar_bsearch(void)
{
    const void* key;
    const void* base;
    size_t num;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)bsearch(key,base,num,size,(int (*)(const void*,const void*))strcmp);
}

void uninitvar_qsort(void)
{
    void *base;
    size_t n;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)qsort(base,n,size, (int (*)(const void*,const void*))strcmp);
}

void uninitvar_putc(void)
{
    int c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)putc(c,stream);
}

void uninitvar_putwc(void)
{
    wchar_t c;
    FILE *stream;
    // cppcheck-suppress uninitvar
    (void)putc(c,stream);
}

void uninitvar_putwchar(void)
{
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)putwchar(c);
}

void uninitvar_realloc(void)
{
    void *block;
    size_t newsize;
    // cppcheck-suppress uninitvar
    void *p = realloc(block, newsize);
    free(p);
}

void uninitvar_remove(void)
{
    const char *s;
    // cppcheck-suppress uninitvar
    (void)remove(s);
}

void uninitvar_rename(void)
{
    const char *s1;
    const char *s2;
    // cppcheck-suppress uninitvar
    (void)rename(s1,s2);
}

void uninitvar_rewind(void)
{
    FILE *f;
    // cppcheck-suppress uninitvar
    (void)rewind(f);
}

void uninitvar_round(void)
{
    float f;
    // cppcheck-suppress uninitvar
    (void)roundf(f);

    double d;
    // cppcheck-suppress uninitvar
    (void)round(d);

    long double ld;
    // cppcheck-suppress uninitvar
    (void)roundl(ld);
}

void uninitvar_scalbn(void)
{
    float f;
    int i1;
    // cppcheck-suppress uninitvar
    (void)scalbnf(f,i1);

    double d;
    int i2;
    // cppcheck-suppress uninitvar
    (void)scalbn(d,i2);

    long double ld;
    int i3;
    // cppcheck-suppress uninitvar
    (void)scalbnl(ld,i3);
}

void uninitvar_scalbln(void)
{
    float f;
    long int i1;
    // cppcheck-suppress uninitvar
    (void)scalblnf(f,i1);

    double d;
    long int i2;
    // cppcheck-suppress uninitvar
    (void)scalbln(d,i2);

    long double ld;
    long int i3;
    // cppcheck-suppress uninitvar
    (void)scalblnl(ld,i3);
}

void uninitvar_signbit(void)
{
    double d;
    // cppcheck-suppress uninitvar
    (void)signbit(d);
}

void uninitvar_signal(void)
{
    int i;
    // cppcheck-suppress uninitvar
    signal(i, exit);
}

void uninitvar_raise(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)raise(i);
}

void uninitvar_scanf(void)
{
    const char *format;
    char str[42];
    // cppcheck-suppress uninitvar
    (void)scanf(format, str);

    // no warning is expected (#9347)
    int i;
    sscanf("0", "%d", &i);
}

void uninitvar_vsscanf(void)
{
    const char *s;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vsscanf(s,format,arg);
}

void uninitvar_vswscanf(void)
{
    const wchar_t *s;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vswscanf(s,format,arg);
}

void uninitvar_vscanf(void)
{
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vscanf(format,arg);
}

void uninitvar_vwscanf(void)
{
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vwscanf(format,arg);
}

void nullPointer_setbuf(FILE *stream, char *buf)
{
    // cppcheck-suppress nullPointer
    setbuf(NULL,buf);
    setbuf(stream,NULL);
    setbuf(stream,buf);
}

int bufferAccessOutOfBounds_setvbuf(FILE* stream, int mode, size_t size)
{
    char buf[42]={0};
    // cppcheck-suppress bufferAccessOutOfBounds
    (void) setvbuf(stream, buf, mode, 43);
    return setvbuf(stream, buf, mode, 42);
}

int nullPointer_setvbuf(FILE* stream, char *buf, int mode, size_t size)
{
    // cppcheck-suppress nullPointer
    (void) setvbuf(NULL, buf, mode, size);
    (void) setvbuf(stream, NULL, mode, size);
    return setvbuf(stream, buf, mode, size);
}

void uninitvar_setbuf(void)
{
    FILE *stream;
    char *buf;
    // cppcheck-suppress uninitvar
    setbuf(stream,buf);
}

void uninitvar_setvbuf(void)
{
    FILE *stream;
    char *buf;
    int mode;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)setvbuf(stream,buf,mode,size);
}

void uninitvar_strcat(char *dest, const char * const source)
{
    char *deststr1, *deststr2;
    const char *srcstr1, *srcstr2;
    // cppcheck-suppress uninitvar
    (void)strcat(deststr1,srcstr1);
    // cppcheck-suppress uninitvar
    (void)strcat(dest,srcstr2);
    // cppcheck-suppress uninitvar
    (void)strcat(deststr2,source);

    // no warning shall be shown for
    (void)strcat(dest,source);
}

void nullPointer_strcpy(char *dest, const char * const source)
{
    // cppcheck-suppress nullPointer
    (void)strcpy(NULL,source);
    // cppcheck-suppress nullPointer
    (void)strcpy(dest,NULL);

    // no warning shall be shown for
    (void)strcpy(dest,source);
}

void nullPointer_strcat(char *dest, const char * const source)
{
    // cppcheck-suppress nullPointer
    (void)strcat(NULL,source);
    // cppcheck-suppress nullPointer
    (void)strcat(dest,NULL);

    // no warning shall be shown for
    (void)strcat(dest,source);
}

void bufferAccessOutOfBounds_strcat(char *dest, const char * const source)
{
    char buf4[4] = {0};
    const char * const srcstr3 = "123";
    const char * const srcstr4 = "1234";
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)strcat(buf4,srcstr4); // off by one issue: strcat is appends \0' at the end

    // no warning shall be shown for
    (void)strcat(dest,source);
    (void)strcat(buf4,srcstr3); // strcat appends '\0' at the end
    (void)strcat(dest,srcstr4); // Cppcheck does not know the length of 'dest'
}

void uninitvar_wcscat(wchar_t *dest, const wchar_t * const source)
{
    wchar_t *deststr_1, *deststr_2;
    const wchar_t *srcstr_1, *srcstr_2;
    // cppcheck-suppress uninitvar
    (void)wcscat(deststr_1,srcstr_1);
    // cppcheck-suppress uninitvar
    (void)wcscat(dest,srcstr_2);
    // cppcheck-suppress uninitvar
    (void)wcscat(deststr_2,source);

    // no warning shall be shown for
    (void)wcscat(dest,source);
}

void uninitvar_wcrtomb(void)
{
    char *s;
    wchar_t wc;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)wcrtomb(s,wc,ps);
}

void uninitvar_strchr(void)
{
    const char *cs;
    int c;
    // cppcheck-suppress uninitvar
    (void)strchr(cs,c);
}

void invalidFunctionArg_strchr(const char *cs, int c)
{
    // cppcheck-suppress invalidFunctionArg
    (void)strchr(cs,-1);

    // No warning shall be issued for
    (void)strchr(cs, 0);
    (void)strchr(cs, 255);

    // cppcheck-suppress invalidFunctionArg
    (void)strchr(cs, 256);
}

void invalidFunctionArg_log10(float f, double d, const long double ld)
{
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log10f(0.0f);
    (void)log10f(1.4013e-45f); // note: calculated by nextafterf(0.0f, 1.0f);
    (void)log10f(f);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    (void)log10f(FLT_MAX);

    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log10(0.0);
    (void)log10(4.94066e-324); // note: calculated by nextafterf(0.0, 1.0);
    (void)log10(d);
    (void)log10(DBL_MAX);

    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log10l(0.0L);
    (void)log10l(4.94066e-324L); // note: calculated by nextafterf(0.0L, 1.0L);
    (void)log10l(ld);
    (void)log10l(LDBL_MAX);
}

void invalidFunctionArg_log(float f, double d, const long double ld)
{
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)logf(0.0f);
    (void)logf(1.4013e-45f); // note: calculated by nextafterf(0.0f, 1.0f);
    (void)logf(f);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    (void)logf(FLT_MAX);

    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log(0.0);
    (void)log(4.94066e-324); // note: calculated by nextafterf(0.0, 1.0);
    (void)log(d);
    (void)log(DBL_MAX);

    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)logl(0.0L);
    (void)logl(4.94066e-324L); // note: calculated by nextafterf(0.0L, 1.0L);
    (void)logl(ld);
    (void)logl(LDBL_MAX);
}

void invalidFunctionArg_log2(float f, double d, const long double ld)
{
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log2f(0.0f);
    (void)log2f(1.4013e-45f); // note: calculated by nextafterf(0.0f, 1.0f);
    (void)log2f(f);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    (void)log2f(FLT_MAX);

    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log2(0.0);
    (void)log2(4.94066e-324); // note: calculated by nextafterf(0.0, 1.0);
    (void)log2(d);
    (void)log2(DBL_MAX);

    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress wrongmathcall
    (void)log2l(0.0L);
    (void)log2l(4.94066e-324L); // note: calculated by nextafterf(0.0L, 1.0L);
    (void)log2l(ld);
    (void)log2l(LDBL_MAX);
}

void uninitvar_wcschr(void)
{
    const wchar_t *cs;
    wchar_t c;
    // cppcheck-suppress uninitvar
    (void)wcschr(cs,c);
}

void nullPointer_strcmp(const char *s1, const char *s2)
{
    // cppcheck-suppress nullPointer
    (void)strcmp(NULL,s2);
    // cppcheck-suppress nullPointer
    (void)strcmp(s1,NULL);
    (void)strcmp(s1,s2);
}

void uninitvar_strcmp(const char *s1, const char *s2)
{
    const char *str1;
    const char *str2;
    const char *str3;
    const char *str4;

    // cppcheck-suppress uninitvar
    (void)strcmp(str1,s2);
    // cppcheck-suppress uninitvar
    (void)strcmp(s1,str2);
    // cppcheck-suppress uninitvar
    (void)strcmp(str3,str4);

    // No warning is expected
    (void)strcmp(s1,s2);
}

void uninitvar_wcscmp(const wchar_t *s1, const wchar_t *s2)
{
    const wchar_t *str1;
    const wchar_t *str2;
    const wchar_t *str3;
    const wchar_t *str4;

    // cppcheck-suppress uninitvar
    (void)wcscmp(str1,s2);
    // cppcheck-suppress uninitvar
    (void)wcscmp(s1,str2);
    // cppcheck-suppress uninitvar
    (void)wcscmp(str3,str4);

    // No warning is expected
    (void)wcscmp(s1,s2);
}

void uninitvar_strcpy(char *d, const char *s)
{
    char *dest1, *dest2;
    const char *src1, *src2;

    // cppcheck-suppress uninitvar
    (void)strcpy(dest1,s);
    // cppcheck-suppress uninitvar
    (void)strcpy(d,src1);
    // cppcheck-suppress uninitvar
    (void)strcpy(dest2,src2);

    // No warning is expected
    (void)strcpy(d,s);
}

#ifdef __STDC_LIB_EXT1__
void uninitvar_strcpy_s(char * strDest, ssize_t s, const char *source)
{
    char *strUninit1;
    const char *strUninit2;
    ssize_t size;

    // cppcheck-suppress uninitvar
    (void)strcpy_s(strUninit1, 1, "a");
    // cppcheck-suppress uninitvar
    (void)strcpy_s(strDest, 1, strUninit2);
    // cppcheck-suppress uninitvar
    (void)strcpy_s(strDest, size, "a");

    // No warning is expected
    (void)strcpy_s(strDest, s, source);
}
#endif

void uninitvar_wcscpy(wchar_t *d, const wchar_t*s)
{
    wchar_t *dest1, *dest2;
    const wchar_t *src1, *src2;

    // cppcheck-suppress uninitvar
    (void)wcscpy(dest1,s);
    // cppcheck-suppress uninitvar
    (void)wcscpy(d,src1);
    // cppcheck-suppress uninitvar
    (void)wcscpy(dest2,src2);

    // No warning is expected
    (void)wcscpy(d,s);
}

size_t bufferAccessOutOfBounds_strftime(char *s, size_t max, const char *fmt, const struct tm *p)
{
    char buf[42] = {0};
    // cppcheck-suppress bufferAccessOutOfBounds
    (void) strftime(buf,43,fmt,p);
    (void) strftime(buf,max,fmt,p);
    return strftime(buf,42,fmt,p);
}

size_t nullPointer_strftime(char *s, size_t max, const char *fmt, const struct tm *p)
{
    // cppcheck-suppress nullPointer
    (void) strftime(NULL,max,fmt,p);
    // cppcheck-suppress nullPointer
    (void) strftime(s,max,NULL,p);
    // cppcheck-suppress nullPointer
    (void) strftime(s,max,fmt,NULL);
    return strftime(s,max,fmt,p);
}

void uninitvar_strftime(void)
{
    char *s;
    size_t max;
    const char *fmt;
    const struct tm *p;
    // cppcheck-suppress uninitvar
    (void)strftime(s,max,fmt,p);
}

void uninitvar_strlen(const char *str)
{
    const char *s;
    // cppcheck-suppress uninitvar
    (void)strlen(s);

    const char x;
    const char *xPtr = &x;
    // cppcheck-suppress uninitvar
    (void)strlen(xPtr);

    // No warning is expected
    (void)strlen(str);
}

void uninitvar_wcslen(void)
{
    const wchar_t *s;
    // cppcheck-suppress uninitvar
    (void)wcslen(s);
}

//char * strncpy ( char * destination, const char * source, size_t num );
void uninitvar_strncpy(char * dest, const char * src, size_t num)
{
    char *d;
    const char *s;
    size_t n;

    // cppcheck-suppress uninitvar
    (void)strncpy(d,src,num);
    // cppcheck-suppress uninitvar
    (void)strncpy(dest,s,num);
    // cppcheck-suppress uninitvar
    (void)strncpy(dest,src,n);

    // No warning is expected for
    (void)strncpy(dest,src,num);
}

#ifdef __STDC_LIB_EXT1__
void uninitvar_strncpy_s(char *Ct, size_t N1, const char *S, size_t N2)
{
    char dest[42];
    const char *s1, *s2;
    size_t n1;
    size_t n2;
    size_t n3;
    size_t n4;

    // cppcheck-suppress uninitvar
    (void)strncpy_s(dest,n1,s1,n2);
    // cppcheck-suppress uninitvar
    (void)strncpy_s(Ct,n3,S,N2);
    // cppcheck-suppress uninitvar
    (void)strncpy_s(Ct,N1,s2,N2);
    // cppcheck-suppress uninitvar
    (void)strncpy_s(Ct,N1,S,n4);

    // no warning is expected for
    (void)strncpy_s(Ct,N1,S,N2);
    (void)strncpy_s(dest,N1,S,N2);
}
#endif

void uninitvar_strpbrk(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)strpbrk(cs,ct);
}

// char * strncat ( char * destination, const char * source, size_t num );
void uninitvar_strncat(char *d, const char *s, size_t n)
{
    char *dest;
    const char *src;
    size_t num;

    // cppcheck-suppress uninitvar
    (void)strncat(dest,s,n);
    // cppcheck-suppress uninitvar
    (void)strncat(d,src,n);
    // cppcheck-suppress uninitvar
    (void)strncat(d,s,num);

    // no warning is expected for
    (void)strncat(d,s,n);
}

void nullPointer_strncat(char *d, const char *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)strncat(NULL,s,n);
    // cppcheck-suppress nullPointer
    (void)strncat(d,NULL,n);
    // no warning is expected for
    (void)strncat(d,s,n);
}

void nullPointer_strncpy(char *d, const char *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)strncpy(NULL,s,n);
    // cppcheck-suppress nullPointer
    (void)strncpy(d,NULL,n);
    // no warning is expected for
    (void)strncpy(d,s,n);
}

#ifdef __STDC_LIB_EXT1__
// errno_t strcat_s(char *restrict dest, rsize_t destsz, const char *restrict src); // since C11
void uninitvar_strcat_s(char *Ct, size_t N, const char *S)
{
    char *ct_1, *ct_2;
    const char *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)strcat_s(ct_1,n1,s1);
    // cppcheck-suppress uninitvar
    (void)strcat_s(ct_2,N,S);
    // cppcheck-suppress uninitvar
    (void)strcat_s(Ct,N,s2);
    // cppcheck-suppress uninitvar
    (void)strcat_s(Ct,n2,S);

    // no warning is expected for
    (void) strcat_s(Ct,N,S);
}

// errno_t wcscat_s(wchar_t *restrict dest, rsize_t destsz, const wchar_t *restrict src); // since C11
void uninitvar_wcscat_s(wchar_t *Ct, size_t N, const wchar_t *S)
{
    wchar_t *ct_1, *ct_2;
    const wchar_t *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)wcscat_s(ct_1,n1,s1);
    // cppcheck-suppress uninitvar
    (void)wcscat_s(ct_2,N,S);
    // cppcheck-suppress uninitvar
    (void)wcscat_s(Ct,N,s2);
    // cppcheck-suppress uninitvar
    (void)wcscat_s(Ct,n2,S);

    // no warning is expected for
    (void) wcscat_s(Ct,N,S);
}

void uninitvar_strncat_s(char *Ct, size_t N1, const char *S, size_t N2)
{
    char *ct_1, *ct_2;
    const char *s1, *s2;
    size_t n1;
    size_t n2;
    size_t n3;
    size_t n4;

    // cppcheck-suppress uninitvar
    (void)strncat_s(ct_1,n1,s1,n2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(ct_2,N1,S,N2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(Ct,n3,S,N2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(Ct,N1,s2,N2);
    // cppcheck-suppress uninitvar
    (void)strncat_s(Ct,N1,S,n4);

    // no warning is expected for
    (void)strncat_s(Ct,N1,S,N2);
}
#endif

void uninitvar_wcsncat(wchar_t *Ct, const wchar_t *S, size_t N)
{
    wchar_t *ct_1, *ct_2;
    const wchar_t *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)wcsncat(ct_1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)wcsncat(ct_2,S,N);
    // cppcheck-suppress uninitvar
    (void)wcsncat(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)wcsncat(Ct,S,n2);

    // no warning is expected for
    (void)wcsncat(Ct,S,N);
}

void uninitvar_strncmp(const char *Ct, const char *S, size_t N)
{
    const char *ct;
    const char *s;
    size_t n1;

    // cppcheck-suppress uninitvar
    (void)strncmp(ct,S,N);
    // cppcheck-suppress uninitvar
    (void)strncmp(Ct,s,N);
    // cppcheck-suppress uninitvar
    (void)strncmp(Ct,S,n1);

    // no warning is expected for
    (void)strncmp(Ct,S,N);
}

void uninitvar_wcsncmp(const wchar_t *Ct, const wchar_t *S, size_t N)
{
    const wchar_t *ct1, *ct2;
    const wchar_t *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)wcsncmp(ct1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)wcsncmp(ct2,S,N);
    // cppcheck-suppress uninitvar
    (void)wcsncmp(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)wcsncmp(Ct,S,n2);

    // no warning is expected for
    (void)wcsncmp(Ct,S,N);
}

void uninitvar_strstr(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)strstr(cs,ct);
}

void uninitvar_wcsstr(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcsstr(cs,ct);
}

void uninitvar_strspn(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)strspn(cs,ct);
}

void uninitvar_strxfrm(void)
{
    char *ds;
    const char *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)strxfrm(ds,ss,n);
}

void bufferAccessOutOfBounds_strxfrm(void)
{
    const char src[3] = "abc";
    char dest[1] = "a";
    // cppcheck-suppress invalidFunctionArgStr
    (void)strxfrm(dest,src,1);
    // cppcheck-suppress [bufferAccessOutOfBounds,invalidFunctionArgStr]
    (void)strxfrm(dest,src,2);
    // cppcheck-suppress [bufferAccessOutOfBounds,invalidFunctionArgStr]
    (void)strxfrm(dest,src,3);
}

void bufferAccessOutOfBounds_strncmp(void)
{
    const char src[3] = "abc";
    const char dest[1] = "a";
    (void)strncmp(dest,src,1);
    (void)strncmp(dest,src,2);
    (void)strncmp(dest,src,3);
}

void nullPointer_wmemcmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wmemcmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)wmemcmp(s1,NULL,n);
    (void)wmemcmp(s1,s2,n);
}

void nullPointer_wmemmove(wchar_t* s1, const wchar_t* s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wmemmove(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)wmemmove(s1,NULL,n);
    (void)wmemmove(s1,s2,n);
}

void nullPointer_wmemset(wchar_t* s, wchar_t c, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wmemset(NULL,c,n);
    (void)wmemset(s,c,n);
}

void nullPointer_memmove(void *s1, void *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)memmove(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)memmove(s1,NULL,n);
    (void)memmove(s1,s2,n);
}

void nullPointer_memcmp(const void *s1, const void *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)memcmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)memcmp(s1,NULL,n);
    (void)memcmp(s1,s2,n);
}

void nullPointer_memcpy(void *s1, const void *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)memcpy(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)memcpy(s1,NULL,n);
    (void)memcpy(s1,s2,n);
}

void nullPointer_strncmp(const char *s1, const char *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)strncmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)strncmp(s1,NULL,n);
    (void)strncmp(s1,s2,n);
}

void uninitvar_wcsxfrm(void)
{
    wchar_t *ds;
    const wchar_t *ss;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wcsxfrm(ds,ss,n);
}

void uninitvar_wcsspn(void)
{
    const wchar_t *ds;
    const wchar_t *ss;
    // cppcheck-suppress uninitvar
    (void)wcsspn(ds,ss);
}

void uninitvar_setlocale(void)
{
    int category;
    const char* locale;
    // cppcheck-suppress uninitvar
    (void)setlocale(category,locale);
}

void uninitvar_strerror(void)
{
    int i;
    // cppcheck-suppress uninitvar
    (void)strerror(i);
}

void uninitvar_strcspn(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)strcspn(cs,ct);
}

void uninitvar_wcscspn(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcscspn(cs,ct);
}

void uninitvar_wcspbrk(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcspbrk(cs,ct);
}

void uninitvar_wcsncpy(void)
{
    wchar_t *cs;
    const wchar_t *ct;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)wcsncpy(cs,ct,n);
}

void uninitvar_strcoll(void)
{
    const char *cs;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)strcoll(cs,ct);
}

void uninitvar_wcscoll(void)
{
    const wchar_t *cs;
    const wchar_t *ct;
    // cppcheck-suppress uninitvar
    (void)wcscoll(cs,ct);
}

//const char * strrchr ( const char * str, int character );
//      char * strrchr (       char * str, int character );
void uninitvar_strrchr(const char * s, int c)
{
    const char * str;
    int character;

    // cppcheck-suppress uninitvar
    (void)strrchr(str,c);
    // cppcheck-suppress uninitvar
    (void)strrchr(s,character);

    // No warning is expected for
    (void)strrchr(s,c);
}

void uninitvar_wcsrchr(void)
{
    const wchar_t* ws;
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)wcsrchr(ws,wc);
}

void uninitvar_wcsrtombs(void)
{
    char *dst;
    const wchar_t * p;;
    size_t len;
    mbstate_t *ps;
    // cppcheck-suppress uninitvar
    (void)wcsrtombs(dst,&p,len,ps);
}

void uninitvar_strtok(void)
{
    char *s;
    const char *ct;
    // cppcheck-suppress uninitvar
    (void)strtok(s,ct);
}

void uninitvar_strtoimax(void)
{
    const char *s1, *s2;
    char **endp1, **endp2;
    int base1, base2;
    // cppcheck-suppress uninitvar
    (void)strtoimax(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)strtoumax(s2,endp2,base2);
}

void uninitvar_strtof(void)
{
    const char *s1, *s2, *s3;
    char **endp1, **endp2, **endp3;
    // cppcheck-suppress uninitvar
    (void)strtof(s1,endp1);
    // cppcheck-suppress uninitvar
    (void)strtod(s2,endp2);
    // cppcheck-suppress uninitvar
    (void)strtold(s3,endp3);
}

void uninitvar_strtol(void)
{
    const char *s1, *s2, *s3, *s4;
    char **endp1, **endp2, **endp3, **endp4;
    int base1, base2, base3, base4;
    // cppcheck-suppress uninitvar
    (void)strtol(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)strtoll(s2,endp2,base2);
    // cppcheck-suppress uninitvar
    (void)strtoul(s3,endp3,base3);
    // cppcheck-suppress uninitvar
    (void)strtoull(s4,endp4,base4);
}

void uninitvar_time(void)
{
    time_t *tp;
    // cppcheck-suppress uninitvar
    (void)time(tp);
}

void uninitvar_tmpnam(void)
{
    char *s;
    // cppcheck-suppress uninitvar
    (void)tmpnam(s);
}

void uninitvar_tolower(int character)
{
    int c1;
    // cppcheck-suppress uninitvar
    (void)tolower(c1);

    int c2;
    // cppcheck-suppress constVariablePointer
    int *pc=&c2;
    // cppcheck-suppress uninitvar
    (void)tolower(*pc);

    // No warning is expected
    (void)tolower(character);

    // cppcheck-suppress constVariablePointer
    int *pChar = &character;
    // No warning is expected
    (void)tolower(*pChar);
}

void uninitvar_toupper(int character)
{
    int c1;
    // cppcheck-suppress uninitvar
    (void)toupper(c1);

    int c2;
    // cppcheck-suppress constVariablePointer
    int *pc=&c2;
    // cppcheck-suppress uninitvar
    (void)toupper(*pc);

    // No warning is expected
    (void)toupper(character);

    // cppcheck-suppress constVariablePointer
    int *pChar = &character;
    // No warning is expected
    (void)toupper(*pChar);
}

void uninitvar_wcstof(void)
{
    const wchar_t *s1, *s2, *s3;
    wchar_t **endp1, **endp2, **endp3;
    // cppcheck-suppress uninitvar
    (void)wcstof(s1,endp1);
    // cppcheck-suppress uninitvar
    (void)wcstod(s2,endp2);
    // cppcheck-suppress uninitvar
    (void)wcstold(s3,endp3);
}

void uninitvar_mbrtowc(void)
{
    wchar_t* pwc;
    const char* pmb;
    size_t max;
    mbstate_t* ps;
    // cppcheck-suppress uninitvar
    (void)mbrtowc(pwc,pmb,max,ps);
}

void uninitvar_wcstok(void)
{
    wchar_t *s;
    const wchar_t *ct;
    wchar_t **ptr;
    // cppcheck-suppress uninitvar
    (void)wcstok(s,ct,ptr);
}

void uninitvar_wcstoimax(void)
{
    const wchar_t *s1, *s2;
    wchar_t ** endp1, **endp2;
    int base1, base2;
    // cppcheck-suppress uninitvar
    (void)wcstoimax(s1,endp1,base1);
    // cppcheck-suppress uninitvar
    (void)wcstoumax(s2,endp2,base2);
}

void uninitvar_wcstol(void)
{
    const wchar_t *s1, *s2, *s3, *s4;
    wchar_t ** endp;
    int base1, base2, base3, base4;
    // cppcheck-suppress uninitvar
    (void)wcstol(s1,endp,base1);
    // cppcheck-suppress uninitvar
    (void)wcstoll(s2,endp,base2);
    // cppcheck-suppress uninitvar
    (void)wcstoul(s3,endp,base3);
    // cppcheck-suppress uninitvar
    (void)wcstoull(s4,endp,base4);
}

void uninitvar_wprintf(const wchar_t *Format, int Argument)
{
    const wchar_t *format1, *format2, *format3;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)wprintf(format1,argument1);
    // cppcheck-suppress uninitvar
    (void)wprintf(format2);
    // cppcheck-suppress uninitvar
    (void)wprintf(Format,argument2);
    // cppcheck-suppress uninitvar
    (void)wprintf(format3,Argument);
    // no warning is expected
    (void)wprintf(Format,Argument);
    (void)wprintf(Format);
}

void uninitvar_sprintf(char *S, const char *Format, int Argument)
{
    char *s1, *s2;
    const char *format1, *format2;
    int argument1, argument2;
    // cppcheck-suppress uninitvar
    (void)sprintf(s1,format1,argument1);
    // cppcheck-suppress uninitvar
    (void)sprintf(s2,Format,Argument);
    // cppcheck-suppress uninitvar
    (void)sprintf(S,format2,Argument);
    // cppcheck-suppress uninitvar
    (void)sprintf(S,Format,argument2);

    // no warning is expected for
    (void)sprintf(S,Format,Argument);
}

void uninitvar_swprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    // cppcheck-suppress uninitvar
    (void)swprintf(s,n,format);
}

void uninitvar_vsprintf(void)
{
    char *s;
    const char *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vsprintf(s,format,arg);
}

void valid_vsprintf_helper(const char * format, ...)
{
    char buffer[2];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    printf(buffer);
    va_end(args);
}

void valid_vsprintf()
{
    // buffer will contain "2\0" => no bufferAccessOutOfBounds
    valid_vsprintf_helper("%1.0f", 2.0f);
}

int nullPointer_vswprintf(wchar_t* restrict ws, size_t s, const wchar_t* restrict format, va_list ap)
{
    // cppcheck-suppress nullPointer
    vswprintf(NULL, s,format, ap);
    // cppcheck-suppress nullPointer
    vswprintf(ws, s,NULL, ap);
    return vswprintf(ws, s,format, ap);
}

void uninitvar_vswprintf(void)
{
    wchar_t *s;
    size_t n;
    const wchar_t *format;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vswprintf(s,n,format,arg);
}

void uninitvar_fwprintf(void)
{
    FILE* stream;
    const wchar_t* format;
    int i;
    // cppcheck-suppress uninitvar
    (void)fwprintf(stream,format,i);
}

void uninitvar_snprintf(char *S, size_t N, const char *Format, int Int)
{
    size_t n1,n2;
    const char *format1, *format2;
    int i1, i2;
    char *s1, *s2;
    // cppcheck-suppress uninitvar
    (void)snprintf(s1,n1,format1,i1);
    // cppcheck-suppress uninitvar
    (void)snprintf(S,n2,Format,Int); // n is uninitialized
    // cppcheck-suppress uninitvar
    (void)snprintf(S,N,format2,Int); // format is uninitialized
    // cppcheck-suppress uninitvar
    (void)snprintf(S,N,Format,i2); // i is uninitialized
    // cppcheck-suppress uninitvar
    (void)snprintf(s2,N,Format,Int);

    // no warning is expected for
    (void)snprintf(S,N,Format,Int);
}

void uninitvar_vsnprintf(char *S, size_t N, const char *Format, va_list Arg)
{
    char *s1, *s2;
    size_t n1, n2;
    const char *format1, *format2;
    va_list arg;
    // cppcheck-suppress va_list_usedBeforeStarted
    // cppcheck-suppress uninitvar
    (void)vsnprintf(s1,n1,format1,arg);
    // cppcheck-suppress uninitvar
    (void)vsnprintf(s2,N,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)vsnprintf(S,n2,Format,Arg);
    // cppcheck-suppress uninitvar
    (void)vsnprintf(S,N,format2,Arg);

    // no warning is expected for
    (void)vsnprintf(S,N,Format,Arg);
    // cppcheck-suppress va_list_usedBeforeStarted
    (void)vsnprintf(S,N,Format,arg);
}

void uninitvar_wscanf(void)
{
    const wchar_t *format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)wscanf(format1);
    // cppcheck-suppress uninitvar
    (void)wscanf(format2,&i);
}

void uninitvar_sscanf(const char *s, const char *f, int i, int *ip)
{
    const char *string1, *string2, *string3;
    const char * format;
    int *pInteger;

    // cppcheck-suppress uninitvar
    (void)sscanf(string1,f);
    // cppcheck-suppress uninitvar
    (void)sscanf(string2,f,i);
    // cppcheck-suppress uninitvar
    (void)sscanf(string3,f,ip);
    // cppcheck-suppress uninitvar
    (void)sscanf(s,format,&i);
    // cppcheck-suppress uninitvar
    (void)sscanf(s,f,pInteger);

    // No warning is expected
    (void)sscanf(s,f,&i);
    (void)sscanf(s,f,ip);
}

void uninitvar_fwscanf(void)
{
    FILE* stream;
    const wchar_t* format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)fwscanf(stream,format1);
    // cppcheck-suppress uninitvar
    (void)fwscanf(stream,format2,&i);
}

void uninitvar_swscanf(void)
{
    const wchar_t* s;
    const wchar_t* format1, *format2;
    int i;
    // cppcheck-suppress uninitvar
    (void)swscanf(s,format1);
    // cppcheck-suppress uninitvar
    (void)swscanf(s,format2,&i);
}

void uninitvar_system(void)
{
    const char *c;
    // cppcheck-suppress uninitvar
    (void)system(c);
}

void nullPointer_system(const char *c)
{
    // If a null pointer is given, command processor is checked for existence
    (void)system(NULL);
    (void)system(c);
}

#ifndef __STDC_NO_THREADS__
int nullPointer_mtx_timedlock( mtx_t *restrict mutex, const struct timespec *restrict time_point )
{
    // cppcheck-suppress nullPointer
    (void) mtx_timedlock(NULL, time_point);
    // cppcheck-suppress nullPointer
    (void) mtx_timedlock(mutex, NULL);
    return mtx_timedlock(mutex, time_point);
}
#endif

#ifdef _WIN32
/* itoa() is not ANSI-C and not implemented by all compilers */
void uninitvar_itoa(void)
{
    int value;
    char * str;
    int base;
    // cppcheck-suppress uninitvar
    (void)itoa(value,str,base);
}
#endif

#ifdef __STD_UTF_16__
void uninitvar_c16rtomb(void)
{
    char * pmb;
    char16_t c16;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)c16rtomb(pmb,c16,ps);
}

void uninitvar_mbrtoc16(void)
{
    char16_t * pc16;
    const char * pmb;
    size_t max;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)mbrtoc16(pc16,pmb,max,ps);
}
#endif // __STD_UTF_16__

#ifdef __STD_UTF_32__
void uninitvar_c32rtomb(void)
{
    char * pmb;
    char32_t c32;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)c32rtomb(pmb,c32,ps);
}

void uninitvar_mbrtoc32(void)
{
    char32_t * pc32;
    const char * pmb;
    size_t max;
    mbstate_t * ps;
    // cppcheck-suppress uninitvar
    (void)mbrtoc32(pc32,pmb,max,ps);
}
#endif // __STD_UTF_32__

void invalidFunctionArgBool_abs(bool b, double x, double y)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)abs(true); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)abs(b); // #6990
    // cppcheck-suppress invalidFunctionArgBool
    (void)abs(x<y); // #5635
}

int invalidFunctionArgBool_tolower(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)tolower(b);
    // cppcheck-suppress invalidFunctionArgBool
    return tolower(c != 0);
}

int invalidFunctionArgBool_toupper(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)toupper(b);
    // cppcheck-suppress invalidFunctionArgBool
    return toupper(c != 0);
}

bool invalidFunctionArgBool_iscntrl(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)iscntrl(b);
    // cppcheck-suppress invalidFunctionArgBool
    return iscntrl(c != 0);
}

bool invalidFunctionArgBool_isalpha(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isalpha(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isalpha(c != 0);
}

bool invalidFunctionArgBool_isalnum(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isalnum(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isalnum(c != 0);
}

bool invalidFunctionArgBool_isspace(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isspace(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isspace(c != 0);
}

bool invalidFunctionArgBool_isdigit(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isdigit(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isdigit(c != 0);
}

bool invalidFunctionArgBool_isgraph(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isgraph(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isgraph(c != 0);
}

bool invalidFunctionArgBool_islower(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)islower(b);
    // cppcheck-suppress invalidFunctionArgBool
    return islower(c != 0);
}

bool invalidFunctionArgBool_iswcntrl(bool b, wint_t c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)iswcntrl(b);
    // cppcheck-suppress invalidFunctionArgBool
    return iswcntrl(c != 0);
}

bool invalidFunctionArgBool_isprint(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isprint(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isprint(c != 0);
}

bool invalidFunctionArgBool_isblank(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isblank(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isblank(c != 0);
}

bool invalidFunctionArgBool_ispunct(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)ispunct(b);
    // cppcheck-suppress invalidFunctionArgBool
    return ispunct(c != 0);
}

bool invalidFunctionArgBool_isupper(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isupper(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isupper(c != 0);
}

bool invalidFunctionArgBool_isxdigit(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isxdigit(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isxdigit(c != 0);
}

void invalidFunctionArg(char c)
{
#ifdef __STDC_LIB_EXT1__
    // cppcheck-suppress asctime_sCalled
    // cppcheck-suppress invalidFunctionArg
    asctime_s(1, 24, 1);
#endif

    /* cppcheck-suppress invalidFunctionArg */
    (void)isalnum(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isalnum(-1);
    /* no warning for*/
    (void)isalnum(c);
    (void)isalnum(0);
    (void)isalnum(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isalpha(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isalpha(-1);
    /* no warning for*/
    (void)isalpha(c);
    (void)isalpha(0);
    (void)isalpha(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isblank(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isblank(-1);
    /* no warning for*/
    (void)isblank(c);
    (void)isblank(0);
    (void)isblank(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)iscntrl(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)iscntrl(-1);
    /* no warning for*/
    (void)iscntrl(c);
    (void)iscntrl(0);
    (void)iscntrl(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isdigit(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isdigit(-1);
    /* no warning for*/
    (void)isdigit(c);
    (void)isdigit(0);
    (void)isdigit(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isgraph(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isgraph(-1);
    /* no warning for*/
    (void)isgraph(c);
    (void)isgraph(0);
    (void)isgraph(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)islower(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)islower(-1);
    /* no warning for*/
    (void)islower(c);
    (void)islower(0);
    (void)islower(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isupper(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isupper(-1);
    /* no warning for*/
    (void)isupper(c);
    (void)isupper(0);
    (void)isupper(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isprint(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isprint(-1);
    /* no warning for*/
    (void)isprint(c);
    (void)isprint(0);
    (void)isprint(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)ispunct(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)ispunct(-1);
    /* no warning for*/
    (void)ispunct(c);
    (void)ispunct(0);
    (void)ispunct(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isspace(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isspace(-1);
    /* no warning for*/
    (void)isspace(c);
    (void)isspace(0);
    (void)isspace(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)isxdigit(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)isxdigit(-1);
    /* no warning for*/
    (void)isxdigit(c);
    (void)isxdigit(0);
    (void)isxdigit(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)tolower(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)tolower(-1);
    /* no warning for*/
    (void)tolower(c);
    (void)tolower(0);
    (void)tolower(255);

    /* cppcheck-suppress invalidFunctionArg */
    (void)toupper(256);
    /* cppcheck-suppress invalidFunctionArg */
    (void)toupper(-1);
    /* no warning for*/
    (void)toupper(c);
    (void)toupper(0);
    (void)toupper(255);

#ifdef __STDC_LIB_EXT1__
    /* cppcheck-suppress invalidFunctionArg */
    (void)strcpy_s(1,0,"a");
#endif
}

void invalidFunctionArgString(char c)
{
    /* cppcheck-suppress invalidFunctionArgStr */
    (void)atoi(&c);
    char x = 'x';
    /* cppcheck-suppress invalidFunctionArgStr */
    (void)strlen(&x);

    char y = '\0';
    (void)strlen(&y);

    // #5225
    char str[80] = "hello worl";
    char d='d';
    /* cppcheck-suppress invalidFunctionArgStr */
    (void)strcat(str,&d);
}

void ignoredReturnValue_abs(int i)
{
    // cppcheck-suppress ignoredReturnValue
    abs(i);
    // cppcheck-suppress ignoredReturnValue
    abs(-100);
}

int clamp(int, int, int, int); // #13599
void ignoredReturnValue_clamp(int a, int b, int c, int d) {
    clamp(a, b, c, d); // not a library function
}

void nullPointer_asctime(void)
{
    const struct tm *tm = 0;
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)asctime(tm);
    // cppcheck-suppress asctimeCalled
    // cppcheck-suppress nullPointer
    (void)asctime(0);
}

#ifdef __STDC_LIB_EXT1__
void nullPointer_asctime_s(void)
{
    const struct tm *tm = 0;
    char * buf = NULL;
    // cppcheck-suppress asctime_sCalled
    // cppcheck-suppress nullPointer
    asctime_s(buf, 26, 1);
    // cppcheck-suppress asctime_sCalled
    // cppcheck-suppress nullPointer
    asctime_s(1, 26, tm);
}
#endif

void nullPointer_fegetenv(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)fegetenv(envp);
    // cppcheck-suppress nullPointer
    (void)fegetenv(0);
}

void nullPointer_fegetexceptflag(int excepts)
{
    fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)fegetexceptflag(flagp,excepts);
    // cppcheck-suppress nullPointer
    (void)fegetexceptflag(0,excepts);
}

void nullPointer_feholdexcept(void)
{
    fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)feholdexcept(envp);
    // cppcheck-suppress nullPointer
    (void)feholdexcept(0);
}

void nullPointer_fesetenv(void)
{
    const fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)fesetenv(envp);
    // cppcheck-suppress nullPointer
    (void)fesetenv(0);
}

void nullPointer_fesetexceptflag(int excepts)
{
    const fexcept_t* flagp = 0;
    // cppcheck-suppress nullPointer
    (void)fesetexceptflag(flagp,excepts);
    // cppcheck-suppress nullPointer
    (void)fesetexceptflag(0,excepts);
}

void invalidFunctionArg_fesetexceptflag(const fexcept_t* flagp, int excepts)
{
    (void)fesetexceptflag(flagp, excepts);
    // cppcheck-suppress invalidFunctionArg
    (void)fesetexceptflag(flagp, 0);
    (void)fesetexceptflag(flagp, FE_DIVBYZERO);
    (void)fesetexceptflag(flagp, FE_INEXACT);
    (void)fesetexceptflag(flagp, FE_INVALID);
    (void)fesetexceptflag(flagp, FE_OVERFLOW);
    (void)fesetexceptflag(flagp, FE_UNDERFLOW);
    (void)fesetexceptflag(flagp, FE_ALL_EXCEPT);
    // cppcheck-suppress invalidFunctionArg
    (void)fesetexceptflag(flagp, FE_ALL_EXCEPT+1);
}

void invalidFunctionArg_fetestexcept(int excepts)
{
    (void)fetestexcept(excepts);
    // cppcheck-suppress invalidFunctionArg
    (void)fetestexcept(0);
    (void)fetestexcept(FE_DIVBYZERO);
    (void)fetestexcept(FE_INEXACT);
    (void)fetestexcept(FE_INVALID);
    (void)fetestexcept(FE_OVERFLOW);
    (void)fetestexcept(FE_UNDERFLOW);
    (void)fetestexcept(FE_ALL_EXCEPT);
    // cppcheck-suppress invalidFunctionArg
    (void)fetestexcept(FE_ALL_EXCEPT+1);
}

void nullPointer_feupdateenv(void)
{
    const fenv_t* envp = 0;
    // cppcheck-suppress nullPointer
    (void)feupdateenv(envp);
    // cppcheck-suppress nullPointer
    (void)feupdateenv(0);
}

void nullPointer_atexit(void)
{
    // cppcheck-suppress nullPointer
    (void)atexit(0);
}

void nullPointer_atof(void)
{
    const char * c = 0;
    // cppcheck-suppress nullPointer
    (void)atof(c);
    // cppcheck-suppress nullPointer
    (void)atof(0);
}

void invalidPrintfArgType_printf(void)
{
    int i = 0;
    // cppcheck-suppress invalidPrintfArgType_float
    printf("%f",i);

    // #7016
    uint8_t n = 7;
    // TODO cppcheck-suppress invalidPrintfArgType_uint
    printf("%" PRIi16 "\n", n);
}


#define AssertAlwaysTrue(C)  if (C) {}

void valueFlow(void)
{
    const char abc[] = "abc";
    const int three = 3, minusThree = -3;
    const int c0='0', ca='a', blank=' ', tab='\t';
    const wint_t wblank=L' ', wtab=L'\t', w0=L'0';

    // When adding functions below, please sort alphabetically.

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(strlen(abc) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(abs(three) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(abs(minusThree) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isblank(blank) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isblank(tab) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isblank(c0) == 0);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isdigit(c0) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(isdigit(ca) == 0);

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswblank(wblank) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswblank(wtab) == 1);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswblank(w0) == 0);

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswdigit(w0) == 0);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(iswdigit(wtab) == 1);

    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(labs(three) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(labs(minusThree) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(llabs(three) == 3);
    // TODO cppcheck-suppress knownConditionTrueFalse
    AssertAlwaysTrue(llabs(minusThree) == 3);
}
