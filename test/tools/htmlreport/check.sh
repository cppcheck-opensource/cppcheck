#!/bin/bash -ex

# Command for checking HTML syntax with HTML Tidy, see http://www.html-tidy.org/
tidy_cmd='tidy -o /dev/null -eq --drop-empty-elements no'

function validate_html {
    if [ ! -f "$1" ]; then
        echo "File $1 does not exist!"
	      exit 1
    fi
    if ! ${tidy_cmd} "$1"; then
        echo "HTML validation failed!"
        exit 1
    fi
}

if [ -z "$PYTHON" ]; then
    PYTHON=python3
fi

SCRIPT_DIR="$(dirname ${BASH_SOURCE[0]})"
ABSOLUTE_SCRIPT_DIR="$(cd -- ${SCRIPT_DIR} ; pwd -P)"
PROJECT_ROOT_DIR="$(cd -- ${SCRIPT_DIR}/../../../ ; pwd -P)"
REPORT_DIR=$(mktemp -d -t htmlreport-XXXXXXXXXX)
INDEX_HTML="$REPORT_DIR/index.html"
STATS_HTML="$REPORT_DIR/stats.html"
GUI_TEST_XML="$REPORT_DIR/gui_test.xml"
ERRORLIST_XML="$REPORT_DIR/errorlist.xml"
UNMATCHEDSUPPR_XML="$REPORT_DIR/unmatchedSuppr.xml"

$PYTHON ${PROJECT_ROOT_DIR}/htmlreport/cppcheck-htmlreport --file ${PROJECT_ROOT_DIR}/gui/test/data/xmlfiles/xmlreport_v2.xml --title "xml2 test" --report-dir "$REPORT_DIR" --source-dir ${PROJECT_ROOT_DIR}/test/
echo -e "\n"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


${PROJECT_ROOT_DIR}/cppcheck ${PROJECT_ROOT_DIR}/samples --enable=all --inconclusive --xml-version=2 2> "$GUI_TEST_XML"
xmllint --noout "$GUI_TEST_XML"
$PYTHON ${PROJECT_ROOT_DIR}/htmlreport/cppcheck-htmlreport --file "$GUI_TEST_XML" --title "xml2 + inconclusive test" --report-dir "$REPORT_DIR"
echo ""
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


${PROJECT_ROOT_DIR}/cppcheck ${PROJECT_ROOT_DIR}/samples --enable=all --inconclusive --verbose --xml-version=2 2> "$GUI_TEST_XML"
xmllint --noout "$GUI_TEST_XML"
$PYTHON ${PROJECT_ROOT_DIR}/htmlreport/cppcheck-htmlreport --file "$GUI_TEST_XML" --title "xml2 + inconclusive + verbose test" --report-dir "$REPORT_DIR"
echo -e "\n"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


${PROJECT_ROOT_DIR}/cppcheck --errorlist --inconclusive --xml-version=2 > "$ERRORLIST_XML"
xmllint --noout "$ERRORLIST_XML"
$PYTHON ${PROJECT_ROOT_DIR}/htmlreport/cppcheck-htmlreport --file "$ERRORLIST_XML" --title "errorlist" --report-dir "$REPORT_DIR"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


${PROJECT_ROOT_DIR}/cppcheck ${PROJECT_ROOT_DIR}/samples/memleak/good.c ${PROJECT_ROOT_DIR}/samples/resourceLeak/good.c  --xml-version=2 --enable=information --suppressions-list="${ABSOLUTE_SCRIPT_DIR}/test_suppressions.txt" --xml 2> "$UNMATCHEDSUPPR_XML"
xmllint --noout "$UNMATCHEDSUPPR_XML"
$PYTHON ${PROJECT_ROOT_DIR}/htmlreport/cppcheck-htmlreport --file "$UNMATCHEDSUPPR_XML" --title "unmatched Suppressions" --report-dir="$REPORT_DIR"
grep "unmatchedSuppression<.*>information<.*>Unmatched suppression: variableScope*<" "$INDEX_HTML"
grep ">unmatchedSuppression</.*>information<.*>Unmatched suppression: uninitstring<" "$INDEX_HTML"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"

rm -rf "$REPORT_DIR"
