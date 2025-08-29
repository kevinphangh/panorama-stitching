#!/bin/bash

# Build script for Panorama Stitching project

echo "Building Panorama Stitching Project..."

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF

make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful! Executable: build/panorama_stitcher"
else
    echo "Build failed!"
    exit 1
fi