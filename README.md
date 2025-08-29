# Panorama Stitching

Real-time panorama stitching implementation for Visual Computing Assignment 1 (Aarhus University 2025).

## Quick Start

```bash
# Build the project
./build.sh

# Stitch two images
./run_panorama.sh --stitch img1.jpg img2.jpg --output panorama.jpg

# Stitch multiple images (3+)
./run_panorama.sh --stitch-multiple img1.jpg img2.jpg img3.jpg --output panorama.jpg
```

## Features

- **Feature Detectors**: ORB (default), AKAZE
- **Feature Matching**: Brute-force with Lowe's ratio test
- **Homography Estimation**: RANSAC with DLT
- **Image Blending**: Simple overlay, feathering, multiband (Laplacian pyramid)

## Project Structure

```
src/
├── main.cpp                    # CLI application entry
├── feature_detection/           # Feature detection algorithms
│   ├── orb_detector.{h,cpp}    # ORB detector
│   └── akaze_detector.{h,cpp}  # AKAZE detector
├── feature_matching/            # Feature matching & RANSAC
│   ├── matcher.{h,cpp}         # Feature matching
│   └── ransac.{h,cpp}          # RANSAC homography filtering
├── homography/                  # Homography estimation
│   └── homography_estimator.{h,cpp}
├── stitching/                   # Image warping & blending
│   ├── image_warper.{h,cpp}    # Perspective warping
│   └── blender.{h,cpp}         # Multi-mode blending
└── experiments/                 # Performance evaluation
    └── experiment_runner.{h,cpp}
```

## Command-Line Options

```bash
./run_panorama.sh [options]

Options:
  --stitch <img1> <img2>        # Stitch two images
  --stitch-multiple <imgs...>   # Stitch 3+ images
  --detector <orb|akaze>        # Feature detector (default: orb)
  --blend-mode <mode>           # Blending mode: simple|feather|multiband
  --ransac-threshold <value>    # RANSAC threshold (default: 3.0)
  --max-features <num>          # Max features to detect (default: 2000)
  --output <path>               # Output file path
  --visualize                   # Show intermediate results
  --experiment-mode             # Run performance experiments
```

## Requirements

- CMake 3.16+
- OpenCV 4.x
- C++17 compiler
- OpenMP (optional)

## Building from Source

```bash
# Clean build
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Test Datasets

Three sample datasets are included in `datasets/`:
- `indoor_scene/`: Indoor environment test images
- `outdoor_scene1/`: Outdoor scene set 1
- `outdoor_scene2/`: Outdoor scene set 2

## Performance

Typical processing times (Intel i7, Release build):
- Feature Detection: ~50ms
- Feature Matching: ~20ms
- RANSAC: ~10ms
- Total Pipeline: ~200ms per image pair

## Known Issues

- AKAZE detector may fail on very large (>5000x5000) sparse images
- Large panoramas are automatically clamped to 5000x5000 pixels

## Notes

- Use `run_panorama.sh` wrapper to handle library path conflicts
- ORB detector recommended for multi-image stitching (more robust)
- Images are processed sequentially from left to right