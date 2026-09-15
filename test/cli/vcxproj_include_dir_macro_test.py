
# python -m pytest vcxproj_include_dir_macro_test.py
#
# Regression coverage for an unresolvable macro in AdditionalIncludeDirectories
# (fsSetIncludePaths(), lib/importproject.cpp). Unlike a ClCompile item's own
# Include/Update/Remove path (see vcxproj_item_macro_path_test.py and friends),
# an include-search-path entry isn't restricted to config-invariant macros --
# AdditionalIncludeDirectories is ordinary ItemDefinitionGroup metadata,
# expanded unconditionally like any other (see vcxproj_property_order_test.py).
# The problem here is different: when a macro in one entry genuinely can't be
# resolved at all (not a project property, not an environment variable),
# fsSetIncludePaths() used to silently drop that whole entry from the search
# path list. Any header that lived under it would then simply go unfound, with
# nothing in cppcheck's normal output pointing back at the real cause -- the
# only trace was an addDebug() call, and addDebug()/debugs is not surfaced by
# the CLI under any flag (it's accumulated but never read anywhere outside
# ImportProject itself).
#
# The fix keeps the entry (as its literal, unexpanded text -- a directory that
# can never exist is harmless to search) and records a normal, always-visible
# message in ImportProject::errors, printed unconditionally by the CLI exactly
# like every other project-import error -- not gated behind --debug like
# addDebug() traces are.
#
# This fixture's only <ClCompile> item is main.cpp, which #includes
# "myheader.h" -- present only under RealInc/, reachable exclusively via
# $(CppcheckTestIncDirMacro)\RealInc, where CppcheckTestIncDirMacro is not
# defined anywhere (no PropertyGroup, and not expected to be a real
# environment variable). main.cpp's #error fires if and only if that header
# was not found, independently confirming the entry was really dropped from
# the search path rather than merely failing to warn.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_include_dir_macro():
    args = [
        '--project=vcxproj_include_dir_macro/vcxproj_include_dir_macro.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    # A normal, always-visible message naming the exact unresolved macro path --
    # not silently dropped, and not hidden behind --debug.
    assert "cppcheck: error: AdditionalIncludeDirectories entry has an unresolved macro, " \
           "include path will not be found: '$(CppcheckTestIncDirMacro)/RealInc'" in stdout, stdout

    # myheader.h under RealInc/ must NOT have been found -- the unresolved
    # entry stays inert (literal, matching no real directory), it is not
    # somehow resolved anyway. main.cpp's own #error is the independent proof.
    filename = 'vcxproj_include_dir_macro/main.cpp'
    normalized_stderr = stderr.replace('\\', '/')
    assert ('%s:3:2: error: #error myheader.h (under RealInc, reached only via the unresolved '
            'AdditionalIncludeDirectories macro) was not found [preprocessorErrorDirective]' % filename) in normalized_stderr, stderr
