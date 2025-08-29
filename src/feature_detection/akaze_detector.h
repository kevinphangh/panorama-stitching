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
    void createDetector();
};

#endif // AKAZE_DETECTOR_H