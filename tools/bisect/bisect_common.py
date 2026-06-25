#!/usr/bin/env python
import subprocess
import os
import shutil

EC_GOOD = 0 # tells bisect that the commit is "good"
EC_BAD = 1 # tells bisect that the commit is "bad"
EC_SKIP = 125 # tells bisect to skip this commit since it cannot be tested

def build_cppcheck(bisect_path):
    commit_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()
    install_path = os.path.join(bisect_path, commit_hash)
    cppcheck_path = os.path.join(install_path, 'cppcheck')
    if os.path.exists(install_path):
        print('binary for {} already exists'.format(commit_hash))
        return cppcheck_path

    bisect_repo_dir = os.path.join(bisect_path, 'cppcheck')

    if os.path.exists(os.path.join(bisect_repo_dir, 'cppcheck')):
        os.remove(os.path.join(bisect_repo_dir, 'cppcheck'))

    # for versions 1.88 and 1.89
    print('patching Makefile')
    subprocess.check_call(['sed', '-i', 's/shell python /shell python3 /g', os.path.join(bisect_repo_dir, 'Makefile')])

    # for versions between 2.0 and 2.2
    print('patching cli/cppcheckexecutor.cpp')
    subprocess.check_call(['sed', '-i', 's/SIGSTKSZ/32768/g', os.path.join(bisect_repo_dir, 'cli', 'cppcheckexecutor.cpp')])

    # TODO: older versions do not build because of include changes in libstdc++ - check compiler version and try to use an earlier one
    # TODO: make jobs configurable
    # TODO: use "make install"?
    # TODO: use CXXOPTS overrides to workaround compiling issues in older versions
    print('building {}'.format(commit_hash))
    # we always need to use CXXFLAGS because we need to support older versions
    subprocess.check_call(['make', '-C', bisect_repo_dir, '-j6', 'MATCHCOMPILER=yes', 'CXXFLAGS=-O2 -w -pipe', '-s'])

    # TODO: remove folder if installation failed
    print('installing {}'.format(commit_hash))
    os.mkdir(install_path)
    if os.path.exists(os.path.join(bisect_repo_dir, 'cfg')):
        shutil.copytree(os.path.join(bisect_repo_dir, 'cfg'), os.path.join(install_path, 'cfg'))
    if os.path.exists(os.path.join(bisect_repo_dir, 'platforms')):
        shutil.copytree(os.path.join(bisect_repo_dir, 'platforms'), os.path.join(install_path, 'platforms'))
    shutil.copy(os.path.join(bisect_repo_dir, 'cppcheck'), cppcheck_path)

    # reset the patches so the subsequent checkout works
    print('resetting repo')
    subprocess.check_call(['git', 'reset', '--hard'])

    return cppcheck_path