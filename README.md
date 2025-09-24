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

# Full experiments (60 tests)
make run

## View Results

Results are saved in:
- `results/` - Raw outputs, panoramas and metrics.csv
- `results_analysis/` - Organized results with metrics analysis chart

```bash
# Analyze results
make analyze
```