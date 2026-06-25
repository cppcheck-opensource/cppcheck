#!/bin/bash
# Script Used by generate_and_run_more_tests.sh

# "gsed" is a GNU compatible version of "sed" on MacOS
SED_CMD=$(command -v gsed || command -v sed)

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

CPPCHECK=$(pwd)/cppcheck

if test -f ./bin/cppcheck; then
    CPPCHECK=$(pwd)/bin/cppcheck
fi

python3 $DIR/extracttests.py --code=$(pwd)/test1 $1

cd test1

# ensure there are not critical errors in extracted tests
$CPPCHECK -q --safety --suppress="*" .

$CPPCHECK -q --template=cppcheck1 . 2> 1.txt

echo "(!x) => (x==0)"
$SED_CMD -ri 's/([(&][ ]*)\!([a-z]+)([ ]*[&)])/\1\2==0\3/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x==0) => (0==x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*==[ ]*0([ ]*[&)])/\10==\2\3/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(0==x) => (!x)"
$SED_CMD -ri 's/([(&][ ]*)0[ ]*==[ ]*([a-z]+)([ ]*[&)])/\1!\2\3/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt




echo "if (x) => (x!=0)"
$SED_CMD -ri 's/(if[ ]*\([ ]*[a-z]+)([ ]*[&)])/\1!=0\2/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "while (x) => (x!=0)"
$SED_CMD -ri 's/(while[ ]*\([ ]*[a-z]+)([ ]*[&)])/\1!=0\2/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x!=0) => (0!=x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*!=[ ]*0([ ]*[&)])/\10!=\2\3/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(0!=x) => (x)"
$SED_CMD -ri 's/([(&][ ]*)0[ ]*!=[ ]*([a-z]+[ ]*[&)])/\1\2/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt


echo "(x < 0) => (0 > x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*<[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x <= 0) => (0 >= x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*<=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x > 0) => (0 < x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*<=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x >= 0) => (0 <= x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*<=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x == 123) => (123 == x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*==[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3==\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(x != 123) => (123 != x)"
$SED_CMD -ri 's/([(&][ ]*)([a-z]+)[ ]*\!=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3!=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt



echo "(0 < x) => (x > 0)"
$SED_CMD -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<[ ]*([a-z]+)([ ]*[&)])/\1\3>\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(0 <= x) => (x >= 0)"
$SED_CMD -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<=[ ]*([a-z]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(0 > x) => (x < 0)"
$SED_CMD -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<=[ ]*([a-z]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(0 >= x) => (x <= 0)"
$SED_CMD -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<=[ ]*([a-z]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(123 == x) => (x == 123)"
$SED_CMD -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*==[ ]*([a-z]+)([ ]*[&)])/\1\3==\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt

echo "(123 != x) => (x <= 123)"
$SED_CMD -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*\!=[ ]*([a-z]+)([ ]*[&)])/\1\3!=\2\4/' *.cpp
$CPPCHECK -q --template=cppcheck1 . 2> 2.txt && diff 1.txt 2.txt


cd ..

rm -rf test1
