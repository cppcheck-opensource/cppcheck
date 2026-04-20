#!/bin/bash

#set -x

SCRIPT_DIR="$(dirname "$(realpath "$0")")"

# TODO: exclude testinternal.cpp
echo 'no --errorlist entry:'
grep -h -o -P '\[[a-zA-Z0-9_]+\]\\n\"' $SCRIPT_DIR/../test/*.cpp | tr -d '[]\"' | sed 's/\\n$//' | sort -u | \
while read -r id; do
  if [ ${#id} -lt 4 ]; then
    continue
  fi
  $SCRIPT_DIR/../cppcheck --errorlist | grep "id=\"$id\"" > /dev/null
  # shellcheck disable=SC2181
  if [ $? -ne 0 ]; then
    echo $id
  fi
done

echo ''

echo 'no test coverage:'
$SCRIPT_DIR/../cppcheck --errorlist | grep -h -o -P 'id=\"[a-zA-Z0-9_]*\"' | sed 's/\id=//' | tr -d '\"' | sort -u | \
while read -r id; do
  grep -h -o -P "\[$id\][\\\\n]*\"" $SCRIPT_DIR/../test/*.cpp > /dev/null
  # shellcheck disable=SC2181
  if [ $? -ne 0 ]; then
    echo $id
  fi
done