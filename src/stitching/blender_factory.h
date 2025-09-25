#ifndef BLENDER_FACTORY_H
#define BLENDER_FACTORY_H

#include <memory>
#include <string>
#include "blender.h"

class BlenderFactory {
public:
    static std::unique_ptr<Blender> createBlender(const std::string& mode);
    static std::unique_ptr<Blender> createBlender(BlendMode mode);

    static BlendMode stringToMode(const std::string& mode);
    static std::string modeToString(BlendMode mode);

private:
    BlenderFactory() = delete;
};

#endif