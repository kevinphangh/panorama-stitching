#include "argument_parser.h"
#include "../config.h"
#include <iostream>
#include <filesystem>

void ArgumentParser::printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --stitch <img1> <img2>       : Stitch two images\n"
              << "  --stitch-multiple <img1> ...  : Stitch multiple images\n"
              << "  --experiment-mode            : Run all experiments\n"
              << "  --detector <orb|akaze|sift>  : Choose feature detector (default: orb)\n"
              << "  --blend-mode <mode>          : Choose blend mode (simple|feather|multiband)\n"
              << "  --ransac-threshold <value>   : Set RANSAC threshold (default: 3.0)\n"
              << "  --max-features <num>         : Set max features (default: 2000)\n"
              << "  --output <path>              : Output path for panorama\n"
              << "  --visualize                  : Show intermediate results\n"
              << "  --help                       : Show this message\n";
}

bool ArgumentParser::isValidOutputPath(const std::string& path) {
    if (path.find("..") != std::string::npos) {
        std::cerr << "Error: Path traversal detected in output path\n";
        return false;
    }

    if (path[0] == '/' &&
        (path.find("/etc") == 0 ||
         path.find("/usr") == 0 ||
         path.find("/bin") == 0 ||
         path.find("/sbin") == 0 ||
         path.find("/boot") == 0 ||
         path.find("/sys") == 0 ||
         path.find("/proc") == 0)) {
        std::cerr << "Error: Cannot write to system directories\n";
        return false;
    }

    return true;
}

bool ArgumentParser::parseDouble(const std::string& arg, double& value, const std::string& param_name,
                                 double min_val, double max_val) {
    if (!parseArgument(arg, value, param_name)) {
        return false;
    }

    if (value < min_val || value > max_val) {
        std::cerr << "Error: " << param_name << " value " << value
                  << " out of range [" << min_val << ", " << max_val << "]\n";
        return false;
    }

    return true;
}

bool ArgumentParser::parseInt(const std::string& arg, int& value, const std::string& param_name,
                              int min_val, int max_val) {
    if (!parseArgument(arg, value, param_name)) {
        return false;
    }

    if (value < min_val || value > max_val) {
        std::cerr << "Error: " << param_name << " value " << value
                  << " out of range [" << min_val << ", " << max_val << "]\n";
        return false;
    }

    return true;
}

ProgramArguments ArgumentParser::parse(int argc, char** argv) {
    ProgramArguments args;

    if (argc < 2) {
        args.show_help = true;
        return args;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            args.show_help = true;
            return args;
        }
        else if (arg == "--stitch") {
            if (i + 2 >= argc) {
                std::cerr << "Error: --stitch requires two image paths\n";
                args.show_help = true;
                return args;
            }
            args.mode = ProgramArguments::STITCH_TWO;
            args.image_paths.push_back(argv[++i]);
            args.image_paths.push_back(argv[++i]);
        }
        else if (arg == "--stitch-multiple") {
            args.mode = ProgramArguments::STITCH_MULTIPLE;
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                args.image_paths.push_back(argv[++i]);
            }
            if (args.image_paths.size() < 2) {
                std::cerr << "Error: --stitch-multiple requires at least two images\n";
                args.show_help = true;
                return args;
            }
        }
        else if (arg == "--experiment-mode") {
            args.mode = ProgramArguments::EXPERIMENT;
        }
        else if (arg == "--detector") {
            if (++i >= argc) {
                std::cerr << "Error: --detector requires a value\n";
                args.show_help = true;
                return args;
            }
            args.detector_type = argv[i];
            if (args.detector_type != "orb" && args.detector_type != "akaze" && args.detector_type != "sift") {
                std::cerr << "Error: Unknown detector type: " << args.detector_type << "\n";
                args.show_help = true;
                return args;
            }
        }
        else if (arg == "--blend-mode") {
            if (++i >= argc) {
                std::cerr << "Error: --blend-mode requires a value\n";
                args.show_help = true;
                return args;
            }
            args.blend_mode = argv[i];
            if (args.blend_mode != "simple" && args.blend_mode != "feather" && args.blend_mode != "multiband") {
                std::cerr << "Error: Unknown blend mode: " << args.blend_mode << "\n";
                args.show_help = true;
                return args;
            }
        }
        else if (arg == "--ransac-threshold") {
            if (++i >= argc) {
                std::cerr << "Error: --ransac-threshold requires a value\n";
                args.show_help = true;
                return args;
            }
            if (!parseDouble(argv[i], args.ransac_threshold, "RANSAC threshold",
                            PanoramaConfig::MIN_RANSAC_THRESHOLD,
                            PanoramaConfig::MAX_RANSAC_THRESHOLD)) {
                args.show_help = true;
                return args;
            }
        }
        else if (arg == "--max-features") {
            if (++i >= argc) {
                std::cerr << "Error: --max-features requires a value\n";
                args.show_help = true;
                return args;
            }
            if (!parseInt(argv[i], args.max_features, "max features",
                         PanoramaConfig::MIN_FEATURES,
                         PanoramaConfig::MAX_FEATURES)) {
                args.show_help = true;
                return args;
            }
        }
        else if (arg == "--output") {
            if (++i >= argc) {
                std::cerr << "Error: --output requires a path\n";
                args.show_help = true;
                return args;
            }
            args.output_path = argv[i];
            if (!isValidOutputPath(args.output_path)) {
                args.show_help = true;
                return args;
            }
        }
        else if (arg == "--visualize") {
            args.visualize = true;
        }
        else {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            args.show_help = true;
            return args;
        }
    }

    return args;
}