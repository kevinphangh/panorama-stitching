#include "experiment_runner.h"
#include "../feature_detection/orb_detector.h"
#include "../feature_detection/akaze_detector.h"
#include "../feature_matching/matcher.h"
#include "../homography/homography_estimator.h"
#include "../stitching/image_warper.h"
#include "../stitching/blender.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

ExperimentRunner::ExperimentRunner() {
    results_.clear();
}

void ExperimentRunner::runAllExperiments() {
    std::cout << "Starting experimental evaluation...\n";
    
    // Check for datasets
    std::string dataset_dir = "datasets/";
    if (!fs::exists(dataset_dir)) {
        std::cerr << "Dataset directory not found. Please add images to " << dataset_dir << "\n";
        return;
    }
    
    // Run different experiment types
    runFeatureDetectorComparison(dataset_dir);
    runRANSACThresholdExperiment(dataset_dir);
    runBlendingComparison(dataset_dir);
    
    std::cout << "Experiments completed. " << results_.size() << " results collected.\n";
}

void ExperimentRunner::runFeatureDetectorComparison(const std::string& dataset_path) {
    std::cout << "\n=== Feature Detector Comparison ===\n";
    
    std::vector<std::pair<std::string, std::string>> image_pairs;
    loadDatasets(dataset_path, image_pairs);
    
    if (image_pairs.empty()) {
        std::cout << "No image pairs found in dataset.\n";
        return;
    }
    
    std::vector<std::string> detectors = {"orb", "akaze"};
    
    for (const auto& [img1_path, img2_path] : image_pairs) {
        for (const auto& detector : detectors) {
            ExperimentConfig config;
            config.name = "detector_comparison";
            config.detector_type = detector;
            config.ransac_threshold = 3.0;
            config.blend_mode = "feather";
            config.max_features = 2000;
            config.ratio_test_threshold = 0.7;
            
            std::cout << "Testing " << detector << " on " << img1_path << "\n";
            auto result = runSingleExperiment(img1_path, img2_path, config);
            results_.push_back(result);
            
            // Store metrics for analysis
            metrics_by_detector_[detector].push_back(result.num_inliers);
            metrics_by_detector_[detector].push_back(result.detection_time_ms);
        }
    }
}

void ExperimentRunner::runRANSACThresholdExperiment(const std::string& dataset_path) {
    std::cout << "\n=== RANSAC Threshold Experiment ===\n";
    
    std::vector<std::pair<std::string, std::string>> image_pairs;
    loadDatasets(dataset_path, image_pairs);
    
    if (image_pairs.empty()) return;
    
    std::vector<double> thresholds = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    for (const auto& [img1_path, img2_path] : image_pairs) {
        for (double threshold : thresholds) {
            ExperimentConfig config;
            config.name = "ransac_threshold";
            config.detector_type = "orb";
            config.ransac_threshold = threshold;
            config.blend_mode = "feather";
            config.max_features = 2000;
            config.ratio_test_threshold = 0.7;
            
            std::cout << "Testing RANSAC threshold " << threshold << "\n";
            auto result = runSingleExperiment(img1_path, img2_path, config);
            results_.push_back(result);
            
            metrics_by_threshold_[threshold].push_back(result.inlier_ratio);
            metrics_by_threshold_[threshold].push_back(result.reprojection_error);
        }
    }
}

void ExperimentRunner::runBlendingComparison(const std::string& dataset_path) {
    std::cout << "\n=== Blending Method Comparison ===\n";
    
    std::vector<std::pair<std::string, std::string>> image_pairs;
    loadDatasets(dataset_path, image_pairs);
    
    if (image_pairs.empty()) return;
    
    std::vector<std::string> blend_modes = {"simple", "feather", "multiband"};
    
    for (const auto& [img1_path, img2_path] : image_pairs) {
        for (const auto& mode : blend_modes) {
            ExperimentConfig config;
            config.name = "blending_comparison";
            config.detector_type = "orb";
            config.ransac_threshold = 3.0;
            config.blend_mode = mode;
            config.max_features = 2000;
            config.ratio_test_threshold = 0.7;
            
            std::cout << "Testing blend mode: " << mode << "\n";
            auto result = runSingleExperiment(img1_path, img2_path, config);
            results_.push_back(result);
        }
    }
}

