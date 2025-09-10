# Panorama Stitching - Visual Computing Assignment 1

## 🚀 Quick Start (One Command!)

```bash
./RUN_EXPERIMENTS.sh
```

This automatically:
1. Builds the C++ project
2. Runs 48 experiments on 3 scenes
3. Tests ORB vs AKAZE detectors
4. Analyzes RANSAC thresholds (1.0-5.0)
5. Compares blending modes
6. Creates organized results with HTML viewer

## 📊 View Results

```bash
# Open in browser
firefox results_organized/index.html
```

## 📁 Project Structure

```
.
├── RUN_EXPERIMENTS.sh      # Main script - runs everything!
├── datasets/               # Image datasets (3 scenes × 3 images)
│   ├── indoor_scene/       # Indoor environment
│   ├── outdoor_scene1/     # Outdoor scene 1
│   └── outdoor_scene2/     # Outdoor scene 2
├── src/                    # C++ source code
├── scripts/                # Helper scripts (called automatically)
└── results_organized/      # Output folder (created after running)
    └── index.html          # Visual results browser
```

## 🔧 Manual Controls (Optional)

### Build Only
```bash
./scripts/build.sh
```

### Run Specific Test
```bash
./build/panorama_stitcher --stitch img1.jpg img2.jpg --output result.jpg
```

### Options
- `--detector` : orb (default) or akaze
- `--blend-mode` : simple, feather (default), or multiband
- `--ransac-threshold` : 1.0 to 5.0 (default: 3.0)

## 📋 Requirements

- CMake 3.16+
- OpenCV 4.x
- C++17 compiler
- Python 3 (for analysis)

## 🎯 Features

- **Feature Detection**: ORB and AKAZE implementations
- **Matching**: Brute-force with Lowe's ratio test
- **RANSAC**: Robust homography estimation
- **Blending**: Simple, feather, and multiband (Laplacian pyramid)
- **Multi-Image**: Stitch 3+ images sequentially

## 📈 Experiments Performed

| Category | Tests | Description |
|----------|-------|-------------|
| Detector Comparison | 18 | ORB vs AKAZE on all image pairs |
| RANSAC Analysis | 15 | Thresholds 1.0-5.0 on 3 scenes |
| Blending Modes | 9 | Simple/Feather/Multiband comparison |
| Multi-Image | 6 | 3-image panoramas per scene |

## ✅ Validation

Check if everything is ready:
```bash
./scripts/validate_setup.sh
```

## 📚 Additional Documentation

- [Assignment Guide](docs/ASSIGNMENT_GUIDE.md) - Detailed assignment requirements
- [Technical Details](docs/TECHNICAL.md) - Implementation details
- [API Reference](docs/CLAUDE.md) - Development notes

## 🏗️ Architecture

```
Feature Detection → Matching → RANSAC → Homography → Warping → Blending
     (ORB/AKAZE)     (BF+Ratio)  (Robust)   (DLT)    (Perspective) (3 modes)
```

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| Build fails | Check OpenCV installation: `pkg-config --modversion opencv4` |
| No matches found | Ensure 30-40% image overlap |
| Script permission denied | Run: `chmod +x RUN_EXPERIMENTS.sh` |
| Python error | Install Python 3: `sudo apt install python3` |

---
**Tip:** Just run `./RUN_EXPERIMENTS.sh` and everything happens automatically! 🎉