#!/bin/sh
# Generate a markdown file with an appendix chapter that includes the given
# markdown files. The chapter heading gets "Appendix A" etc, see
# filters/manual.lua. The result is meant to be passed to pandoc after the manual,
# together with filters/include-files.lua and filters/manual.lua.
#
# Usage:
#   ./make-appendix.sh TITLE FILE... > appendix.md
#
# Example:
#   ./make-appendix.sh "Checkers" checkers/*.md > appendix-checkers.md
#   pandoc manual.md appendix-checkers.md --metadata-file=manual-premium.yaml \
#       --lua-filter=filters/include-files.lua --lua-filter=filters/manual.lua \
#       -o manual-premium.pdf

if [ $# -lt 2 ]; then
    echo "Usage: $0 TITLE FILE..." >&2
    exit 1
fi

title=$1
shift

fence='```'

printf '# %s {.appendix}\n\n' "$title"
# Sort case insensitive, independent of locale
printf '%s\n' "$@" | LC_ALL=C sort -f | while read -r f; do
    # The identifier must match file_id() in filters/manual.lua
    id=file-$(printf '%s' "$f" | sed 's/[^A-Za-z0-9]/-/g')
    printf '::: {#%s .included-file}\n\n%s{.include shift-heading-level-by=1}\n%s\n%s\n\n:::\n\n' "$id" "$fence" "$f" "$fence"
done
