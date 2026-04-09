
# python -m pytest test-helloworld.py

import os
import re
import glob
import json
import shutil
import xml.etree.ElementTree as ET

import pytest

from testutils import create_gui_project_file, cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'helloworld')


# Get Visual Studio configurations checking a file
# Checking {file} {config}...
def __getVsConfigs(stdout, filename):
    ret = []
    for line in stdout.split('\n'):
        if not line.startswith('Checking %s ' % filename):
            continue
        if not line.endswith('...'):
            continue
        res = re.match(r'.* ([A-Za-z0-9|]+)...', line)
        if res:
            ret.append(res.group(1))
    ret.sort()
    return ' '.join(ret)

def test_relative_path():
    args = [
        '--template=cppcheck1',
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename


def test_local_path():
    args = [
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_absolute_path():
    args = [
        '--template=cppcheck1',
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_addon_local_path():
    args = [
        '--addon=misra',
        '--enable=style',
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert stderr == ('[main.c:5]: (error) Division by zero.\n'
                      '[main.c:4]: (style) misra violation (use --rule-texts=<file> to get proper output)\n')

def test_addon_local_path_not_enable():
    args = [
        '--addon=misra',
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_addon_absolute_path():
    args = [
        '--addon=misra',
        '--enable=style',
        '--template=cppcheck1',
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:4]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_relative_path():
    args = [
        '--addon=misra',
        '--enable=style',
        '--template=cppcheck1',
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stdout == ('Checking %s ...\n'
                      'Checking %s: SOME_CONFIG=SOME_CONFIG...\n' % (filename, filename))
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:4]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_with_gui_project(tmp_path):
    shutil.copytree(os.path.join(__script_dir, 'helloworld'), tmp_path / 'helloworld')
    project_file = os.path.join('helloworld', 'test.cppcheck')
    create_gui_project_file(tmp_path / project_file, paths=['.'], addon='misra')
    args = [
        '--template=cppcheck1',
        '--enable=style',
        '--project={}'.format(project_file)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=tmp_path)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stdout.strip().split('\n') == [
        'Checking %s ...' % filename,
        'Checking %s: SOME_CONFIG=SOME_CONFIG...' % filename
    ]
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:4]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_basepath_relative_path():
    args = [
        'helloworld',
        '--template=cppcheck1',
        '-rp=helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_basepath_absolute_path():
    args = [
        '--template=cppcheck1',
        __proj_dir,
        '-rp=' + __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def __test_vs_project_local_path(extra_args=None, exp_vs_cfg='Debug|Win32 Debug|x64 Release|Win32 Release|x64'):
    args = [
        '--template=cppcheck1',
        '--project=helloworld.vcxproj'
    ]
    if extra_args:
        args += extra_args
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert __getVsConfigs(stdout, 'main.c') == exp_vs_cfg
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_vs_project_local_path():
    __test_vs_project_local_path()

def test_vs_project_local_path_select_one():
    __test_vs_project_local_path(['--project-configuration=Release|Win32'], 'Release|Win32')

def test_vs_project_local_path_select_one_multiple():
    __test_vs_project_local_path(['--project-configuration=Debug|Win32', '--project-configuration=Release|Win32'], 'Release|Win32')

def test_vs_project_local_path_no_analyze_all():
    __test_vs_project_local_path(['--no-analyze-all-vs-configs'], 'Debug|Win32')

def test_vs_project_relative_path():
    args = [
        '--template=cppcheck1',
        '--project=' + os.path.join(__proj_dir, 'helloworld.vcxproj')
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert __getVsConfigs(stdout, filename) == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_vs_project_absolute_path():
    args = [
        '--template=cppcheck1',
        '--project=' + os.path.join(__proj_dir, 'helloworld.vcxproj')
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert __getVsConfigs(stdout, filename) == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def __test_cppcheck_project_local_path(extra_args=None, exp_vs_cfg='Debug|x64'):
    args = [
        '--template=cppcheck1',
        '--platform=win64',
        '--project=helloworld.cppcheck'
    ]
    if extra_args:
        args += extra_args
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert __getVsConfigs(stdout, 'main.c') == exp_vs_cfg
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_cppcheck_project_local_path():
    __test_cppcheck_project_local_path()

@pytest.mark.xfail  # TODO: no source files found
def test_cppcheck_project_local_path_select_one():
    __test_cppcheck_project_local_path(['--project-configuration=Release|Win32'], 'Release|Win32')

@pytest.mark.xfail  # TODO: no source files found
def test_cppcheck_project_local_path_select_one_multiple():
    __test_cppcheck_project_local_path(['--project-configuration=Debug|Win32', '--project-configuration=Release|Win32'], 'Release|Win32')

def test_cppcheck_project_local_path_analyze_all():
    __test_cppcheck_project_local_path(['--analyze-all-vs-configs'], 'Debug|Win32 Debug|x64 Release|Win32 Release|x64')

@pytest.mark.parametrize("project_file", ["helloworld.cppcheck", "helloworld_slnx.cppcheck"])
def test_cppcheck_project_relative_path(project_file):
    args = [
        '--template=cppcheck1',
        '--platform=win64',
        '--project=' + os.path.join('helloworld', project_file)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert __getVsConfigs(stdout, filename) == 'Debug|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

@pytest.mark.parametrize("project_file", ["helloworld.cppcheck", "helloworld_slnx.cppcheck"])
def test_cppcheck_project_absolute_path(project_file):
    args = [
        '--template=cppcheck1',
        '--platform=win64',
        '--project=' + os.path.join(__proj_dir, project_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert __getVsConfigs(stdout, filename) == 'Debug|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_suppress_command_line_related():
    args = [
        '--suppress=zerodiv:' + os.path.join('helloworld', 'main.c'),
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == ''

def test_suppress_command_line_absolute():
    args = [
        '--suppress=zerodiv:' + os.path.join(__proj_dir, 'main.c'),
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    assert stderr == ''

def test_suppress_project_relative(tmp_path):
    shutil.copytree(os.path.join(__script_dir, 'helloworld'), tmp_path / 'helloworld')
    project_file = os.path.join('helloworld', 'test.cppcheck')
    create_gui_project_file(tmp_path / project_file,
                            paths=['.'],
                            suppressions=[{'fileName':'main.c', 'id':'zerodiv'}])

    args = [
        '--project={}'.format(project_file)
    ]

    ret, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert ret == 0, stdout
    assert stderr == ''


def test_suppress_project_absolute(tmp_path):
    shutil.copytree(os.path.join(__script_dir, 'helloworld'), tmp_path / 'helloworld')
    project_file = tmp_path / 'helloworld' / 'test.cppcheck'
    create_gui_project_file(project_file,
                            paths=['.'],
                            suppressions=[{'fileName':'main.c', 'id':'zerodiv'}])

    args = [
        '--project={}'.format(project_file)
    ]

    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    assert stderr == ''

@pytest.mark.parametrize("project_file", ["helloworld.cppcheck", "helloworld_slnx.cppcheck"])
def test_exclude(project_file):
    args = [
        '-i' + 'helloworld',
        '--platform=win64',
        '--project=' + os.path.join('helloworld', project_file)
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 1
    lines = stdout.splitlines()
    assert lines == [
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]


def test_build_dir_dump_output(tmpdir):
    args = [
        f'--cppcheck-build-dir={tmpdir}',
        '--addon=misra',
        'helloworld'
    ]

    cppcheck(args)
    cppcheck(args)

    filename = f'{tmpdir}/main.a1.*.dump'
    filelist = glob.glob(filename)
    assert(len(filelist) == 0)


def test_checkers_report(tmpdir):
    filename = os.path.join(tmpdir, '1.txt')
    args = [
        f'--checkers-report={filename}',
        '--no-cppcheck-build-dir',  # TODO: remove this
        'helloworld'
    ]

    cppcheck(args, cwd=__script_dir)

    with open(filename, 'rt') as f:
        data = f.read().splitlines()
        assert 'No   CheckAutoVariables::assignFunctionArg                     require:style,warning' in data, json.dumps(data, indent=4)
        assert 'Yes  CheckAutoVariables::autoVariables' in data, json.dumps(data, indent=4)

    args += [
        '--enable=style'
    ]
    cppcheck(args, cwd=__script_dir)
    with open(filename, 'rt') as f:
        data = f.read().splitlines()
        # checker has been activated by --enable=style
        assert 'Yes  CheckAutoVariables::assignFunctionArg' in data, json.dumps(data, indent=4)


def test_missing_include_system():  # #11283
    args = [
        '--enable=missingInclude',
        '--suppress=zerodiv',
        '--template=simple',
        'helloworld'
    ]

    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.replace('\\', '/') == 'helloworld/main.c:1:2: information: Include file: <stdio.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n'


def test_sarif():
    args = [
        '--output-format=sarif',
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    res = json.loads(stderr)
    assert res['version'] == '2.1.0'
    assert res['runs'][0]['results'][0]['locations'][0]['physicalLocation']['artifactLocation']['uri'] == 'helloworld/main.c'
    assert res['runs'][0]['results'][0]['ruleId'] == 'zerodiv'
    assert res['runs'][0]['tool']['driver']['rules'][0]['id'] == 'zerodiv'
    assert res['runs'][0]['tool']['driver']['rules'][0]['properties']['precision'] == 'high'
    assert res['runs'][0]['tool']['driver']['rules'][0]['properties']['security-severity'] == '9.9'
    assert 'security' in res['runs'][0]['tool']['driver']['rules'][0]['properties']['tags']
    assert re.match(r'[0-9]+(.[0-9]+)+', res['runs'][0]['tool']['driver']['semanticVersion'])
    assert 'level' in res['runs'][0]['tool']['driver']['rules'][0]['defaultConfiguration'] # #13885
    assert res['runs'][0]['tool']['driver']['rules'][0]['shortDescription']['text'] == ''
    assert res['runs'][0]['results'][0]['message']['text'] == 'Division by zero.'
    assert res['runs'][0]['tool']['driver']['rules'][0]['properties']['problem.severity'] == 'error'


def test_xml_checkers_report():
    test_file = os.path.join(__proj_dir, 'main.c')
    args = ['--xml-version=3', '--enable=all', test_file]

    exitcode, _, stderr = cppcheck(args)
    assert exitcode == 0
    assert ET.fromstring(stderr) is not None
