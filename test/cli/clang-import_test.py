
# python -m pytest test-clang-import.py

import os
import re
import subprocess
import sys
import pytest
from testutils import cppcheck, assert_cppcheck

try:
    # TODO: handle exitcode?
    subprocess.call(['clang', '--version'])
except OSError:
    pytest.skip("'clang' does not exist", allow_module_level=True)


# the IDs differ with Visual Studio
if sys.platform == 'win32':
    pytest.skip(allow_module_level=True)


def __get_debug_section(title, stdout):
    s = re.sub(r'0x[0-9a-fA-F]+', '0x12345678', stdout)
    s = re.sub(r'nestedIn: Struct', 'nestedIn: Class', s)
    s = re.sub(r'classDef: struct', 'classDef: class', s)
    s = re.sub(r'isInline: [a-z]+', 'isInline: ---', s)
    s = re.sub(r'needInitialization: .*', 'needInitialization: ---', s)
    s = re.sub(r'functionOf: .*', 'functionOf: ---', s)
    s = re.sub(r'0x12345678 Struct', '0x12345678 Class', s)

    if title == '##AST':
        # TODO set types
        s = re.sub(r"return '[a-zA-Z0-9: *]+'", "return", s)

    pos1 = s.find(title)
    assert pos1 > 0, 'title not found'
    pos1 = s.find('\n', pos1) + 1
    assert pos1 > 0
    pos2 = s.find("\n##", pos1)
    if pos2 < 0:
        return s[pos1:]
    return s[pos1:pos2-1]


def __check_symbol_database(tmpdir, code):
    testfile = os.path.join(tmpdir, 'test.cpp')
    with open(testfile, 'w+t') as f:
        f.write(code)
    ret1, stdout1, _ = cppcheck(['-q', '--clang', '--debug-symdb', testfile])
    ret2, stdout2, _ = cppcheck(['-q', '--debug-symdb', testfile])
    assert 0 == ret1, stdout1
    assert 0 == ret2, stdout2
    assert __get_debug_section('### Symbol database', stdout1) == __get_debug_section('### Symbol database', stdout2)


def __check_ast(tmpdir, code):
    testfile = os.path.join(tmpdir, 'test.cpp')
    with open(testfile, 'w+t') as f:
        f.write(code)
    ret1, stdout1, _ = cppcheck(['-q', '--clang', '--debug-ast', testfile])
    ret2, stdout2, _ = cppcheck(['-q', '--debug-ast', testfile])
    assert 0 == ret1, stdout1
    assert 0 == ret2, stdout1
    assert __get_debug_section('##AST', stdout1) == __get_debug_section('##AST', stdout2)



def test_symbol_database_1(tmpdir):
    __check_symbol_database(tmpdir, 'int main(){return 0;}')

def test_symbol_database_2(tmpdir):
    __check_symbol_database(tmpdir, 'struct Foo { void f(); }; void Foo::f() {}')

def test_symbol_database_3(tmpdir):
    __check_symbol_database(tmpdir, 'struct Fred { int a; }; int b; void f(int c, int d) { int e; }')

def test_symbol_database_4(tmpdir):
    __check_symbol_database(tmpdir, 'void f(const int x) {}')

def test_symbol_database_5(tmpdir):
    __check_symbol_database(tmpdir, 'void f(int);')

def test_symbol_database_6(tmpdir):
    __check_symbol_database(tmpdir, 'inline static int foo(int x) { return x; }')

def test_symbol_database_7(tmpdir):
    __check_symbol_database(tmpdir, 'struct S {int x;}; void f(struct S *s) {}')

def test_symbol_database_class_access_1(tmpdir):
    __check_symbol_database(tmpdir, 'class Fred { void foo ( ) {} } ;')

def test_symbol_database_class_access_2(tmpdir):
    __check_symbol_database(tmpdir, 'class Fred { protected: void foo ( ) {} } ;')

