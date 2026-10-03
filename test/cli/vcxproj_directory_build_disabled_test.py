
# python -m pytest vcxproj_directory_build_disabled_test.py
#
# Regression coverage for $(ImportDirectoryBuildProps) / $(ImportDirectoryBuildTargets):
# attemptSyntheticImport()'s emulation of Microsoft.Cpp.Default.props (which normally
# imports Directory.Build.props) and Microsoft.Cpp.targets (which normally imports
# Directory.Build.targets) must honor these controls, which Microsoft documents in
# "Customize your build by folder or solution". Both default to true, but this
# fixture's Globals PropertyGroup -- processed before either import point is
# reached -- sets both to false, so neither Directory.Build.props nor
# Directory.Build.targets (both sitting right next to the project, exactly where
# the automatic upward search would otherwise find them) may be imported.
#
# main.cpp turns either file's ItemDefinitionGroup contribution into a #error, so
# the only expected diagnostic is the deliberate division by zero.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_directory_build_disabled():
    args = [
        '--template=cppcheck1',
        '--project=vcxproj_directory_build_disabled/vcxproj_directory_build_disabled.vcxproj',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    filename = os.path.join('vcxproj_directory_build_disabled', 'main.cpp')
    assert stderr == '[%s:13]: (error) Division by zero.\n' % filename
