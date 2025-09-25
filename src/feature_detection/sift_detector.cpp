#include "sift_detector.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <algorithm>

SIFTDetector::SIFTDetector() {
    max_features_ = 20000;
    createDetector();
}

void SIFTDetector::createDetector() {
    detector_ = cv::SIFT::create(
        max_features_,        // nfeatures
        3,                    // nOctaveLayers
        0.04,                 // contrastThreshold
        10,                   // edgeThreshold
        1.6                   // sigma
    );
}

void SIFTDetector::setMaxFeatures(int max_features) {
    if (max_features != max_features_) {
        max_features_ = max_features;
        createDetector();
    }
}

DetectionResult SIFTDetector::detect(const cv::Mat& image) {
    DetectionResult result;
    result.detector_name = "SIFT";

    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image;
    }

    result.detection_time_ms = measureTime([&]() {
        detector_->detect(gray, result.keypoints);
    });

    result.description_time_ms = measureTime([&]() {
        detector_->compute(gray, result.keypoints, result.descriptors);
    });

    return result;
}