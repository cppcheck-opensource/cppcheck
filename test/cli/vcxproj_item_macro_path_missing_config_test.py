
# python -m pytest vcxproj_item_macro_path_missing_config_test.py
#
# Regression coverage for mConfigInvariantProperties's priming pass only
# recording a property's value in configurations where it is actually SET
# (lib/importproject.cpp). A property a PropertyGroup sets in only some of a
# project's configurations -- with no PropertyGroup for it at all in the
# others -- still genuinely varies by configuration: real MSBuild resolves
# $(MyRoot) to the PropertyGroup's value under the configuration that sets
# it, and to "" under every configuration that doesn't (no PropertyGroup, no
# matching OS environment variable -- see
# vcxproj_item_macro_path_env_test.py for the case where one does exist).
# Naively collecting only the values each configuration's own property map
# happens to contain misses this: a property set in exactly one
# configuration has exactly one recorded value there and looks spuriously
# invariant, when the real, config-varying picture is "one real value in one
# configuration, empty in the rest" -- two distinct values, not one.
#
# This fixture's only <ClCompile> item is Include="$(MyRoot)\foo.cpp", where
# MyRoot is set only by Debug|x64's PropertyGroup; Release|x64 has none.
# DebugDir/foo.cpp's #error independently proves the item was never expanded
# and checked under Debug -- only that it consistently stays literal (and
# reported missing) in both configurations, exactly like
# vcxproj_item_macro_path_test.py's same-two-explicit-values case.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_macro_path_missing_config():
    args = [
        '--project=vcxproj_item_macro_path_missing_config/vcxproj_item_macro_path_missing_config.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    normalized_stderr = stderr.replace('\\', '/')
    literal_path = 'vcxproj_item_macro_path_missing_config/$(MyRoot)/foo.cpp'

    # $(MyRoot) is unset in Release|x64 -- it genuinely varies by
    # configuration -- so it must NOT be expanded in EITHER configuration,
    # not just the one where it happens to be unset.
    assert ('Checking %s Debug|x64...' % literal_path) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % literal_path) in normalized_stdout, stdout
    assert normalized_stderr.count('%s:0:0: error: File is missing: %s [missingFile]' % (literal_path, literal_path)) == 1, stderr

    # DebugDir/foo.cpp must never be reached -- if it were, its #error would
    # surface here instead, meaning MyRoot's being unset in Release wrongly
    # let Debug's value be treated as config-invariant.
    assert 'DebugDir/foo.cpp' not in normalized_stdout, stdout
