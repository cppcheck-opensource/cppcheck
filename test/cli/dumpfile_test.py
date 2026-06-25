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
