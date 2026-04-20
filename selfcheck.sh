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

# self check externals
./cppcheck $selfcheck_options externals || ec=1
# self check lib/cli
$mkdir_cmd b1
./cppcheck $selfcheck_options $cppcheck_options --cppcheck-build-dir=b1 $naming_options frontend || ec=1
./cppcheck $selfcheck_options $cppcheck_options --cppcheck-build-dir=b1 $naming_options -Ifrontend cli || ec=1
./cppcheck $selfcheck_options $cppcheck_options --cppcheck-build-dir=b1 $naming_options --enable=internal lib || ec=1
# check gui with qt settings
$mkdir_cmd b2
./cppcheck $selfcheck_options $cppcheck_options $gui_options --cppcheck-build-dir=b2 $naming_options -Icmake.output/gui -Ifrontend -Igui gui/*.cpp cmake.output/gui || ec=1
# self check test and tools
./cppcheck $selfcheck_options $cppcheck_options -Ifrontend -Icli test/*.cpp || ec=1
./cppcheck $selfcheck_options $cppcheck_options -Icli tools/dmake/*.cpp || ec=1
# triage
./cppcheck $selfcheck_options $cppcheck_options $gui_options -Icmake.output/tools/triage -Igui tools/triage/*.cpp cmake.output/tools/triage || ec=1

$rm_cmd -rf b2
$rm_cmd -rf b1

exit $ec