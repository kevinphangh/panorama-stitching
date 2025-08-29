#ifndef AKAZE_DETECTOR_H
#define AKAZE_DETECTOR_H

#include "feature_detector.h"
#include <opencv2/features2d.hpp>

class AKAZEDetector : public FeatureDetector {
public:
    AKAZEDetector();
    DetectionResult detect(const cv::Mat& image) override;
    std::string getName() const override { return "AKAZE"; }
    
private:
    cv::Ptr<cv::AKAZE> detector_;
};

#endif // AKAZE_DETECTOR_H