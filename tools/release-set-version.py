#!/usr/bin/env python3
#
# Update Cppcheck version
#
# Usage:
# release: python3 release-set-version.py 2.18.0
# debug:   python3 release-set-version.py 2.18.99

import glob
import re
import subprocess
import sys


def git(args):
    return subprocess.check_output(['git'] + args).decode('utf-8').strip()


def sed(args):
    return subprocess.check_output(['sed'] + args).decode('utf-8').strip()


def egrep(args):
    try:
        return subprocess.check_output(['egrep'] + args).decode('utf-8').strip()
    except Exception:
        return ''


# is there uncommitted changes in folder
def is_uncommitted_changes():
    for f in git(['status', '--short']).split('\n'):
        if not f.startswith('??'):
            return True
    return False


# get current git branch
def get_current_branch():
    return git(['branch', '--show-current'])


def set_version(new_version:str):
    if re.match(r'2[.][0-9][0-9][.][0-9]{1,2}([.][0-9])?', new_version) is None:
        print(f'Aborting, invalid version {new_version}')
        return

    v = new_version.split('.')

    if is_uncommitted_changes():
        print('Aborting, there are uncommitted changes')
        #return

    is_dev_version = (len(v) == 3 and v[-1] == '99')

    if not is_dev_version:
        expected_branch = v[0] + '.' + v[1] + '.x'
        if get_current_branch() != expected_branch:
            print(f'Aborting, must be executed from {expected_branch} branch')
            return

    v3 = '.'.join(v[:3])

    cppcheck_version_string = (v[0] + '.' + str(int(v[1])+1) + ' dev') if is_dev_version else new_version
    cppcheck_version = ','.join((v+['0','0','0','0'])[:4])

    def check_sed(args):
        file = args[-1]
        res = re.match(r's/([^/]+)/.*', args[-2])
        if res is None:
            raise Exception('Failed to verify sed call argument ' + args[-2])
        pattern = res.group(1)
        if len(egrep([pattern, file])) < 4:
            print(f"WARNING: pattern '{pattern}' not found in file {file}")
        sed(args)

    check_sed(['-i', '-r', f's/version 2[.][0-9]+([.][0-9]+)*/version {new_version}/', 'cli/main.cpp'])
    check_sed(['-i', '-r', f's/VERSION 2[.][0-9]+[.]99/VERSION {v3}/', 'CMakeLists.txt']) # version must have 3 parts.
    check_sed(['-i', '-r', f's/#define CPPCHECK_VERSION_STRING .*/#define CPPCHECK_VERSION_STRING "{cppcheck_version_string}"/', 'lib/version.h'])
    check_sed(['-i', '-r', f's/#define CPPCHECK_VERSION .*/#define CPPCHECK_VERSION {cppcheck_version}/', 'lib/version.h'])
    check_sed(['-i', '-r', f's/<[?]define ProductName[ ]*=.*/<?define ProductName = "Cppcheck $(var.Platform) {cppcheck_version_string}" ?>/', 'win_installer/productInfo.wxi'])
    check_sed(['-i', '-r', f's/<[?]define ProductVersion[ ]*=.*/<?define ProductVersion = "{new_version}" ?>/', 'win_installer/productInfo.wxi'])
    for g in glob.glob('man/*.md'):
        check_sed(['-i', '-r', f's/subtitle: Version 2\\.[0-9].*/subtitle: Version {cppcheck_version_string}/', g])
    print('Versions have been changed.')
    print('')
    print('Please double check these results below:')
    for p in ('[^.0-9]2[.][0-9]+[.][0-9]', '2[.][0-9]+ dev'):
        for ext in ('cpp', 'h', 'md'):
            for g in glob.glob(f'*/*.{ext}'):
                s = egrep([p, g])
                if s != '':
                    print(f'{g}: {s}')
    print('')
    print("Please double check output of 'git diff'")
    commit_message = f'bumped version to {new_version}' if is_dev_version else f'{new_version}: Set version'
    print(f"git commit -a -m {commit_message}")

if len(sys.argv) != 2:
    print('Syntax: python3 release-set-version.py [version]')
    sys.exit(1)

set_version(sys.argv[-1])

