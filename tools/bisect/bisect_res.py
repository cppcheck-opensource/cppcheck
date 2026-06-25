#!/usr/bin/env python
import sys

from bisect_common import *

# TODO: detect missing file
def run(cppcheck_path, options):
    cmd = options.split()
    cmd.insert(0, cppcheck_path)
    print('running {}'.format(cppcheck_path))
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True) as p:
        stdout, stderr = p.communicate()
        rc = p.returncode
    # only 0 and 1 are well-defined in this case
    if rc > 1:
        print('error')
        return None, None, None
    # signals are reported as negative exitcode (e.g. SIGSEGV -> -11)
    if rc < 0:
        print('crash')
        return rc, stderr, stdout
    print('done')
    return rc, stderr, stdout


# TODO: check arguments
bisect_path = sys.argv[1]
options = sys.argv[2]
expected = None
if len(sys.argv) == 4:
    expected = sys.argv[3]
if len(expected) == 0:
    expected = None
if expected is None:
    if '--error-exitcode=1' not in options:
        options += ' --error-exitcode=1'
else:
    if '--error-exitcode=0' not in options:
        options += ' --error-exitcode=0'

try:
    cppcheck_path = build_cppcheck(bisect_path)
except Exception as e:
    # TODO: how to persist this so we don't keep compiling these
    print(e)
    sys.exit(EC_SKIP)

run_ec, run_stderr, run_stdout = run(cppcheck_path, options)

if expected is None:
    print(run_ec)
print(run_stdout)
print(run_stderr)

# if no ec is set we encountered an unexpected error
if run_ec is None:
    sys.exit(EC_SKIP)  # error occurred
elif run_ec < 0:
    sys.exit(EC_BAD) # crash occurred

# check output for expected string
if expected is not None:
    if (expected not in run_stderr) and (expected not in run_stdout):
        sys.exit(EC_BAD)  # output not found occurred

    sys.exit(EC_GOOD)  # output found

sys.exit(run_ec) # return the elapsed time - not a result for bisect
