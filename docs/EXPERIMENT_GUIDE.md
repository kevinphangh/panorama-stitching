# Experiment Running Guide

## Quick Start

### 1. Run All Default Experiments
```bash
./build/panorama_stitcher --experiment-mode
```
This runs all pre-configured experiments and generates:
- `results/metrics.csv` - All experimental data
- `results/panorama_*.jpg` - Output panoramas
- Visualization plots (after running experiments)

### 2. Generate Visualization Plots
After running experiments:
```bash
# Compile the plot generator (one time only)
g++ -std=c++17 generate_plots.cpp src/experiments/visualization.cpp \
    -I src -I /usr/include/opencv4 \
    -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui \
    -o generate_plots

# Generate plots
./generate_plots
```
This creates:
- `results/detector_comparison.jpg` - ORB vs AKAZE performance
- `results/inlier_comparison.jpg` - Match quality comparison
- `results/ransac_threshold_plot.jpg` - Threshold vs inlier count
- `results/blending_comparison.jpg` - Blending method performance

## Modifying Experiment Parameters

### Method 1: Command-Line Parameters (Individual Tests)

Test specific configurations without modifying code:

```bash
# Test with different detectors
./build/panorama_stitcher --stitch img1.jpg img2.jpg --detector orb --output test_orb.jpg
./build/panorama_stitcher --stitch img1.jpg img2.jpg --detector akaze --output test_akaze.jpg

# Test different RANSAC thresholds
./build/panorama_stitcher --stitch img1.jpg img2.jpg --ransac-threshold 1.0 --output test_t1.jpg
./build/panorama_stitcher --stitch img1.jpg img2.jpg --ransac-threshold 5.0 --output test_t5.jpg

# Test different feature counts
./build/panorama_stitcher --stitch img1.jpg img2.jpg --max-features 500 --output test_500.jpg
./build/panorama_stitcher --stitch img1.jpg img2.jpg --max-features 5000 --output test_5000.jpg

# Test blending modes
./build/panorama_stitcher --stitch img1.jpg img2.jpg --blend-mode simple --output test_simple.jpg
./build/panorama_stitcher --stitch img1.jpg img2.jpg --blend-mode feather --output test_feather.jpg
./build/panorama_stitcher --stitch img1.jpg img2.jpg --blend-mode multiband --output test_multi.jpg

# Combine parameters
./build/panorama_stitcher --stitch img1.jpg img2.jpg \
    --detector akaze \
    --max-features 3000 \
    --ransac-threshold 2.0 \
    --blend-mode multiband \
    --visualize \
    --output custom_test.jpg
```

### Method 2: Modify Experiment Code (Batch Tests)

Edit `src/experiments/experiment_runner.cpp` to change experiment parameters:

#### Change RANSAC Thresholds Being Tested
```cpp
// Line 85 in experiment_runner.cpp
std::vector<double> thresholds = {1.0, 2.0, 3.0, 4.0, 5.0};
// Change to test different values:
std::vector<double> thresholds = {0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 6.0, 8.0, 10.0};
```

#### Change Feature Counts
```cpp
// Lines 94, 124 in experiment_runner.cpp
config.max_features = 2000;
// Change to:
config.max_features = 5000;  // Test with more features
```

#### Change Ratio Test Threshold
```cpp
// Lines 95, 125 in experiment_runner.cpp
config.ratio_test_threshold = 0.7;
// Change to:
config.ratio_test_threshold = 0.8;  // More permissive matching
// or
config.ratio_test_threshold = 0.6;  // Stricter matching
```

#### Add New Detectors to Test
```cpp
// Line 51 in experiment_runner.cpp
std::vector<std::string> detectors = {"orb", "akaze"};
// Could add more if implemented:
std::vector<std::string> detectors = {"orb", "akaze", "sift"};  // If SIFT added
```

#### Add More Blending Modes
```cpp
// Line 115 in experiment_runner.cpp
std::vector<std::string> blend_modes = {"simple", "feather", "multiband"};
// Add if implemented:
std::vector<std::string> blend_modes = {"simple", "feather", "multiband", "exposure"};
```

### Method 3: Modify Global Configuration

Edit `src/config.h` to change system-wide defaults:

```cpp
// Feature Detection Defaults
constexpr int DEFAULT_MAX_FEATURES = 2000;  // Change to 3000 for more features
constexpr int MIN_FEATURES = 10;            // Minimum allowed
constexpr int MAX_FEATURES = 50000;         // Maximum allowed

// RANSAC Defaults
constexpr double DEFAULT_RANSAC_THRESHOLD = 3.0;     // Change default threshold
constexpr double DEFAULT_RANSAC_CONFIDENCE = 0.995;  // Change confidence level
constexpr int DEFAULT_RANSAC_MAX_ITERATIONS = 2000;  // Change max iterations
constexpr int MIN_INLIERS_REQUIRED = 20;            // Minimum inliers for valid match

// Size Limits
constexpr int MAX_PANORAMA_DIMENSION = 15000;  // Maximum output size
```

