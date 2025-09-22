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

# Full experiments (48 tests)
make run

# Custom panorama
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output result.jpg

# Options
--detector [orb|akaze]
--ransac-threshold [1.0-5.0]
--blend-mode [simple|feather|multiband]
```

## View Results

```bash
make view                               # Interactive viewer
make serve                              # HTTP server (localhost:8000)
firefox results_analysis/analysis_report.html    # Direct access
```

Results are in:
- `results/` - Raw outputs and metrics.csv
- `results_analysis/` - Organized analysis with charts