
# python3 -m pytest test-unused_function_test.py

import os
import json
import re
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


@pytest.mark.parametrize('header', [False, True])
@pytest.mark.parametrize('builddir', [False, True])
def test_unused_functions_qt_property(tmp_path, header, builddir):
    # The accessors are called through Qt's meta-object system, not from C++.
    code = '''class MyType {
    Q_PROPERTY(int property
               READ value WRITE setValue NOTIFY valueChanged)
public:
    int value() const { return 0; }
    void setValue(int) {}
    void valueChanged() {}
    void property() {}
    void unused() {}
};
'''
    source = tmp_path / 'test.cpp'
    if header:
        (tmp_path / 'type.h').write_text(code)
        source.write_text('#include "type.h"\nint main() { MyType t; }\n')
    else:
        source.write_text(code + 'int main() { MyType t; }\n')
    args = ['-q', '--template={id}:{message}', '--enable=unusedFunction', '--library=qt', str(source)]
    if builddir:
        (tmp_path / 'build').mkdir()
        args += ['--cppcheck-build-dir=' + str(tmp_path / 'build')]
    else:
        args += ['--no-cppcheck-build-dir']
    # Also check the saved whole-program data when the build directory is reused.
    for _ in range(2 if builddir else 1):
        ret, stdout, stderr = cppcheck(args)
        assert ret == 0, stdout
        assert stdout == ''
        assert stderr == ("unusedFunction:The function 'property' is never used.\n"
                          "unusedFunction:The function 'unused' is never used.\n")


@pytest.mark.parametrize('library', [False, True])
def test_unused_functions_inactive_qt_property(tmp_path, library):
    source = tmp_path / 'test.cpp'
    source.write_text('''#define Q_PROPERTY(...)
class MyType {
#if 0
    Q_PROPERTY(int value READ inactive)
#endif
    Q_PROPERTY(int value READ active)
public:
    int inactive() const { return 0; }
    int active() const { return 0; }
};
int main() { MyType t; }
''')
    args = ['-q', '--template={id}:{message}', '--enable=unusedFunction',
            '--no-cppcheck-build-dir', str(source)]
    if library:
        args += ['--library=qt']
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    expected = ["unusedFunction:The function 'inactive' is never used."]
    if not library:
        expected += ["unusedFunction:The function 'active' is never used."]
    assert sorted(stderr.splitlines()) == sorted(expected)


def test_unused_functions_qt_property_configurations(tmp_path):
    source = tmp_path / 'test.cpp'
    source.write_text('''class MyType {
#ifdef PROPERTY_VARIANT
    Q_PROPERTY(int value READ first)
#else
    Q_PROPERTY(int value READ second)
#endif
public:
    int first() const { return 0; }
    int second() const { return 0; }
    void unused() {}
};
int main() { MyType t; }
''')
    # Both configurations have identical C++ tokens after Q_PROPERTY is erased.
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    assert stdout == ''
    assert stderr == "unusedFunction:The function 'unused' is never used.\n"


@pytest.mark.parametrize('property_type, property_name', [
    ('READ', 'property'),
    ('const Names::READ*', 'property'),
    ('unsigned long', 'READ'),
    ('Box<Pair<::READ, ::WRITE>>', 'property'),
])
def test_unused_functions_qt_property_keyword_names(tmp_path, property_type, property_name):
    source = tmp_path / 'test.cpp'
    source.write_text('''class READ {};
class WRITE {};
template<class A, class B> struct Pair {};
template<class T> struct Box {};
namespace Names { class READ {}; }
class MyType {
    Q_PROPERTY(@TYPE@ @PROPERTY@ READ value WRITE READ NOTIFY valueChanged)
public:
    @RETURN_TYPE@ value() const { return {}; }
    void READ(@RETURN_TYPE@) {}
    void valueChanged() {}
    void property() {}
    void NOTIFY() {}
};
int main() { MyType t; }
'''.replace('@TYPE@', property_type).replace('@PROPERTY@', property_name)
                      .replace('@RETURN_TYPE@', '::READ' if property_type == 'READ' else property_type))
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    assert stderr == ("unusedFunction:The function 'property' is never used.\n"
                      "unusedFunction:The function 'NOTIFY' is never used.\n")


