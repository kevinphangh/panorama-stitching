#!/bin/bash
# Wrapper script to run panorama_stitcher with correct library paths

# Use system libraries to avoid conda/system conflicts
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# Run the panorama stitcher with all arguments
exec ./build/panorama_stitcher "$@"