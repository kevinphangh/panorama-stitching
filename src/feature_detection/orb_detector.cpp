#include "orb_detector.h"
#include <opencv2/imgproc.hpp>
#include <iostream>

ORBDetector::ORBDetector() {
    createDetector();
}

void ORBDetector::createDetector() {
    // Create ORB detector with specified max features
    // Note: ORB will detect UP TO max_features, but may find fewer if image lacks features
    detector_ = cv::ORB::create(max_features_, 1.2f, 8, 31, 0, 2, 
                               cv::ORB::HARRIS_SCORE, 31, 20);
    std::cout << "ORB detector created with max_features=" << max_features_ << std::endl;
}

void ORBDetector::setMaxFeatures(int max_features) {
    max_features_ = max_features;
    createDetector();  // Recreate detector with new max_features
}

DetectionResult ORBDetector::detect(const cv::Mat& image) {
    DetectionResult result;
    result.detector_name = "ORB";
    
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image;
    }
    
    // Detect keypoints
    result.detection_time_ms = measureTime([&]() {
        detector_->detect(gray, result.keypoints);
    });
    
    // Compute descriptors
    result.description_time_ms = measureTime([&]() {
        detector_->compute(gray, result.keypoints, result.descriptors);
    });
    
    return result;
}