#!/bin/bash

# Manual experiment runner - runs experiments one at a time to avoid memory issues
echo "Running experiments manually (to avoid memory issues)..."

mkdir -p results
rm -f results/experiment_log.txt

# Function to run single experiment and log results
run_experiment() {
    local img1=$1
    local img2=$2
    local detector=$3
    local threshold=$4
    local blend=$5
    local output=$6
    local label=$7
    
    echo "Testing: $label" | tee -a results/experiment_log.txt
    ./build/panorama_stitcher --stitch "$img1" "$img2" \
        --detector "$detector" \
        --ransac-threshold "$threshold" \
        --blend-mode "$blend" \
        --output "$output" 2>&1 | tee -a results/experiment_log.txt
    echo "" | tee -a results/experiment_log.txt
}

# Test 1: Detector Comparison (ORB vs AKAZE)
echo "=== DETECTOR COMPARISON ===" | tee -a results/experiment_log.txt

# ORB on all scenes
run_experiment datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg orb 3.0 feather results/indoor_orb.jpg "ORB Indoor"
run_experiment datasets/outdoor_scene1/img1.jpg datasets/outdoor_scene1/img2.jpg orb 3.0 feather results/outdoor1_orb.jpg "ORB Outdoor1"
run_experiment datasets/outdoor_scene2/img1.jpg datasets/outdoor_scene2/img2.jpg orb 3.0 feather results/outdoor2_orb.jpg "ORB Outdoor2"

# AKAZE on all scenes
run_experiment datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg akaze 3.0 feather results/indoor_akaze.jpg "AKAZE Indoor"
run_experiment datasets/outdoor_scene1/img1.jpg datasets/outdoor_scene1/img2.jpg akaze 3.0 feather results/outdoor1_akaze.jpg "AKAZE Outdoor1"
run_experiment datasets/outdoor_scene2/img1.jpg datasets/outdoor_scene2/img2.jpg akaze 3.0 feather results/outdoor2_akaze.jpg "AKAZE Outdoor2"

# Test 2: RANSAC Threshold Analysis (using indoor scene)
echo "=== RANSAC THRESHOLD ANALYSIS ===" | tee -a results/experiment_log.txt

for threshold in 1.0 2.0 3.0 4.0 5.0; do
    run_experiment datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg orb "$threshold" feather "results/ransac_${threshold}.jpg" "RANSAC Threshold $threshold"
done

# Test 3: Blending Mode Comparison (using indoor scene)
echo "=== BLENDING MODE COMPARISON ===" | tee -a results/experiment_log.txt

run_experiment datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg orb 3.0 simple results/blend_simple.jpg "Simple Blending"
run_experiment datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg orb 3.0 feather results/blend_feather.jpg "Feather Blending"
run_experiment datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg orb 3.0 multiband results/blend_multiband.jpg "Multiband Blending"

echo "Manual experiments completed!"
echo "Check results/experiment_log.txt for details"