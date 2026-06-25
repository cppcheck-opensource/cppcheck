#!/usr/bin/env python3
#
# Downloads all daca2 source code packages.
#
# Usage:
# $ ./daca2-download.py


import subprocess
import sys
import shutil
import glob
import os
import time
import natsort

DEBIAN = ('ftp://ftp.se.debian.org/debian/',
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
        return []
    # TODO: handle exitcode?
    subprocess.call(['nice', 'gunzip', 'ls-lR.gz'])
    with open('ls-lR', 'rt') as f:
        lines = f.readlines()
    # TODO: handle exitcode?
    subprocess.call(['rm', 'ls-lR'])

    path = None
    archives = []
    filename = None
    filenames = []
    for line in lines:
        line = line.strip()
        if len(line) < 4:
            if filename:
                archives.append(path + '/' + latestvername(filenames))
            path = None
            filename = None
            filenames = []
        elif line[:12] == './pool/main/':
            path = line[2:-1]
        elif path and '.orig.tar.' in line:
            filename = line[1 + line.rfind(' '):]
            filenames.append(filename)

    return archives


def handleRemoveReadonly(func, path, exc):
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)


def removeAll():
    count = 5
    while count > 0:
        count -= 1

        filenames = []
        filenames.extend(glob.glob('[#_A-Za-z0-9]*'))
        filenames.extend(glob.glob('.[A-Za-z]*'))

        try:
            for filename in filenames:
                if os.path.isdir(filename):
                    # pylint: disable=deprecated-argument - FIXME: onerror was deprecated in Python 3.12
                    shutil.rmtree(filename, onerror=handleRemoveReadonly)
                else:
                    os.remove(filename)
        # pylint: disable=undefined-variable
        except WindowsError as err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup files/folders')
                print(err)
                sys.exit(1)
            continue
        except OSError as err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup files/folders')
                print(err)
                sys.exit(1)
            continue
        count = 0


def accept_filename(filename:str):
    if '.' not in filename:
        return False
    ext = filename[filename.rfind('.'):]
    if ext == '.proto' and ('--protobuf' in sys.argv):
        print('accept_filename:' + filename)
        return True
    return ext in ('.C', '.c', '.H', '.h', '.cc',
                   '.cpp', '.cxx', '.c++', '.hpp', '.tpp', '.t++')


def removeLargeFiles(path):
    for g in glob.glob(path + '*'):
        if g == '.' or g == '..':
            continue
        if os.path.islink(g):
            continue
        if os.path.isdir(g):
            removeLargeFiles(g + '/')
        elif os.path.isfile(g):
            # remove large files
            statinfo = os.stat(g)
            if statinfo.st_size > 100000:
                os.remove(g)

            # remove non-source files
            elif not accept_filename(g):
                os.remove(g)


def downloadpackage(filepath, outpath):
    # remove all files/folders
    removeAll()

    if not wget(filepath):
        print('Failed to download ' + filepath)
        return

    filename = filepath[filepath.rfind('/') + 1:]
    if filename[-3:] == '.gz':
        # TODO: handle exitcode?
        subprocess.call(['tar', 'xzf', filename])
    elif filename[-3:] == '.xz':
        # TODO: handle exitcode?
        subprocess.call(['tar', 'xJf', filename])
    elif filename[-4:] == '.bz2':
        # TODO: handle exitcode?
        subprocess.call(['tar', 'xjf', filename])
    else:
        return

    removeLargeFiles('')

    for g in glob.glob('[#_A-Za-z0-9]*'):
        if os.path.isdir(g):
            # TODO: handle exitcode?
            subprocess.call(['tar', '-cJf', outpath + filename[:filename.rfind('.')] + '.xz', g])
            break

workdir = os.path.expanduser('~/daca2-packages/tmp/')
if not os.path.isdir(workdir):
    os.makedirs(workdir)
os.chdir(workdir)

packages = getpackages()
if len(packages) == 0:
    print('failed to load packages')
    sys.exit(1)

print('Sleep for 10 seconds..')
time.sleep(10)

for package in packages:
    downloadpackage(package, os.path.expanduser('~/daca2-packages/'))

# remove all files/folders
removeAll()
