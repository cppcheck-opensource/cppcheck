# Helpers for pytest tests
import subprocess
import json
import os


def find_cppcheck_binary():
    possible_locations = [
        "./cppcheck",
        "./build/bin/cppcheck",
        r".\bin\cppcheck.exe",
    ]
    for location in possible_locations:
        if os.path.exists(location):
            break
    else:
        raise RuntimeError("Could not find cppcheck binary")

    return location

def dump_create(fpath, *argv):
    cppcheck_binary = find_cppcheck_binary()
    cmd = [cppcheck_binary, "--dump", "-DDUMMY", "--quiet", fpath] + list(argv)
    with subprocess.Popen(cmd) as p:
        p.communicate()
        if p.returncode != 0:
            raise OSError("cppcheck returns error code: %d" % p.returncode)


def dump_remove(fpath):
    os.remove(fpath + ".dump")


def convert_json_output(raw_json_strings):
    """Convert raw stdout/stderr cppcheck JSON output to python dict."""
    json_output = {}
    for line in raw_json_strings:
        if line.startswith('{"summary":'):
            continue
        try:
            json_line = json.loads(line)
            #  json_output[json_line['errorId']] = json_line
            json_output.setdefault(json_line['errorId'], []).append(json_line)
        except ValueError:
            pass
    return json_output