def test_unused_functions_qt_property_reset_keyword_name(tmp_path):
    source = tmp_path / 'test.cpp'
    source.write_text('''class MyType {
    Q_PROPERTY(int property READ value RESET READ)
public:
    int value() const { return 0; }
    void READ() {}
    void property() {}
};
int main() { MyType t; }
''')
    # The reset method named READ must not be read as another READ attribute.
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    assert stderr == "unusedFunction:The function 'property' is never used.\n"


@pytest.mark.parametrize('attributes, used', [
    ('READ value RESET resetValue', ['value', 'resetValue']),
    ('MEMBER field READ value', ['value']),
    ('MEMBER field WRITE setValue', ['setValue']),
    ('READ value REVISION 2 DESIGNABLE false SCRIPTABLE true STORED false USER true', ['value']),
    ('READ value REVISION(1, 2)', ['value']),
    ('READ default WRITE default BINDABLE bindValue', ['bindValue']),
    ('READ value CONSTANT FINAL', ['value']),
    ('READ value REQUIRED', ['value']),
    ('READ value VIRTUAL', ['value']),
    ('READ value OVERRIDE', ['value']),
] + [('READ value ' + attribute + ' enabled', ['value', 'enabled'])
     for attribute in ['DESIGNABLE', 'SCRIPTABLE', 'STORED', 'USER', 'EDITABLE']]
  + [('READ value DESIGNABLE enabled()', ['value', 'enabled'])])
def test_unused_functions_qt_property_attributes(tmp_path, attributes, used):
    source = tmp_path / 'test.cpp'
    source.write_text('''template<class T> struct QBindable {};
class Other { public: void field() {} };
class MyType {
    Q_PROPERTY(int property @ATTRIBUTES@)
public:
    int field;
    int value() const { return 0; }
    void setValue(int) {}
    void resetValue() {}
    QBindable<int> bindValue() { return {}; }
    bool enabled() const { return true; }
    void property() {}
};
int main() { MyType t; Other other; }
'''.replace('@ATTRIBUTES@', attributes))
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    reported = re.findall(r"unusedFunction:The function '([^']+)' is never used\.", stderr)
    assert set(reported) == {'field', 'value', 'setValue', 'resetValue', 'bindValue', 'enabled', 'property'} - set(used)
    assert len(reported) == len(stderr.splitlines())


@pytest.mark.parametrize('definition, invocation', [
    ('#define PROPERTY Q_PROPERTY(int property READ value)', 'PROPERTY'),
    ('#define PROPERTY(type, name, get) Q_PROPERTY(type name READ get)', 'PROPERTY(int, property, value)'),
    ('#define PROPERTY Q_PROPERTY', 'PROPERTY(int property READ value)'),
    ('#define GETTER value', 'Q_PROPERTY(int property READ GETTER)'),
    ('#define PROPERTY(get) Q_PROPERTY(int property READ get)\n#define WRAPPER(get) PROPERTY(get)', 'WRAPPER(value)'),
])
@pytest.mark.parametrize('header_definition', [False, True])
def test_unused_functions_qt_property_macros(tmp_path, definition, invocation, header_definition):
    # Model Qt's annotation extension point, including a source-level definition
    # of Q_PROPERTY that supersedes the library definition.
    (tmp_path / 'qt.h').write_text('''#ifndef QT_ANNOTATE_CLASS
#define QT_ANNOTATE_CLASS(type, ...)
#endif
#define Q_PROPERTY(...) QT_ANNOTATE_CLASS(qt_property, __VA_ARGS__)
''')
    source = tmp_path / 'test.cpp'
    source.write_text(('#include "qt.h"\n' if header_definition else '') + definition + '''
class MyType {
    @PROPERTY@
public:
    int value() const { return 0; }
    void property() {}
};
int main() { MyType t; }
'''.replace('@PROPERTY@', invocation))
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    assert stderr == "unusedFunction:The function 'property' is never used.\n"


