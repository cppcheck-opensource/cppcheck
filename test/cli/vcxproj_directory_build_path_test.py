
# python -m pytest vcxproj_directory_build_path_test.py
#
# Regression coverage for $(DirectoryBuildPropsPath) / $(DirectoryBuildTargetsPath):
# when set, Microsoft documents that these override attemptSyntheticImport()'s
# normal upward-directory search (see "Customize your build by folder or
# solution") with an explicit file. This fixture's Globals PropertyGroup sets
# both to "CustomBuild.props"/"CustomBuild.targets", while Directory.Build.props
# and Directory.Build.targets -- which the upward search would otherwise find
# first, since they sit right next to the project -- are also present, each
# defining a macro the *other* file does not. main.cpp turns any mismatch (the
# override not imported, or the plain Directory.Build.* file imported anyway)
# into a #error, so the only expected diagnostic is the deliberate division by
# zero.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_directory_build_path():
    args = [
        '--template=cppcheck1',
        '--project=vcxproj_directory_build_path/vcxproj_directory_build_path.vcxproj',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    filename = os.path.join('vcxproj_directory_build_path', 'main.cpp')
    assert stderr == '[%s:19]: (error) Division by zero.\n' % filename
