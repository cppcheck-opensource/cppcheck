#!/bin/sh

cmake_output=cmake.output
selfcheck_options_extra="$1"

cppcheck_bin=./cppcheck

selfcheck_options="-q -j$(nproc) --std=c++11 --template=selfcheck --showtime=file-total -D__GNUC__ --error-exitcode=1 --inline-suppr --suppressions-list=.selfcheck_suppressions --library=gnu --inconclusive --enable=style,performance,portability,warning,missingInclude,information --exception-handling --debug-warnings --check-level=exhaustive"
selfcheck_options="$selfcheck_options $selfcheck_options_extra"
cppcheck_options="-D__CPPCHECK__ -DCHECK_INTERNAL -DHAVE_RULES --library=cppcheck-lib -Ilib -Iexternals/simplecpp/ -Iexternals/tinyxml2"
qt_options="--library=qt -DQT_VERSION=0x060000 -DQ_MOC_OUTPUT_REVISION=68 -DQT_MOC_HAS_STRINGDATA"  # TODO: use 69 as revision
naming_options="--addon-python=$(command -v python) --addon=naming.json"

mkdir_cmd=$(command -v mkdir)
rm_cmd=$(command -v rm)

# clear PATH to prevent unintentional process invocations
export PATH=

ec=0

$cppcheck_bin $selfcheck_options \
  externals \
  || ec=1

$mkdir_cmd b1

$cppcheck_bin $selfcheck_options $cppcheck_options $naming_options \
  --cppcheck-build-dir=b1 \
  frontend \
  || ec=1

$cppcheck_bin $selfcheck_options $cppcheck_options $naming_options \
  --cppcheck-build-dir=b1 \
  -Ifrontend \
  cli \
  || ec=1

$cppcheck_bin $selfcheck_options $cppcheck_options $naming_options \
  --cppcheck-build-dir=b1 --enable=internal \
  lib \
  || ec=1

$mkdir_cmd b2

$cppcheck_bin $selfcheck_options $cppcheck_options $naming_options $qt_options \
  --cppcheck-build-dir=b2 \
  -DQT_CHARTS_LIB \
  -I$cmake_output/gui -Ifrontend -Igui \
  gui/*.cpp $cmake_output/gui \
  || ec=1

$cppcheck_bin $selfcheck_options $cppcheck_options \
  -Ifrontend -Icli \
  test/*.cpp \
  || ec=1

$cppcheck_bin $selfcheck_options $cppcheck_options \
  -Icli \
  tools/dmake/*.cpp \
  || ec=1

$cppcheck_bin $selfcheck_options $cppcheck_options $qt_options \
  -I$cmake_output/tools/triage -Igui \
  tools/triage/*.cpp $cmake_output/tools/triage \
  || ec=1

$rm_cmd -rf b2
$rm_cmd -rf b1

exit $ec