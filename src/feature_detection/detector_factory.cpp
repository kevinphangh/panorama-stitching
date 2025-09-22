#include "detector_factory.h"
#include "orb_detector.h"
#include "akaze_detector.h"
#include "sift_detector.h"
#include <algorithm>
#include <cctype>

std::unique_ptr<FeatureDetector> DetectorFactory::createDetector(const std::string& type) {
    return createDetector(stringToType(type));
}

std::unique_ptr<FeatureDetector> DetectorFactory::createDetector(DetectorType type) {
    switch (type) {
        case DetectorType::ORB:
            return std::make_unique<ORBDetector>();
        case DetectorType::AKAZE:
            return std::make_unique<AKAZEDetector>();
        case DetectorType::SIFT:
            return std::make_unique<SIFTDetector>();
        default:
            throw std::invalid_argument("Unknown detector type");
    }
}

DetectorFactory::DetectorType DetectorFactory::stringToType(const std::string& type) {
    std::string lower_type = type;
    std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(),
                  [](unsigned char c) { return std::tolower(c); });

    if (lower_type == "orb") {
        return DetectorType::ORB;
    } else if (lower_type == "akaze") {
        return DetectorType::AKAZE;
    } else if (lower_type == "sift") {
        return DetectorType::SIFT;
    } else {
        throw std::invalid_argument("Unknown detector type: " + type);
    }
}

std::string DetectorFactory::typeToString(DetectorType type) {
    switch (type) {
        case DetectorType::ORB:
            return "orb";
        case DetectorType::AKAZE:
            return "akaze";
        case DetectorType::SIFT:
            return "sift";
        default:
            return "unknown";
    }
}