#!/bin/bash

###############################################################################
#                          VIEW EXPERIMENT RESULTS                           #
#              Multiple ways to view HTML results based on your setup        #
###############################################################################

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}     VIEWING EXPERIMENT RESULTS${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if results exist
if [ ! -d "results_organized" ] && [ ! -d "results" ]; then
    echo -e "${YELLOW}⚠ No results found. Run ./RUN_EXPERIMENTS.sh first!${NC}"
    exit 1
fi

# Determine which results are available
if [ -d "results_organized" ]; then
    MAIN_RESULTS="results_organized/index.html"
else
    MAIN_RESULTS=""
fi

if [ -f "results/quantitative_report.html" ]; then
    QUANT_REPORT="results/quantitative_report.html"
else
    QUANT_REPORT=""
fi

echo "Choose how to view results:"
echo ""
echo "1) Python HTTP Server (recommended - works everywhere)"
echo "2) Open in default browser"
echo "3) VSCode Preview (if running in VSCode)"
echo "4) Show file paths for manual opening"
echo ""
read -p "Enter choice (1-4): " choice

case $choice in
    1)
        echo -e "\n${GREEN}Starting Python HTTP server...${NC}"
        if [ -n "$MAIN_RESULTS" ]; then
            echo -e "${YELLOW}Gallery: http://localhost:8000/$MAIN_RESULTS${NC}"
        fi
        if [ -n "$QUANT_REPORT" ]; then
            echo -e "${YELLOW}Report: http://localhost:8000/$QUANT_REPORT${NC}"
        fi
        echo -e "${YELLOW}Press Ctrl+C to stop the server${NC}\n"
        
        # Try Python 3 first, then Python 2
        if command -v python3 &> /dev/null; then
            python3 -m http.server 8000
        elif command -v python &> /dev/null; then
            python -m SimpleHTTPServer 8000
        else
            echo -e "${YELLOW}Python not found. Please install Python or use another option.${NC}"
        fi
        ;;
        
    2)
        echo -e "\n${GREEN}Opening in default browser...${NC}"
        
        # Try different commands based on OS
        FILE_TO_OPEN="${MAIN_RESULTS:-$QUANT_REPORT}"
        if [ -z "$FILE_TO_OPEN" ]; then
            echo -e "${YELLOW}No HTML files found yet. Run experiments first.${NC}"
            exit 1
        fi
        
        if command -v xdg-open &> /dev/null; then
            xdg-open "$FILE_TO_OPEN"
        elif command -v open &> /dev/null; then
            open "$FILE_TO_OPEN"
        elif command -v start &> /dev/null; then
            start "$FILE_TO_OPEN"
        else
            echo -e "${YELLOW}Could not detect browser command. Use option 4 for manual paths.${NC}"
        fi
        ;;
        
    3)
        echo -e "\n${GREEN}VSCode Preview Instructions:${NC}"
        echo ""
        echo "Option A - Live Server Extension:"
        echo "  1. Install 'Live Server' extension by Ritwick Dey"
        echo "  2. Right-click on results_organized/index.html"
        echo "  3. Select 'Open with Live Server'"
        echo ""
        echo "Option B - Simple Browser:"
        echo "  1. Press Ctrl+Shift+P (Cmd+Shift+P on Mac)"
        echo "  2. Type 'Simple Browser'"
        echo "  3. Enter: $(pwd)/results_organized/index.html"
        echo ""
        echo "Option C - Preview:"
        echo "  1. Open results_organized/index.html in editor"
        echo "  2. Press Ctrl+Shift+V (Cmd+Shift+V on Mac)"
        ;;
        
    4)
        echo -e "\n${GREEN}File paths for manual opening:${NC}"
        echo ""
        if [ -n "$MAIN_RESULTS" ]; then
            echo "Main gallery:"
            echo "  file://$(pwd)/$MAIN_RESULTS"
            echo ""
        fi
        if [ -n "$QUANT_REPORT" ]; then
            echo "Quantitative report:"
            echo "  file://$(pwd)/$QUANT_REPORT"
            echo ""
        fi
        if [ -z "$MAIN_RESULTS" ] && [ -z "$QUANT_REPORT" ]; then
            echo -e "${YELLOW}No HTML files generated yet. Run experiments first.${NC}"
        else
            echo "Copy and paste these URLs into any browser."
        fi
        ;;
        
    *)
        echo -e "${YELLOW}Invalid choice. Please run the script again.${NC}"
        exit 1
        ;;
esac