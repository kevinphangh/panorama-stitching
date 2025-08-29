#ifndef IMAGE_WARPER_H
#define IMAGE_WARPER_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class ImageWarper {
public:
    ImageWarper();
    
    cv::Mat warpPerspective(
        const cv::Mat& image,
        const cv::Mat& homography,
        const cv::Size& output_size,
        int interpolation = cv::INTER_LINEAR
    );
    
    cv::Mat warpWithMask(
        const cv::Mat& image,
        const cv::Mat& homography,
        const cv::Size& output_size,
        cv::Mat& mask
    );
    
    std::pair<cv::Point, cv::Size> computeWarpedImageBounds(
        const cv::Mat& image,
        const cv::Mat& homography
    );
    
    void setInterpolationMethod(int method) { interpolation_method_ = method; }
    void setBorderMode(int mode) { border_mode_ = mode; }
    
private:
    int interpolation_method_ = cv::INTER_LINEAR;
    int border_mode_ = cv::BORDER_CONSTANT;
    cv::Scalar border_value_ = cv::Scalar(0, 0, 0);
    
    cv::Point2f transformPoint(const cv::Point2f& pt, const cv::Mat& H);
};

#endif