
# python -m pytest inline-suppress_test.py

import json
import os
import pytest
import sys
import time
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_inline_suppres_path = 'proj-inline-suppress' + os.path.sep


def __create_unused_function_compile_commands(tmpdir):
    prjpath = os.path.realpath(os.path.join(__script_dir, 'proj-inline-suppress-unusedFunction'))
    j = [{'directory': prjpath,
          'command': '/usr/bin/c++ -I"' + prjpath + '" -o "' + os.path.join(prjpath, 'B.cpp.o') + '" -c "' + os.path.join(prjpath, 'B.cpp') + '"',
          'file': os.path.join(prjpath, 'B.cpp')},
         {'directory': prjpath,
          'command': '/usr/bin/c++ -I"' + prjpath + '" -o "' + os.path.join(prjpath, 'A.cpp.o') + '" -c "' + os.path.join(prjpath, 'A.cpp') + '"',
          'file': os.path.join(prjpath, 'A.cpp')}]
    compdb_path = os.path.join(tmpdir, 'proj-inline-suppress-unusedFunction')
    os.makedirs(compdb_path)
    compile_commands = os.path.join(compdb_path, 'compile_commands.json')
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j, indent=4))
    return compile_commands


def test_1():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        'proj-inline-suppress'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr == ''
    assert stdout == ''
    assert ret == 0, stdout


def test_2():
    args = [
        '-q',
        '--template=simple',
        'proj-inline-suppress'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}3.cpp:4:19: error: Division by zero. [zerodiv]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_xml():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '--xml-version=3',
        'proj-inline-suppress'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert '<suppression errorId="some_warning_id" fileName="proj-inline-suppress/2.c" lineNumber="2" inline="true" comment="there should be a unmatchedSuppression warning about this" />' in stderr
    assert stdout == ''
    assert ret == 0, stdout


def test_unmatched_suppression():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '--enable=information',
        '--error-exitcode=1',
        '{}2.c'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}2.c:2:1: information: Unmatched suppression: some_warning_id [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_unmatched_suppression_path_with_extra_stuff():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '--enable=information',
        '--error-exitcode=1',
        '{}2.c'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}2.c:2:1: information: Unmatched suppression: some_warning_id [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_backwards_compatibility():
    args = [
        '-q',
        '--template=simple',
        '{}3.cpp'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}3.cpp:4:19: error: Division by zero. [zerodiv]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout

    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '{}3.cpp'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def __test_compile_commands_unused_function(tmpdir, use_j):
    compdb_file = __create_unused_function_compile_commands(tmpdir)
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--error-exitcode=1',
        '--project={}'.format(compdb_file)
    ]
    if use_j:
        args.append('-j2')
    else:
        args.append('-j1')
    ret, stdout, stderr = cppcheck(args)
    proj_path_sep = os.path.join(__script_dir, 'proj-inline-suppress-unusedFunction') + os.path.sep
    lines = stderr.splitlines()
    assert lines == [
        "{}B.cpp:6:9: style: The function 'unusedFunctionTest' is never used. [unusedFunction]".format(proj_path_sep)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_compile_commands_unused_function(tmpdir):
    __test_compile_commands_unused_function(tmpdir, False)


@pytest.mark.skip  # unusedFunction does not work with -j
def test_compile_commands_unused_function_j(tmpdir):
    __test_compile_commands_unused_function(tmpdir, True)


def __test_compile_commands_unused_function_suppression(tmpdir, use_j):
    compdb_file = __create_unused_function_compile_commands(tmpdir)
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        '--error-exitcode=1',
        '--project={}'.format(compdb_file)
    ]
    if use_j:
        args.append('-j2')
    else:
        args.append('-j1')
    ret, stdout, stderr = cppcheck(args)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_compile_commands_unused_function_suppression(tmpdir):
    __test_compile_commands_unused_function_suppression(tmpdir, False)


@pytest.mark.skip  # unusedFunction does not work with -j
def test_compile_commands_unused_function_suppression_j(tmpdir):
    __test_compile_commands_unused_function_suppression(tmpdir, True)


def test_unmatched_suppression_ifdef():
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--inline-suppr',
        '-DNO_ZERO_DIV',
        'trac5704/trac5704a.c'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_unmatched_suppression_ifdef_0():
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--inline-suppr',
        'trac5704/trac5704b.c'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_build_dir(tmpdir):
    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(tmpdir),
        '--enable=all',
        '--inline-suppr',
        '{}4.c'.format(__proj_inline_suppres_path)
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout

def test_build_dir_jobs_suppressions(tmpdir): #14064
    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(tmpdir),
        '--enable=style',
        '--inline-suppr',
        '-j4',
        'reanalysis'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout

    a1Path = os.path.join(tmpdir, 'd.a1')
    assert os.path.exists(a1Path)
    mtimeOld = os.path.getmtime(a1Path)

    time.sleep(1)

    for _ in range(1, 10):
        cppcheck(args, cwd=__script_dir)

    mtimeNew = os.path.getmtime(a1Path)
    assert mtimeOld == mtimeNew


def __test_build_dir_unused_template(tmpdir, extra_args):
    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(tmpdir),
        '--enable=all',
        '--inline-suppr',
        '{}template.cpp'.format(__proj_inline_suppres_path)
    ]

    args = args + extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_build_dir_unused_template(tmpdir):
    __test_build_dir_unused_template(tmpdir, ['-j1', '--no-cppcheck-build-dir'])


def test_build_dir_unused_template_j_thread(tmpdir):
    __test_build_dir_unused_template(tmpdir, ['-j2', '--executor=thread'])


