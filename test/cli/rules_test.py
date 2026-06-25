import os
import sys

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__root_dir = os.path.abspath(os.path.join(__script_dir, '..', '..'))
__rules_dir = os.path.join(__root_dir, 'rules')


def test_empty_catch_block(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
    try
    {
    }
    catch (...)
    {
    }
}
""")

    rule_file = os.path.join(__rules_dir, 'empty-catch-block.xml')
    args = [
        '--template=simple',
        '--rule-file={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule: \\}\\s*catch\\s*\\(.*\\)\\s*\\{\\s*\\}'
    ]
    assert stderr.splitlines() == [
        '{}:6:0: style: Empty catch block found. [rule]'.format(test_file)
    ]


def test_show_all_defines(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
#define DEF_1

void f()
{
}
""")

    rule_file = os.path.join(__rules_dir, 'show-all-defines.rule')
    args = [
        '--template=simple',
        '-DDEF_2',
        '--rule-file={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule: .*',
        'Checking {}: DEF_2=1...'.format(test_file)
    ]
    if sys.platform == 'win32':
        test_file = str(test_file).replace('\\', '/')
    assert stderr.splitlines() == [
        # TODO: this message looks strange
        ":1:0: information: found ' # line 2 \"{}\" # define DEF_1' [showalldefines]".format(test_file)
    ]


def test_stl(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
    std::string s;
    if (s.find("t") == 17)
    {
    }
}
""")

    rule_file = os.path.join(__rules_dir, 'stl.xml')
    args = [
        '--template=simple',
        '--rule-file={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule:  \\. find \\( "[^"]+?" \\) == \\d+ '
    ]
    assert stderr.splitlines() == [
        '{}:5:0: performance: When looking for a string at a fixed position compare [UselessSTDStringFind]'.format(test_file)
    ]


def test_strlen_empty_str(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f(const char* s)
{
    if (strlen(s) > 0)
    {
    }
}
""")

    rule_file = os.path.join(__rules_dir, 'strlen-empty-str.xml')
    args = [
        '--template=simple',
        '--rule-file={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule:  if \\( ([!] )*?(strlen) \\( \\w+? \\) ([>] [0] )*?\\) { '
    ]
    assert stderr.splitlines() == [
        '{}:4:0: performance: Using strlen() to check if a string is empty is not efficient. [StrlenEmptyString]'.format(test_file)
    ]


def test_suggest_nullptr(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
    const char* p = 0;
}
""")

    rule_file = os.path.join(__rules_dir, 'suggest_nullptr.xml')
    args = [
        '--template=simple',
        '--rule-file={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule: (\\b\\w+\\b) \\* (\\b\\w+\\b) = 0 ;'
    ]
    assert stderr.splitlines() == [
        "{}:4:0: style: Prefer to use a 'nullptr' instead of initializing a pointer with 0. [modernizeUseNullPtr]".format(test_file)
    ]


def test_unused_deref(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f(const char* p)
{
    *p++;
}
""")

    rule_file = os.path.join(__rules_dir, 'unused-deref.xml')
    args = [
        '--template=simple',
        '--rule-file={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule:  [;{}] [*] \\w+? (\\+\\+|\\-\\-) ; '
    ]
    assert stderr.splitlines() == [
        '{}:3:0: style: Redundant * found, "*p++" is the same as "*(p++)". [UnusedDeref]'.format(test_file)
    ]
