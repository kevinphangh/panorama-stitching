# Panorama Stitching

## Quick Start

```bash
# 1. Install dependencies (Ubuntu/Linux)
sudo apt install build-essential cmake libopencv-dev python3-pip
pip3 install matplotlib pandas

# 2. Run everything (builds & runs 48 experiments)
./RUN_EXPERIMENTS.sh

# 3. View results (multiple options)
./VIEW_RESULTS.sh  # Interactive viewer with 4 options
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

## Viewing Options

Run `./VIEW_RESULTS.sh` for an interactive menu with:
1. **Python HTTP Server** - Works in any browser (recommended)
2. **Default Browser** - Opens directly
3. **VSCode Preview** - For VSCode users
4. **Manual Paths** - Copy/paste URLs