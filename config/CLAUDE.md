# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Visual Computing course project implementing a panorama stitching system using C++ and OpenCV. The system performs feature detection, matching, homography estimation, and image blending to create panoramic images.

## Build and Development Commands

### Building the Project
```bash
# Recommended - using Makefile
make build              # Build C++ project with CMake
make clean              # Clean build artifacts
make clean-all          # Clean everything including results

# Alternative - direct CMake
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running Tests and Experiments
```bash
# Quick test with sample images
make test               # Runs a simple ORB test on indoor scene

# Full experiment suite (48 experiments)
make run                # Runs all experiments via scripts/run-experiments.sh
./run.sh                # Interactive menu for building and running

# Single panorama with custom parameters
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg \
    --detector akaze \
    --ransac-threshold 3.0 \
    --blend-mode feather \
    --output result.jpg
```

### Analyzing Results
```bash
make analyze            # Basic analysis
make analyze-full       # Full analysis with enhanced organization
make report             # Generate PDF report
python3 scripts/analysis_pipeline.py  # Direct analysis script
```

### Viewing Results
```bash
make view               # Opens interactive result viewer
make serve              # Start HTTP server on port 8000
./view-results.sh       # Interactive viewer with multiple options
```

## Code Architecture

### Core Pipeline Flow
The stitching pipeline (src/pipeline/stitching_pipeline.cpp) orchestrates:
1. **Feature Detection** → Factory pattern creates ORB/AKAZE detectors
2. **Feature Matching** → BFMatcher with ratio test filtering
3. **RANSAC Homography** → Robust homography estimation with configurable thresholds
4. **Image Warping** → Projects images to panorama coordinate system
5. **Blending** → Factory pattern for simple/feathering/multiband blending

### Key Design Patterns

**Factory Pattern**: Used for detector and blender creation
- `detector_factory.cpp`: Creates ORB or AKAZE detectors based on string parameter
- `blender_factory.cpp`: Creates appropriate blender (simple, feathering, multiband)

**Strategy Pattern**: Different blending strategies inherit from base `Blender` class
- Each blender implements `blend()` method differently
- Runtime selection based on user parameters

**Pipeline Pattern**: `StitchingPipeline` class manages the entire workflow
- Modular stages that can be configured independently
- Clear separation between detection, matching, and stitching phases

### Important Configuration

All key parameters are centralized in `src/config.h`:
- `DEFAULT_MAX_FEATURES`: 50000 keypoints per image
- `DEFAULT_RANSAC_THRESHOLD`: 3.0 pixels
- `MIN_INLIERS_REQUIRED`: 20 matches minimum
- `MAX_PANORAMA_DIMENSION`: 15000 pixels
- `MAX_PANORAMA_MEMORY`: 2GB limit

### Memory and Performance Considerations

The system implements several safety checks:
- Panorama size validation before allocation (MAX_PANORAMA_DIMENSION)
- Memory limit checks (MAX_PANORAMA_MEMORY)
- Homography validation to prevent degenerate transformations
- Determinant checks (MIN/MAX_HOMOGRAPHY_DETERMINANT)
- Scale validation (MIN/MAX_HOMOGRAPHY_SCALE)

### Experiment Framework

The experiment runner (`src/experiments/experiment_runner.cpp`) provides:
- Automated testing across multiple parameter combinations
- CSV metrics collection (keypoints, matches, inliers, ratios)
- Visualization generation (keypoints, matches, inlier filtering)
- HTML report generation with comparative analysis

### Python Analysis Pipeline

`scripts/analysis_pipeline.py` consolidates:
- CSV format normalization from C++ output
- Statistical analysis of experiment results
- Matplotlib visualization generation
- HTML report creation with interactive navigation
- Result organization by scene and parameter

## Testing Approach

### Unit Testing
No formal unit test framework is currently implemented. Testing is done through:
- The experiment runner which validates all combinations
- Quick test target: `make test`
- Visual inspection of results

### Integration Testing
Full pipeline testing via `make run` which tests:
- 2 detectors (ORB, AKAZE)
- 3 scenes (indoor, outdoor1, outdoor2)
- 5 RANSAC thresholds (1.0-5.0)
- 3 blending modes (simple, feather, multiband)
- Multiple image pairs and multi-image stitching

## Common Development Tasks

### Adding a New Feature Detector
1. Create new class inheriting from `FeatureDetector` base class
2. Implement `detect()` method
3. Add to `detector_factory.cpp` for runtime selection
4. Update CLI argument parser to accept new detector name

### Modifying RANSAC Parameters
Edit `src/config.h` to adjust:
- `DEFAULT_RANSAC_THRESHOLD`: Inlier distance threshold
- `DEFAULT_RANSAC_CONFIDENCE`: Success probability
- `DEFAULT_RANSAC_MAX_ITERATIONS`: Maximum RANSAC iterations

### Debugging Failed Stitches
1. Check visualizations in `results/visualizations/` for match quality
2. Review metrics.csv for inlier counts and ratios
3. Verify image overlap is sufficient (MIN_INLIERS_REQUIRED)
4. Adjust RANSAC threshold if too strict/permissive

### Performance Optimization
- OpenMP is enabled for parallel processing where applicable
- Compiler optimizations: `-O3 -march=native`
- Consider reducing MAX_FEATURES for faster processing
- Use ORB over AKAZE for speed-critical applications

## Dependencies

- OpenCV 4.x (core, imgcodecs, imgproc, features2d, calib3d, highgui)
- CMake 3.16+
- C++17 compiler (g++ or clang)
- Python 3 with pandas, matplotlib, numpy (for analysis)
- Optional: OpenMP for parallelization