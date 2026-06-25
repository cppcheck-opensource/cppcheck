#!/usr/bin/env python3
"""Test cppcheck-htmlreport."""

import os
import contextlib
import subprocess
import sys
import tempfile

import unittest
import time

TEST_TOOLS_DIR = os.path.abspath(os.path.dirname(__file__))
ROOT_DIR = os.path.split(os.path.dirname(os.path.dirname(TEST_TOOLS_DIR)))[0]
HTMLREPORT_DIR = os.path.join(ROOT_DIR, 'htmlreport')
CPPCHECK_BIN = os.path.join(ROOT_DIR, 'cppcheck')

if os.getenv("PIP_PACKAGE_TEST") is not None:
    HTML_REPORT_BIN = ['cppcheck-htmlreport']
else:
    HTML_REPORT_BIN = [sys.executable, os.path.join(HTMLREPORT_DIR, 'cppcheck-htmlreport')]


class TestHTMLReport(unittest.TestCase):

    def testReportError(self):
        for xml_version in ['2']:
            self.checkReportError(xml_version)

    def checkReportError(self, xml_version):
        with runCheck(
            os.path.join(ROOT_DIR, 'samples', 'memleak', 'bad.c'),
            xml_version=xml_version
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertIn('Memory leak:', report)
            self.assertIn('bad.c', report)

            detail_filename = os.path.join(output_directory.name, '0.html')
            self.assertTrue(
                os.path.exists(detail_filename))

            with open(detail_filename) as input_file:
                detail_contents = input_file.read()
                self.assertIn('<html', detail_contents)
                self.assertIn('Memory leak:', detail_contents)

            output_directory.cleanup()

    def testReportNoError(self):
        for xml_version in ['2']:
            self.checkReportNoError(xml_version)

    def checkReportNoError(self, xml_version):
        with runCheck(
            os.path.join(ROOT_DIR, 'samples', 'memleak', 'good.c'),
            xml_version=xml_version
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertNotIn('Memory leak:', report)
            self.assertNotIn('good.c', report)

            self.assertFalse(
                os.path.exists(os.path.join(output_directory.name, '0.html')))

            output_directory.cleanup()

    def testMissingInclude(self):
        with runCheck(
            xml_filename=os.path.join(TEST_TOOLS_DIR, 'example.xml'),
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertIn('Uninitialized variable:', report)
            self.assertIn('example.cc', report)

            self.assertTrue(
                os.path.exists(os.path.join(output_directory.name, '0.html')))

            output_directory.cleanup()

    def testAddCheckersReport(self):
        with runCheck(
            xml_filename=os.path.join(TEST_TOOLS_DIR, 'example.xml'),
            checkers_filename=os.path.join(TEST_TOOLS_DIR, 'example-checkers.txt')
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertIn('<a href="checkers.html"', report)

            self.assertTrue(
                os.path.exists(os.path.join(output_directory.name, 'checkers.html')))

            output_directory.cleanup()

    def testAddTimestamp(self):
        with runCheck(
            xml_filename=os.path.join(TEST_TOOLS_DIR, 'example.xml'),
        ) as (report, output_directory):
            xml_file = os.path.join(TEST_TOOLS_DIR, 'example.xml')
            t = os.path.getmtime(xml_file)
            t_s = time.ctime(t)

            self.assertIn(t_s, report)

            output_directory.cleanup()

    def testSeverityFilterBar(self):
        with runCheck(
            xml_filename=os.path.join(TEST_TOOLS_DIR, 'example.xml'),
        ) as (report, output_directory):
            self.assertIn('onclick="toggleSeverity(this)"', report)
            output_directory.cleanup()


@contextlib.contextmanager
def runCheck(source_filename=None, xml_version='1', xml_filename=None, checkers_filename=None):
    """Run cppcheck and cppcheck-htmlreport.

    Yield a tuple containing the resulting HTML report index and the directory
    path.

    """
    output_directory = tempfile.TemporaryDirectory(dir='.')
    if xml_filename is None:
        assert source_filename
        xml_filename = os.path.join(output_directory.name, 'output.xml')

        with open(xml_filename, 'w') as output_file:
            subprocess.check_call(
                [CPPCHECK_BIN, '--xml', source_filename,
                 '--xml-version=' + xml_version],
                stderr=output_file)

    assert os.path.exists(xml_filename)

    args = [*HTML_REPORT_BIN,
         '--file=' + os.path.realpath(xml_filename),
         '--report-dir=' + os.path.realpath(output_directory.name)]
    if checkers_filename:
        args.append('--checkers-report-file=' + os.path.realpath(checkers_filename))

    subprocess.check_call(
        args,
        cwd=TEST_TOOLS_DIR)

    with open(os.path.join(output_directory.name, 'index.html')) as index_file:
        index_contents = index_file.read()

    yield index_contents, output_directory


if __name__ == '__main__':
    unittest.main()
