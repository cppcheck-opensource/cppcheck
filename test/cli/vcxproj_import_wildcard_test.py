
# python -m pytest vcxproj_import_wildcard_test.py
#
# Regression coverage for wildcard <Import> support: MSBuild allows wildcards in
# an <Import>'s Project attribute -- a *different*, general mechanism from the
# Visual Studio C++ project-system restriction against wildcards in project
# items (see vcxproj_split_configs_test.py and expandItemSpec()'s rejection of
# '*'/'?' there). This is the mechanism behind the ImportBefore/ImportAfter
# extensibility folders Visual Studio's own C++ toolchain files use, e.g.
# $(VCTargetsPath)\ImportBefore\Default\*.props -- but it applies equally to
# any ordinary user-authored wildcard <Import>, which is what this fixture
# exercises directly (rather than depending on the synthetic, hand-emulated
# Microsoft.Cpp.*.props/targets handlers, which do not read the real files and
# so never reach a wildcard import inside them).
#
# This fixture's <Import Project="ImportBefore\*.props" /> must expand to
# BOTH a.props and b.props (sorted, "as if the order had been explicitly
# set" per Microsoft's own docs), while skipping decoy.txt (its extension
# doesn't match "*.props", so it must never even be attempted as XML -- it
# isn't valid XML, so a match would make cppcheck fail outright). A second
# <Import Project="ImportAfter\*.props" /> pointing at a directory that does
# not exist at all must be a silent no-op, not an error.
#
# main.cpp turns any of these going wrong into a #error, so the only expected
# diagnostic is the deliberate division by zero.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_import_wildcard():
    args = [
        '--template=cppcheck1',
        '--project=vcxproj_import_wildcard/vcxproj_import_wildcard.vcxproj',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    filename = os.path.join('vcxproj_import_wildcard', 'main.cpp')
    assert stderr == '[%s:16]: (error) Division by zero.\n' % filename
