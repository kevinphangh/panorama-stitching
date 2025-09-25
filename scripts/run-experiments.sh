#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m'

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_header "STEP 1: BUILDING PROJECT"

mkdir -p build
if (cd build && cmake .. -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1 && make -j$(nproc) > /dev/null 2>&1); then
    print_status "Project built successfully"
else
    print_error "Build failed! Check CMakeLists.txt and OpenCV installation"
    exit 1
fi

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

print_header "STEP 3: PREPARING RESULTS DIRECTORY"

rm -rf results results_analysis
mkdir -p results

echo "experiment,scene,images,detector,threshold,blend_mode,keypoints1,keypoints2,matches,inliers,inlier_ratio,status" > results/metrics.csv

print_status "Results directory cleaned and ready"

print_header "STEP 4: RUNNING EXPERIMENTS"

declare -a scenes=("indoor_scene" "outdoor_scene1" "outdoor_scene2")
declare -a detectors=("orb" "akaze" "sift")
declare -a blends=("simple" "feather" "multiband")
declare -a thresholds=(1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0)

NUM_SCENES=${#scenes[@]}
NUM_DETECTORS=${#detectors[@]}
NUM_PAIRS=3
NUM_BLENDS=${#blends[@]}
NUM_THRESHOLDS=${#thresholds[@]}

TOTAL_EXPECTED=$((NUM_SCENES * (NUM_DETECTORS * NUM_PAIRS + NUM_THRESHOLDS + NUM_BLENDS + NUM_DETECTORS)))

echo ""
echo "This will test:"
echo "  • 3 detectors (ORB, AKAZE, SIFT)"
echo "  • 3 scenes (indoor, outdoor1, outdoor2)"
echo "  • All image pairs (1-2, 2-3, 1-3)"
echo "  • 20 RANSAC thresholds (1.0 to 20.0)"
echo "  • 3 blending modes (simple, feather, multiband)"
echo "  • Multi-image stitching (all 3 images)"
echo ""
echo "Total: ${TOTAL_EXPECTED} experiments"
echo ""

TOTAL=0
SUCCESS=0
FAILED=0

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

    echo -ne "\r[${TOTAL}/${TOTAL_EXPECTED}] Testing: ${label}...                    "

    local exp_output=$(./scripts/run_panorama.sh --stitch "$img1" "$img2" \
        --detector "$detector" \
        --ransac-threshold "$threshold" \
        --blend-mode "$blend" \
        --output "$output" 2>&1)

    if echo "$exp_output" | grep -q "Panorama saved"; then
        SUCCESS=$((SUCCESS + 1))

        local keypoints_line=$(echo "$exp_output" | grep "Detected")
        local keypoints1=$(echo "$keypoints_line" | grep -oP 'Detected \K\d+' | head -1)
        local keypoints2=$(echo "$keypoints_line" | grep -oP 'and \K\d+' | head -1)
        local matches=$(echo "$exp_output" | grep -oP 'Found \K\d+(?= good matches)')
        local inliers=$(echo "$exp_output" | grep -oP 'RANSAC found \K\d+(?= inliers)')
        local ratio=$(echo "$exp_output" | grep -oP 'inliers \(\K[0-9.]+(?=%\))')

        keypoints1=${keypoints1:-0}
        keypoints2=${keypoints2:-0}
        matches=${matches:-0}
        inliers=${inliers:-0}
        ratio=${ratio:-0}

        local img_pair="$(basename $img1)-$(basename $img2)"

        echo "$label,$scene,$img_pair,$detector,$threshold,$blend,$keypoints1,$keypoints2,$matches,$inliers,$ratio,SUCCESS" >> results/metrics.csv
    else
        FAILED=$((FAILED + 1))

        local keypoints_line=$(echo "$exp_output" | grep "Detected")
        local keypoints1=$(echo "$keypoints_line" | grep -oP 'Detected \K\d+' | head -1)
        local keypoints2=$(echo "$keypoints_line" | grep -oP 'and \K\d+' | head -1)
        local matches=$(echo "$exp_output" | grep -oP 'Found \K\d+(?= good matches)')

        keypoints1=${keypoints1:-0}
        keypoints2=${keypoints2:-0}
        matches=${matches:-0}

        local img_pair="$(basename $img1)-$(basename $img2)"
        echo "$label,$scene,$img_pair,$detector,$threshold,$blend,$keypoints1,$keypoints2,$matches,0,0,FAILED" >> results/metrics.csv
    fi
}

run_multi_experiment() {
    local scene=$1
    local img1=$2
    local img2=$3
    local img3=$4
    local detector=$5
    local output=$6
    local label=$7

    TOTAL=$((TOTAL + 1))

    echo -ne "\r[${TOTAL}/${TOTAL_EXPECTED}] Testing: ${label}...                    "

    local exp_output=$(./scripts/run_panorama.sh --stitch-multiple "$img1" "$img2" "$img3" \
        --detector "$detector" \
        --output "$output" 2>&1)

    if echo "$exp_output" | grep -q "Panorama saved\|created successfully"; then
        SUCCESS=$((SUCCESS + 1))

        echo "$label,$scene,multi-image,$detector,3.0,feather,0,0,0,0,0,SUCCESS" >> results/metrics.csv
    else
        FAILED=$((FAILED + 1))
        echo "$label,$scene,multi-image,$detector,3.0,feather,0,0,0,0,0,FAILED" >> results/metrics.csv
    fi
}

echo -e "${MAGENTA}Running detector comparison...${NC}"
for scene in "${scenes[@]}"; do
    for detector in "${detectors[@]}"; do
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

echo -e "\n${MAGENTA}Running RANSAC analysis...${NC}"
for scene in "${scenes[@]}"; do
    for threshold in "${thresholds[@]}"; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" "datasets/$scene/img2.jpg" \
            "orb" "$threshold" feather \
            "results/${scene}_ransac_${threshold}.jpg" \
            "$scene RANSAC-$threshold"
    done
done

echo -e "\n${MAGENTA}Running blending comparison...${NC}"
for scene in "${scenes[@]}"; do
    for blend in "${blends[@]}"; do
        run_experiment "$scene" \
            "datasets/$scene/img1.jpg" "datasets/$scene/img2.jpg" \
            "orb" 3.0 "$blend" \
            "results/${scene}_blend_${blend}.jpg" \
            "$scene blend-$blend"
    done
done

echo -e "\n${MAGENTA}Running multi-image stitching...${NC}"
for scene in "${scenes[@]}"; do
    for detector in "${detectors[@]}"; do
        run_multi_experiment "$scene" \
            "datasets/$scene/img1.jpg" \
            "datasets/$scene/img2.jpg" \
            "datasets/$scene/img3.jpg" \
            "$detector" \
            "results/${scene}_multi_${detector}.jpg" \
            "$scene multi-$detector"
    done
done

echo ""
print_status "Experiments completed: $SUCCESS successful, $FAILED failed"

print_header "STEP 5: ANALYZING & ORGANIZING RESULTS"

if python3 scripts/analysis_pipeline.py; then
    print_status "Results analyzed and organized"
else
    print_warning "Analysis script had issues, but continuing..."
fi

print_header "EXPERIMENT COMPLETE!"

echo ""
echo -e "${GREEN}Summary:${NC}"
echo "  • Total experiments run: $TOTAL"
echo "  • Successful stitches: $SUCCESS"
echo "  • Failed stitches: $FAILED"
echo "  • Success rate: $((SUCCESS * 100 / TOTAL))%"
echo ""
echo -e "${GREEN}Generated outputs:${NC}"
echo "  • results/           - Raw experiment outputs and panoramas"
echo "  • results_analysis/  - Organized results with metrics analysis"
echo ""
echo -e "${YELLOW}To view results:${NC}"
echo "  • Panoramas: results/*.jpg"
echo "  • Metrics chart: results_analysis/metrics_analysis.png"
echo "  • CSV data: results/metrics.csv"
echo ""
echo -e "${BLUE}For the report:${NC}"
echo "  • All quantitative data is in results_analysis/"
echo "  • Visualizations are in results_analysis/visualizations/"
echo "  • Raw data is in results/ if needed"
echo ""

exit 0
