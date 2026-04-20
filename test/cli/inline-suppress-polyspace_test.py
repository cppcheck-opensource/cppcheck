
# python -m pytest inline-suppress-polyspace_test.py

import os
from testutils import assert_cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_unmatched_polyspace_suppression(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write('int f(void); /* polyspace MISRA2012:8.2 */\n')

    args = ['--addon=misra', '--template=simple', '--enable=style,information', '--inline-suppr', 'test.c']

    out_exp = ['Checking test.c ...']
    err_exp = ['test.c:1:14: information: Unmatched suppression: misra-c2012-8.2 [unmatchedPolyspaceSuppression]']

    assert_cppcheck(args, ec_exp=0, err_exp=err_exp, out_exp=out_exp, cwd=str(tmp_path))


def test_1(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write('int f(); /* polyspace MISRA2012:8.2 */\n')

    args = ['--addon=misra', '--template=simple', '--enable=style,information', '--inline-suppr', 'test.c']

    out_exp = ['Checking test.c ...']
    err_exp = []

    assert_cppcheck(args, ec_exp=0, err_exp=err_exp, out_exp=out_exp, cwd=str(tmp_path))


def test_block(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write('/* polyspace +1 MISRA2012:8.2 */\n'
                'int f();\n' # <- suppression applies to this line
                'int g();\n') # <- suppression does not apply to this line

    args = ['--addon=misra', '--template=simple', '--enable=style,information', '--inline-suppr', 'test.c']

    out_exp = ['Checking test.c ...']
    err_exp = ['test.c:3:6: style: misra violation (use --rule-texts=<file> to get proper output) [misra-c2012-8.2]']

    assert_cppcheck(args, ec_exp=0, err_exp=err_exp, out_exp=out_exp, cwd=str(tmp_path))


def test_begin_end(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write('/* polyspace-begin MISRA2012:8.2 */\n'
                'int f();\n'
                '/* polyspace-end MISRA2012:8.2 */\n')

    args = ['--addon=misra', '--template=simple', '--enable=style,information', '--inline-suppr', 'test.c']

    out_exp = ['Checking test.c ...']
    err_exp = []

    assert_cppcheck(args, ec_exp=0, err_exp=err_exp, out_exp=out_exp, cwd=str(tmp_path))
