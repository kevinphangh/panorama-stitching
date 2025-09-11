#include <iostream>
#include <filesystem>
#include "experiments/experiment_runner.h"

int main() {
    std::cout << "Running minimal experiment test...\n";
    
    // Create results directory
    std::filesystem::create_directories("results/visualizations");
    
    // Initialize experiment runner
    ExperimentRunner runner;
    
    // Run single experiment
    ExperimentConfig config;
    config.name = "test_experiment";
    config.detector_type = "orb";
    config.ransac_threshold = 3.0;
    config.blend_mode = "feather";
    config.max_features = 2000;
    config.ratio_test_threshold = 0.7;
    
    std::string img1 = "datasets/indoor_scene/img1.jpg";
    std::string img2 = "datasets/indoor_scene/img2.jpg";
    
    std::cout << "Testing with: " << img1 << " and " << img2 << "\n";
    
    auto result = runner.runSingleExperiment(img1, img2, config);
    
    // Output timing results
    std::cout << "\n=== Timing Results ===\n";
    std::cout << "Detection time: " << result.detection_time_ms << " ms\n";
    std::cout << "Matching time: " << result.matching_time_ms << " ms\n";
    std::cout << "Homography time: " << result.homography_time_ms << " ms\n";
    std::cout << "Warping time: " << result.warping_time_ms << " ms\n";
    std::cout << "Blending time: " << result.blending_time_ms << " ms\n";
    std::cout << "Total time: " << result.total_time_ms << " ms\n";
    
    // Check for visualizations
    std::cout << "\n=== Checking Visualizations ===\n";
    std::filesystem::path viz_dir = "results/visualizations";
    int viz_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(viz_dir)) {
        if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
            viz_count++;
            std::cout << "Found: " << entry.path().filename() << "\n";
        }
    }
    std::cout << "Total visualizations: " << viz_count << "\n";
    
    // Export metrics
    runner.exportMetricsToCSV("results/test_metrics.csv");
    
    std::cout << "\nTest complete! Check results/test_metrics.csv for data.\n";
    
    return 0;
}