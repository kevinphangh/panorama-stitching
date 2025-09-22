#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <string>

class Visualization {
public:
    static cv::Mat generateMatchDistanceHistogram(
        const std::vector<double>& distances,
        const std::string& title = "Match Distance Distribution"
    );
    
    static cv::Mat plotMetrics(
        const std::vector<double>& x_values,
        const std::vector<double>& y_values,
        const std::string& x_label,
        const std::string& y_label,
        const std::string& title
    );
    
    static cv::Mat generateComparisonChart(
        const std::vector<std::string>& labels,
        const std::vector<double>& values,
        const std::string& title,
        const std::string& y_label
    );
    
    static bool saveVisualization(
        const cv::Mat& visualization,
        const std::string& filepath
    );
    
    static void generateExperimentReport(
        const std::string& csv_path,
        const std::string& output_dir
    );
    
private:
    static cv::Mat drawHistogram(
        const std::vector<int>& histogram,
        int hist_w,
        int hist_h,
        int bin_count,
        double max_value,
        const std::string& title
    );
    
    static cv::Mat drawPlot(
        const std::vector<cv::Point2f>& points,
        int plot_w,
        int plot_h,
        const std::string& x_label,
        const std::string& y_label,
        const std::string& title
    );
};

#endif