
# python -m pytest vcxproj_item_condition_test.py
#
# Regression coverage for Condition on a ClCompile *project item* itself
# (as opposed to Condition on the item's metadata children, which is a
# different, fully-supported feature -- see applyClCompileChild()).
#
# Microsoft documents that the Visual Studio C++ project system does not
# support Condition on project items: "Conditions aren't supported for
# Project items (that is, item types that are treated as project items by
# rules definitions)." -- see
# https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure
#
# This fixture's only <ClCompile> item is Include="foo.cpp"
# Condition="'$(Configuration)'=='Debug'". A discovery/evaluation pass that
# honors that Condition (i.e. treats the item like any other conditioned
# element) drops foo.cpp entirely when evaluating Release|x64 -- but real
# Visual Studio ignores the Condition on the item itself and always includes
# foo.cpp, in both Debug and Release. So foo.cpp must be checked under both
# configurations.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_condition():
    args = [
        '--project=vcxproj_item_condition/vcxproj_item_condition.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    # Windows prints native '\' path separators ("Checking foo\main.cpp ...");
    # normalize before matching so this passes on every platform (same idiom
    # used by test_log() in clang-import_test.py).
    normalized_stdout = stdout.replace('\\', '/')

    # foo.cpp's Condition is gated on Configuration=='Debug', but Visual
    # Studio does not evaluate Condition on project items -- so foo.cpp must
    # be checked under BOTH configurations, not just Debug.
    assert 'Checking vcxproj_item_condition/foo.cpp Debug|x64' in normalized_stdout, stdout
    assert 'Checking vcxproj_item_condition/foo.cpp Release|x64' in normalized_stdout, stdout
