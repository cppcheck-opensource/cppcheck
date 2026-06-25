# python -m pytest metrics_test.py

import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__addon_output = """
{"metric":{"fileName":"1.cpp","function":"write","id":"HISCall","lineNumber":6,"value":2}}
{"metric":{"fileName":"1.cpp","function":"write","id":"HISGoto","lineNumber":6,"value":0}}
{"metric":{"fileName":"1.cpp","function":"write","id":"HISLevel","lineNumber":6,"value":2}}
{"metric":{"fileName":"1.cpp","function":"write","id":"HISParam","lineNumber":6,"value":2}}
{"metric":{"fileName":"1.cpp","function":"write","id":"HISPath","lineNumber":6,"value":3}}
{"metric":{"fileName":"1.cpp","function":"write","id":"HISReturn","lineNumber":6,"value":0}}
{"metric":{"fileName":"1.cpp","function":"write","id":"HISStmt","lineNumber":6,"value":15}}
{"metric":{"fileName":"1.cpp","function":"write","id":"cyclomaticComplexity","lineNumber":6,"value":3}}
"""
__addon_source = f'print("""{__addon_output}""")'
__expected_xml = [
    '<metric fileName="1.cpp" function="write" id="HISCall" lineNumber="6" value="2"/>',
    '<metric fileName="1.cpp" function="write" id="HISGoto" lineNumber="6" value="0"/>',
    '<metric fileName="1.cpp" function="write" id="HISLevel" lineNumber="6" value="2"/>',
    '<metric fileName="1.cpp" function="write" id="HISParam" lineNumber="6" value="2"/>',
    '<metric fileName="1.cpp" function="write" id="HISPath" lineNumber="6" value="3"/>',
    '<metric fileName="1.cpp" function="write" id="HISReturn" lineNumber="6" value="0"/>',
    '<metric fileName="1.cpp" function="write" id="HISStmt" lineNumber="6" value="15"/>',
    '<metric fileName="1.cpp" function="write" id="cyclomaticComplexity" lineNumber="6" value="3"/>'
]

def __create_addon(tmpdir):
    path = os.path.join(tmpdir, 'addon.py')
    with open(path, 'w') as file:
        file.write(__addon_source)
    return path

def __create_source_file(tmpdir):
    path = os.path.join(tmpdir, 'test.c')
    with open(path, 'w') as _:
        pass
    return path

def test_dummy_metrics_xml_report(tmpdir):
    output_file = os.path.join(tmpdir, "results.xml")
    source_path = __create_source_file(tmpdir)
    addon_path = __create_addon(tmpdir)
    args = [
        f'--output-file={output_file}',
        f'--addon={addon_path}',
        '--xml-version=3',
        source_path
    ]

    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stderr == ''
    assert stdout == f'Checking {source_path} ...\n'

    with open(output_file, 'r') as file:
        xml = file.read()

    for expected in __expected_xml:
        assert xml.find(expected) >= 0

def test_dummy_metrics_stdout(tmpdir):
    source_path = __create_source_file(tmpdir)
    addon_path = __create_addon(tmpdir)
    args = [
        f'--addon={addon_path}',
        '--xml-version=3',
        source_path
    ]

    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout == f'Checking {source_path} ...\n'

    for expected in __expected_xml:
        assert stderr.find(expected) >= 0

