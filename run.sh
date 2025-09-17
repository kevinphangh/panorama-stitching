#!/bin/bash

###############################################################################
#                                                                             #
#                    🖼️  PANORAMA STITCHING - MAIN RUNNER  🖼️                 #
#                                                                             #
#     This is the ONLY script you need to run!                              #
#     It will build, test, and show you everything.                         #
#                                                                             #
###############################################################################

# Colors for beautiful output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Clear screen for fresh start
clear

echo -e "${CYAN}${BOLD}"
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                                                              ║"
echo "║            PANORAMA STITCHING SYSTEM                        ║"
echo "║            Computer Vision Assignment                       ║"
echo "║                                                              ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

echo ""
echo -e "${YELLOW}Welcome! This script will:${NC}"
echo "  1. Build the C++ project"
echo "  2. Run a quick test to verify everything works"
echo "  3. Give you options for what to do next"
echo ""
echo -e "${GREEN}Press Enter to start...${NC}"
read

# Step 1: Check dependencies
echo -e "\n${BLUE}═══ Step 1: Checking Dependencies ═══${NC}\n"

check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} $1 found"
        return 0
    else
        echo -e "  ${RED}✗${NC} $1 not found"
        return 1
    fi
}

MISSING_DEPS=0
check_command cmake || MISSING_DEPS=1
check_command make || MISSING_DEPS=1
check_command g++ || MISSING_DEPS=1
check_command python3 || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo -e "\n${RED}Some dependencies are missing!${NC}"
    echo "Please install them:"
    echo -e "${YELLOW}  Ubuntu/Debian: sudo apt-get install cmake g++ libopencv-dev python3-pip${NC}"
    echo -e "${YELLOW}  macOS: brew install cmake opencv python3${NC}"
    exit 1
fi

# Step 2: Build the project
echo -e "\n${BLUE}═══ Step 2: Building Project ═══${NC}\n"

mkdir -p build
cd build

echo "Running cmake..."
if cmake .. -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} CMake configuration successful"
else
    echo -e "${RED}✗${NC} CMake configuration failed"
    echo "Please check that OpenCV is installed"
    exit 1
fi

echo "Compiling..."
if make -j$(nproc) > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Build successful!"
else
    echo -e "${RED}✗${NC} Build failed"
    echo "Please check the error messages above"
    exit 1
fi

cd ..

# Step 3: Quick test
echo -e "\n${BLUE}═══ Step 3: Quick Test ═══${NC}\n"

echo "Testing panorama stitching with indoor scene..."
if ./scripts/run_panorama.sh --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --output test_panorama.jpg 2>&1 | grep -q "Panorama saved"; then
    echo -e "${GREEN}✓${NC} Test successful! Panorama created: test_panorama.jpg"
else
    echo -e "${YELLOW}⚠${NC} Test had issues, but continuing..."
fi

# Step 4: Main menu
echo -e "\n${CYAN}${BOLD}═══════════════════════════════════════════════════${NC}"
echo -e "${CYAN}${BOLD}           What would you like to do?              ${NC}"
echo -e "${CYAN}${BOLD}═══════════════════════════════════════════════════${NC}\n"

echo -e "${BOLD}1)${NC} 🚀 Run ALL experiments (48 tests, ~5 minutes)"
echo -e "${BOLD}2)${NC} ⚡ Run quick demo (3 tests, ~30 seconds)"
echo -e "${BOLD}3)${NC} 🎯 Test with your own images"
echo -e "${BOLD}4)${NC} 📊 View previous results (if available)"
echo -e "${BOLD}5)${NC} 📖 Show project documentation"
echo -e "${BOLD}6)${NC} 🔧 Clean and rebuild everything"
echo -e "${BOLD}7)${NC} ❌ Exit"

echo ""
echo -ne "${GREEN}Enter your choice (1-7):${NC} "
read choice

case $choice in
    1)
        echo -e "\n${YELLOW}Running all experiments...${NC}"
        echo "This will test multiple detectors, thresholds, and blending modes."
        echo "Estimated time: 5-10 minutes"
        echo ""
        ./scripts/run-experiments.sh
        echo -e "\n${GREEN}Complete! Check results_analysis/analysis_report.html${NC}"
        ;;

    2)
        echo -e "\n${YELLOW}Running quick demo...${NC}"
        mkdir -p results

        echo "Test 1: Indoor scene with ORB..."
        ./scripts/run_panorama.sh --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
            --detector orb --output results/demo_indoor_orb.jpg

        echo -e "\nTest 2: Indoor scene with AKAZE..."
        ./scripts/run_panorama.sh --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg \
            --detector akaze --output results/demo_indoor_akaze.jpg

        echo -e "\nTest 3: Outdoor scene..."
        ./scripts/run_panorama.sh --stitch datasets/outdoor_scene1/img1.jpg datasets/outdoor_scene1/img2.jpg \
            --detector orb --output results/demo_outdoor.jpg

        echo -e "\n${GREEN}Demo complete! Check the results/ folder${NC}"
        ;;

    3)
        echo -e "\n${CYAN}Test with your own images${NC}"
        echo "Make sure your images:"
        echo "  • Have 30-40% overlap"
        echo "  • Are taken from the same viewpoint height"
        echo "  • Have similar lighting"
        echo ""
        echo "Enter path to first image:"
        read img1
        echo "Enter path to second image:"
        read img2

        if [ -f "$img1" ] && [ -f "$img2" ]; then
            ./scripts/run_panorama.sh --stitch "$img1" "$img2" --output custom_panorama.jpg
            echo -e "\n${GREEN}Result saved as: custom_panorama.jpg${NC}"
        else
            echo -e "${RED}Error: One or both image files not found${NC}"
        fi
        ;;

    4)
        if [ -f "results_analysis/analysis_report.html" ]; then
            echo -e "\n${YELLOW}Opening results...${NC}"
            echo "Starting web server..."
            echo -e "${GREEN}Visit: http://localhost:8000/analysis_report.html${NC}"
            cd results_analysis && python3 -m http.server 8000
        else
            echo -e "\n${YELLOW}No results found yet.${NC}"
            echo "Run option 1 or 2 first to generate results."
        fi
        ;;

    5)
        echo -e "\n${CYAN}${BOLD}PROJECT DOCUMENTATION${NC}\n"
        echo "📁 Project Structure:"
        echo "  • src/          - C++ source code"
        echo "  • datasets/     - Test images (3 scenes)"
        echo "  • scripts/      - Helper scripts"
        echo "  • results/      - Output panoramas"
        echo ""
        echo "🔧 Key Components:"
        echo "  • Feature Detection: ORB (fast) and AKAZE (accurate)"
        echo "  • Matching: Brute-force with Lowe's ratio test"
        echo "  • RANSAC: Removes outlier matches"
        echo "  • Blending: Simple, Feather, or Multiband"
        echo ""
        echo "📖 For more details, see README.md"
        ;;

    6)
        echo -e "\n${YELLOW}Cleaning and rebuilding...${NC}"
        rm -rf build results results_analysis
        mkdir -p build
        cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)
        cd ..
        echo -e "${GREEN}Clean rebuild complete!${NC}"
        ;;

    7)
        echo -e "\n${GREEN}Goodbye!${NC}"
        exit 0
        ;;

    *)
        echo -e "\n${RED}Invalid choice${NC}"
        ;;
esac

echo -e "\n${CYAN}Run ${BOLD}./run.sh${NC}${CYAN} again to see the menu${NC}"