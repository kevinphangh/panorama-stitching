#ifndef HOMOGRAPHY_ESTIMATOR_H
#define HOMOGRAPHY_ESTIMATOR_H

#include <opencv2/core.hpp>
#include <vector>
#include "../feature_matching/ransac.h"

class HomographyEstimator {
public:
    HomographyEstimator();

    cv::Mat estimateHomography(
        const std::vector<cv::KeyPoint>& keypoints1,
        const std::vector<cv::KeyPoint>& keypoints2,
        const std::vector<cv::DMatch>& matches,
        std::vector<cv::DMatch>& inlier_matches
    );

    void setRANSACThreshold(double threshold) { ransac_threshold_ = threshold; }

    RANSACResult getLastResult() const { return last_result_; }

    static cv::Rect calculateOutputBounds(
        const cv::Mat& img1,
        const cv::Mat& img2,
        const cv::Mat& H
    );

private:
    RANSAC ransac_;
    double ransac_threshold_ = 3.0;
    double ransac_confidence_ = 0.995;
    RANSACResult last_result_;
};

#endif