ExperimentResult ExperimentRunner::runSingleExperiment(
    const std::string& img1_path,
    const std::string& img2_path,
    const ExperimentConfig& config) {
    
    ExperimentResult result;
    result.config = config;
    
    auto total_start = std::chrono::high_resolution_clock::now();
    
    // Load images
    cv::Mat img1 = cv::imread(img1_path);
    cv::Mat img2 = cv::imread(img2_path);
    
    if (img1.empty() || img2.empty()) {
        std::cerr << "Failed to load images\n";
        return result;
    }
    
    // Feature detection
    std::unique_ptr<FeatureDetector> detector;
    if (config.detector_type == "orb") {
        detector = std::make_unique<ORBDetector>();
    } else {
        detector = std::make_unique<AKAZEDetector>();
    }
    detector->setMaxFeatures(config.max_features);
    
    auto det_result1 = detector->detect(img1);
    auto det_result2 = detector->detect(img2);
    
    result.num_keypoints_img1 = det_result1.getKeypointCount();
    result.num_keypoints_img2 = det_result2.getKeypointCount();
    result.detection_time_ms = det_result1.detection_time_ms + det_result2.detection_time_ms;
    result.description_time_ms = det_result1.description_time_ms + det_result2.description_time_ms;
    
    // Feature matching
    FeatureMatcher matcher;
    auto match_result = matcher.matchFeatures(
        det_result1.descriptors, det_result2.descriptors,
        det_result1.keypoints, det_result2.keypoints,
        config.ratio_test_threshold
    );
    
    result.num_initial_matches = match_result.num_initial_matches;
    result.num_good_matches = match_result.num_good_matches;
    result.matching_time_ms = match_result.matching_time_ms;
    
    // Homography estimation
    auto h_start = std::chrono::high_resolution_clock::now();
    HomographyEstimator h_estimator;
    h_estimator.setRANSACThreshold(config.ransac_threshold);
    
    std::vector<cv::DMatch> inlier_matches;
    cv::Mat homography = h_estimator.estimateHomography(
        det_result1.keypoints, det_result2.keypoints,
        match_result.good_matches, inlier_matches
    );
    auto h_end = std::chrono::high_resolution_clock::now();
    
    auto ransac_result = h_estimator.getLastResult();
    result.num_inliers = ransac_result.num_inliers;
    result.inlier_ratio = ransac_result.inlier_ratio;
    result.reprojection_error = ransac_result.reprojection_error;
    result.ransac_iterations = ransac_result.num_iterations;
    result.homography_time_ms = std::chrono::duration<double, std::milli>(h_end - h_start).count();
    
    // Image stitching
    if (!homography.empty()) {
        // Warp
        auto warp_start = std::chrono::high_resolution_clock::now();
        ImageWarper warper;
        auto bounds = HomographyEstimator::calculateOutputBounds(img1, img2, homography);
        cv::Mat panorama = cv::Mat::zeros(bounds.height, bounds.width, img1.type());
        img1.copyTo(panorama(cv::Rect(0, 0, img1.cols, img1.rows)));
        
        cv::Mat warped2 = warper.warpPerspective(img2, homography, panorama.size());
        auto warp_end = std::chrono::high_resolution_clock::now();
        result.warping_time_ms = std::chrono::duration<double, std::milli>(warp_end - warp_start).count();
        
        // Blend
        auto blend_start = std::chrono::high_resolution_clock::now();
        Blender blender;
        if (config.blend_mode == "simple") {
            blender.setBlendMode(BlendMode::SIMPLE_OVERLAY);
        } else if (config.blend_mode == "feather") {
            blender.setBlendMode(BlendMode::FEATHERING);
        } else if (config.blend_mode == "multiband") {
            blender.setBlendMode(BlendMode::MULTIBAND);
        }
        
        cv::Mat mask1 = cv::Mat::zeros(panorama.size(), CV_8UC1);
        mask1(cv::Rect(0, 0, img1.cols, img1.rows)) = 255;
        cv::Mat mask2;
        cv::warpPerspective(cv::Mat::ones(img2.size(), CV_8UC1) * 255, 
                           mask2, homography, panorama.size());
        
        result.panorama = blender.blend(panorama, warped2, mask1, mask2);
        auto blend_end = std::chrono::high_resolution_clock::now();
        result.blending_time_ms = std::chrono::duration<double, std::milli>(blend_end - blend_start).count();
    }
    
    auto total_end = std::chrono::high_resolution_clock::now();
    result.total_time_ms = std::chrono::duration<double, std::milli>(total_end - total_start).count();
    
    return result;
}

void ExperimentRunner::loadDatasets(const std::string& dataset_dir,
                                   std::vector<std::pair<std::string, std::string>>& image_pairs) {
    // Look for image pairs in subdirectories
    for (const auto& entry : fs::directory_iterator(dataset_dir)) {
        if (fs::is_directory(entry)) {
            std::vector<std::string> images;
            for (const auto& file : fs::directory_iterator(entry)) {
                std::string ext = file.path().extension();
                if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") {
                    images.push_back(file.path().string());
                }
            }
            
            // Create pairs from consecutive images
            for (size_t i = 0; i + 1 < images.size(); i++) {
                image_pairs.push_back({images[i], images[i + 1]});
            }
        }
    }
}

void ExperimentRunner::saveResults(const std::string& output_dir) {
    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }
    
    // Save panoramas
    int idx = 0;
    for (const auto& result : results_) {
        if (!result.panorama.empty()) {
            std::string filename = output_dir + "/panorama_" + 
                                 result.config.detector_type + "_" +
                                 std::to_string(idx++) + ".jpg";
            cv::imwrite(filename, result.panorama);
        }
    }
    
    exportMetricsToCSV(output_dir + "/metrics.csv");
}

void ExperimentRunner::exportMetricsToCSV(const std::string& csv_path) {
    std::ofstream file(csv_path);
    
    // Write header
    file << "experiment,detector,ransac_threshold,blend_mode,";
    file << "num_keypoints_1,num_keypoints_2,num_matches,num_inliers,";
    file << "inlier_ratio,reprojection_error,";
    file << "detection_time,matching_time,homography_time,total_time\n";
    
    // Write data
    for (const auto& result : results_) {
        file << result.config.name << ","
             << result.config.detector_type << ","
             << result.config.ransac_threshold << ","
             << result.config.blend_mode << ","
             << result.num_keypoints_img1 << ","
             << result.num_keypoints_img2 << ","
             << result.num_good_matches << ","
             << result.num_inliers << ","
             << result.inlier_ratio << ","
             << result.reprojection_error << ","
             << result.detection_time_ms << ","
             << result.matching_time_ms << ","
             << result.homography_time_ms << ","
             << result.total_time_ms << "\n";
    }
    
    file.close();
    std::cout << "Metrics saved to " << csv_path << "\n";
}

void ExperimentRunner::generateReport(const std::string& output_path) {
    std::cout << "Generating report to " << output_path << "\n";
}