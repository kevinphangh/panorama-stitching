#include "image_warper.h"

ImageWarper::ImageWarper() {
    interpolation_method_ = cv::INTER_LINEAR;
    border_mode_ = cv::BORDER_CONSTANT;
    border_value_ = cv::Scalar(0, 0, 0);
}

cv::Mat ImageWarper::warpPerspective(
    const cv::Mat& image,
    const cv::Mat& homography,
    const cv::Size& output_size,
    int interpolation) {
    
    cv::Mat warped;
    cv::warpPerspective(image, warped, homography, output_size,
                       interpolation, border_mode_, border_value_);
    
    return warped;
}

cv::Mat ImageWarper::warpWithMask(
    const cv::Mat& image,
    const cv::Mat& homography,
    const cv::Size& output_size,
    cv::Mat& mask) {
    
    // Create mask for valid pixels
    mask = cv::Mat::ones(image.size(), CV_8UC1) * 255;
    
    // Warp both image and mask
    cv::Mat warped_image = warpPerspective(image, homography, output_size);
    cv::Mat warped_mask;
    cv::warpPerspective(mask, warped_mask, homography, output_size,
                       cv::INTER_NEAREST, border_mode_, cv::Scalar(0));
    
    mask = warped_mask;
    return warped_image;
}

std::pair<cv::Point, cv::Size> ImageWarper::computeWarpedImageBounds(
    const cv::Mat& image,
    const cv::Mat& homography) {
    
    // Get image corners
    std::vector<cv::Point2f> corners(4);
    corners[0] = cv::Point2f(0, 0);
    corners[1] = cv::Point2f(image.cols, 0);
    corners[2] = cv::Point2f(image.cols, image.rows);
    corners[3] = cv::Point2f(0, image.rows);
    
    // Transform corners
    std::vector<cv::Point2f> transformed_corners;
    cv::perspectiveTransform(corners, transformed_corners, homography);
    
    // Find bounding box
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::min();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::min();
    
    for (const auto& pt : transformed_corners) {
        min_x = std::min(min_x, pt.x);
        max_x = std::max(max_x, pt.x);
        min_y = std::min(min_y, pt.y);
        max_y = std::max(max_y, pt.y);
    }
    
    cv::Point offset(static_cast<int>(min_x), static_cast<int>(min_y));
    cv::Size size(static_cast<int>(max_x - min_x), static_cast<int>(max_y - min_y));
    
    return {offset, size};
}


cv::Point2f ImageWarper::transformPoint(const cv::Point2f& pt, const cv::Mat& H) {
    cv::Mat pt_h = (cv::Mat_<double>(3, 1) << pt.x, pt.y, 1.0);
    cv::Mat pt_transformed = H * pt_h;
    
    double w = pt_transformed.at<double>(2);
    return cv::Point2f(
        pt_transformed.at<double>(0) / w,
        pt_transformed.at<double>(1) / w
    );
}