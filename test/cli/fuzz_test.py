import os
import subprocess

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_fuzz_crash():
    failures = {}

    fuzz_crash_dir = os.path.join(__script_dir, 'fuzz-crash')
    for f in os.listdir(fuzz_crash_dir):
        ret, stdout, stderr = cppcheck(['-q', '--language=c++', '--enable=all', '--inconclusive', f], cwd=fuzz_crash_dir)
        if ret != 0:
            failures[f] = stdout + stderr

    assert failures == {}


def test_fuzz_crash_c():
    failures = {}

    fuzz_crash_dir = os.path.join(__script_dir, 'fuzz-crash_c')
    # TODO: remove check if we have test data
    if not os.path.exists(fuzz_crash_dir):
        return
    for f in os.listdir(fuzz_crash_dir):
        ret, stdout, stderr = cppcheck(['-q', '--language=c', '--enable=all', '--inconclusive', f], cwd=fuzz_crash_dir)
        if ret != 0:
            failures[f] = stdout + stderr

    assert failures == {}


def test_fuzz_timeout():
    failures = []

    fuzz_timeout_dir = os.path.join(__script_dir, 'fuzz-timeout')
    for f in os.listdir(fuzz_timeout_dir):
        try:
            cppcheck(['-q', '--language=c++', '--enable=all', '--inconclusive', f], cwd=fuzz_timeout_dir, timeout=5)
        except subprocess.TimeoutExpired:
            failures.append(f)

    assert failures == []
