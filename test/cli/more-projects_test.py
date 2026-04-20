# python -m pytest test-more-projects.py
import json
import os
import pytest
import sys
from testutils import cppcheck, assert_cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

def test_project_force_U(tmpdir):
    # 10018
    # -U does not work with compile_commands.json
    with open(os.path.join(tmpdir, 'bug1.cpp'), 'wt') as f:
        f.write("""
                int x = 123 / 0;
                #ifdef MACRO1
                int y = 1000 / 0;
                #endif
                """)

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "bug1.cpp",
         "output": "bug1.o"}
    ]

    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    # Without -U => both bugs are found
    ret, stdout, stderr = cppcheck(['--project=' + compile_commands, '--force', '-rp=' + str(tmpdir), '--template=cppcheck1'])
    assert ret == 0, stdout
    assert (stderr == '[bug1.cpp:2]: (error) Division by zero.\n'
                      '[bug1.cpp:4]: (error) Division by zero.\n')

    # With -U => only first bug is found
    ret, stdout, stderr = cppcheck(['--project=' + compile_commands, '--force', '-UMACRO1', '-rp=' + str(tmpdir), '--template=cppcheck1'])
    assert ret == 0, stdout
    assert stderr == '[bug1.cpp:2]: (error) Division by zero.\n'


def __write_cppcheck_project_file(tmpdir, platform=None, importproject=None):
    project_file = os.path.join(tmpdir, 'Project.cppcheck')

    if platform is not None:
        platform = '<platform>{}</platform>'.format(platform)
    if importproject is not None:
        platform = '<importproject>{}</importproject>'.format(importproject)

    with open(project_file, 'wt') as f:
        f.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
    {}
    {}
    <paths>
        <dir name="."/>
    </paths>
</project>""".format(platform, importproject)
        )

    return project_file


def test_project_custom_platform(tmpdir):
    """
    import cppcheck project that contains a custom platform file
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='p1.xml')

    with open(os.path.join(tmpdir, 'p1.xml'), 'wt') as f:
        f.write('<?xml version="1.0"?>\n<platform/>')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 0, stdout
    assert stdout == ''
    assert stderr == ''


def test_project_empty_platform(tmpdir):
    """
    import cppcheck project that contains an empty platform type
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 0, stdout
    assert stdout == ''
    assert stderr == ''


def test_project_unspecified_platform(tmpdir):
    """
    import cppcheck project that contains the deprecated platform type "Unspecified"
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='Unspecified')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: unrecognized platform: 'Unspecified'.\n"
    assert stderr == ''


def test_project_unknown_platform(tmpdir):
    """
    import cppcheck project that contains an unknown platform type
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='dummy')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: unrecognized platform: 'dummy'.\n"
    assert stderr == ''


def test_project_empty_fields(tmpdir):
    """
    import cppcheck project that contains all empty fields - make sure there are no crashes
    """
    project_file = os.path.join(tmpdir, 'Project.cppcheck')

    with open(project_file, 'wt') as f:
        f.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
  <root/>
  <builddir/>
  <includedir/>
  <includedir>
    <dir/>
  </includedir>
  <defines/>
  <defines>
    <define/>
  </defines>
  <undefines/>
  <undefines>
    <undefine/>
  </undefines>
  <importproject/>
  <paths/>
  <paths>
    <dir/>
  </paths>
  <exclude/>
  <exclude>
    <path/>
  </exclude>
  <function-contracts/>
  <variable-contracts/>
  <ignore/>
  <ignore>
    <path/>
  </ignore>
  <libraries/>
  <libraries>
    <library/>
  </libraries>
  <suppressions/>
  <suppressions>
    <suppression/>
  </suppressions>
  <vs-configurations/>
  <vs-configurations>
    <config/>
  </vs-configurations>
  <platform/>
  <analyze-all-vs-configs/>
  <parser/>
  <addons/>
  <addons>
    <addon/>
  </addons>
  <tags/>
  <tools/>
  <tools>
    <tool/>
  </tools>
  <check-headers/>
  <check-unused-templates/>
  <max-ctu-depth/>
  <max-template-recursion/>
  <check-unknown-function-return-values/>
  <safe-checks/>
  <safe-checks>
    <class-public/>
    <external-functions/>
    <internal-functions/>
    <external-variables/>
  </safe-checks>
  <tag-warnings/>
  <bug-hunting/>
  <cert-c-int-precision/>
  <coding-standards/>
  <coding-standards>
    <coding-standard/>
  </coding-standards>
</project>""")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout # do not crash
    assert stdout == 'cppcheck: error: no C or C++ source files found.\n'
    assert stderr == ''


