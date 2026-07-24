#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/
LIBDIR="$DIR"../../lib

ec=0
gcc -c constness_ptr_test.cpp -Wall -Wextra -I$LIBDIR || ec=1
gcc -c constness_ptr_test.cpp -Wall -Wextra -I$LIBDIR -DBAD && ec=1
exit $ec