#include "experiment_runner.h"
#include "visualization.h"
#include "report_generator.h"
#include "../feature_detection/detector_factory.h"
#include "../feature_matching/matcher.h"
#include "../homography/homography_estimator.h"
#include "../stitching/image_warper.h"
#include "../stitching/blender.h"
#include "../stitching/blender_factory.h"
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
    
    std::string dataset_dir = "datasets/";
    if (!fs::exists(dataset_dir)) {
        std::cerr << "Dataset directory not found. Please add images to " << dataset_dir << "\n";
        return;
    }
    
    runFeatureDetectorComparison(dataset_dir);
    runRANSACThresholdExperiment(dataset_dir);
    runBlendingComparison(dataset_dir);
    
    std::cout << "Experiments completed. " << results_.size() << " results collected.\n";
    
    exportMetricsToCSV("results/metrics.csv");
    exportMatchDistances("results");
    generateVisualizations("results");
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
    
    cv::Mat img1 = cv::imread(img1_path);
    cv::Mat img2 = cv::imread(img2_path);
    
    if (img1.empty() || img2.empty()) {
        std::cerr << "Failed to load images\n";
        return result;
    }
    
    // Save original images for visualization
    std::string exp_name = fs::path(img1_path).parent_path().filename().string() + "_" + 
                          fs::path(img1_path).stem().string() + "_" + 
                          fs::path(img2_path).stem().string() + "_" + 
                          config.detector_type;
    
    std::string viz_dir = "results/visualizations";
    if (!fs::exists(viz_dir)) {
        fs::create_directories(viz_dir);
    }
    
    cv::imwrite(viz_dir + "/" + exp_name + "_img1.jpg", img1);
    cv::imwrite(viz_dir + "/" + exp_name + "_img2.jpg", img2);
    
    std::unique_ptr<FeatureDetector> detector;
    try {
        detector = DetectorFactory::createDetector(config.detector_type);
    } catch (const std::exception& e) {
        std::cerr << "Error creating detector: " << e.what() << "\n";
        return ExperimentResult{}; // Return empty result
    }
    detector->setMaxFeatures(config.max_features);
    
    auto det_result1 = detector->detect(img1);
    auto det_result2 = detector->detect(img2);
    
    result.num_keypoints_img1 = det_result1.getKeypointCount();
    result.num_keypoints_img2 = det_result2.getKeypointCount();
    result.detection_time_ms = det_result1.detection_time_ms + det_result2.detection_time_ms;
    result.description_time_ms = det_result1.description_time_ms + det_result2.description_time_ms;
    
    // Save keypoint visualizations
    cv::Mat kp_vis1, kp_vis2;
    cv::drawKeypoints(img1, det_result1.keypoints, kp_vis1, cv::Scalar(0, 255, 0), 
                     cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(img2, det_result2.keypoints, kp_vis2, cv::Scalar(0, 255, 0),
                     cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::imwrite(viz_dir + "/" + exp_name + "_keypoints1.jpg", kp_vis1);
    cv::imwrite(viz_dir + "/" + exp_name + "_keypoints2.jpg", kp_vis2);
    
    FeatureMatcher matcher;
    auto match_result = matcher.matchFeatures(
        det_result1.descriptors, det_result2.descriptors,
        det_result1.keypoints, det_result2.keypoints,
        config.ratio_test_threshold
    );
    
    result.num_initial_matches = match_result.num_initial_matches;
    result.num_good_matches = match_result.num_good_matches;
    result.matching_time_ms = match_result.matching_time_ms;
    result.match_distances = match_result.match_distances;
    
    // Save match visualization (before RANSAC)
    cv::Mat match_vis;
    cv::drawMatches(img1, det_result1.keypoints, img2, det_result2.keypoints,
                   match_result.good_matches, match_vis, cv::Scalar(0, 255, 0),
                   cv::Scalar(255, 0, 0), std::vector<char>(),
                   cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    cv::imwrite(viz_dir + "/" + exp_name + "_matches_before.jpg", match_vis);
    
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
    
    // Save inlier match visualization (after RANSAC)
    if (!inlier_matches.empty()) {
        cv::Mat inlier_vis;
        cv::drawMatches(img1, det_result1.keypoints, img2, det_result2.keypoints,
                       inlier_matches, inlier_vis, cv::Scalar(0, 255, 0),
                       cv::Scalar(255, 0, 0), std::vector<char>(),
                       cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
        cv::imwrite(viz_dir + "/" + exp_name + "_matches_after.jpg", inlier_vis);
    }
    
    if (!homography.empty()) {
        auto warp_start = std::chrono::high_resolution_clock::now();
        ImageWarper warper;
        auto bounds = HomographyEstimator::calculateOutputBounds(img1, img2, homography);
        cv::Mat panorama = cv::Mat::zeros(bounds.height, bounds.width, img1.type());
        img1.copyTo(panorama(cv::Rect(0, 0, img1.cols, img1.rows)));
        
        cv::Mat warped2 = warper.warpPerspective(img2, homography, panorama.size());
        auto warp_end = std::chrono::high_resolution_clock::now();
        result.warping_time_ms = std::chrono::duration<double, std::milli>(warp_end - warp_start).count();
        
        auto blend_start = std::chrono::high_resolution_clock::now();
        std::unique_ptr<Blender> blender;
        try {
            blender = BlenderFactory::createBlender(config.blend_mode);
        } catch (const std::exception& e) {
            std::cerr << "Error creating blender: " << e.what() << "\n";
            return ExperimentResult{}; // Return empty result
        }
        
        cv::Mat mask1 = cv::Mat::zeros(panorama.size(), CV_8UC1);
        mask1(cv::Rect(0, 0, img1.cols, img1.rows)) = 255;
        cv::Mat mask2;
        cv::warpPerspective(cv::Mat::ones(img2.size(), CV_8UC1) * 255, 
                           mask2, homography, panorama.size());
        
        result.panorama = blender->blend(panorama, warped2, mask1, mask2);
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
    ReportGenerator generator;
    generator.saveExperimentResults(results_, output_dir);

    exportMetricsToCSV(output_dir + "/metrics.csv");
}

void ExperimentRunner::exportMetricsToCSV(const std::string& csv_path) {
    ReportGenerator generator;
    generator.exportToCSV(results_, csv_path);
}

void ExperimentRunner::generateReport(const std::string& output_path) {
    ReportGenerator generator;
    generator.generateMarkdownReport(results_, output_path);
}

void ExperimentRunner::generateVisualizations(const std::string& output_dir) {
    std::cout << "Generating visualizations...\n";
    
    // Create output directory if it doesn't exist
    fs::create_directories(output_dir);
    
    // Generate visualizations from the CSV file
    std::string csv_path = output_dir + "/metrics.csv";
    if (fs::exists(csv_path)) {
        Visualization::generateExperimentReport(csv_path, output_dir);
    }
    
    // Generate match distance histograms for each detector
    std::map<std::string, std::vector<double>> distances_by_detector;
    
    for (const auto& result : results_) {
        if (!result.match_distances.empty()) {
            distances_by_detector[result.config.detector_type].insert(
                distances_by_detector[result.config.detector_type].end(),
                result.match_distances.begin(),
                result.match_distances.end()
            );
        }
    }
    
    // Save histograms as images
    for (const auto& [detector, distances] : distances_by_detector) {
        if (!distances.empty()) {
            std::string title = detector + " Match Distances";
            cv::Mat histogram = Visualization::generateMatchDistanceHistogram(distances, title);
            if (!histogram.empty()) {
                std::string filename = output_dir + "/" + detector + "_match_histogram.png";
                cv::imwrite(filename, histogram);
                std::cout << "Saved histogram: " << filename << "\n";
            }
        }
    }
    
    std::cout << "Visualizations saved to " << output_dir << "\n";
}

void ExperimentRunner::exportMatchDistances(const std::string& output_dir) {
    ReportGenerator generator;
    generator.exportMatchDistances(results_, output_dir);
}