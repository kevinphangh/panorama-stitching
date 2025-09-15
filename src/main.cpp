#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "config.h"
#include "feature_detection/feature_detector.h"
#include "feature_detection/orb_detector.h"
#include "feature_detection/akaze_detector.h"
#include "feature_matching/matcher.h"
#include "feature_matching/ransac.h"
#include "homography/homography_estimator.h"
#include "stitching/image_warper.h"
#include "stitching/blender.h"
#include "experiments/experiment_runner.h"
#include "experiments/visualization.h"

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --stitch <img1> <img2>       : Stitch two images\n"
              << "  --stitch-multiple <img1> ...  : Stitch multiple images\n"
              << "  --experiment-mode            : Run all experiments\n"
              << "  --detector <orb|akaze>       : Choose feature detector (default: orb)\n"
              << "  --blend-mode <mode>          : Choose blend mode (simple|feather|multiband)\n"
              << "  --ransac-threshold <value>   : Set RANSAC threshold (default: 3.0)\n"
              << "  --max-features <num>         : Set max features (default: 2000)\n"
              << "  --output <path>              : Output path for panorama\n"
              << "  --visualize                  : Show intermediate results\n"
              << "  --help                       : Show this message\n";
}

bool isValidOutputPath(const std::string& path) {
    // Security: prevent path traversal and writing to system directories
    if (path.find("..") != std::string::npos) {
        std::cerr << "Error: Path traversal detected in output path\n";
        return false;
    }
    
    if (path[0] == '/' && 
        (path.find("/etc") == 0 || 
         path.find("/usr") == 0 || 
         path.find("/bin") == 0 ||
         path.find("/sbin") == 0 ||
         path.find("/boot") == 0 ||
         path.find("/sys") == 0 ||
         path.find("/proc") == 0)) {
        std::cerr << "Error: Cannot write to system directories\n";
        return false;
    }
    
    return true;
}

template<typename T>
bool parseArgument(const std::string& arg, T& value, const std::string& param_name) {
    std::istringstream iss(arg);
    T temp;
    
    if (!(iss >> temp)) {
        std::cerr << "Error: Invalid " << param_name << " value: '" << arg << "'\n";
        return false;
    }
    
    std::string remainder;
    if (iss >> remainder) {
        std::cerr << "Error: Invalid " << param_name << " value: '" << arg << "' (extra characters)\n";
        return false;
    }
    
    value = temp;
    return true;
}

bool parseDouble(const std::string& arg, double& value, const std::string& param_name,
                double min_val = std::numeric_limits<double>::lowest(),
                double max_val = std::numeric_limits<double>::max()) {
    if (!parseArgument(arg, value, param_name)) {
        return false;
    }
    
    if (value < min_val || value > max_val) {
        std::cerr << "Error: " << param_name << " value " << value 
                  << " out of range [" << min_val << ", " << max_val << "]\n";
        return false;
    }
    
    return true;
}

bool parseInt(const std::string& arg, int& value, const std::string& param_name,
             int min_val = std::numeric_limits<int>::min(),
             int max_val = std::numeric_limits<int>::max()) {
    if (!parseArgument(arg, value, param_name)) {
        return false;
    }
    
    if (value < min_val || value > max_val) {
        std::cerr << "Error: " << param_name << " value " << value 
                  << " out of range [" << min_val << ", " << max_val << "]\n";
        return false;
    }
    
    return true;
}

int calculateAdaptiveFeatures(int image_pixels, int max_features) {
    int base_pixels = PanoramaConfig::REFERENCE_IMAGE_HEIGHT * PanoramaConfig::REFERENCE_IMAGE_WIDTH;
    
    if (image_pixels > base_pixels * PanoramaConfig::PANORAMA_SCALE_THRESHOLD) {
        int adaptive_features = static_cast<int>(max_features * std::sqrt(static_cast<double>(image_pixels) / base_pixels));
        std::cout << "Scaling features for large image: " << adaptive_features << " (from " << max_features << ")\n";
        return adaptive_features;
    }
    
    return max_features;
}

