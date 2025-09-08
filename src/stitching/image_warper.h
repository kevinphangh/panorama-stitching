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
    
    
private:
    int border_mode_ = cv::BORDER_CONSTANT;
    cv::Scalar border_value_ = cv::Scalar(0, 0, 0);
};

#endif