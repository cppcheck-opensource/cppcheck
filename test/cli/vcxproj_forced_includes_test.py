
# python -m pytest vcxproj_forced_includes_test.py

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'vcxproj_forced_includes')

def get_lines(s):
    return sorted(s.split('\n'))

def test_vcxproj_forced_includes():
    args = [
        '--template=cppcheck1',
        '--project=vcxproj_forced_includes/vcxproj_forced_includes.cppcheck',
        '--no-cppcheck-build-dir'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename1 = os.path.join('vcxproj_forced_includes', 'DebugX64.cpp')
    filename2 = os.path.join('vcxproj_forced_includes', 'DebugX64.h')
    filename3 = os.path.join('vcxproj_forced_includes', 'AllX64.h')
    filename4 = os.path.join('vcxproj_forced_includes', 'GlobalDebugX64.h')
    filename5 = os.path.join('vcxproj_forced_includes', 'PropsDebugX64.h')
    assert ret == 0, stdout
    expected = (
        '[%s:6]: (error) Division by zero.\n'
        '[%s:4]: (error) Division by zero.\n'
        '[%s:4]: (error) Division by zero.\n'
        '[%s:4]: (error) Division by zero.\n'
        '[%s:4]: (error) Division by zero.\n' % (filename1, filename2, filename3, filename4, filename5)
    )
    assert get_lines(stderr) == get_lines(expected)
