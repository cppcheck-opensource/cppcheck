# triage_version.py

A script to run a code sample against a given set of Cppcheck versions.

## Usage

```
usage: triage_version.py [-h] [--compare] [--verbose] [--debug] [--debug-warnings] [--check-library]
                         [--timeout TIMEOUT] [--compact] [--no-quiet] [--perf] [--start START]
                         [--no-stderr | --no-stdout]
                         dir infile [repo]

positional arguments:
  dir                directory with versioned folders
  infile             the file to analyze
  repo               the git repository (for sorting commit hashes)

options:
  -h, --help         show this help message and exit
  --compare          compare output and only show when changed
  --verbose          verbose output for debugging
  --debug            passed through to binary if supported
  --debug-warnings   passed through to binary if supported
  --check-library    passed through to binary if supported
  --timeout TIMEOUT  the amount of seconds to wait for the analysis to finish
  --compact          only print versions with changes with --compare
  --no-quiet         do not specify -q
  --perf             output duration of execution in seconds (CSV format)
  --start START      specify the start version/commit
  --no-stderr        do not display stdout
  --no-stdout        do not display stderr
```

### Structure of `dir`

It expects the given `dir` to contain folders which are either named after version numbers or commit hashes. These folders must each contain a `cppcheck` and the associated files for that version.

If the first folder is not a valid version the names are interpreted as commit hashes. These are internally sorted but that requires the `repo` parameter to be set.

It is not possible to mix versions and commit hashes.