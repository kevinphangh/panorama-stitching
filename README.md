# Panorama Stitching

Advanced panorama stitching application with multiple feature detectors and blending modes. Real-time implementation for Visual Computing Assignment 1 (Aarhus University 2025).

## Quick Start

```bash
# Build the project
./scripts/build.sh

# Stitch two images
./build/panorama_stitcher --stitch img1.jpg img2.jpg --output panorama.jpg

# Stitch multiple images (3+)
./build/panorama_stitcher --stitch-multiple img1.jpg img2.jpg img3.jpg --output panorama.jpg

# Or use the wrapper script (handles library paths)
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output panorama.jpg
```

## Features

- **Feature Detectors**: ORB (default), AKAZE
- **Feature Matching**: Brute-force with Lowe's ratio test
- **Homography Estimation**: RANSAC with DLT
- **Image Blending**: Simple overlay, feathering, multiband (Laplacian pyramid)

## Examples with Indoor Scene Dataset

### Two-Image Stitching
```bash
# Basic two-image stitching (img1 + img2)
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --output indoor_panorama_1_2.jpg

# Two-image stitching (img2 + img3) with feather blending
./build/panorama_stitcher --stitch datasets/indoor_scene/img2.jpg datasets/indoor_scene/img3.jpg --blend-mode feather --output indoor_panorama_2_3.jpg

# Using AKAZE detector for more features
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --detector akaze --output indoor_akaze.jpg
```

### Three-Image Panorama (Full Indoor Scene)
```bash
# Stitch all three indoor images together
./build/panorama_stitcher --stitch-multiple datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg datasets/indoor_scene/img3.jpg --output indoor_full_panorama.jpg

# With multiband blending for smoother transitions
./build/panorama_stitcher --stitch-multiple datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg datasets/indoor_scene/img3.jpg --blend-mode multiband --output indoor_multiband.jpg

# With visualization of intermediate steps
./build/panorama_stitcher --stitch-multiple datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg datasets/indoor_scene/img3.jpg --visualize --output indoor_debug.jpg
```

### Advanced Options
```bash
# High-quality stitching with more features and precise matching
./build/panorama_stitcher --stitch-multiple datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg datasets/indoor_scene/img3.jpg \
    --max-features 5000 \
    --ransac-threshold 1.0 \
    --blend-mode multiband \
    --output indoor_high_quality.jpg

# Fast stitching for real-time applications
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
    --max-features 500 \
    --blend-mode simple \
    --output indoor_fast.jpg
```

### Expected Output
- **Two-image stitching**: Creates a wide panorama showing continuous indoor space
- **Three-image stitching**: Produces a complete 180° view of the indoor environment
- **File sizes**: Typically 1-3MB for JPEG output
- **Processing time**: ~200ms per image pair on modern hardware

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
./scripts/run_panorama.sh [options]

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

Sample datasets included in `datasets/`:

### Indoor Scene (`datasets/indoor_scene/`)
- **img1.jpg**: Left view of modern classroom/workspace
- **img2.jpg**: Center view showing tables and windows
- **img3.jpg**: Right view completing the panoramic scene
- **Characteristics**: Well-lit interior with strong geometric features (tables, ceiling slats, windows)
- **Best for**: Testing feature detection on regular patterns and indoor lighting

### Quick Test Commands
```bash
# Test with indoor scene (most reliable)
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --output test.jpg

# Full indoor panorama (3 images)
./build/panorama_stitcher --stitch-multiple datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg datasets/indoor_scene/img3.jpg --output full_indoor.jpg
```

## Performance

Typical processing times (Intel i7, Release build):
- Feature Detection: ~50ms
- Feature Matching: ~20ms
- RANSAC: ~10ms
- Total Pipeline: ~200ms per image pair

## Troubleshooting

### Common Issues

1. **Library conflicts (libgomp.so.1)**
   ```bash
   # Use the wrapper script instead of direct binary
   ./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output result.jpg
   ```

2. **"Not enough features detected"**
   - Use ORB detector (default) instead of AKAZE
   - Increase `--max-features` to 5000
   - Ensure images have sufficient texture/detail

3. **Poor alignment or failed stitching**
   - Images must have 20-40% overlap
   - Ensure images are taken from same viewpoint height
   - Try reducing `--ransac-threshold` to 1.0 for stricter matching

4. **AKAZE failing on large images**
   - AKAZE may fail on images >5000x5000 pixels
   - Use ORB detector for large or multi-image stitching

## Tips for Best Results

- **Image Order**: Provide images from left to right for proper panorama assembly
- **Overlap**: Maintain 30-40% overlap between consecutive images
- **Lighting**: Consistent lighting across images produces better blending
- **Features**: Images with distinct features (corners, edges) stitch better than uniform surfaces
- **Multi-image**: For 3+ images, use default ORB detector for robustness