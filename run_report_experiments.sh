#!/bin/bash

# Run experiments for Kevin's report
echo "Running experiments for report generation..."

# Create results directory
mkdir -p results
rm -f results/experiment_log.txt

# Test ORB vs AKAZE on indoor scene
echo "=== ORB vs AKAZE Comparison ===" | tee -a results/experiment_log.txt
echo "" | tee -a results/experiment_log.txt

echo "ORB Detector:" | tee -a results/experiment_log.txt
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
    --detector orb --output results/indoor_orb.jpg 2>&1 | tee -a results/experiment_log.txt

echo "" | tee -a results/experiment_log.txt
echo "AKAZE Detector:" | tee -a results/experiment_log.txt
./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
    --detector akaze --output results/indoor_akaze.jpg 2>&1 | tee -a results/experiment_log.txt

# Test different RANSAC thresholds
echo "" | tee -a results/experiment_log.txt
echo "=== RANSAC Threshold Analysis ===" | tee -a results/experiment_log.txt
for threshold in 1.0 2.0 3.0 4.0 5.0; do
    echo "" | tee -a results/experiment_log.txt
    echo "RANSAC Threshold: $threshold" | tee -a results/experiment_log.txt
    ./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
        --ransac-threshold $threshold --output results/indoor_ransac_${threshold}.jpg 2>&1 | tee -a results/experiment_log.txt
done

# Test different blending modes
echo "" | tee -a results/experiment_log.txt
echo "=== Blending Mode Comparison ===" | tee -a results/experiment_log.txt
for blend in simple feather multiband; do
    echo "" | tee -a results/experiment_log.txt
    echo "Blend Mode: $blend" | tee -a results/experiment_log.txt
    ./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
        --blend-mode $blend --output results/indoor_${blend}.jpg 2>&1 | tee -a results/experiment_log.txt
done

# Test on outdoor scenes
echo "" | tee -a results/experiment_log.txt
echo "=== Outdoor Scene Tests ===" | tee -a results/experiment_log.txt

echo "" | tee -a results/experiment_log.txt
echo "Outdoor Scene 1:" | tee -a results/experiment_log.txt
./build/panorama_stitcher --stitch datasets/outdoor_scene1/img1.jpg datasets/outdoor_scene1/img2.jpg \
    --output results/outdoor1_panorama.jpg 2>&1 | tee -a results/experiment_log.txt

echo "" | tee -a results/experiment_log.txt
echo "Outdoor Scene 2:" | tee -a results/experiment_log.txt
./build/panorama_stitcher --stitch datasets/outdoor_scene2/img1.jpg datasets/outdoor_scene2/img2.jpg \
    --output results/outdoor2_panorama.jpg 2>&1 | tee -a results/experiment_log.txt

# Test different feature counts
echo "" | tee -a results/experiment_log.txt
echo "=== Feature Count Analysis ===" | tee -a results/experiment_log.txt
for features in 500 1000 2000 5000; do
    echo "" | tee -a results/experiment_log.txt
    echo "Max Features: $features" | tee -a results/experiment_log.txt
    ./build/panorama_stitcher --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
        --max-features $features --output results/indoor_features_${features}.jpg 2>&1 | tee -a results/experiment_log.txt
done

echo "" | tee -a results/experiment_log.txt
echo "Experiments complete! Check results/experiment_log.txt for details."