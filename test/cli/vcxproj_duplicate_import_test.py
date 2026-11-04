
# python -m pytest vcxproj_duplicate_import_test.py
#
# Regression coverage for MSBuild/Visual Studio duplicate-import handling
# (MSB4011): once a project file has been imported, a later <Import> of the
# same file (by resolved, case-insensitive path) is a no-op -- its content is
# not evaluated a second time. This fixture imports Common.props via two
# separate <Import> elements; Common.props appends to a property
# (Counter = $(Counter)X) non-idempotently, so double-processing is directly
# observable: processed once, Counter ends up "X"; processed twice, "XX".
# Checked in all three evaluation passes (Properties/ItemDefs/Items), not just
# Properties, since each pass has its own 'imported' dedup state that must
# independently reproduce the same "process once" result.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_duplicate_import():
    args = [
        '--project=vcxproj_duplicate_import/vcxproj_duplicate_import.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    dump_file = os.path.join(__script_dir, 'vcxproj_duplicate_import', 'main.cpp.dump')
    assert os.path.exists(dump_file), "Dump file not found at %s" % dump_file

    with open(dump_file, 'rt') as f:
        dump_content = f.read()

    # Common.props must be evaluated exactly once despite being referenced by
    # two separate <Import> elements -- Counter must be "X", not "XX".
    assert 'COUNTER_VALUE=X;' in dump_content, dump_content
    assert 'COUNTER_VALUE=XX' not in dump_content, dump_content
