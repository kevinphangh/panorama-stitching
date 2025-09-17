#!/bin/bash

###############################################################################
#                          VIEW RESULTS SCRIPT                               #
#         Interactive viewer for panorama stitching experiment results       #
###############################################################################

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo ""
echo -e "${BLUE}╔═══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       PANORAMA STITCHING RESULTS VIEWER              ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if results exist
if [ ! -d "results" ]; then
    echo -e "${RED}✗ No results directory found!${NC}"
    echo "  Please run ./scripts/run-experiments.sh first"
    exit 1
fi

# Run organization if not done yet
if [ ! -f "results/index.html" ]; then
    echo -e "${YELLOW}⚠ Results not organized yet. Running organization...${NC}"
    python3 scripts/analysis_pipeline.py --enhance
fi

echo -e "${GREEN}Available viewing options:${NC}"
echo ""
echo "  1) 🌐 Open in default browser"
echo "  2) 🔥 Firefox"
echo "  3) 🌍 Chrome/Chromium"
echo "  4) 🖥️  VSCode Preview"
echo "  5) 🐍 Python HTTP Server (access from any device)"
echo "  6) 📊 View raw metrics (CSV)"
echo "  7) 📄 View PDF report"
echo "  8) 🔄 Re-organize results"
echo "  9) ❌ Exit"
echo ""

read -p "Select option (1-9): " choice

case $choice in
    1)
        echo -e "${GREEN}Opening in default browser...${NC}"
        if command -v xdg-open > /dev/null; then
            xdg-open results/index.html
        elif command -v open > /dev/null; then
            open results/index.html
        else
            echo -e "${YELLOW}Could not detect default browser${NC}"
        fi
        ;;
    2)
        echo -e "${GREEN}Opening in Firefox...${NC}"
        firefox results/index.html &
        ;;
    3)
        echo -e "${GREEN}Opening in Chrome...${NC}"
        if command -v google-chrome > /dev/null; then
            google-chrome results/index.html &
        elif command -v chromium > /dev/null; then
            chromium results/index.html &
        else
            echo -e "${YELLOW}Chrome/Chromium not found${NC}"
        fi
        ;;
    4)
        echo -e "${GREEN}Opening in VSCode...${NC}"
        if command -v code > /dev/null; then
            code results/index.html
            echo "Use the 'Live Preview' extension or Ctrl+Shift+V for preview"
        else
            echo -e "${YELLOW}VSCode not found${NC}"
        fi
        ;;
    5)
        echo -e "${GREEN}Starting Python HTTP server...${NC}"
        echo -e "${YELLOW}Access from any device on network at:${NC}"
        echo ""

        # Get IP address
        if command -v ip > /dev/null; then
            IP=$(ip addr show | grep -oP '(?<=inet\s)\d+(\.\d+){3}' | grep -v 127.0.0.1 | head -1)
        else
            IP="your-ip-address"
        fi

        echo -e "  Local:   ${BLUE}http://localhost:8000/results/${NC}"
        echo -e "  Network: ${BLUE}http://$IP:8000/results/${NC}"
        echo ""
        echo -e "${YELLOW}Press Ctrl+C to stop the server${NC}"
        echo ""

        cd $(dirname $0)
        python3 -m http.server 8000
        ;;
    6)
        echo -e "${GREEN}Viewing metrics...${NC}"
        if [ -f "results/metrics.csv" ]; then
            # Show first 10 lines with column
            echo ""
            echo "First 10 experiments:"
            echo "─────────────────────────────────────────"
            head -n 11 results/metrics.csv | column -t -s ','
            echo "─────────────────────────────────────────"
            echo ""
            echo "Total lines: $(wc -l < results/metrics.csv)"
            echo "Full file: results/metrics.csv"
        else
            echo -e "${RED}metrics.csv not found${NC}"
        fi
        ;;
    7)
        echo -e "${GREEN}Opening PDF report...${NC}"
        if [ -f "Panorama_Stitching_Report.pdf" ]; then
            if command -v xdg-open > /dev/null; then
                xdg-open Panorama_Stitching_Report.pdf
            elif command -v open > /dev/null; then
                open Panorama_Stitching_Report.pdf
            else
                echo "PDF exists at: Panorama_Stitching_Report.pdf"
            fi
        else
            echo -e "${YELLOW}PDF not generated yet. Run:${NC}"
            echo "  python3 scripts/generate_pdf_report.py"
        fi
        ;;
    8)
        echo -e "${GREEN}Re-organizing results...${NC}"
        python3 scripts/analysis_pipeline.py --enhance
        echo -e "${GREEN}✓ Results re-organized${NC}"
        ;;
    9)
        echo -e "${BLUE}Goodbye!${NC}"
        exit 0
        ;;
    *)
        echo -e "${RED}Invalid option${NC}"
        ;;
esac

echo ""
echo -e "${GREEN}✓ Done!${NC}"