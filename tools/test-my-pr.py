#!/usr/bin/env python3

# Run this script from your branch with proposed Cppcheck patch to verify your
# patch against current main. It will compare output of testing a bunch of
# opensource packages
# If running on Windows, make sure that git.exe, wget.exe, and MSBuild.exe are available in PATH

import donate_cpu_lib as lib
import argparse
import glob
import gzip
import natsort
import os
import sys
import random
import re
import requests
import subprocess


def format_float(a, b=1):
    if a > 0 and b > 0:
        return '{:.2f}'.format(a / b)
    return 'N/A'


def ftp_get(url):
    try:
        response = requests.get(url, timeout=300)
        response.raise_for_status()
        return response.content
    except requests.RequestException as err:
        print('Failed to fetch {}: {}'.format(url, err))
    return None


def latestvername(names):
    s = natsort.natsorted(names, key=lambda x: x[x.index('_')+1:x.index('.orig.tar')])
    return s[-1]


def getpackages():
    debian = 'https://ftp.debian.org/debian/'

    data = ftp_get(debian + 'ls-lR.gz')
    if data is None:
        print('Failed to fetch ls-lR.gz')
        sys.exit(1)

    lines = gzip.decompress(data).decode('utf-8', errors='replace').splitlines()

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
            path = debian + line[2:-1]
        elif path and line.endswith(('.orig.tar.gz', '.orig.tar.bz2', '.orig.tar.xz')):
            filename = line[1 + line.rfind(' '):]
            filenames.append(filename)

    return archives