def test_symbol_database_class_access_3(tmpdir):
    __check_symbol_database(tmpdir, 'class Fred { public: void foo ( ) {} } ;')

def test_symbol_database_operator(tmpdir):
    __check_symbol_database(tmpdir, 'struct Fred { void operator=(int x); };')

def test_symbol_database_struct_1(tmpdir):
    __check_symbol_database(tmpdir, 'struct S {};')

def test_ast_calculations(tmpdir):
    __check_ast(tmpdir, 'int x = 5; int y = (x + 4) * 2;')
    __check_ast(tmpdir, 'long long dostuff(int x) { return x ? 3 : 5; }')

def test_ast_control_flow(tmpdir):
    __check_ast(tmpdir, 'void foo(int x) { if (x > 5){} }')
    __check_ast(tmpdir, 'int dostuff() { for (int x = 0; x < 10; x++); }')
    __check_ast(tmpdir, 'void foo(int x) { switch (x) {case 1: break; } }')
    __check_ast(tmpdir, 'void foo(int a, int b, int c) { foo(a,b,c); }')

def test_ast(tmpdir):
    __check_ast(tmpdir, 'struct S { int x; }; S* foo() { return new S(); }')

def test_log(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--clang', test_file]
    out_lines = [
        'Checking {} ...'.format(test_file).replace('\\', '/'),
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_warning(tmpdir):  # #12424
    test_file = os.path.join(tmpdir, 'test_2')
    with open(test_file, 'wt') as f:
        f.write('''void f() {}''')

    exitcode, stdout, stderr = cppcheck(['-q', '--enable=warning', '--clang', test_file])
    assert exitcode == 0, stderr # do not assert
    assert stdout == ''
    assert stderr == ''


def __test_cmd(tmp_path, file_name, extra_args, stdout_exp_1, content=''):
    test_file = tmp_path / file_name
    with open(test_file, 'wt') as f:
        f.write(content)

    args = [
        '--enable=information',
        '--verbose',
        '--clang',
        file_name
    ]

    args += extra_args

    if stdout_exp_1:
        stdout_exp_1 += ' '

    exitcode, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert exitcode == 0, stderr if not stdout else stdout
    assert stderr == ''
    assert stdout.splitlines() == [
        'Checking {} ...'.format(file_name),
        'clang -fsyntax-only -Xclang -ast-dump -fno-color-diagnostics {}{}'.format(stdout_exp_1, file_name)
    ]


def test_cmd_c(tmp_path):
    __test_cmd(tmp_path, 'test.c', [], '-x c')


def test_cmd_cpp(tmp_path):
    __test_cmd(tmp_path, 'test.cpp', [], '-x c++')


# files with unknown extensions are treated as C++
def test_cmd_unk(tmp_path):
    __test_cmd(tmp_path, 'test.cplusplus', [], '-x c++')


# headers are treated as C by default
def test_cmd_hdr(tmp_path):
    __test_cmd(tmp_path, 'test.h', [], '-x c')


def test_cmd_hdr_probe(tmp_path):
    __test_cmd(tmp_path, 'test.h', ['--cpp-header-probe'], '-x c++', '// -*- C++ -*-')


def test_cmd_inc(tmp_path):
    inc_path = tmp_path / 'inc'
    os.makedirs(inc_path)
    __test_cmd(tmp_path, 'test.cpp',['-Iinc'], '-x c++ -Iinc/')


def test_cmd_def(tmp_path):
    __test_cmd(tmp_path, 'test.cpp',['-DDEF'], '-x c++ -DDEF=1')


def test_cmd_include(tmp_path):
    inc_file = tmp_path / 'inc.h'
    with open(inc_file, 'wt'):
        pass
    __test_cmd(tmp_path, 'test.cpp',['--include=inc.h'], '-x c++ --include inc.h')


def test_cmd_enforce_c(tmp_path):  # #13128
    __test_cmd(tmp_path, 'test.cpp',['-x', 'c'], '-x c')


def test_cmd_enforce_cpp(tmp_path):  # #13128
    __test_cmd(tmp_path, 'test.c',['-x', 'c++'], '-x c++')


def test_cmd_std_c(tmp_path):  # #13129
    __test_cmd(tmp_path, 'test.cpp',['--std=c89', '--std=c++14'], '-x c++ -std=c++14')


# TODO: remove when we inject the build-dir into all tests
def test_cmd_std_c_builddir(tmp_path):  # #13129
    build_dir = tmp_path / 'b1'
    os.makedirs(build_dir)
    __test_cmd(tmp_path, 'test.cpp',['--std=c89', '--std=c++14', '--cppcheck-build-dir={}'.format(build_dir)], '-x c++ -std=c++14')


def test_cmd_std_cpp(tmp_path):  # #13129
    __test_cmd(tmp_path, 'test.c',['--std=c89', '--std=c++14'], '-x c -std=c89')


def test_cmd_std_c_enforce(tmp_path):  # #13128/#13129
    __test_cmd(tmp_path, 'test.cpp',['--language=c', '--std=c89', '--std=c++14'], '-x c -std=c89')


def test_cmd_std_cpp_enforce(tmp_path):  # #13128/#13129
    __test_cmd(tmp_path, 'test.c',['--language=c++', '--std=c89', '--std=c++14'], '-x c++ -std=c++14')


def test_cmd_std_c_enforce_alias(tmp_path):  # #13128/#13129/#13130
    __test_cmd(tmp_path, 'test.c',['--language=c', '--std=gnu99', '--std=gnu++11'], '-x c -std=gnu99')


def test_cmd_std_c_enforce_alias_2(tmp_path):  # #13128/#13129/#13130
    __test_cmd(tmp_path, 'test.c',['--language=c', '--std=iso9899:1999', '--std=gnu++11'], '-x c -std=iso9899:1999')


def test_cmd_std_cpp_enforce_alias(tmp_path):  # #13128/#13129/#13130
    __test_cmd(tmp_path, 'test.c',['--language=c++', '--std=gnu99', '--std=gnu++11'], '-x c++ -std=gnu++11')


def test_debug_clang_output(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""
void f() {}
""")

    args = [
        '-q',
        '--clang',
        '--debug-clang-output',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stderr if not stdout else stdout
    assert stderr == ''
    assert stdout.startswith('TranslationUnitDecl'), stdout
    assert stdout.find(str(test_file)) != -1, stdout


def test_debug_clang_output_failure_exitcode(tmp_path):
    # the given code will cause clang to fail with an exitcode
    #
    # Failed to execute 'clang -fsyntax-only -Xclang -ast-dump -fno-color-diagnostics -x c++ a.cpp 2>&1' - (exitcode: 1 / output: a.cpp:3:12: error: indirection requires pointer operand ('int' invalid)
    # 3 |     (void)(*0);
    # |            ^~
    # 1 error generated.
    # TranslationUnitDecl 0x6127d5d9d4e8 <<invalid sloc>> <invalid sloc>
    # ...
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""void f()
{
    (void)(*0);
}
""")

    args = [
        '-q',
        '--clang',
        '--debug-clang-output',
        '--no-cppcheck-build-dir',  # TODO: test without this?
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stderr if not stdout else stdout
    stderr_lines = stderr.splitlines()
    assert len(stderr_lines) > 5, stderr_lines
    assert (stderr_lines[0] ==
            "Failed to execute 'clang -fsyntax-only -Xclang -ast-dump -fno-color-diagnostics -x c {} 2>&1' - (exitcode: 1 / output: {}:3:12: error: indirection requires pointer operand ('int' invalid)".format(test_file, test_file))
    assert stdout.find('TranslationUnitDecl') != -1, stdout
    assert stdout.find(str(test_file)) != -1, stdout