#!/bin/bash

###############################################################################
#                    FEATURE COUNT SCALING ANALYSIS                          #
#              Tests performance with different max_features values          #
###############################################################################

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  FEATURE SCALING EXPERIMENT${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Create results directory
mkdir -p results/feature_scaling

# Test different feature counts
declare -a feature_counts=(500 1000 2000 5000 10000 20000 50000)
declare -a scenes=("indoor_scene" "outdoor_scene1")

# CSV header
echo "scene,detector,max_features,keypoints_detected,matches,inliers,time_ms,success" > results/feature_scaling/metrics.csv

# Function to run experiment
run_feature_test() {
    local scene=$1
    local detector=$2
    local max_feat=$3
    
    echo -ne "\rTesting: $scene $detector max_features=$max_feat...     "
    
    output=$(./build/panorama_stitcher --stitch \
        "datasets/$scene/img1.jpg" "datasets/$scene/img2.jpg" \
        --detector "$detector" \
        --max-features "$max_feat" \
        --ransac-threshold 3.0 \
        --blend-mode feather \
        --output "results/feature_scaling/${scene}_${detector}_${max_feat}.jpg" 2>&1)
    
    if echo "$output" | grep -q "Panorama saved"; then
        keypoints=$(echo "$output" | grep -oP 'Detected \K\d+' | head -1)
        matches=$(echo "$output" | grep -oP 'Found \K\d+(?= good matches)')
        inliers=$(echo "$output" | grep -oP 'RANSAC found \K\d+(?= inliers)')
        time=$(echo "$output" | grep -oP 'Total time: \K[0-9.]+(?= ms)')
        
        echo "$scene,$detector,$max_feat,$keypoints,$matches,$inliers,$time,SUCCESS" >> results/feature_scaling/metrics.csv
    else
        echo "$scene,$detector,$max_feat,0,0,0,0,FAILED" >> results/feature_scaling/metrics.csv
    fi
}

# Run tests
total=0
for scene in "${scenes[@]}"; do
    for detector in orb akaze; do
        for max_feat in "${feature_counts[@]}"; do
            run_feature_test "$scene" "$detector" "$max_feat"
            total=$((total + 1))
        done
    done
done

echo ""
echo -e "${GREEN}✓ Completed $total feature scaling tests${NC}"
echo ""

# Generate analysis
cat > results/feature_scaling/analysis.py << 'EOF'
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load data
df = pd.read_csv('results/feature_scaling/metrics.csv')
df = df[df['success'] == 'SUCCESS']

# Create plots
fig, axes = plt.subplots(2, 2, figsize=(12, 10))
fig.suptitle('Feature Count Scaling Analysis', fontsize=16)

# Plot 1: Keypoints detected vs requested
ax = axes[0, 0]
for detector in df['detector'].unique():
    data = df[df['detector'] == detector].groupby('max_features')['keypoints_detected'].mean()
    ax.plot(data.index, data.values, marker='o', label=detector.upper())
ax.plot([500, 50000], [500, 50000], 'k--', alpha=0.3, label='Ideal')
ax.set_xlabel('Max Features Requested')
ax.set_ylabel('Keypoints Detected')
ax.set_xscale('log')
ax.set_title('Actual vs Requested Features')
ax.legend()
ax.grid(True, alpha=0.3)

# Plot 2: Processing time
ax = axes[0, 1]
for detector in df['detector'].unique():
    data = df[df['detector'] == detector].groupby('max_features')['time_ms'].mean()
    ax.plot(data.index, data.values, marker='o', label=detector.upper())
ax.set_xlabel('Max Features')
ax.set_ylabel('Processing Time (ms)')
ax.set_xscale('log')
ax.set_title('Processing Time Scaling')
ax.legend()
ax.grid(True, alpha=0.3)

# Plot 3: Inlier ratio
ax = axes[1, 0]
for detector in df['detector'].unique():
    data = df[df['detector'] == detector]
    grouped = data.groupby('max_features').agg({
        'inliers': 'mean',
        'matches': 'mean'
    })
    ratio = (grouped['inliers'] / grouped['matches'] * 100).fillna(0)
    ax.plot(ratio.index, ratio.values, marker='o', label=detector.upper())
ax.set_xlabel('Max Features')
ax.set_ylabel('Inlier Ratio (%)')
ax.set_xscale('log')
ax.set_title('Match Quality vs Feature Count')
ax.legend()
ax.grid(True, alpha=0.3)

# Plot 4: Efficiency (inliers per ms)
ax = axes[1, 1]
for detector in df['detector'].unique():
    data = df[df['detector'] == detector].groupby('max_features').agg({
        'inliers': 'mean',
        'time_ms': 'mean'
    })
    efficiency = data['inliers'] / data['time_ms']
    ax.plot(efficiency.index, efficiency.values, marker='o', label=detector.upper())
ax.set_xlabel('Max Features')
ax.set_ylabel('Inliers per ms')
ax.set_xscale('log')
ax.set_title('Efficiency (Quality/Speed Trade-off)')
ax.legend()
ax.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('results/feature_scaling/scaling_analysis.png', dpi=150, bbox_inches='tight')
print("Analysis saved to results/feature_scaling/scaling_analysis.png")

# Print summary
print("\n=== Feature Scaling Summary ===")
for detector in df['detector'].unique():
    det_data = df[df['detector'] == detector]
    print(f"\n{detector.upper()}:")
    print(f"  Max keypoints detected: {det_data['keypoints_detected'].max()}")
    print(f"  Best efficiency at: {det_data.loc[det_data['inliers']/det_data['time_ms'].idxmax(), 'max_features']} features")
    print(f"  Time range: {det_data['time_ms'].min():.1f} - {det_data['time_ms'].max():.1f} ms")
EOF

python3 results/feature_scaling/analysis.py 2>/dev/null || echo "Install matplotlib for plots: pip install matplotlib"

echo -e "${BLUE}Results saved in results/feature_scaling/${NC}"
echo "View metrics: results/feature_scaling/metrics.csv"
echo "View plots: results/feature_scaling/scaling_analysis.png"