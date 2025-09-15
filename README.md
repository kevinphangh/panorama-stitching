# 🖼️ Panorama Stitching System

Visual Computing Assignment 1 - Feature Detection, Matching, and Panorama Stitching

## 🚀 Quick Start

```bash
# Run everything with one command:
./RUN.sh

# This interactive menu lets you:
# 1. Run all experiments (48 tests)
# 2. Quick demo (3 tests)
# 3. Test with your own images
# 4. View results
```

## 📁 Project Structure

```
.
├── src/                    # C++ implementation
│   ├── feature_detection/  # ORB & AKAZE detectors
│   ├── feature_matching/   # Brute-force matcher with Lowe's ratio
│   ├── stitching/         # Warping & blending (3 modes)
│   └── experiments/       # Experiment runner & visualization
├── datasets/              # Test images (3 scenes x 3 images)
├── scripts/               # Helper scripts
├── results/               # Output panoramas & metrics
├── results_analysis/      # HTML reports & charts
└── docs/                  # Assignment specs & guides
```

## 🔬 Technical Implementation

### Feature Detection
- **ORB**: 50,000 keypoints max, optimized for speed
- **AKAZE**: Variable keypoints, more robust to transformations

### Feature Matching
- Brute-force matching with Hamming distance (ORB) or L2 (AKAZE)
- Lowe's ratio test (threshold: 0.7) for outlier rejection

### RANSAC Homography
- Thresholds tested: 1.0, 2.0, 3.0, 4.0, 5.0 pixels
- Minimum 4 point correspondences required
- 2000 iterations maximum

### Image Blending
1. **Simple Overlay**: Direct pixel replacement
2. **Feathering**: Linear blending in overlap region
3. **Multiband**: Laplacian pyramid blending

## 📊 Experiment Results

**48 experiments** testing combinations of:
- 2 detectors (ORB, AKAZE)
- 3 scenes (indoor, outdoor1, outdoor2)
- 5 RANSAC thresholds
- 3 blending modes
- Multi-image stitching

### Key Findings
- **Success Rate**: 81% (39/48 successful panoramas)
- **ORB**: Best for outdoor scenes with texture
- **AKAZE**: Superior for indoor/structured environments
- **Optimal RANSAC**: Threshold 3.0 for most scenes

## 🛠️ Build & Run

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install cmake g++ libopencv-dev python3-pip

# macOS
brew install cmake opencv python3
```

### Manual Build
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
cd ..
```

### Run Experiments
```bash
# All experiments (5-10 minutes)
./scripts/RUN_EXPERIMENTS.sh

# Single panorama
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output panorama.jpg

# With options
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg \
    --detector akaze \
    --threshold 3.0 \
    --blend-mode multiband \
    --output result.jpg
```

## 📈 View Results

```bash
# Open HTML report in browser
python3 -m http.server 8000 --directory results_analysis
# Visit: http://localhost:8000/analysis_report.html

# Or directly open:
firefox results_analysis/analysis_report.html
```

## 📝 Documentation

- **Assignment Report**: `Panorama_Stitching_Report.pdf`
- **Assignment Checklist**: `docs/ASSIGNMENT_CHECKLIST.md`
- **Demo Video Guide**: `docs/DEMO_VIDEO_GUIDE.md`
- **Assignment Spec**: `docs/VC1Assignment_1.pdf`

## 🎯 Assignment Deliverables

✅ **Implemented**: All required components
✅ **Experiments**: 48 tests with metrics
✅ **Report**: 3-page PDF with analysis
✅ **Visualizations**: Keypoints, matches, histograms
✅ **Multi-image**: 3+ image stitching support

## 📄 License

Academic project for Visual Computing course, Aarhus University 2025