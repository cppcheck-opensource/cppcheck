#!/usr/bin/env python
import os.path
import subprocess
import sys
import argparse
import time
import difflib

from packaging.version import Version

parser = argparse.ArgumentParser()
parser.add_argument('dir', help='directory with versioned folders')
parser.add_argument('infile', help='the file to analyze')
parser.add_argument('repo', nargs='?', default=None, help='the git repository (for sorting commit hashes)')
parser.add_argument('--compare', action='store_true', help='compare output and only show when changed')
parser.add_argument('--verbose', action='store_true', help='verbose output for debugging')
parser.add_argument('--debug', action='store_true', help='passed through to binary if supported')
parser.add_argument('--debug-warnings', action='store_true', help='passed through to binary if supported')
parser.add_argument('--check-library', action='store_true', help='passed through to binary if supported')
parser.add_argument('--timeout', type=int, default=2, help='the amount of seconds to wait for the analysis to finish')
parser.add_argument('--compact', action='store_true', help='only print versions with changes with --compare')
parser.add_argument('--no-quiet', action='store_true', default=False, help='do not specify -q')
parser.add_argument('--perf', action='store_true', default=False, help='output duration of execution in seconds (CSV format)')
parser.add_argument('--start', default=None, help='specify the start version/commit')
parser.add_argument('--diff', action='store_true', help='show differences as unified diff')
package_group = parser.add_mutually_exclusive_group()
package_group.add_argument('--no-stderr', action='store_true', default=False, help='do not display stdout')
package_group.add_argument('--no-stdout', action='store_true', default=False, help='do not display stderr')
args = parser.parse_args()

def sort_commit_hashes(commits):
    git_cmd = 'git rev-list --abbrev-commit --topo-order --no-walk=sorted --reverse ' + ' '.join(commits)
    with subprocess.Popen(git_cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=git_repo, universal_newlines=True) as p:
        stdout, stderr = p.communicate()
        rc = p.returncode
    if rc != 0:
        print('error: sorting commit hashes failed')
        print(stderr)
        sys.exit(1)
    return stdout.splitlines()

verbose = args.verbose
do_compare = args.compare
if args.compact:
    if not do_compare:
        print('error: --compact requires --compare')
        sys.exit(1)
if args.perf:
    if args.compact:
        print('error: --compact has no effect with --perf')
    if args.no_stdout:
        print('error: --no-stdout has no effect with --perf')
    if args.no_stderr:
        print('error: --no-stderr has no effect with --perf')

directory = args.dir
input_file = args.infile
git_repo = args.repo

use_hashes = None
versions = []

for filename in os.listdir(directory):
    f = os.path.join(directory, filename)
    if not os.path.isdir(f):
        continue
    versions.append(filename)

if not len(versions):
    print("error: no versions found in '{}'".format(directory))
    sys.exit(1)

if verbose:
    print("found {} versions in '{}'".format(len(versions), directory))

try:
    Version(versions[0])
    use_hashes = False
    versions.sort(key=Version)
except:
    if verbose:
        print("'{}' not a version - assuming commit hashes".format(versions[0]))
    if not git_repo:
        print('error: git repository argument required for commit hash sorting')
        sys.exit(1)
    if verbose:
        print("using git repository '{}' to sort commit hashes".format(git_repo))
    use_hashes = True
    # if you use the folder from the bisect script that contains the repo as a folder - so remove it from the list
    if versions.count('cppcheck'):
        versions.remove('cppcheck')
    # this is the commit hash for the 2.9 release tag. it does not exist in the main branch so the version for it cannot be determined
    if versions.count('aca3f6fef'):
        versions.remove('aca3f6fef')
    # 2.8 tags
    if versions.count('61f846073'):
        versions.remove('61f846073')
    if versions.count('f998703a5'):
        versions.remove('f998703a5')
    # ???
    if versions.count('d4505827b'):
        versions.remove('d4505827b')
    # 2.6 tag
    if versions.count('d873b8e77'):
        versions.remove('d873b8e77')
    len_in = len(versions)
    versions = sort_commit_hashes(versions)
    if len(versions) != len_in:
        print('error: unexpected amount of versions after commit hash sorting')
        sys.exit(1)

if verbose:
    print("analyzing '{}'".format(input_file))

last_udiff_version = ''
last_ec = None
last_out = None
last_udiff = None

if args.perf:
    print('version,time')

start_entry = args.start

