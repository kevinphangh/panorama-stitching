# Panorama Stitching

## Quick Start

```bash
# 1. Install dependencies (Ubuntu/Linux)
sudo apt install build-essential cmake libopencv-dev python3-pip
pip3 install matplotlib pandas

# 2. Run everything (builds & runs 48 experiments)
./RUN_EXPERIMENTS.sh

# 3. View results
firefox results_organized/index.html
```

## What's Tested (48 Experiments)

- **Detectors**: ORB (~25k keypoints) vs AKAZE (~5k keypoints)
- **RANSAC**: 5 thresholds (1.0-5.0, best=3.0)
- **Blending**: Simple, Feather, Multiband (best quality)
- **Scenes**: Indoor, Outdoor1, Outdoor2
- **Stitching**: All pairs (1-2, 2-3, 1-3) & 3-image panoramas
- **Success Rate**: 96% (46/48)

## Output

- `results_organized/` - Visual comparison with HTML viewer
- `results/quantitative_report.html` - Performance metrics & charts
- `results/metrics.csv` - Raw data