@pytest.mark.parametrize('definition, invocation', [
    ('#define IGNORE(...)', 'IGNORE(Q_PROPERTY(int property READ value))'),
    ('#define TEXT(...) #__VA_ARGS__', 'const char* text = TEXT(Q_PROPERTY(int property READ value));'),
    ('', 'QT_ANNOTATE_CLASS(qt_enums, value)'),
])
def test_unused_functions_qt_property_discarded_metadata(tmp_path, definition, invocation):
    source = tmp_path / 'test.cpp'
    source.write_text(definition + '''
class MyType {
    @PROPERTY@
public:
    int value() const { return 0; }
};
int main() { MyType t; }
'''.replace('@PROPERTY@', invocation))
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    assert stderr == "unusedFunction:The function 'value' is never used.\n"


def test_qt_annotations_preserve_user_code(tmp_path):
    source = tmp_path / 'test.cpp'
    source.write_text('''#define CALL __cppcheck_qt_annotation__(1, 2)
#define QT_ANNOTATE_CLASS(type, ...) int userHook;
void __cppcheck_qt_annotation__(int, int) {}
class MyType {
    Q_PROPERTY(int property READ value)
public:
    int value() const { return 0; }
};
int main() { CALL; }
''')
    ret, stdout, stderr = cppcheck(['-q', '-E', '--library=qt', str(source)])
    assert ret == 0, stderr
    tokens = ' '.join(stdout.split())
    assert 'int userHook ;' in tokens
    assert '__cppcheck_qt_annotation__ ( 1 , 2 )' in tokens


@pytest.mark.parametrize('replacement', [
    'QT_ANNOTATE_CLASS(qt_other, ignored) __cppcheck_qt_annotation__(1, 2)',
    '__cppcheck_qt_annotation__(1, 2) QT_ANNOTATE_CLASS(qt_other, ignored)',
    'QT_ANNOTATE_CLASS(qt_other, ignored) __cppcheck_qt_annotation__(1, 2) QT_ANNOTATE_CLASS(qt_another, ignored)',
])
def test_qt_annotations_share_macro_location(tmp_path, replacement):
    source = tmp_path / 'test.cpp'
    source.write_text('#define MIXED() ' + replacement + '''
void __cppcheck_qt_annotation__(int, int) {}
int main() { MIXED(); }
''')
    # Object-like expansion can attach the outer macro's name and location to
    # both a Qt annotation and unrelated user tokens. The internal tag matters.
    ret, stdout, stderr = cppcheck(['-q', '-E', '--library=qt', str(source)])
    assert ret == 0, stderr
    tokens = ' '.join(stdout.split())
    assert 'int main ( ) { __cppcheck_qt_annotation__ ( 1 , 2 ) ; }' in tokens
    assert 'qt_other' not in tokens
    assert 'qt_another' not in tokens


def test_unused_functions_qt_property_qualified_accessor(tmp_path):
    source = tmp_path / 'test.cpp'
    source.write_text('''class Base {
public:
    int value() const { return 0; }
};
class MyType : public Base {
    Q_PROPERTY(int property READ (Base::value))
public:
    void unused() {}
};
int main() { MyType t; }
''')
    ret, stdout, stderr = cppcheck(['-q', '--template={id}:{message}',
                                   '--enable=unusedFunction', '--library=qt',
                                   '--no-cppcheck-build-dir', str(source)])
    assert ret == 0, stdout
    assert stderr == "unusedFunction:The function 'unused' is never used.\n"


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
