
# python -m pytest vcxproj_item_macro_path_case_test.py
#
# Regression coverage for MSBuild property-name case-insensitivity in the
# ClCompile item-path macro analysis (expandItemSpec() /
# hasOnlyInvariantItemPathVariables(), lib/importproject.cpp). Real MSBuild
# property names are case-insensitive -- PropertiesMap itself already uses
# cppcheck::stricmp for exactly this reason -- but the config-invariance
# analysis this feature added (mConfigInvariantProperties, the priming-pass
# valuesByProperty map, and the fixed invariantItemPathProperties() set) used
# plain, case-sensitive std::set/std::map. That mismatch has two distinct,
# independently regressed failure modes:
#
# 1. A property genuinely set to DIFFERENT values in different
#    configurations, but spelled with different casing in each
#    PropertyGroup (e.g. <CaseRoot> for Debug, <caseroot> for Release), gets
#    tracked as two unrelated single-valued names instead of one
#    multi-valued one -- each looks (wrongly) invariant on its own, so
#    cppcheck would expand and check two different real files instead of
#    correctly leaving the item unexpanded (matching real Visual Studio's
#    documented restriction -- see vcxproj_item_macro_path_test.py).
#    test_vcxproj_item_macro_path_case_variant covers this.
#
# 2. A property that genuinely IS config-invariant (same value everywhere),
#    or one of the fixed MSBuildThisFile.../MSBuildProject... properties,
#    fails to be recognized as such if it's spelled with different casing
#    across the PropertyGroups that set it and/or the item path that
#    references it -- real Visual Studio resolves it regardless, but
#    cppcheck would wrongly reject it and report a real, checkable file as
#    missing. test_vcxproj_item_macro_path_case_invariant (a user-defined
#    property spelled three different ways) and
#    test_vcxproj_item_macro_path_case_builtin (a fixed MSBuild property
#    referenced in lowercase) cover this.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_macro_path_case_variant():
    args = [
        '--project=vcxproj_item_macro_path_case_variant/vcxproj_item_macro_path_case_variant.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    normalized_stderr = stderr.replace('\\', '/')
    literal_path = 'vcxproj_item_macro_path_case_variant/$(CaseRoot)/foo.cpp'

    # $(CaseRoot) genuinely varies by configuration -- "CaseRoot" (Debug) and
    # "caseroot" (Release) are the same MSBuild property, just spelled
    # differently -- so it must NOT be expanded in either configuration,
    # exactly like vcxproj_item_macro_path_test.py's same-casing case.
    assert ('Checking %s Debug|x64...' % literal_path) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % literal_path) in normalized_stdout, stdout
    assert normalized_stderr.count('%s:0:0: error: File is missing: %s [missingFile]' % (literal_path, literal_path)) == 1, stderr

    # The real files under DebugCaseDir/ and ReleaseCaseDir/ must never be
    # reached -- if they were, their #error would surface here instead, and
    # that would mean the case-sensitivity bug let a genuinely-varying
    # property slip through as "invariant".
    assert 'DebugCaseDir/foo.cpp' not in normalized_stdout, stdout
    assert 'ReleaseCaseDir/foo.cpp' not in normalized_stdout, stdout


def test_vcxproj_item_macro_path_case_invariant():
    args = [
        '--project=vcxproj_item_macro_path_case_invariant/vcxproj_item_macro_path_case_invariant.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    filename = 'vcxproj_item_macro_path_case_invariant/SameCaseDir/mathplot.cpp'

    # $(MYROOT) must resolve in both configurations to the one property real
    # Visual Studio sees, however it's spelled at each definition/reference
    # site (MyRoot / myroot / MYROOT) -- not be rejected as unsupported.
    assert ('Checking %s Debug|x64...' % filename) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % filename) in normalized_stdout, stdout

    normalized_stderr = stderr.replace('\\', '/')
    assert normalized_stderr.count('%s:6:15: error: Division by zero.' % filename) == 1, stderr


def test_vcxproj_item_macro_path_case_builtin():
    args = [
        '--project=vcxproj_item_macro_path_case_builtin/vcxproj_item_macro_path_case_builtin.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    filename = 'vcxproj_item_macro_path_case_builtin/foo.cpp'

    # $(msbuildprojectdirectory), all lowercase, must still be recognized as
    # the fixed MSBuildProjectDirectory property and expanded.
    assert ('Checking %s Debug|x64...' % filename) in normalized_stdout, stdout

    normalized_stderr = stderr.replace('\\', '/')
    assert normalized_stderr.count('%s:6:15: error: Division by zero.' % filename) == 1, stderr
