# 📁 PROJECT STRUCTURE EXPLAINED

## 🗂️ Directory Overview

```
panorama-stitching/
│
├── 🚀 RUN.sh                    # START HERE! Main entry point
├── 📖 QUICK_START.md            # Beginner's guide
├── 📋 README.md                 # Full documentation
├── 📁 PROJECT_STRUCTURE.md      # This file
│
├── 💻 src/                      # C++ Source Code
│   ├── main.cpp                 # Program entry point
│   ├── config.h                 # Configuration settings
│   │
│   ├── 🔍 feature_detection/    # Finding interesting points
│   │   ├── feature_detector.h   # Base class
│   │   ├── orb_detector.cpp     # ORB detector (fast)
│   │   └── akaze_detector.cpp   # AKAZE detector (accurate)
│   │
│   ├── 🔗 feature_matching/     # Connecting points between images
│   │   ├── feature_matcher.h    # Matching interface
│   │   └── feature_matcher.cpp  # Implementation
│   │
│   ├── 🖼️ stitching/            # Combining images
│   │   ├── panorama_stitcher.h  # Main stitching logic
│   │   ├── image_warper.cpp     # Transforms images
│   │   └── image_blender.cpp    # Blends overlapping areas
│   │
│   ├── 🧪 experiments/          # Testing & evaluation
│   │   └── experiment_runner.cpp # Runs all tests
│   │
│   └── 🛠️ utils/                # Helper functions
│       └── visualization.cpp    # Drawing keypoints/matches
│
├── 📸 datasets/                  # Test Images
│   ├── indoor_scene/            # Indoor test (3 images)
│   ├── outdoor_scene1/          # Outdoor test 1 (3 images)
│   └── outdoor_scene2/          # Outdoor test 2 (3 images)
│
├── 🔧 scripts/                   # Helper Scripts
│   ├── RUN_EXPERIMENTS.sh       # Runs all 48 tests
│   ├── run_panorama.sh          # Wrapper for C++ program
│   ├── analyze_results.py       # Creates report
│   ├── build.sh                 # Build helper
│   └── VIEW_RESULTS.sh          # Opens results
│
├── 🏗️ build/                    # [GENERATED] Compiled files
│   └── panorama_stitcher        # The executable program
│
├── 📊 results/                   # [GENERATED] Output panoramas
│   ├── *.jpg                    # Stitched panoramas
│   └── metrics.csv              # Performance data
│
└── 📈 results_analysis/          # [GENERATED] Analysis
    ├── analysis_report.html     # Interactive report
    └── metrics_analysis.png     # Performance charts
```

---

## 🔄 How It All Works Together

### 1. **You Run:** `./RUN.sh`
   ↓
### 2. **It Builds:** CMake compiles `src/` → `build/panorama_stitcher`
   ↓
### 3. **It Tests:** Runs the program on images in `datasets/`
   ↓
### 4. **It Saves:** Panoramas go to `results/`
   ↓
### 5. **It Analyzes:** Python script creates report in `results_analysis/`

---

## 🧩 Key Components Explained

### 📍 Feature Detection (`src/feature_detection/`)
**What it does:** Finds interesting points in images (corners, edges)
- **ORB:** Fast, finds lots of points (25,000-50,000)
- **AKAZE:** Slower, but more accurate points (4,000-5,000)

### 🔗 Feature Matching (`src/feature_matching/`)
**What it does:** Finds which points in Image 1 match points in Image 2
- Uses "descriptors" (like fingerprints for each point)
- Filters bad matches using Lowe's ratio test

### 🎯 RANSAC (in `panorama_stitcher.cpp`)
**What it does:** Removes incorrect matches
- Randomly tests different match combinations
- Finds the best transformation between images
- Typical result: 100-600 good matches from 1000+ initial matches

### 🖼️ Image Warping (`src/stitching/image_warper.cpp`)
**What it does:** Transforms Image 2 to align with Image 1
- Uses homography matrix (3x3 transformation)
- Handles rotation, scale, and perspective

### 🎨 Image Blending (`src/stitching/image_blender.cpp`)
**What it does:** Smoothly combines overlapping areas
- **Simple:** Just overlays (visible seam)
- **Feather:** Gradual fade (smooth transition)
- **Multiband:** Frequency-based (best quality)

---

## 📝 Configuration (`src/config.h`)

Key settings you can adjust:
```cpp
MAX_FEATURES = 50000        // Maximum keypoints to detect
LOWE_RATIO = 0.7            // Match quality threshold
RANSAC_THRESHOLD = 3.0      // Outlier rejection strictness
RANSAC_CONFIDENCE = 0.995  // How sure we want to be
```

---

## 🗃️ Data Flow

1. **Input:** Two overlapping images
2. **Detect:** Find keypoints in both images
3. **Match:** Connect similar keypoints
4. **Filter:** Remove bad matches with RANSAC
5. **Transform:** Warp image 2 to align with image 1
6. **Blend:** Combine images smoothly
7. **Output:** Final panorama

---

## 📊 Metrics Collected

For each experiment, we record:
- Number of keypoints detected
- Number of initial matches
- Number of inliers (good matches)
- Inlier ratio (quality indicator)
- Processing time
- Success/failure status

---

## 🎯 Where to Look If You Want to...

### Modify the algorithm:
- Feature detection → `src/feature_detection/`
- Matching logic → `src/feature_matching/`
- Stitching process → `src/stitching/`

### Change parameters:
- Edit `src/config.h`

### Add new test images:
- Put them in `datasets/your_scene/`
- Name them: img1.jpg, img2.jpg, img3.jpg

### Modify experiments:
- Edit `scripts/RUN_EXPERIMENTS.sh`

### Change analysis/visualization:
- Edit `scripts/analyze_results.py`

---

## 💡 Tips for Understanding the Code

1. **Start with:** `src/main.cpp` - see how everything connects
2. **Follow the flow:** main → PanoramaStitcher → detect → match → stitch
3. **Key class:** `PanoramaStitcher` in `src/stitching/panorama_stitcher.cpp`
4. **Visualize:** Run with `--visualize` flag to see intermediate steps

---

## 🐛 Debugging

Common issues and where to look:
- **No keypoints found** → Check `feature_detector.cpp`
- **Bad matches** → Check `feature_matcher.cpp` and LOWE_RATIO
- **Failed stitching** → Check RANSAC_THRESHOLD in `config.h`
- **Visible seams** → Try different blend_mode in `image_blender.cpp`

---

Remember: Start with `./RUN.sh` - it handles everything for you! 🚀