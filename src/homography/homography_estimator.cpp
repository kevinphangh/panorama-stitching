#include "homography_estimator.h"
#include "../config.h"
#include <opencv2/calib3d.hpp>
#include <cmath>
#include <iostream>
#include <limits>

HomographyEstimator::HomographyEstimator() {
    ransac_threshold_ = PanoramaConfig::DEFAULT_RANSAC_THRESHOLD;
    ransac_confidence_ = PanoramaConfig::DEFAULT_RANSAC_CONFIDENCE;
}

cv::Mat HomographyEstimator::estimateHomography(
    const std::vector<cv::KeyPoint>& keypoints1,
    const std::vector<cv::KeyPoint>& keypoints2,
    const std::vector<cv::DMatch>& matches,
    std::vector<cv::DMatch>& inlier_matches,
    double reprojection_threshold) {
    
    if (matches.size() < 4) {
        return cv::Mat();
    }
    
    // Extract points ensuring both arrays stay synchronized
    std::vector<cv::Point2f> points1, points2;
    points1.reserve(matches.size());
    points2.reserve(matches.size());
    
    for (const auto& match : matches) {
        if (match.queryIdx >= 0 && match.queryIdx < static_cast<int>(keypoints1.size()) &&
            match.trainIdx >= 0 && match.trainIdx < static_cast<int>(keypoints2.size())) {
            points1.push_back(keypoints1[match.queryIdx].pt);
            points2.push_back(keypoints2[match.trainIdx].pt);
        }
    }
    
    if (points1.size() < 4) {
        std::cerr << "Not enough valid point correspondences: " << points1.size() << std::endl;
        return cv::Mat();
    }
    
    ransac_.setReprojectionThreshold(reprojection_threshold);
    last_result_ = ransac_.findHomography(points1, points2, 
                                         reprojection_threshold,
                                         ransac_confidence_);
    
    // Extract inlier matches (need to map back to original matches)
    inlier_matches.clear();
    size_t valid_idx = 0;
    for (size_t i = 0; i < matches.size(); i++) {
        if (matches[i].queryIdx >= 0 && matches[i].queryIdx < static_cast<int>(keypoints1.size()) &&
            matches[i].trainIdx >= 0 && matches[i].trainIdx < static_cast<int>(keypoints2.size())) {
            if (valid_idx < last_result_.inlier_mask.size() && last_result_.inlier_mask[valid_idx]) {
                inlier_matches.push_back(matches[i]);
            }
            valid_idx++;
        }
    }
    
    return last_result_.homography;
}


cv::Rect HomographyEstimator::calculateOutputBounds(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const cv::Mat& H) {
    
    // Transform corners of img2
    std::vector<cv::Point2f> corners2(4);
    corners2[0] = cv::Point2f(0, 0);
    corners2[1] = cv::Point2f(static_cast<float>(img2.cols), 0);
    corners2[2] = cv::Point2f(static_cast<float>(img2.cols), static_cast<float>(img2.rows));
    corners2[3] = cv::Point2f(0, static_cast<float>(img2.rows));
    
    std::vector<cv::Point2f> corners2_transformed;
    cv::perspectiveTransform(corners2, corners2_transformed, H);
    
    // Combine with img1 corners
    std::vector<cv::Point2f> all_corners;
    all_corners.push_back(cv::Point2f(0, 0));
    all_corners.push_back(cv::Point2f(static_cast<float>(img1.cols), 0));
    all_corners.push_back(cv::Point2f(static_cast<float>(img1.cols), static_cast<float>(img1.rows)));
    all_corners.push_back(cv::Point2f(0, static_cast<float>(img1.rows)));
    all_corners.insert(all_corners.end(), corners2_transformed.begin(), corners2_transformed.end());
    
    // Find bounding box
    float min_x = 0;  // Start from 0 to ensure first image is included
    float max_x = static_cast<float>(img1.cols);
    float min_y = 0;
    float max_y = static_cast<float>(img1.rows);
    
    for (const auto& pt : corners2_transformed) {
        min_x = std::min(min_x, pt.x);
        max_x = std::max(max_x, pt.x);
        min_y = std::min(min_y, pt.y);
        max_y = std::max(max_y, pt.y);
    }
    
    // Add padding and ensure positive dimensions
    int padding = PanoramaConfig::PANORAMA_PADDING;
    int width = static_cast<int>(std::ceil(max_x - min_x)) + padding * 2;
    int height = static_cast<int>(std::ceil(max_y - min_y)) + padding * 2;
    
    // Limit maximum size (using same limit as main.cpp for consistency)
    width = std::min(width, PanoramaConfig::MAX_PANORAMA_DIMENSION);
    height = std::min(height, PanoramaConfig::MAX_PANORAMA_DIMENSION);
    
    return cv::Rect(0, 0, width, height);
}