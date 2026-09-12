
# python -m pytest vcxproj_item_macro_path_invariant_test.py
#
# Regression coverage for the flip side of vcxproj_item_macro_path_test.py:
# a macro in a ClCompile project item's Include/Update/Remove path whose
# value is the SAME in every one of the project's configurations. Real Visual
# Studio has no trouble resolving this -- it's the well-known pattern of a
# property sheet (or, as here, an unconditioned PropertyGroup) defining a
# fixed third-party library location, e.g. $(wxMathPlot) or $(BoostRoot), and
# referencing it directly in an Include path. Microsoft's documented
# restriction is specifically about macros whose value COULD differ by
# configuration -- see vcxproj_item_macro_path_test.py and
# https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure
# -- so a config-invariant macro like this one is not the case that
# restriction describes, and cppcheck must expand it just like real Visual
# Studio does, not treat it the same as a genuinely varying one.
#
# This fixture has two configurations (Debug|x64, Release|x64) and one
# <ClCompile Include="$(ExternalLibRoot)\mathplot.cpp" />, where
# ExternalLibRoot is set once, unconditionally, to the same value regardless
# of configuration. main.cpp's deliberate division by zero proves the file
# was actually found and checked (not silently skipped as unresolved) in
# every configuration.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_macro_path_invariant():
    args = [
        '--project=vcxproj_item_macro_path_invariant/vcxproj_item_macro_path_invariant.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    filename = 'vcxproj_item_macro_path_invariant/ExternalLibRoot/mathplot.cpp'

    # Both configurations must resolve $(ExternalLibRoot)\mathplot.cpp to the
    # same real file and actually check it -- not treat the macro as
    # unsupported and end up with no valid source files.
    assert ('Checking %s Debug|x64...' % filename) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % filename) in normalized_stdout, stdout

    # Same underlying file checked under both configurations -- cppcheck
    # dedups the identical diagnostic, so it's reported exactly once.
    normalized_stderr = stderr.replace('\\', '/')
    assert normalized_stderr.count('%s:5:15: error: Division by zero.' % filename) == 1, stderr
