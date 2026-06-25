#!/usr/bin/env python3
from __future__ import print_function
import argparse
import xml.etree.ElementTree as ET


def main():
    parser = argparse.ArgumentParser(description="List all error without a CWE assigned in CSV format")
    parser.add_argument("-F", metavar="filename", required=True,
                        help="XML file containing output from: ./cppcheck --errorlist --xml-version=2")
    parsed = parser.parse_args()

    tree = ET.parse(vars(parsed)["F"])
    root = tree.getroot()
    for child in root.iter("error"):
        if "cwe" not in child.attrib:
            print(child.attrib["id"], child.attrib["severity"], child.attrib["verbose"], sep=", ")

if __name__ == "__main__":
    main()
