#include "image_warper.h"

ImageWarper::ImageWarper() {
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