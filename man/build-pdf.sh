#!/bin/sh
# Usage:
#   ./build-pdf.sh NAME                  # NAME.md -> NAME.pdf
#   ./build-pdf.sh NAME INPUT... [pandoc options]
#                                        # INPUT... -> NAME.pdf
#
# This uses installed fonts, which vary between systems
# Segoe UI and Consolas are standard in Windows 10, DejaVu is more common on Linux
name=$1
shift
if [ $# -eq 0 ]; then
    set -- "$name.md"
fi
echo Building $name.pdf

MainFont="Segoe UI"
MonoFont="Consolas"

is_font_installed() {
    fontname=$1
    fc-list | grep -i "$fontname" >/dev/null
}

if ! is_font_installed "$MainFont"; then
    MainFont="DejaVu Sans"
fi
if ! is_font_installed "$MonoFont"; then
    MonoFont="DejaVu Sans Mono"
fi

# echo Using $MainFont / $MonoFont

pandoc "$@" -o $name.pdf -s --number-sections --toc \
    --pdf-engine=xelatex \
    --listings \
    -f markdown \
    -V mainfont="$MainFont" \
    -V monofont="$MonoFont" \
    -V geometry:a4paper \
    -V geometry:margin=2.4cm \
    -V subparagraph \
    -H manual-style.tex \
    --lua-filter=filters/include-files.lua \
    --lua-filter=filters/manual.lua
