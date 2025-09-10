#!/bin/bash
# Wrapper script to run panorama_stitcher with correct library paths
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
./build/panorama_stitcher "$@"
