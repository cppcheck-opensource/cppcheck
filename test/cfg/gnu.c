
// Test library configuration for gnu.cfg
//
// Usage:
// $ cppcheck --check-library --library=gnu --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/gnu.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file [valueFlowBailout,purgedConfiguration]

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <wchar.h>
#include <execinfo.h>
#if !defined(__CYGWIN__) && !defined(__APPLE__)
#include <sys/epoll.h>
#endif
#include <strings.h>
#ifdef __gnu_linux__
#include <error.h>
#endif
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#if !defined(__APPLE__)
#include <byteswap.h>
#include <features.h>
#endif
#include <sys/socket.h>
#include <time.h>
#include <stddef.h>

#ifdef __gnu_linux__
void unreachableCode_error(void) // #11197
{
    error(1, 0, ""); // will call exit() if the first parameter is non-zero
    // cppcheck-suppress unusedVariable
    // TODO cppcheck-suppress unreachableCode
    int i;
}
#endif


int deallocuse_backtrace(int size) {
    void **buffer = (void **)malloc(sizeof(void *) * size);
    free(buffer);
    // cppcheck-suppress deallocuse
    // cppcheck-suppress nullPointerOutOfMemory
    int numEntries = backtrace(buffer, size);
    return numEntries;
}

#if !defined(__APPLE__)
void leakReturnValNotUsed_get_current_dir_name(void)
{
    // cppcheck-suppress leakReturnValNotUsed
    get_current_dir_name();
}

void memleak_get_current_dir_name0(void)
{
    const char *const name = get_current_dir_name();
    if (name)
    {
        // cppcheck-suppress memleak
        return;
    }
}

void memleak_get_current_dir_name1(void)
{
    const char *const name = get_current_dir_name();
    if (name)
    {
        free(name);
        return;
    }
}

