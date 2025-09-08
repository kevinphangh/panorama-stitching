# Assignment Completion Guide - Task Division

## ✅ Already Completed
- [x] Feature detection (ORB & AKAZE)
- [x] Feature matching with Lowe's ratio test
- [x] RANSAC homography estimation
- [x] Image warping implementation
- [x] Three blending modes (simple, feather, multiband)
- [x] Experiment framework
- [x] Visualization tools
- [x] Multi-image panorama support
- [x] Report template with methodology

## 🤖 What I (Claude) Can Do Now

### 1. Run Complete Experiments
```bash
# I will execute these commands and collect all metrics
./scripts/build.sh
./build/panorama_stitcher --experiment-mode
```
**Generates:**
- `results/metrics.csv` with timing data
- `results/panorama_*.jpg` output images
- Performance measurements for all configurations

### 2. Generate Quantitative Analysis
I will create:
- **Detector comparison table**:
  - ORB vs AKAZE keypoint counts
  - Matching times (ms)
  - Inlier counts and ratios
  - Reprojection errors

- **RANSAC threshold analysis**:
  - Threshold values: 1.0, 2.0, 3.0, 4.0, 5.0
  - Corresponding inlier counts
  - Runtime impact measurements
  - Success/failure rates

- **Performance metrics table**:
  - Feature detection time
  - Matching time
  - RANSAC time
  - Warping time
  - Total pipeline time

### 3. Create Visualization Plots
```bash
# Generate all required plots
./build/panorama_stitcher --experiment-mode
# Then extract visualization data
```
I will generate:
- Match distance histograms
- Inlier count vs threshold plots
- Detection time comparison charts
- CSV data for all metrics

### 4. Fill Report Sections
I will complete these parts of `docs/REPORT.md`:
- Quantitative results tables
- Performance metrics
- Runtime comparisons
- Algorithm complexity analysis
- Technical implementation details

## 👤 What YOU Need to Do

### 1. Personal Information (5 minutes)
**Add to report:**
```bash
# Get your system info
lscpu | grep "Model name"           # Your CPU
free -h | grep "Mem"                 # Your RAM
uname -r                             # Kernel version
g++ --version | head -n1             # Compiler version
```

**Fill in REPORT.md line 5:**
- Your name

**Fill in Section 4.3:**
- Hardware: [Your CPU, RAM from above]
- Software: [Ubuntu/Windows version]
- Compiler: [GCC version from above]

### 2. Take Real Photos (30 minutes) - OPTIONAL but Recommended
**Required: 2 outdoor scenes**

**Scene 1 - Easy outdoor (building/architecture):**
- 3 photos with 30-40% overlap
- Stand in one spot, rotate camera
- Avoid moving objects (cars, people)
- Record: Time, weather, location

**Scene 2 - Challenging outdoor (nature/complex):**
- 3 photos with 30-40% overlap  
- Different lighting or texture from Scene 1
- Record: Time, weather, what makes it challenging

**Save as:**
```
datasets/outdoor_scene1/img1.jpg, img2.jpg, img3.jpg
datasets/outdoor_scene2/img1.jpg, img2.jpg, img3.jpg
```

### 3. Subjective Visual Evaluation (20 minutes)
**Run these commands and LOOK at outputs:**

```bash
# A. Compare blending modes visually
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --blend-mode simple --output eval_simple.jpg
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --blend-mode feather --output eval_feather.jpg
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --blend-mode multiband --output eval_multiband.jpg

# B. View with image viewer
eog eval_*.jpg  # or your preferred viewer
```

**Write your observations in Section 5.3:**
- Which has visible seams? Rate 1-10
- Which has best color consistency? 
- Which preserves details best?
- Any ghosting or artifacts?

**Example subjective assessment:**
```markdown
### 5.3 Blending Method Evaluation

Visual inspection reveals:
- **Simple overlay**: Hard visible seams at image boundaries (8/10 visibility), 
  abrupt color transitions, fastest but lowest quality
- **Feather blending**: Smooth transitions with no visible seams (1/10), 
  slight color bleeding in overlap regions, good balance
- **Multiband**: No visible seams (0/10), excellent detail preservation, 
  best color consistency, highest quality but slowest
```

### 4. Failure Case Analysis (10 minutes)
**Try difficult pairs and document failures:**

```bash
# Test image pairs that might fail
./build/panorama_stitcher --stitch datasets/outdoor_scene1/img1.jpg datasets/outdoor_scene1/img3.jpg --output fail_test.jpg
```

**Document in Section 6.3:**
- Which image pairs failed?
- Why did they fail? (not enough overlap, parallax, lighting)
- Visual evidence of failure

### 5. Dataset Descriptions (5 minutes)
**Fill Section 4.1:**

Even if using temporary images, describe:
```markdown
- **Indoor Scene**: Modern classroom with tables and ceiling slats. 
  Fluorescent lighting, rich geometric features, minimal shadows.
  
- **Outdoor Scene 1**: [Your description or "Architectural scene with 
  buildings, captured midday under clear sky, strong edge features"]
  
- **Outdoor Scene 2**: [Your description or "Natural landscape with 
  trees and paths, evening lighting, varied textures"]
```

### 6. Record Demo Video (10 minutes)
**Screen recording showing:**
1. Start terminal
2. Run command: `./scripts/run_panorama.sh --stitch img1.jpg img2.jpg --visualize --output demo.jpg`
3. Show feature detection visualization
4. Show matching visualization  
5. Show final panorama output
6. Open result in image viewer

**Tools:**
- Linux: `SimpleScreenRecorder` or `OBS Studio`
- Windows: `Windows+G` game bar or `OBS Studio`
- Mac: `Command+Shift+5` or `OBS Studio`

Keep it under 2 minutes. No audio needed.

### 7. Final PDF Generation (2 minutes)
```bash
# After all sections filled
./scripts/compile_report.sh
# Creates docs/REPORT.pdf

# If pandoc not installed:
sudo apt-get install pandoc texlive-latex-base
```

## 📋 Final Checklist

### What I Will Do (After You Confirm):
- [ ] Run full experiment suite
- [ ] Generate metrics.csv with all data
- [ ] Create visualization plots
- [ ] Fill quantitative sections of report
- [ ] Add performance tables
- [ ] Complete technical analysis

### What You Must Do:
- [ ] Add your name to report (line 5)
- [ ] Add system specs (Section 4.3)
- [ ] Run visual comparison commands above
- [ ] Write subjective quality assessment (Section 5.3)
- [ ] Describe datasets (Section 4.1) 
- [ ] Document any failure cases (Section 6.3)
- [ ] Write 3 main conclusions (Section 7)
- [ ] Record 1-2 minute demo video
- [ ] Generate final PDF

### Optional But Recommended:
- [ ] Take 6 real outdoor photos (2 sets × 3 images)
- [ ] Re-run experiments with your photos
- [ ] Add best panorama examples to appendix

## 🚀 Ready to Start?

**Total time needed:**
- Me: ~5 minutes to run experiments and fill report
- You: ~45-60 minutes for photos, evaluation, and video
- Without real photos: ~30 minutes

**Command me to start:**
"Run the experiments and fill the report with results"

Then you just need to add personal touches and subjective evaluations!