# python -m pytest gitlab_code_quality_test.py

import json

from testutils import cppcheck

TEST_CODE = """
int main()
{
    int x = 0;
    return 1 / x;
}
"""


def test_gitlab_code_quality(tmp_path):
    (tmp_path / "test.cpp").write_text(TEST_CODE)

    exitcode, _, stderr = cppcheck(
        ["--output-format=gitlab-codequality", "--enable=warning", "test.cpp"],
        cwd=tmp_path)

    assert exitcode == 0
    findings = json.loads(stderr)
    assert len(findings) == 1

    finding = findings[0]
    assert finding["description"] == "Division by zero."
    assert finding["check_name"] == "zerodiv"
    assert finding["fingerprint"]
    assert finding["severity"] == "critical"
    assert finding["location"]["path"] == "test.cpp"
    assert finding["location"]["lines"]["begin"] == 5


def test_gitlab_code_quality_output_file(tmp_path):
    (tmp_path / "test.cpp").write_text(TEST_CODE)

    exitcode, _, stderr = cppcheck(
        ["--output-format=gitlab-codequality", "--enable=warning", "--output-file=report.json", "test.cpp"],
        cwd=tmp_path)

    assert exitcode == 0
    assert stderr == ""
    findings = json.loads((tmp_path / "report.json").read_text())
    assert len(findings) == 1
    assert findings[0]["check_name"] == "zerodiv"
