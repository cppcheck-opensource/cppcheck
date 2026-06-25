import subprocess
import os
import sys
import pytest

# TODO: only run on Windows

def _lookup_cppcheck_exe(exe_name):
    # path the script is located in
    script_path = os.path.dirname(os.path.realpath(__file__))

    if sys.platform == "win32":
        exe_name += ".exe"

    for base in (script_path + '/../../', './'):
        for path in ('', 'bin/', 'bin/debug/'):
            exe_path = base + path + exe_name
            if os.path.isfile(exe_path):
                print("using '{}'".format(exe_path))
                return exe_path

    return None

def _call_process(arg):
    exe = _lookup_cppcheck_exe('test-sehwrapper')
    if exe is None:
        raise Exception('executable not found')
    p = subprocess.Popen([exe, arg], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    return p.returncode, stdout, stderr


def test_assert():
    exitcode, stdout, stderr = _call_process('assert')
    assert stdout == ''
    # Assertion failed: false, file S:\GitHub\cppcheck-fw\test\seh\test-sehwrapper.cpp, line 33
    assert stderr.startswith("Assertion failed: false, file "), stderr
    assert stderr.endswith("test-sehwrapper.cpp, line 33\n"), stderr
    assert exitcode == 3


def test_abort():
    exitcode, stdout, stderr = _call_process('abort')
    # nothing is written in case of abort()
    # it will show the "Debug Error!" MessageBox though which we suppress in the test
    assert stdout == ''
    assert stderr == ''
    assert exitcode == 3


def test_segv():
    exitcode, stdout, stderr = _call_process('segv')
    assert stderr == ''
    lines = stdout.splitlines()
    assert lines[0].startswith('Internal error: Access violation (instruction: 0x'), lines[0]
    assert lines[0].endswith(') reading from 0x0000000000000000'), lines[0]
    assert lines[1].startswith('0. 0x'), lines[1]
    assert lines[1].endswith(' in my_segv'), lines[1]
    assert lines[2].startswith('1. 0x'), lines[2]
    assert lines[2].endswith(' in main'), lines[2]
    assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
    assert exitcode == 4294967295  # returns -1


# TODO: make this work
@pytest.mark.skip
def test_fpe():
    exitcode, stdout, stderr = _call_process('fpe')
    assert stderr == ''
    lines = stdout.splitlines()
    assert lines[0].startswith('Internal error: cppcheck received signal SIGFPE - FPE_FLTDIV (at 0x7f'), lines[0]
    assert lines[0].endswith(').'), lines[0]
    assert lines[1] == 'Callstack:'
    assert lines[2].endswith('my_fpe()'), lines[2]
    assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
    assert exitcode == 4294967295  # returns -1
