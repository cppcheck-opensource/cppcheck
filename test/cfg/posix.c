
// Test library configuration for posix.cfg
//
// Usage:
// $ cppcheck --check-library --library=posix --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/posix.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file [valueFlowBailout]

#define _BSD_SOURCE
#define _XOPEN_SOURCE // wcwidth

#include <aio.h>
#include <stdio.h> // <- FILE
#include <dirent.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <dlfcn.h>
#include <fcntl.h>
// #include <ndbm.h> // unavailable on some linux systems
#include <netdb.h>
#include <regex.h>
#include <time.h>
#include <pthread.h>
#include <syslog.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#if defined(__APPLE__)
#include <xlocale.h>
#endif
#if !defined(__APPLE__)
#include <mqueue.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <sys/stat.h>
#include <utime.h>
#include <stddef.h>


#if !defined(__APPLE__)
void nullPointer_mq_timedsend(mqd_t mqdes, const char* msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec* abs_timeout) {
    // cppcheck-suppress nullPointer
    (void) mq_timedsend(mqdes, NULL, msg_len, msg_prio, abs_timeout);
    // cppcheck-suppress nullPointer
    (void) mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}
#endif

#if __TRACE_H__ // <trace.h>

void nullPointer_posix_trace_event(trace_event_id_t event_id, const void* restrictdata_ptr, size_t data_len)
{
    // cppcheck-suppress nullPointer
    (void) posix_trace_event(event_id, NULL, data_len);
    (void) posix_trace_event(event_id, restrictdata_ptr, 0);
}

void nullPointer_posix_trace_trygetnext_event(trace_id_t trid,
                                              struct posix_trace_event_info *event,
                                              void *data, size_t num_bytes,
                                              size_t *data_len, int *unavailable)
{
    // cppcheck-suppress nullPointer
    (void) posix_trace_trygetnext_event(trid, NULL, data, num_bytes, data_len, unavailable);
    // cppcheck-suppress nullPointer
    (void) posix_trace_trygetnext_event(trid, event, NULL, num_bytes, data_len, unavailable);
    // cppcheck-suppress nullPointer
    (void) posix_trace_trygetnext_event(trid, event, data, num_bytes, NULL, unavailable);
    // cppcheck-suppress nullPointer
    (void) posix_trace_trygetnext_event(trid, event, data, num_bytes, data_len, NULL);
}

int nullPointer_posix_trace_timedgetnext_event(trace_id_t trid, struct posix_trace_event_info *restrict event, void *restrict data, size_t num_bytes, size_t *restrict data_len, int *restrict unavailable, const struct timespec *restrict abstime)
{
    // cppcheck-suppress nullPointer
    (void) posix_trace_timedgetnext_event(trid, NULL, data, num_bytes, data_len, unavailable, abstime);
    // cppcheck-suppress nullPointer
    (void) posix_trace_timedgetnext_event(trid, event, NULL, num_bytes, data_len, unavailable, abstime);
    // cppcheck-suppress nullPointer
    (void) posix_trace_timedgetnext_event(trid, event, data, num_bytes, NULL, unavailable, abstime);
    // cppcheck-suppress nullPointer
    (void) posix_trace_timedgetnext_event(trid, event, data, num_bytes, data_len, NULL, abstime);
    // cppcheck-suppress nullPointer
    (void) posix_trace_timedgetnext_event(trid, event, data, num_bytes, data_len, unavailable, NULL);
    return posix_trace_timedgetnext_event(trid, event, data, num_bytes, data_len, unavailable, abstime);
}

int nullPointer_posix_trace_getnext_event(trace_id_t trid, struct posix_trace_event_info *restrict event, const void *restrict data, size_t num_bytes, size_t *restrict data_len, int *restrict unavailable)
{
    // cppcheck-suppress nullPointer
    (void) posix_trace_getnext_event(trid, NULL, data, num_bytes, data_len, unavailable);
    // cppcheck-suppress nullPointer
    (void) posix_trace_getnext_event(trid, event, NULL, num_bytes, data_len, unavailable);
    // cppcheck-suppress nullPointer
    (void) posix_trace_getnext_event(trid, event, data, num_bytes, NULL, unavailable);
    // cppcheck-suppress nullPointer
    (void) posix_trace_getnext_event(trid, event, data, num_bytes, data_len, NULL);
    return posix_trace_getnext_event(trid, event, data, num_bytes, data_len, unavailable);
}
#endif // __TRACE_H__

size_t nullPointer_strxfrm_l(char *restrict dest, const char *restrict src, size_t count, locale_t locale)
{
    (void)strxfrm_l(dest, src, count, locale);
    // In case the 3rd argument is 0, the 1st argument is permitted to be a null pointer. (#6306)
    (void)strxfrm_l(NULL, src, 0, locale);
    (void)strxfrm_l(NULL, src, 1, locale);
    (void)strxfrm_l(NULL, src, count, locale);
    // cppcheck-suppress nullPointer
    return strxfrm_l(dest, NULL, count, locale);
}

void nullPointer_pthread_attr_getstack(const pthread_attr_t *attr, void *stackaddr, size_t stacksize) {
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(NULL, &stackaddr, &stacksize);
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(attr, NULL, &stacksize);
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(attr, &stackaddr, NULL);
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(NULL, NULL, &stacksize);
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(NULL, &stackaddr, NULL);
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(attr, NULL, NULL);
    // cppcheck-suppress nullPointer
    (void) pthread_attr_getstack(NULL, NULL, NULL);
}

void nullPointer_pthread_attr_setstack(const pthread_attr_t *attr) {
    // cppcheck-suppress nullPointer
    (void) pthread_attr_setstack(NULL, NULL, 0);
    (void) pthread_attr_setstack(attr, NULL, 0);
    // cppcheck-suppress nullPointer
    // cppcheck-suppress intToPointerCast
    (void) pthread_attr_setstack(NULL, (void*) 1, 0);
}

#ifndef __linux__
void nullPointer_setkey(const char *key)
{
    // cppcheck-suppress nullPointer
    setkey(NULL);
}

