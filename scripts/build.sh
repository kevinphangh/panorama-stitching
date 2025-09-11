#!/bin/bash
# Build script for panorama stitcher

set -e

echo "Building panorama stitcher..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..
echo "Build complete! Binary at: build/panorama_stitcher"