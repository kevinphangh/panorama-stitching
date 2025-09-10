#!/bin/bash

###############################################################################
#                    SETUP VALIDATION SCRIPT                                 #
###############################################################################
# This script validates that everything is properly set up                   #
###############################################################################

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "      VALIDATING EXPERIMENT SETUP"
echo "=========================================="
echo ""

ERRORS=0
WARNINGS=0

# Check 1: Main script exists and is executable
echo -n "Checking main experiment script... "
if [ -f "RUN_EXPERIMENTS.sh" ] && [ -x "RUN_EXPERIMENTS.sh" ]; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: RUN_EXPERIMENTS.sh not found or not executable"
    echo "  Fix: chmod +x RUN_EXPERIMENTS.sh"
    ERRORS=$((ERRORS + 1))
fi

# Check 2: Build script exists
echo -n "Checking build script... "
if [ -f "scripts/build.sh" ] && [ -x "scripts/build.sh" ]; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: scripts/build.sh not found"
    ERRORS=$((ERRORS + 1))
fi

# Check 3: Analysis script exists
echo -n "Checking analysis script... "
if [ -f "scripts/analyze_and_organize.py" ]; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: scripts/analyze_and_organize.py not found"
    ERRORS=$((ERRORS + 1))
fi

# Check 4: Python3 is available
echo -n "Checking Python 3... "
if command -v python3 &> /dev/null; then
    echo -e "${GREEN}✓${NC} ($(python3 --version 2>&1))"
else
    echo -e "${YELLOW}⚠${NC}"
    echo "  WARNING: python3 not found (needed for analysis)"
    WARNINGS=$((WARNINGS + 1))
fi

# Check 5: CMake is available
echo -n "Checking CMake... "
if command -v cmake &> /dev/null; then
    echo -e "${GREEN}✓${NC} ($(cmake --version | head -1))"
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: cmake not found (needed for building)"
    ERRORS=$((ERRORS + 1))
fi

# Check 6: C++ compiler
echo -n "Checking C++ compiler... "
if command -v g++ &> /dev/null; then
    echo -e "${GREEN}✓${NC} ($(g++ --version | head -1))"
elif command -v clang++ &> /dev/null; then
    echo -e "${GREEN}✓${NC} ($(clang++ --version | head -1))"
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: No C++ compiler found"
    ERRORS=$((ERRORS + 1))
fi

# Check 7: OpenCV installation (check if pkg-config knows about it)
echo -n "Checking OpenCV... "
if pkg-config --exists opencv4 2>/dev/null || pkg-config --exists opencv 2>/dev/null; then
    if pkg-config --exists opencv4 2>/dev/null; then
        version=$(pkg-config --modversion opencv4)
    else
        version=$(pkg-config --modversion opencv)
    fi
    echo -e "${GREEN}✓${NC} (version $version)"
else
    echo -e "${YELLOW}⚠${NC}"
    echo "  WARNING: OpenCV not detected via pkg-config"
    echo "  (May still work if installed differently)"
    WARNINGS=$((WARNINGS + 1))
fi

# Check 8: Datasets
echo -n "Checking datasets... "
DATASET_OK=true
DATASET_COUNT=0
for scene in indoor_scene outdoor_scene1 outdoor_scene2; do
    if [ -d "datasets/$scene" ]; then
        count=$(ls datasets/$scene/*.jpg 2>/dev/null | wc -l)
        if [ $count -eq 3 ]; then
            DATASET_COUNT=$((DATASET_COUNT + 1))
        fi
    fi
done

if [ $DATASET_COUNT -eq 3 ]; then
    echo -e "${GREEN}✓${NC} (all 3 scenes ready)"
elif [ $DATASET_COUNT -gt 0 ]; then
    echo -e "${YELLOW}⚠${NC}"
    echo "  WARNING: Only $DATASET_COUNT/3 scenes have complete datasets"
    WARNINGS=$((WARNINGS + 1))
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: No complete datasets found"
    echo "  Need: datasets/*/img1.jpg, img2.jpg, img3.jpg"
    ERRORS=$((ERRORS + 1))
fi

# Check 9: Build directory
echo -n "Checking build status... "
if [ -f "build/panorama_stitcher" ]; then
    echo -e "${GREEN}✓${NC} (executable exists)"
else
    echo -e "${YELLOW}⚠${NC}"
    echo "  WARNING: Not built yet (will build automatically)"
    WARNINGS=$((WARNINGS + 1))
fi

# Check 10: Write permissions
echo -n "Checking write permissions... "
if touch .test_write_permission 2>/dev/null; then
    rm .test_write_permission
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    echo "  ERROR: Cannot write to current directory"
    ERRORS=$((ERRORS + 1))
fi

# Summary
echo ""
echo "=========================================="
if [ $ERRORS -eq 0 ]; then
    if [ $WARNINGS -eq 0 ]; then
        echo -e "${GREEN}VALIDATION PASSED!${NC}"
        echo ""
        echo "Everything is ready! Run:"
        echo "  ./RUN_EXPERIMENTS.sh"
    else
        echo -e "${YELLOW}VALIDATION PASSED WITH WARNINGS${NC}"
        echo ""
        echo "Setup is functional but has $WARNINGS warning(s)"
        echo "You can still run:"
        echo "  ./RUN_EXPERIMENTS.sh"
    fi
else
    echo -e "${RED}VALIDATION FAILED!${NC}"
    echo ""
    echo "Found $ERRORS error(s) and $WARNINGS warning(s)"
    echo "Please fix the errors before running experiments"
fi
echo "=========================================="
echo ""

exit $ERRORS