void nullPointer_encrypt(const char block[64], int edflag)
{
    // cppcheck-suppress nullPointer
    encrypt(NULL, edflag);
    encrypt(block, edflag);
}
#endif

int nullPointer_getopt(int argc, char* const argv[], const char* optstring)
{
    // cppcheck-suppress nullPointer
    (void) getopt(argc, NULL, optstring);
    // cppcheck-suppress nullPointer
    (void) getopt(argc, argv, NULL);
    return getopt(argc, argv, optstring);
}

#if !defined(__APPLE__)
int invalidFunctionArgStr_mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
{
    // No warning is expected for:
    const char msg = '0';
    (void) mq_send(mqdes, &msg, 1, 0);
    return mq_send(mqdes, msg_ptr, msg_len, 0);
}
#endif

void invalidFunctionArgStr_mbsnrtowcs(void)
{
    wchar_t wenough[10];
    mbstate_t s;
    memset (&s, '\0', sizeof (s));
    const char* cp = "ABC";
    wcscpy (wenough, L"DEF");
    // No warning is expected for - #11119
    if (mbsnrtowcs (wenough, &cp, 1, 10, &s) != 1 || wcscmp (wenough, L"AEF") != 0) {}
}

struct tm * ignoredReturnValue_localtime(const time_t *tp)
{
    // cppcheck-suppress [ignoredReturnValue,localtimeCalled]
    localtime(tp);
    // cppcheck-suppress localtimeCalled
    return localtime(tp);
}

int nullPointer_getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
    // cppcheck-suppress nullPointer
    (void) getpwuid_r(uid, NULL, buffer, bufsize, result);
    // cppcheck-suppress nullPointer
    (void) getpwuid_r(uid, pwd, NULL, bufsize, result);
    // cppcheck-suppress nullPointer
    (void) getpwuid_r(uid, pwd, buffer, bufsize, NULL);
    return getpwuid_r(uid, pwd, buffer, bufsize, result);
}

int nullPointer_getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
    // cppcheck-suppress nullPointer
    (void) getpwnam_r(NULL, pwd, buffer, bufsize, result);
    // cppcheck-suppress nullPointer
    (void) getpwnam_r(name, NULL, buffer, bufsize, result);
    // cppcheck-suppress nullPointer
    (void) getpwnam_r(name, pwd, NULL, bufsize, result);
    // cppcheck-suppress nullPointer
    (void) getpwnam_r(name, pwd, buffer, bufsize, NULL);
    return getpwnam_r(name, pwd, buffer, bufsize, result);
}

#if !defined(__APPLE__)
int nullPointer_fgetpwent_r(FILE *restrict stream, const struct passwd *restrict pwbuf, char *restrict buf, size_t buflen, struct passwd **restrict pwbufp)
{
    // cppcheck-suppress nullPointer
    (void) fgetpwent_r(NULL, pwbuf, buf, buflen, pwbufp);
    // cppcheck-suppress nullPointer
    (void) fgetpwent_r(stream, NULL, buf, buflen, pwbufp);
    // cppcheck-suppress nullPointer
    (void) fgetpwent_r(stream, pwbuf, NULL, buflen, pwbufp);
    // cppcheck-suppress nullPointer
    (void) fgetpwent_r(stream, pwbuf, buf, buflen, NULL);
    return fgetpwent_r(stream, pwbuf, buf, buflen, pwbufp);
}

int nullPointer_getpwent_r(const struct passwd *restrict pwbuf, char *restrict buf, size_t buflen, struct passwd **restrict pwbufp)
{
    // cppcheck-suppress nullPointer
    (void) getpwent_r(NULL, buf, buflen, pwbufp);
    // cppcheck-suppress nullPointer
    (void) getpwent_r(pwbuf, NULL, buflen, pwbufp);
    // cppcheck-suppress nullPointer
    (void) getpwent_r(pwbuf, buf, buflen, NULL);
    return getpwent_r(pwbuf, buf, buflen, pwbufp);
}
#endif

