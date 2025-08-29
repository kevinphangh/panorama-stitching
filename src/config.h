#ifndef CONFIG_H
#define CONFIG_H

namespace PanoramaConfig {
    // Feature Detection Configuration
    constexpr int DEFAULT_MAX_FEATURES = 2000;
    constexpr int MIN_FEATURES = 10;
    constexpr int MAX_FEATURES = 50000;
    
    // RANSAC Configuration
    constexpr double DEFAULT_RANSAC_THRESHOLD = 3.0;
    constexpr double MIN_RANSAC_THRESHOLD = 0.1;
    constexpr double MAX_RANSAC_THRESHOLD = 50.0;
    constexpr double DEFAULT_RANSAC_CONFIDENCE = 0.995;
    constexpr int DEFAULT_RANSAC_MAX_ITERATIONS = 2000;
    constexpr int MIN_INLIERS_REQUIRED = 20;
    
    // Panorama Size Limits
    constexpr int MAX_PANORAMA_DIMENSION = 15000;  // Increased for legitimate large panoramas
    constexpr int MIN_IMAGE_DIMENSION = 50;
    constexpr int PANORAMA_PADDING = 10;
    constexpr size_t MAX_PANORAMA_MEMORY = 2147483648;  // 2GB max memory for panorama
    
    // Homography Validation
    constexpr double MIN_HOMOGRAPHY_DETERMINANT = 0.001;
    constexpr double MAX_HOMOGRAPHY_DETERMINANT = 1000.0;
    constexpr double MIN_HOMOGRAPHY_SCALE = 0.1;
    constexpr double MAX_HOMOGRAPHY_SCALE = 10.0;
    constexpr double HOMOGRAPHY_EPSILON = 1e-10;
    
    // Blending Configuration
    constexpr int DEFAULT_FEATHER_RADIUS = 30;
    constexpr int DEFAULT_PYRAMID_LEVELS = 5;
    
    // Matching Configuration
    constexpr double DEFAULT_RATIO_TEST_THRESHOLD = 0.7;
    
    // Image Size Reference (for adaptive feature scaling)
    constexpr int REFERENCE_IMAGE_WIDTH = 2048;
    constexpr int REFERENCE_IMAGE_HEIGHT = 1536;
    constexpr double PANORAMA_SCALE_THRESHOLD = 1.5; // When to scale features
    
    // Memory Limits
    constexpr size_t MAX_IMAGE_PIXELS = 100000000; // 100 megapixels
    constexpr size_t WARNING_IMAGE_PIXELS = 50000000; // 50 megapixels
}

#endif // CONFIG_H