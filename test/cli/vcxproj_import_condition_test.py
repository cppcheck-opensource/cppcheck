
# python -m pytest vcxproj_import_condition_test.py
#
# Regression coverage for MSBuild/Visual Studio's three-pass evaluation order:
# an <Import>'s Condition is decided exactly once, during the Properties pass,
# using properties as accumulated at the point Properties reaches it -- not
# re-evaluated during the ItemDefs/Items passes against end-of-Properties
# (final) property state. This fixture's <Import Project="conditional.props">
# has a Condition that depends on a property (LateFlag) which is only assigned
# by a PropertyGroup positioned AFTER the Import -- so the Properties pass
# sees it unset and does not take the import. If ItemDefs/Items instead
# re-evaluated the same Condition against the now-final properties (where
# LateFlag=true), they would incorrectly take the import there and pick up
# conditional.props's ItemDefinitionGroup, leaking SHOULD_NOT_APPEAR into the
# dump even though the import was never actually part of the resolved graph
# Properties built.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_import_condition():
    args = [
        '--project=vcxproj_import_condition/vcxproj_import_condition.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    dump_file = os.path.join(__script_dir, 'vcxproj_import_condition', 'main.cpp.dump')
    assert os.path.exists(dump_file), "Dump file not found at %s" % dump_file

    with open(dump_file, 'rt') as f:
        dump_content = f.read()

    # conditional.props must stay un-imported in every phase -- its Condition
    # was false when Properties decided it, and that decision must be replayed
    # (not re-evaluated) by ItemDefs/Items.
    assert 'SHOULD_NOT_APPEAR' not in dump_content, dump_content
