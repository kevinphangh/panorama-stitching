#include "stitching_pipeline.h"
#include "../config.h"
#include "../feature_detection/feature_detector.h"
#include "../feature_detection/detector_factory.h"
#include "../feature_matching/matcher.h"
#include "../homography/homography_estimator.h"
#include "../stitching/image_warper.h"
#include "../stitching/blender.h"
#include "../stitching/blender_factory.h"
#include "../experiments/visualization.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <filesystem>

int StitchingPipeline::calculateAdaptiveFeatures(int image_pixels, int max_features) {
    int base_pixels = PanoramaConfig::REFERENCE_IMAGE_HEIGHT * PanoramaConfig::REFERENCE_IMAGE_WIDTH;

    if (image_pixels > base_pixels * PanoramaConfig::PANORAMA_SCALE_THRESHOLD) {
        double scale_factor = std::sqrt(static_cast<double>(image_pixels) / base_pixels);
        scale_factor = std::min(scale_factor, 3.0);
        int adaptive_features = static_cast<int>(max_features * scale_factor);
        std::cout << "Scaling features for large image: " << adaptive_features << " (from " << max_features << ")\n";
        return adaptive_features;
    }

    return max_features;
}

cv::Mat StitchingPipeline::performStitching(
    const std::string& img1_path,
    const std::string& img2_path,
    const std::string& detector_type,
    const std::string& blend_mode,
    double ransac_threshold,
    int max_features,
    bool visualize
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

cv::Mat StitchingPipeline::performStitchingDirect(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const std::string& detector_type,
    const std::string& blend_mode,
    double ransac_threshold,
    int max_features,
    bool visualize,
    int max_panorama_dimension
) {
    auto start_time = std::chrono::high_resolution_clock::now();

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

    int adaptive_features1 = calculateAdaptiveFeatures(img1_pixels, max_features);
    int adaptive_features2 = calculateAdaptiveFeatures(img2_pixels, max_features);

    std::unique_ptr<FeatureDetector> detector1;
    std::unique_ptr<FeatureDetector> detector2;

    try {
        detector1 = DetectorFactory::createDetector(detector_type);
        detector2 = DetectorFactory::createDetector(detector_type);
    } catch (const std::exception& e) {
        std::cerr << "Error creating detector: " << e.what() << "\n";
        return cv::Mat();
    }

    detector1->setMaxFeatures(adaptive_features1);
    detector2->setMaxFeatures(adaptive_features2);

    std::cout << "Detecting features...\n";
    auto result1 = detector1->detect(img1);
    auto result2 = detector2->detect(img2);

    std::cout << "Detected " << result1.getKeypointCount() << " keypoints (img1) and "
              << result2.getKeypointCount() << " keypoints (img2)\n";

    {
        namespace fs = std::filesystem;
        std::string viz_dir = "results/visualizations";
        if (!fs::exists(viz_dir)) {
            fs::create_directories(viz_dir);
        }

        static int viz_counter = 0;
        viz_counter++;
        std::string base_name = "stitch_" + std::to_string(viz_counter) + "_" + detector_type;

        cv::Mat kp_vis1, kp_vis2;
        cv::drawKeypoints(img1, result1.keypoints, kp_vis1, cv::Scalar(0, 255, 0),
                         cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::drawKeypoints(img2, result2.keypoints, kp_vis2, cv::Scalar(0, 255, 0),
                         cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        std::string kp1_path = viz_dir + "/" + base_name + "_keypoints1.jpg";
        std::string kp2_path = viz_dir + "/" + base_name + "_keypoints2.jpg";

        cv::imwrite(kp1_path, kp_vis1);
        cv::imwrite(kp2_path, kp_vis2);
        std::cout << "Saved keypoint visualizations to " << viz_dir << "/\n";
    }

    std::cout << "Matching features...\n";
    FeatureMatcher matcher;
    if (detector_type == "sift") {
        matcher.setMatcherType("BruteForce-L2");
    } else {
        matcher.setMatcherType("BruteForce-Hamming");
    }
    auto match_result = matcher.matchFeatures(
        result1.descriptors, result2.descriptors,
        result1.keypoints, result2.keypoints,
        0.75
    );

    std::cout << "Found " << match_result.num_good_matches << " good matches\n";

    {
        namespace fs = std::filesystem;
        std::string viz_dir = "results/visualizations";
        if (!fs::exists(viz_dir)) {
            fs::create_directories(viz_dir);
        }

            static int match_counter = 0;
        match_counter++;
        std::string base_name = "stitch_" + std::to_string(match_counter) + "_" + detector_type;

        cv::Mat match_vis_before;
        cv::drawMatches(img1, result1.keypoints, img2, result2.keypoints,
                       match_result.good_matches, match_vis_before,
                       cv::Scalar(0, 255, 0), cv::Scalar(255, 0, 0),
                       std::vector<char>(),
                       cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

        std::string matches_before_path = viz_dir + "/" + base_name + "_matches_before_ransac.jpg";
        cv::imwrite(matches_before_path, match_vis_before);
        std::cout << "Saved matches visualization (before RANSAC)\n";
    }

    std::cout << "Estimating homography...\n";
    HomographyEstimator h_estimator;
    h_estimator.setRANSACThreshold(ransac_threshold);

    std::vector<cv::DMatch> inlier_matches;
    cv::Mat homography = h_estimator.estimateHomography(
        result1.keypoints, result2.keypoints,
        match_result.good_matches, inlier_matches
    );

    auto ransac_result = h_estimator.getLastResult();

    if (!inlier_matches.empty()) {
        namespace fs = std::filesystem;
        std::string viz_dir = "results/visualizations";
        if (!fs::exists(viz_dir)) {
            fs::create_directories(viz_dir);
        }

            static int inlier_counter = 0;
        inlier_counter++;
        std::string base_name = "stitch_" + std::to_string(inlier_counter) + "_" + detector_type;

        cv::Mat match_vis_after;
        cv::drawMatches(img1, result1.keypoints, img2, result2.keypoints,
                       inlier_matches, match_vis_after,
                       cv::Scalar(0, 255, 0), cv::Scalar(255, 0, 0),
                       std::vector<char>(),
                       cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

        std::string matches_after_path = viz_dir + "/" + base_name + "_matches_after_ransac.jpg";
        cv::imwrite(matches_after_path, match_vis_after);
        std::cout << "Saved inlier matches visualization (after RANSAC)\n";
    }

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

    cv::Mat warped2;
    cv::Mat warped_mask2;
    cv::warpPerspective(img2, warped2, translation * H_inv, panorama_size);
    cv::warpPerspective(cv::Mat::ones(img2.size(), CV_8UC1) * 255,
                       warped_mask2, translation * H_inv, panorama_size);

    std::cout << "Blending images...\n";

    std::unique_ptr<Blender> blender;
    try {
        blender = BlenderFactory::createBlender(blend_mode);
    } catch (const std::exception& e) {
        std::cerr << "Error creating blender: " << e.what() << "\n";
        std::cerr << "Falling back to feathering blend mode\n";
        blender = BlenderFactory::createBlender(BlendMode::FEATHERING);
    }

    panorama = blender->blend(warped1, warped2, mask1, warped_mask2);

    std::cout << "Panorama created successfully!\n";

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Total time: " << duration.count() << " ms\n";

    return panorama;
}

cv::Mat StitchingPipeline::performSequentialStitching(
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

    if (images.size() == 1) {
        return images[0].clone();
    }

    size_t middle_idx = images.size() / 2;
    std::cout << "Starting from image " << (middle_idx + 1) << " as reference\n";

    cv::Mat panorama = images[middle_idx].clone();

    for (int i = middle_idx - 1; i >= 0; i--) {
        std::cout << "\n=== Stitching image " << (i + 1) << " (left side) ===\n";

        cv::Mat result = performStitchingDirect(
            images[i], panorama,  // Note: reversed order for left side
            detector_type, blend_mode,
            ransac_threshold, max_features,
            visualize, PanoramaConfig::MAX_PANORAMA_DIMENSION
        );

        if (result.empty()) {
            std::cerr << "Failed to stitch image " << (i + 1) << "\n";
            if (i == 0 && middle_idx + 1 < images.size()) {
                std::cerr << "Continuing with right side images...\n";
                panorama = images[middle_idx].clone();
                break;
            }
            return cv::Mat();
        }
        panorama = result;
    }

    for (size_t i = middle_idx + 1; i < images.size(); i++) {
        std::cout << "\n=== Stitching image " << (i + 1) << " (right side) ===\n";

        cv::Mat result = performStitchingDirect(
            panorama, images[i],
            detector_type, blend_mode,
            ransac_threshold, max_features,
            visualize, PanoramaConfig::MAX_PANORAMA_DIMENSION
        );

        if (result.empty()) {
            std::cerr << "Failed to stitch image " << (i + 1) << "\n";
            if (i == images.size() - 1) {
                std::cerr << "Returning partial panorama...\n";
                return panorama;
            }
            return cv::Mat();
        }
        panorama = result;
    }

    return panorama;
}
