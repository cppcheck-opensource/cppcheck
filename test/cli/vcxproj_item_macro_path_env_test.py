
# python -m pytest vcxproj_item_macro_path_env_test.py
#
# Regression coverage for the environment-variable fallback in
# expandItemSpec()'s ClCompile item-path macro check (see
# vcxproj_item_macro_path_test.py / vcxproj_item_macro_path_invariant_test.py
# for the general rule this is a special case of): a $(...) reference in an
# Include/Update/Remove path to a property no PropertyGroup or property
# sheet in the project ever sets, but that resolves via a real OS
# environment variable of the same name, is config-invariant exactly like a
# property-sheet constant is -- an environment variable has exactly one
# value for the whole cppcheck invocation, it cannot differ between
# Debug|x64 and Release|x64 -- and real MSBuild property evaluation itself
# falls back to the environment for anything nothing else defines. So real
# Visual Studio resolves it, and cppcheck must too.
#
# This fixture's only <ClCompile> item is
# Include="$(CppcheckTestEnvLibRoot)\mathplot.cpp", where
# CppcheckTestEnvLibRoot is not defined anywhere in the project itself.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_macro_path_env_var_set():
    env = os.environ.copy()
    env['CppcheckTestEnvLibRoot'] = 'EnvLibRoot'

    args = [
        '--project=vcxproj_item_macro_path_env/vcxproj_item_macro_path_env.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir, env=env)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    filename = 'vcxproj_item_macro_path_env/EnvLibRoot/mathplot.cpp'

    # Both configurations must resolve $(CppcheckTestEnvLibRoot)\mathplot.cpp
    # via the environment variable to the same real file and actually check
    # it -- not treat the macro as unsupported just because no PropertyGroup
    # in the project itself ever set it.
    assert ('Checking %s Debug|x64...' % filename) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % filename) in normalized_stdout, stdout

    normalized_stderr = stderr.replace('\\', '/')
    assert normalized_stderr.count('%s:6:15: error: Division by zero.' % filename) == 1, stderr


def test_vcxproj_item_macro_path_env_var_unset():
    env = os.environ.copy()
    env.pop('CppcheckTestEnvLibRoot', None)

    args = [
        '--project=vcxproj_item_macro_path_env/vcxproj_item_macro_path_env.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir, env=env)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    normalized_stderr = stderr.replace('\\', '/')
    literal_path = 'vcxproj_item_macro_path_env/$(CppcheckTestEnvLibRoot)/mathplot.cpp'

    # With no environment variable and no project property defining it
    # either, $(CppcheckTestEnvLibRoot) is genuinely unresolvable -- the item
    # must still be checked (and reported missing) under its literal path in
    # both configurations, not silently dropped.
    assert ('Checking %s Debug|x64...' % literal_path) in normalized_stdout, stdout
    assert ('Checking %s Release|x64...' % literal_path) in normalized_stdout, stdout
    assert normalized_stderr.count('%s:0:0: error: File is missing: %s [missingFile]' % (literal_path, literal_path)) == 1, stderr
