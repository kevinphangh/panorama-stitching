#ifndef BLENDER_H
#define BLENDER_H

#include <opencv2/core.hpp>
#include <vector>
#include <string>

enum class BlendMode {
    SIMPLE_OVERLAY,
    FEATHERING,
    MULTIBAND,
    EXPOSURE_COMPENSATION
};

class Blender {
public:
    Blender();
    
    cv::Mat blend(
        const cv::Mat& img1,
        const cv::Mat& img2,
        const cv::Mat& mask1,
        const cv::Mat& mask2
    );
    
    void setBlendMode(BlendMode mode) { blend_mode_ = mode; }
    BlendMode getBlendMode() const { return blend_mode_; }
    
    cv::Mat simpleOverlay(const cv::Mat& img1, const cv::Mat& img2,
                         const cv::Mat& mask1, const cv::Mat& mask2);
    
    cv::Mat featherBlend(const cv::Mat& img1, const cv::Mat& img2,
                        const cv::Mat& mask1, const cv::Mat& mask2,
                        int feather_radius = 30);
    
    cv::Mat multibandBlend(const cv::Mat& img1, const cv::Mat& img2,
                          const cv::Mat& mask1, const cv::Mat& mask2,
                          int num_bands = 5);
    
private:
    BlendMode blend_mode_ = BlendMode::FEATHERING;
    int feather_radius_ = 30;
    int num_bands_ = 5;
    
    std::vector<cv::Mat> createGaussianPyramid(const cv::Mat& img, int levels);
    std::vector<cv::Mat> createLaplacianPyramid(const cv::Mat& img, int levels);
    cv::Mat reconstructFromPyramid(const std::vector<cv::Mat>& pyramid);
};

#endif