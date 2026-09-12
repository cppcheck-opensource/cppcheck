
# python -m pytest vcxproj_item_absolute_drive_path_test.py
#
# Regression coverage for ImportProject::toAbsoluteExpanded() using the
# host-dependent Path::isAbsolute() to decide whether a ClCompile item path
# is already fully rooted (lib/importproject.cpp). Path::isAbsolute() only
# recognizes a leading '/' on a non-Windows host, so a Windows
# drive-absolute path straight out of a .vcxproj -- e.g. "C:/nonexistent/foo.cpp",
# with no macro involved at all -- would be misclassified as relative and
# joined onto the project directory instead of used as-is, producing
# something like "<projectDir>/C:/nonexistent/foo.cpp" -- not what real
# Visual Studio does (and not what toAbsoluteExpanded() itself would do
# running natively on Windows, where Path::isAbsolute() DOES recognize
# "C:\..." as absolute). The fix uses classifyPath() -- the same
# host-independent Windows/Unix path classification already used throughout
# this file (see pathCombineAppend(), toAbsolute(), etc.) -- instead.
#
# This fixture's only <ClCompile> item is Include="C:\nonexistent\foo.cpp",
# a plain drive-absolute path with nothing to macro-expand, so
# expandItemSpec() passes it straight through to toAbsoluteExpanded().

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_absolute_drive_path():
    args = [
        '--project=vcxproj_item_absolute_drive_path/vcxproj_item_absolute_drive_path.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    normalized_stderr = stderr.replace('\\', '/')

    # The drive-absolute path must be used exactly as-is -- not joined onto
    # the project directory.
    assert 'Checking C:/nonexistent/foo.cpp Debug|x64...' in normalized_stdout, stdout
    assert normalized_stderr.count('C:/nonexistent/foo.cpp:0:0: error: File is missing: C:/nonexistent/foo.cpp [missingFile]') == 1, stderr

    # It must NOT have been nested under the project directory -- that would
    # mean toAbsoluteExpanded() wrongly treated the already-absolute path as
    # relative and joined it onto projectDir.
    assert 'vcxproj_item_absolute_drive_path/C:' not in normalized_stdout, stdout
    assert 'vcxproj_item_absolute_drive_path/C:' not in normalized_stderr, stderr
