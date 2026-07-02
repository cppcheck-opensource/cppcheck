
# python -m pytest slnx-folders_test.py

import os
import re

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'slnx-folders')

def get_lines(s):
    return sorted(s.split('\n'))

# Get Visual Studio configurations checking a file
# Checking {file} {config}...
def __getVsConfigs(stdout, filename):
    ret = []
    for line in stdout.split('\n'):
        if not line.startswith('Checking %s ' % filename):
            continue
        if not line.endswith('...'):
            continue
        res = re.match(r'.* ([A-Za-z0-9|]+)...', line)
        if res:
            ret.append(res.group(1))
    ret.sort()
    return ' '.join(ret)

def test_relative_path():
    args = [
        '--template=cppcheck1',
        'slnx-folders'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename1 = os.path.join('slnx-folders', 'app', 'app.cpp')
    filename2 = os.path.join('slnx-folders', 'lib', 'lib.cpp')
    assert ret == 0, stdout
    expected = (
        '[%s:5]: (error) Division by zero.\n'
        '[%s:7]: (error) Division by zero.\n' % (filename1, filename2)
    )
    assert get_lines(stderr) == get_lines(expected)

def test_local_path():
    args = [
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    filename1 = os.path.join('app', 'app.cpp')
    filename2 = os.path.join('lib', 'lib.cpp')
    assert ret == 0, stdout
    expected = (
        '[%s:5]: (error) Division by zero.\n'
        '[%s:7]: (error) Division by zero.\n' % (filename1, filename2)
    )
    assert get_lines(stderr) == get_lines(expected)

def test_absolute_path():
    args = [
        '--template=cppcheck1',
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    filename1 = os.path.join(__proj_dir, 'app', 'app.cpp')
    filename2 = os.path.join(__proj_dir, 'lib', 'lib.cpp')
    assert ret == 0, stdout
    expected = (
        '[%s:5]: (error) Division by zero.\n'
        '[%s:7]: (error) Division by zero.\n' % (filename1, filename2)
    )
    assert get_lines(stderr) == get_lines(expected)