if __name__ == "__main__":
    __my_script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
    __work_path = os.path.expanduser(os.path.join('~', 'cppcheck-' + __my_script_name + '-workfolder'))

    parser = argparse.ArgumentParser(description='Run this script from your branch with proposed Cppcheck patch to verify your patch against current main. It will compare output of testing bunch of opensource packages')
    parser.add_argument('-j', default=1, type=int, help='Concurency execution threads')
    package_group = parser.add_mutually_exclusive_group()
    package_group.add_argument('-p', default=256, type=int, help='Count of packages to check')
    package_group.add_argument('--packages', nargs='+', help='Check specific packages and then stop.')
    package_group.add_argument('--packages-path', default=None, type=str, help='Check packages in path.')
    parser.add_argument('-o', default='my_check_diff.log', help='Filename of result inside a working path dir')

    language_group = parser.add_mutually_exclusive_group()
    language_group.add_argument('--c-only', dest='c_only', help='Only process c packages', action='store_true')
    language_group.add_argument('--cpp-only', dest='cpp_only', help='Only process c++ packages', action='store_true')
    parser.add_argument('--work-path', '--work-path=', default=__work_path, type=str, help='Working directory for reference repo')
    args = parser.parse_args()

    print(args)

    if args.packages_path:
        # You can download packages using daca2-download.py
        args.packages = glob.glob(os.path.join(args.packages_path, '*.tar.xz'))
        random.shuffle(args.packages)
    elif args.packages is None:
        args.packages = getpackages()
        random.shuffle(args.packages)

    print('\n'.join(args.packages[:20]))

    if not lib.check_requirements():
        print("Error: Check requirements")
        sys.exit(1)

    work_path = os.path.abspath(args.work_path)
    if not os.path.exists(work_path):
        os.makedirs(work_path)
    repo_dir = os.path.join(work_path, 'repo')
    old_repo_dir = os.path.join(work_path, 'cppcheck')
    main_dir = os.path.join(work_path, 'tree-main')

    lib.set_jobs('-j' + str(args.j))
    result_file = os.path.abspath(os.path.join(work_path, args.o))
    (f, ext) = os.path.splitext(result_file)
    timing_file = f + '_timing' + ext
    your_repo_dir = os.path.dirname(os.path.dirname(os.path.abspath(sys.argv[0])))

    if os.path.exists(result_file):
        os.remove(result_file)
    if os.path.exists(timing_file):
        os.remove(timing_file)

    try:
        lib.clone_cppcheck(repo_dir, old_repo_dir)
    except Exception as e:
        print('Failed to clone Cppcheck repository ({}), retry later'.format(e))
        sys.exit(1)

    try:
        lib.checkout_cppcheck_version(repo_dir, 'main', main_dir)
    except Exception as e:
        print('Failed to checkout main ({}), retry later'.format(e))
        sys.exit(1)

    try:
        commit_id = (subprocess.check_output(['git', 'merge-base', 'origin/main', 'HEAD'], cwd=your_repo_dir)).strip().decode('ascii')
        with open(result_file, 'a') as myfile:
            myfile.write('Common ancestor: ' + commit_id + '\n\n')
        package_width = '140'
        timing_width = '>7'
        with open(timing_file, 'a') as myfile:
            myfile.write('{:{package_width}} {:{timing_width}} {:{timing_width}} {:{timing_width}}\n'.format(
                'Package', 'main', 'your', 'Factor', package_width=package_width, timing_width=timing_width))

        subprocess.check_call(['git', 'fetch', '--depth=1', 'origin', commit_id])
        subprocess.check_call(['git', 'checkout', '-f', commit_id])
    except BaseException as e:
        print('Error: {}'.format(e))
        print('Failed to switch to common ancestor of your branch and main')
        sys.exit(1)

    if not lib.compile_cppcheck(main_dir):
        print('Failed to compile main of Cppcheck')
        sys.exit(1)

    print('Testing your PR from directory: ' + your_repo_dir)
    if not lib.compile_cppcheck(your_repo_dir):
        print('Failed to compile your version of Cppcheck')
        sys.exit(1)

    packages_processed = 0
    crashes = []
    timeouts = []

    while packages_processed < args.p and args.packages:
        package = args.packages.pop()

        if package.startswith('ftp://') or package.startswith('https://'):
            tgz = lib.download_package(work_path, package, None)
            if tgz is None:
                print("No package downloaded")
                continue
        else:
            print('Package: ' + package)
            tgz = package

        source_path, source_found = lib.unpack_package(work_path, tgz, c_only=args.c_only, cpp_only=args.cpp_only)
        if not source_found:
            print("No files to process")
            continue

        results_to_diff = []

        main_crashed = False
        your_crashed = False

        main_timeout = False
        your_timeout = False

        libraries = lib.library_includes.get_libraries(source_path)
        c, errout, info, time_main, cppcheck_options, timing_info = lib.scan_package(main_dir, source_path, libraries)
        if c < 0:
            if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                # No sourcefile found (for example only headers present)
                print('Error: 101')
            elif c == lib.RETURN_CODE_TIMEOUT:
                print('Main timed out!')
                main_timeout = True
            else:
                print('Main crashed!')
                main_crashed = True
        results_to_diff.append(errout)

        c, errout, info, time_your, cppcheck_options, timing_info = lib.scan_package(your_repo_dir, source_path, libraries)
        if c < 0:
            if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                # No sourcefile found (for example only headers present)
                print('Error: 101')
            elif c == lib.RETURN_CODE_TIMEOUT:
                print('Your code timed out!')
                your_timeout = True
            else:
                print('Your code crashed!')
                your_crashed = True
        results_to_diff.append(errout)

        if main_crashed or your_crashed:
            who = None
            if main_crashed and your_crashed:
                who = 'Both'
            elif main_crashed:
                who = 'Main'
            else:
                who = 'Your'
            crashes.append(package + ' ' + who)

        if main_timeout or your_timeout:
            who = None
            if main_timeout and your_timeout:
                who = 'Both'
            elif main_timeout:
                who = 'Main'
            else:
                who = 'Your'
            timeouts.append(package + ' ' + who)

        with open(result_file, 'a') as myfile:
            myfile.write(package + '\n')
            diff = lib.diff_results('main', results_to_diff[0], 'your', results_to_diff[1])
            if not main_crashed and not your_crashed and diff != '':
                myfile.write('libraries:' + ','.join(libraries) +'\n')
                myfile.write('diff:\n' + diff + '\n')

        if not main_crashed and not your_crashed:
            with open(timing_file, 'a') as myfile:
                myfile.write('{:{package_width}} {:{timing_width}} {:{timing_width}} {:{timing_width}}\n'.format(
                    package, format_float(time_main),
                    format_float(time_your), format_float(time_your, time_main),
                    package_width=package_width, timing_width=timing_width))

        packages_processed += 1
        print(str(packages_processed) + ' of ' + str(args.p) + ' packages processed\n')

    with open(result_file, 'a') as myfile:
        myfile.write('\n\ncrashes\n')
        myfile.write('\n'.join(crashes))

    with open(result_file, 'a') as myfile:
        myfile.write('\n\ntimeouts\n')
        myfile.write('\n'.join(timeouts) + '\n')

    print('Result saved to: ' + result_file)
