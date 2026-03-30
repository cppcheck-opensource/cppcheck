#!/usr/bin/env python3

# Run this script from your branch with proposed Cppcheck patch to verify your
# patch against current main. It will compare output of testing a bunch of
# opensource packages
# If running on Windows, make sure that git.exe, wget.exe, and MSBuild.exe are available in PATH

import donate_cpu_lib as lib
import argparse
import glob
import os
import sys
import random
import subprocess


def format_float(a, b=1):
    if a > 0 and b > 0:
        return f'{a / b:.2f}'
    return 'N/A'


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
        print(f'Failed to clone Cppcheck repository ({e}), retry later')
        sys.exit(1)

    try:
        lib.checkout_cppcheck_version(repo_dir, 'main', main_dir)
    except Exception as e:
        print(f'Failed to checkout main ({e}), retry later')
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
        print(f'Error: {e}')
        print('Failed to switch to common ancestor of your branch and main')
        sys.exit(1)

    if not lib.compile_cppcheck(main_dir):
        print('Failed to compile main of Cppcheck')
        sys.exit(1)

    print('Testing your PR from directory: ' + your_repo_dir)
    if not lib.compile_cppcheck(your_repo_dir):
        print('Failed to compile your version of Cppcheck')
        sys.exit(1)

    if args.packages_path:
        # You can download packages using daca2-download.py
        args.packages = glob.glob(os.path.join(args.packages_path, '*.tar.xz'))
        args.p = len(args.packages)
        packages_idxs = list(range(args.p))
        random.shuffle(packages_idxs)
    elif args.packages:
        args.p = len(args.packages)
        packages_idxs = []
    else:
        packages_count = lib.get_packages_count()
        if not packages_count:
            print("network or server might be temporarily down..")
            sys.exit(1)

        packages_idxs = list(range(packages_count))
        random.shuffle(packages_idxs)

    packages_processed = 0
    crashes = []
    timeouts = []

    while (packages_processed < args.p and len(packages_idxs) > 0) or args.packages:
        if args.packages:
            package = args.packages.pop()
        else:
            package = lib.get_package(packages_idxs.pop())

        if package.startswith('ftp://') or package.startswith('http://'):
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
