#!/usr/bin/env python3

# Compare results and timings of different valueflow options
# Example usage:
# cd ~/cppcheck && make CXXOPTS=-O2 MATCHCOMPILER=yes
# python3 compare-valueflow-options.py --cppcheck-path=~/cppcheck --packages-path=~/daca2-packages

import donate_cpu_lib as lib
import argparse
import glob
import os
import re
import sys
import random


# Do not report execution times below this limit, short execution time measurements are flaky
TIME_LIMIT = 2


def format_float(a, b=1):
    if a > 0 and b > 0:
        return '{:.2f}'.format(a / b)
    return 'N/A'


def count_errors(errout:str, c:set):
    for line in errout.split('\n'):
        if not line.endswith(']'):
            continue
        res = re.match(r'^[^:]+:[0-9]+:[0-9]+: (error|warning|style|portability|performance):.*\[([a-zA-Z0-9_\-]+)\]$', line)
        if res is None:
            print('No warning? ' + line)
            continue
        severity = res.group(1)
        c[severity] = c.get(severity, 0) + 1
        error_id = res.group(2)
        c[error_id] = c.get(error_id, 0) + 1


if __name__ == "__main__":
    __my_script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
    __work_path = os.path.expanduser(os.path.join('~', 'cppcheck-' + __my_script_name + '-workfolder'))

    parser = argparse.ArgumentParser(description='Compare --check-level=normal and --check-level=exhaustive output')
    parser.add_argument('-j', default=1, type=int, help='Concurency execution threads')
    parser.add_argument('--cppcheck-path', default=None, type=str, help='Path to Cppcheck binary, if not given then clone and compile')
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

    lib.set_jobs('-j' + str(args.j))

    results_file = os.path.join(work_path, args.o)
    summary_file = os.path.join(work_path, 'summary.log')

    for f in (results_file, summary_file):
        if os.path.exists(f):
            os.remove(f)

    opts = {'0': '--check-level=exhaustive --suppress=valueFlow*',
            'it2': '--check-level=exhaustive --performance-valueflow-max-iterations=2 --suppress=valueFlow*',
            'it1': '--check-level=exhaustive --performance-valueflow-max-iterations=1 --suppress=valueFlow*',
            'if8': '--check-level=exhaustive --performance-valueflow-max-if-count=8 --suppress=valueFlow*'}

    cppcheck_path = args.cppcheck_path

    if cppcheck_path is None:
        cppcheck_path = os.path.join(work_path, 'cppcheck')
        try:
            lib.clone_cppcheck(cppcheck_path, '')
        except Exception as e:
            print('Failed to clone Cppcheck repository ({}), retry later'.format(e))
            sys.exit(1)

        if not lib.compile_cppcheck(cppcheck_path):
            print('Failed to compile Cppcheck')
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
    summary_results = {}
    for id in opts.keys():
        summary_results[id] = {}

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

        results0 = None
        time0 = None

        enable = 'style'
        debug_warnings = False

        libraries = lib.library_includes.get_libraries(source_path)

        with open(results_file, 'at') as myfile:
            myfile.write('package:' + package + '\n')
            myfile.write('libraries:' + ','.join(libraries) +'\n')

        for id, extra_args in opts.items():
            print('scan:' + id)
            c, errout, info, time, cppcheck_options, timing_info = lib.scan_package(cppcheck_path, source_path, libraries, enable=enable, extra_args=extra_args)
            error_text = None
            if c < 0:
                if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                    # No sourcefile found (for example only headers present)
                    error_text = f'{id} ERR no source file'
                elif c == lib.RETURN_CODE_TIMEOUT:
                    error_text = f'{id} timeout'
                else:
                    error_text = f'{id} crash code={c}'

            with open(results_file, 'at') as myfile:
                if error_text is not None:
                    myfile.write(f'{error_text}\n')
                else:
                    results = {}
                    count_errors(errout, results)
                    count_errors(errout, summary_results[id])
                    if results0 is None:
                        results0 = results
                        time0 = time
                    else:
                        for error_id, count in results0.items():
                            current_count = results.get(error_id, 0)
                            if count > current_count:
                                myfile.write(f'{id}: FN {error_id}: {current_count} of {count}\n')
                        if time > TIME_LIMIT or time0 > TIME_LIMIT:
                            myfile.write(f'{id}: Time: %.1f\n' % time)
                            time_factor = time / time0
                            myfile.write(f'{id}: Timefactor: %.3f\n' % time_factor)

        with open(summary_file, 'wt') as myfile:
            all = {}
            for id, c in summary_results.items():
                for error_id, count in c.items():
                    if error_id not in all:
                        all[error_id] = {}
                        for id2 in opts.keys():
                            all[error_id][id2] = 0
                    all[error_id][id] += count

            for error_id, id_count in all.items():
                myfile.write(f'{error_id}:')
                for id, count in id_count.items():
                    myfile.write(f' {id}:{count}')
                myfile.write('\n')

        packages_processed += 1
        print(str(packages_processed) + ' of ' + str(args.p) + ' packages processed\n')

    print('Result saved to: ' + results_file)
