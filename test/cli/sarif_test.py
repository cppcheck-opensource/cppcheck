# python -m pytest sarif_test.py

import os
import json
import tempfile

import pytest

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

# Test code with various error types
TEST_CODE = """
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <memory>

class TestClass {
public:
    TestClass() : value(0) {}
    ~TestClass() { delete ptr; }
    
    void setValue(int v) { value = v; }
    int getValue() const { return value; }
    
private:
    int value;
    int* ptr = nullptr;
};

void testSecurityViolations() {
    // Null pointer dereference
    int* ptr = nullptr;
    *ptr = 5;
    
    // Array bounds violation  
    int array[5];
    array[10] = 1;
    
    // Memory leak
    int* mem = (int*)malloc(sizeof(int) * 10);
    // forgot to free mem
    
    // Uninitialized variable
    int x;
    printf("%d", x);
    
    // Double free
    int* p = (int*)malloc(sizeof(int));
    free(p);
    free(p);
    
    // Buffer overflow with strcpy
    char buffer[10];
    char source[20] = "This is too long";
    strcpy(buffer, source);
    
    // Use after free
    int* freed = (int*)malloc(sizeof(int));
    free(freed);
    *freed = 42;
}

void testStyleAndPortabilityIssues() {
    // Redundant assignment
    int redundant = 5;
    redundant = redundant;
    
    // Unused variable
    int unused = 42;
    
    // Variable scope reduction
    int i;
    for (i = 0; i < 10; i++) {
        // i could be declared in for loop
    }
}

int main() {
    testSecurityViolations();
    testStyleAndPortabilityIssues();
    return 0;
}
"""

BASIC_TEST_CODE = """
int main() {
    int* p = nullptr;
    *p = 5; // null pointer dereference
    return 0;
}
"""


def create_test_file(tmp_path, filename="test.cpp", content=TEST_CODE):
    """Create a temporary test file with the given content."""
    filepath = tmp_path / filename
    filepath.write_text(content)
    return str(filepath)


def run_sarif_check(code, extra_args=None):
    """Run cppcheck with SARIF output on the given code."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tmp_path = os.path.join(tmpdir, "test.cpp")
        with open(tmp_path, "w") as f:
            f.write(code)

        args = ["--output-format=sarif", "--enable=all", tmp_path]

        if extra_args:
            args.extend(extra_args)

        _, _, stderr = cppcheck(args)

        # SARIF output is in stderr
        try:
            sarif_data = json.loads(stderr)
            return sarif_data
        except json.JSONDecodeError as e:
            pytest.fail(f"Failed to parse SARIF JSON: {e}\nOutput: {stderr}")
            return None


def test_sarif_basic_structure():
    """Test that SARIF output has the correct basic structure."""
    sarif = run_sarif_check(BASIC_TEST_CODE)

    # Check required SARIF fields
    assert sarif["version"] == "2.1.0"
    assert "$schema" in sarif
    assert "sarif-schema" in sarif["$schema"]
    assert "runs" in sarif
    assert len(sarif["runs"]) == 1

    run = sarif["runs"][0]
    assert "tool" in run
    assert "results" in run

    tool = run["tool"]
    assert "driver" in tool

    driver = tool["driver"]
    assert driver["name"] == "Cppcheck"
    assert "rules" in driver
    assert "semanticVersion" in driver


def test_sarif_null_pointer():
    """Test SARIF output for null pointer dereference."""
    sarif = run_sarif_check(BASIC_TEST_CODE)

    run = sarif["runs"][0]
    results = run["results"]

    # Should have at least one result
    assert len(results) > 0

    # Find null pointer result
    null_pointer_results = [r for r in results if r["ruleId"] == "nullPointer"]
    assert len(null_pointer_results) > 0

    result = null_pointer_results[0]
    assert result["level"] == "error"
    assert "message" in result
    assert "text" in result["message"]
    assert (
        "null" in result["message"]["text"].lower()
        or "nullptr" in result["message"]["text"].lower()
    )

    # Check location information
    assert "locations" in result
    assert len(result["locations"]) > 0
    location = result["locations"][0]
    assert "physicalLocation" in location
    assert "artifactLocation" in location["physicalLocation"]
    assert "region" in location["physicalLocation"]

    region = location["physicalLocation"]["region"]
    assert "startLine" in region
    assert region["startLine"] > 0
    assert "startColumn" in region
    assert region["startColumn"] > 0


def test_sarif_security_rules():
    """Test that security-related rules have proper security properties."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    # Check for security-related rules
    security_rule_ids = [
        "nullPointer",
        "arrayIndexOutOfBounds",
        "memleak",
        "uninitvar",
        "doubleFree",
    ]

    for rule_id in security_rule_ids:
        matching_rules = [r for r in rules if r["id"] == rule_id]
        if matching_rules:
            rule = matching_rules[0]
            props = rule.get("properties", {})

            # Security rules should have security-severity
            if "tags" in props and "security" in props["tags"]:
                assert "security-severity" in props
                assert float(props["security-severity"]) > 0

            # Should have problem.severity
            assert "problem.severity" in props

            # Should have precision
            assert "precision" in props


