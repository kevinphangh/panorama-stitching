# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Visual Computing Assignment 1 - Real-time panorama stitching system implementing feature detection, matching, homography estimation, and image warping with experimental evaluation framework.

## Build Commands

```bash
# Quick build (done automatically by RUN_EXPERIMENTS.sh)
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Clean rebuild
rm -rf build && mkdir -p build && cd build && cmake .. && make
```

## Development Commands

```bash
# Run panorama stitcher directly
./build/panorama_stitcher --stitch img1.jpg img2.jpg --output result.jpg

# Multi-image stitching (3+ images)
./build/panorama_stitcher --stitch-multiple img1.jpg img2.jpg img3.jpg --output panorama.jpg

# Run with specific detector and blending
./build/panorama_stitcher --stitch img1.jpg img2.jpg --detector akaze --blend-mode multiband

# Run all experiments (recommended)
./RUN_EXPERIMENTS.sh

# Run tests (when implemented)
cd build && ctest
```

## Architecture Overview

The codebase follows a modular pipeline architecture:

1. **Feature Detection** (`src/feature_detection/`): Abstract `FeatureDetector` base class with ORB and AKAZE implementations. Detectors extract keypoints and descriptors from input images.

2. **Feature Matching** (`src/feature_matching/`): `Matcher` class performs brute-force matching with Lowe's ratio test. `RANSAC` class filters matches using homography estimation to remove outliers.

3. **Homography Estimation** (`src/homography/`): `HomographyEstimator` uses RANSAC with OpenCV's homography estimation for robust computation of homography matrices from point correspondences.

4. **Image Warping & Blending** (`src/stitching/`): `ImageWarper` applies perspective transformations. `Blender` supports three modes: simple overlay, feathering, and multiband (Laplacian pyramid) blending.

5. **Experiment Framework** (`src/experiments/`): `ExperimentRunner` executes performance evaluations across different datasets and parameter configurations, collecting timing and quality metrics.

The main application (`src/main.cpp`) provides a CLI interface that orchestrates these components. All modules use OpenCV for core computer vision operations and support configurable parameters for experimentation.

## Key Implementation Details

- **C++17** with OpenCV 4.x for computer vision operations
- Template-based timing utilities for performance measurement
- RANSAC uses adaptive iteration count based on inlier ratio
- Multiband blending uses 5-level Laplacian pyramids
- Thread-safe operations for potential parallelization
- Configurable parameters: detector type, blend mode, RANSAC threshold, output format

## Testing Datasets

Three test dataset directories in `datasets/`:
- `indoor_scene/`: Indoor environment images (3 images: img1.jpg, img2.jpg, img3.jpg)
- `outdoor_scene1/`: Outdoor scene set 1 (currently empty - add images for testing)
- `outdoor_scene2/`: Outdoor scene set 2 (currently empty - add images for testing)

Note: Only indoor_scene currently contains test images. Add images to outdoor directories for full experiment testing.