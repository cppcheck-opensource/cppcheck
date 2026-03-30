
# python -m pytest test-other.py

import os
import sys
import pytest
import glob
import json
import subprocess
import shutil

from testutils import cppcheck, assert_cppcheck, cppcheck_ex, __lookup_cppcheck_exe
from xml.etree import ElementTree


def __remove_verbose_log(l : list):
    l.remove('Defines:')
    l.remove('Undefines:')
    l.remove('Includes:')
    l.remove('Platform:native')
    return l


def test_missing_include(tmpdir):  # #11283
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
#include "test.h"
""")

    args = ['--enable=missingInclude', '--template=simple', test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == '{}:2:2: information: Include file: "test.h" not found. [missingInclude]\n'.format(test_file)


def __test_missing_include_check_config(tmpdir, use_j):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    # TODO: -rp is not working requiring the full path in the assert
    args = '--check-config -rp={} {}'.format(tmpdir, test_file)
    if use_j:
        args = '-j2 ' + args

    _, _, stderr = cppcheck(args.split())
    assert stderr == '' # --check-config no longer reports the missing includes


def test_missing_include_check_config(tmpdir):
    __test_missing_include_check_config(tmpdir, False)


def test_missing_include_check_config_j(tmpdir):
    __test_missing_include_check_config(tmpdir, True)


def test_missing_include_inline_suppr(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                // cppcheck-suppress missingInclude
                #include "missing.h"
                // cppcheck-suppress missingIncludeSystem
                #include <missing2.h>
                """)

    args = ['--enable=missingInclude', '--inline-suppr', test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == ''


def test_preprocessor_error(tmpdir):
    test_file = os.path.join(tmpdir, '10866.c')
    with open(test_file, 'wt') as f:
        f.write('#error test\nx=1;\n')
    exitcode, _, stderr = cppcheck(['--error-exitcode=1', test_file])
    assert 'preprocessorErrorDirective' in stderr
    assert exitcode != 0


__ANSI_BOLD = "\x1b[1m"
__ANSI_FG_RED = "\x1b[31m"
__ANSI_FG_DEFAULT = "\x1b[39m"
__ANSI_FG_RESET = "\x1b[0m"


@pytest.mark.parametrize("env,color_expected", [({"CLICOLOR_FORCE":"1"}, True), ({"NO_COLOR": "1", "CLICOLOR_FORCE":"1"}, False)])
def test_color_non_tty(tmpdir, env, color_expected):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('#error test\nx=1;\n')
    exitcode, stdout, stderr = cppcheck([test_file], env=env)

    assert exitcode == 0, stdout if stdout else stderr
    assert stderr
    assert (__ANSI_BOLD in stderr) == color_expected
    assert (__ANSI_FG_RED in stderr) == color_expected
    assert (__ANSI_FG_DEFAULT in stderr) == color_expected
    assert (__ANSI_FG_RESET in stderr) == color_expected


@pytest.mark.skipif(sys.platform == "win32", reason="TTY not supported in Windows")
@pytest.mark.parametrize("env,color_expected", [({}, True), ({"NO_COLOR": "1"}, False)])
def test_color_tty(tmpdir, env, color_expected):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('#error test\nx=1;\n')
    exitcode, stdout, stderr = cppcheck([test_file], env=env, tty=True)

    assert exitcode == 0, stdout if stdout else stderr
    assert stderr
    assert (__ANSI_BOLD in stderr) == color_expected
    assert (__ANSI_FG_RED in stderr) == color_expected
    assert (__ANSI_FG_DEFAULT in stderr) == color_expected
    assert (__ANSI_FG_RESET in stderr) == color_expected


def test_invalid_library(tmpdir):
    args = ['--library=none', '--library=posix', '--library=none2', 'file.c']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    assert (stdout == "cppcheck: Failed to load library configuration file 'none'. File not found\n"
                      "cppcheck: Failed to load library configuration file 'none2'. File not found\n")
    assert stderr == ""


def test_message_j(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("")

    args = ['-j2', test_file]

    _, stdout, _ = cppcheck(args)
    assert stdout == "Checking {} ...\n".format(test_file) # we were adding stray \0 characters at the end

# TODO: test missing std.cfg


def test_progress(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--report-progress=0', '--enable=all', '--inconclusive', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    pos = stdout.find('\n')
    assert(pos != -1)
    pos += 1
    assert stdout[:pos] == "Checking {} ...\n".format(test_file)
    assert (stdout[pos:] ==
            "progress: Tokenize (typedef) 0%\n"
            "progress: Tokenize (typedef) 12%\n"
            "progress: Tokenize (typedef) 25%\n"
            "progress: Tokenize (typedef) 37%\n"
            "progress: Tokenize (typedef) 50%\n"
            "progress: Tokenize (typedef) 62%\n"
            "progress: Tokenize (typedef) 75%\n"
            "progress: Tokenize (typedef) 87%\n"
            "progress: Tokenize (typedef) 100%\n"
            "progress: SymbolDatabase (find all scopes) 0%\n"
            "progress: SymbolDatabase (find all scopes) 12%\n"
            "progress: SymbolDatabase (find all scopes) 87%\n"
            "progress: SymbolDatabase (find all scopes) 100%\n"
            "progress: ValueFlow 0%\n"
            "progress: ValueFlow::valueFlowImpossibleValues(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowImpossibleValues(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowSymbolicOperators(symboldatabase, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowSymbolicOperators(symboldatabase, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowCondition(SymbolicConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 0%\n"
            "progress: ValueFlow::valueFlowCondition(SymbolicConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 100%\n"
            "progress: ValueFlow::valueFlowSymbolicInfer(symboldatabase, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowSymbolicInfer(symboldatabase, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowArrayBool(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowArrayBool(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowArrayElement(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowArrayElement(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowRightShift(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowRightShift(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowCondition(ContainerConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 0%\n"
            "progress: ValueFlow::valueFlowCondition(ContainerConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 100%\n"
            "progress: ValueFlow::valueFlowAfterAssign(tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 0%\n"
            "progress: ValueFlow::valueFlowAfterAssign(tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 100%\n"
            "progress: ValueFlow::valueFlowAfterSwap(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowAfterSwap(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowCondition(SimpleConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 0%\n"
            "progress: ValueFlow::valueFlowCondition(SimpleConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 100%\n"
            "progress: ValueFlow::valueFlowInferCondition(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowInferCondition(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowSwitchVariable(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowSwitchVariable(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowForLoop(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowForLoop(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowSubFunction(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowSubFunction(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowFunctionReturn(tokenlist, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowFunctionReturn(tokenlist, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowLifetime(tokenlist, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowLifetime(tokenlist, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowFunctionDefaultParameter(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowFunctionDefaultParameter(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowUninit(tokenlist, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowUninit(tokenlist, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowAfterMove(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowAfterMove(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowSmartPointer(tokenlist, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowSmartPointer(tokenlist, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowIterators(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowIterators(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowCondition(IteratorConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 0%\n"
            "progress: ValueFlow::valueFlowCondition(IteratorConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 100%\n"
            "progress: ValueFlow::valueFlowIteratorInfer(tokenlist, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowIteratorInfer(tokenlist, settings) 1 100%\n"
            "progress: ValueFlow::valueFlowContainerSize(tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 0%\n"
            "progress: ValueFlow::valueFlowContainerSize(tokenlist, symboldatabase, errorLogger, settings, skippedFunctions) 1 100%\n"
            "progress: ValueFlow::valueFlowSafeFunctions(tokenlist, symboldatabase, errorLogger, settings) 1 0%\n"
            "progress: ValueFlow::valueFlowSafeFunctions(tokenlist, symboldatabase, errorLogger, settings) 1 100%\n"
            "progress: ValueFlow 100%\n"
            "progress: Run checkers 0%\n"
            "progress: Run checkers 100%\n"
            )
    assert stderr == ""


def test_progress_j(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--report-progress=0', '--enable=all', '--inconclusive', '-j2', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    assert stdout == "Checking {} ...\n".format(test_file)
    assert stderr == ""


def test_execute_addon_failure_py_auto(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    args = ['--addon=naming', test_file]

    # provide empty PATH environment variable so python is not found and execution of addon fails
    env = {'PATH': ''}
    _, _, stderr = cppcheck(args, env)
    assert stderr == '{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to auto detect python [internalError]\n\n^\n'.format(test_file)


def test_execute_addon_failure_py_notexist(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    # specify non-existent python executable so execution of addon fails
    args = ['--addon=naming', '--addon-python=python5.x', test_file]

    _, _, stderr = cppcheck(args)
    ec = 1 if os.name == 'nt' else 127
    assert stderr == "{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'naming' - exitcode is {} [internalError]\n\n^\n".format(test_file, ec)


def test_execute_addon_failure_json_notexist(tmpdir):
    # specify non-existent python executable so execution of addon fails
    addon_json = os.path.join(tmpdir, 'addon.json')
    with open(addon_json, 'wt') as f:
        f.write(json.dumps({'executable': 'notexist'}))

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    args = [
        '--addon={}'.format(addon_json),
        test_file
    ]

    _, _, stderr = cppcheck(args)
    ec = 1 if os.name == 'nt' else 127
    assert stderr == "{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon.json' - exitcode is {} [internalError]\n\n^\n".format(test_file, ec)


@pytest.mark.skipif(sys.platform != "win32", reason="Windows specific issue")
def test_execute_addon_path_with_spaces(tmpdir):
    addon_json = os.path.join(tmpdir, 'addon.json')
    addon_dir = os.path.join(tmpdir, 'A Folder')
    addon_script = os.path.join(addon_dir, 'addon.bat')

    with open(addon_json, 'wt') as f:
        f.write(json.dumps({'executable': addon_script }))

    os.makedirs(addon_dir, exist_ok=True)

    with open(addon_script, 'wt') as f:
        f.write('@echo {"file":"1.c","linenr":1,"column":1,"severity":"error","message":"hello world","errorId":"hello","addon":"test"}')

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    args = [
        '--addon={}'.format(addon_json),
        test_file,
    ]

    _, _, stderr = cppcheck(args)

    # Make sure the full command is used
    assert '1.c:1:1: error: hello world [test-hello]\n' in stderr


def test_execute_addon_failure_json_ctu_notexist(tmpdir):
    # specify non-existent python executable so execution of addon fails
    addon_json = os.path.join(tmpdir, 'addon.json')
    with open(addon_json, 'wt') as f:
        f.write(json.dumps({
            'executable': 'notexist',
            'ctu': True
        }))

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f(); """)

    args = [
        '--template=simple',
        '--addon={}'.format(addon_json),
        test_file
    ]

    _, _, stderr = cppcheck(args)
    ec = 1 if os.name == 'nt' else 127
    assert stderr.splitlines() == [
        "{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon.json' - exitcode is {} [internalError]".format(test_file, ec),
        ":0:0: error: Bailing out from analysis: Whole program analysis failed: Failed to execute addon 'addon.json' - exitcode is {} [internalError]".format(ec)
    ]


def test_execute_addon_file0(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('void foo() {}\n')

    args = ['--xml', '--addon=misra', '--enable=style', test_file]

    _, _, stderr = cppcheck(args)
    assert 'misra-c2012-8.2' in stderr
    assert '.dump' not in stderr


# TODO: find a test case which always fails
@pytest.mark.skip
def test_internal_error(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#include <cstdio>

void f() {
    double gc = 3333.3333;
    char stat[80];
    sprintf(stat,"'%2.1f'",gc);
}
                """)

    args = [test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == '{}:0:0: error: Bailing from out analysis: Checking file failed: converting \'1f\' to integer failed - not an integer [internalError]\n\n^\n'.format(test_file)


def test_addon_ctu_exitcode(tmpdir):
    """ #12440 - Misra ctu violations found => exit code should be non-zero """
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""typedef enum { BLOCK =  0x80U, } E;""")
    args = ['--addon=misra', '--enable=style', '--error-exitcode=1', test_file]
    exitcode, _, stderr = cppcheck(args)
    assert '2.3' in stderr, stderr
    assert exitcode == 1


# TODO: test with -j2
def test_addon_misra(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
        """)

    args = ['--addon=misra', '--enable=all', '--disable=unusedFunction', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:2:1: style: misra violation (use --rule-texts=<file> to get proper output) [misra-c2012-2.3]\ntypedef int MISRA_5_6_VIOLATION;\n^\n'.format(test_file)


def test_addon_y2038(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    # TODO: trigger warning
    with open(test_file, 'wt') as f:
        f.write("""
extern void f()
{
    time_t t = std::time(nullptr);
    (void)t;
}
        """)

    args = ['--addon=y2038', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:21: warning: time is Y2038-unsafe [y2038-unsafe-call]\n'.format(test_file)


def test_addon_threadsafety(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
extern const char* f()
{
    return strerror(1);
}
        """)

    args = ['--addon=threadsafety', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:12: warning: strerror is MT-unsafe [threadsafety-unsafe-call]\n'.format(test_file)


def test_addon_naming(tmpdir):
    # the addon does nothing without a config
    addon_file = os.path.join(tmpdir, 'naming1.json')
    with open(addon_file, 'wt') as f:
        f.write("""
{
    "script": "addons/naming.py",
    "args": [
        "--var=[_a-z].*"
    ]
}
                """)

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
int Var;
        """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:2:1: style: Variable Var violates naming convention [naming-varname]\n'.format(test_file)


def test_addon_namingng(tmpdir):
    addon_file = os.path.join(tmpdir, 'namingng.json')
    addon_config_file = os.path.join(tmpdir, 'namingng.config.json')
    with open(addon_file, 'wt') as f:
        f.write("""
{
    "script": "addons/namingng.py",
    "args": [
        "--configfile=%s"
    ]
}
                """%(addon_config_file).replace('\\','\\\\'))

    with open(addon_config_file, 'wt') as f:
        f.write("""
{
    "RE_FILE": [
        "[^/]*[a-z][a-z0-9_]*[a-z0-9]\\.c\\Z"
    ],
    "RE_CLASS_NAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_NAMESPACE": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_VARNAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_PUBLIC_MEMBER_VARIABLE": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_PRIVATE_MEMBER_VARIABLE": {
        ".*_tmp\\Z":[true,"illegal suffix _tmp"],
        "priv_.*\\Z":[false,"required prefix priv_ missing"]
    },
    "RE_GLOBAL_VARNAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_FUNCTIONNAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "include_guard": {
        "input": "basename",
        "prefix": "_",
        "suffix": "",
        "case": "upper",
        "max_linenr": 5,
        "RE_HEADERFILE": ".*\\.h\\Z",
        "required": true
    },
    "var_prefixes": {"uint32_t": "ui32"},
    "function_prefixes": {"uint16_t": "ui16",
                          "uint32_t": "ui32"},
    "skip_one_char_variables": false
}
                """.replace('\\','\\\\'))

    test_unguarded_include_file_basename = 'test_unguarded.h'
    test_unguarded_include_file = os.path.join(tmpdir, test_unguarded_include_file_basename)
    with open(test_unguarded_include_file, 'wt') as f:
        f.write("""
void InvalidFunctionUnguarded();
""")

    test_include_file_basename = '_test.h'
    test_include_file = os.path.join(tmpdir, test_include_file_basename)
    with open(test_include_file, 'wt') as f:
        f.write("""
#ifndef TEST_H
#define TEST_H

void InvalidFunction();
extern int _invalid_extern_global;

#include "{}"

#endif
""".format(test_unguarded_include_file))

    test_file_basename = 'test_.cpp'
    test_file = os.path.join(tmpdir, test_file_basename)
    with open(test_file, 'wt') as f:
        f.write("""
#include "%s"

void invalid_function_();
void _invalid_function();
void valid_function1();
void valid_function2(int _invalid_arg);
void valid_function3(int invalid_arg_);
void valid_function4(int valid_arg32);
void valid_function5(uint32_t invalid_arg32);
void valid_function6(uint32_t ui32_valid_arg);
uint16_t invalid_function7(int valid_arg);
uint16_t ui16_valid_function8(int valid_arg);

int _invalid_global;
static int _invalid_static_global;

class _clz {
public:
    _clz() : _invalid_public(0), _invalid_private(0), priv_good(0), priv_bad_tmp(0) { }
    int _invalid_public;
private:
    char _invalid_private;
    int priv_good;
    int priv_bad_tmp;
};

namespace _invalid_namespace { }
        """%(test_include_file_basename))

    args = ['--addon='+addon_file, '--verbose', '--enable=all', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_verbose_log(stdout.splitlines())
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    lines = [line for line in stderr.splitlines() if line != '']
    expect = [
        '{}:0:0: style: File name {} violates naming convention [namingng-namingConvention]'.format(test_include_file,test_include_file_basename),
        '^',
        '{}:2:9: style: include guard naming violation; TEST_H != _TEST_H [namingng-includeGuardName]'.format(test_include_file),
        '#ifndef TEST_H',
        '        ^',
        '{}:5:6: style: Function InvalidFunction violates naming convention [namingng-namingConvention]'.format(test_include_file),
        'void InvalidFunction();',
        '     ^',
        '{}:6:12: style: Global variable _invalid_extern_global violates naming convention [namingng-namingConvention]'.format(test_include_file),
        'extern int _invalid_extern_global;',
        '           ^',

        '{}:0:0: style: File name {} violates naming convention [namingng-namingConvention]'.format(test_unguarded_include_file,test_unguarded_include_file_basename),
        '^',
        '{}:0:0: style: Missing include guard [namingng-includeGuardMissing]'.format(test_unguarded_include_file),
        '^',
        '{}:2:6: style: Function InvalidFunctionUnguarded violates naming convention [namingng-namingConvention]'.format(test_unguarded_include_file),
        'void InvalidFunctionUnguarded();',
        '     ^',

        '{}:0:0: style: File name {} violates naming convention [namingng-namingConvention]'.format(test_file,test_file_basename),
        '^',
        '{}:7:26: style: Variable _invalid_arg violates naming convention [namingng-namingConvention]'.format(test_file),
        'void valid_function2(int _invalid_arg);',
        '                         ^',
        '{}:8:26: style: Variable invalid_arg_ violates naming convention [namingng-namingConvention]'.format(test_file),
        'void valid_function3(int invalid_arg_);',
        '                         ^',
        '{}:10:31: style: Variable invalid_arg32 violates naming convention [namingng-namingConvention]'.format(test_file),
        'void valid_function5(uint32_t invalid_arg32);',
        '                              ^',
        '{}:4:6: style: Function invalid_function_ violates naming convention [namingng-namingConvention]'.format(test_file),
        'void invalid_function_();',
        '     ^',
        '{}:5:6: style: Function _invalid_function violates naming convention [namingng-namingConvention]'.format(test_file),
        'void _invalid_function();',
        '     ^',
        '{}:12:10: style: Function invalid_function7 violates naming convention [namingng-namingConvention]'.format(test_file),
        'uint16_t invalid_function7(int valid_arg);',
        '         ^',
        '{}:15:5: style: Global variable _invalid_global violates naming convention [namingng-namingConvention]'.format(test_file),
        'int _invalid_global;',
        '    ^',
        '{}:16:12: style: Global variable _invalid_static_global violates naming convention [namingng-namingConvention]'.format(test_file),
        'static int _invalid_static_global;',
        '           ^',
        '{}:20:5: style: Class Constructor _clz violates naming convention [namingng-namingConvention]'.format(test_file),
        '    _clz() : _invalid_public(0), _invalid_private(0), priv_good(0), priv_bad_tmp(0) { }',
        '    ^',
        '{}:21:9: style: Public member variable _invalid_public violates naming convention [namingng-namingConvention]'.format(test_file),
        '    int _invalid_public;',
        '        ^',
        '{}:23:10: style: Private member variable _invalid_private violates naming convention: required prefix priv_ missing [namingng-namingConvention]'.format(test_file),
        '    char _invalid_private;',
        '         ^',
        '{}:25:9: style: Private member variable priv_bad_tmp violates naming convention: illegal suffix _tmp [namingng-namingConvention]'.format(test_file),
        '    int priv_bad_tmp;',
        '        ^',
        '{}:28:11: style: Namespace _invalid_namespace violates naming convention [namingng-namingConvention]'.format(test_file),
        'namespace _invalid_namespace { }',
        '          ^',
    ]
    # test sorted lines; the order of messages may vary and is not of importance
    lines.sort()
    expect.sort()
    assert lines == expect


# TODO: test with -j2
def test_addon_namingng_config(tmpdir):
    addon_file = os.path.join(tmpdir, 'namingng.json')
    addon_config_file = os.path.join(tmpdir, 'namingng.config.json')
    with open(addon_file, 'wt') as f:
        f.write("""
{
    "script": "addons/namingng.py",
    "args": [
        "--configfile=%s"
    ]
}
                """%(addon_config_file).replace('\\','\\\\'))

    with open(addon_config_file, 'wt') as f:
        f.write("""
{
    "RE_FILE": "[^/]*[a-z][a-z0-9_]*[a-z0-9]\\.c\\Z",
    "RE_NAMESPACE": false,
    "RE_VARNAME": ["+bad pattern","[a-z]_good_pattern\\Z","(parentheses?"],
    "RE_PRIVATE_MEMBER_VARIABLE": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "RE_PUBLIC_MEMBER_VARIABLE": {
        "tmp_.*\\Z":[true,"illegal prefix tmp_"],
        "bad_.*\\Z":true,
        "public_.*\\Z":[false],
        "pub_.*\\Z":[0,"required prefix pub_ missing"]
    },
    "RE_GLOBAL_VARNAME": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "RE_FUNCTIONNAME": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "RE_CLASS_NAME": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "_comment1": "these should all be arrays, or null, or not set",

    "include_guard": true,
    "var_prefixes": ["bad"],
    "function_prefixes": false,
    "_comment2": "these should all be dict",

    "skip_one_char_variables": "false",
    "_comment3": "this should be bool",

    "RE_VAR_NAME": "typo"
}
                """.replace('\\','\\\\'))

    test_file_basename = 'test.c'
    test_file = os.path.join(tmpdir, test_file_basename)
    with open(test_file, 'a'):
        # only create the file
        pass

    args = ['--addon='+addon_file, '--verbose', '--enable=all', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr

    lines = __remove_verbose_log(stdout.splitlines())
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    lines = stderr.splitlines()
    # ignore the first line, stating that the addon failed to run properly
    lines.pop(0)
    assert lines == [
        "Output:",
        "config error: RE_FILE must be list (not str), or not set",
        "config error: RE_NAMESPACE must be list or dict (not bool), or not set",
        "config error: include_guard must be dict (not bool), or not set",
        "config error: item '+bad pattern' of 'RE_VARNAME' is not a valid regular expression: nothing to repeat at position 0",
        "config error: item '(parentheses?' of 'RE_VARNAME' is not a valid regular expression: missing ), unterminated subpattern at position 0",
        "config error: var_prefixes must be dict (not list), or not set",
        "config error: RE_PRIVATE_MEMBER_VARIABLE must be list or dict (not str), or not set",
        "config error: item 'bad_.*\\Z' of 'RE_PUBLIC_MEMBER_VARIABLE' must be an array [bool,string]",
        "config error: item 'public_.*\\Z' of 'RE_PUBLIC_MEMBER_VARIABLE' must be an array [bool,string]",
        "config error: item 'pub_.*\\Z' of 'RE_PUBLIC_MEMBER_VARIABLE' must be an array [bool,string]",
        "config error: RE_GLOBAL_VARNAME must be list or dict (not str), or not set",
        "config error: RE_FUNCTIONNAME must be list or dict (not str), or not set",
        "config error: function_prefixes must be dict (not bool), or not set",
        "config error: RE_CLASS_NAME must be list or dict (not str), or not set",
        "config error: skip_one_char_variables must be bool (not str), or not set",
        "config error: unknown config key 'RE_VAR_NAME' [internalError]",
        "",
        "^",
    ]


def test_addon_findcasts(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
        extern void f(char c)
        {
            int i = (int)c;
            (void)i;
        }
        """)

    args = ['--addon=findcasts', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:21: information: found a cast [findcasts-cast]\n'.format(test_file)


def test_addon_misc(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
extern void f()
{
    const char* c[] = {"a" "b"};
}
        """)

    args = ['--addon=misc', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:28: style: String concatenation in array initialization, missing comma? [misc-stringConcatInArrayInit]\n'.format(test_file)


def test_invalid_addon_json(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.json')
    with open(addon_file, 'wt') as f:
        f.write("""
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--addon={}'.format(addon_file), test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    lines = stdout.splitlines()
    assert lines == [
        'Loading {} failed. syntax error at line 2 near: '.format(addon_file)
    ]
    assert stderr == ''


def test_invalid_addon_py(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.py')
    with open(addon_file, 'wt') as f:
        f.write("""
raise Exception()
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
                """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == "{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 1 [internalError]\n\n^\n".format(test_file)


# TODO: test with -j2
def test_invalid_addon_py_verbose(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.py')
    with open(addon_file, 'wt') as f:
        f.write("""
raise Exception()
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
                """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', '--verbose', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = __remove_verbose_log(stdout.splitlines())
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    """
/tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/file.cpp:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 1: python3 /home/user/CLionProjects/cppcheck/addons/runaddon.py /tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/addon1.py --cli /tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/file.cpp.24762.dump
Output:
Traceback (most recent call last):
  File "/home/user/CLionProjects/cppcheck/addons/runaddon.py", line 8, in <module>
    runpy.run_path(addon, run_name='__main__')
  File "<frozen runpy>", line 291, in run_path
  File "<frozen runpy>", line 98, in _run_module_code
  File "<frozen runpy>", line 88, in _run_code
  File "/tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/addon1.py", line 2, in <module>
    raise Exception()
Exceptio [internalError]
    """
    # /tmp/pytest-of-user/pytest-10/test_invalid_addon_py_20/file.cpp:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 256.: python3 /home/user/CLionProjects/cppcheck/addons/runaddon.py /tmp/pytest-of-user/pytest-10/test_invalid_addon_py_20/addon1.py --cli /tmp/pytest-of-user/pytest-10/test_invalid_addon_py_20/file.cpp.24637.dump
    assert stderr.startswith("{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 1: ".format(test_file))
    assert stderr.count('Output:\nTraceback')
    assert stderr.endswith('raise Exception()\nException [internalError]\n\n^\n')


def test_addon_result(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.py')
    with open(addon_file, 'wt') as f:
        f.write("""
print("Checking ...")
print("")
print('{"file": "test.cpp", "linenr": 1, "column": 1, "severity": "style", "message": "msg", "addon": "addon1", "errorId": "id", "extra": ""}')
print('{"loc": [{"file": "test.cpp", "linenr": 1, "column": 1, "info": ""}], "severity": "style", "message": "msg", "addon": "addon1", "errorId": "id", "extra": ""}')
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
                """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == 'test.cpp:1:1: style: msg [addon1-id]\n\n^\n'


# TODO: test with -j2
# #11483
def __test_unused_function_include(tmpdir, extra_args):
    test_cpp_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_cpp_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    test_h_file = os.path.join(tmpdir, 'test.h')
    with open(test_h_file, 'wt') as f:
        f.write("""
                class A {
                public:
                    void f() {}
                    // cppcheck-suppress unusedFunction
                    void f2() {}
                };
                """)

    args = [
        '--enable=unusedFunction',
        '--inline-suppr',
        '--template=simple',
        '-j1',
        test_cpp_file
    ]

    args += extra_args

    _, _, stderr = cppcheck(args)
    assert stderr == "{}:4:26: style: The function 'f' is never used. [unusedFunction]\n".format(test_h_file)


def test_unused_function_include(tmpdir):
    __test_unused_function_include(tmpdir, [])


# TODO: test with clang-tidy
# TODO: test with --addon
# TODO: test with FileSettings
# TODO: test with multiple files
def __test_showtime(tmp_path, showtime, exp_res, exp_last, extra_args=None):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write(
"""
void f()
{
    (void)(*((int*)0)); // cppcheck-suppress nullPointer
}
""")

    args = [
        f'--showtime={showtime}',
        '--quiet',
        '--inline-suppr',
        str(test_file)
    ]

    if extra_args:
        args += extra_args

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    exp_len = exp_res
    if exp_res:
        exp_len += 1  # empty line at the beginning - only added when individual results exist
    if 'cppcheck internal API usage' in stdout:
        exp_len += 1
    exp_len += 1  # last line
    assert len(lines) == exp_len
    if exp_res:
        assert lines[0] == ''
    for i in range(1, exp_res):
        assert 'avg.' in lines[i]
    assert lines[exp_len-1].startswith(exp_last)
    assert stderr == ''


def test_showtime_top5_file(tmp_path):
    __test_showtime(tmp_path, 'top5_file', 5, 'Check time: ')


# TODO: remove extra args when --executor=process works works
def test_showtime_top5_summary(tmp_path):
    __test_showtime(tmp_path, 'top5_summary', 5, 'Overall time: ', ['-j1'])


# TODO: remove when --executor=process works works
def test_showtime_top5_summary_j_thread(tmp_path):
    __test_showtime(tmp_path, 'top5_summary', 5, 'Overall time: ', ['-j2', '--executor=thread'])


# TODO: remove override when fixed
@pytest.mark.skipif(sys.platform == 'win32', reason="requires ProcessExecutor")
@pytest.mark.xfail(strict=True)  # TODO: need to transfer the timer results to parent process - see #4452
def test_showtime_top5_summary_j_process(tmp_path):
    __test_showtime(tmp_path, 'top5_summary', 5, 'Overall time: ', ['-j2', '--executor=process'])


def test_showtime_file(tmp_path):
    __test_showtime(tmp_path, 'file', 79, 'Check time: ')


# TODO: remove extra args when --executor=process works works
def test_showtime_summary(tmp_path):
    __test_showtime(tmp_path, 'summary', 79, 'Overall time: ', ['-j1'])


# TODO: remove when --executor=process works works
def test_showtime_summary_j_thread(tmp_path):
    __test_showtime(tmp_path, 'summary', 79, 'Overall time: ', ['-j2', '--executor=thread'])


# TODO: remove override when fixed
@pytest.mark.skipif(sys.platform == 'win32', reason="requires ProcessExecutor")
@pytest.mark.xfail(strict=True)  # TODO: need to transfer the timer results to parent process - see #4452
def test_showtime_summary_j_process(tmp_path):
    __test_showtime(tmp_path, 'summary', 79, 'Overall time: ', ['-j2', '--executor=process'])


def test_showtime_file_total(tmp_path):
    __test_showtime(tmp_path, 'file-total', 0, 'Check time: ')


def test_showtime_unique(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write(
"""
void f()
{
    (void)(*((int*)0)); // cppcheck-suppress nullPointer
}
""")

    args = [
        '--showtime=summary',
        '--quiet',
        '--inline-suppr',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    multi_res = []
    for line in stdout.splitlines():
        # TODO: remove when we no longer emit empty line
        if not line:
            continue
        if any(i in line for i in ['1 result(s)', 'Overall time:']):
            continue
        multi_res.append(line)
    assert multi_res == []
    assert stderr == ''


def test_missing_addon(tmpdir):
    args = ['--addon=misra3', '--addon=misra', '--addon=misra2', 'file.c']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    lines = stdout.splitlines()
    lines.sort()
    assert lines == [
        'Did not find addon misra2.py',
        'Did not find addon misra3.py'
    ]
    assert stderr == ""


def test_file_filter(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--file-filter=*.cpp', test_file]
    out_lines = [
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_file_filter_2(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt'):
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt'):
        pass

    args = ['--file-filter=*.cpp', test_file_1, test_file_2]
    out_lines = [
        'Checking {} ...'.format(test_file_1)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_file_filter_3(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt'):
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt'):
        pass

    args = ['--file-filter=*.c', test_file_1, test_file_2]
    out_lines = [
        'Checking {} ...'.format(test_file_2)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_file_filter_no_match(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--file-filter=*.c', test_file]
    out_lines = [
        'cppcheck: error: could not find any files matching the filter:*.c'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_file_order(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass
    test_file_d = os.path.join(tmpdir, 'd.c')
    with open(test_file_d, 'wt'):
        pass

    args = [test_file_c, test_file_d, test_file_b, test_file_a, '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/4 files checked 25% done',
        'Checking {} ...'.format(test_file_d),
        '2/4 files checked 50% done',
        'Checking {} ...'.format(test_file_b),
        '3/4 files checked 75% done',
        'Checking {} ...'.format(test_file_a),
        '4/4 files checked 100% done'
    ]
    assert stderr == ''


def test_markup(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test_1.qml')
    with open(test_file_1, 'wt'):
        pass
    test_file_2 = os.path.join(tmpdir, 'test_2.cpp')
    with open(test_file_2, 'wt'):
        pass
    test_file_3 = os.path.join(tmpdir, 'test_3.qml')
    with open(test_file_3, 'wt'):
        pass
    test_file_4 = os.path.join(tmpdir, 'test_4.cpp')
    with open(test_file_4, 'wt'):
        pass

    args = ['--library=qt', test_file_1, test_file_2, test_file_3, test_file_4, '-j1']
    out_lines = [
        'Checking {} ...'.format(test_file_2),
        '1/4 files checked 25% done',
        'Checking {} ...'.format(test_file_4),
        '2/4 files checked 50% done',
        'Checking {} ...'.format(test_file_1),
        '3/4 files checked 75% done',
        'Checking {} ...'.format(test_file_3),
        '4/4 files checked 100% done'
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_markup_j(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test_1.qml')
    with open(test_file_1, 'wt'):
        pass
    test_file_2 = os.path.join(tmpdir, 'test_2.cpp')
    with open(test_file_2, 'wt'):
        pass
    test_file_3 = os.path.join(tmpdir, 'test_3.qml')
    with open(test_file_3, 'wt'):
        pass
    test_file_4 = os.path.join(tmpdir, 'test_4.cpp')
    with open(test_file_4, 'wt'):
        pass

    args = ['--library=qt', '-j2', test_file_1, test_file_2, test_file_3, test_file_4]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()

    assert sorted(lines) == [
        '1/4 files checked 25% done',
        '2/4 files checked 50% done',
        '3/4 files checked 75% done',
        '4/4 files checked 100% done',
        'Checking {} ...'.format(test_file_1),
        'Checking {} ...'.format(test_file_2),
        'Checking {} ...'.format(test_file_3),
        'Checking {} ...'.format(test_file_4)
    ]
    assert stderr == ''


def test_valueflow_debug(tmpdir):
    test_file_cpp = os.path.join(tmpdir, 'test_1.cpp')
    with open(test_file_cpp, 'wt') as f:
        f.write("""
#include "test.h"

void f()
{
    int i = 0;
}
"""
                )
    test_file_h = os.path.join(tmpdir, 'test.h')
    with open(test_file_h, 'wt') as f:
        f.write("""
#include "test2.h"
inline void f1()
{
    int i = 0;
}
"""
                )
    test_file_h_2 = os.path.join(tmpdir, 'test2.h')
    with open(test_file_h_2, 'wt') as f:
        f.write("""
inline void f2()
{
    int i = 0;
}
"""
                )

    args = ['--debug', test_file_cpp]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    if sys.platform == "win32":
        stdout = stdout.replace('/', '\\')
    assert f'Checking {test_file_cpp} ...' in stdout
    stdout = stdout.replace(f'Checking {test_file_cpp} ...\n', '').strip()
    assert stdout == '''##file {}
2: void f2 ( )
3: {{
4: int i@var1 ; i@var1 = 0 ;
5: }}

##file {}

1:
2:
3: void f1 ( )
4: {{
5: int i@var2 ; i@var2 = 0 ;
6: }}

##file {}

1:
2:
3:
4: void f ( )
5: {{
6: int i@var3 ; i@var3 = 0 ;
7: }}



##Value flow
File {}
Line 4
  = always 0
  0 always 0
File {}
Line 5
  = always 0
  0 always 0
File {}
Line 6
  = always 0
  0 always 0'''.format(test_file_h_2, test_file_h, test_file_cpp,
                       test_file_h_2, test_file_h, test_file_cpp)
    assert stderr == ''


def test_file_duplicate(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    args = [test_file_a, test_file_a, str(tmpdir)]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_a)
    ]
    assert stderr == ''


def test_file_duplicate_2(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass

    args = [test_file_c, test_file_a, test_file_b, str(tmpdir), test_file_b, test_file_c, test_file_a, str(tmpdir), '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/3 files checked 33% done',
        'Checking {} ...'.format(test_file_a),
        '2/3 files checked 66% done',
        'Checking {} ...'.format(test_file_b),
        '3/3 files checked 100% done'
    ]
    assert stderr == ''


def test_file_duplicate_3(tmpdir):  # #12834
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    # multiple ways to specify the same file
    in_file_a = 'a.c'
    in_file_b = os.path.join('.', 'a.c')
    in_file_c = os.path.join('dummy', '..', 'a.c')
    in_file_d = os.path.join(tmpdir, 'a.c')
    in_file_e = os.path.join(tmpdir, '.', 'a.c')
    in_file_f = os.path.join(tmpdir, 'dummy', '..', 'a.c')

    args = [in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, str(tmpdir)]

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format('a.c')
    ]
    assert stderr == ''


@pytest.mark.skipif(sys.platform != 'win32', reason="requires Windows")
def test_file_duplicate_4(tmpdir):  # #12834
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    # multiple ways to specify the same file
    in_file_a = 'a.c'
    in_file_b = os.path.join('.', 'a.c')
    in_file_c = os.path.join('dummy', '..', 'a.c')
    in_file_d = os.path.join(tmpdir, 'a.c')
    in_file_e = os.path.join(tmpdir, '.', 'a.c')
    in_file_f = os.path.join(tmpdir, 'dummy', '..', 'a.c')

    args1 = [in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, str(tmpdir)]
    args2 = []
    for a in args1:
        args2.append(a.replace('\\', '/'))
    args = args1 + args2

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format('a.c')
    ]
    assert stderr == ''


def test_file_ignore(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['-itest.cpp', test_file]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_build_dir_j_memleak(tmpdir): #12111
    build_dir = os.path.join(tmpdir, 'build-dir')
    os.mkdir(build_dir)

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write('int main() {}')

    args = ['--cppcheck-build-dir={}'.format(build_dir), '-j2', test_file]
    out_lines = [
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def __test_addon_json_invalid(tmpdir, addon_json, expected):
    addon_file = os.path.join(tmpdir, 'invalid.json')
    with open(addon_file, 'wt') as f:
        f.write(addon_json)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--addon={}'.format(addon_file), test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    lines = stdout.splitlines()
    assert len(lines) == 1
    assert lines == [
        'Loading {} failed. {}'.format(addon_file, expected)
    ]
    assert stderr == ''


def test_addon_json_invalid_no_obj(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps([]), "JSON is not an object.")


def test_addon_json_invalid_args_1(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'args':0}), "'args' must be an array.")


def test_addon_json_invalid_args_2(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'args':[0]}), "'args' entry is not a string.")


def test_addon_json_invalid_ctu(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'ctu':0}), "'ctu' must be a boolean.")


def test_addon_json_invalid_python(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'python':0}), "'python' must be a string.")


def test_addon_json_invalid_executable(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'executable':0}), "'executable' must be a string.")


def test_addon_json_invalid_script_1(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'Script':''}), "'script' is missing.")


def test_addon_json_invalid_script_2(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'script':0}), "'script' must be a string.")


def test_unknown_extension(tmpdir):
    test_file = os.path.join(tmpdir, 'test_2')
    with open(test_file, 'wt') as f:
        f.write('''
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['-q', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    assert stdout == ''
    assert stderr == ''


def test_rule_file_define_multiple(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rules>
    <rule>
        <tokenlist>define</tokenlist>
        <pattern>DEF_1</pattern>
        <message>
            <severity>error</severity>
            <id>ruleId1</id>
        </message>
    </rule>
    <rule>
        <tokenlist>define</tokenlist>
        <pattern>DEF_2</pattern>
        <message>
            <severity>error</severity>
            <id>ruleId2</id>
            <summary>define2</summary>
        </message>
    </rule>
</rules>""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), '-DDEF_3', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: DEF_1',
        'Processing rule: DEF_2',
        'Checking {}: DEF_3=1...'.format(test_file)
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:2:0: error: found 'DEF_1' [ruleId1]".format(test_file),
        "{}:3:0: error: define2 [ruleId2]".format(test_file)
    ]


def test_rule_file_define(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rule>
    <tokenlist>define</tokenlist>
    <pattern>DEF_.</pattern>
</rule>
""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), '-DDEF_3', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: DEF_.',
        'Checking {}: DEF_3=1...'.format(test_file)
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:2:0: style: found 'DEF_1' [rule]".format(test_file),
        "{}:3:0: style: found 'DEF_2' [rule]".format(test_file)
    ]


def test_rule_file_normal(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rule>
    <pattern>int</pattern>
</rule>
""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
typedef int i32;
void f(i32) { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: int',
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:5:0: style: found 'int' [rule]".format(test_file)
    ]


def test_rule_file_raw(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rule>
    <tokenlist>raw</tokenlist>
    <pattern>i32</pattern>
</rule>
""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
typedef int i32;
void f(i32) { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: i32',
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:4:0: style: found 'i32' [rule]".format(test_file),
        "{}:5:0: style: found 'i32' [rule]".format(test_file)
    ]


def test_rule(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule=f', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: f',
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:4:0: style: found 'f' [rule]".format(test_file)
    ]


def test_rule_multiple_files(tmpdir):
    stderr_exp = []
    for i in range(10):
        test_file = os.path.join(tmpdir, f'test_{i}.c')
        stderr_exp.append("{}:4:0: style: found 'f' [rule]".format(test_file))
        with open(test_file, 'wt') as f:
            f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['-q', '--template=simple', '--rule=f', str(tmpdir)])
    assert exitcode == 0, stdout if stdout else stderr
    assert stdout.splitlines() == []
    lines = stderr.splitlines()
    lines.sort()
    stderr_exp.sort()
    assert lines == stderr_exp


def test_filelist(tmpdir):
    list_dir = os.path.join(tmpdir, 'list-dir')
    os.mkdir(list_dir)

    with open(os.path.join(list_dir, 'aaa.c'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'zzz.c'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'valueflow.cpp'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'vfvalue.cpp'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'vf_enumvalue.cpp'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'vf_analyze.h'), 'wt'):
        pass

    sub_dir_1 = os.path.join(list_dir, 'valueflow')
    os.mkdir(sub_dir_1)
    with open(os.path.join(sub_dir_1, 'file.cpp'), 'wt'):
        pass
    with open(os.path.join(sub_dir_1, 'file.c'), 'wt'):
        pass
    with open(os.path.join(sub_dir_1, 'file.h'), 'wt'):
        pass

    sub_dir_2 = os.path.join(list_dir, 'vfvalue')
    os.mkdir(sub_dir_2)
    with open(os.path.join(sub_dir_2, 'file.cpp'), 'wt'):
        pass
    with open(os.path.join(sub_dir_2, 'file.c'), 'wt'):
        pass
    with open(os.path.join(sub_dir_2, 'file.h'), 'wt'):
        pass

    sub_dir_3 = os.path.join(list_dir, 'vf_enumvalue')
    os.mkdir(sub_dir_3)
    with open(os.path.join(sub_dir_3, 'file.cpp'), 'wt'):
        pass
    with open(os.path.join(sub_dir_3, 'file.c'), 'wt'):
        pass
    with open(os.path.join(sub_dir_3, 'file.h'), 'wt'):
        pass

    # TODO: -rp is not applied to "Checking" messages
    #exitcode, stdout, stderr = cppcheck(['-j1', '-rp', list_dir])
    exitcode, stdout, stderr = cppcheck(['-j1', '.'], cwd=list_dir)
    assert exitcode == 0, stdout if stdout else stderr
    if sys.platform == "win32":
        stdout = stdout.replace('\\', '/')
    lines = stdout.splitlines()
    expected = [
        'Checking aaa.c ...',
        'Checking valueflow.cpp ...',
        'Checking valueflow/file.c ...',
        'Checking valueflow/file.cpp ...',
        'Checking vf_enumvalue.cpp ...',
        'Checking vf_enumvalue/file.c ...',
        'Checking vf_enumvalue/file.cpp ...',
        'Checking vfvalue.cpp ...',
        'Checking vfvalue/file.c ...',
        'Checking vfvalue/file.cpp ...',
        'Checking zzz.c ...'
    ]
    assert len(expected), len(lines)
    for i in range(1, len(expected)+1):
        lines.remove('{}/{} files checked {}% done'.format(i, len(expected), int(100 * i // len(expected))))
    assert lines == expected


def test_markup_lang(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test_1.qml')
    with open(test_file_1, 'wt'):
        pass
    test_file_2 = os.path.join(tmpdir, 'test_2.cpp')
    with open(test_file_2, 'wt'):
        pass

    # do not assert processing markup file with enforced language
    args = [
        '--library=qt',
        '--enable=unusedFunction',
        '--language=c++',
        '-j1',
        test_file_1,
        test_file_2
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr


def test_cpp_probe(tmpdir):
    test_file = os.path.join(tmpdir, 'test.h')
    with open(test_file, 'wt') as f:
        f.writelines([
            'class A {};'
        ])

    args = ['-q', '--template=simple', '--cpp-header-probe', '--verbose', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == []
    lines = stderr.splitlines()
    assert lines == [
        # TODO: fix that awkward format
        "{}:1:1: error: Code 'classA{{' is invalid C code.: Use --std, -x or --language to enforce C++. Or --cpp-header-probe to identify C++ headers via the Emacs marker. [syntaxError]".format(test_file)
    ]


def test_cpp_probe_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.h')
    with open(test_file, 'wt') as f:
        f.writelines([
            '// -*- C++ -*-',
            'class A {};'
        ])

    args = ['-q', '--template=simple', '--cpp-header-probe', test_file]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=[])


def test_config_invalid(tmpdir):
    # cppcheck.cfg needs to be next to executable
    exe = shutil.copy2(__lookup_cppcheck_exe(), tmpdir)
    if sys.platform == 'win32':
        shutil.copy2(os.path.join(os.path.dirname(__lookup_cppcheck_exe()), 'cppcheck-core.dll'), tmpdir)
    shutil.copytree(os.path.join(os.path.dirname(__lookup_cppcheck_exe()), 'cfg'), os.path.join(tmpdir, 'cfg'))

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex([test_file], cwd=tmpdir, cppcheck_exe=exe)
    assert exitcode == 1, stdout if stdout else stderr
    assert stdout.splitlines() == [
        'cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 1 near: '
    ]


def test_config_override(tmpdir):
    # cppcheck.cfg needs to be next to executable
    exe = shutil.copy2(__lookup_cppcheck_exe(), tmpdir)
    if sys.platform == 'win32':
        shutil.copy2(os.path.join(os.path.dirname(__lookup_cppcheck_exe()), 'cppcheck-core.dll'), tmpdir)
    shutil.copytree(os.path.join(os.path.dirname(__lookup_cppcheck_exe()), 'cfg'), os.path.join(tmpdir, 'cfg'))

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'wt') as f:
        f.write(json.dumps({
            'safety': False
        }))

    exitcode, stdout, stderr, exe = cppcheck_ex(['-q', '--safety', test_file], cwd=tmpdir, cppcheck_exe=exe, remove_checkers_report=False)
    assert exitcode == 0, stdout if stdout else stderr
    assert stdout.splitlines() == []
    assert 'checkersReport' in stderr


def test_checkers_report(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('x=1;')
    checkers_report = os.path.join(tmpdir, 'r.txt')
    exitcode, stdout, stderr = cppcheck(['--enable=all', '--checkers-report=' + checkers_report, test_file], remove_checkers_report=False)
    assert exitcode == 0, stdout if stdout else stderr
    assert 'Active checkers:' in stderr
    assert '--checkers-report' not in stderr


def test_checkers_report_misra_json(tmpdir):
    """check that misra checkers are reported properly when --addon=misra.json is used"""
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('x=1;')
    misra_json = os.path.join(tmpdir, 'misra.json')
    with open(misra_json, 'wt') as f:
        f.write('{"script":"misra.py"}')
    exitcode, stdout, stderr = cppcheck('--enable=style --addon=misra.json --xml-version=3 test.c'.split(), cwd=tmpdir)
    assert exitcode == 0, stdout if stdout else stderr
    assert '<checker id="Misra C 2012: 8.1"/>' in stderr


def __test_ignore_file(tmpdir, ign, append=False, inject_path=False):
    os.mkdir(os.path.join(tmpdir, 'src'))
    test_file = os.path.join(tmpdir, 'src', 'test.cpp')
    with open(test_file, 'wt'):
        pass

    # TODO: this should say that all paths are ignored
    lines_exp = [
        'ignored path: {}'.format(test_file),
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    args = [
        '--debug-ignore',
        test_file
    ]

    if inject_path:
        ign = ign.replace('$path', str(test_file))

    if append:
        args += ['-i{}'.format(ign)]
    else:
        args = ['-i{}'.format(ign)] + args

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 1, stdout if stdout else stderr
    assert stdout.splitlines() == lines_exp


def test_ignore_file(tmpdir):
    __test_ignore_file(tmpdir, 'test.cpp')


def test_ignore_file_append(tmpdir):
    __test_ignore_file(tmpdir, 'test.cpp', append=True)


def test_ignore_file_wildcard_back(tmpdir):
    __test_ignore_file(tmpdir, 'test.c*')


def test_ignore_file_wildcard_front(tmpdir):
    __test_ignore_file(tmpdir, '*test.cpp')


def test_ignore_file_placeholder(tmpdir):
    __test_ignore_file(tmpdir, 't?st.cpp')


def test_ignore_file_relative(tmpdir):
    __test_ignore_file(tmpdir, 'src/test.cpp')


def test_ignore_file_relative_backslash(tmpdir):
    __test_ignore_file(tmpdir, 'src\\test.cpp')


def test_ignore_file_relative_wildcard(tmpdir):
    __test_ignore_file(tmpdir, 'src/test.c*')


def test_ignore_file_relative_wildcard_backslash(tmpdir):
    __test_ignore_file(tmpdir, 'src\\test.c*')


def test_ignore_path_relative(tmpdir):
    __test_ignore_file(tmpdir, 'src/')


def test_ignore_path_relative_backslash(tmpdir):
    __test_ignore_file(tmpdir, 'src\\')


def test_ignore_path_relative_wildcard(tmpdir):
    __test_ignore_file(tmpdir, 'src*/')


def test_ignore_path_relative_wildcard_backslash(tmpdir):
    __test_ignore_file(tmpdir, 'src*\\')


def test_ignore_abspath(tmpdir):
    __test_ignore_file(tmpdir, '$path', inject_path=True)


def __write_gui_project(tmpdir, test_file, ignore):
    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
        """<?xml version="1.0" encoding="UTF-8"?>
<project>
<paths>
<dir name="{}"/>
</paths>
<ignore>
<path name="{}"/>
</ignore>
</project>""".format(test_file, ignore))

    return project_file


def __test_ignore_project(tmpdir, ign_proj, ign_cli=None, append_cli=False, inject_path_proj=False):
    os.mkdir(os.path.join(tmpdir, 'src'))
    test_file = os.path.join(tmpdir, 'src', 'test.cpp')
    with open(test_file, 'wt'):
        pass

    # TODO: this should say that all paths were ignored
    lines_exp = [
        'ignored path: {}'.format(test_file),
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    if inject_path_proj:
        ign_proj = ign_proj.replace('$path', str(test_file))

    project_file = __write_gui_project(tmpdir, test_file, ign_proj)
    args = [
        '--debug-ignore',
        '--project={}'.format(project_file)
    ]

    if append_cli:
        args += ['-i{}'.format(ign_cli)]
    else:
        args = ['-i{}'.format(ign_cli)] + args

    exitcode, stdout, _ = cppcheck(args, cwd=tmpdir)
    assert exitcode == 1, stdout
    assert stdout.splitlines() == lines_exp


def test_ignore_project_file(tmpdir):
    __test_ignore_project(tmpdir, 'test.cpp')


def test_ignore_project_file_cli_prepend(tmpdir):
    __test_ignore_project(tmpdir, ign_proj='test2.cpp', ign_cli='test.cpp')


def test_ignore_project_file_cli_append(tmpdir):
    __test_ignore_project(tmpdir, ign_proj='test2.cpp', ign_cli='test.cpp', append_cli=True)


def test_ignore_project_file_wildcard_back(tmpdir):
    __test_ignore_project(tmpdir, 'test.c*')


def test_ignore_project_file_wildcard_front(tmpdir):
    __test_ignore_project(tmpdir, '*test.cpp')


def test_ignore_project_file_placeholder(tmpdir):
    __test_ignore_project(tmpdir, 't?st.cpp')


def test_ignore_project_file_relative(tmpdir):
    __test_ignore_project(tmpdir, 'src/test.cpp')


def test_ignore_project_file_relative_backslash(tmpdir):
    __test_ignore_project(tmpdir, 'src\\test.cpp')


def test_ignore_project_path_relative(tmpdir):
    __test_ignore_project(tmpdir, 'src/')


def test_ignore_project_path_relative_backslash(tmpdir):
    __test_ignore_project(tmpdir, 'src\\')


def test_ignore_project_abspath(tmpdir):
    __test_ignore_project(tmpdir, '$path', inject_path_proj=True)


def __write_compdb(tmpdir, test_file):
    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    j = [
        {
            'directory': os.path.dirname(test_file),
            'file': test_file,
            'command': 'gcc -c {}'.format(test_file)
        }
    ]
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j))
    return compile_commands


def __test_ignore_project_2(tmpdir, extra_args, append=False, inject_path=False):
    os.mkdir(os.path.join(tmpdir, 'src'))
    test_file = os.path.join(tmpdir, 'src', 'test.cpp')
    with open(test_file, 'wt'):
        pass

    lines_exp = [
        'ignored path: {}'.format(str(test_file).replace('\\', '/')),
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]
    project_file = __write_compdb(tmpdir, test_file)
    args = [
        '--debug-ignore',
        '--project={}'.format(project_file)
    ]

    if inject_path:
        extra_args = [ extra_args[0].replace('$path', str(test_file)) ]
    if append:
        args += extra_args
    else:
        args = extra_args + args
    print(args)

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 1, stdout if stdout else stderr
    assert stdout.splitlines() == lines_exp


def test_ignore_project_2_file(tmpdir):
    __test_ignore_project_2(tmpdir, ['-itest.cpp'])


def test_ignore_project_2_file_append(tmpdir):
    # make sure it also matches when specified after project
    __test_ignore_project_2(tmpdir, ['-itest.cpp'], append=True)


def test_ignore_project_2_file_wildcard_back(tmpdir):
    __test_ignore_project_2(tmpdir, ['-itest.c*'])


def test_ignore_project_2_file_wildcard_front(tmpdir):
    __test_ignore_project_2(tmpdir, ['-i*test.cpp'])


def test_ignore_project_2_file_placeholder(tmpdir):
    __test_ignore_project_2(tmpdir, ['-it?st.cpp'])


def test_ignore_project_2_file_relative(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc/test.cpp'])


def test_ignore_project_2_file_relative_backslash(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc\\test.cpp'])


def test_ignore_project_2_file_relative_wildcard(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc/test.c*'])


def test_ignore_project_2_file_relative_wildcard_backslash(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc\\test.c*'])


def test_ignore_project_2_path_relative(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc/'])


def test_ignore_project_2_path_relative_backslash(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc\\'])


def test_ignore_project_2_path_relative_wildcard(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc*/'])


def test_ignore_project_2_path_relative_wildcard_backslash(tmpdir):
    __test_ignore_project_2(tmpdir, ['-isrc*\\'])


def test_ignore_project_2_abspath(tmpdir):
    __test_ignore_project_2(tmpdir, ['-i$path'], inject_path=True)


def test_dumpfile_platform(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('x=1;\n')
    cppcheck('--dump --platform=unix64 test.c'.split(), cwd=tmpdir)
    platform = ''
    with open(test_file + '.dump', 'rt') as f:
        for line in f:
            if line.find('<platform name="') > 0:
                platform = line.strip()
                break
    assert ' wchar_t_bit="' in platform
    assert ' size_t_bit="' in platform


def test_builddir_hash_check_level(tmp_path):  # #13376
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f(bool b)
{
    for (int i = 0; i < 2; ++i)
    {
        if (i == 0) {}
        if (b) continue;
    }
}
""")

    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)

    args = [ # // #14029
        '--enable=warning',  # to execute the code which generates the normalCheckLevelMaxBranches message
        '--cppcheck-build-dir={}'.format(build_dir),
        '--template=simple',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stderr == ''

    args = [
        '--enable=warning',  # to execute the code which generates the normalCheckLevelMaxBranches message
        '--enable=information',  # to show the normalCheckLevelMaxBranches message
        '--cppcheck-build-dir={}'.format(build_dir),
        '--template=simple',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stderr == '{}:0:0: information: Limiting analysis of branches. Use --check-level=exhaustive to analyze all branches. [normalCheckLevelMaxBranches]\n'.format(test_file)

    cache_file = (build_dir / 'test.a1')

    root = ElementTree.fromstring(cache_file.read_text())
    hash_1 = root.get('hash')

    args += ['--check-level=exhaustive']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stderr == ''

    root = ElementTree.fromstring(cache_file.read_text())
    hash_2 = root.get('hash')

    assert hash_1 != hash_2


def test_def_undef(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
#ifndef DEF_1
    (void)(*((int*)0));
#endif
}
""")

    args = [
        '--template=simple',
        '-DDEF_1',
        '-UDEF_1',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Checking {}: DEF_1=1...'.format(test_file)  # TODO: should not print DEF_1 - see #13335
    ]
    assert stderr.splitlines() == [
        '{}:5:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]


@pytest.mark.xfail(strict=True)
def test_def_def(tmp_path):  # #13334
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
#if DEF_1 == 3
    (void)(*((int*)0));
#endif
#if DEF_1 == 7
    (void)(*((int*)0));
#endif
}
""")

    args = [
        '--template=simple',
        '-DDEF_1=3',
        '-DDEF_1=7',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Checking {}: DEF_1=3;DEF_1=7...'.format(test_file)  # TODO: should not print DEF_1 twice - see #13335
    ]
    assert stderr.splitlines() == [
        '{}:8:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]


@pytest.mark.xfail(strict=True)
def test_def_undef_def(tmp_path):  # #13334
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
#ifdef DEF_1
    (void)(*((int*)0));
#endif
}
""")

    args = [
        '--template=simple',
        '-DDEF_1',
        '-UDEF_1',
        '-DDEF_1',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Checking {}: DEF_1=1;DEF_1=1...'.format(test_file)  # TODO: should not print DEF_1 twice - see #13335
    ]
    assert stderr.splitlines() == [
        '{}:5:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]


def test_undef(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
#ifndef DEF_1
    (void)(*((int*)0));
#endif
}
""")

    args = [
        '--template=simple',
        '-UDEF_1',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr.splitlines() == [
        '{}:5:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]


@pytest.mark.xfail(strict=True)
def test_undef_src(tmp_path):  # #13340
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
#define DEF_1

void f()
{
#ifdef DEF_1
    (void)(*((int*)0));
#endif
}
""")

    args = [
        '--template=simple',
        '-UDEF_1',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr.splitlines() == [
        '{}:7:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]


def test_dump_check_config(tmp_path):  # #13432
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f() {}
""")

    args = [
        '-q',
        '--template=simple',
        '--dump',
        '--check-config',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout == ''
    assert stderr == ''

    # no dump file should have been generated
    assert not os.path.exists(str(test_file) + '.dump')


# TODO: remove all overrides when fully fixed
def __test_inline_suppr(tmp_path, extra_args):  # #13087
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f() {
  // cppcheck-suppress memleak
}
""")

    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--inline-suppr',
        str(test_file)
    ]

    args += extra_args

    exitcode, stdout, stderr, = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout == ''
    assert stderr.splitlines() == [
        '{}:4:0: information: Unmatched suppression: memleak [unmatchedSuppression]'.format(test_file)
    ]


def test_inline_suppr(tmp_path):
    __test_inline_suppr(tmp_path, ['-j1'])


def test_inline_suppr_j(tmp_path):
    __test_inline_suppr(tmp_path, ['-j2'])


def test_inline_suppr_builddir(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_inline_suppr(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j1'])


def test_inline_suppr_builddir_cached(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_inline_suppr(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j1'])
    __test_inline_suppr(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j1'])


def test_inline_suppr_builddir_j(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_inline_suppr(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j2'])


def test_inline_suppr_builddir_j_cached(tmp_path):
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)
    __test_inline_suppr(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j2'])
    __test_inline_suppr(tmp_path, ['--cppcheck-build-dir={}'.format(build_dir), '-j2'])


def test_duplicate_suppression(tmp_path):
    test_file = tmp_path / 'file.cpp'
    with open(test_file, 'wt'):
        pass

    args = [
        '-q',
        '--suppress=uninitvar',
        '--suppress=uninitvar',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1, stdout
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'uninitvar' already exists"
    ]
    assert stderr == ''


def test_duplicate_suppressions_list(tmp_path):
    suppr_file = tmp_path / 'suppressions'
    with open(suppr_file, 'wt') as f:
        f.write('''
uninitvar
uninitvar
''')

    test_file = tmp_path / 'file.cpp'
    with open(test_file, 'wt'):
        pass

    args = [
        '-q',
        '--suppressions-list={}'.format(suppr_file),
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1, stdout
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'uninitvar' already exists"
    ]
    assert stderr == ''


def test_duplicate_suppressions_mixed(tmp_path):
    suppr_file = tmp_path / 'suppressions'
    with open(suppr_file, 'wt') as f:
        f.write('uninitvar')

    test_file = tmp_path / 'file.cpp'
    with open(test_file, 'wt'):
        pass

    args = [
        '-q',
        '--suppress=uninitvar',
        '--suppressions-list={}'.format(suppr_file),
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1, stdout
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'uninitvar' already exists"
    ]
    assert stderr == ''


def test_xml_output(tmp_path):  # #13391 / #13485
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f(const void* p)
{
    if(p) {}
    (void)*p; // REMARK: boom
}
""")

    _, version_str, _ = cppcheck(['--version'])
    version_str = version_str.replace('Cppcheck ', '').strip()

    args = [
        '-q',
        '--enable=style',
        '--xml',
        str(test_file)
    ]
    exitcode_1, stdout_1, stderr_1 = cppcheck(args)
    assert exitcode_1 == 0, stdout_1
    assert stdout_1 == ''
    test_file_exp = str(test_file).replace('\\', '/')
    assert (stderr_1 ==
'''<?xml version="1.0" encoding="UTF-8"?>
<results version="2">
    <cppcheck version="{}"/>
    <errors>
        <error id="nullPointerRedundantCheck" severity="warning" msg="Either the condition &apos;p&apos; is redundant or there is possible null pointer dereference: p." verbose="Either the condition &apos;p&apos; is redundant or there is possible null pointer dereference: p." cwe="476" file0="{}" remark="boom">
            <location file="{}" line="5" column="12" info="Null pointer dereference"/>
            <location file="{}" line="4" column="8" info="Assuming that condition &apos;p&apos; is not redundant"/>
            <symbol>p</symbol>
        </error>
    </errors>
</results>
'''.format(version_str, test_file_exp, test_file_exp, test_file_exp))


def test_outputfile(tmp_path): # #14051
    test_file = tmp_path / 'test.cpp'
    out_file = tmp_path / 'out.txt'
    with open(test_file, 'wt') as f:
        f.write(
"""
int main()
{
    int x = 1 / 0;
}
""")

    args = [
        '-q',
        '--output-file={}'.format(out_file),
        str(test_file)
    ]

    out_exp = [
        '{}:4:15: error: Division by zero. [zerodiv]'.format(test_file),
        '    int x = 1 / 0;',
        '              ^',
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout == ''
    assert stderr == ''

    with open(out_file, 'rt') as f:
        out_text = f.read()

    assert out_text.splitlines() == out_exp


def test_internal_error_loc_int(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""
void f() {
    int i = 0x10000000000000000;
}
""")

    args = [
        '-q',
        '--template=simple',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        '{}:3:13: error: Internal Error. MathLib::toBigUNumber: out_of_range: 0x10000000000000000 [internalError]'.format(test_file)
    ]


def __test_addon_suppr(tmp_path, extra_args):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
// cppcheck-suppress misra-c2012-2.3
typedef int MISRA_5_6_VIOLATION;
typedef int MISRA_5_6_VIOLATION_1;
        """)

    args = [
        '-q',
        '--template=simple',
        '--enable=style',
        '--addon=misra',
        str(test_file)
    ]

    args += extra_args

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout == ''
    assert stderr.splitlines() == [
        '{}:4:1: style: misra violation (use --rule-texts=<file> to get proper output) [misra-c2012-2.3]'.format(test_file),
    ]


# TODO: remove override when all issues are fixed
def test_addon_suppr_inline(tmp_path):
    __test_addon_suppr(tmp_path, ['--inline-suppr', '-j1'])

# TODO: remove override when all issues are fixed
def test_addon_suppr_inline_j(tmp_path):
    __test_addon_suppr(tmp_path, ['--inline-suppr', '-j2'])


def test_addon_suppr_cli_line(tmp_path):
    __test_addon_suppr(tmp_path, ['--suppress=misra-c2012-2.3:*:3'])


def test_addon_suppr_cli_file_line(tmp_path):
    __test_addon_suppr(tmp_path, ['--suppress=misra-c2012-2.3:test.c:3'])


def test_addon_suppr_cli_absfile_line(tmp_path):
    test_file = tmp_path / 'test.c'
    __test_addon_suppr(tmp_path, ['--suppress=misra-c2012-2.3:{}:3'.format(test_file)])


def test_ctu_path_builddir(tmp_path):  # #11883
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)

    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f(int *p) { *p = 3; }
int main() {
    int *p = 0;
f(p);
}
        """)

    args = [
        '-q',
        '--enable=style',
        '--suppress=nullPointer',  # we only care about the CTU findings
        '--cppcheck-build-dir={}'.format(build_dir),
        str(test_file)
    ]

    # the CTU path was not properly read leading to missing location information
    stderr_exp = [
        '{}:2:19: error: Null pointer dereference: p [ctunullpointer]'.format(test_file),
        'void f(int *p) { *p = 3; }',
        '                  ^',
        "{}:4:14: note: Assignment 'p=0', assigned value is 0".format(test_file),
        '    int *p = 0;',
        '             ^',
        '{}:5:2: note: Calling function f, 1st argument is null'.format(test_file),
        'f(p);',
        ' ^',
        '{}:2:19: note: Dereferencing argument p that is null'.format(test_file),
        'void f(int *p) { *p = 3; }',
        '                  ^'
    ]

    exitcode_1, stdout_1, stderr_1 = cppcheck(args)
    assert exitcode_1 == 0, stdout_1
    assert stdout_1 == ''
    assert stderr_1.splitlines() == stderr_exp

    exitcode_2, stdout_2, stderr_2 = cppcheck(args)
    assert exitcode_2 == 0, stdout_2
    assert stdout_2 == ''
    assert stderr_2.splitlines() == stderr_exp


@pytest.mark.xfail(strict=True)
def test_ctu_builddir(tmp_path):  # #11883
    build_dir = tmp_path / 'b1'
    os.mkdir(build_dir)

    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""
void f(int *p) { *p = 3; }
int main() {
    int *p = 0;
f(p);
}
        """)

    args = [
        '-q',
        '--template=simple',
        '--enable=style',
        '--suppress=nullPointer',  # we only care about the CTU findings
        '--cppcheck-build-dir={}'.format(build_dir),
        '-j1',
        '--emit-duplicates',
        str(test_file)
    ]

    # the CTU was run and then evaluated again from the builddir leading to duplicated findings
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout == ''
    assert stderr.splitlines() == [
        '{}:2:19: error: Null pointer dereference: p [ctunullpointer]'.format(test_file)
    ]


def test_debug(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--debug',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') != -1
    assert stdout.find('##Value flow') != -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == []


def test_debug_xml(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--debug',
        '--xml',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout

    assert stderr
    assert ElementTree.fromstring(stderr) is not None

    assert stdout.find('##file ') != -1  # also exists in CDATA
    assert stdout.find('##Value flow') == -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1

    debug_xml = ElementTree.fromstring(stdout)
    assert debug_xml is not None
    assert debug_xml.tag == 'debug'
    file_elem = debug_xml.findall('file')
    assert len(file_elem) == 1
    valueflow_elem = debug_xml.findall('valueflow')
    assert len(valueflow_elem) == 1
    scopes_elem = debug_xml.findall('scopes')
    assert len(scopes_elem) == 1
    ast_elem = debug_xml.findall('ast')
    assert len(ast_elem) == 0


def test_debug_verbose(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--debug',
        '--verbose',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') != -1
    assert stdout.find('##Value flow') != -1
    assert stdout.find('### Symbol database ###') != -1
    assert stdout.find('##AST') != -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == []


def test_debug_verbose_xml(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--debug',
        '--verbose',
        '--xml',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout

    assert stderr
    assert ElementTree.fromstring(stderr) is not None

    assert stdout.find('##file ') != -1  # also exists in CDATA
    assert stdout.find('##Value flow') == -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1

    debug_xml = ElementTree.fromstring(stdout)
    assert debug_xml is not None
    assert debug_xml.tag == 'debug'
    file_elem = debug_xml.findall('file')
    assert len(file_elem) == 1
    valueflow_elem = debug_xml.findall('valueflow')
    assert len(valueflow_elem) == 1
    scopes_elem = debug_xml.findall('scopes')
    assert len(scopes_elem) == 1
    ast_elem = debug_xml.findall('ast')
    assert len(ast_elem) == 1


# TODO: remove interaction with --debug?
# TODO: test with --xml
def __test_debug_template(tmp_path, verbose=False, debug=False):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, "w") as f:
        f.write(
"""template<class T> class TemplCl;
void f()
{
    (void)(*((int*)nullptr));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-template',
        str(test_file)
    ]

    if verbose:
        args += ['--verbose']
    if debug:
        args += ['--debug']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    if debug:
        assert stdout.find('##file ') != -1
    else:
        assert stdout.find('##file ') == -1
    if debug:
        assert stdout.find('##Value flow') != -1
    else:
        assert stdout.find('##Value flow') == -1
    if debug and verbose:
        assert stdout.find('### Symbol database ###') != -1
    else:
        assert stdout.find('### Symbol database ###') == -1
    if debug and verbose:
        assert stdout.find('##AST') != -1
    else:
        assert stdout.find('##AST') == -1
    if debug:
        assert stdout.count('### Template Simplifier pass ') == 2
    else:
        assert stdout.count('### Template Simplifier pass ') == 1
    assert stderr.splitlines() == [
        '{}:4:14: error: Null pointer dereference: (int*)nullptr [nullPointer]'.format(test_file)
    ]
    return stdout


def test_debug_template(tmp_path):
    __test_debug_template(tmp_path, verbose=False)


def test_debug_template_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_template(tmp_path, verbose=False) == __test_debug_template(tmp_path, verbose=True)


def test_debug_template_debug(tmp_path):
    __test_debug_template(tmp_path, debug=True)


@pytest.mark.xfail(strict=True)  # TODO: remove dependency on --verbose
def test_debug_template_debug_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_template(tmp_path, debug=True, verbose=False) == __test_debug_template(tmp_path, debug=True, verbose=True)


def test_file_ignore_2(tmp_path):  # #13570
    tests_path = tmp_path / 'tests'
    os.mkdir(tests_path)

    lib_path = tmp_path / 'lib'
    os.mkdir(lib_path)

    test_file_1 = lib_path / 'test_1.c'
    with open(test_file_1, 'wt'):
        pass

    args = [
        '-itests',
        '-itest_1.c',
        '.'
    ]

    exitcode, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert exitcode == 1, stdout
    assert stdout.splitlines() == [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]
    assert stderr.splitlines() == []


def test_debug_valueflow_data(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""int f()
{
    double d = 1.0 / 0.5;
    return d;
}
""")

    args = [
        '-q',
        '--debug-valueflow',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout

    # check sections in output
    assert stdout.find('##file ') == -1
    assert stdout.find('##Value flow') != -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == []

    # check precision in output - #13607
    valueflow = stdout[stdout.find('##Value flow'):]
    assert valueflow.splitlines() == [
        '##Value flow',
        'File {}'.format(str(test_file).replace('\\', '/')),
        'Line 3',
        '  = always 2.0',
        '  1.0 always 1.0',
        '  / always 2.0',
        '  0.5 always 0.5',
        'Line 4',
        '  d always {symbolic=(1.0/0.5),2.0}'
    ]


def test_debug_valueflow_data_xml(tmp_path):  # #13606
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""double f()
{
    double d = 0.0000001;
    return d;
}
""")

    args = [
        '-q',
        '--debug-valueflow',
        '--xml',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout

    assert stderr
    assert ElementTree.fromstring(stderr) is not None

    # check sections in output
    assert stdout.find('##file ') == -1
    assert stdout.find('##Value flow') == -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1

    # check XML nodes in output
    debug_xml = ElementTree.fromstring(stdout)
    assert debug_xml is not None
    assert debug_xml.tag == 'debug'
    valueflow_elem = debug_xml.findall('valueflow')
    assert len(valueflow_elem) == 1
    scopes_elem = debug_xml.findall('scopes')
    assert len(scopes_elem) == 1
    ast_elem = debug_xml.findall('ast')
    assert len(ast_elem) == 0

    # check precision in output - #13606
    value_elem = valueflow_elem[0].findall('values/value')
    assert len(value_elem) == 3
    assert 'floatvalue' in value_elem[0].attrib
    assert value_elem[0].attrib['floatvalue'] == '1e-07'
    assert 'floatvalue' in value_elem[1].attrib
    assert value_elem[1].attrib['floatvalue'] == '1e-07'
    assert 'floatvalue' in value_elem[2].attrib
    assert value_elem[2].attrib['floatvalue'] == '1e-07'


def test_dir_ignore(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt'):
        pass

    lib_dir = tmp_path / 'lib'
    os.mkdir(lib_dir)
    lib_test_file = lib_dir / 'test.cpp'
    with open(lib_test_file, 'wt'):
        pass

    args = [
        '-ilib',
        '--debug-ignore',
        str(tmp_path)
    ]
    # make sure the whole directory is being ignored instead of each of its contents individually
    out_lines = [
        'ignored path: {}'.format(lib_dir),
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines, cwd=str(tmp_path))


def test_check_headers(tmp_path):
    test_file_h = tmp_path / 'test.h'
    with open(test_file_h, 'wt') as f:
        f.write(
"""
inline void hdr()
{
    (void)(*((int*)0));
}
""")

    test_file_c = tmp_path / 'test.c'
    with open(test_file_c, 'wt') as f:
        f.write(
"""
#include "test.h"

void f() {}
""")

    args = [
        '-q',
        '--template=simple',
        '--no-check-headers',
        str(test_file_c)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == []  # no error since the header is not checked


def test_unique_error(tmp_path):  # #6366
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""void f()
{
    const long m[9] = {};
    long a=m[9], b=m[9];
    (void)a;
    (void)b;
}
""")

    args = [
        '-q',
        '--template=simple',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}:4:13: error: Array 'm[9]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]".format(test_file),
        "{}:4:21: error: Array 'm[9]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]".format(test_file)
    ]


def test_check_unused_templates_class(tmp_path):
    test_file_h = tmp_path / 'test.h'
    with open(test_file_h, 'wt') as f:
        f.write(
"""template<class T>
class HdrCl1
{
    HdrCl1()
    {
        (void)(*((int*)0));
    }
};

template<typename T>
class HdrCl2
{
    HdrCl2()
    {
        (void)(*((int*)0));
    }
};

template<class T>
struct HdrSt1
{
    HdrSt1()
    {
        (void)(*((int*)0));
    }
};

template<typename T>
struct HdrSt2
{
    HdrSt2()
    {
        (void)(*((int*)0));
    }
};
""")

    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write(
"""#include "test.h"

template<class T>
class Cl1
{
    CL1()
    {
        (void)(*((int*)0));
    }
};

template<typename T>
class Cl2
{
    Cl2()
    {
        (void)(*((int*)0));
    }
};

template<class T>
struct St1
{
    St1()
    {
        (void)(*((int*)0));
    }
};

template<typename T>
struct St2
{
    St2()
    {
        (void)(*((int*)0));
    }
};

void f() {}
""")

    args = [
        '-q',
        '--template=simple',
        '--no-check-unused-templates',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == []  # no error since the unused templates are not being checked


def test_check_unused_templates_func(tmp_path):  # #13714
    test_file_h = tmp_path / 'test.h'
    with open(test_file_h, 'wt') as f:
        f.write(
"""template<class T>
void f_t_hdr_1()
{
    (void)(*((int*)0));
}

template<typename T>
void f_t_hdr_2()
{
    (void)(*((int*)0));
}
""")

    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write(
"""#include "test.h"

template<class T>
void f_t_1()
{
    (void)(*((int*)0));
}

template<typename T>
void f_t_2()
{
    (void)(*((int*)0));
}

void f() {}
""")

    args = [
        '-q',
        '--template=simple',
        '--no-check-unused-templates',
        str(test_file)
    ]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == []  # no error since the unused templates are not being checked

try:
    # TODO: handle exitcode?
    subprocess.call(['clang-tidy', '--version'])
    has_clang_tidy = True
except OSError:
    has_clang_tidy = False

def __test_clang_tidy(tmpdir, use_compdb):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write(
"""static void foo() // NOLINT(misc-use-anonymous-namespace)
{
    (void)(*((int*)nullptr));
}""")

    project_file = __write_compdb(tmpdir, test_file) if use_compdb else None

    args = [
        '-q',
        '--template=simple',
        '--clang-tidy'
    ]
    if project_file:
        args += ['--project={}'.format(project_file)]
    else:
        args += [str(test_file)]
    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == [
    ]
    assert stderr.splitlines() == [
        '{}:3:14: error: Null pointer dereference: (int*)nullptr [nullPointer]'.format(test_file),
        '{}:3:14: style: C-style casts are discouraged; use static_cast/const_cast/reinterpret_cast [clang-tidy-google-readability-casting]'.format(test_file)
    ]


@pytest.mark.skipif(not has_clang_tidy, reason='clang-tidy is not available')
@pytest.mark.xfail(strict=True)  # TODO: clang-tidy is only invoked with FileSettings - see #12053
def test_clang_tidy(tmpdir):  # #12053
    __test_clang_tidy(tmpdir, False)


@pytest.mark.skipif(not has_clang_tidy, reason='clang-tidy is not available')
def test_clang_tidy_project(tmpdir):
    __test_clang_tidy(tmpdir, True)


@pytest.mark.skipif(not has_clang_tidy, reason='clang-tidy is not available')
def test_clang_tidy_error_exit(tmp_path):  # #13828 / #13829
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write(
"""#include <string>
#include <utility>

// cppcheck-suppress clang-tidy-modernize-use-trailing-return-type
static bool f() // NOLINT(misc-use-anonymous-namespace)
{
    std::string str;
    const std::string str1 = std::move(str);
    (void)str1;
    return str.empty();
}""")

    # TODO: remove project file usage when --clang-tidy works with non-project files
    project_file = __write_compdb(tmp_path, str(test_file))

    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '--std=c++11',
        '--clang-tidy',
        '--project={}'.format(project_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}:10:12: warning: 'str' used after it was moved [clang-tidy-bugprone-use-after-move]".format(test_file),
        "{}:10:12: style: 'str' used after it was moved [clang-tidy-hicpp-invalid-access-moved]".format(test_file)
    ]
    assert exitcode == 0, stdout


def test_suppress_unmatched_wildcard(tmp_path):  # #13660
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    # need to run in the temporary folder because the path of the suppression has to match
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--suppress=nullPointer:test*.c',  # checked and matched
        '--suppress=id:test*.c',  # checked and unmatched
        '--suppress=id2:test*.c',  # checked and unmatched
        '--suppress=id2:tes?.c',  # checked and unmatched
        '--suppress=*:test*.c',  # checked and unmatched
        '--suppress=id:test*.cpp',  # unchecked
        '--suppress=id2:test?.cpp',  # unchecked
        'test.c'
    ]
    exitcode, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        'test*.c:0:0: information: Unmatched suppression: id [unmatchedSuppression]',
        'test*.c:0:0: information: Unmatched suppression: id2 [unmatchedSuppression]',
        'tes?.c:0:0: information: Unmatched suppression: id2 [unmatchedSuppression]'
    ]


def test_suppress_unmatched_wildcard_cached(tmp_path):  # #14585
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    build_dir = tmp_path / 'b1'
    os.makedirs(build_dir)

    # need to run in the temporary folder because the path of the suppression has to match
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--cppcheck-build-dir={}'.format(build_dir),
        '--suppress=nullPointer:test*.c',
        'test.c'
    ]

    stderr_exp = []

    exitcode, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == stderr_exp

    exitcode, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == stderr_exp


def test_suppress_unmatched_wildcard_unchecked(tmp_path):
    # make sure that unmatched wildcards suppressions are reported if files matching the expressions were processesd
    # but isSuppressed() has never been called (i.e. no findings in file at all)
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write("""void f() {}""")

    # need to run in the temporary folder because the path of the suppression has to match
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--suppress=id:test*.c',
        '--suppress=id:tes?.c',
        '--suppress=id2:*',
        '--suppress=*:test*.c',
        'test.c'
    ]
    exitcode, stdout, stderr = cppcheck(args, cwd=tmp_path)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        'test*.c:0:0: information: Unmatched suppression: id [unmatchedSuppression]',
        'tes?.c:0:0: information: Unmatched suppression: id [unmatchedSuppression]',
        '*:0:0: information: Unmatched suppression: id2 [unmatchedSuppression]',
        'test*.c:0:0: information: Unmatched suppression: * [unmatchedSuppression]'
    ]


def test_preprocess_enforced_c(tmp_path):  # #10989
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write(
"""#ifdef __cplusplus
#error "err"
#endif""")

    args = [
        '-q',
        '--template=simple',
        '--language=c',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    assert stdout.splitlines() == []
    assert stderr.splitlines() == []


def test_preprocess_enforced_cpp(tmp_path):  # #10989
    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write(
"""#ifdef __cplusplus
#error "err"
#endif""")

    args = [
        '-q',
        '--template=simple',
        '--language=c++',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout if stdout else stderr
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        '{}:2:2: error: #error "err" [preprocessorErrorDirective]'.format(test_file)
    ]


def test_preprocess_system_include(tmp_path): # #13928
    g = glob.glob('/usr/include/c++/*/string')
    if len(g) != 1:
        pytest.skip('<string> header file not found')

    test_file = tmp_path / 'test.c'
    with open(test_file, 'wt') as f:
        f.write('#include <string>\n'
                ';\n')

    def has_missing_include_string_warning(e):
        return '<string>' in e

    args = [
        '--enable=missingInclude',
        str(test_file)
    ]

    # include path not provided => missing include warning about <string>
    _, _, stderr = cppcheck(args)
    assert has_missing_include_string_warning(stderr), stderr

    # include path provided => no missing include warning about <string>
    args.append('-I' + os.path.dirname(str(g[0])))
    _, _, stderr = cppcheck(args)
    assert not has_missing_include_string_warning(stderr), stderr


# TODO: test with --xml
def __test_debug_normal(tmp_path, verbose):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-normal',
        str(test_file)
    ]

    if verbose:
        args += ['--verbose']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') != -1
    assert stdout.find('##Value flow') != -1
    if verbose:
        assert stdout.find('### Symbol database ###') != -1
    else:
        assert stdout.find('### Symbol database ###') == -1
    if verbose:
        assert stdout.find('##AST') != -1
    else:
        assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == [
        '{}:3:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]
    return stdout


def test_debug_normal(tmp_path):
    __test_debug_normal(tmp_path, False)


@pytest.mark.xfail(strict=True)  # TODO: remove dependency on --verbose
def test_debug_normal_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_normal(tmp_path, False) == __test_debug_normal(tmp_path, True)


# TODO: test with --xml
def __test_debug_simplified(tmp_path, verbose):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-simplified',
        str(test_file)
    ]

    if verbose:
        args += ['--verbose']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') != -1
    assert stdout.find('##Value flow') == -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == [
        '{}:3:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]
    return stdout


def test_debug_simplified(tmp_path):
    __test_debug_simplified(tmp_path, False)


def test_debug_simplified_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_simplified(tmp_path, False) == __test_debug_simplified(tmp_path, True)


# TODO: test with --xml
def __test_debug_symdb(tmp_path, verbose):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-symdb',
        str(test_file)
    ]

    if verbose:
        args += ['--verbose']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') == -1
    assert stdout.find('##Value flow') == -1
    assert stdout.find('### Symbol database ###') != -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == [
        '{}:3:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]
    return stdout


def test_debug_symdb(tmp_path):
    __test_debug_symdb(tmp_path, False)


@pytest.mark.skip  # TODO: this contains memory addresses the output will always differ - would require stable identifier
def test_debug_symdb_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_symdb(tmp_path, False) == __test_debug_symdb(tmp_path, True)


# TODO: test with --xml
def __test_debug_ast(tmp_path, verbose):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-ast',
        str(test_file)
    ]

    if verbose:
        args += ['--verbose']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') == -1
    assert stdout.find('##Value flow') == -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') != -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == [
        '{}:3:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]
    return stdout


def test_debug_ast(tmp_path):
    __test_debug_ast(tmp_path, False)


def test_debug_ast_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_ast(tmp_path, False) == __test_debug_ast(tmp_path, True)


# TODO: test with --xml
def __test_debug_valueflow(tmp_path, verbose):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-valueflow',
        str(test_file)
    ]

    if verbose:
        args += ['--verbose']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') == -1
    assert stdout.find('##Value flow') != -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == [
        '{}:3:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file)
    ]
    return stdout


def test_debug_valueflow(tmp_path):
    __test_debug_valueflow(tmp_path, False)


def test_debug_valueflow_verbose_nodiff(tmp_path):
    # make sure --verbose does not change the output
    assert __test_debug_valueflow(tmp_path, False) == __test_debug_valueflow(tmp_path, True)


def test_debug_syntaxerror_c(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""
template<class T> class TemplCl;
void f()
{
    (void)(*((int*)0));
}
""")

    args = [
        '-q',
        '--template=simple',
        '--debug-normal',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.find('##file ') != -1
    assert stdout.find('##Value flow') != -1
    assert stdout.find('### Symbol database ###') == -1
    assert stdout.find('##AST') == -1
    assert stdout.find('### Template Simplifier pass ') == -1
    assert stderr.splitlines() == [
        "{}:2:1: error: Code 'template<...' is invalid C code. [syntaxError]".format(test_file)
    ]


def test_ast_max_depth(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, "w") as f:
        f.write(
"""
#define PTR1 (* (* (* (*
#define PTR2 PTR1 PTR1 PTR1 PTR1
#define PTR3 PTR2 PTR2 PTR2 PTR2
#define PTR4 PTR3 PTR3 PTR3 PTR3

#define RBR1 ) ) ) )
#define RBR2 RBR1 RBR1 RBR1 RBR1
#define RBR3 RBR2 RBR2 RBR2 RBR2
#define RBR4 RBR3 RBR3 RBR3 RBR3

int PTR4 q4_var RBR4 = 0;
""")

    args = [
        '-q',
        '--template=simple',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        '{}:12:5: error: maximum AST depth exceeded [internalAstError]'.format(test_file)
    ]


# do not report unmatched misra-* suppressions when misra is not provided
def test_misra_disabled_unmatched(tmp_path):  #14232
    test_file = tmp_path / 'test.c'
    with open(test_file, "w"):
        pass

    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--suppress=misra-c2012-20.5',
        '--suppress=uninitvar',
        str(test_file)
    ]

    ret, stdout, stderr = cppcheck(args)
    assert stderr.splitlines() == [
        'nofile:0:0: information: Unmatched suppression: uninitvar [unmatchedSuppression]'
    ]
    assert stdout == ''
    assert ret == 0, stdout


# do not report unmatched premium-* suppressions when application is not premium
def test_premium_disabled_unmatched(tmp_path):  #13663
    test_file = tmp_path / 'test.c'
    with open(test_file, "w"):
        pass

    args = [
        '-q',
        '--template=simple',
        '--enable=warning,information',
        '--suppress=premium-misra-cpp-2023-12.2.1',
        '--suppress=uninitvar',
        str(test_file)
    ]

    ret, stdout, stderr = cppcheck(args)
    assert stderr.splitlines() == [
        'nofile:0:0: information: Unmatched suppression: uninitvar [unmatchedSuppression]'
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_unmatched_file(tmp_path):  # #14248 / #14249
    lib_path = tmp_path / 'lib'
    os.makedirs(lib_path)

    test_file = lib_path / 'test.c'
    with open(test_file, "w"):
        pass

    suppr_txt = tmp_path / 'suppr.txt'
    with open(suppr_txt, "w") as f:
        f.write('''
error:lib/test.c
error2:lib\\test.c
''')

    suppr_xml = tmp_path / 'suppr.xml'
    with open(suppr_xml, "w") as f:
        f.write('''
<suppressions>
    <suppress>
        <id>error3</id>
        <fileName>lib/test.c</fileName>
    </suppress>
    <suppress>
        <id>error4</id>
        <fileName>lib\\test.c</fileName>
    </suppress>
</suppressions>
''')

    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        f'--suppressions-list={suppr_txt}',
        f'--suppress-xml={suppr_xml}',
        '--suppress=error5:lib/test.c',
        '--suppress=error6:lib\\test.c',
        str(test_file)
    ]

    lib_file = 'lib' + os.path.sep + 'test.c'

    ret, stdout, stderr = cppcheck(args)
    assert stdout == ''
    assert stderr.splitlines() == [
        f'{lib_file}:0:0: information: Unmatched suppression: error [unmatchedSuppression]',
        f'{lib_file}:0:0: information: Unmatched suppression: error2 [unmatchedSuppression]',
        f'{lib_file}:0:0: information: Unmatched suppression: error3 [unmatchedSuppression]',
        f'{lib_file}:0:0: information: Unmatched suppression: error4 [unmatchedSuppression]',
        f'{lib_file}:0:0: information: Unmatched suppression: error5 [unmatchedSuppression]',
        f'{lib_file}:0:0: information: Unmatched suppression: error6 [unmatchedSuppression]'
    ]
    assert ret == 0, stdout


def test_simplecpp_warning(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""
#define warning "warn msg"
""")

    args = [
        '-q',
        '--template=simple',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == []


def test_simplecpp_unhandled_char(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w", encoding='utf-8') as f:
        f.write(
"""
int 你=0;
""")

    args = [
        '-q',
        '--template=simple',
        '--emit-duplicates',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        '{}:2:5: error: The code contains unhandled character(s) (character code=228). Neither unicode nor extended ascii is supported. [unhandledChar]'.format(test_file)
    ]


def test_simplecpp_include_nested_too_deeply(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write('#include "test.h"')

    test_h = tmp_path / 'test.h'
    with open(test_h, "w") as f:
        f.write('#include "test_0.h"')

    for i in range(400):
        test_h = tmp_path / f'test_{i}.h'
        with open(test_h, "w") as f:
            f.write('#include "test_{}.h"'.format(i+1))

    args = [
        '-q',
        '--template=simple',
        '--emit-duplicates',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    test_h = tmp_path / 'test_398.h'
    assert stderr.splitlines() == [
        # TODO: should only report the error once
        '{}:1:2: error: #include nested too deeply [includeNestedTooDeeply]'.format(test_h),
        '{}:1:2: error: #include nested too deeply [includeNestedTooDeeply]'.format(test_h)
    ]


def test_simplecpp_syntax_error(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write('#include ""')

    args = [
        '-q',
        '--template=simple',
        '--emit-duplicates',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        # TODO: should only report the error once
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file),
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file)
    ]


@pytest.mark.parametrize('max_configs,number_of_configs,check_config,expected_warn', [
    # max configs = default, max configs < number of configs => warn
    (None, 20, False, True),
    (None, 20, True, True),

    # max configs != default, max configs < number of configs => warn if --check-config
    (6, 20, False, False),
    (6, 20, True, True),

    # max configs >= number of configs => no warning
    (20, 20, False, False),
    (20, 20, False, False)
])
def test_max_configs(tmp_path, max_configs, number_of_configs, check_config, expected_warn):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, "w") as f:
        for i in range(1,number_of_configs):
            dir = 'if' if i == 1 else 'elif'
            f.write(f'#{dir} defined(X{i})\nx = {i};\n')
        f.write('#endif\n')

    args = ['--enable=information', '--template=simple', str(test_file)]

    if max_configs is None:
        max_configs = 12  # default value
    else:
        args = [f'--max-configs={max_configs}'] + args

    if check_config:
        args = ['--check-config'] + args

    # default max configs is set to 12, warn if code contains more configurations than that
    _, _, stderr = cppcheck(args)
    if not expected_warn:
        assert stderr.splitlines() == []
    else:
        assert stderr.splitlines() == [
            '{}:0:0: information: Too many #ifdef configurations - cppcheck only checks {} of {} configurations. Use --force to check all configurations. [toomanyconfigs]'
            .format(test_file, max_configs, number_of_configs)
        ]


def test_no_valid_configuration(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""#include ""
#ifdef DEF_1
#include ""
#endif
""")

    args = [
        '--template=simple',
        '--emit-duplicates',
        '--enable=information',
        '--suppress=checkersReport',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file)
    ]
    # TODO: this lacks context about the configuration which encounters these errors
    # TODO: add message when a configuration is dropped?
    assert stderr.splitlines() == [
        # TODO: should only report the error once
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file),
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file),
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file),
        '{}:0:0: information: This file is not analyzed. No working configuration could be extracted. Use -v for more details. [noValidConfiguration]'.format(test_file)
    ]


def test_no_valid_configuration_check_config(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""#include ""
#ifdef DEF_1
#include ""
#endif
""")

    args = [
        '--template=simple',
        '--emit-duplicates',
        '--enable=information',
        '--suppress=checkersReport',
        '--check-config',
        str(test_file)
    ]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file)
    ]
    # TODO: this lacks context about the configuration which encounters these errors
    # TODO: add message when a configuration is dropped
    assert stderr.splitlines() == [
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file),
        '{}:1:2: error: No header in #include [syntaxError]'.format(test_file)
    ]


def __test_active_checkers(tmp_path, active_cnt, total_cnt, use_misra=False, use_unusedfunction_only=False, checkers_exp=None):
    test_file = tmp_path / 'test.c'
    with open(test_file, 'w') as f:
        f.write('int i;')

    build_dir = None
    if checkers_exp is not None:
        build_dir = tmp_path / 'b1'
        os.makedirs(build_dir)

    args = [
        '-q',
        '--enable=information',
        '-j1',
        str(test_file)
    ]

    if use_misra:
        args += ['--addon=misra']
    if build_dir:
        args += ['--cppcheck-build-dir={}'.format(build_dir)]
    else:
        args += ['--no-cppcheck-build-dir']

    env = {}
    if use_unusedfunction_only:
        env = {'UNUSEDFUNCTION_ONLY': '1'}
        args += ['--enable=unusedFunction']
    exitcode, stdout, stderr, _ = cppcheck_ex(args, remove_checkers_report=False, env=env)
    assert exitcode == 0, stdout
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f'nofile:0:0: information: Active checkers: {active_cnt}/{total_cnt} (use --checkers-report=<filename> to see details) [checkersReport]',
        ''  # TODO: get rid of extra newline
    ]

    if build_dir:
        checkers_file = build_dir / 'checkers.txt'
        with open(checkers_file, 'r') as f:
            checkers = f.read().splitlines()

        assert checkers == checkers_exp
        assert len(checkers) == active_cnt


def test_active_unusedfunction_only(tmp_path):
    __test_active_checkers(tmp_path, 1, 186, use_unusedfunction_only=True)


def test_active_unusedfunction_only_builddir(tmp_path):
    checkers_exp = [
        'CheckUnusedFunctions::check'
    ]
    __test_active_checkers(tmp_path, 1, 186, use_unusedfunction_only=True, checkers_exp=checkers_exp)


def test_active_unusedfunction_only_misra(tmp_path):
    __test_active_checkers(tmp_path, 1, 386, use_unusedfunction_only=True, use_misra=True)


def test_active_unusedfunction_only_misra_builddir(tmp_path):
    checkers_exp = [
        'CheckUnusedFunctions::check'
    ]
    __test_active_checkers(tmp_path, 1, 386, use_unusedfunction_only=True, use_misra=True, checkers_exp=checkers_exp)


def test_analyzerinfo(tmp_path):
    test_file = tmp_path / 'test.c'
    with open(test_file, "w") as f:
        f.write(
"""void f()
{
    (void)(*((int*)0));
}
""")

    build_dir = tmp_path / 'b1'
    os.makedirs(build_dir)

    test_a1_file = build_dir / 'test.a1'

    args = [
        '-q',
        '--debug-analyzerinfo',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(build_dir),
        '--enable=all',
        str(test_file)
    ]

    stderr_exp = [
        '{}:3:14: error: Null pointer dereference: (int*)0 [nullPointer]'.format(test_file),
        "{}:1:6: style: The function 'f' is never used. [unusedFunction]".format(test_file)
    ]

    def run_and_assert_cppcheck(stdout_exp):
        exitcode, stdout, stderr = cppcheck(args)
        assert exitcode == 0, stdout
        assert stdout.splitlines() == stdout_exp
        assert stderr.splitlines() == stderr_exp

    test_file_s = str(test_file).replace('\\', '/')
    test_a1_file_s = str(test_a1_file).replace('\\', '/')

    # no cached results
    run_and_assert_cppcheck([
        "no cached result '{}' for '{}' found".format(test_a1_file_s, test_file_s)
    ])

    # cached results
    run_and_assert_cppcheck([
        "skipping analysis - loaded 1 cached finding(s) from '{}' for '{}'".format(test_a1_file_s, test_file_s)
    ])

    # modified file
    with open(test_file, 'a') as f:
        f.write('\n#define DEF')

    run_and_assert_cppcheck([
        "discarding cached result from '{}' for '{}' - hash mismatch".format(test_a1_file_s, test_file_s)
    ])

    # invalid XML
    with open(test_a1_file, 'a') as f:
        f.write('.')

    run_and_assert_cppcheck([
        "discarding cached result - failed to load '{}' for '{}' (XML_ERROR_PARSING_TEXT)".format(test_a1_file_s, test_file_s)
    ])

    # missing root node
    with open(test_a1_file, 'w') as f:
        f.write('<?xml version="1.0"?>')

    run_and_assert_cppcheck([
        "discarding cached result from '{}' for '{}' - no root node found".format(test_a1_file_s, test_file_s)
    ])

    # mismatched root node
    with open(test_a1_file, 'w') as f:
        f.write('<?xml version="1.0"?><root/>')

    run_and_assert_cppcheck([
        "discarding cached result from '{}' for '{}' - unexpected root node".format(test_a1_file_s, test_file_s)
    ])

    # missing 'hash' attribute
    with open(test_a1_file, 'w') as f:
        f.write('<?xml version="1.0"?><analyzerinfo/>')

    run_and_assert_cppcheck([
        "discarding cached result from '{}' for '{}' - no 'hash' attribute found".format(test_a1_file_s, test_file_s)
    ])

    # invalid 'hash' attribute
    with open(test_a1_file, 'w') as f:
        f.write('<?xml version="1.0"?><analyzerinfo hash="hash"/>')

    run_and_assert_cppcheck([
        "discarding cached result from '{}' for '{}' - hash mismatch".format(test_a1_file_s, test_file_s)
    ])

    # TODO:
    # - invalid error
    # - internalError


def test_ctu_function_call_path_slash(tmp_path):  # #14591
    test_file = tmp_path / 'test.cpp'
    with open(test_file, "w") as f:
        f.write(
"""void g(T* p)
{
    *p = 0;
}

void f(T* p)
{
    p = nullptr;
    g(p);
}
""")

    build_dir = tmp_path / 'b1'
    os.makedirs(build_dir)

    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(build_dir),
        str(test_file)
    ]

    exitcode, _, _ = cppcheck(args)
    assert exitcode == 0

    test_a1_file = build_dir / 'test.a1'
    analyzerinfo = ElementTree.fromstring(test_a1_file.read_text())
    function_call_paths = analyzerinfo.findall('FileInfo/function-call/path')
    assert len(function_call_paths) == 1
    file = function_call_paths[0].attrib['file']
    assert file
    assert not '\\' in file  # the path was incorrectly converted to native
