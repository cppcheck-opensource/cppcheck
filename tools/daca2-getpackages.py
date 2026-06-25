#!/usr/bin/env python3
#
# Get list of debian packages
#
# Usage:
#
# cd cppcheck/tools
# ./daca2-getpackages.py
#


import subprocess
import sys
import os
import re
import time
import natsort

DEBIAN = ('ftp://ftp.de.debian.org/debian/',
          'ftp://ftp.debian.org/debian/')

def wget(filepath):
    filename = filepath
    if '/' in filepath:
        filename = filename[filename.rfind('/') + 1:]
    for d in DEBIAN:
        # TODO: handle exitcode?
        subprocess.call(
            ['nice', 'wget', '--tries=10', '--timeout=300', '-O', filename, d + filepath])
        if os.path.isfile(filename):
            return True
        print('Sleep for 10 seconds..')
        time.sleep(10)
    return False


def latestvername(names):
    s = natsort.natsorted(names, key=lambda x: x[x.index('_')+1:x.index('.orig.tar')])
    return s[-1]


def getpackages():
    if not wget('ls-lR.gz'):
        sys.exit(1)
    # TODO: handle exitcode?
    subprocess.call(['nice', 'gunzip', 'ls-lR.gz'])
    if not os.path.isfile('ls-lR'):
        sys.exit(1)
    with open('ls-lR', 'rt') as f:
        lines = f.readlines()
    # TODO: handle exitcode?
    subprocess.call(['rm', 'ls-lR'])

    # Example content in ls-lR:
    #./pool/main/0/0xffff:
    #total 1452
    #-rw-r--r-- 2 dak debadmin  6524 Dec 25  2016 0xffff_0.7-2.debian.tar.xz
    #-rw-r--r-- 2 dak debadmin  1791 Dec 25  2016 0xffff_0.7-2.dsc
    #-rw-r--r-- 2 dak debadmin 57168 Dec 25  2016 0xffff_0.7-2_amd64.deb
    #-rw-r--r-- 2 dak debadmin 48578 Dec 26  2016 0xffff_0.7-2_arm64.deb
    #-rw-r--r-- 2 dak debadmin 56730 Dec 26  2016 0xffff_0.7-2_armel.deb
    #-rw-r--r-- 2 dak debadmin 57296 Dec 26  2016 0xffff_0.7-2_armhf.deb
    #-rw-r--r-- 2 dak debadmin 60254 Dec 26  2016 0xffff_0.7-2_i386.deb
    #-rw-r--r-- 2 dak debadmin 53130 Dec 26  2016 0xffff_0.7-2_mips.deb
    #-rw-r--r-- 2 dak debadmin 52542 Dec 26  2016 0xffff_0.7-2_mips64el.deb
    #-rw-r--r-- 2 dak debadmin 53712 Dec 26  2016 0xffff_0.7-2_mipsel.deb
    #-rw-r--r-- 2 dak debadmin 51908 Dec 26  2016 0xffff_0.7-2_ppc64el.deb
    #-rw-r--r-- 2 dak debadmin 53548 Dec 26  2016 0xffff_0.7-2_s390x.deb
    #-rw-r--r-- 2 dak debadmin 65248 Dec 25  2016 0xffff_0.7.orig.tar.gz
    #-rw-r--r-- 2 dak debadmin  6884 Jul 19 19:08 0xffff_0.8-1.debian.tar.xz
    #-rw-r--r-- 2 dak debadmin  1807 Jul 19 19:08 0xffff_0.8-1.dsc
    #-rw-r--r-- 2 dak debadmin 58908 Jul 19 19:08 0xffff_0.8-1_amd64.deb
    #-rw-r--r-- 2 dak debadmin 51340 Jul 19 19:58 0xffff_0.8-1_arm64.deb
    #-rw-r--r-- 2 dak debadmin 57612 Jul 19 20:13 0xffff_0.8-1_armel.deb
    #-rw-r--r-- 2 dak debadmin 58584 Jul 19 19:58 0xffff_0.8-1_armhf.deb
    #-rw-r--r-- 2 dak debadmin 57544 Jul 19 20:23 0xffff_0.8-1_hurd-i386.deb
    #-rw-r--r-- 2 dak debadmin 62048 Jul 19 23:54 0xffff_0.8-1_i386.deb
    #-rw-r--r-- 2 dak debadmin 55080 Jul 23 19:07 0xffff_0.8-1_kfreebsd-amd64.deb
    #-rw-r--r-- 2 dak debadmin 58392 Jul 23 19:07 0xffff_0.8-1_kfreebsd-i386.deb
    #-rw-r--r-- 2 dak debadmin 54144 Jul 19 22:28 0xffff_0.8-1_mips.deb
    #-rw-r--r-- 2 dak debadmin 53648 Jul 20 00:56 0xffff_0.8-1_mips64el.deb
    #-rw-r--r-- 2 dak debadmin 54740 Jul 19 22:58 0xffff_0.8-1_mipsel.deb
    #-rw-r--r-- 2 dak debadmin 57424 Jul 19 19:58 0xffff_0.8-1_ppc64el.deb
    #-rw-r--r-- 2 dak debadmin 53764 Jul 19 22:28 0xffff_0.8-1_s390x.deb
    #-rw-r--r-- 2 dak debadmin 64504 Jul 19 19:08 0xffff_0.8.orig.tar.gz
    #

    path = None
    previous_path = ''
    archives = []
    filename = None
    filenames = []
    for line in lines:
        line = line.strip()
        if len(line) < 4:
            if filename:
                res1 = re.match(r'(.*)-[0-9.]+$', path)
                if res1 is None:
                    res1 = re.match(r'(.*)[-.][0-9.]+$', path)
                res2 = re.match(r'(.*)-[0-9.]+$', previous_path)
                if res2 is None:
                    res2 = re.match(r'(.*)[-.][0-9.]+$', previous_path)
                if res1 is None or res2 is None or res1.group(1) != res2.group(1):
                    archives.append(path + '/' + latestvername(filenames))
                else:
                    archives[-1] = path + '/' + latestvername(filenames)
            if path:
                previous_path = path
            path = None
            filename = None
            filenames = []
        elif line.startswith('./pool/main/'):
            path = line[2:-1]
        elif path and line.endswith(('.orig.tar.gz', '.orig.tar.bz2', '.orig.tar.xz')):
            filename = line[1 + line.rfind(' '):]
            filenames.append(filename)

    return archives



for p in getpackages():
    print(DEBIAN[0] + p)

