#!/bin/sh
# Usage:
#   ./build-html.sh NAME                  # NAME.md -> NAME.html
#   ./build-html.sh NAME INPUT... [pandoc options]
#                                         # INPUT... -> NAME.html
name=$1
shift
if [ $# -eq 0 ]; then
    set -- "$name.md"
fi
echo Building $name.html
pandoc "$@" -o $name.html -s --number-sections --toc --css manual.css \
    --lua-filter=filters/include-files.lua \
    --lua-filter=filters/manual.lua
