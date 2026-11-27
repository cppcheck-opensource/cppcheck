#!/bin/sh
# To install required tools in debian:
# sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended texlive-latex-extra
# For Windows you can use the MiKTeX installer https://miktex.org/download

set -e

# The documentation of all checkers is added as an appendix
./make-appendix.sh "Checkers" checkers/*.md > appendix-checkers.md

# manual.md contains the content for both the open source and the premium
# manual, the metadata file selects which one is built.
./build-pdf.sh manual manual.md appendix-checkers.md --metadata-file=manual-opensource.yaml
./build-html.sh manual manual.md appendix-checkers.md --metadata-file=manual-opensource.yaml

./build-pdf.sh manual-premium manual.md appendix-checkers.md --metadata-file=manual-premium.yaml
./build-html.sh manual-premium manual.md appendix-checkers.md --metadata-file=manual-premium.yaml

./build-pdf.sh reference-cfg-format
./build-html.sh reference-cfg-format
