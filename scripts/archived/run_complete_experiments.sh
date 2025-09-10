#!/bin/bash

# Complete experiment runner for Visual Computing Assignment 1
# This script:
# 1. Builds the project
# 2. Runs all experiments using --experiment-mode
# 3. Generates all required plots
# 4. Creates summary tables

echo "=========================================="
echo "Visual Computing Assignment 1 - Experiments"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Create results directory
mkdir -p results
echo -e "${GREEN}✓${NC} Created results directory"

# Step 1: Build the project
echo -e "\n${YELLOW}Step 1:${NC} Building project..."
if ./scripts/build.sh; then
    echo -e "${GREEN}✓${NC} Build successful"
else
    echo -e "${RED}✗${NC} Build failed"
    exit 1
fi

# Step 2: Check datasets exist
echo -e "\n${YELLOW}Step 2:${NC} Checking datasets..."
DATASETS_OK=true

for scene in indoor_scene outdoor_scene1 outdoor_scene2; do
    if [ -d "datasets/$scene" ]; then
        count=$(ls datasets/$scene/*.jpg 2>/dev/null | wc -l)
        if [ $count -ge 2 ]; then
            echo -e "${GREEN}✓${NC} $scene: $count images found"
        else
            echo -e "${RED}✗${NC} $scene: Not enough images (found $count, need at least 2)"
            DATASETS_OK=false
        fi
    else
        echo -e "${RED}✗${NC} $scene: Directory not found"
        DATASETS_OK=false
    fi
done

if [ "$DATASETS_OK" = false ]; then
    echo -e "${YELLOW}Warning:${NC} Some datasets are missing. Results may be incomplete."
fi

# Step 3: Run experiments using C++ experiment mode
echo -e "\n${YELLOW}Step 3:${NC} Running experiments..."
echo "This will test:"
echo "  - ORB vs AKAZE detectors"
echo "  - RANSAC thresholds: 1.0, 2.0, 3.0, 4.0, 5.0"
echo "  - Blending modes: simple, feather, multiband"
echo "  - All 3 scenes (indoor, outdoor1, outdoor2)"
echo ""

if ./build/panorama_stitcher --experiment-mode; then
    echo -e "${GREEN}✓${NC} Experiments completed successfully"
else
    echo -e "${RED}✗${NC} Experiments failed"
    exit 1
fi

# Step 4: Generate plots from the data
echo -e "\n${YELLOW}Step 4:${NC} Generating plots and analysis..."

# Check if Python and required libraries are available
if command -v python3 &> /dev/null; then
    # Check for required Python packages
    python3 -c "import pandas, matplotlib, numpy" 2>/dev/null
    if [ $? -eq 0 ]; then
        if python3 scripts/generate_plots.py results; then
            echo -e "${GREEN}✓${NC} Plots generated successfully"
        else
            echo -e "${YELLOW}Warning:${NC} Plot generation encountered issues"
        fi
    else
        echo -e "${YELLOW}Warning:${NC} Python packages missing. Install with:"
        echo "  pip install pandas matplotlib numpy"
    fi
else
    echo -e "${YELLOW}Warning:${NC} Python3 not found. Skipping plot generation."
fi

# Step 5: Parse results for tables
echo -e "\n${YELLOW}Step 5:${NC} Generating result tables..."

if [ -f "scripts/parse_results.py" ]; then
    if python3 scripts/parse_results.py > results/tables.txt 2>/dev/null; then
        echo -e "${GREEN}✓${NC} Tables generated: results/tables.txt"
        echo ""
        echo "=== Quick Results Preview ==="
        head -n 30 results/tables.txt
    else
        echo -e "${YELLOW}Warning:${NC} Table generation failed"
    fi
fi

# Step 6: Summary
echo -e "\n${YELLOW}=========================================="
echo -e "Experiment Summary"
echo -e "==========================================${NC}"

echo -e "\n${GREEN}Generated Files:${NC}"
for file in results/*.csv results/*.png results/*.jpg results/*.txt; do
    if [ -f "$file" ]; then
        size=$(du -h "$file" | cut -f1)
        echo "  ✓ $(basename $file) ($size)"
    fi
done

# Count results
if [ -f "results/metrics.csv" ]; then
    num_experiments=$(tail -n +2 results/metrics.csv | wc -l)
    echo -e "\n${GREEN}Statistics:${NC}"
    echo "  - Total experiments run: $num_experiments"
    
    # Extract some key metrics
    if command -v python3 &> /dev/null; then
        python3 << 'EOF'
import csv
import sys

try:
    with open('results/metrics.csv', 'r') as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        
        if rows:
            # Count successful stitches (where inliers > 30)
            successful = sum(1 for r in rows if float(r.get('num_inliers', 0)) > 30)
            
            # Average inlier ratio
            ratios = [float(r.get('inlier_ratio', 0)) for r in rows if r.get('inlier_ratio')]
            avg_ratio = sum(ratios) / len(ratios) if ratios else 0
            
            # Average times
            det_times = [float(r.get('detection_time', 0)) for r in rows if r.get('detection_time')]
            avg_det_time = sum(det_times) / len(det_times) if det_times else 0
            
            print(f"  - Successful stitches: {successful}/{len(rows)} ({100*successful/len(rows):.1f}%)")
            print(f"  - Average inlier ratio: {avg_ratio*100:.1f}%")
            print(f"  - Average detection time: {avg_det_time:.1f} ms")
except Exception as e:
    pass
EOF
    fi
fi

echo -e "\n${GREEN}✅ Experiment pipeline completed!${NC}"
echo ""
echo "Next steps:"
echo "1. Review the generated plots in results/"
echo "2. Check results/tables.txt for formatted tables"
echo "3. Use results/metrics.csv for detailed analysis"
echo "4. Add subjective visual quality assessment to your report"
echo ""
echo "To view results:"
echo "  - Images: eog results/*.png"
echo "  - Tables: cat results/tables.txt"
echo "  - Metrics: less results/metrics.csv"