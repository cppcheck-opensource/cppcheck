
# python -m pytest vcxproj_split_configs_test.py
#
# Regression coverage for the vcxproj configuration-discovery pass (used only
# when a project has no inline <ItemGroup Label="ProjectConfigurations"> and
# must find its configurations by walking imports): it must not stop scanning
# as soon as it finds the first configuration. Real MSBuild/Visual Studio has
# to know the complete configuration set before evaluation with a specific
# Configuration/Platform pair is even possible, so its own preliminary parse
# does not stop early either -- configurations can legitimately be split
# across multiple imports.
#
# This fixture is deliberately ordered so a naive "stop at the first
# configuration found" discovery pass fails it: ConfigsRelease.props is
# unconditional and comes first, so it is found regardless; ConfigsDebug.props
# is reachable only through an ImportGroup gated on Configuration=='Debug',
# positioned AFTER it. A discovery pass that stops once Release|x64 is found
# never reaches that second import at all -- and simply re-running the real
# per-configuration evaluation pass for Release|x64 does not rescue it either,
# since that pass evaluates with Configuration=Release, so the Debug-gated
# ImportGroup's Condition is false during it too. Only a discovery pass that
# keeps scanning (using its own guessed Configuration, seeded before the scan
# starts) reaches and evaluates the second import and finds Debug|x64 as well.

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_vcxproj_split_configs():
    args = [
        '--project=vcxproj_split_configs/vcxproj_split_configs.vcxproj',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout

    # Windows prints native '\' path separators ("Checking foo\main.cpp ...");
    # normalize before matching so this passes on every platform (same idiom
    # used by test_log() in clang-import_test.py).
    normalized_stdout = stdout.replace('\\', '/')

    # Both configurations must be discovered and checked, not just whichever
    # one the (possibly early-stopping) discovery pass happens to find first.
    assert 'Checking vcxproj_split_configs/main.cpp Release|x64' in normalized_stdout, stdout
    assert 'Checking vcxproj_split_configs/main.cpp Debug|x64' in normalized_stdout, stdout
