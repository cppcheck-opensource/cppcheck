import os
import sys

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__root_dir = os.path.abspath(os.path.join(__script_dir, '..', '..'))


def test_samples():
    failures = {}

    samples_dir = os.path.join(__root_dir, 'samples')
    for entry in os.listdir(samples_dir):
        sample_dir = os.path.join(samples_dir, entry)
        if not os.path.isdir(sample_dir):
            continue

        with open(os.path.join(sample_dir, 'out.txt')) as out_in:
            out_txt = out_in.read()
        if sys.platform != 'win32':
            out_txt = out_txt.replace('\\', '/')

        if not os.path.exists(os.path.join(sample_dir, 'good.c')):
            good_src = os.path.join('samples', entry, 'good.cpp')
            bad_src = os.path.join('samples', entry, 'bad.cpp')
        else:
            good_src = os.path.join('samples', entry, 'good.c')
            bad_src = os.path.join('samples', entry, 'bad.c')

        # check that good input does not produce any warnings
        ret, _, stderr = cppcheck(['-q', '--enable=all', '--disable=missingInclude', '--inconclusive', '--check-level=exhaustive', '--error-exitcode=1', good_src], cwd=__root_dir)
        if ret != 0:
            failures[good_src] = stderr

        # check that the bad inout produces a warning
        ret, _, stderr = cppcheck(['-q', '--enable=all', '--disable=missingInclude', '--inconclusive', '--check-level=exhaustive', '--error-exitcode=1', bad_src], cwd=__root_dir)
        if ret != 1:
            failures[bad_src] = stderr

        # check that the bad input procudes the expected output
        if not stderr == out_txt:
            failures[bad_src] = stderr

    assert failures == {}
