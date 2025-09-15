#ifndef EXPERIMENT_RUNNER_H
#define EXPERIMENT_RUNNER_H

#include <string>
#include <vector>
#include <map>
#include <opencv2/core.hpp>

struct ExperimentConfig {
    std::string name;
    std::string detector_type;
    double ransac_threshold;
    std::string blend_mode;
    int max_features;
    double ratio_test_threshold;
};

struct ExperimentResult {
    ExperimentConfig config;
    
    // Feature detection metrics
    int num_keypoints_img1;
    int num_keypoints_img2;
    double detection_time_ms;
    double description_time_ms;
    
    // Matching metrics
    int num_initial_matches;
    int num_good_matches;
    int num_inliers;
    double inlier_ratio;
    double matching_time_ms;
    std::vector<double> match_distances;
    
    // Homography metrics
    double homography_time_ms;
    double reprojection_error;
    int ransac_iterations;
    
    // Stitching metrics
    double warping_time_ms;
    double blending_time_ms;
    double total_time_ms;
    
    cv::Mat panorama;
};

class ExperimentRunner {
public:
    ExperimentRunner();
    
    void runAllExperiments();
    void runFeatureDetectorComparison(const std::string& dataset_path);
    void runRANSACThresholdExperiment(const std::string& dataset_path);
    void runBlendingComparison(const std::string& dataset_path);
    
    ExperimentResult runSingleExperiment(
        const std::string& img1_path,
        const std::string& img2_path,
        const ExperimentConfig& config
    );
    
    void saveResults(const std::string& output_dir);
    void generateReport(const std::string& output_path);
    void exportMetricsToCSV(const std::string& csv_path);
    void generateVisualizations(const std::string& output_dir);
    void exportMatchDistances(const std::string& output_dir);
    
private:
    std::vector<ExperimentResult> results_;
    
    void loadDatasets(const std::string& dataset_dir,
                     std::vector<std::pair<std::string, std::string>>& image_pairs);
};

#endif