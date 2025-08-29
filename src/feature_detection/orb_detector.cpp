#include "orb_detector.h"
#include <opencv2/imgproc.hpp>

ORBDetector::ORBDetector() {
    detector_ = cv::ORB::create(max_features_, 1.2f, 8, 31, 0, 2, 
                               cv::ORB::HARRIS_SCORE, 31, 20);
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