int nullPointer_getgrgid_r(gid_t gid, struct group *restrict grp, char *restrict buf, size_t buflen, struct group **restrict result)
{
    // cppcheck-suppress nullPointer
    (void) getgrgid_r(gid, NULL, buf, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getgrgid_r(gid, grp, NULL, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getgrgid_r(gid, grp, buf, buflen, NULL);
    return getgrgid_r(gid, grp, buf, buflen, result);
}

int nullPointer_getgrnam_r(const char *restrict name, struct group *restrict grp, char *restrict buf, size_t buflen, struct group **restrict result)
{
    // cppcheck-suppress nullPointer
    (void) getgrnam_r(NULL, grp, buf, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getgrnam_r(name, NULL, buf, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getgrnam_r(name, grp, NULL, buflen, result);
    // cppcheck-suppress nullPointer
    (void) getgrnam_r(name, grp, buf, buflen, NULL);
    return getgrnam_r(name, grp, buf, buflen, result);
}

void knownConditionTrueFalse_ffs(int i)
{
    // ffs() returns the position of the first bit set, or 0 if no bits are set in i.
    const int x = ffs(0);
    // cppcheck-suppress knownConditionTrueFalse
    if (x == 0) {} // always true
    // cppcheck-suppress knownConditionTrueFalse
    if (x == 1) {} // always false
    if (ffs(i) == 0) {}
}

ssize_t nullPointer_readlink(const char *path, char *buf, size_t bufsiz)
{
    // cppcheck-suppress nullPointer
    (void)readlink(NULL, buf, bufsiz);
    // cppcheck-suppress nullPointer
    (void)readlink(path, NULL, bufsiz);
    return readlink(path, buf, bufsiz);
}

int nullPointer_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
    // cppcheck-suppress nullPointer
    (void) readlinkat(dirfd, NULL, buf, bufsiz);
    // cppcheck-suppress nullPointer
    (void) readlinkat(dirfd, pathname, NULL, bufsiz);
    return readlinkat(dirfd, pathname, buf, bufsiz);
}

ssize_t nullPointer_recv(int sockfd, void *buf, size_t len, int flags)
{
    // cppcheck-suppress nullPointer
    (void) recv(sockfd, NULL, len, flags);
    return recv(sockfd, buf, len, flags);
}

ssize_t nullPointer_recvfrom(int sockfd, void *buf, size_t len, int flags,
                             struct sockaddr *src_addr, socklen_t *addrlen)
{
    // If src_addr is not NULL, and the underlying protocol provides the source address, this source address is filled in.
    (void) recvfrom(sockfd, buf, len, flags, NULL, addrlen);
    (void) recvfrom(sockfd, buf, len, flags, src_addr, NULL);
    (void) recvfrom(sockfd, buf, len, flags, NULL, NULL);
    // cppcheck-suppress nullPointer
    (void) recvfrom(sockfd, NULL, len, flags, src_addr, addrlen);
    return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}
int nullPointer_semop(int semid, struct sembuf *sops, size_t nsops)
{
    // cppcheck-suppress nullPointer
    (void)semop(semid, NULL, nsops);
    return semop(semid, sops, nsops);
}

int nullPointer_socketpair(int domain, int t, int protocol, int sv[2])
{
    // cppcheck-suppress nullPointer
    (void) socketpair(domain, t, protocol, NULL);
    return socketpair(domain, t, protocol, sv);
}

void nullPointer_lcong48(const unsigned short param[7])
{
    // cppcheck-suppress nullPointer
    (void) lcong48(NULL);
    return lcong48(param);
}

long int nullPointer_jrand48(unsigned short xsubi[3])
{
    // cppcheck-suppress nullPointer
    (void) jrand48(NULL);
    return jrand48(xsubi);
}

long int nullPointer_nrand48(unsigned short xsubi[3])
{
    // cppcheck-suppress nullPointer
    (void) nrand48(NULL);
    return nrand48(xsubi);
}

double nullPointer_erand48(unsigned short xsubi[3])
{
    // cppcheck-suppress nullPointer
    (void) erand48(NULL);
    return erand48(xsubi);
}

struct non_const_parameter_erand48_struct { unsigned short xsubi[3]; };
// No warning is expected that dat can be const
double non_const_parameter_erand48(struct non_const_parameter_erand48_struct *dat)
{
    return erand48(dat->xsubi);
}

unsigned short *nullPointer_seed48(const unsigned short seed16v[3])
{
    // cppcheck-suppress nullPointer
    (void) seed48(NULL);
    return seed48(seed16v);
}

int nullPointer_getlogin_r(char *buf, size_t bufsize)
{
    // cppcheck-suppress nullPointer
    (void)getlogin_r(NULL,bufsize);
    return getlogin_r(buf,bufsize);
}

ssize_t uninitvar_pread(int fd, void *buf, size_t nbyte, off_t offset)
{
    int Fd;
    // cppcheck-suppress uninitvar
    (void)pread(Fd,buf,nbyte,offset);
    size_t Nbyte;
    // cppcheck-suppress uninitvar
    (void)pread(fd,buf,Nbyte,offset);
    off_t Offset;
    // cppcheck-suppress uninitvar
    (void)pread(fd,buf,nbyte,Offset);
    return pread(fd,buf,nbyte,offset);
}

ssize_t nullPointer_pwrite(int fd, const void *buf, size_t nbyte, off_t offset)
{
    // cppcheck-suppress nullPointer
    (void)pwrite(fd,NULL,nbyte,offset);
    return pwrite(fd,buf,nbyte,offset);
}

int nullPointer_ttyname_r(int fd, char *buf, size_t buflen)
{
    // cppcheck-suppress nullPointer
    (void)ttyname_r(fd,NULL,buflen);
    return ttyname_r(fd,buf,buflen);
}

size_t bufferAccessOutOfBounds_wcsnrtombs(char *restrict dest, const wchar_t **restrict src, size_t nwc, size_t len, mbstate_t *restrict ps)
{
    char buf[42];
    (void)wcsnrtombs(buf,src,nwc,42,ps);
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)wcsnrtombs(buf,src,nwc,43,ps);
    return wcsnrtombs(dest,src,nwc,len,ps);
}

size_t nullPointer_wcsnrtombs(char *restrict dest, const wchar_t **restrict src, size_t nwc, size_t len, mbstate_t *restrict ps)
{
    // It is allowed to set the first arg to NULL
    (void)wcsnrtombs(NULL,src,nwc,len,ps);
    // cppcheck-suppress nullPointer
    (void)wcsnrtombs(dest,NULL,nwc,len,ps);
    // It is allowed to set the last arg to NULL
    (void)wcsnrtombs(dest,src,nwc,len,NULL);
    return wcsnrtombs(dest,src,nwc,len,ps);
}

int nullPointer_wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wcsncasecmp(NULL,s2,n);
    // cppcheck-suppress nullPointer
    (void)wcsncasecmp(s1,NULL,n);
    return wcsncasecmp(s1,s2,n);
}

int uninitvar_wcwidth(const wchar_t c)
{
    wchar_t wc;
    // cppcheck-suppress uninitvar
    (void)wcwidth(wc);
    // No warning is expected
    return wcwidth(c);
}

int nullPointer_wcsnlen(const wchar_t *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wcsnlen(NULL, n);
    // No warning is expected
    return wcsnlen(s, n);
}

size_t bufferAccessOutOfBounds_wcsnlen(void) // #10997
{
    const wchar_t buf[2]={L'4',L'2'};
    size_t len = wcsnlen(buf,2);
    // TODO cppcheck-suppress bufferAccessOutOfBounds
    len+=wcsnlen(buf,3);
    return len;
}

int nullPointer_gethostname(char *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)gethostname(NULL, n);
    // No warning is expected
    return gethostname(s, n);
}

int nullPointer_wcswidth(const wchar_t *s, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wcswidth(NULL, n);
    // No warning is expected
    return wcswidth(s, n);
}

int nullPointer_aio_cancel(int fd, struct aiocb *aiocbp)
{
    // No warning is expected
    (void)aio_cancel(fd, NULL);
    // No warning is expected
    return aio_cancel(fd, aiocbp);
}

