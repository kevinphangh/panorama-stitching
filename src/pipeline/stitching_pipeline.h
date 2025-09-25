#ifndef STITCHING_PIPELINE_H
#define STITCHING_PIPELINE_H

#include <opencv2/core.hpp>
#include <string>
#include <vector>

class StitchingPipeline {
public:
    static cv::Mat performStitching(
        const std::string& img1_path,
        const std::string& img2_path,
        const std::string& detector_type = "orb",
        const std::string& blend_mode = "feather",
        double ransac_threshold = 3.0,
        int max_features = 20000,
        bool visualize = false
    );

    static cv::Mat performStitchingDirect(
        const cv::Mat& img1,
        const cv::Mat& img2,
        const std::string& detector_type,
        const std::string& blend_mode,
        double ransac_threshold,
        int max_features,
        bool visualize,
        int max_panorama_dimension
    );

    static cv::Mat performSequentialStitching(
        const std::vector<cv::Mat>& images,
        const std::string& detector_type,
        const std::string& blend_mode,
        double ransac_threshold,
        int max_features,
        bool visualize
    );

    static int calculateAdaptiveFeatures(int image_pixels, int max_features);

private:
    static bool validatePanoramaSize(int width, int height);
    static cv::Mat createEmptyPanorama();
};

#endif // STITCHING_PIPELINE_H