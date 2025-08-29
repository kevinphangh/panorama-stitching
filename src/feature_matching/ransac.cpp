#include "ransac.h"
#include <random>
#include <chrono>
#include <iostream>

RANSAC::RANSAC() {
    reprojection_threshold_ = 3.0;
    confidence_ = 0.995;
    max_iterations_ = 2000;
}

RANSACResult RANSAC::findHomography(
    const std::vector<cv::Point2f>& points1,
    const std::vector<cv::Point2f>& points2,
    double reprojection_threshold,
    double confidence,
    int max_iterations) {
    
    RANSACResult result;
    
    if (points1.size() < 4 || points1.size() != points2.size()) {
        return result;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    reprojection_threshold_ = reprojection_threshold;
    confidence_ = confidence;
    max_iterations_ = max_iterations;
    
    int n_points = points1.size();
    int best_inliers = 0;
    cv::Mat best_H;
    std::vector<bool> best_mask(n_points, false);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, n_points - 1);
    
    int iterations = 0;
    double p = confidence_;
    double w = 0.0;
    
    while (iterations < max_iterations_) {
        std::vector<int> sample;
        while (sample.size() < 4) {
            int idx = dis(gen);
            if (std::find(sample.begin(), sample.end(), idx) == sample.end()) {
                sample.push_back(idx);
            }
        }
        
        std::vector<cv::Point2f> pts1_sample, pts2_sample;
        for (int idx : sample) {
            pts1_sample.push_back(points1[idx]);
            pts2_sample.push_back(points2[idx]);
        }
        
        cv::Mat H = computeHomographyDLT(pts1_sample, pts2_sample);
        
        if (H.empty()) continue;
        
        std::vector<bool> inlier_mask = findInliers(H, points1, points2, reprojection_threshold);
        int n_inliers = std::count(inlier_mask.begin(), inlier_mask.end(), true);
        
        if (n_inliers > best_inliers) {
            best_inliers = n_inliers;
            best_H = H.clone();
            best_mask = inlier_mask;
            
            w = static_cast<double>(best_inliers) / n_points;
            if (w > 0.0 && w < 1.0) {
                double new_iterations = std::log(1 - p) / std::log(1 - std::pow(w, 4));
                max_iterations = std::min(static_cast<int>(new_iterations) + 1, max_iterations_);
            }
        }
        
        iterations++;
    }
    
    if (best_inliers >= 4) {
        std::vector<cv::Point2f> inlier_pts1, inlier_pts2;
        for (size_t i = 0; i < points1.size(); i++) {
            if (best_mask[i]) {
                inlier_pts1.push_back(points1[i]);
                inlier_pts2.push_back(points2[i]);
            }
        }
        
        cv::Mat refined_H = cv::findHomography(inlier_pts1, inlier_pts2, 0);
        if (!refined_H.empty()) {
            best_H = refined_H;
            best_mask = findInliers(best_H, points1, points2, reprojection_threshold);
            best_inliers = std::count(best_mask.begin(), best_mask.end(), true);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    result.homography = best_H;
    result.inlier_mask = best_mask;
    result.num_inliers = best_inliers;
    result.inlier_ratio = static_cast<double>(best_inliers) / n_points;
    result.num_iterations = iterations;
    result.reprojection_error = computeReprojectionError(best_H, points1, points2, best_mask);
    result.computation_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

cv::Mat RANSAC::computeHomographyDLT(const std::vector<cv::Point2f>& pts1,
                                    const std::vector<cv::Point2f>& pts2) {
    if (pts1.size() != 4 || pts2.size() != 4) {
        return cv::Mat();
    }
    
    cv::Mat A(8, 9, CV_64F);
    
    for (int i = 0; i < 4; i++) {
        double x1 = pts1[i].x;
        double y1 = pts1[i].y;
        double x2 = pts2[i].x;
        double y2 = pts2[i].y;
        
        A.at<double>(2*i, 0) = x1;
        A.at<double>(2*i, 1) = y1;
        A.at<double>(2*i, 2) = 1;
        A.at<double>(2*i, 3) = 0;
        A.at<double>(2*i, 4) = 0;
        A.at<double>(2*i, 5) = 0;
        A.at<double>(2*i, 6) = -x2*x1;
        A.at<double>(2*i, 7) = -x2*y1;
        A.at<double>(2*i, 8) = -x2;
        
        A.at<double>(2*i+1, 0) = 0;
        A.at<double>(2*i+1, 1) = 0;
        A.at<double>(2*i+1, 2) = 0;
        A.at<double>(2*i+1, 3) = x1;
        A.at<double>(2*i+1, 4) = y1;
        A.at<double>(2*i+1, 5) = 1;
        A.at<double>(2*i+1, 6) = -y2*x1;
        A.at<double>(2*i+1, 7) = -y2*y1;
        A.at<double>(2*i+1, 8) = -y2;
    }
    
    cv::Mat u, w, vt;
    cv::SVD::compute(A, w, u, vt, cv::SVD::FULL_UV);
    
    // Check if SVD succeeded 
    if (vt.cols != 9) {
        std::cerr << "SVD failed: vt has " << vt.cols << " columns (expected 9)" << std::endl;
        return cv::Mat();
    }
    
    // The solution is the last row of vt (corresponding to the smallest singular value)
    // With FULL_UV, vt should be 9x9, so we want row 8
    cv::Mat H = vt.row(8).reshape(0, 3);
    
    // Check for degenerate homography
    double det = H.at<double>(2, 2);
    if (std::abs(det) < 1e-10) {
        std::cerr << "Degenerate homography detected" << std::endl;
        return cv::Mat();
    }
    
    H = H / det;
    
    return H;
}

std::vector<bool> RANSAC::findInliers(const cv::Mat& H,
                                     const std::vector<cv::Point2f>& pts1,
                                     const std::vector<cv::Point2f>& pts2,
                                     double threshold) {
    std::vector<bool> mask(pts1.size(), false);
    
    for (size_t i = 0; i < pts1.size(); i++) {
        cv::Mat pt1 = (cv::Mat_<double>(3, 1) << pts1[i].x, pts1[i].y, 1.0);
        cv::Mat pt2_est = H * pt1;
        
        double x = pt2_est.at<double>(0) / pt2_est.at<double>(2);
        double y = pt2_est.at<double>(1) / pt2_est.at<double>(2);
        
        double dx = x - pts2[i].x;
        double dy = y - pts2[i].y;
        double error = std::sqrt(dx*dx + dy*dy);
        
        if (error < threshold) {
            mask[i] = true;
        }
    }
    
    return mask;
}

double RANSAC::computeReprojectionError(
    const cv::Mat& homography,
    const std::vector<cv::Point2f>& points1,
    const std::vector<cv::Point2f>& points2,
    const std::vector<bool>& inlier_mask) {
    
    if (homography.empty()) return -1.0;
    
    double total_error = 0.0;
    int count = 0;
    
    for (size_t i = 0; i < points1.size(); i++) {
        if (!inlier_mask[i]) continue;
        
        cv::Mat pt1 = (cv::Mat_<double>(3, 1) << points1[i].x, points1[i].y, 1.0);
        cv::Mat pt2_est = homography * pt1;
        
        double x = pt2_est.at<double>(0) / pt2_est.at<double>(2);
        double y = pt2_est.at<double>(1) / pt2_est.at<double>(2);
        
        double dx = x - points2[i].x;
        double dy = y - points2[i].y;
        double error = std::sqrt(dx*dx + dy*dy);
        
        total_error += error;
        count++;
    }
    
    return count > 0 ? total_error / count : -1.0;
}

std::vector<cv::Point2f> RANSAC::extractPoints(
    const std::vector<cv::KeyPoint>& keypoints,
    const std::vector<cv::DMatch>& matches,
    bool query_points) {
    
    std::vector<cv::Point2f> points;
    points.reserve(matches.size());
    
    for (const auto& match : matches) {
        int idx = query_points ? match.queryIdx : match.trainIdx;
        if (idx >= 0 && idx < static_cast<int>(keypoints.size())) {
            points.push_back(keypoints[idx].pt);
        } else {
            std::cerr << "Warning: Invalid keypoint index " << idx 
                      << " (keypoints size: " << keypoints.size() << ")" << std::endl;
        }
    }
    
    return points;
}