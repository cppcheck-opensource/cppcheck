#!/bin/sh

cmake_output="$1"
selfcheck_options_extra="$2"

selfcheck_options="-q -j$(nproc) --std=c++11 --template=selfcheck --showtime=file-total -D__GNUC__ --error-exitcode=1 --inline-suppr --suppressions-list=.selfcheck_suppressions --library=gnu --inconclusive --enable=style,performance,portability,warning,missingInclude,information --exception-handling --debug-warnings --check-level=exhaustive"
selfcheck_options="$selfcheck_options $selfcheck_options_extra"
cppcheck_options="-D__CPPCHECK__ -DCHECK_INTERNAL -DHAVE_RULES --library=cppcheck-lib -Ilib -Iexternals/simplecpp/ -Iexternals/tinyxml2"
qt_options="--library=qt -DQT_VERSION=0x060000 -DQ_MOC_OUTPUT_REVISION=69 -DQT_MOC_HAS_STRINGDATA"
qt_options="$qt_options --suppress=autoNoType:*/moc_*.cpp --suppress=symbolDatabaseWarning:*/moc_*.cpp"
naming_options="--addon-python=$(command -v python) --addon=naming.json"

# clear PATH to prevent unintentional process invocations
export PATH=

ec=0

$cmake_output/bin/cppcheck $selfcheck_options \
  externals \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options $naming_options \
  frontend \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options $naming_options \
  -Ifrontend \
  cli \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options $naming_options \
  --enable=internal \
  lib \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options $naming_options $qt_options \
  --suppress=constVariablePointer:*/moc_*.cpp \
  -DQT_CHARTS_LIB \
  -I$cmake_output/gui -Ifrontend -Igui \
  gui/*.cpp $cmake_output/gui \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options \
  -Icli -Ifrontend \
  test/*.cpp \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options \
  -Icli \
  tools/dmake/*.cpp \
  || ec=1

$cmake_output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  -I$cmake_output/tools/triage -Igui \
  tools/triage/*.cpp $cmake_output/tools/triage \
  || ec=1

exit $ec