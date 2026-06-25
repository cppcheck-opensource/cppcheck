// Test library configuration for bsd.cfg
//
// Usage:
// $ cppcheck --check-library --library=bsd --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/bsd.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/uio.h>

void nullPointer_setbuffer(FILE *stream, char *buf, size_t size)
{
    // cppcheck-suppress nullPointer
    (void) setbuffer(NULL, buf, size);
    (void) setbuffer(stream, NULL, size);
    (void) setbuffer(stream, buf, size);
}

void nullPointer_setlinebuf(FILE *stream)
{
    // cppcheck-suppress nullPointer
    (void)setlinebuf(NULL);
    (void)setlinebuf(stream);
}

// #9323, #9331
void verify_timercmp(struct timeval t)
{
    // TODO cppcheck-suppress duplicateExpression
    (void)timercmp(&t, &t, <);
    // TODO cppcheck-suppress duplicateExpression
    (void)timercmp(&t, &t, <=);
    (void)timercmp(&t, &t, ==);
    (void)timercmp(&t, &t, !=);
    // TODO cppcheck-suppress duplicateExpression
    (void)timercmp(&t, &t, >=);
    // TODO cppcheck-suppress duplicateExpression
    (void)timercmp(&t, &t, >);
}

ssize_t nullPointer_readv(int fd, const struct iovec *iov, int iovcnt)
{
    // cppcheck-suppress nullPointer
    (void)readv(fd,NULL,iovcnt);
    return readv(fd,iov,iovcnt);
}

ssize_t nullPointer_writev(int fd, const struct iovec *iov, int iovcnt)
{
    // cppcheck-suppress nullPointer
    (void)writev(fd,NULL,iovcnt);
    return writev(fd,iov,iovcnt);
}

ssize_t nullPointer_preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
    // cppcheck-suppress nullPointer
    (void)preadv(fd,NULL,iovcnt,offset);
    return preadv(fd,iov,iovcnt,offset);
}

ssize_t nullPointer_pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
    // cppcheck-suppress nullPointer
    (void)pwritev(fd,NULL,iovcnt,offset);
    return pwritev(fd,iov,iovcnt,offset);
}

// False negative: #9346
void uninitvar_timercmp(struct timeval t)
{
    struct timeval uninit;
    // cppcheck-suppress uninitvar
    (void)timercmp(&t, &uninit, <);
    (void)timercmp(&uninit, &t, <=);
    (void)timercmp(&uninit, &uninit, ==);
}

void nullPointer_timercmp(struct timeval t)
{
    // cppcheck-suppress constVariablePointer
    struct timeval *p=0;
    // cppcheck-suppress nullPointer
    (void)timercmp(&t, p, <);
    // cppcheck-suppress nullPointer
    (void)timercmp(p, &t, <=);
    // cppcheck-suppress nullPointer
    (void)timercmp(p, p, ==);
}

// size_t strlcat(char *dst, const char *src, size_t size);
void uninitvar_strlcat(char *Ct, const char *S, size_t N)
{
    char *ct1, *ct2;
    char *s1, *s2;
    size_t n1, n2;
    // cppcheck-suppress uninitvar
    (void)strlcat(ct1,s1,n1);
    // cppcheck-suppress uninitvar
    (void)strlcat(ct2,S,N);
    // cppcheck-suppress uninitvar
    (void)strlcat(Ct,s2,N);
    // cppcheck-suppress uninitvar
    (void)strlcat(Ct,S,n2);

    // no warning is expected for
    (void)strlcat(Ct,S,N);
}

void bufferAccessOutOfBounds(void)
{
    uint16_t uint16Buf[4];
    // cppcheck-suppress bufferAccessOutOfBounds
    arc4random_buf(uint16Buf, 9);
    // valid
    arc4random_buf(uint16Buf, 8);
}

void ignoredReturnValue(void)
{
    // cppcheck-suppress ignoredReturnValue
    arc4random();
    // cppcheck-suppress ignoredReturnValue
    arc4random_uniform(10);
}

void invalidFunctionArg()
{
    // cppcheck-suppress invalidFunctionArg
    (void) arc4random_uniform(1);
    // valid
    (void) arc4random_uniform(2);
}

void nullPointer(void)
{
    // cppcheck-suppress nullPointer
    arc4random_buf(NULL, 5);
}

void uninitvar(void)
{
    uint32_t uint32Uninit;

    // cppcheck-suppress uninitvar
    (void) arc4random_uniform(uint32Uninit);
}

void arrayIndexOutOfBounds(void)
{
    char * pAlloc = calloc(2, 3);
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc[5] = 'a';
    // cppcheck-suppress arrayIndexOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    pAlloc[6] = 1;
    // cppcheck-suppress memleakOnRealloc
    pAlloc = reallocarray(pAlloc, 3, 3);
    pAlloc[8] = 'a';
    // cppcheck-suppress arrayIndexOutOfBounds
    pAlloc[9] = 1;
    free(pAlloc);
}

void reallocarray_memleak(void) {
    char *a = (char *)malloc(10);
    // cppcheck-suppress [memleakOnRealloc, unreadVariable]
    a = reallocarray(a, 100, 2);
    // cppcheck-suppress memleak
}

void reallocarray_notused(void)
{
    // cppcheck-suppress [leakReturnValNotUsed, ignoredReturnValue]
    reallocarray(NULL, 10, 10);
}
