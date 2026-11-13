
# python -m pytest vcxproj_metadata_self_reference_case_test.py
#
# Regression coverage for addMetadata()'s "$(eName) self-reference"
# accumulation idiom (lib/importproject.cpp) using plain, case-sensitive
# findAndReplace() instead of findAndReplaceCaseInsensitive() -- the same
# function already used everywhere else in this file for property/metadata
# name comparisons, and MetadataMap itself is already case-insensitive
# (cppcheck::stricmp). Real MSBuild item metadata names are case-insensitive,
# so a self-reference like $(preprocessordefinitions) inside a
# <PreprocessorDefinitions> element must resolve regardless of casing -- see
# vcxproj_metadata_self_reference_case/vcxproj_metadata_self_reference_case.vcxproj,
# which accumulates PreprocessorDefinitions across two ItemDefinitionGroup
# blocks, the second referencing the first back via a differently-cased
# $(preprocessordefinitions).

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_metadata_self_reference_case():
    args = [
        '--project=vcxproj_metadata_self_reference_case/vcxproj_metadata_self_reference_case.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    filename = 'vcxproj_metadata_self_reference_case/foo.cpp'

    assert ('Checking %s Debug|x64...' % filename) in normalized_stdout, stdout

    normalized_stderr = stderr.replace('\\', '/')

    # Reached only if BOTH BASE_DEFINE (accumulated from the first
    # ItemDefinitionGroup, via the case-mismatched self-reference) and
    # EXTRA_DEFINE ended up defined.
    assert normalized_stderr.count('%s:13:15: error: Division by zero.' % filename) == 1, stderr