@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_build_dir_unused_template_j_process(tmpdir):
    __test_build_dir_unused_template(tmpdir, ['-j2', '--executor=process'])


def test_suppress_unmatched_inline_suppression():  # 11172
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--suppress=unmatchedSuppression',
        '--inline-suppr',
        '{}2.c'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


@pytest.mark.skip  # TODO: this test makes no sense
@pytest.mark.xfail(strict=True)  # no error as inline suppressions are currently not being propagated back
def test_duplicate(tmpdir):
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        'proj-inline-suppress/duplicate.cpp'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == []
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'unreadVariable' already exists"
    ]
    assert ret == 0, stdout


# no error as inline suppressions are handled separately
def __test_duplicate_cmd(tmpdir, extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        '--suppress=unreadVariable',
        'proj-inline-suppress/4.c'
    ]

    args = args + extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    # this is the suppression provided via the command-line which is unused because only the inline suppression is being matched
    assert lines == [
        'nofile:0:0: information: Unmatched suppression: unreadVariable [unmatchedSuppression]'
    ]
    assert stdout == ''
    assert ret == 0, stdout


@pytest.mark.skip  # TODO: behavior of duplicate suppressions across inline and non-inline is currently undefined
def test_duplicate_cmd(tmp_path):
    __test_duplicate_cmd(tmp_path, ['-j1'])


@pytest.mark.skip  # TODO: behavior of duplicate suppressions across inline and non-inline is currently undefined
def test_duplicate_cmd_j(tmp_path):
    __test_duplicate_cmd(tmp_path, ['-j2'])


# no error as inline suppressions are handled separately
def __test_duplicate_file(tmp_path, extra_args):
    suppr_file =  tmp_path / 'suppressions'
    with open(suppr_file, 'wt') as f:
        f.write('unreadVariable')

    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        '--suppressions-list={}'.format(suppr_file),
        'proj-inline-suppress/4.c'
    ]

    args = args + extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    # this is the suppression provided via the suppression file which is unused because only the inline suppression is being matched
    assert lines == [
        'nofile:0:0: information: Unmatched suppression: unreadVariable [unmatchedSuppression]'
    ]
    assert stdout == ''
    assert ret == 0, stdout


@pytest.mark.skip  # TODO: behavior of duplicate suppressions across inline and non-inline is currently undefined
def test_duplicate_file(tmpdir):
    __test_duplicate_file(tmpdir, ['-j1'])


@pytest.mark.skip  # TODO: behavior of duplicate suppressions across inline and non-inline is currently undefined
def test_duplicate_file_j(tmpdir):
    __test_duplicate_file(tmpdir, ['-j2'])


# reporting of inline unusedFunction is deferred
def __test_unused_function_unmatched(tmpdir, extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        'proj-inline-suppress/unusedFunctionUnmatched.cpp'
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    lines.sort()
    assert lines == [
        '{}unusedFunctionUnmatched.cpp:5:1: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path),
        '{}unusedFunctionUnmatched.cpp:5:1: information: Unmatched suppression: unusedFunction [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_unused_function_unmatched(tmpdir):
    __test_unused_function_unmatched(tmpdir, ['-j1', '--no-cppcheck-build-dir'])


@pytest.mark.xfail(strict=True)  # TODO: check error - do not work with -j2
def test_unused_function_unmatched_j(tmpdir):
    __test_unused_function_unmatched(tmpdir, ['-j2', '--no-cppcheck-build-dir'])


def test_unused_function_unmatched_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_function_unmatched(tmpdir, ['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


def test_unused_function_unmatched_builddir_j_thread(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_function_unmatched(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=thread'])


@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_function_unmatched_builddir_j_process(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_function_unmatched(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir), '--executor=process'])


# do not report unmatched unusedFunction inline suppressions when unusedFunction check is disabled
def test_unused_function_disabled_unmatched():
    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--inline-suppr',
        'proj-inline-suppress/unusedFunctionUnmatched.cpp'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == [
        '{}unusedFunctionUnmatched.cpp:5:1: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_unmatched_cfg():
    # make sure we do not report unmatched inline suppressions from inactive code blocks
    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--inline-suppr',
        'proj-inline-suppress/cfg.c'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == [
        '{}cfg.c:5:5: information: Unmatched suppression: id [unmatchedSuppression]'.format(__proj_inline_suppres_path),
        '{}cfg.c:9:5: information: Unmatched suppression: id [unmatchedSuppression]'.format(__proj_inline_suppres_path),
    ]
    assert stdout == ''
    assert ret == 0, stdout


# do not report unmatched unusedFunction inline suppressions when unusedFunction check is disabled
# unusedFunction is disabled when -j2 is specified without a builddir
def test_unused_function_disabled_unmatched_j():
    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--inline-suppr',
        '-j2',
        '--no-cppcheck-build-dir',
        'proj-inline-suppress/unusedFunctionUnmatched.cpp'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == [
        '{}unusedFunctionUnmatched.cpp:5:1: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


# do not report unmatched misra-* inline suppressions when misra is not provided
def test_misra_disabled_unmatched():  #14232
    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--inline-suppr',
        'proj-inline-suppress/misraUnmatched.c'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == [
        '{}misraUnmatched.c:5:1: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


# do not report unmatched premium-* inline suppressions when application is not premium
def test_premium_disabled_unmatched():  #13663
    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--inline-suppr',
        'proj-inline-suppress/premiumUnmatched.cpp'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == [
        '{}premiumUnmatched.cpp:5:1: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout