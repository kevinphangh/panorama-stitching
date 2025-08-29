#ifndef RANSAC_H
#define RANSAC_H

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <vector>

struct RANSACResult {
    cv::Mat homography;
    std::vector<bool> inlier_mask;
    int num_inliers;
    double inlier_ratio;
    int num_iterations;
    double reprojection_error;
    double computation_time_ms;
};

class RANSAC {
public:
    RANSAC();
    
    RANSACResult findHomography(
        const std::vector<cv::Point2f>& points1,
        const std::vector<cv::Point2f>& points2,
        double reprojection_threshold = 3.0,
        double confidence = 0.995,
        int max_iterations = 2000
    );
    
    void setMethod(int method) { method_ = method; }
    void setReprojectionThreshold(double threshold) { reprojection_threshold_ = threshold; }
    
    double computeReprojectionError(
        const cv::Mat& homography,
        const std::vector<cv::Point2f>& points1,
        const std::vector<cv::Point2f>& points2,
        const std::vector<bool>& inlier_mask
    );
    
    static std::vector<cv::Point2f> extractPoints(
        const std::vector<cv::KeyPoint>& keypoints,
        const std::vector<cv::DMatch>& matches,
        bool query_points
    );
    
private:
    int method_ = cv::RANSAC;
    double reprojection_threshold_ = 3.0;
    double confidence_ = 0.995;
    int max_iterations_ = 2000;
    
    cv::Mat computeHomographyDLT(const std::vector<cv::Point2f>& pts1, 
                                 const std::vector<cv::Point2f>& pts2);
    
    std::vector<bool> findInliers(const cv::Mat& H,
                                  const std::vector<cv::Point2f>& pts1,
                                  const std::vector<cv::Point2f>& pts2,
                                  double threshold);
};

#endif