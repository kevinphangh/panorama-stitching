#ifndef AKAZE_DETECTOR_H
#define AKAZE_DETECTOR_H

#include "feature_detector.h"
#include <opencv2/features2d.hpp>

class AKAZEDetector : public FeatureDetector {
public:
    AKAZEDetector();
    DetectionResult detect(const cv::Mat& image) override;
    std::string getName() const override { return "AKAZE"; }
    void setMaxFeatures(int max_features) override;
    
private:
    cv::Ptr<cv::AKAZE> detector_;
    float base_threshold_ = 0.001f;
    float current_threshold_ = base_threshold_;
    static constexpr float MIN_THRESHOLD = 2.5e-4f;
    static constexpr float THRESHOLD_DECAY = 0.6f;
    static constexpr int MAX_ADAPTIVE_STEPS = 3;
    void createDetector();
    void resetDetector();
};

#endif // AKAZE_DETECTOR_H
