
# python -m pytest vcxproj_item_macro_path_test.py
#
# Regression coverage for macro ($(...)) expansion in a ClCompile project
# item's Include/Update/Remove path (as opposed to expansion elsewhere, e.g.
# in metadata values or Condition attributes, which is unaffected).
#
# Microsoft documents that the Visual Studio C++ project system does not
# reliably resolve a macro in a project item path if that macro's value could
# differ by configuration -- "The IDE doesn't expect project item paths to be
# different for different project configurations" -- see
# https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure
# That is specifically about macros whose value VARIES by configuration: the
# fixed set of MSBuild "this file"/"this project" location properties
# (MSBuildThisFileDirectory and friends) are always exempt, since they can't
# vary by construction -- see test/cli/shared-items-project, covered
# separately by test_shared_items_project() in more-projects_test.py -- and so
# is any other property THIS project happens to resolve to the same value in
# every one of its configurations, e.g. one a property sheet sets once,
# unconditionally (see vcxproj_item_macro_path_invariant_test.py).
#
# This fixture's only <ClCompile> item is Include="$(SomeDir)\foo.cpp", where
# SomeDir is an ordinary user-defined property that genuinely differs between
# the project's two configurations (SomeDir for Debug|x64, SomeOtherDir for
# Release|x64) -- exactly the case Visual Studio can't reliably resolve, since
# a single Solution Explorer file list can't show two different files
# depending on which configuration happens to be active. Expanding $(SomeDir)
# here would resolve to a real file that genuinely exists on disk in either
# configuration -- but real Visual Studio does not expand it, so cppcheck
# must not either.
#
# Critically, "must not expand it" does not mean the item silently vanishes:
# a user whose real project has a file like this would have no way to know
# it went unchecked. So the item is still kept, under its literal,
# unexpanded path ("vcxproj_item_macro_path/$(SomeDir)/foo.cpp") -- which
# can never exist on disk -- and cppcheck reports it exactly like any other
# missing source file, a normal, visible "File is missing: ..." error that
# also names the exact unresolved macro path, not just an addDebug() trace
# nobody but a --debug run would ever see.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_macro_path():
    args = [
        '--project=vcxproj_item_macro_path/vcxproj_item_macro_path.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    normalized_stderr = stderr.replace('\\', '/')
    literal_path = 'vcxproj_item_macro_path/$(SomeDir)/foo.cpp'

    # $(SomeDir) must NOT be expanded -- Visual Studio can't reliably resolve
    # it either, since it genuinely differs between configurations -- but the
    # item must still be checked (and fail to be found) under its literal,
    # unexpanded path, in both configurations, rather than disappearing.
    assert ('Checking %s Debug|x64...' % literal_path) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % literal_path) in normalized_stdout, stdout

    # Same literal (nonexistent) path in both configurations -- cppcheck
    # dedups the identical diagnostic, so it's reported exactly once.
    assert normalized_stderr.count('%s:0:0: error: File is missing: %s [missingFile]' % (literal_path, literal_path)) == 1, stderr
