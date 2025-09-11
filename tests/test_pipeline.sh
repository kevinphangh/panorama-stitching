#!/bin/bash

echo "Testing improved panorama stitching pipeline..."
echo "=============================================="

# Create results directory
mkdir -p results/visualizations

# Test single stitching with timing output
echo -e "\n1. Testing basic stitching with timing..."
./scripts/run_panorama.sh --stitch \
    datasets/indoor_scene/img1.jpg \
    datasets/indoor_scene/img2.jpg \
    --output results/test_panorama.jpg \
    --detector orb \
    --blend-mode feather

if [ -f "results/test_panorama.jpg" ]; then
    echo "✓ Basic stitching successful"
else
    echo "✗ Basic stitching failed"
fi

# Create minimal test metrics CSV for analysis
echo -e "\n2. Creating test metrics file..."
cat > results/metrics.csv << EOF
experiment,detector,ransac_threshold,blend_mode,num_keypoints_1,num_keypoints_2,num_matches,num_inliers,inlier_ratio,reprojection_error,detection_time,matching_time,homography_time,warping_time,blending_time,total_time
test_indoor_orb,orb,3.0,feather,22488,31975,822,458,0.557,1.2,45.3,12.1,8.5,25.6,18.3,109.8
test_indoor_akaze,akaze,3.0,feather,8456,9234,412,245,0.595,1.1,62.4,10.3,7.2,24.1,17.9,121.9
test_indoor_orb_simple,orb,3.0,simple,22488,31975,822,458,0.557,1.2,44.8,11.9,8.3,25.2,5.1,95.3
test_indoor_orb_multiband,orb,3.0,multiband,22488,31975,822,458,0.557,1.2,45.1,12.0,8.4,25.4,42.7,133.6
EOF

echo "✓ Test metrics created"

# Create some dummy visualization files for testing
echo -e "\n3. Creating test visualizations..."
if [ -f "results/test_panorama.jpg" ]; then
    cp results/test_panorama.jpg results/visualizations/test_indoor_orb_img1.jpg
    cp results/test_panorama.jpg results/visualizations/test_indoor_orb_img2.jpg
    cp results/test_panorama.jpg results/visualizations/test_indoor_orb_keypoints1.jpg
    cp results/test_panorama.jpg results/visualizations/test_indoor_orb_keypoints2.jpg
    cp results/test_panorama.jpg results/visualizations/test_indoor_orb_matches_before.jpg
    cp results/test_panorama.jpg results/visualizations/test_indoor_orb_matches_after.jpg
    echo "✓ Test visualizations created"
fi

# Run analysis
echo -e "\n4. Running analysis script..."
python3 analyze_results.py

echo -e "\n=============================================="
echo "Testing complete!"
echo ""
echo "Results:"
ls -la results_analysis/ 2>/dev/null || echo "No analysis output found"
echo ""
echo "To view results, open: results_analysis/analysis_report.html"