def test_project_missing_subproject(tmpdir):
    """
    import cppcheck project that contains an unknown platform type
    """
    project_file = __write_cppcheck_project_file(tmpdir, importproject='dummy.json')

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: failed to open project '{}/dummy.json'. The file does not exist.\n".format(str(tmpdir).replace('\\', '/'))
    assert stderr == ''


def test_project_std(tmpdir):
    with open(os.path.join(tmpdir, 'bug1.cpp'), 'wt') as f:
        f.write("""
                #if __cplusplus == 201402L
                int x = 123 / 0;
                #endif
                """)

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')

    compilation_db = [
        {
            "directory": str(tmpdir),
            "command": "c++ -o bug1.o -c bug1.cpp -std=c++14",
            "file": "bug1.cpp",
            "output": "bug1.o"
        }
    ]

    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    ret, stdout, stderr = cppcheck(['--project=' + compile_commands, '--enable=all', '-rp=' + str(tmpdir), '--template=cppcheck1'])
    assert ret == 0, stdout
    assert (stderr == '[bug1.cpp:3]: (error) Division by zero.\n')



@pytest.mark.skip() # clang-tidy is not available in all cases
def test_clang_tidy(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                  (void)argc;
                }
                """)

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
  <paths>
   <dir name="{}"/>
  </paths>>
  <tools>
    <tool>clang-tidy</tool>
  </tools>
</project>""".format(test_file))

    args = ['--project={}'.format(project_file)]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    lines = stdout.splitlines()
    # TODO: should detect clang-tidy issue
    assert len(lines) == 1
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == ''


@pytest.mark.parametrize("file_filter", [
    ['--file-filter=test.cpp'],
    ['--file-filter=*.cpp'],
    ['--file-filter=+', 'test.cpp'],
    ['--file-filter=+', '*.cpp'],
])
def test_project_file_filter(tmpdir, file_filter):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    args = file_filter + ['--project={}'.format(project_file)]
    out_lines = [
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


@pytest.mark.parametrize("file_filter", [
    ['--file-filter=*.cpp'],
    ['--file-filter=+', '*.cpp'],
])
def test_project_file_filter_cpp(tmpdir, file_filter):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_1, test_file_2))

    args = file_filter + ['--project={}'.format(project_file)]
    out_lines = [
        'Checking {} ...'.format(test_file_1)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


@pytest.mark.parametrize("file_filter", [
    ['--file-filter=*.c'],
    ['--file-filter=+', '*.c'],
])
def test_project_file_filter_c(tmpdir, file_filter):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_1, test_file_2))

    args = file_filter + ['--project={}'.format(project_file)]
    out_lines = [
        'Checking {} ...'.format(test_file_2)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_project_relpath_file_filter_abspath(tmpdir):
    """
    relative paths in project file, absolute path in file filter
    """
    test_file_cpp = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_cpp, 'wt') as f:
        pass
    test_file_c = os.path.join(tmpdir, 'test.c')
    with open(test_file_c, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="test.cpp"/>
        <dir name="test.c"/>
    </paths>
</project>""")

    out_lines = [
        'Checking test.c ...'
    ]

    args = ['--file-filter={}'.format(test_file_c), '--project=test.cppcheck']
    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines, cwd=tmpdir)


def test_project_abspath_file_filter_relpath(tmpdir):
    """
    absolute paths in project file, relative path in file filter
    """
    test_file_cpp = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_cpp, 'wt') as f:
        pass
    test_file_c = os.path.join(tmpdir, 'test.c')
    with open(test_file_c, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_c, test_file_cpp))

    out_lines = [
        'Checking {} ...'.format(test_file_c)
    ]

    args = ['--file-filter=test.c', '--project=test.cppcheck']
    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines, cwd=tmpdir)


def test_project_pathmatch_other_cwd(tmpdir):
    """
    mixed relative and absolute paths in project file and on command line, executed in a different directory
    """
    test_root = tmpdir
    test_cwd = os.path.join(test_root, 'cwd')
    test_dir_1 = os.path.join(test_root, 'a')
    test_dir_2 = os.path.join(test_root, 'b')
    test_dir_3 = os.path.join(test_cwd, 'b')

    os.mkdir(test_cwd)
    os.mkdir(test_dir_1)
    os.mkdir(test_dir_2)
    os.mkdir(test_dir_3)

    test_file_1 = os.path.join(test_dir_1, 'a-abs.c')
    with open(test_file_1, 'wt') as f:
        pass

    test_file_2 = os.path.join(test_dir_1, 'a-rel.c')
    with open(test_file_2, 'wt') as f:
        pass

    test_file_3 = os.path.join(test_dir_2, 'b-abs.c')
    with open(test_file_3, 'wt') as f:
        pass

    test_file_4 = os.path.join(test_dir_2, 'b-rel.c')
    with open(test_file_4, 'wt') as f:
        pass

    test_file_5 = os.path.join(test_dir_3, 'b-abs.c')
    with open(test_file_5, 'wt') as f:
        pass

    test_file_6 = os.path.join(test_dir_3, 'b-rel.c')
    with open(test_file_6, 'wt') as f:
        pass

    project_file = os.path.join(test_root, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <exclude>
        <path name="./?/"/>
    </exclude>
    <paths>
        <dir name="{}"/>
        <dir name="a/a-rel.c"/>
        <dir name="{}"/>
        <dir name="b/b-rel.c"/>
        <dir name="{}"/>
        <dir name="cwd/b/b-rel.c"/>
    </paths>
</project>""".format(test_file_1, test_file_3, test_file_5))

    out_lines = [
        'Checking {} ...'.format(test_file_5),
        'Checking {} ...'.format(os.path.join("..", "cwd", "b", "b-rel.c")),
    ]

    args = ['--file-filter={}/*/?/**.c*'.format(test_root), '--project=../test.cppcheck']
    exitcode, stdout, stderr = cppcheck(args, cwd=test_cwd)
    stdout_lines = stdout.splitlines()
    assert 0 == exitcode
    assert '' == stderr
    assert 4 == len(stdout_lines)
    assert set(out_lines) <= set(stdout_lines)


