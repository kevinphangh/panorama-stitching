#include "matcher.h"
#include "ransac.h"
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <iostream>

FeatureMatcher::FeatureMatcher() {
    setMatcherType("BruteForce-Hamming");
}

void FeatureMatcher::setMatcherType(const std::string& type) {
    matcher_type_ = type;
    if (type == "BruteForce-Hamming") {
        matcher_ = cv::BFMatcher::create(cv::NORM_HAMMING, cross_check_);
    } else if (type == "BruteForce-L2") {
        matcher_ = cv::BFMatcher::create(cv::NORM_L2, cross_check_);
    } else if (type == "FlannBased") {
        matcher_ = cv::FlannBasedMatcher::create();
    }
}

MatchingResult FeatureMatcher::matchFeatures(
    const cv::Mat& descriptors1, 
    const cv::Mat& descriptors2,
    const std::vector<cv::KeyPoint>& keypoints1,
    const std::vector<cv::KeyPoint>& keypoints2,
    double ratio_threshold) {
    
    MatchingResult result;
    result.ratio_test_threshold = ratio_threshold;

    if (descriptors1.empty() || descriptors2.empty()) {
        std::cerr << "Error: Empty descriptors provided to matcher\n";
        return result;
    }
    
    if (descriptors1.cols != descriptors2.cols) {
        std::cerr << "Error: Descriptor dimensions don't match\n";
        return result;
    }
    
    if (keypoints1.size() != static_cast<size_t>(descriptors1.rows) ||
        keypoints2.size() != static_cast<size_t>(descriptors2.rows)) {
        std::cerr << "Error: Keypoint and descriptor counts don't match\n";
        return result;
    }
    
    std::vector<std::vector<cv::DMatch>> knn_matches;
    
    auto start = std::chrono::high_resolution_clock::now();
    matcher_->knnMatch(descriptors1, descriptors2, knn_matches, 2);
    auto end = std::chrono::high_resolution_clock::now();
    result.matching_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    result.num_initial_matches = knn_matches.size();
    
    start = std::chrono::high_resolution_clock::now();
    result.good_matches = ratioTest(knn_matches, ratio_threshold);
    end = std::chrono::high_resolution_clock::now();
    result.filtering_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    result.num_good_matches = result.good_matches.size();
    
    // Store match distances for histogram generation
    result.match_distances.clear();
    for (const auto& match : result.good_matches) {
        result.match_distances.push_back(match.distance);
    }
    
    return result;
}

std::vector<cv::DMatch> FeatureMatcher::ratioTest(
    const std::vector<std::vector<cv::DMatch>>& knn_matches,
    double ratio_threshold) {
    
    std::vector<cv::DMatch> good_matches;
    
    for (const auto& match_pair : knn_matches) {
        if (match_pair.size() == 2) {
            if (match_pair[0].distance < ratio_threshold * match_pair[1].distance) {
                good_matches.push_back(match_pair[0]);
            }
        }
    }
    
    return good_matches;
}

cv::Mat FeatureMatcher::visualizeMatches(
    const cv::Mat& img1, 
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& keypoints1,
    const std::vector<cv::KeyPoint>& keypoints2,
    const std::vector<cv::DMatch>& matches) {
    
    cv::Mat match_img;
    cv::drawMatches(img1, keypoints1, img2, keypoints2, matches, match_img,
                   cv::Scalar(0, 255, 0), cv::Scalar(255, 0, 0),
                   std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    
    return match_img;
}