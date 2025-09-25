#ifndef SIFT_DETECTOR_H
#define SIFT_DETECTOR_H

#include "feature_detector.h"
#include <opencv2/features2d.hpp>

class SIFTDetector : public FeatureDetector {
public:
    SIFTDetector();
    DetectionResult detect(const cv::Mat& image) override;
    std::string getName() const override { return "SIFT"; }
    void setMaxFeatures(int max_features) override;

private:
    cv::Ptr<cv::SIFT> detector_;
    void createDetector();
};

#endif