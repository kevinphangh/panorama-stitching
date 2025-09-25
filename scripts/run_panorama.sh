#!/bin/bash

export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

if [ ! -f "./build/panorama_stitcher" ]; then
    echo "Error: panorama_stitcher not found!"
    echo "Please build the project first (run 'make build')."
    exit 1
fi

./build/panorama_stitcher "$@"
