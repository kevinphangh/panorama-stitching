# 🖼️ Panorama Stitching System

Visual Computing Assignment 1 - Feature Detection, Matching, and Panorama Stitching

## 🚀 Quick Start

```bash
# Option 1: Use Makefile
make build          # Build the project
make test           # Quick test
make run            # Run all experiments
make view           # View results

# Option 2: Interactive menu
./run.sh            # Build, test, and run experiments interactively

# Option 3: View results
./view-results.sh   # Multiple viewing options for results
```

## 📁 Project Structure

```
.
├── Makefile               # Build automation
├── run.sh                 # Interactive runner
├── view-results.sh        # Results viewer
├── config/               # Configuration
│   └── CLAUDE.md        # AI assistant guide
├── docs/                 # Documentation (3 files)
│   ├── ASSIGNMENT_DOCUMENTATION.md
│   ├── Panorama_Stitching_Report.pdf
│   └── VC1Assignment_1.pdf
├── scripts/              # Core scripts (4 files)
│   ├── analysis_pipeline.py
│   ├── generate_pdf_report.py
│   ├── run-experiments.sh
│   └── run_panorama.sh
├── src/                  # C++ source code
│   ├── cli/             # Command line interface
│   ├── experiments/     # Experiment runner
│   ├── feature_detection/
│   ├── feature_matching/
│   ├── homography/
│   ├── pipeline/
│   └── stitching/
└── datasets/            # Test images (3 scenes)
```

## 🔬 Technical Implementation

### Feature Detection
- **ORB**: 2000 keypoints, fast with binary descriptors
- **AKAZE**: 2000 keypoints, more robust to scale changes

### Feature Matching
- Brute-force matching with Hamming distance (ORB) or L2 (AKAZE)
- Lowe's ratio test (threshold: 0.7) for outlier rejection

### RANSAC Homography
- Thresholds tested: 1.0, 2.0, 3.0, 4.0, 5.0 pixels
- Minimum 20 inliers required for stable homography
- 2000 iterations maximum

### Image Blending
1. **Simple**: Direct pixel replacement (fastest)
2. **Feathering**: Linear blending in overlap region (best balance)
3. **Multiband**: Laplacian pyramid blending (highest quality)

## 🛠️ Build & Run

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install cmake g++ libopencv-dev python3-pip

# Python dependencies
pip3 install pandas matplotlib numpy
```

### Building
```bash
# Using Makefile (recommended)
make build

# Or manually
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

### Running Experiments
```bash
# Full experiment suite (43 tests)
make run
# or
./scripts/run-experiments.sh

# Single panorama test
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output result.jpg

# With custom parameters
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg \
    --detector akaze \
    --ransac-threshold 3.0 \
    --blend-mode feather \
    --output result.jpg
```

## 📊 Results

### Experiment Summary
- **Total Tests**: 43 experiments across 3 scenes
- **Success Rate**: 60.4% (26/43 successful)
- **Generated**: 25 panoramas, 78+ visualizations

### Performance by Scene
- **Indoor**: 12 successful - Good structure for feature matching
- **Outdoor1**: 12 successful - Rich texture provides reliable features
- **Outdoor2**: 2 successful - Challenging with poor image overlap

### Best Configurations
| Metric | Configuration | Result |
|--------|--------------|---------|
| Highest Inlier Ratio | ORB + Feathering | 81.2% |
| Most Reliable | AKAZE + RANSAC 3.0 | Consistent matches |
| Fastest | ORB + Simple blending | ~1.5s per panorama |
| Best Quality | AKAZE + Multiband | Seamless blending |

## 📈 View Results

```bash
# Interactive viewer with multiple options
./view-results.sh

# Or use Makefile
make view           # Opens result viewer
make serve          # HTTP server on port 8000

# Direct access
firefox results_analysis/analysis_report.html    # Main report
firefox results/index.html                       # Enhanced navigation
```

### Results Organization
```
results/
├── metrics.csv           # Experiment data
├── index.html            # Navigation hub
├── *.jpg                 # Panorama outputs
└── visualizations/       # Debug images

results_analysis/
├── analysis_report.html  # Comprehensive analysis
├── metrics_analysis.png  # Performance charts
└── panoramas/           # Organized gallery
```

## 📝 Documentation

- **Complete Guide**: [`docs/ASSIGNMENT_DOCUMENTATION.md`](docs/ASSIGNMENT_DOCUMENTATION.md)
- **Technical Report**: [`docs/Panorama_Stitching_Report.pdf`](docs/Panorama_Stitching_Report.pdf)
- **Assignment Spec**: [`docs/VC1Assignment_1.pdf`](docs/VC1Assignment_1.pdf)

## 🎯 Key Features

✅ **Dual Detectors**: ORB (speed) and AKAZE (robustness)
✅ **Multiple Blending**: Simple, Feathering, Multiband
✅ **RANSAC Validation**: 5 threshold levels tested
✅ **Comprehensive Analysis**: Auto-generated HTML reports
✅ **Visualization**: Keypoints, matches, and inlier filtering
✅ **Multi-image Support**: Sequential stitching capability

## 📄 License

Academic project for Visual Computing course, Aarhus University 2025