#!/usr/bin/env python
import time
import sys

from bisect_common import *

MAX_RT_FACTOR = 1.3

# TODO: detect missing file
def run(cppcheck_path, options, elapsed_time=None):
    timeout = None
    if elapsed_time:
        timeout = elapsed_time * 2
    cmd = options.split()
    cmd.insert(0, cppcheck_path)
    print('running (timeout: {}) {}'.format(timeout, cppcheck_path))
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL) as p:
        try:
            stdout, _ = p.communicate(timeout=timeout)
            if p.returncode != 0:
                print('error')
                return None
            print('done')
        except subprocess.TimeoutExpired:
            print('timeout')
            p.kill()
            stdout, _ = p.communicate()
            return False
        finally:
            stdout = stdout.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
            print(stdout)

    return True


# TODO: check arguments
bisect_path = sys.argv[1]
options = sys.argv[2]
if '--error-exitcode=0' not in options:
    options += ' --error-exitcode=0'
if '--showtime=file' not in options:
    options += ' --showtime=file-total'
if '-q' not in options:
    options += ' -q'
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
    sys.exit(max(round(elapsed_time - .5), 1))  # return the time

t = time.perf_counter()
run_res = run(cppcheck_path, options, elapsed_time)
run_time = time.perf_counter() - t

if not elapsed_time:
    # TODO: handle error result
    print('elapsed_time: {}'.format(run_time))
    # TODO: write to stdout and redirect all printing to stderr
    sys.exit(max(round(run_time - .5), 1))  # return the time

if run_res is None:
    sys.exit(EC_SKIP)  # error occurred

if not run_res:
    sys.exit(EC_BAD if not invert else EC_GOOD)  # timeout occurred

print('run_time: {}'.format(run_time))
run_time_factor = run_time / elapsed_time
print('run_time_factor: {}'.format(run_time_factor))
run_time_diff = run_time - elapsed_time
print('run_time_diff: {}'.format(run_time_diff))

if run_time_factor >= MAX_RT_FACTOR:
    sys.exit(EC_BAD if not invert else EC_GOOD)  # factor exceeded

sys.exit(EC_GOOD if not invert else EC_BAD)  # no timeout
