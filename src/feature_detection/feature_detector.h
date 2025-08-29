#ifndef FEATURE_DETECTOR_H
#define FEATURE_DETECTOR_H

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <string>
#include <chrono>

struct DetectionResult {
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    double detection_time_ms;
    double description_time_ms;
    std::string detector_name;
    
    size_t getKeypointCount() const { return keypoints.size(); }
};

class FeatureDetector {
public:
    FeatureDetector() = default;
    virtual ~FeatureDetector() = default;
    
    virtual DetectionResult detect(const cv::Mat& image) = 0;
    virtual std::string getName() const = 0;
    
    virtual void setMaxFeatures(int max_features) { max_features_ = max_features; }
    int getMaxFeatures() const { return max_features_; }
    
    
protected:
    int max_features_ = 2000;
    
    template<typename Func>
    double measureTime(Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

#endif