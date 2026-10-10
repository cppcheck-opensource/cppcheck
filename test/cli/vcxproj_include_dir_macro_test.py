# python -m pytest vcxproj_include_dir_macro_test.py
#
# Regression coverage for an unresolvable macro in AdditionalIncludeDirectories
# (fsSetIncludePaths(), lib/importproject.cpp).
#
# MSBuild expands an undefined property to the empty string, and the result is
# handed to cl.exe like any other include directory. cppcheck must do the same
# rather than keep the literal "$(...)" text, which can never name a real
# directory. Whether the expanded entry finds anything depends on what is left:
#
#   $(Undefined)\RealInc -> "\RealInc"  root of the project directory's drive
#   $(Undefined)RealInc  -> "RealInc"   relative to the project directory
#
# Both fixture projects compile main.cpp, which #includes "myheader.h" --
# present only under RealInc/ next to the projects. CppcheckTestIncDirMacro is
# not defined anywhere (no PropertyGroup, and not expected to be a real
# environment variable). main.cpp's #error fires if and only if the header was
# not found, which independently shows how the entry was resolved.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

__not_found = ('vcxproj_include_dir_macro/main.cpp:3:2: error: #error myheader.h (under RealInc) '
               'was not found [preprocessorErrorDirective]')


def __run(project):
    args = [
        '--project=vcxproj_include_dir_macro/%s' % project,
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    # The unresolved macro is expanded to an empty string, as Visual Studio
    # does -- it is not reported as a project import error.
    assert 'cppcheck: error:' not in stdout, stdout
    return stderr.replace('\\', '/')


def test_vcxproj_include_dir_macro_root_relative():
    # "\RealInc" is the root of the drive, not the RealInc folder next to the
    # project, so myheader.h must NOT be found.
    stderr = __run('vcxproj_include_dir_macro.vcxproj')
    assert __not_found in stderr, stderr


def test_vcxproj_include_dir_macro_relative():
    # "RealInc" is relative to the project directory, so myheader.h IS found.
    stderr = __run('vcxproj_include_dir_macro_relative.vcxproj')
    assert __not_found not in stderr, stderr
    assert 'preprocessorErrorDirective' not in stderr, stderr
