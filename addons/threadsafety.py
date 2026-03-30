#!/usr/bin/env python3
"""
cppcheck addon for threadsafety detection.

 This script analyses Cppcheck dump files to locate threadsafety issues.
 It warns about
 - static local objects
 - MT-Unsafe symbols listed in the "Attributes" sections of man pages.

"""

import re
import sys

import cppcheckdata

# --------------------------------
# List of MT-Unsafe identifiers
# --------------------------------

# This is Work In Progress.
# Eventually it should contain all identifiers (types
# and functions) which are MT-Unsafe.

# The script tools/MT-Unsafe.py can help to re-generate this list.
# It reads a man-page tree and report identifiers marked as "MT-Unsafe"
# (see man 7 attributes for what this means), eg
#   MT-Unsafe.py /usr/share/man/man3

id_MTunsafe_full = {
    # MT-Unsafe types by definition
    # 'pthread_t',
    # Types marked MT-Unsafe
    'const:env',
    'const:hostid',
    'const:mallopt',
    'const:sigintr',
    'race:LogMask',
    'race:asctime',
    'race:crypt',
    'race:crypt_gensalt',
    'race:cuserid/!string',
    'race:dirstream',
    'race:drand48',
    'race:ecvt',
    'race:exit',
    'race:fcvt',
    'race:fgetgrent',
    'race:fgetpwent',
    'race:fgetspent',
    'race:fsent',
    'race:getdate',
    'race:getlogin',
    'race:getopt',
    'race:getspent',
    'race:getspnam',
    'race:grent',
    'race:grgid',
    'race:grnam',
    'race:hostbyaddr',
    'race:hostbyname',
    'race:hostbyname2',
    'race:hostent',
    'race:hsearch',
    'race:l64a',
    'race:localeconv',
    'race:mbrlen/!ps',
    'race:mbrtowc/!ps',
    'race:mbsnrtowcs/!ps',
    'race:mbsrtowcs/!ps',
    'race:mcheck',
    'race:mntentbuf',
    'race:netbyaddr',
    'race:netbyname',
    'race:netent',
    'race:netgrent',
    'race:protobyname',
    'race:protobynumber',
    'race:protoent',
    'race:ptsname',
    'race:pwent',
    'race:pwnam',
    'race:pwuid',
    'race:qecvt',
    'race:qfcvt',
    'race:servbyname',
    'race:servbyport',
    'race:servent',
    'race:sgetspent',
    'race:signgam',
    'race:stdin',
    'race:stdout',
    'race:streams',
    'race:strerror',
    'race:strsignal',
    'race:strtok',
    'race:tmbuf',
    'race:tmpnam/!s',
    'race:ttyent',
    'race:ttyname',
    'race:utent',
    'race:wcrtomb/!ps',
    'race:wcsnrtombs/!ps',
    'race:wcsrtombs/!ps',
    'sig:ALRM',
    'sig:SIGCHLD/linux',
    # APIs marked MT-Unsafe
    'asctime',
    'clearenv',
    'ctime',
    'cuserid',
    'drand48',
    'ecvt',
    'encrypt',
    'endfsent',
    'endgrent',
    'endhostent',
    'endnetent',
    'endnetgrent',
    'endprotoent',
    'endpwent',
    'endservent',
    'endspent',
    'endttyent',
    'endusershell',
    'endutent',
    'erand48',
    'error_at_line',
    'ether_aton',
    'ether_ntoa',
    'exit',
    'fcloseall',
    'fcvt',
    'fgetgrent',
    'fgetpwent',
    'fgetspent',
    'fts_children',
    'fts_read',
    'gamma',
    'gammaf',
    'gammal',
    'getaliasbyname',
    'getaliasent',
    'getchar_unlocked',
    'getdate',
    'getfsent',
    'getfsfile',
    'getfsspec',
    'getgrent',
    'getgrent_r',
    'getgrgid',
    'getgrnam',
    'gethostbyaddr',
    'gethostbyname',
    'gethostbyname2',
    'gethostent',
    'gethostent_r',
    'getlogin',
    'getlogin_r',
    'getmntent',
    'getnetbyaddr',
    'getnetbyname',
    'getnetent',
    'getnetgrent',
    'getnetgrent_r',
    'getopt',
    'getopt_long',
    'getopt_long_only',
    'getpass',
    'getprotobyname',
    'getprotobynumber',
    'getprotoent',
    'getpwent',
    'getpwent_r',
    'getpwnam',
    'getpwuid',
    'getrpcbyname',
    'getrpcbynumber',
    'getrpcent',
    'getservbyname',
    'getservbyport',
    'getservent',
    'getspent',
    'getspent_r',
    'getspnam',
    'getttyent',
    'getttynam',
    'getusershell',
    'getutent',
    'getutid',
    'getutline',
    'getwchar_unlocked',
    'glob',
    'gmtime',
    'hcreate',
    'hdestroy',
    'hsearch',
    'innetgr',
    'jrand48',
    'l64a',
    'lcong48',
    'localeconv',
    'localtime',
    'login',
    'login_tty',
    'logout',
    'logwtmp',
    'lrand48',
    'mallinfo',
    'mallinfo2',
    'mblen',
    'mbrlen',
    'mbrtowc',
    'mbsnrtowcs',
    'mbsrtowcs',
    'mbtowc',
    'mcheck',
    'mcheck_check_all',
    'mcheck_pedantic',
    'mprobe',
    'mrand48',
    'mtrace',
    'muntrace',
    'nrand48',
    'profil',
    'ptsname',
    'putchar_unlocked',
    'putenv',
    'pututline',
    'putwchar_unlocked',
    'pvalloc',
    'qecvt',
    'qfcvt',
    'rcmd',
    'rcmd_af',
    're_comp',
    're_exec',
    'readdir',
    'rexec',
    'rexec_af',
    'seed48',
    'setenv',
    'setfsent',
    'setgrent',
    'sethostent',
    'sethostid',
    'setkey',
    'setlogmask',
    'setnetent',
    'setnetgrent',
    'setprotoent',
    'setpwent',
    'setservent',
    'setspent',
    'setttyent',
    'setusershell',
    'setutent',
    'sgetspent',
    'siginterrupt',
    'sleep',
    'srand48',
    'strerror',
    'strsignal',
    'strtok',
    'tmpnam',
    'ttyname',
    'ttyslot',
    'unsetenv',
    'updwtmp',
    'utmpname',
    'valloc',
    'wcrtomb',
    'wcsnrtombs',
    'wcsrtombs',
    'wctomb',
    'wordexp'
}

