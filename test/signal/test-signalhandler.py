import subprocess
import os
import sys
import pytest
import platform

from packaging.version import Version

def __lookup_cppcheck_exe(exe_name):
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

def __call_process(arg):
    exe = __lookup_cppcheck_exe('test-signalhandler')
    if exe is None:
        raise Exception('executable not found')
    with subprocess.Popen([exe, arg], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        stdout, stderr = p.communicate()
        rc = p.returncode
    stdout = stdout.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = stderr.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    return rc, stdout, stderr


def test_assert():
    exitcode, stdout, stderr = __call_process('assert')
    if sys.platform == "darwin":
        assert stderr.startswith("Assertion failed: (false), function my_assert, file test-signalhandler.cpp, line "), stderr
    else:
        assert stderr.endswith("test-signalhandler.cpp:43: void my_assert(): Assertion `false' failed.\n"), stderr
    lines = stdout.splitlines()
    assert lines[0] == 'Internal error: cppcheck received signal SIGABRT - abort or assertion'
    # no stacktrace of macOS
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_abort()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
    assert exitcode == -6


def test_abort():
    exitcode, stdout, _ = __call_process('abort')
    lines = stdout.splitlines()
    assert lines[0] == 'Internal error: cppcheck received signal SIGABRT - abort or assertion'
    # no stacktrace on macOS
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_segv()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
    assert exitcode == -6


def test_segv():
    exitcode, stdout, stderr = __call_process('segv')
    assert stderr == ''
    lines = stdout.splitlines()
    if sys.platform == "darwin":
        if Version(platform.mac_ver()[0]) >= Version('14'):
            assert lines[0] == 'Internal error: cppcheck received signal SIGSEGV - SEGV_ACCERR (at 0x0).'
        else:
            assert lines[0] == 'Internal error: cppcheck received signal SIGSEGV - SEGV_MAPERR (at 0x0).'
    else:
        assert lines[0] == 'Internal error: cppcheck received signal SIGSEGV - SEGV_MAPERR (reading at 0x0).'
    # no stacktrace on macOS
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_segv()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
    assert exitcode == -11


@pytest.mark.skipif(sys.platform == 'darwin', reason='Cannot raise FPE on macOS')
def test_fpe():
    exitcode, stdout, stderr = __call_process('fpe')
    assert stderr == ''
    lines = stdout.splitlines()
    assert lines[0].startswith('Internal error: cppcheck received signal SIGFPE - FPE_FLTINV (at 0x'), lines[0]
    assert lines[0].endswith(').'), lines[0]
    assert lines[1] == 'Callstack:'
    assert lines[3].endswith('my_fpe()'), lines[2]
    assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
    assert exitcode == -8
