
# python -m pytest vcxproj_property_order_test.py
#
# Regression coverage for MSBuild/Visual Studio property-before-items evaluation
# order: a project fully resolves every property across the whole file before
# evaluating any ItemDefinitionGroup or ItemGroup, regardless of where in the
# document that property happens to be set. This fixture's ItemDefinitionGroup
# references a property that is only assigned by a PropertyGroup positioned AFTER
# it (and after the ItemGroup) -- if cppcheck evaluated the project as one
# document-order pass instead of a real Properties-then-ItemDefs-then-Items
# sequence, that property would still be unset (and therefore unexpanded) at the
# point the ItemDefinitionGroup is evaluated.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_property_order():
    args = [
        '--project=vcxproj_property_order/vcxproj_property_order.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    dump_file = os.path.join(__script_dir, 'vcxproj_property_order', 'main.cpp.dump')
    assert os.path.exists(dump_file), "Dump file not found at %s" % dump_file

    with open(dump_file, 'rt') as f:
        dump_content = f.read()

    # LateDefinedProp is set by a PropertyGroup below the ItemDefinitionGroup that
    # uses it; the ItemDefinitionGroup must still see its final value (42), not an
    # unexpanded '$(LateDefinedProp)' or an empty expansion.
    assert 'LATE_DEFINE=42' in dump_content, dump_content
