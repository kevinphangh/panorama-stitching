#include "blender_factory.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

std::unique_ptr<Blender> BlenderFactory::createBlender(const std::string& mode) {
    auto blend_mode = stringToMode(mode);
    auto blender = std::make_unique<Blender>();
    blender->setBlendMode(blend_mode);
    return blender;
}

std::unique_ptr<Blender> BlenderFactory::createBlender(BlendMode mode) {
    auto blender = std::make_unique<Blender>();
    blender->setBlendMode(mode);
    return blender;
}

BlendMode BlenderFactory::stringToMode(const std::string& mode) {
    std::string lower_mode = mode;
    std::transform(lower_mode.begin(), lower_mode.end(), lower_mode.begin(),
                  [](unsigned char c) { return std::tolower(c); });

    if (lower_mode == "simple") {
        return BlendMode::SIMPLE_OVERLAY;
    } else if (lower_mode == "feather") {
        return BlendMode::FEATHERING;
    } else if (lower_mode == "multiband") {
        return BlendMode::MULTIBAND;
    } else {
        throw std::invalid_argument("Unknown blend mode: " + mode);
    }
}

std::string BlenderFactory::modeToString(BlendMode mode) {
    switch (mode) {
        case BlendMode::SIMPLE_OVERLAY:
            return "simple";
        case BlendMode::FEATHERING:
            return "feather";
        case BlendMode::MULTIBAND:
            return "multiband";
        default:
            return "unknown";
    }
}