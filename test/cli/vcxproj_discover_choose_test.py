
# python -m pytest vcxproj_discover_choose_test.py
#
# Regression coverage for the vcxproj configuration-discovery pass (used only
# when a project has no inline <ItemGroup Label="ProjectConfigurations"> and
# must find its configurations by walking imports): it must also look inside
# a top-level <Choose>, not just top-level PropertyGroup/ImportGroup/Import.
#
# This fixture's only configuration (Debug|x64) is defined in Configs.props,
# which is reachable only through an <ImportGroup> nested inside a top-level
# <Choose><When Condition="true">. A discovery pass that walks
# PropertyGroup/ImportGroup/Import at the top level but never inspects <Choose>
# at all never finds Configs.props, so no configuration is discovered and
# cppcheck fails outright ("no C or C++ source files found") even though the
# project is well-formed -- this is a different bug than the discovery pass
# stopping early (covered by vcxproj_split_configs_test.py): here the node type
# is never even recognized, so it doesn't matter whether it's found first,
# last, or is the only import in the project.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_discover_choose():
    args = [
        '--project=vcxproj_discover_choose/vcxproj_discover_choose.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    # Windows prints native '\' path separators ("Checking foo\main.cpp ...");
    # normalize before matching so this passes on every platform (same idiom
    # used by test_log() in clang-import_test.py).
    normalized_stdout = stdout.replace('\\', '/')

    # The configuration reachable only through the top-level Choose must be
    # discovered and checked, not silently dropped.
    assert 'Checking vcxproj_discover_choose/main.cpp Debug|x64' in normalized_stdout, stdout
