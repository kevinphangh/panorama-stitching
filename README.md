# 🖼️ Panorama Stitching System

Combines two overlapping photos into one seamless panorama image.

## 🚀 Quick Start (3 Steps)

### 1️⃣ Build
```bash
./scripts/build.sh
```

### 2️⃣ Run Experiments  
```bash
./scripts/RUN_EXPERIMENTS.sh
```

### 3️⃣ View Results
```bash
# Generate report
python3 scripts/analyze_results.py

# Open in browser
cd results_analysis && python3 -m http.server 8000
# Visit: http://localhost:8000
```

## 📊 What You'll See

- **Original Images** - Input photos from 3 test scenes
- **Stitched Panoramas** - 60+ combined images showing results
- **Processing Times** - Speed comparison graphs
- **Feature Visualizations** - Keypoints and matches explained

## 🎯 Try Your Own Images

```bash
./scripts/run_panorama.sh --stitch photo1.jpg photo2.jpg --output result.jpg
```

## 📖 Understanding Results

**New to panorama stitching?** Read [SIMPLE_GUIDE.md](SIMPLE_GUIDE.md) for easy explanations!

## 📁 Clean Project Structure

```
.
├── src/               # C++ source code
├── scripts/           # Utility scripts  
├── datasets/          # Test images
├── tests/             # Test files
├── build/             # Build output (generated)
├── results/           # Experiment data (generated)
└── results_analysis/  # HTML reports (generated)
```

## 🎯 Features

- **Feature Detectors**: ORB (fast, ~25k keypoints) & AKAZE (accurate, ~5k keypoints)
- **RANSAC Filtering**: Configurable thresholds (1.0-5.0)
- **Blending Modes**: Simple, Feather, Multiband
- **Comprehensive Analysis**: 
  - Processing time breakdown
  - Feature/match visualizations
  - Interactive HTML reports

## 📊 View Results

After running experiments:
```bash
# Open analysis report
open results_analysis/analysis_report.html

# Or use Python server
cd results_analysis && python3 -m http.server 8000
# Then visit: http://localhost:8000
```

## 🛠️ Requirements

```bash
# Ubuntu/Debian
sudo apt-get install cmake g++ libopencv-dev python3-pip
pip3 install pandas matplotlib

# macOS
brew install cmake opencv python3
pip3 install pandas matplotlib
```

## 🔧 Troubleshooting

**Build fails?**
```bash
rm -rf build && ./scripts/build.sh
```

**Python error?**
```bash
pip3 install --upgrade pandas matplotlib numpy
```

**Can't open HTML?**
```bash
cd results_analysis && python3 -m http.server 8000
# Open browser to: http://localhost:8000
```