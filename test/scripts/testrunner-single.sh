#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -n "$1" ]; then
  testrunner_bin=$1
else
  make -s -C "$SCRIPT_DIR/../.." -j"$(nproc)" testrunner # CXXOPTS="-g -O2 -w" CPPOPTS="-DHAVE_BOOST"
  testrunner_bin=$SCRIPT_DIR/../../testrunner
fi

ec=0

tests=$($testrunner_bin -d | cut -d'(' -f2 | cut -d')' -f1)
for test in $tests; do
  $testrunner_bin -n "$test" || ec=1
done

exit $ec
