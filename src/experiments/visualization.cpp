#include "visualization.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>

cv::Mat Visualization::generateMatchDistanceHistogram(
    const std::vector<double>& distances,
    const std::string& title) {
    
    if (distances.empty()) {
        return cv::Mat();
    }
    
    // Parameters for histogram
    int hist_w = 800;
    int hist_h = 400;
    int bin_count = 50;
    
    // Find min and max distances
    double min_dist = *std::min_element(distances.begin(), distances.end());
    double max_dist = *std::max_element(distances.begin(), distances.end());
    
    if (max_dist <= min_dist) {
        max_dist = min_dist + 1.0;
    }
    
    // Create histogram bins
    std::vector<int> histogram(bin_count, 0);
    double bin_width = (max_dist - min_dist) / bin_count;
    
    // Fill histogram
    for (double dist : distances) {
        int bin = static_cast<int>((dist - min_dist) / bin_width);
        if (bin >= bin_count) bin = bin_count - 1;
        if (bin < 0) bin = 0;
        histogram[bin]++;
    }
    
    // Find max count for scaling
    int max_count = *std::max_element(histogram.begin(), histogram.end());
    
    // Draw histogram
    cv::Mat hist_image = drawHistogram(histogram, hist_w, hist_h, bin_count, max_count, title);
    
    // Add labels
    cv::putText(hist_image, "Distance", 
                cv::Point(hist_w/2 - 30, hist_h - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    
    cv::putText(hist_image, "Count", 
                cv::Point(10, 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    
    // Add statistics
    double mean = std::accumulate(distances.begin(), distances.end(), 0.0) / distances.size();
    std::stringstream stats;
    stats << "Mean: " << std::fixed << std::setprecision(2) << mean 
          << "  Min: " << min_dist 
          << "  Max: " << max_dist
          << "  Count: " << distances.size();
    
    cv::putText(hist_image, stats.str(), 
                cv::Point(hist_w/4, hist_h - 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(100, 100, 100), 1);
    
    return hist_image;
}

cv::Mat Visualization::plotMetrics(
    const std::vector<double>& x_values,
    const std::vector<double>& y_values,
    const std::string& x_label,
    const std::string& y_label,
    const std::string& title) {
    
    if (x_values.empty() || y_values.empty() || x_values.size() != y_values.size()) {
        return cv::Mat();
    }
    
    int plot_w = 800;
    int plot_h = 600;
    int margin = 80;
    
    // Create white background
    cv::Mat plot_image(plot_h, plot_w, CV_8UC3, cv::Scalar(255, 255, 255));
    
    // Find data ranges
    double x_min = *std::min_element(x_values.begin(), x_values.end());
    double x_max = *std::max_element(x_values.begin(), x_values.end());
    double y_min = *std::min_element(y_values.begin(), y_values.end());
    double y_max = *std::max_element(y_values.begin(), y_values.end());
    
    // Add some padding
    double x_range = x_max - x_min;
    double y_range = y_max - y_min;
    if (x_range == 0) x_range = 1;
    if (y_range == 0) y_range = 1;
    
    x_min -= x_range * 0.1;
    x_max += x_range * 0.1;
    y_min -= y_range * 0.1;
    y_max += y_range * 0.1;
    
    // Convert data to plot coordinates
    std::vector<cv::Point2f> points;
    for (size_t i = 0; i < x_values.size(); ++i) {
        float px = margin + (x_values[i] - x_min) / (x_max - x_min) * (plot_w - 2*margin);
        float py = plot_h - margin - (y_values[i] - y_min) / (y_max - y_min) * (plot_h - 2*margin);
        points.push_back(cv::Point2f(px, py));
    }
    
    // Draw axes
    cv::line(plot_image, 
             cv::Point(margin, plot_h - margin), 
             cv::Point(plot_w - margin, plot_h - margin), 
             cv::Scalar(0, 0, 0), 2);
    cv::line(plot_image, 
             cv::Point(margin, margin), 
             cv::Point(margin, plot_h - margin), 
             cv::Scalar(0, 0, 0), 2);
    
    // Draw grid lines
    int grid_lines = 5;
    for (int i = 1; i < grid_lines; ++i) {
        int x = margin + i * (plot_w - 2*margin) / grid_lines;
        int y = margin + i * (plot_h - 2*margin) / grid_lines;
        
        cv::line(plot_image, 
                 cv::Point(x, margin), 
                 cv::Point(x, plot_h - margin), 
                 cv::Scalar(230, 230, 230), 1);
        cv::line(plot_image, 
                 cv::Point(margin, y), 
                 cv::Point(plot_w - margin, y), 
                 cv::Scalar(230, 230, 230), 1);
    }
    
    // Plot data points and lines
    for (size_t i = 0; i < points.size(); ++i) {
        cv::circle(plot_image, points[i], 5, cv::Scalar(0, 0, 255), -1);
        if (i > 0) {
            cv::line(plot_image, points[i-1], points[i], cv::Scalar(255, 0, 0), 2);
        }
    }
    
    // Add title
    cv::putText(plot_image, title, 
                cv::Point(plot_w/2 - title.length()*8, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
    
    // Add axis labels
    cv::putText(plot_image, x_label, 
                cv::Point(plot_w/2 - x_label.length()*4, plot_h - 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
    
    // Y-axis label (rotated text approximation)
    cv::putText(plot_image, y_label, 
                cv::Point(15, plot_h/2),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
    
    // Add axis values
    for (int i = 0; i <= grid_lines; ++i) {
        double x_val = x_min + i * (x_max - x_min) / grid_lines;
        double y_val = y_min + i * (y_max - y_min) / grid_lines;
        
        std::stringstream x_str, y_str;
        x_str << std::fixed << std::setprecision(1) << x_val;
        y_str << std::fixed << std::setprecision(1) << y_val;
        
        int x_pos = margin + i * (plot_w - 2*margin) / grid_lines;
        int y_pos = plot_h - margin - i * (plot_h - 2*margin) / grid_lines;
        
        cv::putText(plot_image, x_str.str(), 
                    cv::Point(x_pos - 15, plot_h - margin + 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
        
        cv::putText(plot_image, y_str.str(), 
                    cv::Point(margin - 35, y_pos + 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    }
    
    return plot_image;
}

cv::Mat Visualization::generateComparisonChart(
    const std::vector<std::string>& labels,
    const std::vector<double>& values,
    const std::string& title,
    const std::string& y_label) {
    
    if (labels.empty() || values.empty() || labels.size() != values.size()) {
        return cv::Mat();
    }
    
    int chart_w = 800;
    int chart_h = 600;
    int margin = 80;
    int bar_width = (chart_w - 2*margin) / (labels.size() * 2);
    
    // Create white background
    cv::Mat chart_image(chart_h, chart_w, CV_8UC3, cv::Scalar(255, 255, 255));
    
    // Find max value for scaling
    double max_val = *std::max_element(values.begin(), values.end());
    if (max_val == 0) max_val = 1;
    
    // Draw axes
    cv::line(chart_image, 
             cv::Point(margin, chart_h - margin), 
             cv::Point(chart_w - margin, chart_h - margin), 
             cv::Scalar(0, 0, 0), 2);
    cv::line(chart_image, 
             cv::Point(margin, margin), 
             cv::Point(margin, chart_h - margin), 
             cv::Scalar(0, 0, 0), 2);
    
    // Draw bars
    for (size_t i = 0; i < labels.size(); ++i) {
        int bar_height = static_cast<int>((values[i] / max_val) * (chart_h - 2*margin));
        int x_pos = margin + (i * 2 + 1) * bar_width;
        
        cv::rectangle(chart_image,
                     cv::Point(x_pos - bar_width/2, chart_h - margin - bar_height),
                     cv::Point(x_pos + bar_width/2, chart_h - margin),
                     cv::Scalar(100, 150, 200), -1);
        
        // Draw bar outline
        cv::rectangle(chart_image,
                     cv::Point(x_pos - bar_width/2, chart_h - margin - bar_height),
                     cv::Point(x_pos + bar_width/2, chart_h - margin),
                     cv::Scalar(0, 0, 0), 2);
        
        // Add labels
        cv::putText(chart_image, labels[i], 
                    cv::Point(x_pos - labels[i].length()*4, chart_h - margin + 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        
        // Add value on top of bar
        std::stringstream val_str;
        val_str << std::fixed << std::setprecision(1) << values[i];
        cv::putText(chart_image, val_str.str(), 
                    cv::Point(x_pos - 20, chart_h - margin - bar_height - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    }
    
    // Add title
    cv::putText(chart_image, title, 
                cv::Point(chart_w/2 - title.length()*8, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
    
    // Add y-axis label
    cv::putText(chart_image, y_label, 
                cv::Point(15, chart_h/2),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
    
    return chart_image;
}

cv::Mat Visualization::drawHistogram(
    const std::vector<int>& histogram,
    int hist_w,
    int hist_h,
    int bin_count,
    double max_value,
    const std::string& title) {
    
    cv::Mat hist_image(hist_h + 60, hist_w, CV_8UC3, cv::Scalar(255, 255, 255));
    
    // Draw title
    cv::putText(hist_image, title, 
                cv::Point(hist_w/2 - title.length()*8, 25),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
    
    int bin_w = cvRound((double) hist_w / bin_count);
    int margin = 40;
    
    // Normalize histogram
    std::vector<int> normalized_hist(bin_count);
    for (int i = 0; i < bin_count; ++i) {
        normalized_hist[i] = cvRound((double)histogram[i] / max_value * (hist_h - margin));
    }
    
    // Draw histogram bars
    for (int i = 0; i < bin_count; ++i) {
        cv::rectangle(hist_image,
                     cv::Point(i * bin_w, hist_h - normalized_hist[i] + margin),
                     cv::Point((i + 1) * bin_w, hist_h + margin),
                     cv::Scalar(100, 150, 200), -1);
        
        cv::rectangle(hist_image,
                     cv::Point(i * bin_w, hist_h - normalized_hist[i] + margin),
                     cv::Point((i + 1) * bin_w, hist_h + margin),
                     cv::Scalar(0, 0, 0), 1);
    }
    
    // Draw axes
    cv::line(hist_image, 
             cv::Point(0, hist_h + margin), 
             cv::Point(hist_w, hist_h + margin), 
             cv::Scalar(0, 0, 0), 2);
    cv::line(hist_image, 
             cv::Point(0, margin), 
             cv::Point(0, hist_h + margin), 
             cv::Scalar(0, 0, 0), 2);
    
    return hist_image;
}

bool Visualization::saveVisualization(
    const cv::Mat& visualization,
    const std::string& filepath) {
    
    if (visualization.empty()) {
        return false;
    }
    
    return cv::imwrite(filepath, visualization);
}

void Visualization::generateExperimentReport(
    const std::string& csv_path,
    const std::string& output_dir) {
    
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        std::cerr << "Could not open CSV file: " << csv_path << std::endl;
        return;
    }
    
    // Parse CSV data
    std::string line;
    std::getline(file, line); // Skip header
    
    std::vector<double> orb_times, akaze_times;
    std::vector<double> orb_inliers, akaze_inliers;
    std::vector<double> thresholds, threshold_inliers;
    std::map<std::string, double> blend_times;
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string experiment, detector, thresh_str, blend_mode;
        double detection_time, num_inliers;
        
        std::getline(ss, experiment, ',');
        std::getline(ss, detector, ',');
        std::getline(ss, thresh_str, ',');
        std::getline(ss, blend_mode, ',');
        
        // Skip to relevant fields
        std::string temp;
        for (int i = 0; i < 4; ++i) std::getline(ss, temp, ',');
        ss >> num_inliers;
        ss.ignore();
        for (int i = 0; i < 2; ++i) std::getline(ss, temp, ',');
        ss >> detection_time;
        
        // Collect data by experiment type
        if (experiment.find("detector") != std::string::npos) {
            if (detector == "orb") {
                orb_times.push_back(detection_time);
                orb_inliers.push_back(num_inliers);
            } else if (detector == "akaze") {
                akaze_times.push_back(detection_time);
                akaze_inliers.push_back(num_inliers);
            }
        }
        
        if (experiment.find("ransac") != std::string::npos) {
            double threshold = std::stod(thresh_str);
            thresholds.push_back(threshold);
            threshold_inliers.push_back(num_inliers);
        }
        
        if (experiment.find("blend") != std::string::npos) {
            blend_times[blend_mode] = detection_time;
        }
    }
    
    file.close();
    
    // Generate visualizations
    std::cout << "Generating experiment visualizations...\n";
    
    // 1. Detector comparison chart
    if (!orb_times.empty() && !akaze_times.empty()) {
        std::vector<std::string> detector_labels = {"ORB", "AKAZE"};
        std::vector<double> avg_times = {
            std::accumulate(orb_times.begin(), orb_times.end(), 0.0) / orb_times.size(),
            std::accumulate(akaze_times.begin(), akaze_times.end(), 0.0) / akaze_times.size()
        };
        
        cv::Mat detector_chart = generateComparisonChart(
            detector_labels, avg_times,
            "Feature Detector Performance Comparison",
            "Detection Time (ms)"
        );
        saveVisualization(detector_chart, output_dir + "/detector_comparison.jpg");
        
        // Inlier comparison
        std::vector<double> avg_inliers = {
            std::accumulate(orb_inliers.begin(), orb_inliers.end(), 0.0) / orb_inliers.size(),
            std::accumulate(akaze_inliers.begin(), akaze_inliers.end(), 0.0) / akaze_inliers.size()
        };
        
        cv::Mat inlier_chart = generateComparisonChart(
            detector_labels, avg_inliers,
            "Detector Match Quality Comparison",
            "Number of Inliers"
        );
        saveVisualization(inlier_chart, output_dir + "/inlier_comparison.jpg");
    }
    
    // 2. RANSAC threshold plot
    if (!thresholds.empty() && !threshold_inliers.empty()) {
        cv::Mat threshold_plot = plotMetrics(
            thresholds, threshold_inliers,
            "RANSAC Threshold",
            "Number of Inliers",
            "RANSAC Threshold vs Match Quality"
        );
        saveVisualization(threshold_plot, output_dir + "/ransac_threshold_plot.jpg");
    }
    
    // 3. Blending method comparison
    if (!blend_times.empty()) {
        std::vector<std::string> blend_labels;
        std::vector<double> blend_values;
        
        for (const auto& pair : blend_times) {
            blend_labels.push_back(pair.first);
            blend_values.push_back(pair.second);
        }
        
        cv::Mat blend_chart = generateComparisonChart(
            blend_labels, blend_values,
            "Blending Method Performance",
            "Processing Time (ms)"
        );
        saveVisualization(blend_chart, output_dir + "/blending_comparison.jpg");
    }
    
    std::cout << "Visualizations saved to " << output_dir << "/\n";
}