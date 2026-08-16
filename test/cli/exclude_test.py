
# python -m pytest exclude_test.py

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'exclude')

def test_exclude():
    args = [
        '--template=cppcheck1',
        '--project=exclude/exclude.cppcheck',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('exclude', 'DebugX64.cpp')
    assert ret == 0, stdout
    assert stderr == '[%s:6]: (error) Division by zero.\n' % filename
