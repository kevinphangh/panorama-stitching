#ifndef MATCHER_H
#define MATCHER_H

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <chrono>

struct MatchingResult {
    std::vector<cv::DMatch> good_matches;
    std::vector<double> match_distances;
    double matching_time_ms;
    double filtering_time_ms;
    double ratio_test_threshold;
    int num_initial_matches;
    int num_good_matches;
};

class FeatureMatcher {
public:
    FeatureMatcher();
    
    MatchingResult matchFeatures(
        const cv::Mat& descriptors1, 
        const cv::Mat& descriptors2,
        const std::vector<cv::KeyPoint>& keypoints1,
        const std::vector<cv::KeyPoint>& keypoints2,
        double ratio_threshold = 0.7
    );
    
    void setMatcherType(const std::string& type);
    
    cv::Mat visualizeMatches(
        const cv::Mat& img1, 
        const cv::Mat& img2,
        const std::vector<cv::KeyPoint>& keypoints1,
        const std::vector<cv::KeyPoint>& keypoints2,
        const std::vector<cv::DMatch>& matches
    );
    
private:
    cv::Ptr<cv::DescriptorMatcher> matcher_;
    bool cross_check_ = false;
    std::string matcher_type_ = "BruteForce-Hamming";
    
    std::vector<cv::DMatch> ratioTest(
        const std::vector<std::vector<cv::DMatch>>& knn_matches,
        double ratio_threshold
    );
};

#endif