def test_project_file_filter_no_match(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    args = ['--file-filter=*.c', '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find any files matching the filter:*.c'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_order(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass
    test_file_d = os.path.join(tmpdir, 'd.c')
    with open(test_file_d, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_c, test_file_d, test_file_b, test_file_a))

    args = ['--project={}'.format(project_file), '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/4 files checked 25% done',
        'Checking {} ...'.format(test_file_d),
        '2/4 files checked 50% done',
        'Checking {} ...'.format(test_file_b),
        '3/4 files checked 75% done',
        'Checking {} ...'.format(test_file_a),
        '4/4 files checked 100% done'
    ]
    assert stderr == ''


def test_project_file_duplicate(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_a, test_file_a, tmpdir))

    args = ['--project={}'.format(project_file)]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_a)
    ]
    assert stderr == ''


def test_project_file_duplicate_2(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_c, test_file_a, test_file_b, tmpdir, test_file_b, test_file_c, test_file_a, tmpdir))

    args = ['--project={}'.format(project_file), '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/3 files checked 33% done',
        'Checking {} ...'.format(test_file_a),
        '2/3 files checked 66% done',
        'Checking {} ...'.format(test_file_b),
        '3/3 files checked 100% done'
    ]
    assert stderr == ''


def test_project_file_duplicate_3(tmpdir):  # #12834
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    # multiple ways to specify the same file
    in_file_a = 'a.c'
    in_file_b = os.path.join('.', 'a.c')
    in_file_c = os.path.join('dummy', '..', 'a.c')
    in_file_d = os.path.join(tmpdir, 'a.c')
    in_file_e = os.path.join(tmpdir, '.', 'a.c')
    in_file_f = os.path.join(tmpdir, 'dummy', '..', 'a.c')

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, tmpdir))

    args = ['--project={}'.format(project_file)]

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_a)
    ]
    assert stderr == ''


@pytest.mark.skipif(sys.platform != 'win32', reason="requires Windows")
def test_project_file_duplicate_4(tmpdir):  # #12834
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    # multiple ways to specify the same file
    in_file_a = 'a.c'
    in_file_b = os.path.join('.', 'a.c')
    in_file_c = os.path.join('dummy', '..', 'a.c')
    in_file_d = os.path.join(tmpdir, 'a.c')
    in_file_e = os.path.join(tmpdir, '.', 'a.c')
    in_file_f = os.path.join(tmpdir, 'dummy', '..', 'a.c')

    args1 = [in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, str(tmpdir)]
    args2 = []
    for a in args1:
        args2.append(a.replace('\\', '/'))

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, tmpdir,
                     args2[0], args2[1], args2[2], args2[3], args2[4], args2[5], args2[6]))

    args = ['--project={}'.format(project_file)]

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_a)
    ]
    assert stderr == ''

