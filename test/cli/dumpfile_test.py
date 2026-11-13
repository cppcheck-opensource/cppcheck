# python -m pytest dumpfile_test.py

import json
import os
import pathlib

from testutils import cppcheck
import xml.etree.ElementTree as ET


def test_libraries(tmpdir):  #13701
    test_file = str(tmpdir / 'test.c')
    with open(test_file, 'wt') as f:
        f.write('x=1;\n')

    args = ['--library=posix', '--dump', test_file]
    _, _, _ = cppcheck(args)

    dumpfile = test_file + '.dump'
    assert os.path.isfile(dumpfile)
    with open(dumpfile, 'rt') as f:
        dump = f.read()
    assert '<library lib="posix"/>' in dump
    assert dump.find('<library lib="posix"/>') < dump.find('<dump cfg=')


def __test_language(tmp_path, file_ext, exp_lang, force_lang=None):
    test_file = tmp_path / ('test.' + file_ext)
    with open(test_file, 'wt'):
        pass

    args = [
        '--dump',
        str(test_file)
    ]
    if force_lang:
        args += ['--language=' + force_lang]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr

    dump_s = pathlib.Path(str(test_file) + '.dump').read_text()

    dump_xml = ET.fromstring(dump_s)
    assert 'language' in dump_xml.attrib
    assert dump_xml.attrib['language'] == exp_lang


def test_language_c(tmp_path):
    __test_language(tmp_path, 'c', exp_lang='c')


def test_language_c_force_c(tmp_path):
    __test_language(tmp_path, 'c', force_lang='c', exp_lang='c')


def test_language_c_force_cpp(tmp_path):
    __test_language(tmp_path, 'c', force_lang='c++', exp_lang='cpp')


def test_language_cpp(tmp_path):
    __test_language(tmp_path, 'cpp', exp_lang='cpp')


def test_language_cpp_force_cpp(tmp_path):
    __test_language(tmp_path, 'cpp', force_lang='c++', exp_lang='cpp')


def test_language_cpp_force_c(tmp_path):
    __test_language(tmp_path, 'cpp', force_lang='c', exp_lang='c')


# headers default to C
def test_language_h(tmp_path):
    __test_language(tmp_path, 'h', exp_lang='c')


def test_language_h_force_c(tmp_path):
    __test_language(tmp_path, 'h', force_lang='c', exp_lang='c')


def test_language_h_force_cpp(tmp_path):
    __test_language(tmp_path, 'h', force_lang='c++', exp_lang='cpp')


# files with unknown extensions default to C++
def test_language_unk(tmp_path):
    __test_language(tmp_path, 'src', exp_lang='cpp')


def test_language_unk_force_c(tmp_path):
    __test_language(tmp_path, 'src', force_lang='c', exp_lang='c')


def test_language_unk_force_cpp(tmp_path):
    __test_language(tmp_path, 'src', force_lang='c++', exp_lang='cpp')


def test_duplicate_file_entries(tmpdir):  #13333
    test_file = str(tmpdir / 'test.c')
    with open(test_file, 'wt') as f:
        f.write('x=1;\n')

    project_file = str(tmpdir / 'compile_commands.json')
    with open(project_file, 'wt') as f:
        f.write(json.dumps([{
               "file": test_file,
               "directory": str(tmpdir),
               "command": "cc -c test.c"
            },{
               "file": test_file,
               "directory": str(tmpdir),
               "command": "cc -c test.c"
            }]))

    args = ['--project=compile_commands.json', '--dump']
    _, _, _ = cppcheck(args, cwd=str(tmpdir))

    assert os.path.isfile(test_file + '.dump')
    assert os.path.isfile(test_file + '.1.dump')


def test_container_methods(tmpdir):
    test_file = str(tmpdir / 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write('std::string s;\n')

    exitcode, _, stderr = cppcheck(['--dump', '.'], cwd=str(tmpdir))
    assert exitcode == 0, stderr

    dumpfile = test_file + '.dump'
    with open(dumpfile, 'rt') as f:
        dump = f.read()
    assert '<f name="emplace" action="push" yield="iterator"/>' in dump

def test_line_macro(tmpdir):
    test_file = str(tmpdir / 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write('#define MACRO() __LINE__\n')
        f.write('int x = MACRO();\n')

    exitcode, _, stderr = cppcheck(['--dump', test_file])
    assert exitcode == 0, stderr

    dumpfile = test_file + '.dump'
    with open(dumpfile, 'rt') as f:
        dump = f.read()
    assert 'type="number" isInt="true" macroName="__LINE__"' in dump


def test_build_dir_project_configurations(tmp_path):
    test_file = tmp_path / 'main.c'
    with open(test_file, 'wt') as f:
        f.write('int main(void) { return 0; }\n')

    configs = ''.join(
        f'<ProjectConfiguration Include="{c}|{p}"><Configuration>{c}</Configuration><Platform>{p}</Platform></ProjectConfiguration>'
        for c in ('Debug', 'Release') for p in ('Win32', 'x64'))
    project_file = tmp_path / 'test.vcxproj'
    with open(project_file, 'wt') as f:
        f.write('<?xml version="1.0" encoding="utf-8"?>'
                '<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">'
                f'<ItemGroup Label="ProjectConfigurations">{configs}</ItemGroup>'
                '<ItemGroup><ClCompile Include="main.c" /></ItemGroup>'
                '</Project>')

    build_dir = tmp_path / 'b'
    os.mkdir(build_dir)

    args = ['-q', '--project=test.vcxproj', '--cppcheck-build-dir=b', '--addon=misra']
    exitcode, stdout, stderr = cppcheck(args, cwd=str(tmp_path))
    assert exitcode == 0, stdout
    assert stderr == ''

    files = os.listdir(build_dir)
    assert 'main.c.analyzerinfo.ctu-info' not in files
    for i in range(1, 5):
        assert f'main.a{i}.ctu-info' in files