# From man 7 attributes
# the full token could be feature:function/condition - we just want function.
id_MTunsafe = [re.sub('^.*:', '', re.sub('/.*$', '', x))
               for x in id_MTunsafe_full
               ]


def reportError(token, severity, msg, errid):    # noqa: D103
    cppcheckdata.reportError(token, severity, msg, 'threadsafety', errid)


def checkstatic(data):    # noqa: D103
    for var in data.variables:
        if var.isStatic and var.isLocal:
            vartype = None
            if var.isClass:
                vartype = 'object'
            else:
                vartype = 'variable'
            if var.isConst:
                if data.standards.cpp == 'c++03':
                    reportError(
                        var.typeStartToken,
                        'warning',
                        'Local constant static '
                        + vartype + "'" + var.nameToken.str
                        + "', dangerous if it is initialized"
                        + ' in parallel threads',
                        'threadsafety-const')
            else:
                reportError(var.typeStartToken, 'warning',
                            'Local static ' + vartype + ': '
                            + var.nameToken.str,
                            'threadsafety')


def check_MTunsafe(cfg):
    """
    Look for functions marked MT-unsafe in their man pages.

    The MT-unsafe functions are listed in id_MTunsafe (and id_MTunsafe_full).
    That section of code can be regenerated by the external script MT-Unsafe.py
    """
    for token in cfg.tokenlist:
        if token.str in id_MTunsafe:
            reportError(token, 'warning', token.str + ' is MT-unsafe',
                        'unsafe-call')


if __name__ == '__main__':
    parser = cppcheckdata.ArgumentParser()
    args = parser.parse_args()

    quiet = args.quiet or args.cli

    if not args.dumpfile:
        if not args.quiet:
            print('no input files.')
        sys.exit(0)

    for dumpfile in args.dumpfile:
        # load XML from .dump file
        data = cppcheckdata.CppcheckData(dumpfile)

        for cfg in data.iterconfigurations():
            if not args.quiet:
                srcfile = data.files[0]
                print('Checking {}, config {}...'.format(srcfile, cfg.name))
            check_MTunsafe(cfg)
            checkstatic(cfg)

    sys.exit(cppcheckdata.EXIT_CODE)
