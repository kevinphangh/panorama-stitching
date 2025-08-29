#ifndef ORB_DETECTOR_H
#define ORB_DETECTOR_H

#include "feature_detector.h"
#include <opencv2/features2d.hpp>

class ORBDetector : public FeatureDetector {
public:
    ORBDetector();
    DetectionResult detect(const cv::Mat& image) override;
    std::string getName() const override { return "ORB"; }
    void setMaxFeatures(int max_features) override;
    
private:
    cv::Ptr<cv::ORB> detector_;
    void createDetector();
};

#endif // ORB_DETECTOR_H