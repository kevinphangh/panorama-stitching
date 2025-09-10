#!/bin/bash

###############################################################################
#                    VISUAL COMPUTING ASSIGNMENT 1                           #
#                  COMPLETE EXPERIMENT RUNNER - ALL IN ONE                   #
###############################################################################
#                                                                             #
#  This is the ONLY script you need to run for all experiments!            #
#                                                                             #
#  Usage: ./RUN_EXPERIMENTS.sh                                              #
#                                                                             #
#  What it does:                                                            #
#    1. Builds the C++ project                                              #
#    2. Runs ALL experiments on ALL scenes                                  #
#    3. Analyzes and organizes results                                      #
#    4. Creates visual comparison HTML pages                                #
#                                                                             #
###############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Function to print colored headers
print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

# Function to print status
print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

###############################################################################
# STEP 1: BUILD THE PROJECT
###############################################################################

print_header "STEP 1: BUILDING PROJECT"

if [ ! -f "scripts/build.sh" ]; then
    print_error "Build script not found. Are you in the project root?"
    exit 1
fi

if ./scripts/build.sh > /dev/null 2>&1; then
    print_status "Project built successfully"
else
    print_error "Build failed! Check scripts/build.sh"
    exit 1
fi

###############################################################################
# STEP 2: CHECK DATASETS
###############################################################################

print_header "STEP 2: CHECKING DATASETS"

