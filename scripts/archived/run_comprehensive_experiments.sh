#!/bin/bash

# Comprehensive experiment runner that tests ALL image combinations
# Tests all pairs (1-2, 2-3, 1-3) and multi-image (1-2-3) for all scenes

echo "=========================================="
echo "COMPREHENSIVE EXPERIMENTS - ALL SCENES & IMAGES"
echo "=========================================="

# Clear and create results directory
rm -rf results/*
mkdir -p results

# Create CSV header for metrics
echo "experiment,scene,images,detector,threshold,blend_mode,keypoints1,keypoints2,matches,inliers,inlier_ratio,status" > results/comprehensive_metrics.csv

# Function to run and log experiment
run_experiment() {
    local scene=$1
    local img1=$2
    local img2=$3
    local detector=$4
    local threshold=$5
    local blend=$6
    local exp_type=$7
    local label=$8
    
    echo "Testing: $label"
    output=$(./build/panorama_stitcher --stitch "$img1" "$img2" \
        --detector "$detector" \
        --ransac-threshold "$threshold" \
        --blend-mode "$blend" \
        --output "results/${scene}_${exp_type}_${detector}_t${threshold}_${blend}.jpg" 2>&1)
    
    # Parse output for metrics
    keypoints=$(echo "$output" | grep -oP 'Detected \K\d+' | head -1)
    matches=$(echo "$output" | grep -oP 'Found \K\d+(?= good matches)')
    inliers=$(echo "$output" | grep -oP 'RANSAC found \K\d+(?= inliers)')
    ratio=$(echo "$output" | grep -oP 'inliers \(\K[0-9.]+(?=%\))')
    
    if echo "$output" | grep -q "Panorama saved"; then
        status="SUCCESS"
    else
        status="FAILED"
    fi
    
    # Append to CSV
    echo "$exp_type,$scene,$img1-$img2,$detector,$threshold,$blend,$keypoints,$keypoints,$matches,$inliers,$ratio,$status" >> results/comprehensive_metrics.csv
    
    # Append to log
    echo "$label: $status (Matches: $matches, Inliers: $inliers)" | tee -a results/experiment_summary.txt
}

# Function to run multi-image stitching
run_multi_experiment() {
    local scene=$1
    local img1=$2
    local img2=$3
    local img3=$4
    local detector=$5
    local blend=$6
    local label=$7
    
    echo "Testing: $label"
    output=$(./build/panorama_stitcher --stitch-multiple "$img1" "$img2" "$img3" \
        --detector "$detector" \
        --blend-mode "$blend" \
        --output "results/${scene}_multi_${detector}_${blend}.jpg" 2>&1)
    
    if echo "$output" | grep -q "Panorama saved\|created successfully"; then
        status="SUCCESS"
    else
        status="FAILED"
    fi
    
    echo "$label: $status" | tee -a results/experiment_summary.txt
}

# Initialize summary
echo "EXPERIMENT SUMMARY" > results/experiment_summary.txt
echo "==================" >> results/experiment_summary.txt
echo "" >> results/experiment_summary.txt

# Define scenes
declare -a scenes=("indoor_scene" "outdoor_scene1" "outdoor_scene2")

# PART 1: Test all image pairs with both detectors
echo -e "\n=== PART 1: ALL IMAGE PAIRS - DETECTOR COMPARISON ===" | tee -a results/experiment_summary.txt

for scene in "${scenes[@]}"; do
    echo -e "\n--- Scene: $scene ---" | tee -a results/experiment_summary.txt
    
    for detector in orb akaze; do
        # Test all three pairs: 1-2, 2-3, 1-3
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img2.jpg" \
            "$detector" 3.0 feather "pair_1_2" \
            "$scene Pair(1-2) $detector"
        
        run_experiment "$scene" \
            "datasets/$scene/img2.jpg" \
            "datasets/$scene/img3.jpg" \
            "$detector" 3.0 feather "pair_2_3" \
            "$scene Pair(2-3) $detector"
        
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img3.jpg" \
            "$detector" 3.0 feather "pair_1_3" \
            "$scene Pair(1-3) $detector"
    done
done

# PART 2: RANSAC threshold analysis (using best performing pair from each scene)
echo -e "\n=== PART 2: RANSAC THRESHOLD ANALYSIS ===" | tee -a results/experiment_summary.txt

for scene in "${scenes[@]}"; do
    echo -e "\n--- Scene: $scene ---" | tee -a results/experiment_summary.txt
    
    for threshold in 1.0 2.0 3.0 4.0 5.0; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img2.jpg" \
            "orb" "$threshold" feather "ransac_t${threshold}" \
            "$scene RANSAC-$threshold"
    done
done

# PART 3: Blending mode comparison
echo -e "\n=== PART 3: BLENDING MODE COMPARISON ===" | tee -a results/experiment_summary.txt

for scene in "${scenes[@]}"; do
    echo -e "\n--- Scene: $scene ---" | tee -a results/experiment_summary.txt
    
    for blend in simple feather multiband; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img2.jpg" \
            "orb" 3.0 "$blend" "blend_${blend}" \
            "$scene Blend-$blend"
    done
done

# PART 4: Multi-image stitching (all 3 images)
echo -e "\n=== PART 4: MULTI-IMAGE STITCHING (3 IMAGES) ===" | tee -a results/experiment_summary.txt

for scene in "${scenes[@]}"; do
    echo -e "\n--- Scene: $scene ---" | tee -a results/experiment_summary.txt
    
    for detector in orb akaze; do
        run_multi_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img2.jpg" \
            "datasets/$scene/img3.jpg" \
            "$detector" "feather" \
            "$scene Multi(1-2-3) $detector"
    done
done

# PART 5: Generate statistics
echo -e "\n=== GENERATING STATISTICS ===" | tee -a results/experiment_summary.txt

# Count results
total_experiments=$(grep -c "Testing:" results/experiment_summary.txt 2>/dev/null || echo 0)
successful=$(grep -c "SUCCESS" results/experiment_summary.txt 2>/dev/null || echo 0)
failed=$(grep -c "FAILED" results/experiment_summary.txt 2>/dev/null || echo 0)

echo -e "\n=== FINAL SUMMARY ===" | tee -a results/experiment_summary.txt
echo "Total experiments: $total_experiments" | tee -a results/experiment_summary.txt
echo "Successful: $successful" | tee -a results/experiment_summary.txt
echo "Failed: $failed" | tee -a results/experiment_summary.txt
echo "Success rate: $(echo "scale=1; $successful * 100 / $total_experiments" | bc)%" | tee -a results/experiment_summary.txt

# List generated files
echo -e "\n=== GENERATED FILES ===" | tee -a results/experiment_summary.txt
num_images=$(ls results/*.jpg 2>/dev/null | wc -l)
echo "Panorama images: $num_images" | tee -a results/experiment_summary.txt
echo "Metrics file: results/comprehensive_metrics.csv" | tee -a results/experiment_summary.txt
echo "Summary file: results/experiment_summary.txt" | tee -a results/experiment_summary.txt

echo -e "\n✅ Comprehensive experiments completed!"
echo "Check results/experiment_summary.txt for detailed results"