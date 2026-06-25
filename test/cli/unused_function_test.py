
# python3 -m pytest test-unused_function_test.py

import os
import json
import sys
import pytest
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

__project_dir = os.path.join(__script_dir, 'unusedFunction')
__project_dir_sep = __project_dir + os.path.sep


# TODO: make this a generic helper function
def __create_compdb(tmpdir, projpath):
    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    files = []
    for f in os.listdir(projpath):
        files.append(f)
    files.sort()
    j = []
    for f in files:
        j.append({
            'directory': projpath,
            'file': os.path.join(projpath, f),
            'command': 'gcc -c {}'.format(f)
        })
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j, indent=4))
    return compile_commands


def __test_unused_functions(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=unusedFunction',
        '--inline-suppr',
        __project_dir
    ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:6: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions():
    __test_unused_functions(['-j1', '--no-cppcheck-build-dir'])


def test_unused_functions_j():
    args = [
        '-q',
        '--template=simple',
        '--enable=unusedFunction',
        '--inline-suppr',
        '-j2',
        '--no-cppcheck-build-dir',
        __project_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_unused_functions_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions(['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


def test_unused_functions_builddir_j_thread(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions(['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=thread'])


@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_functions_builddir_j_process(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions(['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=process'])


def __test_unused_functions_project(extra_args):
    project_file = os.path.join(__project_dir, 'unusedFunction.cppcheck')
    args = [
        '-q',
        '--template=simple',
        '--enable=unusedFunction',
        '--inline-suppr',
        '--project={}'.format(project_file),
    ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    assert [
        "{}3.c:3:6: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_project():
    __test_unused_functions_project(['-j1', '--no-cppcheck-build-dir'])


def test_unused_functions_project_j():
    project_file = os.path.join(__project_dir, 'unusedFunction.cppcheck')
    args = [
        '-q',
        '--template=simple',
        '--enable=unusedFunction',
        '--inline-suppr',
        '--project={}'.format(project_file),
        '-j2',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."
    ]
    assert [] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_project_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions_project(['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


def test_unused_functions_project_builddir_j_thread(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions_project(['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=thread'])


@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_functions_project_builddir_j_process(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions_project(['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=process'])


def __test_unused_functions_compdb(tmpdir, extra_args):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    args = [
        '-q',
        '--template=simple',
        '--enable=unusedFunction',
        '--inline-suppr',
        '--project={}'.format(compdb_file)
    ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:6: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions_compdb(tmpdir):
    __test_unused_functions_compdb(tmpdir, ['-j1', '--no-cppcheck-build-dir'])


def test_unused_functions_compdb_j(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    args = [
        '-q',
        '--template=simple',
        '--enable=unusedFunction',
        '--inline-suppr',
        '--project={}'.format(compdb_file),
        '-j2',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_unused_functions_compdb_buildir_j_thread(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions_compdb(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=thread'])


@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_functions_compdb_builddir_j_process(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions_compdb(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=process'])