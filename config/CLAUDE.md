# CLAUDE.md

AI assistant guide for this Visual Computing panorama stitching project.

## Quick Commands

```bash
# Run all experiments
./scripts/run-experiments.sh

# Manual build
cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j4

# Test single stitch
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output result.jpg
```

## Architecture

- **Feature Detection**: ORB & AKAZE detectors (src/feature_detection/)
- **Matching**: Brute-force with Lowe's ratio test (src/feature_matching/)
- **RANSAC**: Homography estimation with outlier removal
- **Blending**: SIMPLE_OVERLAY, FEATHERING, MULTIBAND modes (src/stitching/)
- **Max Features**: Set to 50,000 for true performance

## Key Files

- `scripts/run-experiments.sh` - Main experiment runner (48 tests)
- `scripts/analysis_pipeline.py` - Results organization & analysis
- `src/config.h` - Configuration constants