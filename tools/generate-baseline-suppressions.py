#!/usr/bin/env python3
"""Convert a cppcheck XML results file into a cppcheck XML suppressions file.

Usage: generate-baseline-suppressions.py <results.xml> <suppressions.xml>

Each <error> in the results file that has a non-zero "hash" attribute is
converted into a <suppress> entry using the error's id, the file of its
first <location> (the primary location used by cppcheck's suppression
matching) and the hash.
"""
import sys
import xml.etree.ElementTree as ET


def convert(results_path, suppressions_path):
    tree = ET.parse(results_path)
    root = tree.getroot()
    errors_el = root.find('errors')

    seen = set()
    suppressions = []
    for error in errors_el.findall('error'):
        error_id = error.get('id')
        error_hash = error.get('hash')
        location = error.find('location')
        if error_id is None or location is None:
            continue
        if not error_hash or error_hash == '0':
            continue
        file_name = location.get('file')
        key = (error_id, file_name, error_hash)
        if key in seen:
            continue
        seen.add(key)
        suppressions.append(key)

    out_root = ET.Element('suppressions')
    for error_id, file_name, error_hash in suppressions:
        suppress = ET.SubElement(out_root, 'suppress')
        ET.SubElement(suppress, 'id').text = error_id
        ET.SubElement(suppress, 'fileName').text = file_name
        ET.SubElement(suppress, 'hash').text = error_hash

    ET.indent(out_root, space='  ')
    out_tree = ET.ElementTree(out_root)
    out_tree.write(suppressions_path, encoding='UTF-8', xml_declaration=True)
    print(f'Wrote {len(suppressions)} suppression(s) to {suppressions_path}')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} <results.xml> <suppressions.xml>', file=sys.stderr)
        sys.exit(1)
    convert(sys.argv[1], sys.argv[2])
