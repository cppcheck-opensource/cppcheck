# Running the test with Python 3:
# Command in cppcheck directory:
# PYTHONPATH=./addons python3 -m pytest addons/test/misra_test.py

import pytest
import re
import sys
import os

from .util import dump_create, dump_remove, convert_json_output
from addons.misra import C11_STDLIB_IDENTIFIERS, C99_STDLIB_IDENTIFIERS,C90_STDLIB_IDENTIFIERS, isStdLibId, isKeyword

TEST_SOURCE_FILES = [os.path.join('addons','test','misra','misra-test.c')]


def remove_misra_config(s:str):
    ret = ''
    for line in s.splitlines():
        if '[misra-config]' not in line:
            ret += line + '\n'
    return ret


@pytest.fixture(scope="function")
def checker():
    from addons.misra import MisraChecker, MisraSettings, get_args_parser
    parser = get_args_parser()
    args = parser.parse_args([])
    settings = MisraSettings(args)
    return MisraChecker(settings)


# FIXME: files are generates in the source tree so it will cause issues if tests are run with xdist.
@pytest.fixture
def test_files():
    for f in TEST_SOURCE_FILES:
        dump_create(f)
    yield
    for f in TEST_SOURCE_FILES:
        dump_remove(f)


def test_loadRuleTexts_structure(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_structure.txt")
    assert(checker.ruleTexts.get(101, None) is None)
    assert(checker.ruleTexts[102].text == "Rule text.")
    assert(checker.ruleTexts.get(103, None) is None)
    assert(checker.ruleTexts[201].text == "Rule text for 2.1.")
    assert(checker.ruleTexts.get(202, None) is None)


def test_loadRuleTexts_empty_lines(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_empty_lines.txt")
    assert(len(checker.ruleTexts) == 3)
    assert(len(checker.ruleTexts[102].text) == len("Rule text."))


def test_loadRuleTexts_mutiple_lines(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_multiple_lines.txt")
    assert(checker.ruleTexts[101].text == "Multiple lines text.")
    assert(checker.ruleTexts[102].text == "Multiple lines text.")
    assert(checker.ruleTexts[103].text == "Multiple lines text.")
    assert(checker.ruleTexts[104].text == "Should")
    assert(checker.ruleTexts[105].text == "Should starts from lowercase letter.")
    assert(checker.ruleTexts[106].text == "Can contain empty lines.")


def test_verifyRuleTexts(checker, capsys):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.verifyRuleTexts()
    captured = capsys.readouterr().out
    assert("21.3" not in captured)
    assert("1.3" in captured)


def test_rules_misra_severity(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    assert(checker.ruleTexts[1004].misra_severity == 'Mandatory')
    assert(checker.ruleTexts[401].misra_severity == 'Required')
    assert(checker.ruleTexts[1505].misra_severity == 'Advisory')
    assert(checker.ruleTexts[2104].misra_severity == '')


def test_json_out(checker, capsys, test_files):
    sys.argv.append("--cli")
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.parseDump("./addons/test/misra/misra-test.c.dump")
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    sys.argv.remove("--cli")
    json_output = convert_json_output(captured)
    assert("Mandatory" in json_output['c2012-10.4'][0]['extra'])
    assert("Required" in json_output['c2012-21.3'][0]['extra'])
    assert("Advisory" in json_output['c2012-20.1'][0]['extra'])


def test_rules_cppcheck_severity(checker, capsys, test_files):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.parseDump("./addons/test/misra/misra-test.c.dump")
    captured = capsys.readouterr().err
    assert("(error)" not in remove_misra_config(captured))
    assert("(warning)" not in captured)
    assert("(style)" in captured)

def test_rules_cppcheck_severity_custom(checker, capsys, test_files):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.setSeverity("custom-severity")
    checker.parseDump("./addons/test/misra/misra-test.c.dump")
    captured = capsys.readouterr().err
    assert("(error)" not in remove_misra_config(captured))
    assert("(warning)" not in captured)
    assert("(style)" not in captured)
    assert("(custom-severity)" in captured)

def test_rules_suppression(checker, capsys):
    test_sources = ["addons/test/misra/misra-suppressions1-test.c",
                    "addons/test/misra/misra-suppressions2-test.c"]

    for src in test_sources:
        re_suppressed= r"\[%s\:[0-9]+\]" % src
        dump_create(src, "--suppressions-list=addons/test/misra/suppressions.txt","--inline-suppr")
        checker.parseDump(src + ".dump")
        captured = capsys.readouterr().err
        found = re.search(re_suppressed, captured)
        assert found is None, 'Unexptected output:\n' + captured
        dump_remove(src)

def test_arguments_regression():
    args_ok = ["-generate-table",
               "--rule-texts=./addons/test/assets/misra_rules_multiple_lines.txt",
               "--verify-rule-texts",
               "-t=foo", "--template=foo",
               "--suppress-rules=15.1",
               "--quiet",
               "--cli",
               "--no-summary",
               "--show-suppressed-rules",
               "-P=src/", "--file-prefix=src/",
               "--severity=misra-warning"]
    # Arguments with expected SystemExit
    args_exit = ["--non-exists", "--non-exists-param=42", "-h", "--help"]

    from addons.misra import get_args_parser

    # sys.argv contains all pytest arguments - so clear all existing arguments first and restore afterwards
    sys_argv_old = sys.argv
    sys.argv = [sys.argv[0]]

    try:
        for arg in args_exit:
            sys.argv.append(arg)
            with pytest.raises(SystemExit):
                parser = get_args_parser()
                parser.parse_args()
            sys.argv.remove(arg)

        for arg in args_ok:
            sys.argv.append(arg)
            try:
                parser = get_args_parser()
                parser.parse_args()
            except SystemExit:
                pytest.fail("Unexpected SystemExit with '%s'" % arg)
            sys.argv.remove(arg)
    finally:
        sys.argv = sys_argv_old


def test_read_ctu_info_line(checker):
    assert checker.read_ctu_info_line('{') is None
    assert checker.read_ctu_info_line('{"summary":"123"}') is None
    assert checker.read_ctu_info_line('{"data":123}') is None
    assert checker.read_ctu_info_line('{"summary":"123","data":123}') is not None

def test_platform(checker):
    test_file = os.path.join('addons','test','misra','misra-test.c')
    dump_create(test_file, "--language=c++")
    checker.parseDump(test_file + ".dump")
    assert checker.is_cpp is True

    dump_create(test_file, "--language=c")
    checker.parseDump(test_file + ".dump")
    assert checker.is_cpp is False

def test_std99_identifiers():
    for headerfile in C90_STDLIB_IDENTIFIERS:
        for identifier in C90_STDLIB_IDENTIFIERS[headerfile]:
            assert identifier in C99_STDLIB_IDENTIFIERS[headerfile], f"{identifier} of C90 not found in C99_STDLIB_IDENTIFIERS"

def test_stdC11_identifiers():
    for headerfile in C90_STDLIB_IDENTIFIERS:
        for identifier in C90_STDLIB_IDENTIFIERS[headerfile]:
            assert identifier in C99_STDLIB_IDENTIFIERS[headerfile], f"{identifier} of C90 not found in C11_STDLIB_IDENTIFIERS"
    for headerfile in C99_STDLIB_IDENTIFIERS:
        for identifier in C99_STDLIB_IDENTIFIERS[headerfile]:
            assert identifier in C11_STDLIB_IDENTIFIERS[headerfile], f"{identifier} of C99 not found in C11_STDLIB_IDENTIFIERS"

def test_isStdLibId():
    # Check that Identifiers from C90 are correctly classified
    assert isStdLibId("assert", 'c89') is True
    assert isStdLibId("assert", 'c99') is True
    assert isStdLibId("assert", 'c11') is True
    assert isStdLibId("assert", 'c23') is True

    # Check that Identifiers from C99 are correctly classified
    assert isStdLibId("UINT32_C", 'c89') is False
    assert isStdLibId("UINT32_C", 'c99') is True
    assert isStdLibId("UINT32_C", 'c11') is True
    assert isStdLibId("UINT32_C", 'c23') is True

    # Check that Identifiers from C11 are correctly classified
    assert isStdLibId("sprintf_s", 'c89') is False
    assert isStdLibId("sprintf_s", 'c99') is False
    assert isStdLibId("sprintf_s", 'c11') is True
    assert isStdLibId("sprintf_s", 'c23') is True

    # Function Defaulting to C99
    assert isStdLibId("assert") is True
    assert isStdLibId("UINT32_C") is True
    assert isStdLibId("sprintf_s") is False

def test_isKeyword():
    # Check that Keywords from C90 are correctly classified
    assert isKeyword("if", 'c89') is True
    assert isKeyword("if", 'c99') is True
    assert isKeyword("if", 'c11') is True
    assert isKeyword("if", 'c23') is True

    # Check that Keywords from C99 are correctly classified
    assert isKeyword("inline", 'c89') is False
    assert isKeyword("inline", 'c99') is True
    assert isKeyword("inline", 'c11') is True
    assert isKeyword("inline", 'c23') is True

    # Check that Keywords from C11 are correctly classified
    assert isKeyword("static_assert", 'c89') is False
    assert isKeyword("static_assert", 'c99') is False
    assert isKeyword("static_assert", 'c11') is True
    assert isKeyword("static_assert", 'c23') is True

    # Function Defaulting to C99
    assert isKeyword("if") is True
    assert isKeyword("inline") is True
    assert isKeyword("static_assert") is False