After modifying config.h or experiment_runner.cpp, rebuild:
```bash
cd build && make -j4
```

## Custom Experiment Scenarios

### 1. Test Different Image Qualities
```bash
# Create different quality versions of images
convert input.jpg -quality 100 high_quality.jpg
convert input.jpg -quality 50 medium_quality.jpg
convert input.jpg -quality 20 low_quality.jpg

# Test each
for quality in high medium low; do
    ./build/panorama_stitcher --stitch ${quality}_quality1.jpg ${quality}_quality2.jpg \
        --output result_${quality}.jpg
done
```

### 2. Test with Different Image Scales
```bash
# Create scaled versions
convert img1.jpg -resize 50% img1_half.jpg
convert img2.jpg -resize 50% img2_half.jpg

# Test at different scales
./build/panorama_stitcher --stitch img1_half.jpg img2_half.jpg --output half_scale.jpg
./build/panorama_stitcher --stitch img1.jpg img2.jpg --output full_scale.jpg
```

### 3. Batch Testing Script
Create `run_custom_experiments.sh`:
```bash
#!/bin/bash

# Test matrix of parameters
DETECTORS=("orb" "akaze")
THRESHOLDS=(1.0 2.0 3.0 4.0 5.0)
FEATURES=(500 1000 2000 5000)
BLEND_MODES=("simple" "feather" "multiband")

for detector in "${DETECTORS[@]}"; do
    for threshold in "${THRESHOLDS[@]}"; do
        for features in "${FEATURES[@]}"; do
            for blend in "${BLEND_MODES[@]}"; do
                output="result_${detector}_t${threshold}_f${features}_${blend}.jpg"
                echo "Testing: $detector, threshold=$threshold, features=$features, blend=$blend"
                
                ./build/panorama_stitcher --stitch \
                    datasets/indoor_scene/img1.jpg \
                    datasets/indoor_scene/img2.jpg \
                    --detector $detector \
                    --ransac-threshold $threshold \
                    --max-features $features \
                    --blend-mode $blend \
                    --output $output
            done
        done
    done
done
```

## Analyzing Results

### 1. View Metrics CSV
```bash
# View in terminal
column -t -s, results/metrics.csv | less -S

# Open in spreadsheet
libreoffice --calc results/metrics.csv
```

### 2. Metrics to Analyze
- **Detection Time**: How fast features are detected
- **Number of Inliers**: Quality of feature matches
- **Inlier Ratio**: Percentage of good matches
- **Reprojection Error**: Accuracy of homography
- **Total Time**: Overall performance

### 3. Compare Results
```bash
# Extract specific metrics
grep "orb" results/metrics.csv | cut -d',' -f8,9,10 > orb_results.txt
grep "akaze" results/metrics.csv | cut -d',' -f8,9,10 > akaze_results.txt

# Compare averages
awk -F',' '{sum+=$1; count++} END {print "Avg Inliers:", sum/count}' orb_results.txt
```

## Adding New Experiments

### 1. Create New Experiment Function
Add to `src/experiments/experiment_runner.h`:
```cpp
void runCustomExperiment(const std::string& dataset_path);
```

Add implementation to `experiment_runner.cpp`:
```cpp
void ExperimentRunner::runCustomExperiment(const std::string& dataset_path) {
    std::cout << "\n=== Custom Experiment ===\n";
    
    // Your custom test parameters
    std::vector<int> custom_values = {100, 200, 300, 400, 500};
    
    for (int value : custom_values) {
        ExperimentConfig config;
        config.name = "custom_experiment";
        config.max_features = value;
        // ... set other parameters
        
        auto result = runSingleExperiment(img1_path, img2_path, config);
        results_.push_back(result);
    }
}
```

### 2. Call from runAllExperiments()
```cpp
void ExperimentRunner::runAllExperiments() {
    // ... existing experiments
    runCustomExperiment(dataset_dir);  // Add your experiment
}
```

## Tips for Experiment Design

1. **Control Variables**: Change only one parameter at a time
2. **Multiple Runs**: Run each configuration 3-5 times for consistency
3. **Datasets**: Test on indoor, outdoor, day, night scenes
4. **Document Results**: Keep notes on what worked best
5. **Visualize Early**: Use `--visualize` flag to see intermediate results

## Common Parameter Ranges

| Parameter | Min | Default | Max | Test Range |
|-----------|-----|---------|-----|-----------------|
| max_features | 10 | 2000 | 50000 | 500-5000 |
| ransac_threshold | 0.1 | 3.0 | 50.0 | 1.0-5.0 |
| ratio_test | 0.4 | 0.7 | 0.9 | 0.6-0.8 |
| confidence | 0.9 | 0.995 | 0.9999 | 0.99-0.999 |

## Troubleshooting

- **Too few matches**: Increase `max_features` or decrease `ratio_test_threshold`
- **Poor alignment**: Decrease `ransac_threshold` for stricter matching
- **Slow performance**: Decrease `max_features` or use ORB instead of AKAZE
- **Memory issues**: Reduce image size or `MAX_PANORAMA_DIMENSION` in config.h