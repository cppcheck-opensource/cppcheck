# python -m pytest test-projects.py

import pytest
import os
import json
import sys
from testutils import cppcheck


@pytest.mark.parametrize("project_ext", ["json", "sln", "vcxproj", "bpr", "cppcheck"])
def test_missing_project(project_ext):
    project_file = "file.{}".format(project_ext)

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert 1 == ret
    assert "cppcheck: error: failed to open project '{}'. The file does not exist.\n".format(project_file) == stdout
    assert "" == stderr


def __test_project_error(tmpdir, ext, content, expected):
    project_file = os.path.join(tmpdir, "file.{}".format(ext))

    with open(project_file, 'w') as f:
        if content is not None:
            f.write(content)

    ret, stdout, stderr = cppcheck(['--project=' + str(project_file)])
    assert 1 == ret
    assert "cppcheck: error: " + expected + "\ncppcheck: error: failed to load project '{}'. An error occurred.\n".format(project_file) == stdout
    assert "" == stderr


@pytest.mark.parametrize("project_ext, expected", [
    ("json", "compilation database is not a JSON array"),
    ("sln", "Visual Studio solution file is empty"),
    ("vcxproj", "Visual Studio project file is not a valid XML - XML_ERROR_EMPTY_DOCUMENT"),
    ("bpr", "Borland project file is not a valid XML - XML_ERROR_EMPTY_DOCUMENT"),
    ("cppcheck", "Cppcheck GUI project file is not a valid XML - XML_ERROR_EMPTY_DOCUMENT")
])
def test_empty_project(tmpdir, project_ext, expected):
    __test_project_error(tmpdir, project_ext, None, expected)


def test_json_entry_file_not_found(tmpdir):
    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "bug1.cpp",
         "output": "bug1.o"}
    ]

    project_file = os.path.join(tmpdir, "file.json")
    missing_file = os.path.join(tmpdir, "bug1.cpp")
    missing_file_posix = missing_file

    if sys.platform == "win32":
        missing_file_posix = missing_file_posix.replace('\\', '/')

    with open(project_file, 'w') as f:
        f.write(json.dumps(compilation_db))

    ret, _, stderr = cppcheck([
        '--template=simple',
        "--project=" + str(project_file)
    ])
    assert 0 == ret
    assert stderr == f"{missing_file}:0:0: error: File is missing: {missing_file_posix} [missingFile]\n"


def test_json_no_arguments(tmpdir):
    compilation_db = [
        {"directory": str(tmpdir),
         "file": "bug1.cpp",
         "output": "bug1.o"}
    ]

    expected = "no 'arguments' or 'command' field found in compilation database entry"

    __test_project_error(tmpdir, "json", json.dumps(compilation_db), expected)


def test_json_invalid_arguments(tmpdir):
    compilation_db = [
        {"directory": str(tmpdir),
         "arguments": "",
         "file": "bug1.cpp",
         "output": "bug1.o"}
    ]

    expected = "'arguments' field in compilation database entry is not a JSON array"

    __test_project_error(tmpdir, "json", json.dumps(compilation_db), expected)


def test_sln_invalid_file(tmpdir):
    content = "some file"

    expected = "Visual Studio solution file header not found"

    __test_project_error(tmpdir, "sln", content, expected)


def test_sln_no_header(tmpdir):
    content = "\xEF\xBB\xBF\r\n" \
              "some header"

    expected = "Visual Studio solution file header not found"

    __test_project_error(tmpdir, "sln", content, expected)


def test_sln_no_projects(tmpdir):
    content = "\xEF\xBB\xBF\r\n" \
              "Microsoft Visual Studio Solution File, Format Version 12.00\r\n"

    expected = "no projects found in Visual Studio solution file"

    __test_project_error(tmpdir, "sln", content, expected)


def test_sln_project_file_not_found(tmpdir):
    content = "\xEF\xBB\xBF\r\n" \
              "Microsoft Visual Studio Solution File, Format Version 12.00\r\n" \
              "# Visual Studio Version 16\r\n" \
              "VisualStudioVersion = 16.0.29020.237\r\n" \
              "MinimumVisualStudioVersion = 10.0.40219.1\r\n" \
              'Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "cli", "cli\\cli.vcxproj", "{35CBDF51-2456-3EC3-99ED-113C30858883}"\r\n' \
              "ProjectSection(ProjectDependencies) = postProject\r\n" \
              "{C183DB5B-AD6C-423D-80CA-1F9549555A1A} = {C183DB5B-AD6C-423D-80CA-1F9549555A1A}\r\n" \
              "EndProjectSection\r\n" \
              "EndProject\r\n"

    expected = "Visual Studio project file is not a valid XML - XML_ERROR_FILE_NOT_FOUND\n" \
               "cppcheck: error: failed to load '{}' from Visual Studio solution".format(os.path.join(tmpdir, "cli/cli.vcxproj"))
    if sys.platform == "win32":
        expected = expected.replace('\\', '/')

    __test_project_error(tmpdir, "sln", content, expected)


def test_vcxproj_no_xml_root(tmpdir):
    content = '<?xml version="1.0" encoding="utf-8"?>'

    expected = "Visual Studio project file has no XML root node"

    __test_project_error(tmpdir, "vcxproj", content, expected)


def test_bpr_no_xml_root(tmpdir):
    content = '<?xml version="1.0" encoding="utf-8"?>'

    expected = "Borland project file has no XML root node"

    __test_project_error(tmpdir, "bpr", content, expected)


def test_cppcheck_no_xml_root(tmpdir):
    content = '<?xml version="1.0" encoding="utf-8"?>'

    expected = "Cppcheck GUI project file has no XML root node"

    __test_project_error(tmpdir, "cppcheck", content, expected)
