#!/usr/bin/env python
import time
import sys

from bisect_common import *

# TODO: detect missing file
def run(cppcheck_path, options, elapsed_time=None):
    timeout = None
    if elapsed_time:
        timeout = elapsed_time * 2
    cmd = options.split()
    cmd.insert(0, cppcheck_path)
    print('running {}'.format(cppcheck_path))
    with subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) as p:
        try:
            p.communicate(timeout=timeout)
            if p.returncode != 0:
                print('error')
                return None
            print('done')
        except subprocess.TimeoutExpired:
            print('timeout')
            p.kill()
            p.communicate()
            return False

    return True


# TODO: check arguments
bisect_path = sys.argv[1]
options = sys.argv[2]
if '--error-exitcode=0' not in options:
    options += ' --error-exitcode=0'
if len(sys.argv) >= 4:
    elapsed_time = float(sys.argv[3])
else:
    elapsed_time = None
if len(sys.argv) == 5:
    invert = sys.argv[4] == "2"
else:
    invert = False

try:
    cppcheck_path = build_cppcheck(bisect_path)
except Exception as e:
    # TODO: how to persist this so we don't keep compiling these
    print(e)
    sys.exit(EC_SKIP)

if not elapsed_time:
    t = time.perf_counter()
    # TODO: handle error result
    run(cppcheck_path, options)
    elapsed_time = time.perf_counter() - t
    print('elapsed_time: {}'.format(elapsed_time))
    # TODO: write to stdout and redirect all all printing to stderr
    sys.exit(round(elapsed_time + .5))  # return the time

t = time.perf_counter()
run_res = run(cppcheck_path, options, elapsed_time)
run_time = time.perf_counter() - t

if not elapsed_time:
    # TODO: handle error result
    print('elapsed_time: {}'.format(run_time))
    # TODO: write to stdout and redirect all printing to stderr
    sys.exit(round(run_time + .5))  # return the time

if run_res is None:
    sys.exit(EC_SKIP)  # error occurred

if not run_res:
    sys.exit(EC_BAD if not invert else EC_GOOD)  # timeout occurred

print('run_time: {}'.format(run_time))

sys.exit(EC_GOOD if not invert else EC_BAD)  # no timeout
