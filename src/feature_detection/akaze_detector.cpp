#include "akaze_detector.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

AKAZEDetector::AKAZEDetector() {
    createDetector();
}

void AKAZEDetector::createDetector() {
    detector_ = cv::AKAZE::create(cv::AKAZE::DESCRIPTOR_MLDB,
                                  0, 3, 0.001f, 4, 4, cv::KAZE::DIFF_PM_G2);
}

void AKAZEDetector::setMaxFeatures(int max_features) {
    if (max_features != max_features_) {
        max_features_ = max_features;
        // Note: AKAZE doesn't have a native max_features parameter like ORB.
        // We limit features post-detection by sorting by response strength.
        // This provides consistent behavior across detectors but may have 
        // different performance characteristics than ORB's built-in limiting.
    }
}

DetectionResult AKAZEDetector::detect(const cv::Mat& image) {
    DetectionResult result;
    result.detector_name = "AKAZE";
    
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image;
    }
    
    std::vector<cv::KeyPoint> all_keypoints;
    
    result.detection_time_ms = measureTime([&]() {
        detector_->detect(gray, all_keypoints);
    });
    
    if (all_keypoints.size() > static_cast<size_t>(max_features_)) {
        std::partial_sort(all_keypoints.begin(), 
                         all_keypoints.begin() + max_features_,
                         all_keypoints.end(),
                         [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                             return a.response > b.response;
                         });
        
        all_keypoints.resize(max_features_);
    }
    
    result.description_time_ms = measureTime([&]() {
        detector_->compute(gray, all_keypoints, result.descriptors);
    });
    
    result.keypoints = all_keypoints;
    
    return result;
}