int nullPointer_gethostbyname2_r(const char* name, int af, struct hostent* ret, const char* buf, size_t buflen, struct hostent** result, const int* h_errnop)
{
    // cppcheck-suppress nullPointer
    (void) gethostbyname2_r(NULL, af, ret, buf, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname2_r(name, af, NULL, buf, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname2_r(name, af, ret, NULL, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname2_r(name, af, ret, buf, buflen, NULL, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname2_r(name, af, ret, buf, buflen, result, NULL);
    return gethostbyname2_r(name, af, ret, buf, buflen, result, h_errnop);
}

int nullPointer_gethostbyname_r(const char* name, struct hostent* ret, const char* buf, size_t buflen, struct hostent** result, const int* h_errnop)
{
    // cppcheck-suppress nullPointer
    (void) gethostbyname_r(NULL, ret, buf, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname_r(name, NULL, buf, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname_r(name, ret, NULL, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname_r(name, ret, buf, buflen, NULL, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyname_r(name, ret, buf, buflen, result, NULL);
    return gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}

int nullPointer_gethostbyaddr_r(const void* addr, socklen_t len, int type, struct hostent* ret, const char* buf, size_t buflen, struct hostent** result, const int* h_errnop)
{
    // cppcheck-suppress nullPointer
    (void) gethostbyaddr_r(NULL, len, type, ret, buf, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyaddr_r(addr, len, type, NULL, buf, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyaddr_r(addr, len, type, ret, NULL, buflen, result, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyaddr_r(addr, len, type, ret, buf, buflen, NULL, h_errnop);
    // cppcheck-suppress nullPointer
    (void) gethostbyaddr_r(addr, len, type, ret, buf, buflen, result, NULL);
    return gethostbyaddr_r(addr, len, type, ret, buf, buflen, result, h_errnop);
}
#endif

int nullPointer_getopt_long(int argc, char **argv, const char *optstring,
                            const struct option *longopts, int *longindex)
{
    // cppcheck-suppress nullPointer
    (void) getopt_long(argc, argv, NULL, longopts, longindex);
    // cppcheck-suppress nullPointer
    (void) getopt_long(argc, argv, optstring, NULL, longindex);
    // cppcheck-suppress nullPointer
    (void) getopt_long(argc, NULL, optstring, longopts, longindex);
    return getopt_long(argc, argv, optstring, longopts, longindex);
}

int nullPointer_getopt_long_only(int argc, char* const* argv, const char* optstring,
                                 const struct option* longopts, int* longindex)
{
    // cppcheck-suppress nullPointer
    (void) getopt_long_only(argc, NULL, optstring, longopts, longindex);
    // cppcheck-suppress nullPointer
    (void) getopt_long_only(argc, argv, NULL, longopts, longindex);
    // cppcheck-suppress nullPointer
    (void) getopt_long_only(argc, argv, optstring, NULL, longindex);
    return getopt_long_only(argc, argv, optstring, longopts, longindex);
}

#if !defined(__APPLE__)
int nullPointer_getservent_r(struct servent *restrict result_buf, const char *restrict buf, size_t buflen, struct servent **restrict result)
{
    // cppcheck-suppress nullPointer
    (void) getservent_r(NULL, buf, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getservent_r(result_buf, NULL, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getservent_r(result_buf, buf, buflen, NULL);
    return getservent_r(result_buf, buf, buflen, result);
}

void *bufferAccessOutOfBounds_memrchr(const void *s, int c, size_t n)
{
    const char buf[42]={0};
    (void)memrchr(buf,c,42);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)memrchr(buf,c,43);
    return memrchr(s,c,n);
}
#endif

void knownConditionTrueFalse_ffsl(long i)
{
    // ffs() returns the position of the first bit set, or 0 if no bits are set in i.
    const int x = ffsl(0);
    // cppcheck-suppress knownConditionTrueFalse
    if (x == 0) {}
    if (ffsl(i) == 0) {}
}

void knownConditionTrueFalse_ffsll(long long i)
{
    // ffs() returns the position of the first bit set, or 0 if no bits are set in i.
    const int x = ffsll(0);
    // cppcheck-suppress knownConditionTrueFalse
    if (x == 0) {}
    if (ffsll(i) == 0) {}
}

#if !defined(__APPLE__)
int nullPointer_semtimedop(int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout)
{
    (void) semtimedop(semid, sops, nsops, NULL); // If the timeout argument is NULL, then semtimedop() behaves exactly like semop().
    (void) semtimedop(semid, sops, nsops, timeout);
    // cppcheck-suppress nullPointer
    return semtimedop(semid, NULL, nsops, timeout);
}

void *nullPointer_mempcpy(void *dest, const void *src, size_t n)
{
    // cppcheck-suppress nullPointer
    (void) mempcpy(NULL,src,n);
    // cppcheck-suppress nullPointer
    (void) mempcpy(dest,NULL,n);
    return mempcpy(dest,src,n);
}

wchar_t *nullPointer_wmempcpy(wchar_t *dest, const wchar_t *src, size_t n)
{
    // cppcheck-suppress nullPointer
    (void) wmempcpy(NULL,src,n);
    // cppcheck-suppress nullPointer
    (void) wmempcpy(dest,NULL,n);
    return wmempcpy(dest,src,n);
}

int uninitvar_getpw(uid_t uid, char *buf)
{
    uid_t someUid;
    // cppcheck-suppress getpwCalled
    (void)getpw(uid, buf);
    // cppcheck-suppress getpwCalled
    // cppcheck-suppress uninitvar
    return getpw(someUid, buf);
}
#endif

// Declaration necessary because there is no specific / portable header.
// https://www.eyrie.org/~eagle/software/rra-c-util/xmalloc.html
extern void *xcalloc(size_t nmemb, size_t size);
extern void *xmalloc(size_t size);
extern void *xrealloc(void *block, size_t newsize);
extern void xfree(void *ptr);

void resourceLeak_mkostemps(char *template, int suffixlen, int flags)
{
    // cppcheck-suppress unreadVariable
    int fp = mkostemps(template, suffixlen, flags);
    // cppcheck-suppress resourceLeak
}

void no_resourceLeak_mkostemps_01(char *template, int suffixlen, int flags)
{
    int fp = mkostemps(template, suffixlen, flags);
    close(fp);
}

int no_resourceLeak_mkostemps_02(char *template, int suffixlen, int flags)
{
    return mkostemps(template, suffixlen, flags);
}

void resourceLeak_mkstemps(char *template, int suffixlen)
{
    // cppcheck-suppress unreadVariable
    int fp = mkstemps(template, suffixlen);
    // cppcheck-suppress resourceLeak
}

void no_resourceLeak_mkstemps_01(char *template, int suffixlen)
{
    int fp = mkstemps(template, suffixlen);
    close(fp);
}

int no_resourceLeak_mkstemps_02(char *template, int suffixlen)
{
    return mkstemps(template, suffixlen);
}

void resourceLeak_mkostemp(char *template, int flags)
{
    // cppcheck-suppress unreadVariable
    int fp = mkostemp(template, flags);
    // cppcheck-suppress resourceLeak
}

void no_resourceLeak_mkostemp_01(char *template, int flags)
{
    int fp = mkostemp(template, flags);
    close(fp);
}

int no_resourceLeak_mkostemp_02(char *template, int flags)
{
    return mkostemp(template, flags);
}

void valid_code(int argInt1, va_list valist_arg, const int * parg)
{
    char *p;

    if (__builtin_expect(argInt1, 0)) {}
    if (__builtin_expect_with_probability(argInt1 + 1, 2, 0.5)) {}
#ifdef __GLIBC__
    if (__glibc_unlikely(argInt1 != 0)) {}
    if (__glibc_likely(parg != NULL)) {}
#endif
    const void *ax1 = __builtin_assume_aligned(parg, 16);
    printf("%p", ax1);
    const void *ax2 = __builtin_assume_aligned(parg, 32, 8);
    printf("%p", ax2);

    p = (char *)malloc(10);
    free(p);
    p = (char *)malloc(5);
    xfree(p);
    p = (char *)xmalloc(10);
    free(p);
    p = (char *)xmalloc(5);
    xfree(p);

    // cppcheck-suppress allocaCalled
    p = __builtin_alloca(5);
    p[0] = 1;
    // TODO cppcheck-suppress arrayIndexOutOfBounds
    p[5] = 1;
    __builtin_prefetch(p, 0, 1);

    if (__builtin_types_compatible_p(int, char)) {}

    char * pStr = NULL;
    if (vasprintf(&pStr, "%d %d", valist_arg) != -1) {
        free(pStr);
    }

    printf("%d", 0b010);
    printf("%d", __extension__ 0b10001000);

    if (__alignof__(int) == 4) {}

    // cppcheck-suppress valueFlowBailoutIncompleteVar
    const void * p_mmap = mmap(NULL, 1, PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    printf("%p", p_mmap);
    // cppcheck-suppress nullPointerOutOfMemory
    munmap(p_mmap, 1);

    uint16_t i16_1 = 0, i16_2;
    // cppcheck-suppress unreadVariable
    i16_2 = __builtin_bswap16(i16_1++);
    uint32_t i32_1 = 0, i32_2;
    // cppcheck-suppress unreadVariable
    i32_2 = __builtin_bswap32(i32_1++);
    uint64_t i64_1 = 0, i64_2;
    // cppcheck-suppress unreadVariable
    i64_2 = __builtin_bswap64(i64_1++);

#if !defined(__APPLE__)
    // cppcheck-suppress zerodiv
    // cppcheck-suppress unreadVariable
    i16_1 /= bswap_16(0x1234) - 0x3412;
    // cppcheck-suppress zerodiv
    // cppcheck-suppress unreadVariable
    i32_1 /= bswap_32(0x12345678) - 0x78563412;
    // cppcheck-suppress zerodiv
    // cppcheck-suppress unreadVariable
    i64_1 /= bswap_64(0x023456789abcde0f) - 0x0fdebc9a78563402;
#endif
}

void ignoreleak(void)
{
    char *p = (char *)malloc(10);
    __builtin_memset(&(p[0]), 0, 10);
    // cppcheck-suppress memleak
}

void memleak_asprintf(char **ptr, const char *fmt, const int arg)
{
    // No warning is expected for
    if (-1 != asprintf(ptr,fmt,arg)) {
        free(ptr);
    }
    if (-1 != asprintf(ptr,fmt,arg)) {}
}

void memleak_asprintf2() { // #12186
    char* p = malloc(5);
    // cppcheck-suppress memleak
    (void)asprintf(&p, "%s", "test");
    // cppcheck-suppress memleak
}

void memleak_asprintf3() {
    char* p = malloc(5);
    // cppcheck-suppress memleak
    asprintf(&p, "%s", "test");
    free(p);
}

void memleak_asprintf4(char** p) {
    asprintf(p, "%s", "test");
}

void memleak_asprintf5(char* p) {
    asprintf(&p, "%s", "test");
    // cppcheck-suppress memleak
}

void memleak_asprintf6(const char* fmt, const int arg) {
    char* ptr;
    if (-1 == asprintf(&ptr, fmt, arg))
        return;
    printf("%s", ptr);
    free(ptr);
}

void memleak_asprintf7(const char* fmt, const int arg) {
    char* ptr;
    if (asprintf(&ptr, fmt, arg) != -1) {
        printf("%s", ptr);
        free(ptr);
    }
    else
        return;
}

void memleak_asprintf8(const char *fmt, const int arg) // #12204
{
    char* ptr;
    int ret = asprintf(&ptr, fmt, arg);
    if (-1 == ret) {
        return;
    }
    printf("%s", ptr);
    free(ptr);
}

void memleak_xmalloc()
{
    char *p = (char*)xmalloc(10);
    // cppcheck-suppress nullPointerOutOfMemory
    p[9] = 0;
    // cppcheck-suppress memleak
}

void memleak_mmap()
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    const void * p_mmap = mmap(NULL, 1, PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    printf("%p", p_mmap);
    // cppcheck-suppress memleak
}

void uninitvar__builtin_memset(void)
{
    void *s;
    int c;
    size_t n;
    // cppcheck-suppress uninitvar
    (void)__builtin_memset(s,c,n);
}

void bufferAccessOutOfBounds__builtin_memset(void)
{
    uint8_t buf[42];
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)__builtin_memset(buf,0,1000);
}

void bufferAccessOutOfBounds()
{
    const char buf[2] = "a";
    // This is valid
    sethostname(buf, 2);
    // cppcheck-suppress bufferAccessOutOfBounds
    sethostname(buf, 4);

    char * pAlloc1 = xcalloc(2, 4);
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc1, 0, 8);
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc1, 0, 9);
    free(pAlloc1);

    char * pAlloc2 = xmalloc(4);
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc2, 0, 4);
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc2, 0, 5);

    pAlloc2 = xrealloc(pAlloc2, 10);
    memset(pAlloc2, 0, 10);
    // cppcheck-suppress bufferAccessOutOfBounds
    memset(pAlloc2, 0, 11);

    free(pAlloc2);
}

#if !defined(__APPLE__)
void leakReturnValNotUsed()
{
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    char* ptr = (char*)strdupa("test");
    // cppcheck-suppress ignoredReturnValue
    strdupa("test");
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    char* ptr2 = (char*)strndupa("test", 1);
    // cppcheck-suppress ignoredReturnValue
    strndupa("test", 1);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress nullPointer
    strcasestr("test", NULL);

    // FIXME cppcheck-suppress knownConditionTrueFalse
    // cppcheck-suppress duplicateExpression
    if (42 == __builtin_expect(42, 0))
        return;
}
#endif

#if !defined(__CYGWIN__) && !defined(__APPLE__)
int nullPointer_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    // no warning is expected
    (void)epoll_ctl(epfd, op, fd, event);

    // No nullpointer warning is expected in case op is set to EPOLL_CTL_DEL
    //   EPOLL_CTL_DEL
    //          Remove (deregister) the target file descriptor fd from the
    //          epoll instance referred to by epfd.  The event is ignored and
    //          can be NULL.
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}
#endif
