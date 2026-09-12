
# python -m pytest vcxproj_discover_property_path_test.py
#
# Regression coverage for the vcxproj configuration-discovery pass (used only
# when a project has no inline <ItemGroup Label="ProjectConfigurations"> and
# must find its configurations by walking imports): it must still seed a
# guessed Configuration/Platform, even though it now ignores every Condition
# (see vcxproj_discover_choose_test.py / mDiscovering).
#
# This fixture's only <Import> has no Condition and is not behind a <Choose> --
# there is nothing for "ignore every Condition" to help with here. Instead, the
# import's PATH ITSELF embeds $(Configuration): <Import Project=
# "$(Configuration)\Configs.props" />. Resolving that path at all requires a
# concrete value for $(Configuration); without a seeded guess it is left as a
# literal, unexpanded "$(Configuration)" segment, which
# simplifyPathWithVariables() treats as unresolvable, so the import -- and the
# only configuration it defines -- is silently skipped and cppcheck fails
# outright ("no C or C++ source files found") despite the project being
# well-formed.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_discover_property_path():
    args = [
        '--project=vcxproj_discover_property_path/vcxproj_discover_property_path.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    # Windows prints native '\' path separators ("Checking foo\main.cpp ...");
    # normalize before matching so this passes on every platform (same idiom
    # used by test_log() in clang-import_test.py).
    normalized_stdout = stdout.replace('\\', '/')

    # The configuration reachable only through the unconditional, path-embedded
    # $(Configuration) import must be discovered and checked.
    assert 'Checking vcxproj_discover_property_path/main.cpp Debug|x64' in normalized_stdout, stdout
