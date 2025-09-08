#!/bin/bash

# Compile markdown report to PDF using pandoc
# Requires: pandoc and pdflatex/xelatex

if ! command -v pandoc &> /dev/null; then
    echo "Error: pandoc is not installed"
    echo "Install with: sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended"
    exit 1
fi

echo "Compiling report to PDF..."
pandoc docs/REPORT.md \
    -o docs/REPORT.pdf \
    --pdf-engine=pdflatex \
    --highlight-style=tango \
    --toc \
    --variable geometry:margin=1in \
    --variable fontsize=11pt \
    --variable documentclass=article

if [ $? -eq 0 ]; then
    echo "Report compiled successfully: docs/REPORT.pdf"
else
    echo "Report compilation failed"
    exit 1
fi