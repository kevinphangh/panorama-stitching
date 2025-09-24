#include "akaze_detector.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <iostream>

AKAZEDetector::AKAZEDetector() {
    createDetector();
}

void AKAZEDetector::createDetector() {
    detector_ = cv::AKAZE::create(cv::AKAZE::DESCRIPTOR_MLDB,
                                  0, 3, current_threshold_, 4, 4, cv::KAZE::DIFF_PM_G2);
}

void AKAZEDetector::resetDetector() {
    current_threshold_ = base_threshold_;
    createDetector();
}

void AKAZEDetector::setMaxFeatures(int max_features) {
    if (max_features != max_features_) {
        FeatureDetector::setMaxFeatures(max_features);
        resetDetector();
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
    
    resetDetector();

    std::vector<cv::KeyPoint> best_keypoints;
    double total_detection_time_ms = 0.0;
    bool threshold_adjusted = false;
    float used_threshold = current_threshold_;
    const size_t target_features = max_features_ > 0 ? static_cast<size_t>(max_features_) : 0;

    for (int iter = 0; iter <= MAX_ADAPTIVE_STEPS; ++iter) {
        std::vector<cv::KeyPoint> iteration_keypoints;
        total_detection_time_ms += measureTime([&]() {
            detector_->detect(gray, iteration_keypoints);
        });

        best_keypoints = std::move(iteration_keypoints);

        if (target_features == 0 ||
            best_keypoints.size() >= target_features ||
            current_threshold_ <= MIN_THRESHOLD ||
            iter == MAX_ADAPTIVE_STEPS) {
            used_threshold = current_threshold_;
            break;
        }

        threshold_adjusted = true;
        current_threshold_ = std::max(current_threshold_ * THRESHOLD_DECAY, MIN_THRESHOLD);
        createDetector();
    }

    result.detection_time_ms = total_detection_time_ms;

    if (threshold_adjusted && used_threshold != base_threshold_) {
        std::cout << "AKAZE adjusted threshold to " << used_threshold
                  << " (target " << max_features_
                  << ", found " << best_keypoints.size() << ")" << std::endl;
    }

    if (target_features > 0 && best_keypoints.size() > target_features) {
        auto cutoff = best_keypoints.begin() + static_cast<std::ptrdiff_t>(max_features_);
        std::partial_sort(best_keypoints.begin(),
                         cutoff,
                         best_keypoints.end(),
                         [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                             return a.response > b.response;
                         });
        best_keypoints.resize(max_features_);
    }

    result.description_time_ms = measureTime([&]() {
        detector_->compute(gray, best_keypoints, result.descriptors);
    });

    result.keypoints = best_keypoints;

    resetDetector();
    
    return result;
}