int nullPointer_aio_fsync(int op, struct aiocb *aiocbp)
{
    // cppcheck-suppress nullPointer
    (void)aio_fsync(op, NULL);
    // No warning is expected
    return aio_fsync(op, aiocbp);
}

ssize_t nullPointer_aio_return(struct aiocb *aiocbp)
{
    // cppcheck-suppress nullPointer
    (void)aio_return(NULL);
    // No warning is expected
    return aio_return(aiocbp);
}

int nullPointer_aio_error(const struct aiocb *aiocbp)
{
    // cppcheck-suppress nullPointer
    (void)aio_error(NULL);
    // No warning is expected
    return aio_error(aiocbp);
}

int nullPointer_aio_read(struct aiocb *aiocbp)
{
    // cppcheck-suppress nullPointer
    (void)aio_read(NULL);
    // No warning is expected
    return aio_read(aiocbp);
}

int nullPointer_aio_write(struct aiocb *aiocbp)
{
    // cppcheck-suppress nullPointer
    (void)aio_write(NULL);
    // No warning is expected
    return aio_write(aiocbp);
}

int nullPointer_aio_suspend(const struct aiocb *const aiocb_list[], int nitems, const struct timespec *restrict timeout)
{
    // cppcheck-suppress nullPointer
    (void)aio_suspend(NULL, nitems, timeout);
    // No warning is expected
    return aio_suspend(aiocb_list, nitems, timeout);
}

#if !defined(__linux__) && !defined(__APPLE__)
// Note: Since glibc 2.28, this function symbol is no longer available to newly linked applications.
void invalidFunctionArg_llseek(int fd, loff_t offset, int origin)
{
    // cppcheck-suppress llseekCalled
    // cppcheck-suppress invalidFunctionArg
    (void)llseek(-1, offset, SEEK_SET);
    // cppcheck-suppress llseekCalled
    // cppcheck-suppress invalidFunctionArg
    (void)llseek(fd, offset, -1);
    // cppcheck-suppress llseekCalled
    // cppcheck-suppress invalidFunctionArg
    (void)llseek(fd, offset, 3);
    // cppcheck-suppress llseekCalled
    // cppcheck-suppress invalidFunctionArg
    (void)llseek(fd, offset, 42+SEEK_SET);
    // cppcheck-suppress llseekCalled
    // cppcheck-suppress invalidFunctionArg
    (void)llseek(fd, offset, SEEK_SET+42);
    // No invalidFunctionArg warning is expected for
    // cppcheck-suppress llseekCalled
    (void)llseek(0, offset, origin);
    // cppcheck-suppress llseekCalled
    (void)llseek(fd, offset, origin);
    // cppcheck-suppress llseekCalled
    (void)llseek(fd, offset, SEEK_SET);
    // cppcheck-suppress llseekCalled
    (void)llseek(fd, offset, SEEK_CUR);
    // cppcheck-suppress llseekCalled
    (void)llseek(fd, offset, SEEK_END);
}

