import glob
import os
import pytest
import json
import shutil
from testutils import cppcheck
import xml.etree.ElementTree as ET

__script_dir = os.path.dirname(os.path.abspath(__file__))

# TODO: use dedicated addon
# TODO: test CheckNullPointer
# TODO: test CheckUninitVar
# TODO: test CheckBufferOverrun


def __create_compile_commands(dir, entries):
    j = []
    for e in entries:
        f = os.path.basename(e)
        obj = {
            'directory': os.path.dirname(os.path.abspath(e)),
            'command': 'gcc -c {}'.format(f),
            'file': f
        }
        j.append(obj)
    compile_commands = os.path.join(dir, 'compile_commmands.json')
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j))
    return compile_commands


def test_addon_suppress_inline():
    args = [
        '-q',
        '--addon=misra',
        '--template=simple',
        '--enable=information,style',
        '--inline-suppr',
        '--error-exitcode=1',
        'whole-program/whole1.c',
        'whole-program/whole2.c'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_addon_suppress_inline_project(tmpdir):
    compile_db = __create_compile_commands(tmpdir, [
        os.path.join(__script_dir, 'whole-program', 'whole1.c'),
        os.path.join(__script_dir, 'whole-program', 'whole2.c')
    ])

    args = [
        '-q',
        '--addon=misra',
        '--template=simple',
        '--enable=information,style',
        '--inline-suppr',
        '--error-exitcode=1',
        '--project={}'.format(compile_db)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


# TODO: remove overrides when this is fully working
def __test_suppress_inline(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--inline-suppr',
        '--error-exitcode=1',
        'whole-program/odr1.cpp',
        'whole-program/odr2.cpp'
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_suppress_inline():
    __test_suppress_inline(['-j1', '--no-cppcheck-build-dir'])


@pytest.mark.xfail(strict=True)
def test_suppress_inline_j():
    __test_suppress_inline(['-j2', '--no-cppcheck-build-dir'])


def test_suppress_inline_builddir(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline(['--cppcheck-build-dir={}'.format(build_dir), '-j1'])


def test_suppress_inline_builddir_cached(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline(['--cppcheck-build-dir={}'.format(build_dir), '-j1'])
    __test_suppress_inline(['--cppcheck-build-dir={}'.format(build_dir), '-j1'])


def test_suppress_inline_builddir_j(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline(['--cppcheck-build-dir={}'.format(build_dir), '-j2'])


def test_inline_suppr_builddir_j_cached(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline(['--cppcheck-build-dir={}'.format(build_dir), '-j2'])
    __test_suppress_inline(['--cppcheck-build-dir={}'.format(build_dir), '-j2'])


# TODO: remove overrides when it is fully working
def __test_suppress_inline_project(tmp_path, extra_args):
    compile_db = __create_compile_commands(str(tmp_path), [
        os.path.join(__script_dir, 'whole-program', 'odr1.cpp'),
        os.path.join(__script_dir, 'whole-program', 'odr2.cpp')
    ])

    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--inline-suppr',
        '--error-exitcode=1',
        '--project={}'.format(compile_db)
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout



def test_suppress_inline_project(tmp_path):
    __test_suppress_inline_project(tmp_path, ['-j1', '--no-cppcheck-build-dir'])


@pytest.mark.xfail(strict=True)
def test_suppress_inline_project_j(tmp_path):
    __test_suppress_inline_project(tmp_path, ['-j2', '--no-cppcheck-build-dir'])


def test_suppress_inline_project_builddir(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline_project(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j1'])


def test_suppress_inline_project_builddir_cached(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline_project(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j1'])
    __test_suppress_inline_project(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j1'])


def test_suppress_inline_project_builddir_j(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline_project(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j2'])


def test_suppress_inline_project_builddir_j_cached(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_suppress_inline_project(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j2'])
    __test_suppress_inline_project(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j2'])


@pytest.mark.parametrize("builddir", (False,True))
def test_addon_rerun(tmp_path, builddir):
    """Rerun analysis and ensure that misra CTU works; with and without build dir"""
    args = [
        '--addon=misra',
        '--enable=style',
        '--template={id}',
        'whole-program']
    # do not use the injection because the directory needs to survive runs
    if builddir:
        args.append('--cppcheck-build-dir=' + str(tmp_path))
    else:
        args.append('--no-cppcheck-build-dir')
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr

def test_addon_builddir_use_ctuinfo(tmp_path):
    """Test that ctu-info files are used when builddir is used"""
    args = [
        '--cppcheck-build-dir=' + str(tmp_path),
        '--addon=misra',
        '--enable=style',
        '--template={id}',
        'whole-program']
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr
    with open(tmp_path / 'whole1.a1.ctu-info', 'wt'):
        pass
    with open(tmp_path / 'whole2.a1.ctu-info', 'wt'):
        pass
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' not in stderr


@pytest.mark.parametrize("builddir", (False,True))
def test_addon_no_artifacts(tmp_path, builddir):
    """Test that there are no artifacts left after analysis"""
    shutil.copyfile(os.path.join(__script_dir, 'whole-program', 'whole1.c'), tmp_path / 'whole1.c')
    shutil.copyfile(os.path.join(__script_dir, 'whole-program', 'whole2.c'), tmp_path / 'whole2.c')
    build_dir = str(tmp_path / 'b1')
    os.mkdir(build_dir)
    args = [
        '--addon=misra',
        '--enable=style',
        '--template={id}',
        str(tmp_path)]
    if builddir:
        args.append('--cppcheck-build-dir=' + build_dir)
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr
    files = []
    for f in glob.glob(str(tmp_path / '*')):
        if os.path.isfile(f):
            files.append(os.path.basename(f))
    files.sort()
    assert ' '.join(files) == 'whole1.c whole2.c'


def __test_checkclass(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--error-exitcode=1',
        'whole-program/odr1.cpp',
        'whole-program/odr2.cpp',
        'whole-program/odr3.cpp'
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        "whole-program{}odr1.cpp:6:1: error: The one definition rule is violated, different classes/structs have the same name 'C' [ctuOneDefinitionRuleViolation]".format(os.path.sep)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_checkclass():
    __test_checkclass(['-j1'])


@pytest.mark.xfail(strict=True) # TODO: check error
def test_checkclass_j():
    __test_checkclass(['-j2', '--no-cppcheck-build-dir'])


def test_checkclass_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass(['--cppcheck-build-dir={}'.format(build_dir)])


def test_checkclass_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass(['-j2', '--cppcheck-build-dir={}'.format(build_dir)])


def __test_checkclass_project(tmpdir, extra_args):
    odr_file_1 = os.path.join(__script_dir, 'whole-program', 'odr1.cpp')

    compile_db = __create_compile_commands(tmpdir, [
        odr_file_1,
        os.path.join(__script_dir, 'whole-program', 'odr2.cpp')
    ])

    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--error-exitcode=1',
        '--project={}'.format(compile_db)
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        "{}:6:1: error: The one definition rule is violated, different classes/structs have the same name 'C' [ctuOneDefinitionRuleViolation]".format(odr_file_1)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_checkclass_project(tmpdir):
    __test_checkclass_project(tmpdir, ['-j1'])


@pytest.mark.xfail(strict=True) # TODO: check error
def test_checkclass_project_j(tmpdir):
    __test_checkclass_project(tmpdir, ['-j2', '--no-cppcheck-build-dir'])


def test_checkclass_project_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass_project(tmpdir, ['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


def test_checkclass_project_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass_project(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir)])

def test_ctu_odr_config():
    args = [
        '-q',
        '-j1',
        '--template=simple',
        '--enable=information,style',
        '--error-exitcode=1',
        'whole-program/odr_cfg1.cpp',
        'whole-program/odr_cfg2.cpp'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        "whole-program{}odr_cfg1.cpp:2:1: error: The one definition rule is violated, different classes/structs have the same name 'S' [ctuOneDefinitionRuleViolation]".format(os.path.sep)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def __test_nullpointer_file0(extra_args):
    args = [
        '-q',
        '--xml',
        '--error-exitcode=1',
        'whole-program/nullpointer1.cpp'
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    results = ET.fromstring(stderr)
    file0 = ''
    for e in results.findall('errors/error[@id="ctunullpointer"]'):
        file0 = e.attrib['file0']

    assert ret == 1, stdout if stdout else stderr
    assert stdout == ''
    assert file0 == 'whole-program/nullpointer1.cpp', stderr


def test_nullpointer_file0():
    __test_nullpointer_file0(['-j1'])


@pytest.mark.xfail(strict=True) # no CTU without builddir
def test_nullpointer_file0_j():
    __test_nullpointer_file0(['-j2', '--no-cppcheck-build-dir'])


def test_nullpointer_file0_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_nullpointer_file0(['-j2', '--cppcheck-build-dir={}'.format(build_dir)])

# TODO: this only succeeded because it depedent on the bugged unqiue message handling
@pytest.mark.parametrize("single_file", [
    False,
    pytest.param(True, marks=pytest.mark.xfail(strict=True)),
])
def test_nullpointer_out_of_memory(tmpdir, single_file):
    """Ensure that there are not duplicate warnings related to memory/resource allocation failures
       https://trac.cppcheck.net/ticket/13521
    """
    code1 = 'void f(int* p) { *p = 0; }\n'
    code2 = 'int main() { int* p = malloc(10); f(p); return 0; }\n'
    if single_file:
        with open(tmpdir / 'test.c', 'wt') as f:
            f.write(code1 + code2)
    else:
        with open(tmpdir / 'header.h', 'wt') as f:
            f.write('void f(int* p);\n')
        with open(tmpdir / 'test1.c', 'wt') as f:
            f.write('#include "header.h"\n' + code1)
        with open(tmpdir / 'test2.c', 'wt') as f:
            f.write('#include "header.h"\n' + code2)
    args = [
        '--cppcheck-build-dir=.',
        '--enable=style',
        '.']
    _, _, stderr = cppcheck(args, cwd=tmpdir)
    results = []
    for line in stderr.splitlines():
        if line.endswith(']'):
            results.append(line[line.find('['):])

    if single_file:
        # the bug is found and reported using normal valueflow analysis
        # ctu finding is not reported
        assert results == ['[nullPointerOutOfMemory]']
    else:
        # the bug is found using ctu analysis
        assert results == ['[ctunullpointerOutOfMemory]']