cv::Mat performStitchingDirect(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const std::string& detector_type,
    const std::string& blend_mode,
    double ransac_threshold,
    int max_features,
    bool visualize,
    int max_panorama_dimension = PanoramaConfig::MAX_PANORAMA_DIMENSION
);

cv::Mat performSequentialStitching(
    const std::vector<cv::Mat>& images,
    const std::string& detector_type,
    const std::string& blend_mode,
    double ransac_threshold,
    int max_features,
    bool visualize
);

cv::Mat performStitching(
    const std::string& img1_path,
    const std::string& img2_path,
    const std::string& detector_type = "orb",
    const std::string& blend_mode = "feather",
    double ransac_threshold = 3.0,
    int max_features = 2000,
    bool visualize = false
) {
    cv::Mat img1 = cv::imread(img1_path);
    cv::Mat img2 = cv::imread(img2_path);
    
    if (img1.empty() || img2.empty()) {
        std::cerr << "Error: Could not load images: " << img1_path << " or " << img2_path << "\n";
        return cv::Mat();
    }
    
    return performStitchingDirect(img1, img2, detector_type, blend_mode, 
                                  ransac_threshold, max_features, visualize, PanoramaConfig::MAX_PANORAMA_DIMENSION);
}

cv::Mat performStitchingDirect(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const std::string& detector_type,
    const std::string& blend_mode,
    double ransac_threshold,
    int max_features,
    bool visualize,
    int max_panorama_dimension
) {
    
    if (img1.empty() || img2.empty()) {
        std::cerr << "Error: One or both input images are empty\n";
        return cv::Mat();
    }
    
    if (img1.type() != CV_8UC3 || img2.type() != CV_8UC3) {
        std::cerr << "Error: Input images must be 8-bit 3-channel (BGR)\n";
        return cv::Mat();
    }
    
    if (img1.cols < PanoramaConfig::MIN_IMAGE_DIMENSION || img1.rows < PanoramaConfig::MIN_IMAGE_DIMENSION || 
        img2.cols < PanoramaConfig::MIN_IMAGE_DIMENSION || img2.rows < PanoramaConfig::MIN_IMAGE_DIMENSION) {
        std::cerr << "Error: Images too small (minimum " << PanoramaConfig::MIN_IMAGE_DIMENSION << "x" << PanoramaConfig::MIN_IMAGE_DIMENSION << " pixels)\n";
        return cv::Mat();
    }
    
    if (ransac_threshold <= 0 || ransac_threshold > PanoramaConfig::MAX_RANSAC_THRESHOLD) {
        std::cerr << "Warning: Invalid RANSAC threshold, using default " << PanoramaConfig::DEFAULT_RANSAC_THRESHOLD << "\n";
        ransac_threshold = PanoramaConfig::DEFAULT_RANSAC_THRESHOLD;
    }
    
    if (max_features < PanoramaConfig::MIN_FEATURES || max_features > PanoramaConfig::MAX_FEATURES) {
        std::cerr << "Warning: Invalid max_features, using default " << PanoramaConfig::DEFAULT_MAX_FEATURES << "\n";
        max_features = PanoramaConfig::DEFAULT_MAX_FEATURES;
    }
    
    std::cout << "Loaded images: " << img1.size() << " and " << img2.size() << "\n";
    
    size_t total_pixels = static_cast<size_t>(img1.rows) * img1.cols + 
                         static_cast<size_t>(img2.rows) * img2.cols;
    if (total_pixels > PanoramaConfig::MAX_IMAGE_PIXELS) {
        std::cerr << "Error: Combined image size exceeds maximum allowed (" 
                  << PanoramaConfig::MAX_IMAGE_PIXELS / 1000000 << " megapixels)\n";
        return cv::Mat();
    }
    if (total_pixels > PanoramaConfig::WARNING_IMAGE_PIXELS) {
        std::cerr << "Warning: Large image size detected. Processing may be slow.\n";
    }
    
    int img1_pixels = img1.rows * img1.cols;
    int img2_pixels = img2.rows * img2.cols;
    
    // Larger panoramas need proportionally more features for robust matching
    int adaptive_features1 = calculateAdaptiveFeatures(img1_pixels, max_features);
    int adaptive_features2 = calculateAdaptiveFeatures(img2_pixels, max_features);
    
    std::unique_ptr<FeatureDetector> detector1;
    std::unique_ptr<FeatureDetector> detector2;
    
    if (detector_type == "orb") {
        detector1 = std::make_unique<ORBDetector>();
        detector2 = std::make_unique<ORBDetector>();
    } else if (detector_type == "akaze") {
        detector1 = std::make_unique<AKAZEDetector>();
        detector2 = std::make_unique<AKAZEDetector>();
    } else {
        std::cerr << "Unknown detector type: " << detector_type << "\n";
        return cv::Mat();
    }
    
    detector1->setMaxFeatures(adaptive_features1);
    detector2->setMaxFeatures(adaptive_features2);
    
    std::cout << "Detecting features...\n";
    auto result1 = detector1->detect(img1);
    auto result2 = detector2->detect(img2);
    
    std::cout << "Detected " << result1.getKeypointCount() << " and " 
              << result2.getKeypointCount() << " keypoints\n";
    
    std::cout << "Matching features...\n";
    FeatureMatcher matcher;
    auto match_result = matcher.matchFeatures(
        result1.descriptors, result2.descriptors,
        result1.keypoints, result2.keypoints
    );
    
    std::cout << "Found " << match_result.num_good_matches << " good matches\n";
    
    std::cout << "Estimating homography...\n";
    HomographyEstimator h_estimator;
    h_estimator.setRANSACThreshold(ransac_threshold);
    
    std::vector<cv::DMatch> inlier_matches;
    cv::Mat homography = h_estimator.estimateHomography(
        result1.keypoints, result2.keypoints,
        match_result.good_matches, inlier_matches
    );
    
    auto ransac_result = h_estimator.getLastResult();
    std::cout << "RANSAC found " << ransac_result.num_inliers 
              << " inliers (" << ransac_result.inlier_ratio * 100 << "%)\n";
    
    if (homography.empty()) {
        std::cerr << "Failed to compute homography\n";
        return cv::Mat();
    }
    
    for (int i = 0; i < homography.rows; i++) {
        for (int j = 0; j < homography.cols; j++) {
            double val = homography.at<double>(i, j);
            if (std::isnan(val) || std::isinf(val)) {
                std::cerr << "Error: Invalid homography matrix (contains NaN or Inf)\n";
                return cv::Mat();
            }
        }
    }
    
    double det = cv::determinant(homography);
    if (std::abs(det) < PanoramaConfig::MIN_HOMOGRAPHY_DETERMINANT || std::abs(det) > PanoramaConfig::MAX_HOMOGRAPHY_DETERMINANT) {
        std::cerr << "Error: Homography determinant out of reasonable range: " << det << std::endl;
        std::cerr << "This indicates poor feature matches or incompatible images\n";
        std::cerr << "Try: 1) Using ORB detector instead of AKAZE\n";
        std::cerr << "     2) Ensuring images have sufficient overlap (30-40%)\n";
        std::cerr << "     3) Increasing max_features for better matching\n";
        return cv::Mat();
    }
    
    cv::Mat H_normalized = homography.clone();
    double h22 = H_normalized.at<double>(2, 2);
    if (std::abs(h22) < 1e-10) {
        std::cerr << "Error: Homography matrix is singular (H[2,2] = " << h22 << ")\n";
        return cv::Mat();
    }
    H_normalized /= h22;

    double scale_x = std::sqrt(H_normalized.at<double>(0,0) * H_normalized.at<double>(0,0) +
                               H_normalized.at<double>(1,0) * H_normalized.at<double>(1,0));
    double scale_y = std::sqrt(H_normalized.at<double>(0,1) * H_normalized.at<double>(0,1) + 
                               H_normalized.at<double>(1,1) * H_normalized.at<double>(1,1));
    
    if (scale_x < PanoramaConfig::MIN_HOMOGRAPHY_SCALE || scale_x > PanoramaConfig::MAX_HOMOGRAPHY_SCALE || 
        scale_y < PanoramaConfig::MIN_HOMOGRAPHY_SCALE || scale_y > PanoramaConfig::MAX_HOMOGRAPHY_SCALE) {
        std::cerr << "Error: Homography implies extreme scaling (x=" << scale_x << ", y=" << scale_y << ")\n";
        std::cerr << "Images may not be from the same scene or have insufficient overlap\n";
        return cv::Mat();
    }
    
    if (ransac_result.num_inliers < PanoramaConfig::MIN_INLIERS_REQUIRED) {
        std::cerr << "Error: Too few inliers (" << ransac_result.num_inliers << ") for reliable stitching\n";
        std::cerr << "Minimum " << PanoramaConfig::MIN_INLIERS_REQUIRED << " inliers required for stable homography\n";
        return cv::Mat();
    }
    
    if (visualize) {
        cv::Mat match_img = matcher.visualizeMatches(
            img1, img2, result1.keypoints, result2.keypoints, inlier_matches
        );
        cv::imshow("Inlier Matches", match_img);
        cv::waitKey(0);
        
        if (!match_result.match_distances.empty()) {
            cv::Mat histogram = Visualization::generateMatchDistanceHistogram(
                match_result.match_distances,
                "Match Distance Distribution"
            );
            if (!histogram.empty()) {
                std::string hist_filename = "match_distances_histogram.jpg";
                if (Visualization::saveVisualization(histogram, hist_filename)) {
                    std::cout << "Match distance histogram saved to: " << hist_filename << std::endl;
                }
                cv::imshow("Match Distance Histogram", histogram);
                cv::waitKey(0);
            }
        }
    }
    
    std::cout << "Warping images...\n";
    ImageWarper warper;
    
    // CRITICAL: homography maps img1 -> img2, but we need img2 -> img1 for warping
    cv::Mat H_inv;
    try {
        H_inv = homography.inv();
    } catch (const cv::Exception& e) {
        std::cerr << "Failed to invert homography: " << e.what() << std::endl;
        return cv::Mat();
    }
    
    std::vector<cv::Point2f> corners2(4);
    corners2[0] = cv::Point2f(0, 0);
    corners2[1] = cv::Point2f(static_cast<float>(img2.cols), 0);
    corners2[2] = cv::Point2f(static_cast<float>(img2.cols), static_cast<float>(img2.rows));
    corners2[3] = cv::Point2f(0, static_cast<float>(img2.rows));
    
    std::vector<cv::Point2f> corners2_transformed;
    cv::perspectiveTransform(corners2, corners2_transformed, H_inv);
    
    // Find min/max coordinates
    float min_x = 0, max_x = static_cast<float>(img1.cols);
    float min_y = 0, max_y = static_cast<float>(img1.rows);
    
    for (const auto& pt : corners2_transformed) {
        min_x = std::min(min_x, pt.x);
        max_x = std::max(max_x, pt.x);
        min_y = std::min(min_y, pt.y);
        max_y = std::max(max_y, pt.y);
    }
    
    cv::Mat translation = (cv::Mat_<double>(3, 3) << 
        1, 0, -min_x,
        0, 1, -min_y,
        0, 0, 1);
    
    // Calculate output size with padding
    cv::Size panorama_size(
        static_cast<int>(max_x - min_x) + PanoramaConfig::PANORAMA_PADDING * 2,
        static_cast<int>(max_y - min_y) + PanoramaConfig::PANORAMA_PADDING * 2
    );
    
    if (panorama_size.width <= 0 || panorama_size.height <= 0) {
        std::cerr << "Invalid panorama size (negative)" << std::endl;
        return cv::Mat();
    }
    
    if (panorama_size.width > max_panorama_dimension || panorama_size.height > max_panorama_dimension) {
        std::cerr << "Error: Panorama size would be " << panorama_size.width 
                  << "x" << panorama_size.height << " pixels (max: " << max_panorama_dimension << ")\n";
        std::cerr << "This usually indicates:\n";
        std::cerr << "  1) Poor feature matches between images\n";
        std::cerr << "  2) Images from different scenes\n";
        std::cerr << "  3) Insufficient overlap between images\n";
        std::cerr << "Recommendations:\n";
        std::cerr << "  - Use ORB detector (more robust for multi-image stitching)\n";
        std::cerr << "  - Ensure 30-40% overlap between consecutive images\n";
        std::cerr << "  - Verify images are from the same scene\n";
        return cv::Mat();
    }
    
    size_t estimated_bytes = static_cast<size_t>(panorama_size.width) * panorama_size.height * 3 * 2;  // x2 for processing overhead
    if (estimated_bytes > PanoramaConfig::MAX_PANORAMA_MEMORY) {
        std::cerr << "Error: Panorama would require approximately " 
                  << (estimated_bytes / 1048576) << " MB of memory (max: " 
                  << (PanoramaConfig::MAX_PANORAMA_MEMORY / 1048576) << " MB)\n";
        std::cerr << "Consider using simpler blend mode (--blend-mode simple) or processing in smaller segments\n";
        return cv::Mat();
    }
    
    cv::Mat panorama = cv::Mat::zeros(panorama_size, img1.type());
    cv::Mat mask1 = cv::Mat::zeros(panorama_size, CV_8UC1);
    cv::Mat mask2 = cv::Mat::zeros(panorama_size, CV_8UC1);
    
    cv::Mat warped1;
    cv::warpPerspective(img1, warped1, translation, panorama_size);
    cv::warpPerspective(cv::Mat::ones(img1.size(), CV_8UC1) * 255, 
                       mask1, translation, panorama_size);
    
    // Warp second image with inverse homography (img2 -> img1 coordinate system)
    cv::Mat warped2;
    cv::Mat warped_mask2;
    cv::warpPerspective(img2, warped2, translation * H_inv, panorama_size);
    cv::warpPerspective(cv::Mat::ones(img2.size(), CV_8UC1) * 255, 
                       warped_mask2, translation * H_inv, panorama_size);
    
    std::cout << "Blending images...\n";
    Blender blender;
    
    if (blend_mode == "simple") {
        blender.setBlendMode(BlendMode::SIMPLE_OVERLAY);
    } else if (blend_mode == "feather") {
        blender.setBlendMode(BlendMode::FEATHERING);
    } else if (blend_mode == "multiband") {
        blender.setBlendMode(BlendMode::MULTIBAND);
    }
    
    panorama = blender.blend(warped1, warped2, mask1, warped_mask2);
    
    std::cout << "Panorama created successfully!\n";
    
    return panorama;
}

