#ifndef CONFIG_H
#define CONFIG_H

/******************************************************************************
 * CONFIGURATION FILE - Adjust these values to tune the panorama stitcher
 *
 * This file contains all the key parameters that control:
 * - How many features to detect
 * - How strict the matching should be
 * - Memory limits and safety checks
 *****************************************************************************/

namespace PanoramaConfig {
    // FEATURE DETECTION - Controls how many keypoints we find
    constexpr int DEFAULT_MAX_FEATURES = 50000;  // Max keypoints per image
    constexpr int MIN_FEATURES = 10;             // Minimum needed to work
    constexpr int MAX_FEATURES = 50000;          // Hard limit for memory

    // RANSAC - Controls how we filter out bad matches
    constexpr double DEFAULT_RANSAC_THRESHOLD = 3.0;   // Pixels - how far off a match can be
                                                        // Lower = stricter (fewer but better matches)
                                                        // Higher = more permissive (more matches, possibly wrong)
    constexpr double MIN_RANSAC_THRESHOLD = 0.1;       // Very strict
    constexpr double MAX_RANSAC_THRESHOLD = 50.0;      // Very permissive
    constexpr double DEFAULT_RANSAC_CONFIDENCE = 0.995; // 99.5% confidence we found the right transformation
    constexpr int DEFAULT_RANSAC_MAX_ITERATIONS = 2000; // Max attempts to find good model
    constexpr int MIN_INLIERS_REQUIRED = 20;           // Need at least this many good matches
    
    // Panorama Size Limits
    constexpr int MAX_PANORAMA_DIMENSION = 15000;  // Increased for legitimate large panoramas
    constexpr int MIN_IMAGE_DIMENSION = 50;
    constexpr int PANORAMA_PADDING = 10;
    constexpr size_t MAX_PANORAMA_MEMORY = 2147483648;  // 2GB max memory for panorama
    
    // Homography Validation
    constexpr double MIN_HOMOGRAPHY_DETERMINANT = 0.001;  // Prevents degenerate transformations
    constexpr double MAX_HOMOGRAPHY_DETERMINANT = 1000.0;
    constexpr double MIN_HOMOGRAPHY_SCALE = 0.1;  // Prevents excessive shrinking
    constexpr double MAX_HOMOGRAPHY_SCALE = 10.0;  // Prevents excessive magnification
    constexpr double HOMOGRAPHY_EPSILON = 1e-10;
    
    // Image Size Reference (for adaptive feature scaling)
    constexpr int REFERENCE_IMAGE_WIDTH = 2048;
    constexpr int REFERENCE_IMAGE_HEIGHT = 1536;
    constexpr double PANORAMA_SCALE_THRESHOLD = 1.5; // When to scale features
    
    // Memory Limits
    constexpr size_t MAX_IMAGE_PIXELS = 100000000; // 100 megapixels
    constexpr size_t WARNING_IMAGE_PIXELS = 50000000; // 50 megapixels
}

#endif // CONFIG_H