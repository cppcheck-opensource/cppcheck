#!/bin/sh

selfcheck_options="-q -j$(nproc) --std=c++11 --template=selfcheck --showtime=file-total -D__GNUC__ --error-exitcode=1 --inline-suppr --suppressions-list=.selfcheck_suppressions --library=gnu --inconclusive --enable=style,performance,portability,warning,missingInclude,information --exception-handling --debug-warnings --check-level=exhaustive"
cppcheck_options="-D__CPPCHECK__ -DCHECK_INTERNAL -DHAVE_RULES --library=cppcheck-lib -Ilib -Iexternals/simplecpp/ -Iexternals/tinyxml2"
gui_options="-DQT_VERSION=0x060000 -DQ_MOC_OUTPUT_REVISION=68 -DQT_CHARTS_LIB -DQT_MOC_HAS_STRINGDATA --library=qt"
naming_options="--addon-python=$(command -v python) --addon=naming.json"

if [ -n "$1" ]; then
  selfcheck_options="$selfcheck_options $1"
fi

mkdir_cmd=$(command -v mkdir)
rm_cmd=$(command -v rm)

# clear PATH to prevent unintentional process invocations
export PATH=

ec=0

# check gui with qt settings
$mkdir_cmd b2
./cppcheck $selfcheck_options $cppcheck_options $gui_options --cppcheck-build-dir=b2 $naming_options -Icmake.output/gui -Ifrontend -Igui -igui/test/data gui cmake.output/gui || ec=1


$rm_cmd -rf b2

exit $ec