void invalidFunctionArg_lseek64(int fd, off_t offset, int origin)
{
    // cppcheck-suppress invalidFunctionArg
    (void)lseek64(-1, offset, SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek64(fd, offset, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek64(fd, offset, 3);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek64(fd, offset, 42+SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek64(fd, offset, SEEK_SET+42);
    // No warning is expected for
    (void)lseek64(0, offset, origin);
    (void)lseek64(fd, offset, origin);
    (void)lseek64(fd, offset, SEEK_SET);
    (void)lseek64(fd, offset, SEEK_CUR);
    (void)lseek64(fd, offset, SEEK_END);
}
#endif

void invalidFunctionArg_lseek(int fd, off_t offset, int origin)
{
    // cppcheck-suppress invalidFunctionArg
    (void)lseek(-1, offset, SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek(fd, offset, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek(fd, offset, 3);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek(fd, offset, 42+SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)lseek(fd, offset, SEEK_SET+42);
    // No warning is expected for
    (void)lseek(0, offset, origin);
    (void)lseek(fd, offset, origin);
    (void)lseek(fd, offset, SEEK_SET);
    (void)lseek(fd, offset, SEEK_CUR);
    (void)lseek(fd, offset, SEEK_END);
}

void invalidFunctionArg_fseeko(FILE* stream, off_t offset, int origin)
{
    // cppcheck-suppress invalidFunctionArg
    (void)fseeko(stream, offset, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)fseeko(stream, offset, 3);
    // cppcheck-suppress invalidFunctionArg
    (void)fseeko(stream, offset, 42+SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)fseeko(stream, offset, SEEK_SET+42);
    // No warning is expected for
    (void)fseeko(stream, offset, origin);
    (void)fseeko(stream, offset, SEEK_SET);
    (void)fseeko(stream, offset, SEEK_CUR);
    (void)fseeko(stream, offset, SEEK_END);
}

wchar_t *nullPointer_wcpncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
    // cppcheck-suppress nullPointer
    (void)wcpncpy(NULL, src, n);
    // cppcheck-suppress nullPointer
    (void)wcpncpy(dest, NULL, n);
    return wcpncpy(dest, src, n);
}

int nullPointer_utimes(const char *path, const struct timeval times[2])
{
    // cppcheck-suppress nullPointer
    // cppcheck-suppress utimesCalled
    (void)utimes(NULL, times);
    // cppcheck-suppress utimesCalled
    return utimes(path, times);
}

char * overlappingWriteFunction_stpcpy(char *src, char *dest)
{
    // No warning shall be shown:
    (void) stpcpy(dest, src);
    // cppcheck-suppress overlappingWriteFunction
    return stpcpy(src, src);
}

int nullPointer_strcasecmp(const char *a, const char *b)
{
    // No warning shall be shown:
    (void) strcasecmp(a, b);
    // cppcheck-suppress nullPointer
    (void) strcasecmp(a, NULL);
    // cppcheck-suppress nullPointer
    return strcasecmp(NULL, b);
}

int nullPointer_strncasecmp(const char *a, const char *b, size_t n)
{
    // No warning shall be shown:
    (void) strncasecmp(a, b, n);
    // cppcheck-suppress nullPointer
    (void) strncasecmp(a, NULL, n);
    // cppcheck-suppress nullPointer
    return strncasecmp(NULL, b, n);
}

int nullPointer_bcmp(const void *a, const void *b, size_t n)
{
    // No nullPointer warning shall be shown:
    // cppcheck-suppress bcmpCalled
    (void) bcmp(a, b, n);
    // cppcheck-suppress nullPointer
    // cppcheck-suppress bcmpCalled
    (void) bcmp(a, NULL, n);
    // cppcheck-suppress nullPointer
    // cppcheck-suppress bcmpCalled
    return bcmp(NULL, b, n);
}

void nullPointer_bzero(void *s, size_t n)
{
    // cppcheck-suppress nullPointer
    // cppcheck-suppress bzeroCalled
    bzero(NULL,n);
    // No nullPointer-warning shall be shown:
    // cppcheck-suppress bzeroCalled
    bzero(s,n);
}

void bufferAccessOutOfBounds_bzero(void *s, size_t n)
{
    char buf[42];
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress bzeroCalled
    bzero(buf,43);
    // cppcheck-suppress bzeroCalled
    bzero(buf,42);
    // No nullPointer-warning shall be shown:
    // cppcheck-suppress bzeroCalled
    bzero(s,n);
}

size_t bufferAccessOutOfBounds_strnlen(const char *s, size_t maxlen)
{
    const char buf[2]={'4','2'};
    size_t len = strnlen(buf,2);
    // cppcheck-suppress bufferAccessOutOfBounds
    len+=strnlen(buf,3);
    return len;
}

void bufferAccessOutOfBounds_wcpncpy()
{
    wchar_t s[16];
    wcpncpy(s, L"abc", 16);
    // cppcheck-suppress bufferAccessOutOfBounds
    wcpncpy(s, L"abc", 17);
}

size_t nullPointer_strnlen(const char *s, size_t maxlen)
{
    // No warning shall be shown:
    (void) strnlen(s, maxlen);
    // cppcheck-suppress nullPointer
    return strnlen(NULL, maxlen);
}

char * nullPointer_stpcpy(const char *src, char *dest)
{
    // No warning shall be shown:
    (void) stpcpy(dest, src);
    // cppcheck-suppress nullPointer
    (void) stpcpy(dest, NULL);
    // cppcheck-suppress nullPointer
    return stpcpy(NULL, src);
}

char * nullPointer_strsep(char **stringptr, char *delim)
{
    // No warning shall be shown:
    (void) strsep(stringptr, delim);
    // cppcheck-suppress nullPointer
    (void) strsep(stringptr, NULL);
    // cppcheck-suppress nullPointer
    return strsep(NULL, delim);
}

void overlappingWriteFunction_bcopy(char *buf, const size_t count)
{
    // No warning shall be shown:
    // cppcheck-suppress bcopyCalled
    bcopy(&buf[0], &buf[3], count); // size is not known
    // cppcheck-suppress bcopyCalled
    bcopy(&buf[0], &buf[3], 3U);    // no-overlap
    // cppcheck-suppress bcopyCalled
    bcopy(&buf[0], &buf[3], 4U);    // The result is correct, even when both areas overlap.
}

void nullPointer_bcopy(const void *src, void *dest, size_t n)
{
    // No warning shall be shown:
    // cppcheck-suppress bcopyCalled
    bcopy(src, dest, n);
    // cppcheck-suppress bcopyCalled
    // cppcheck-suppress nullPointer
    bcopy(NULL, dest, n);
    // cppcheck-suppress bcopyCalled
    // cppcheck-suppress nullPointer
    bcopy(src, NULL, n);
}

void overlappingWriteFunction_memccpy(const unsigned char *src, unsigned char *dest, int c, size_t count)
{
    // No warning shall be shown:
    (void)memccpy(dest, src, c, count);
    (void)memccpy(dest, src, 42, count);
    // cppcheck-suppress overlappingWriteFunction
    (void)memccpy(dest, dest, c, 4);
    // cppcheck-suppress overlappingWriteFunction
    (void)memccpy(dest, dest+3, c, 4);
}

void overlappingWriteFunction_stpncpy(char *src, char *dest, ssize_t n)
{
    // No warning shall be shown:
    (void) stpncpy(dest, src, n);
    // cppcheck-suppress overlappingWriteFunction
    (void)stpncpy(src, src+3, 4);
}

wchar_t* overlappingWriteFunction_wcpncpy(wchar_t *src, wchar_t *dest, ssize_t n)
{
    // No warning shall be shown:
    (void) wcpncpy(dest, src, n);
    // cppcheck-suppress overlappingWriteFunction
    return wcpncpy(src, src+3, 4);
}

void overlappingWriteFunction_swab(char *src, char *dest, ssize_t n)
{
    // No warning shall be shown:
    swab(src, dest, n);
    // cppcheck-suppress overlappingWriteFunction
    swab(src, src+3, 4);
}

void bufferAccessOutOfBounds_swab(char *src, char *dest, ssize_t n)
{
    // No warning shall be shown:
    swab(dest, src, n);
    const char srcBuf[42] = {0};
    char destBuf[42] = {0};
    swab(srcBuf, dest, 42);
    // cppcheck-suppress bufferAccessOutOfBounds
    swab(srcBuf, dest, 43);
    swab(src, destBuf, 42);
    // cppcheck-suppress bufferAccessOutOfBounds
    swab(src, destBuf, 43);
}

void nullPointer_swab(char *src, char *dest, ssize_t n)
{
    // No warning shall be shown:
    swab(dest, src, n);
    // cppcheck-suppress nullPointer
    swab(NULL, dest, n);
    // cppcheck-suppress nullPointer
    swab(src, NULL, n);
}

bool invalidFunctionArgBool_isascii(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isascii(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isascii(c != 0);
}

void uninitvar_putenv(char * envstr)
{
    // No warning is expected
    (void)putenv(envstr);

    char * p;
    // cppcheck-suppress uninitvar
    (void)putenv(p);
}

void nullPointer_putenv(char * envstr)
{
    // No warning is expected
    (void)putenv(envstr);

    char * p=NULL;
    // cppcheck-suppress nullPointer
    (void)putenv(p);
}

void memleak_scandir(void)
{
    struct dirent **namelist;
    int n = scandir(".", &namelist, NULL, alphasort);
    if (n == -1) {
        return;
    }

    // http://man7.org/linux/man-pages/man3/scandir.3.html
    /* The scandir() function scans the directory dirp, calling filter() on
       each directory entry.  Entries for which filter() returns nonzero are
       stored in strings allocated via malloc(3), sorted using qsort(3) with
       the comparison function compar(), and collected in array namelist
       which is allocated via malloc(3).  If filter is NULL, all entries are
       selected.*/

    // cppcheck-suppress memleak
}

void no_memleak_scandir(void)
{
    struct dirent **namelist;
    int n = scandir(".", &namelist, NULL, alphasort);
    if (n == -1) {
        return;
    }
    while (n--) {
        free(namelist[n]);
    }
    free(namelist);
}

void validCode(va_list valist_arg1, va_list valist_arg2)
{
    void *ptr;
    if (posix_memalign(&ptr, sizeof(void *), sizeof(void *)) == 0)
        free(ptr);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    syslog(LOG_ERR, "err %u", 0U);
    syslog(LOG_WARNING, "warn %d %d", 5, 1);
    vsyslog(LOG_EMERG, "emerg %d", valist_arg1);
    vsyslog(LOG_INFO, "test %s %d %p", valist_arg2);

    void* handle = dlopen("/lib.so", RTLD_NOW);
    if (handle) {
        dlclose(handle);
    }
}

typedef struct {
    size_t N;
    int* data;
} S_memalign;

S_memalign* posix_memalign_memleak(size_t n) { // #12248
    S_memalign* s = malloc(sizeof(*s));
    // cppcheck-suppress nullPointerOutOfMemory
    s->N = n;
    if (0 != posix_memalign((void**)&s->data, 16, n * sizeof(int))) {
        free(s);
        return NULL;
    }
    // cppcheck-suppress nullPointerOutOfMemory
    memset(s->data, 0, n * sizeof(int));
    return s;
}

ssize_t nullPointer_send(int socket, const void *buf, size_t len, int flags)
{
    // cppcheck-suppress nullPointer
    (void) send(socket, NULL, len, flags);
    return send(socket, buf, len, flags);
}

ssize_t nullPointer_sendto(int socket, const void *message, size_t length,
                           int flags, const struct sockaddr *dest_addr,
                           socklen_t dest_len)
{
    // cppcheck-suppress nullPointer
    (void) sendto(socket, NULL, length, flags, dest_addr, dest_len);
    (void) sendto(socket, message, length, flags, NULL, dest_len);
    return sendto(socket, message, length, flags, dest_addr, dest_len);
}

void bufferAccessOutOfBounds(int fd)
{
    char a[5];
    read(fd,a,5);
    // cppcheck-suppress bufferAccessOutOfBounds
    read(fd,a,6);
    write(fd,a,5);
    // cppcheck-suppress bufferAccessOutOfBounds
    write(fd,a,6);
    recv(fd,a,5,0);
    // cppcheck-suppress bufferAccessOutOfBounds
    recv(fd,a,6,0);
    recvfrom(fd,a,5,0,0x0,0x0);
    // cppcheck-suppress bufferAccessOutOfBounds
    recvfrom(fd,a,6,0,0x0,0x0);
    send(fd,a,5,0);
    // cppcheck-suppress bufferAccessOutOfBounds
    send(fd,a,6,0);
    sendto(fd,a,5,0,0x0,0x0);
    // cppcheck-suppress bufferAccessOutOfBounds
    sendto(fd,a,6,0,0x0,0x0);
    // cppcheck-suppress constStatement
    0;
    readlink("path", a, 5);
    // cppcheck-suppress bufferAccessOutOfBounds
    readlink("path", a, 6);
    readlinkat(1, "path", a, 5);
    // cppcheck-suppress bufferAccessOutOfBounds
    readlinkat(1, "path", a, 6);
    // This is valid
    gethostname(a, 5);
    // cppcheck-suppress bufferAccessOutOfBounds
    gethostname(a, 6);
}

void nullPointer(char *p, int fd, pthread_mutex_t mutex)
{
    // cppcheck-suppress ignoredReturnValue
    isatty(0);
    mkdir(p, 0);
    getcwd(0, 0);
    // cppcheck-suppress nullPointer
    readdir(0);
    // cppcheck-suppress nullPointer
    // cppcheck-suppress utimeCalled
    utime(NULL, NULL);
    // not implemented yet: cppcheck-suppress nullPointer
    read(fd,NULL,42);
    read(fd,NULL,0);
    // not implemented yet: cppcheck-suppress nullPointer
    write(fd,NULL,42);
    write(fd,NULL,0);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    open(NULL, 0);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    open(NULL, 0, 0);
    // cppcheck-suppress unreadVariable
    // cppcheck-suppress nullPointer
    int ret = access(NULL, 0);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    fdopen(fd, NULL);
    // cppcheck-suppress strtokCalled
    // cppcheck-suppress nullPointer
    strtok(p, NULL);

    // cppcheck-suppress nullPointer
    pthread_mutex_init(NULL, NULL);
    // Second argument can be NULL
    pthread_mutex_init(&mutex, NULL);
    // cppcheck-suppress nullPointer
    pthread_mutex_destroy(NULL);
    // cppcheck-suppress nullPointer
    pthread_mutex_lock(NULL);
    // cppcheck-suppress nullPointer
    (void)pthread_mutex_trylock(NULL);
    // cppcheck-suppress nullPointer
    pthread_mutex_unlock(NULL);
}

// cppcheck-suppress constParameterCallback
void* f_returns_NULL(void* arg)
{
    return NULL;
}

void nullPointer_pthread_create() // #12396
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void* (*)(void*))f_returns_NULL, NULL);
}

void memleak_getaddrinfo() // #6994
{
    struct addrinfo * res=NULL;
    getaddrinfo("node", NULL, NULL, &res);
    freeaddrinfo(res);
    getaddrinfo("node", NULL, NULL, &res);
    // cppcheck-suppress memleak
}

void memleak_getaddrinfo_if() // #12506
{
    struct addrinfo hints = {};
    struct addrinfo* addrs;
    int err = getaddrinfo("example.com", "https", &hints, &addrs);
    if (err != 0) {}
    else {
        freeaddrinfo(addrs);
    }
}

void memleak_getaddrinfo_if2() // #12996
{
    struct addrinfo *addrs = NULL;
    if (getaddrinfo("a", "b", NULL, &addrs)) {
        return;
    }
    freeaddrinfo(addrs);
}

void memleak_mmap(int fd)
{
    // cppcheck-suppress [unusedAllocatedMemory, unreadVariable, constVariablePointer]
    void *addr = mmap(NULL, 255, PROT_NONE, MAP_PRIVATE, fd, 0);
    // cppcheck-suppress memleak
}

void * memleak_mmap2() // #8327
{
    void * data = mmap(NULL, 10, PROT_READ, MAP_PRIVATE, 1, 0);
    if (data != MAP_FAILED)
        return data;
    return NULL;
}

void memleak_getline() { // #11043
    char *line = NULL;
    size_t size = 0;
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    getline(&line, &size, stdin);
    // cppcheck-suppress memleak
    line = NULL;
    getline(&line, &size, stdin);
    // cppcheck-suppress memleak
    line = NULL;
}

void memleak_getline_array(FILE* stream) { // #12498
    char* a[2] = { 0 };
    size_t n;
    getline(&a[0], &n, stream);
    getline(&a[1], &n, stream);
    free(a[0]);
    free(a[1]);
}

void memleak_getdelim(int delim) {
    char *line = NULL;
    size_t size = 0;
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    getdelim(&line, &size, delim, stdin);
    // cppcheck-suppress memleak
    line = NULL;
    getdelim(&line, &size, delim, stdin);
    // cppcheck-suppress memleak
    line = NULL;
}

void memleak_getdelim_array(FILE* stream, int delim) {
    char* a[2] = { 0 };
    size_t n;
    getdelim(&a[0], &n, delim, stream);
    getdelim(&a[1], &n, delim, stream);
    free(a[0]);
    free(a[1]);
}

void * identicalCondition_mmap(int fd, size_t size) // #9940
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    void* buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        return NULL;
    }
    return buffer;
}

