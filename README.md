# Panorama Stitching

## Setup

```bash
# Dependencies
sudo apt-get install cmake g++ libopencv-dev python3-pip
pip3 install pandas matplotlib numpy

# Build
make build
```

## Run

```bash
# Quick test
make test

# Full experiments (60 tests with SIFT)
make run

# Custom panorama
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output result.jpg

# Options
--detector [orb|akaze|sift]
--ransac-threshold [1.0-5.0]
--blend-mode [simple|feather|multiband]
```

## View Results

Results are saved in:
- `results/` - Raw outputs, panoramas and metrics.csv
- `results_analysis/` - Organized results with metrics analysis chart

```bash
# Analyze results
make analyze

# View metrics chart
open results_analysis/metrics_analysis.png      # macOS
xdg-open results_analysis/metrics_analysis.png  # Linux
```