DATASETS_OK=true
for scene in indoor_scene outdoor_scene1 outdoor_scene2; do
    if [ -d "datasets/$scene" ]; then
        count=$(ls datasets/$scene/*.jpg 2>/dev/null | wc -l)
        if [ $count -eq 3 ]; then
            print_status "$scene: 3 images found"
        else
            print_warning "$scene: Found $count images (expected 3)"
            DATASETS_OK=false
        fi
    else
        print_error "$scene: Directory not found"
        DATASETS_OK=false
    fi
done

if [ "$DATASETS_OK" = false ]; then
    print_warning "Some datasets are incomplete. Results may be limited."
    echo "Please ensure each scene folder has exactly 3 images: img1.jpg, img2.jpg, img3.jpg"
fi

###############################################################################
# STEP 3: CLEAN PREVIOUS RESULTS
###############################################################################

print_header "STEP 3: PREPARING RESULTS DIRECTORY"

rm -rf results results_organized
mkdir -p results

# Create CSV header for metrics
echo "experiment,scene,images,detector,threshold,blend_mode,keypoints,matches,inliers,inlier_ratio,processing_time_ms,status" > results/metrics.csv

print_status "Results directory cleaned and ready"

###############################################################################
# STEP 4: RUN ALL EXPERIMENTS
###############################################################################

print_header "STEP 4: RUNNING EXPERIMENTS"

echo ""
echo "This will test:"
echo "  • 2 detectors (ORB, AKAZE)"
echo "  • 3 scenes (indoor, outdoor1, outdoor2)"
echo "  • All image pairs (1-2, 2-3, 1-3)"
echo "  • 5 RANSAC thresholds (1.0 to 5.0)"
echo "  • 3 blending modes (simple, feather, multiband)"
echo "  • Multi-image stitching (all 3 images)"
echo ""
echo "Total: ~48 experiments"
echo ""

# Initialize counters
TOTAL=0
SUCCESS=0
FAILED=0

# Function to run single experiment
run_experiment() {
    local scene=$1
    local img1=$2
    local img2=$3
    local detector=$4
    local threshold=$5
    local blend=$6
    local output=$7
    local label=$8
    
    TOTAL=$((TOTAL + 1))
    
    # Show progress
    echo -ne "\r[${TOTAL}/48] Testing: ${label}...                    "
    
    # Run experiment and capture output for metrics
    local exp_output=$(./build/panorama_stitcher --stitch "$img1" "$img2" \
        --detector "$detector" \
        --ransac-threshold "$threshold" \
        --blend-mode "$blend" \
        --output "$output" 2>&1)
    
    if echo "$exp_output" | grep -q "Panorama saved"; then
        SUCCESS=$((SUCCESS + 1))
        
        # Extract metrics from output
        local keypoints=$(echo "$exp_output" | grep -oP 'Detected \K\d+' | head -1)
        local matches=$(echo "$exp_output" | grep -oP 'Found \K\d+(?= good matches)')
        local inliers=$(echo "$exp_output" | grep -oP 'RANSAC found \K\d+(?= inliers)')
        local ratio=$(echo "$exp_output" | grep -oP 'inliers \(\K[0-9.]+(?=%\))')
        local time=$(echo "$exp_output" | grep -oP 'Total time: \K[0-9.]+(?= ms)')
        
        # Write images pair info
        local img_pair="$(basename $img1)-$(basename $img2)"
        
        # Append to CSV
        echo "$label,$scene,$img_pair,$detector,$threshold,$blend,$keypoints,$matches,$inliers,$ratio,$time,SUCCESS" >> results/metrics.csv
    else
        FAILED=$((FAILED + 1))
        local img_pair="$(basename $img1)-$(basename $img2)"
        echo "$label,$scene,$img_pair,$detector,$threshold,$blend,0,0,0,0,0,FAILED" >> results/metrics.csv
    fi
}

# Function to run multi-image experiment
run_multi_experiment() {
    local scene=$1
    local img1=$2
    local img2=$3
    local img3=$4
    local detector=$5
    local output=$6
    local label=$7
    
    TOTAL=$((TOTAL + 1))
    
    echo -ne "\r[${TOTAL}/48] Testing: ${label}...                    "
    
    # Run experiment and capture output
    local exp_output=$(./build/panorama_stitcher --stitch-multiple "$img1" "$img2" "$img3" \
        --detector "$detector" \
        --output "$output" 2>&1)
    
    if echo "$exp_output" | grep -q "Panorama saved\|created successfully"; then
        SUCCESS=$((SUCCESS + 1))
        # Extract any available metrics
        local time=$(echo "$exp_output" | grep -oP 'Total time: \K[0-9.]+(?= ms)' | head -1)
        echo "$label,$scene,multi-image,$detector,3.0,feather,0,0,0,0,$time,SUCCESS" >> results/metrics.csv
    else
        FAILED=$((FAILED + 1))
        echo "$label,$scene,multi-image,$detector,3.0,feather,0,0,0,0,0,FAILED" >> results/metrics.csv
    fi
}

# Define scenes
declare -a scenes=("indoor_scene" "outdoor_scene1" "outdoor_scene2")

# Part 1: Detector comparison (all image pairs)
echo -e "${MAGENTA}Running detector comparison...${NC}"
for scene in "${scenes[@]}"; do
    for detector in orb akaze; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" "datasets/$scene/img2.jpg" \
            "$detector" 3.0 feather \
            "results/${scene}_pair_1_2_${detector}.jpg" \
            "$scene pair(1-2) $detector"
        
        run_experiment "$scene" \
            "datasets/$scene/img2.jpg" "datasets/$scene/img3.jpg" \
            "$detector" 3.0 feather \
            "results/${scene}_pair_2_3_${detector}.jpg" \
            "$scene pair(2-3) $detector"
        
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" "datasets/$scene/img3.jpg" \
            "$detector" 3.0 feather \
            "results/${scene}_pair_1_3_${detector}.jpg" \
            "$scene pair(1-3) $detector"
    done
done

# Part 2: RANSAC threshold analysis
echo -e "\n${MAGENTA}Running RANSAC analysis...${NC}"
for scene in "${scenes[@]}"; do
    for threshold in 1.0 2.0 3.0 4.0 5.0; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" "datasets/$scene/img2.jpg" \
            "orb" "$threshold" feather \
            "results/${scene}_ransac_${threshold}.jpg" \
            "$scene RANSAC-$threshold"
    done
done

# Part 3: Blending comparison
echo -e "\n${MAGENTA}Running blending comparison...${NC}"
for scene in "${scenes[@]}"; do
    for blend in simple feather multiband; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" "datasets/$scene/img2.jpg" \
            "orb" 3.0 "$blend" \
            "results/${scene}_blend_${blend}.jpg" \
            "$scene blend-$blend"
    done
done

# Part 4: Multi-image stitching
echo -e "\n${MAGENTA}Running multi-image stitching...${NC}"
for scene in "${scenes[@]}"; do
    for detector in orb akaze; do
        run_multi_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img2.jpg" \
            "datasets/$scene/img3.jpg" \
            "$detector" \
            "results/${scene}_multi_${detector}.jpg" \
            "$scene multi-$detector"
    done
done

echo ""  # Clear the progress line
print_status "Experiments completed: $SUCCESS successful, $FAILED failed"

###############################################################################
# STEP 5: ANALYZE AND ORGANIZE RESULTS
###############################################################################

print_header "STEP 5: ANALYZING & ORGANIZING RESULTS"

# Run the analysis and organization script
if python3 scripts/analyze_and_organize.py; then
    print_status "Results analyzed and organized"
else
    print_warning "Analysis script had issues, but continuing..."
fi

###############################################################################
# STEP 6: FINAL SUMMARY
###############################################################################

print_header "EXPERIMENT COMPLETE!"

echo ""
echo -e "${GREEN}Summary:${NC}"
echo "  • Total experiments run: $TOTAL"
echo "  • Successful stitches: $SUCCESS"
echo "  • Failed stitches: $FAILED"
echo "  • Success rate: $((SUCCESS * 100 / TOTAL))%"
echo ""
echo -e "${GREEN}Generated outputs:${NC}"
echo "  • results/           - Raw experiment outputs"
echo "  • results_organized/ - Organized results with HTML viewer"
echo ""
echo -e "${YELLOW}To view results:${NC}"
echo "  1. Open results_organized/index.html in a web browser"
echo "  2. Or run: firefox results_organized/index.html"
echo ""
echo -e "${BLUE}For the report:${NC}"
echo "  • All quantitative data is in results_organized/"
echo "  • Screenshots can be taken from the HTML pages"
echo "  • Raw data is in results/ if needed"
echo ""

# Success!
exit 0