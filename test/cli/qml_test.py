
# python3 -m pytest test-qml.py

import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

# there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
__project_dir = os.path.join(__script_dir, 'QML-Samples-TableView')
__project_dir_sep = __project_dir + os.path.sep


def __test_unused_functions(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--library=qt',
        '--enable=unusedFunction',
        __project_dir
    ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    lines = stderr.splitlines()
    lines.sort()
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert lines == [
        "{}samplemodel.cpp:15:23: style: The function 'data' is never used. [unusedFunction]".format(__project_dir_sep),
        "{}samplemodel.cpp:38:37: style: The function 'roleNames' is never used. [unusedFunction]".format(__project_dir_sep),
        "{}samplemodel.cpp:9:18: style: The function 'rowCount' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions():
    __test_unused_functions(['-j1', '--no-cppcheck-build-dir'])


def test_unused_functions_j():
    args = [
        '-q',
        '--template=simple',
        '--library=qt',
        '--enable=unusedFunction',
        '-j2',
        '--no-cppcheck-build-dir',
        __project_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == ["cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_unused_functions_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_unused_functions(['--cppcheck-build-dir={}'.format(build_dir)])


# TODO: test with project file
# TODO: test with FileSettings