cv::Mat performSequentialStitching(
    const std::vector<cv::Mat>& images,
    const std::string& detector_type,
    const std::string& blend_mode,
    double ransac_threshold,
    int max_features,
    bool visualize
) {
    std::cout << "\n=== Using sequential stitching ===\n";
    
    if (images.empty()) {
        std::cerr << "Error: No images provided for stitching\n";
        return cv::Mat();
    }
    
    cv::Mat panorama = images[0].clone();
    
    for (size_t i = 1; i < images.size(); i++) {
        std::cout << "\n=== Stitching image " << (i + 1) << " of " << images.size() << " ===\n";
        
        cv::Mat result = performStitchingDirect(
            panorama, images[i],
            detector_type, blend_mode,
            ransac_threshold, max_features,
            visualize, PanoramaConfig::MAX_PANORAMA_DIMENSION
        );
        
        if (result.empty()) {
            std::cerr << "Failed to stitch image " << (i + 1) << "\n";
            return cv::Mat();
        }
        
        panorama = result;
    }
    
    return panorama;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "--help") {
        printUsage(argv[0]);
        return 0;
    }
    
    if (mode == "--experiment-mode") {
        std::cout << "Running experiments...\n";
        ExperimentRunner runner;
        runner.runAllExperiments();
        runner.saveResults("results/");
        runner.generateReport("results/experiment_report.pdf");
        return 0;
    }
    
    if (mode == "--stitch" && argc >= 4) {
        std::string img1_path = argv[2];
        std::string img2_path = argv[3];
        
        std::string detector_type = "orb";
        std::string blend_mode = "feather";
        double ransac_threshold = PanoramaConfig::DEFAULT_RANSAC_THRESHOLD;
        int max_features = PanoramaConfig::DEFAULT_MAX_FEATURES;
        std::string output_path = "panorama.jpg";
        bool visualize = false;
        
        for (int i = 4; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--detector" && i + 1 < argc) {
                detector_type = argv[++i];
            } else if (arg == "--blend-mode" && i + 1 < argc) {
                blend_mode = argv[++i];
            } else if (arg == "--ransac-threshold" && i + 1 < argc) {
                if (!parseDouble(argv[++i], ransac_threshold, "RANSAC threshold", PanoramaConfig::MIN_RANSAC_THRESHOLD, PanoramaConfig::MAX_RANSAC_THRESHOLD)) {
                    std::cerr << "Using default RANSAC threshold: " << PanoramaConfig::DEFAULT_RANSAC_THRESHOLD << "\n";
                    ransac_threshold = PanoramaConfig::DEFAULT_RANSAC_THRESHOLD;
                }
            } else if (arg == "--max-features" && i + 1 < argc) {
                if (!parseInt(argv[++i], max_features, "max features", PanoramaConfig::MIN_FEATURES, PanoramaConfig::MAX_FEATURES)) {
                    std::cerr << "Using default max features: " << PanoramaConfig::DEFAULT_MAX_FEATURES << "\n";
                    max_features = PanoramaConfig::DEFAULT_MAX_FEATURES;
                }
            } else if (arg == "--output" && i + 1 < argc) {
                output_path = argv[++i];
            } else if (arg == "--visualize") {
                visualize = true;
            }
        }
        
        cv::Mat panorama = performStitching(
            img1_path, img2_path,
            detector_type, blend_mode,
            ransac_threshold, max_features,
            visualize
        );
        
        if (!panorama.empty()) {
            if (!isValidOutputPath(output_path)) {
                std::cerr << "Error: Invalid output path specified\n";
                return 1;
            }
            cv::imwrite(output_path, panorama);
            std::cout << "Panorama saved to: " << output_path << "\n";
            
            if (visualize) {
                cv::imshow("Panorama", panorama);
                cv::waitKey(0);
            }
        }
        
        return 0;
    }
    
    if (mode == "--stitch-multiple" && argc >= 4) {
        std::vector<std::string> image_paths;
        for (int i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                break;
            }
            image_paths.push_back(argv[i]);
        }
        
        if (image_paths.size() < 3) {
            std::cerr << "Need at least 3 images for multi-image stitching\n";
            return 1;
        }
        
        std::string detector_type = "orb";
        std::string blend_mode = "feather";
        double ransac_threshold = PanoramaConfig::DEFAULT_RANSAC_THRESHOLD;
        int max_features = PanoramaConfig::DEFAULT_MAX_FEATURES;
        std::string output_path = "multi_panorama.jpg";
        bool visualize = false;
        
        int option_start = 2 + image_paths.size();
        for (int i = option_start; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--detector" && i + 1 < argc) {
                detector_type = argv[++i];
            } else if (arg == "--blend-mode" && i + 1 < argc) {
                blend_mode = argv[++i];
            } else if (arg == "--ransac-threshold" && i + 1 < argc) {
                if (!parseDouble(argv[++i], ransac_threshold, "RANSAC threshold", PanoramaConfig::MIN_RANSAC_THRESHOLD, PanoramaConfig::MAX_RANSAC_THRESHOLD)) {
                    std::cerr << "Using default RANSAC threshold: " << PanoramaConfig::DEFAULT_RANSAC_THRESHOLD << "\n";
                    ransac_threshold = PanoramaConfig::DEFAULT_RANSAC_THRESHOLD;
                }
            } else if (arg == "--max-features" && i + 1 < argc) {
                if (!parseInt(argv[++i], max_features, "max features", PanoramaConfig::MIN_FEATURES, PanoramaConfig::MAX_FEATURES)) {
                    std::cerr << "Using default max features: " << PanoramaConfig::DEFAULT_MAX_FEATURES << "\n";
                    max_features = PanoramaConfig::DEFAULT_MAX_FEATURES;
                }
            } else if (arg == "--output" && i + 1 < argc) {
                output_path = argv[++i];
            } else if (arg == "--visualize") {
                visualize = true;
            }
        }
        
        std::cout << "Multi-image stitching with " << image_paths.size() << " images\n";
        
        std::vector<cv::Mat> images;
        for (const auto& path : image_paths) {
            cv::Mat img = cv::imread(path);
            if (img.empty()) {
                std::cerr << "Error: Could not load image " << path << "\n";
                return 1;
            }
            images.push_back(img);
            std::cout << "Loaded: " << path << " [" << img.size() << "]\n";
        }
        
        cv::Mat panorama;
        bool use_sequential = false;
        
        if (images.size() == 3) {
            std::cout << "\n=== Attempting reference-based stitching (img2 as center) ===\n";
            
            std::cout << "\n--- Stitching img1 to img2 (left side) ---\n";
            // Use center image as reference, stitch left image to it
            cv::Mat left_stitch = performStitchingDirect(
                images[1], images[0],  // img2, img1 - reversed for left-to-center
                detector_type, blend_mode,
                ransac_threshold, max_features,
                visualize, PanoramaConfig::MAX_PANORAMA_DIMENSION
            );
            
            if (!left_stitch.empty()) {
                std::cout << "\n--- Stitching img3 to panorama (right side) ---\n";
                // Add right image to the accumulated panorama
                panorama = performStitchingDirect(
                    left_stitch, images[2],  // left_stitch, img3
                    detector_type, blend_mode,
                    ransac_threshold, max_features,
                    visualize, PanoramaConfig::MAX_PANORAMA_DIMENSION
                );
                
                if (panorama.empty()) {
                    std::cout << "Failed to stitch img3, falling back to sequential approach...\n";
                    use_sequential = true;
                }
            } else {
                std::cout << "Failed to stitch img1 to img2, falling back to sequential approach...\n";
                use_sequential = true;
            }
        } else {
            use_sequential = true;
        }
        
        if (use_sequential) {
            panorama = performSequentialStitching(
                images, detector_type, blend_mode,
                ransac_threshold, max_features, visualize
            );
            
            if (panorama.empty()) {
                std::cerr << "Sequential stitching failed\n";
                return 1;
            }
        }
        
        if (!output_path.empty()) {
            if (!isValidOutputPath(output_path)) {
                std::cerr << "Error: Invalid output path specified\n";
                return 1;
            }
            cv::imwrite(output_path, panorama);
            std::cout << "\nMulti-image panorama saved to: " << output_path << "\n";
        } else {
            cv::imwrite("multi_panorama.jpg", panorama);
            std::cout << "\nMulti-image panorama saved to: multi_panorama.jpg\n";
        }
        
        return 0;
    }
    
    printUsage(argv[0]);
    return 1;
}