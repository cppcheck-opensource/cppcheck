#!/bin/bash

cd ~/cppcheck || exit 1
rm -rf test1
python tools/extracttests.py --code=test1 test/testleakautovar.cpp
cd ~/cppcheck/test1 || exit 1
~/cppcheck/test/scripts/run_more_tests.sh

