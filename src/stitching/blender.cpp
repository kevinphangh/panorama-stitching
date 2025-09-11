#include "blender.h"
#include <opencv2/imgproc.hpp>
#include <iostream>

Blender::Blender() : blend_mode_(BlendMode::FEATHERING) {}

cv::Mat Blender::blend(const cv::Mat& img1, const cv::Mat& img2,
                      const cv::Mat& mask1, const cv::Mat& mask2) {
    switch (blend_mode_) {
        case BlendMode::SIMPLE_OVERLAY:
            return simpleOverlay(img1, img2, mask1, mask2);
        case BlendMode::FEATHERING:
            return featherBlend(img1, img2, mask1, mask2);
        case BlendMode::MULTIBAND:
            return multibandBlend(img1, img2, mask1, mask2);
        default:
            std::cerr << "Unknown blend mode, using simple overlay\n";
            return simpleOverlay(img1, img2, mask1, mask2);
    }
}

cv::Mat Blender::simpleOverlay(const cv::Mat& img1, const cv::Mat& img2,
                              [[maybe_unused]] const cv::Mat& mask1, const cv::Mat& mask2) {
    if (img1.size() != img2.size() || img1.type() != img2.type()) {
        std::cerr << "Error: Images must have same size and type for blending\n";
        return cv::Mat();
    }
    
    cv::Mat result = img1.clone();
    
    // Use OpenCV's optimized copyTo instead of pixel-by-pixel iteration
    img2.copyTo(result, mask2);
    
    return result;
}

cv::Mat Blender::featherBlend(const cv::Mat& img1, const cv::Mat& img2,
                            const cv::Mat& mask1, const cv::Mat& mask2,
                            int feather_radius) {
    if (img1.size() != img2.size() || img1.type() != img2.type()) {
        std::cerr << "Error: Images must have same size and type for blending\n";
        return cv::Mat();
    }
    
    cv::Mat result = cv::Mat::zeros(img1.size(), img1.type());
    
    cv::Mat weight1, weight2;
    
    if (feather_radius > 0) {
        cv::Mat dist1, dist2;
        cv::distanceTransform(mask1, dist1, cv::DIST_L2, 3);
        cv::distanceTransform(mask2, dist2, cv::DIST_L2, 3);
        
        // Normalize distance values to [0,1] range within feather radius
        dist1 = cv::min(dist1, static_cast<double>(feather_radius)) / feather_radius;
        dist2 = cv::min(dist2, static_cast<double>(feather_radius)) / feather_radius;
        
        dist1.convertTo(weight1, CV_32F);
        dist2.convertTo(weight2, CV_32F);
    } else {
        // Binary weights from masks (no gradient)
        mask1.convertTo(weight1, CV_32F, 1.0/255.0);
        mask2.convertTo(weight2, CV_32F, 1.0/255.0);
    }
    
    cv::Mat overlap;
    cv::bitwise_and(mask1, mask2, overlap);
    
    cv::Mat weight_sum = weight1 + weight2;
    weight_sum += (weight_sum == 0);  // Prevent division by zero
    
    cv::Mat img1_float, img2_float;
    img1.convertTo(img1_float, CV_32FC3);
    img2.convertTo(img2_float, CV_32FC3);
    
    std::vector<cv::Mat> channels1, channels2, result_channels;
    cv::split(img1_float, channels1);
    cv::split(img2_float, channels2);
    
    for (int c = 0; c < 3; c++) {
        cv::Mat weighted1 = channels1[c].mul(weight1);
        cv::Mat weighted2 = channels2[c].mul(weight2);
        cv::Mat blended = (weighted1 + weighted2) / weight_sum;
        result_channels.push_back(blended);
    }
    
    cv::Mat result_float;
    cv::merge(result_channels, result_float);
    result_float.convertTo(result, CV_8UC3);
    
    return result;
}

