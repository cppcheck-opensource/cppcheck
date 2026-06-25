import subprocess
import os
import sys

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

def __call_process():
    exe = __lookup_cppcheck_exe('test-stacktrace')
    if exe is None:
        raise Exception('executable not found')
    with subprocess.Popen([exe], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        stdout, stderr = p.communicate()
        rc = p.returncode
    stdout = stdout.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = stderr.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    return rc, stdout, stderr


def test_stack():
    _, stdout, stderr = __call_process()
    assert stderr == ''
    lines = stdout.splitlines()
    assert lines[0] == 'Callstack:'
    assert lines[1].endswith('my_func_2()')
    assert lines[2].endswith('my_func()')
