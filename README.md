# Panorama Stitching

## Setup

### Requirements
- Ubuntu/Linux with CMake 3.16+, OpenCV 4.x, C++17 compiler
- Python 3 with matplotlib, pandas, numpy

### Install Dependencies
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake pkg-config libopencv-dev python3-pip
pip3 install matplotlib pandas numpy

# macOS
brew install cmake opencv python3
pip3 install matplotlib pandas numpy
```

### Clone and Prepare
```bash
git clone https://github.com/kevinphangh/panorama-stitching.git
cd panorama-stitching
chmod +x RUN_EXPERIMENTS.sh scripts/*.sh
```

## Run Experiments

```bash
./RUN_EXPERIMENTS.sh
```

This runs all 48 experiments (~5 minutes):
- Tests ORB vs AKAZE detectors
- Analyzes RANSAC thresholds (1.0-5.0)  
- Compares 3 blending modes
- Generates quantitative analysis

## View Results

```bash
# Option 1: Use Python HTTP server (recommended)
python3 -m http.server 8000
# Then open browser to: http://localhost:8000/results/quantitative_report.html

# Option 2: Direct file opening
firefox results/quantitative_report.html          # Quantitative analysis
firefox results_organized/index.html              # Panorama gallery
```

### Generated Files
- `results/` - Contains metrics.csv, analysis charts (.png), and quantitative_report.html
- `results_organized/` - Organized panoramas by experiment type with HTML viewer