def test_project_file_ignore(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    args = ['-itest.cpp', '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_ignore_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
    <exclude>
        <path name="test.cpp"/>
    </exclude>
</project>""".format(test_file))

    args = ['--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_ignore_3(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
    <ignore>
        <path name="test.cpp"/>
    </ignore>
</project>""".format(test_file))

    args = ['--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_json_file_ignore(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "test.cpp",
         "output": "test.o"}
    ]

    project_file = os.path.join(tmpdir, 'test.json')
    with open(project_file, 'wt') as f:
        f.write(json.dumps(compilation_db))

    args = ['-itest.cpp', '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_json_file_ignore_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "test.cpp",
         "output": "test.o"}
    ]

    project_file = os.path.join(tmpdir, 'test.json')
    with open(project_file, 'wt') as f:
        f.write(json.dumps(compilation_db))

    args = ['-i{}'.format(test_file), '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


@pytest.mark.xfail(strict=True)
def test_project_D(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#ifndef __GNUC__
#error "requirement not met"
#endif
                """)

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """
<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
  <paths>
   <dir name="{}"/>
  </paths>
</project>
            """.format(test_file))

    args = [
        '--project=' + project_file,
        '--template=simple',
    ]
    arg_D = ['-D__GNUC__']

    out_expected = [
        'Checking {} ...'.format(test_file),
        'Checking {}: __GNUC__=1...'.format(test_file)
    ]

    args1 = args + arg_D
    ret, stdout, stderr = cppcheck(args1)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout

    # TODO: -D__GNUC__ is lost
    args2 = arg_D + args
    ret, stdout, stderr = cppcheck(args2)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_compdb_D(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#ifndef __GNUC__
#error "requirement not met"
#endif
                """)

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o test.o -c test.cpp",
         "file": "test.cpp",
         "output": "test.o"}
    ]
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    args = [
        '--project=' + compile_commands,
        '--template=simple',
        ]
    arg_D = ['-D__GNUC__']

    out_expected = [
        'Checking {} ...'.format(test_file),
        'Checking {}: __GNUC__=1;...'.format(test_file)  # TODO: get rid of extra ;
    ]

    args1 = args + arg_D
    ret, stdout, stderr = cppcheck(args1)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout

    args2 = arg_D + args
    ret, stdout, stderr = cppcheck(args2)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_shared_items_project():
    solution_file = os.path.join('shared-items-project', 'Solution.sln')

    args = [
        '--platform=win64',
        '--project={}'.format(solution_file),
        '--project-configuration=Release|x64'
    ]

    exitcode, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert exitcode == 0

    # Assume no errors, and that shared items code files have been checked as well
    assert '2/2 files checked ' in stdout  # only perform partial check since -j2 does not report a percentage right now
    assert stderr == ''


def test_project_file_nested(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt'):
        pass

    level3_file = tmp_path / 'level3.cppcheck'
    with open(level3_file, 'wt') as f:
        f.write(
"""<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    level2_file = tmp_path / 'level2.cppcheck'
    with open(level2_file, 'wt') as f:
        f.write(
"""<project>
    <importproject>level3.cppcheck</importproject>
</project>""")

    level1_file = tmp_path / 'level1.cppcheck'
    with open(level1_file, 'wt') as f:
        f.write(
"""<project>
    <importproject>level2.cppcheck</importproject>
</project>""")

    args = ['--project={}'.format(level1_file)]
    out_lines = [
        'cppcheck: error: nested Cppcheck GUI projects are not supported.'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_no_analyze_all_vs_configs(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt'):
        pass

    project_path = tmp_path / 'project.cppcheck'
    with open(project_path, 'wt') as f:
        f.write(
"""<project>
    <analyze-all-vs-configs>false</analyze-all-vs-configs>
    <paths>
        <dir name="."/>
    </paths>
</project>""")

    ret, stdout, stderr = cppcheck(['--project=' + str(project_path)])
    assert ret == 0, stdout
    assert stderr == ''


@pytest.mark.parametrize("j,executor", [
    (1, "thread"),
    (2, "thread"),
    (2, "process"),
])
def test_project_progress(tmp_path, j, executor):
    if sys.platform == 'win32' and executor == "process":
        pytest.skip("process executor not supported on Windows")

    code = 'x = 1;'
    with open(tmp_path / 'test1.c', 'wt') as f:
        f.write(code)
    with open(tmp_path / 'test2.c', 'wt') as f:
        f.write(code)

    compilation_db = [
        {"directory": str(tmp_path),
         "command": "gcc -c test1.c",
         "file": "test1.c",
         "output": "test1.o"},
        {"directory": str(tmp_path),
         "command": "gcc -c test2.c",
         "file": "test2.c",
         "output": "test2.o"},
    ]

    project_file = tmp_path / 'compile_commands.json'

    with open(project_file, 'wt') as f:
        f.write(json.dumps(compilation_db))

    _, stdout, _ = cppcheck([f'--project={project_file}', f'-j{j}', f'--executor={executor}'])
    assert '1/2 files checked 50% done' in stdout
    assert '2/2 files checked 100% done' in stdout
