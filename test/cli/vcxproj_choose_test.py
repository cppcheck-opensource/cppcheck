
# python -m pytest vcxproj_choose_test.py
#
# Regression coverage for MSBuild/Visual Studio <Choose>/<When>/<Otherwise>
# support: Visual Studio's project evaluator (the same Microsoft.Build engine
# msbuild.exe uses) selects a Choose's branch once, using properties as
# accumulated at that point in the document, and reuses that same branch for
# item-definition and item evaluation -- it does not re-evaluate the <When>
# Condition a second time against final properties. This fixture's <Choose>
# depends on a property that is only assigned by a PropertyGroup positioned
# AFTER the Choose; if cppcheck dropped Choose/When/Otherwise entirely (as it
# used to), the ItemDefinitionGroup inside it would never be evaluated at all,
# and if it instead re-evaluated the When Condition against final properties
# during item-definition/item evaluation, it would pick the wrong branch
# ("When" instead of the "Otherwise" branch Properties actually selected).

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_choose():
    args = [
        '--project=vcxproj_choose/vcxproj_choose.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    dump_file = os.path.join(__script_dir, 'vcxproj_choose', 'main.cpp.dump')
    assert os.path.exists(dump_file), "Dump file not found at %s" % dump_file

    with open(dump_file, 'rt') as f:
        dump_content = f.read()

    # UseSpecialDefine is unset at the point Properties reaches the Choose (it is
    # only assigned afterwards), so the Properties pass selects <Otherwise> --
    # and ItemDefs/Items must replay that same selection rather than
    # re-evaluating the When Condition against the now-final property value.
    assert 'CHOSEN_BRANCH=Otherwise' in dump_content, dump_content
    assert 'CHOSEN_BRANCH=When' not in dump_content, dump_content