def test_sarif_rule_descriptions():
    """Test that rule descriptions are properly formatted."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    assert len(rules) > 0

    for rule in rules:
        # Each rule should have required fields
        assert "id" in rule
        assert "name" in rule
        assert "shortDescription" in rule
        assert "fullDescription" in rule

        # Descriptions should be empty (allowing GitHub to use instance messages)
        assert rule["name"] == ""
        assert rule["shortDescription"]["text"] == ""
        assert rule["fullDescription"]["text"] == ""

        # Should have properties
        assert "properties" in rule


def test_sarif_cwe_tags():
    """Test that CWE tags are properly formatted."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    # Find rules with CWE tags
    rules_with_cwe = []
    for rule in rules:
        props = rule.get("properties", {})
        tags = props.get("tags", [])
        cwe_tags = [t for t in tags if t.startswith("external/cwe/cwe-")]
        if cwe_tags:
            rules_with_cwe.append((rule["id"], cwe_tags))

    # Should have at least some rules with CWE tags
    assert len(rules_with_cwe) > 0

    # Validate CWE tag format
    for _, cwe_tags in rules_with_cwe:
        for tag in cwe_tags:
            assert tag.startswith("external/cwe/cwe-")
            cwe_num = tag[17:]  # After 'external/cwe/cwe-'
            assert cwe_num.isdigit()
            assert int(cwe_num) > 0


def test_sarif_severity_levels():
    """Test that different severity levels are properly mapped."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    results = run["results"]

    # Collect severity levels
    levels = set()
    for result in results:
        levels.add(result["level"])

    # Should have at least error level
    assert "error" in levels

    # Valid SARIF levels
    valid_levels = {"error", "warning", "note", "none"}
    for level in levels:
        assert level in valid_levels


def test_sarif_instance_specific_messages():
    """Test that result messages contain instance-specific information."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    results = run["results"]

    # Check that messages are instance-specific
    found_specific_messages = False

    for result in results:
        message_text = result["message"]["text"]
        rule_id = result["ruleId"]

        # Skip system include warnings
        if rule_id == "missingIncludeSystem":
            continue

        # Messages should not be empty
        assert len(message_text) > 0

        # Check for specific variable names or values from our test code
        if rule_id == "nullPointer" and "ptr" in message_text:
            found_specific_messages = True
        elif rule_id == "arrayIndexOutOfBounds" and (
            "array" in message_text or "10" in message_text
        ):
            found_specific_messages = True
        elif rule_id == "uninitvar" and "x" in message_text:
            found_specific_messages = True
        elif rule_id == "memleak" and "mem" in message_text:
            found_specific_messages = True
        elif rule_id == "doubleFree" and "p" in message_text:
            found_specific_messages = True

    assert (
        found_specific_messages
    ), "Should find at least some instance-specific messages"


