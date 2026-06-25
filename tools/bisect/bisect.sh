#!/bin/sh

# TODO: set -e
set -x

# TODO: check parameters
hash_good=$1
hash_bad=$2
options=$3
expected=$4

# TODO: verify "good" commit happened before "bad" commit

# 0 - regular result based bisect
# 1 - find commit which started hang
# 2 - find commit which resolved hang
hang=0

script_dir="$(dirname "$(realpath "$0")")"

# TODO: make configurable
bisect_dir=~/.bisect

mkdir -p "$bisect_dir" || exit 1

cd "$bisect_dir" || exit 1

if [ ! -d 'cppcheck' ]; then
  git clone https://github.com/cppcheck-opensource/cppcheck.git || exit 1
fi

bisect_repo_dir="$bisect_dir/cppcheck"

cd $bisect_repo_dir || exit 1

git fetch --all --tags || exit 1

# clean up in case we previously exited prematurely
git restore . || exit 1
git clean -df || exit 1

# reset potentially unfinished bisect - also reverts to 'main' branch
git bisect reset || exit 1

# update `main` branch
git pull || exit 1

# TODO: filter addons, cfg and platforms based on the options
# limit to paths which actually affect the analysis
git bisect start -- Makefile 'addons/*.py' 'cfg/*.cfg' 'cli/*.cpp' 'cli/*.h' 'externals/**/*.cpp' 'externals/**/*.h' 'lib/*.cpp' 'lib/*.h' platforms tools/matchcompiler.py || exit 1

git checkout "$hash_good" || exit 1

if [ $hang -ne 0 ]; then
  # TODO: exitcode overflow on 255
  # get expected time from good commit
  python3 "$script_dir/bisect_hang.py" "$bisect_dir" "$options"
  elapsed_time=$?
else
  # verify the given commit is actually "good"
  python3 "$script_dir/bisect_res.py" "$bisect_dir" "$options" "$expected"
  # shellcheck disable=SC2181
  if [ $? -ne 0 ]; then
    echo "given good commit is not actually good"
    exit 1
  fi
fi

# mark commit as "good"
git bisect good || exit 1

git checkout "$hash_bad" || exit 1

# verify the given commit is actually "bad"
if [ $hang -ne 0 ]; then
  python3 "$script_dir/bisect_hang.py" "$bisect_dir" "$options" $elapsed_time $hang
else
  python3 "$script_dir/bisect_res.py" "$bisect_dir" "$options" "$expected"
fi

if [ $? -ne 1 ]; then
  echo "given bad commit is not actually bad"
  exit 1
fi

# mark commit as "bad"
git bisect bad || exit 1

# perform the actual bisect
if [ $hang -ne 0 ]; then
  git bisect run python3 "$script_dir/bisect_hang.py" "$bisect_dir" "$options" $elapsed_time $hang || exit 1
else
  git bisect run python3 "$script_dir/bisect_res.py" "$bisect_dir" "$options" "$expected" || exit 1
fi

# show the bisect log
git bisect log || exit 1

git bisect reset || exit 1