for entry in versions:
    if start_entry:
        if start_entry != entry:
            continue
        start_entry = None

    exe_path = os.path.join(directory, entry)
    exe = os.path.join(exe_path, 'cppcheck')

    if not use_hashes:
        version = entry
    else:
        # get version string
        version_cmd = exe + ' ' + '--version'
        with subprocess.Popen(version_cmd.split(), stdout=subprocess.PIPE, universal_newlines=True) as p:
            # TODO: handle p.returncode?
            version = p.stdout.read().strip()
        # sanitize version
        version = version.replace('Cppcheck ', '').replace(' dev', '')

    if version == 'CPPCHECK_MAJOR.CPPCHECK_DEVMINOR':
        continue

    cmd = [exe]
    if do_compare and not args.no_quiet:
        cmd.append('-q')
    if args.debug and Version(version) >= Version('1.45'):
        cmd.append('--debug')
    if args.debug_warnings and Version(version) >= Version('1.45'):
        cmd.append('--debug-warnings')
    if args.check_library and Version(version) >= Version('1.61'):
        cmd.append('--check-library')
    if Version(version) >= Version('1.39'):
        cmd.append('--enable=all')
    if Version(version) >= Version('1.40'):
        cmd.append('--inline-suppr')
    if Version(version) >= Version('1.48'):
        cmd.append('--suppress=missingInclude')
        cmd.append('--suppress=missingIncludeSystem')
        cmd.append('--suppress=unmatchedSuppression')
        cmd.append('--suppress=unusedFunction')
    if Version(version) >= Version('1.49'):
        cmd.append('--inconclusive')
    if Version(version) >= Version('1.69'):
        cmd.append('--platform=native')
    if Version(version) >= Version('1.52') and Version(version) < Version('2.0'):
        # extend Cppcheck 1.x format with error ID
        if Version(version) < Version('1.61'):
            # TODO: re-add inconclusive
            cmd.append('--template=[{file}:{line}]: ({severity}) {message} [{id}]')
        else:
            # TODO: re-add inconclusive: {callstack}: ({severity}{inconclusive:, inconclusive}) {message
            cmd.append('--template={callstack}: ({severity}) {message} [{id}]')
    if Version(version) >= Version('2.13'):
        cmd.append('--suppress=checkersReport')
    # TODO: how to pass additional options?
    if args.perf:
        cmd.append('--error-exitcode=0')
    cmd.append(input_file)
    if verbose:
        print("running '{}'". format(' '.join(cmd)))
    if args.perf:
        start = time.time_ns()
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=exe_path, universal_newlines=True)
    try:
        stdout, stderr = p.communicate(timeout=args.timeout)
        if args.perf:
            end = time.time_ns()
        out = ''
        if not args.no_stdout:
            out += stdout
        if not args.no_stdout and not args.no_stderr:
            out += '\n'
        if not args.no_stderr:
            out += stderr
    except subprocess.TimeoutExpired:
        out = "timeout"
        p.kill()
        p.communicate()

    ec = p.returncode
    p = None

    if not do_compare:
        if not use_hashes:
            ver_str = version
        else:
            ver_str = '{} ({})'.format(entry, version)
        if args.perf:
            if out == "timeout":
                data_str = "0.0" # TODO: how to handle these properly?
            elif ec != 0:
                continue # skip errors
            else:
                data_str = '{}'.format((end - start) / 1000.0 / 1000.0 / 1000.0)
            print('"{}",{}'.format(ver_str, data_str))
            continue
        print(ver_str)
        print(ec)
        print(out)
        continue

    # filter out some false positives
    # [*]: (information) Unmatched suppression: missingInclude
    # [*]: (information) Unmatched suppression: missingIncludeSystem
    # [*]: (information) Unmatched suppression: unmatchedSuppression
    # [*]: (information) Unmatched suppression: unusedFunction
    if not use_hashes and (Version(version) >= Version('1.48') or Version(version) <= Version('1.49')):
        lines = out.splitlines()
        out = ""
        for line in lines:
            if line.startswith('[*]: (information) Unmatched suppression:'):
                continue
            out += line + '\n'

    out = out.strip()

    if last_ec is None:
        # first run - only print version
        if not use_hashes:
            print(version)
        else:
            print('{} ({})'.format(entry, version))

        last_ec = ec
        last_out = out
        continue

    do_print = False
    udiff = None

    if last_ec != ec:
        if verbose:
            print("{}: exitcode changed".format(version))
        do_print = True

    if last_out != out:
        if verbose:
            print("{}: output changed".format(version))
        do_print = True
        if args.diff:
            udiff = difflib.unified_diff(last_out.splitlines(True), out.splitlines(True), fromfile=last_udiff_version, tofile=version)
            last_udiff_version = version

    if do_print:
        print(last_ec)
        print(last_out)
        if last_udiff:
            sys.stdout.writelines(last_udiff)
            sys.stdout.write('\n')

    # do not print intermediate versions with --compact
    if not args.compact or do_print:
        if not use_hashes:
            print(version)
        else:
            print('{} ({})'.format(entry, version))

    last_ec = ec
    last_out = out
    if udiff:
        last_udiff = udiff

if do_compare:
    print(last_ec)
    print(last_out)
    if last_udiff:
        sys.stdout.writelines(last_udiff)
        sys.stdout.write('\n')

if verbose:
    print('done')