def test_sarif_location_info():
    """Test that location information is correct."""
    sarif = run_sarif_check(BASIC_TEST_CODE)

    run = sarif["runs"][0]
    results = run["results"]

    for result in results:
        assert "locations" in result
        locations = result["locations"]
        assert len(locations) > 0

        for location in locations:
            assert "physicalLocation" in location
            phys_loc = location["physicalLocation"]

            assert "artifactLocation" in phys_loc
            assert "uri" in phys_loc["artifactLocation"]

            assert "region" in phys_loc
            region = phys_loc["region"]

            # SARIF requires line and column numbers >= 1
            assert "startLine" in region
            assert region["startLine"] >= 1

            assert "startColumn" in region
            assert region["startColumn"] >= 1


def test_sarif_with_multiple_files(tmp_path):
    """Test SARIF output with multiple source files."""
    # Create two test files
    file1_content = """
    void test1() {
        int* p = nullptr;
        *p = 1;
    }
    """

    file2_content = """
    void test2() {
        int arr[5];
        arr[10] = 2;
    }
    """

    file1 = tmp_path / "file1.cpp"
    file2 = tmp_path / "file2.cpp"
    file1.write_text(file1_content)
    file2.write_text(file2_content)

    args = ["--output-format=sarif", "--enable=all", str(tmp_path)]

    _, _, stderr = cppcheck(args)
    sarif = json.loads(stderr)

    run = sarif["runs"][0]
    results = run["results"]

    # Should have results from both files
    files = set()
    for result in results:
        if "locations" in result:
            for location in result["locations"]:
                uri = location["physicalLocation"]["artifactLocation"]["uri"]
                files.add(os.path.basename(uri))

    assert "file1.cpp" in files or "file2.cpp" in files