int munmap_no_double_free(int tofd, // #11396
                          int fromfd,
                          size_t len)
{
    int rc;
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    const void* fptr = mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,fromfd,(off_t)0);
    if (fptr == MAP_FAILED) {
        return -1;
    }

    void* tptr = mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,tofd,(off_t)0);
    if (tptr == MAP_FAILED) {
        // cppcheck-suppress memleak
        return -1;
    }

    // cppcheck-suppress nullPointerOutOfMemory
    memcpy(tptr,fptr,len);

    // cppcheck-suppress nullPointerOutOfMemory
    if ((rc = munmap(fptr,len)) != 0) {
        // cppcheck-suppress memleak
        return -1;
    }

    // cppcheck-suppress nullPointerOutOfMemory
    if ((rc = munmap(tptr,len)) != 0) {
        return -1;
    }

    return rc;
}

void resourceLeak_fdopen(int fd)
{
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    FILE *f = fdopen(fd, "r");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_fdopen2(const char* fn) // #2767
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    int fi = open(fn, O_RDONLY);
    FILE* fd = fdopen(fi, "r");
    // cppcheck-suppress nullPointerOutOfResources
    fclose(fd);
}

void resourceLeak_mkstemp(char *template)
{
    // cppcheck-suppress unreadVariable
    int fp = mkstemp(template);
    // cppcheck-suppress resourceLeak
}

