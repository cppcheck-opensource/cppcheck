
# python -m pytest props_dirs_test.py
#
# Regression coverage for MSBuild property-sheet (.props) loading across multiple
# directories:
#   - $(MSBuildThisFileDirectory) must resolve to each .props file's own directory,
#     not the importing project's directory, even through a chain of nested imports
#     (ProjA/ -> shared/shared.props -> common/common.props).
#   - AdditionalIncludeDirectories set via that chain must actually make a header in a
#     different directory (common/common.h) resolvable from the project's source file.
#   - A project that imports common/common.props directly (ProjB) must pick up exactly
#     what that file sets and nothing that a *different* project in the same solution
#     (ProjA) added on top - no cross-project variable leakage.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

__ERR_A = ('%s:14:14: error: Division by zero. [zerodiv]\n'
           '    return x / 0;\n'
           '             ^\n') % os.path.join('props-dirs', 'ProjA', 'a.cpp')
__ERR_B = ('%s:14:14: error: Division by zero. [zerodiv]\n'
           '    return y / 0;\n'
           '             ^\n') % os.path.join('props-dirs', 'ProjB', 'b.cpp')


def __get_lines(s):
    # file order is not guaranteed when multiple jobs are used (TEST_CPPCHECK_INJECT_J) so
    # compare output order-independently
    return sorted(s.split('\n'))


def test_props_dirs_solution():
    args = [
        '--project=props-dirs/props-dirs.slnx',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    # both files were actually analyzed (division by zero fires) which also proves
    # "common.h" was found via AdditionalIncludeDirectories - if it hadn't resolved, the
    # #error guard in each .cpp would have fired instead and there would be no zerodiv
    assert __get_lines(stderr) == __get_lines(__ERR_A + __ERR_B)


def test_props_dirs_defines_and_standard():
    args = [
        '--project=props-dirs/props-dirs.slnx',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    dump_a = os.path.join(__script_dir, 'props-dirs', 'ProjA', 'a.cpp.dump')
    dump_b = os.path.join(__script_dir, 'props-dirs', 'ProjB', 'b.cpp.dump')
    assert os.path.exists(dump_a), f"Dump file not found at {dump_a}"
    assert os.path.exists(dump_b), f"Dump file not found at {dump_b}"

    with open(dump_a, 'rt') as f:
        dump_a_content = f.read()
    with open(dump_b, 'rt') as f:
        dump_b_content = f.read()

    # ProjA imports shared/shared.props (-> common/common.props) and shared/shared2.props
    # (-> common/common2.props), and sets its own PROJA_DEFINE.
    # v143 toolset -> _MSC_VER=1930/_MSC_FULL_VER=193000000; common.props sets stdcpp17
    # -> _MSVC_LANG=201703L.
    assert '_WIN32=1' in dump_a_content
    assert '_WIN64=1' in dump_a_content
    assert '_M_X64=100' in dump_a_content
    assert '_MSC_VER=1930' in dump_a_content
    assert '_MSC_FULL_VER=193000000' in dump_a_content
    assert '_MSVC_LANG=201703L' in dump_a_content
    assert 'PROJA_DEFINE=1' in dump_a_content
    assert 'SHARED_DEFINE=1' in dump_a_content
    assert 'COMMON_DEFINE=1' in dump_a_content
    assert '<cpp version="c++17"/>' in dump_a_content

    # ProjB imports common/common.props and common/common2.props directly - it must see
    # COMMON2_DEFINE and COMMON_DEFINE, but none of ProjA's or shared's defines.
    assert '_MSC_VER=1930' in dump_b_content
    assert '_MSC_FULL_VER=193000000' in dump_b_content
    assert '_MSVC_LANG=201703L' in dump_b_content
    assert 'COMMON2_DEFINE=1' in dump_b_content
    assert 'COMMON_DEFINE=1' in dump_b_content
    assert '<cpp version="c++17"/>' in dump_b_content
    assert 'PROJA_DEFINE' not in dump_b_content
    assert 'SHARED_DEFINE' not in dump_b_content
    assert 'SHARED2_DEFINE' not in dump_b_content
