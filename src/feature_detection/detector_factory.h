#ifndef DETECTOR_FACTORY_H
#define DETECTOR_FACTORY_H

#include <memory>
#include <string>
#include <stdexcept>
#include "feature_detector.h"

class DetectorFactory {
public:
    enum class DetectorType {
        ORB,
        AKAZE
    };

    static std::unique_ptr<FeatureDetector> createDetector(const std::string& type);
    static std::unique_ptr<FeatureDetector> createDetector(DetectorType type);

    static DetectorType stringToType(const std::string& type);
    static std::string typeToString(DetectorType type);

private:
    DetectorFactory() = delete; // Static class, prevent instantiation
};

#endif // DETECTOR_FACTORY_H