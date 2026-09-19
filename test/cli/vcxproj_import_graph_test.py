# python -m pytest vcxproj_import_graph_test.py
#
# MSBuild (and therefore Visual Studio) resolves the import graph exactly once, while
# evaluating properties, and then walks that same graph for item definitions and items.
# This fixture checks that cppcheck's three-phase evaluation does the same:
#   - an <Import> guarded by a property that the imported file itself sets
#     (<Import Project="guarded.props" Condition="'$(GuardedImported)' != 'true'"/>)
#     still contributes its ItemDefinitionGroup
#   - a file that is imported twice is processed only once (MSB4011)
#   - Directory.Build.props, imported through the Microsoft.Cpp.Default.props emulation,
#     contributes its ItemDefinitionGroup as well
# main.cpp turns every one of these into an #error, so the only expected diagnostic is
# the deliberate division by zero.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_import_graph():
    args = [
        '--template=cppcheck1',
        '--project=vcxproj_import_graph/vcxproj_import_graph.slnx',
        '--project-configuration=Debug|x64',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    filename = os.path.join('vcxproj_import_graph', 'main.cpp')
    assert stderr == '[%s:18]: (error) Division by zero.\n' % filename
