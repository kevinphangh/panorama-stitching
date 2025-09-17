#include "report_generator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <opencv2/opencv.hpp>

bool ReportGenerator::generateMarkdownReport(const std::vector<ExperimentResult>& results,
                                            const std::string& output_path) {
    std::ofstream file(output_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open report file: " << output_path << "\n";
        return false;
    }

    file << "# Panorama Stitching Experiment Report\n\n";
    file << "## Summary\n\n";
    file << "Total experiments run: " << results.size() << "\n\n";

    file << generateSummaryTable(results);
    file << "\n## Detailed Results\n\n";
    file << generateDetailedResults(results);

    file.close();
    std::cout << "Report generated: " << output_path << "\n";
    return true;
}

bool ReportGenerator::exportToCSV(const std::vector<ExperimentResult>& results,
                                 const std::string& csv_path) {
    std::ofstream file(csv_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file for writing: " << csv_path << "\n";
        return false;
    }

    writeCSVHeader(file);

    for (const auto& result : results) {
        writeCSVRow(file, result);
    }

    file.close();
    std::cout << "Metrics saved to " << csv_path << "\n";
    return true;
}

bool ReportGenerator::exportMatchDistances(const std::vector<ExperimentResult>& results,
                                          const std::string& output_dir) {
    namespace fs = std::filesystem;

    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }

    for (const auto& result : results) {
        if (result.match_distances.empty()) continue;

        std::string filename = output_dir + "/" + result.config.name +
                              "_" + result.config.detector_type + "_distances.csv";
        std::ofstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file: " << filename << "\n";
            continue;
        }

        file << "match_index,distance\n";
        for (size_t i = 0; i < result.match_distances.size(); ++i) {
            file << i << "," << result.match_distances[i] << "\n";
        }

        file.close();
    }

    std::cout << "Match distances exported to " << output_dir << "\n";
    return true;
}

bool ReportGenerator::saveExperimentResults(const std::vector<ExperimentResult>& results,
                                           const std::string& output_dir) {
    namespace fs = std::filesystem;

    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }

    for (const auto& result : results) {
        if (!result.panorama.empty()) {
            std::string filename = output_dir + "/" + result.config.name + "_" +
                                 result.config.detector_type + "_" +
                                 result.config.blend_mode + ".jpg";
            cv::imwrite(filename, result.panorama);
        }
    }

    std::cout << "Results saved to " << output_dir << "\n";
    return true;
}

std::string ReportGenerator::formatDuration(double ms) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << ms << " ms";
    return oss.str();
}

std::string ReportGenerator::generateSummaryTable(const std::vector<ExperimentResult>& results) const {
    std::ostringstream oss;

    oss << "| Experiment | Detector | RANSAC | Blend Mode | Inliers | Time (ms) |\n";
    oss << "|------------|----------|--------|------------|---------|----------|\n";

    for (const auto& r : results) {
        oss << "| " << r.config.name << " | "
            << r.config.detector_type << " | "
            << r.config.ransac_threshold << " | "
            << r.config.blend_mode << " | "
            << r.num_inliers << " | "
            << formatDuration(r.total_time_ms) << " |\n";
    }

    return oss.str();
}

std::string ReportGenerator::generateDetailedResults(const std::vector<ExperimentResult>& results) const {
    std::ostringstream oss;

    for (const auto& r : results) {
        oss << "### " << r.config.name << "\n\n";
        oss << "**Configuration:**\n";
        oss << "- Detector: " << r.config.detector_type << "\n";
        oss << "- RANSAC Threshold: " << r.config.ransac_threshold << "\n";
        oss << "- Blend Mode: " << r.config.blend_mode << "\n";
        oss << "- Max Features: " << r.config.max_features << "\n\n";

        oss << "**Results:**\n";
        oss << "- Keypoints: " << r.num_keypoints_img1 << " / " << r.num_keypoints_img2 << "\n";
        oss << "- Matches: " << r.num_good_matches << " (initial: " << r.num_initial_matches << ")\n";
        oss << "- Inliers: " << r.num_inliers << " (" << (r.inlier_ratio * 100) << "%)\n";
        oss << "- Reprojection Error: " << r.reprojection_error << "\n\n";

        oss << "**Timing:**\n";
        oss << "- Detection: " << formatDuration(r.detection_time_ms) << "\n";
        oss << "- Matching: " << formatDuration(r.matching_time_ms) << "\n";
        oss << "- Homography: " << formatDuration(r.homography_time_ms) << "\n";
        oss << "- Warping: " << formatDuration(r.warping_time_ms) << "\n";
        oss << "- Blending: " << formatDuration(r.blending_time_ms) << "\n";
        oss << "- **Total: " << formatDuration(r.total_time_ms) << "**\n\n";
    }

    return oss.str();
}

void ReportGenerator::writeCSVHeader(std::ofstream& file) const {
    file << "experiment,detector,ransac_threshold,blend_mode,";
    file << "num_keypoints_1,num_keypoints_2,num_matches,num_inliers,";
    file << "inlier_ratio,reprojection_error,";
    file << "detection_time,matching_time,homography_time,warping_time,blending_time,total_time\n";
}

void ReportGenerator::writeCSVRow(std::ofstream& file, const ExperimentResult& result) const {
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
         << result.warping_time_ms << ","
         << result.blending_time_ms << ","
         << result.total_time_ms << "\n";
}