def test_sarif_default_configuration():
    """Test that rules have default configuration with level."""
    sarif = run_sarif_check(BASIC_TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    for rule in rules:
        # Check for defaultConfiguration.level (#13885)
        assert "defaultConfiguration" in rule
        assert "level" in rule["defaultConfiguration"]

        level = rule["defaultConfiguration"]["level"]
        assert level in ["error", "warning", "note", "none"]


def test_sarif_rule_properties():
    """Test that rules have required properties."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    for rule in rules:
        assert "properties" in rule
        props = rule["properties"]

        # Required properties
        assert "precision" in props
        assert props["precision"] in ["very-high", "high", "medium", "low"]

        assert "problem.severity" in props
        assert props["problem.severity"] in [
            "error",
            "warning",
            "style",
            "performance",
            "portability",
            "information",
            "note",
        ]


def test_sarif_rule_coverage():
    """Test that a variety of rules are triggered by comprehensive test code."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    # Collect all rule IDs
    rule_ids = set(rule["id"] for rule in rules)

    # Should have at least 5 different rules triggered
    assert (
        len(rule_ids) >= 5
    ), f"Expected at least 5 rules, found {len(rule_ids)}: {rule_ids}"

    # Check for some specific expected rules from different categories
    expected_rules = [
        "nullPointer",  # Security
        "arrayIndexOutOfBounds",  # Security
        "memleak",  # Security
        "uninitvar",  # Security
        "unusedVariable",  # Style
        "redundantAssignment",  # Style
        "unusedFunction",  # Style (if enabled)
        "constParameter",  # Style/Performance
        "cstyleCast",  # Style
        "variableScope",  # Style
    ]

    found_expected_rules = sum(1 for rule in expected_rules if rule in rule_ids)

    # Should find at least 3 of our expected rules
    assert (
        found_expected_rules >= 3
    ), f"Expected at least 3 known rules, found {found_expected_rules}"


def test_sarif_generic_descriptions():
    """Test that ALL rule descriptions are empty for GitHub integration."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    assert len(rules) > 0, "Should have at least one rule"

    # Verify that ALL rule descriptions are empty so GitHub uses instance-specific messages
    for rule in rules:
        rule_id = rule["id"]

        # All rules must have these fields
        assert "name" in rule, f"Rule {rule_id} missing 'name'"
        assert "shortDescription" in rule, f"Rule {rule_id} missing 'shortDescription'"
        assert "fullDescription" in rule, f"Rule {rule_id} missing 'fullDescription'"

        # The key test: ALL descriptions should be empty
        assert (
            rule["name"] == ""
        ), f"Rule {rule_id} name should be empty, got: {rule['name']}"
        assert (
            rule["shortDescription"]["text"] == ""
        ), f"Rule {rule_id} shortDescription should be empty, got: {rule['shortDescription']['text']}"
        assert (
            rule["fullDescription"]["text"] == ""
        ), f"Rule {rule_id} fullDescription should be empty, got: {rule['fullDescription']['text']}"


def test_sarif_security_rules_classification():
    """Test that security classification is correctly based on CWE IDs."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]

    found_rule_with_cwe = False
    found_rule_without_cwe = False

    for rule in rules:
        rule_id = rule["id"]
        props = rule.get("properties", {})
        tags = props.get("tags", [])

        # Check if rule has CWE tag
        cwe_tags = [t for t in tags if t.startswith("external/cwe/")]
        has_cwe = len(cwe_tags) > 0

        if has_cwe:
            found_rule_with_cwe = True

            # Rules with CWE should have security-severity and security tag
            assert (
                "security-severity" in props
            ), f"Rule {rule_id} with CWE should have security-severity"
            assert (
                float(props["security-severity"]) > 0
            ), f"Rule {rule_id} security-severity should be positive"

            # Check for security tag
            assert (
                "security" in tags
            ), f"Rule {rule_id} with CWE should have 'security' tag"
        else:
            found_rule_without_cwe = True

            # Rules without CWE should NOT have security-severity or security tag
            assert (
                "security-severity" not in props
            ), f"Rule {rule_id} without CWE should not have security-severity"
            assert (
                "security" not in tags
            ), f"Rule {rule_id} without CWE should not have 'security' tag"

        # All rules should still have basic properties
        assert "precision" in props, f"Rule {rule_id} missing 'precision'"
        assert "problem.severity" in props, f"Rule {rule_id} missing 'problem.severity'"

    # Should find at least some rules in test data
    assert (
        found_rule_with_cwe or found_rule_without_cwe
    ), "Should find at least some rules with or without CWE"


def test_sarif_results_consistency():
    """Test consistency between rule definitions and results."""
    sarif = run_sarif_check(TEST_CODE)

    run = sarif["runs"][0]
    driver = run["tool"]["driver"]
    rules = driver["rules"]
    results = run["results"]

    # Collect rule IDs from both rules and results
    rule_ids_in_rules = set(rule["id"] for rule in rules)
    rule_ids_in_results = set(result["ruleId"] for result in results)

    # Every rule ID in results should have a corresponding rule definition
    for result_rule_id in rule_ids_in_results:
        assert (
            result_rule_id in rule_ids_in_rules
        ), f"Result references undefined rule: {result_rule_id}"

    # Verify severity level consistency
    severity_levels = set()
    for result in results:
        level = result["level"]
        severity_levels.add(level)

        # Valid SARIF levels
        assert level in [
            "error",
            "warning",
            "note",
            "none",
        ], f"Invalid severity level: {level}"

    # Should have at least error level
    assert "error" in severity_levels, "Should have at least one error"

    # Should have multiple severity levels for comprehensive test
    assert (
        len(severity_levels) >= 2
    ), f"Expected multiple severity levels, found: {severity_levels}"
