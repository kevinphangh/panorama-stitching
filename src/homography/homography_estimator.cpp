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
    
    cv::Mat inlier_mask;
    cv::Mat homography = cv::findHomography(points1, points2, cv::RANSAC,
                                           reprojection_threshold, inlier_mask);

    int inlier_count = inlier_mask.empty() ? 0 : cv::countNonZero(inlier_mask);
    if (homography.empty() || inlier_count < PanoramaConfig::MIN_INLIERS_REQUIRED) {
        std::cout << "RANSAC found only " << inlier_count << " inliers, trying LMEDS...\n";
        homography = cv::findHomography(points1, points2, cv::LMEDS,
                                       reprojection_threshold, inlier_mask);
        inlier_count = inlier_mask.empty() ? 0 : cv::countNonZero(inlier_mask);
    }

    inlier_matches.clear();
    for (size_t i = 0; i < matches.size() && i < static_cast<size_t>(inlier_mask.rows); i++) {
        if (inlier_mask.at<uchar>(i)) {
            inlier_matches.push_back(matches[i]);
        }
    }

    std::cout << "RANSAC found " << inlier_matches.size() << " inliers ("
              << (100.0 * inlier_matches.size() / matches.size()) << "%)\n";

    last_result_.homography = homography;
    last_result_.num_inliers = inlier_matches.size();
    last_result_.inlier_ratio = static_cast<double>(inlier_matches.size()) / matches.size();

    return homography;
}


cv::Rect HomographyEstimator::calculateOutputBounds(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const cv::Mat& H) {
    
    std::vector<cv::Point2f> corners2(4);
    corners2[0] = cv::Point2f(0, 0);
    corners2[1] = cv::Point2f(static_cast<float>(img2.cols), 0);
    corners2[2] = cv::Point2f(static_cast<float>(img2.cols), static_cast<float>(img2.rows));
    corners2[3] = cv::Point2f(0, static_cast<float>(img2.rows));
    
    std::vector<cv::Point2f> corners2_transformed;
    cv::perspectiveTransform(corners2, corners2_transformed, H);
    
    std::vector<cv::Point2f> all_corners;
    all_corners.push_back(cv::Point2f(0, 0));
    all_corners.push_back(cv::Point2f(static_cast<float>(img1.cols), 0));
    all_corners.push_back(cv::Point2f(static_cast<float>(img1.cols), static_cast<float>(img1.rows)));
    all_corners.push_back(cv::Point2f(0, static_cast<float>(img1.rows)));
    all_corners.insert(all_corners.end(), corners2_transformed.begin(), corners2_transformed.end());
    
    float min_x = 0;
    float max_x = static_cast<float>(img1.cols);
    float min_y = 0;
    float max_y = static_cast<float>(img1.rows);
    
    for (const auto& pt : corners2_transformed) {
        min_x = std::min(min_x, pt.x);
        max_x = std::max(max_x, pt.x);
        min_y = std::min(min_y, pt.y);
        max_y = std::max(max_y, pt.y);
    }
    
    int padding = PanoramaConfig::PANORAMA_PADDING;
    int width = static_cast<int>(std::ceil(max_x - min_x)) + padding * 2;
    int height = static_cast<int>(std::ceil(max_y - min_y)) + padding * 2;
    
    width = std::min(width, PanoramaConfig::MAX_PANORAMA_DIMENSION);
    height = std::min(height, PanoramaConfig::MAX_PANORAMA_DIMENSION);
    
    return cv::Rect(0, 0, width, height);
}