cv::Mat Blender::multibandBlend(const cv::Mat& img1, const cv::Mat& img2,
                               const cv::Mat& mask1, const cv::Mat& mask2,
                               int num_bands) {
    // Multiband blending using Laplacian pyramids
    // This technique blends different frequency bands separately to achieve smooth transitions
    // while preserving fine details. High frequencies (edges, details) are blended with sharp
    // transitions, while low frequencies (colors, gradients) are blended smoothly.
    //
    // Algorithm:
    // 1. Build Laplacian pyramids for both images (frequency decomposition)
    // 2. Build Gaussian pyramids for masks (smooth blending weights)
    // 3. Blend each pyramid level using corresponding mask weights
    // 4. Reconstruct the final image from the blended pyramid
    
    if (img1.size() != img2.size() || img1.type() != img2.type()) {
        std::cerr << "Error: Images must have same size and type for blending\n";
        return cv::Mat();
    }
    
    // Reduce pyramid levels for large images to avoid excessive memory usage
    size_t pixel_count = static_cast<size_t>(img1.rows) * img1.cols;
    size_t estimated_memory = pixel_count * 3 * 4 * 2 * num_bands;  // bytes: pixels * channels * float * images * levels
    
    if (estimated_memory > 1073741824) {  // 1GB limit
        num_bands = std::max(3, num_bands - 2);
        std::cout << "Reducing pyramid levels to " << num_bands << " to conserve memory\n";
    }
    
    std::vector<cv::Mat> pyramid1 = createLaplacianPyramid(img1, num_bands);
    std::vector<cv::Mat> pyramid2 = createLaplacianPyramid(img2, num_bands);
    
    std::vector<cv::Mat> mask_pyramid1 = createGaussianPyramid(mask1, num_bands);
    std::vector<cv::Mat> mask_pyramid2 = createGaussianPyramid(mask2, num_bands);
    
    std::vector<cv::Mat> blended_pyramid;
    
    for (int i = 0; i < num_bands; i++) {
        cv::Mat mask1_float, mask2_float;
        mask_pyramid1[i].convertTo(mask1_float, CV_32F, 1.0/255.0);
        mask_pyramid2[i].convertTo(mask2_float, CV_32F, 1.0/255.0);
        
        // Normalize mask weights to sum to 1.0
        cv::Mat mask_sum = mask1_float + mask2_float;
        mask_sum += (mask_sum == 0);
        mask1_float = mask1_float / mask_sum;
        mask2_float = mask2_float / mask_sum;
        
        cv::Mat blended = cv::Mat::zeros(pyramid1[i].size(), pyramid1[i].type());
        
        std::vector<cv::Mat> channels1, channels2, blended_channels;
        cv::split(pyramid1[i], channels1);
        cv::split(pyramid2[i], channels2);
        
        for (int c = 0; c < 3; c++) {
            cv::Mat channel_blend = channels1[c].mul(mask1_float) + 
                                   channels2[c].mul(mask2_float);
            blended_channels.push_back(channel_blend);
        }
        
        cv::merge(blended_channels, blended);
        blended_pyramid.push_back(blended);
    }
    
    return reconstructFromPyramid(blended_pyramid);
}

std::vector<cv::Mat> Blender::createGaussianPyramid(const cv::Mat& img, int levels) {
    std::vector<cv::Mat> pyramid;
    cv::Mat current = img.clone();
    
    pyramid.push_back(current);
    
    for (int i = 1; i < levels; i++) {
        cv::Mat down;
        cv::pyrDown(current, down);
        pyramid.push_back(down);
        current = down;
    }
    
    return pyramid;
}

std::vector<cv::Mat> Blender::createLaplacianPyramid(const cv::Mat& img, int levels) {
    std::vector<cv::Mat> laplacian_pyramid;
    cv::Mat current = img.clone();
    
    if (current.type() == CV_8UC3) {
        current.convertTo(current, CV_32FC3);
    }
    
    for (int i = 0; i < levels - 1; i++) {
        cv::Mat down, up;
        cv::pyrDown(current, down);
        cv::pyrUp(down, up, current.size());
        
        cv::Mat laplacian = current - up;
        laplacian_pyramid.push_back(laplacian);
        current = down;
    }
    
    laplacian_pyramid.push_back(current);
    
    return laplacian_pyramid;
}

cv::Mat Blender::reconstructFromPyramid(const std::vector<cv::Mat>& pyramid) {
    cv::Mat current = pyramid.back();
    
    for (int i = pyramid.size() - 2; i >= 0; i--) {
        cv::Mat up;
        cv::pyrUp(current, up, pyramid[i].size());
        current = up + pyramid[i];
    }
    
    cv::Mat result;
    current.convertTo(result, CV_8UC3);
    
    return result;
}