void no_resourceLeak_mkstemp_01(char *template)
{
    int fp = mkstemp(template);
    close(fp);
}

int no_resourceLeak_mkstemp_02(char *template)
{
    return mkstemp(template);
}

void resourceLeak_fdopendir(int fd)
{
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    DIR* leak1 = fdopendir(fd);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_opendir(void)
{
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    DIR* leak1 = opendir("abc");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_socket(void)
{
    // cppcheck-suppress unreadVariable
    int s = socket(AF_INET, SOCK_STREAM, 0);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_open1(void)
{
    // cppcheck-suppress [unreadVariable,valueFlowBailoutIncompleteVar]
    int fd = open("file", O_RDWR | O_CREAT);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_open2(void)
{
    // cppcheck-suppress [unreadVariable,valueFlowBailoutIncompleteVar]
    int fd = open("file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    // cppcheck-suppress resourceLeak
}

void noleak(int x, int y, int z)
{
    DIR *p1 = fdopendir(x);
    // cppcheck-suppress nullPointerOutOfResources
    closedir(p1);
    DIR *p2 = opendir("abc");
    // cppcheck-suppress nullPointerOutOfResources
    closedir(p2);
    int s = socket(AF_INET,SOCK_STREAM,0);
    close(s);
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    int fd1 = open("a", O_RDWR | O_CREAT);
    close(fd1);
    int fd2 = open("a", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    close(fd2);
}


// unused return value

void ignoredReturnValue(const void *addr, int fd)
{
    // cppcheck-suppress leakReturnValNotUsed
    mmap(addr, 255, PROT_NONE, MAP_PRIVATE, fd, 0);
    // cppcheck-suppress ignoredReturnValue
    getuid();
    // cppcheck-suppress ignoredReturnValue
    access("filename", 1);
    // no ignoredReturnValue shall be shown for
    setuid(42);
}


// valid range

void invalidFunctionArg()
{
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress usleepCalled
    usleep(-1);
    // cppcheck-suppress usleepCalled
    usleep(0);
    // cppcheck-suppress usleepCalled
    usleep(999999);
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress usleepCalled
    usleep(1000000);
}

void invalidFunctionArg_close(int fd)
{
    if (fd < 0) {
        // cppcheck-suppress invalidFunctionArg
        (void)close(fd);
    }
}

void uninitvar(int fd)
{
    int x1, x2, x3, x4;
    char buf[2];
    int decimal, sign;
    double d;
    const void *p;
    pthread_mutex_t mutex, mutex1, mutex2, mutex3;
    // cppcheck-suppress uninitvar
    write(x1,"ab",2);
    // cppcheck-suppress uninitvar
    write(fd,buf,2); // #6325
    // cppcheck-suppress uninitvar
    write(fd,"ab",x2);
    // cppcheck-suppress uninitvar
    write(fd,p,2);


    /* int regcomp(regex_t *restrict preg, const char *restrict pattern, int cflags); */
    regex_t reg;
    const char * pattern;
    int cflags1, cflags2;
    // cppcheck-suppress uninitvar
    regcomp(&reg, pattern, cflags1);
    pattern="";
    // cppcheck-suppress uninitvar
    regcomp(&reg, pattern, cflags2);
    regerror(0, &reg, 0, 0);
#ifndef __CYGWIN__
    // cppcheck-suppress [uninitvar, unreadVariable, ecvtCalled, constVariablePointer]
    char *buffer = ecvt(d, 11, &decimal, &sign);
#endif
    // cppcheck-suppress gcvtCalled
    gcvt(3.141, 2, buf);

    const char *filename1, *filename2;
    const struct utimbuf *times;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress utimeCalled
    utime(filename1, times);
    // cppcheck-suppress constVariable
    struct utimbuf times1[2];
    // cppcheck-suppress uninitvar
    // cppcheck-suppress utimeCalled
    utime(filename2, times1);

    // cppcheck-suppress unreadVariable
    // cppcheck-suppress uninitvar
    int access_ret = access("file", x3);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    fdopen(x4, "rw");

    char *strtok_arg1;
    // cppcheck-suppress strtokCalled
    // cppcheck-suppress uninitvar
    strtok(strtok_arg1, ";");

    // cppcheck-suppress uninitvar
    pthread_mutex_lock(&mutex1);
    // cppcheck-suppress uninitvar
    (void)pthread_mutex_trylock(&mutex2);
    // cppcheck-suppress uninitvar
    pthread_mutex_unlock(&mutex3);
    // after initialization it must be OK to call lock, trylock and unlock for this mutex
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    (void)pthread_mutex_trylock(&mutex);
    pthread_mutex_unlock(&mutex);
}

void uninitvar_getcwd(void)
{
    char *buf;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)getcwd(buf,size);
}


void uninitvar_types(void)
{
    // cppcheck-suppress unassignedVariable
    blkcnt_t b;
    // cppcheck-suppress [uninitvar,constStatement]
    b + 1;

    struct dirent d;
    // cppcheck-suppress [uninitvar,constStatement]
    d.d_ino + 1;
}

void timet_h(const struct timespec* ptp1)
{
    clockid_t clk_id1, clk_id2, clk_id3;
    // cppcheck-suppress constVariablePointer
    struct timespec* ptp;
    // cppcheck-suppress [uninitvar,valueFlowBailoutIncompleteVar]
    clock_settime(CLOCK_REALTIME, ptp);
    // cppcheck-suppress uninitvar
    clock_settime(clk_id1, ptp);
    // cppcheck-suppress uninitvar
    clock_settime(clk_id2, ptp1);

    struct timespec tp;
    // cppcheck-suppress uninitvar
    clock_settime(CLOCK_REALTIME, &tp); // #6577 - false negative
    // cppcheck-suppress uninitvar
    clock_settime(clk_id3, &tp);

    time_t clock = time(0);
    char buf[26];
    // cppcheck-suppress ctime_rCalled
    ctime_r(&clock, buf);
}

void dl(const char* libname, const char* func)
{
    void* lib = dlopen(libname, RTLD_NOW);
    // cppcheck-suppress redundantInitialization
    // cppcheck-suppress resourceLeak
    lib = dlopen(libname, RTLD_LAZY);
    const char* funcname;
    // cppcheck-suppress [uninitvar, unreadVariable, constVariablePointer]
    void* sym = dlsym(lib, funcname);
    // cppcheck-suppress ignoredReturnValue
    dlsym(lib, "foo");
    // cppcheck-suppress unassignedVariable
    void* uninit;
    // cppcheck-suppress uninitvar
    dlclose(uninit);
    // cppcheck-suppress resourceLeak
}

void asctime_r_test(const struct tm * tm, char * bufSizeUnknown)
{
    struct tm tm_uninit_data;
    const struct tm * tm_uninit_pointer;
    char bufSize5[5];
    char bufSize25[25];
    char bufSize26[26];
    char bufSize100[100];

    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    asctime_r(tm, bufSize5);
    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    asctime_r(tm, bufSize25);
    // cppcheck-suppress asctime_rCalled
    asctime_r(tm, bufSize26);
    // cppcheck-suppress asctime_rCalled
    asctime_r(tm, bufSize100);

    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress uninitvar
    asctime_r(&tm_uninit_data, bufSize100);
    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress uninitvar
    asctime_r(tm_uninit_pointer, bufSize100);

    // cppcheck-suppress asctime_rCalled
    asctime_r(tm, bufSizeUnknown);
}

void ctime_r_test(const time_t * timep, char * bufSizeUnknown)
{
    time_t time_t_uninit_data;
    const time_t * time_t_uninit_pointer;
    char bufSize5[5];
    char bufSize25[25];
    char bufSize26[26];
    char bufSize100[100];

    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    ctime_r(timep, bufSize5);
    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    ctime_r(timep, bufSize25);
    // cppcheck-suppress ctime_rCalled
    ctime_r(timep, bufSize26);
    // cppcheck-suppress ctime_rCalled
    ctime_r(timep, bufSize100);

    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress uninitvar
    ctime_r(&time_t_uninit_data, bufSize100);
    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress uninitvar
    ctime_r(time_t_uninit_pointer, bufSize100);

    // cppcheck-suppress ctime_rCalled
    ctime_r(timep, bufSizeUnknown);
}

void invalidFunctionArg_nice(int inc)
{
    // cppcheck-suppress invalidFunctionArg
    nice(-21);
    nice(-20);
    nice(19);
    // cppcheck-suppress invalidFunctionArg
    nice(+20);
}
