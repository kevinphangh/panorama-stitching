# 🖼️ Panorama Stitching - Computer Vision Project

**Combines overlapping photos into seamless panoramas using C++ and OpenCV.**

## ⚡ Quick Start

```bash
./RUN.sh
```

This launches an interactive menu that handles everything. That's all you need!

## 📁 Clean Project Structure

```
panorama-stitching/
│
├── 🚀 RUN.sh              # START HERE - Interactive launcher
├── 📖 README.md           # This file
├── ⚙️ CMakeLists.txt      # Build configuration
├── 🤖 CLAUDE.md           # AI assistant instructions
│
├── 💻 src/                # C++ Source Code
│   ├── main.cpp           # Entry point
│   ├── config.h           # Tunable parameters
│   ├── feature_detection/ # ORB & AKAZE detectors
│   ├── feature_matching/  # Point correspondence
│   ├── stitching/         # Panorama creation
│   └── experiments/       # Testing framework
│
├── 📸 datasets/           # Test Images (3 scenes)
├── 🔧 scripts/            # Helper Scripts
├── 📚 docs/               # Documentation
├── 🧪 tests/              # Unit Tests
│
└── 📦 [Generated Folders]
    ├── build/             # Compiled binaries
    ├── results/           # Output panoramas
    └── results_analysis/  # HTML reports
```

## 🎯 Features

- **Two Detectors**: ORB (fast, 50k keypoints) & AKAZE (accurate, 5k keypoints)
- **Smart Matching**: RANSAC filtering removes bad matches
- **Three Blending Modes**: Simple, Feather, Multiband
- **Automatic Analysis**: Generates performance reports and visualizations

## 📊 Latest Results

- ✅ **75% success rate** on test dataset
- ✅ **32 panoramas** generated automatically
- ✅ **Real metrics** - no fake data
- ✅ **Interactive reports** with charts

## 🛠️ Installation

### Ubuntu/Linux:
```bash
sudo apt-get install cmake g++ libopencv-dev python3-pip
pip3 install pandas matplotlib
```

### macOS:
```bash
brew install cmake opencv python3
pip3 install pandas matplotlib
```

## 💡 Usage Examples

### Interactive (Recommended):
```bash
./RUN.sh
# Then choose from menu:
# 1 = Run all experiments
# 2 = Quick demo
# 3 = Your own images
```

### Command Line:
```bash
# Single stitch
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --output result.jpg

# With options
./scripts/run_panorama.sh --stitch img1.jpg img2.jpg \
    --detector akaze \
    --blend-mode multiband \
    --output panorama.jpg
```

## 📈 Understanding Results

After experiments, check:
- `results/` - Panorama images
- `results_analysis/analysis_report.html` - Interactive report
- `results/metrics.csv` - Raw performance data

Key metrics:
- **Keypoints**: Feature points detected (more = better)
- **Inliers**: Verified matches after RANSAC (>50 = good)
- **Inlier Ratio**: Match quality (>30% = success likely)

## 🔧 Configuration

Edit `src/config.h` to tune:
- `MAX_FEATURES` - Keypoint limit (default: 50000)
- `RANSAC_THRESHOLD` - Match strictness (default: 3.0)
- `LOWE_RATIO` - Match quality filter (default: 0.7)

## 📚 Learn More

| Guide | Description |
|-------|-------------|
| [Quick Start](docs/QUICK_START.md) | 2-minute beginner guide |
| [Project Structure](docs/PROJECT_STRUCTURE.md) | Code organization explained |
| [Getting Started](docs/GETTING_STARTED.md) | Absolute simplest guide |

## 🏆 Tips

- **Indoor scenes**: Use AKAZE detector
- **Outdoor scenes**: Use ORB detector
- **Best quality**: Multiband blending
- **Fastest**: Simple blending
- **Need 30-40% overlap** between images

## 📝 License

Academic use - Computer Vision Course Assignment

---

**Just run `./RUN.sh` and follow the menu!** 🚀