
# python -m pytest vcxproj_item_macro_path_hyphen_test.py
#
# Regression coverage for hasOnlyInvariantItemPathVariables()'s inline
# property-name scanner rejecting '-' (lib/importproject.cpp). Real MSBuild
# property names are [A-Za-z_][A-Za-z0-9_-]* -- '-' IS valid after the
# first character, e.g. <My-Root>HyphenDir</My-Root> referenced as
# $(My-Root) -- but the scanner used to decide whether a ClCompile item
# path's macro references are all safely config-invariant only accepted
# [A-Za-z0-9_], so it stopped at the first '-' and captured a truncated
# name. Since the character right after that truncated name was '-' (not
# the required closing ')'), the whole reference was rejected as "not
# provably invariant" and left unexpanded, even though this property is
# only ever set once, to one value -- genuinely invariant, exactly like
# vcxproj_item_macro_path_invariant_test.py's ExternalLibRoot. See
# vcxproj_item_macro_path_hyphen/vcxproj_item_macro_path_hyphen.vcxproj.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_item_macro_path_hyphen():
    args = [
        '--project=vcxproj_item_macro_path_hyphen/vcxproj_item_macro_path_hyphen.vcxproj',
        '--no-cppcheck-build-dir',
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    normalized_stdout = stdout.replace('\\', '/')
    filename = 'vcxproj_item_macro_path_hyphen/HyphenDir/foo.cpp'

    # $(My-Root) must resolve to "HyphenDir" and be expanded -- reaching and
    # checking the real file, not being left as the literal, un-expandable
    # "$(My-Root)\foo.cpp" text.
    assert ('Checking %s Debug|x64...' % filename) in normalized_stdout, stdout

    normalized_stderr = stderr.replace('\\', '/')
    assert normalized_stderr.count('%s:8:15: error: Division by zero.' % filename) == 1, stderr

    # It must never be reported as a missing file under the literal,
    # unexpanded macro text -- that would mean the hyphen truncated the
    # scanned property name and the reference was wrongly rejected as
    # "not provably invariant".
    assert '$(My-Root)' not in normalized_stdout, stdout
    assert '$(My-Root)' not in normalized_stderr, stderr
