#!/bin/bash
# Wrapper script to run panorama_stitcher with correct library paths
# This fixes library compatibility issues with conda/miniconda installations

# Fix library path to use system libraries instead of conda
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# Check if the executable exists
if [ ! -f "./build/panorama_stitcher" ]; then
    echo "Error: panorama_stitcher not found!"
    echo "Please build the project first (run 'make build')."
    exit 1
fi

# Run the panorama stitcher with all passed arguments
./build/panorama_stitcher "$@"
