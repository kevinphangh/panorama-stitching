#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>

struct ProgramArguments {
    enum Mode {
        NONE,
        STITCH_TWO,
        STITCH_MULTIPLE,
        EXPERIMENT
    };

    Mode mode = NONE;
    std::vector<std::string> image_paths;
    std::string output_path = "panorama_output.jpg";
    std::string detector_type = "orb";
    std::string blend_mode = "feather";
    double ransac_threshold = 3.0;
    int max_features = 20000;
    bool visualize = false;
    bool show_help = false;
};

class ArgumentParser {
public:
    static ProgramArguments parse(int argc, char** argv);
    static void printUsage(const char* program_name);
    static bool isValidOutputPath(const std::string& path);

    template<typename T>
    static bool parseArgument(const std::string& arg, T& value, const std::string& param_name);

    static bool parseDouble(const std::string& arg, double& value, const std::string& param_name,
                           double min_val = std::numeric_limits<double>::lowest(),
                           double max_val = std::numeric_limits<double>::max());

    static bool parseInt(const std::string& arg, int& value, const std::string& param_name,
                        int min_val = std::numeric_limits<int>::min(),
                        int max_val = std::numeric_limits<int>::max());
};

template<typename T>
bool ArgumentParser::parseArgument(const std::string& arg, T& value, const std::string& param_name) {
    std::istringstream iss(arg);
    T temp;

    if (!(iss >> temp)) {
        std::cerr << "Error: Invalid " << param_name << " value: '" << arg << "'\n";
        return false;
    }

    std::string remainder;
    if (iss >> remainder) {
        std::cerr << "Error: Invalid " << param_name << " value: '" << arg << "' (extra characters)\n";
        return false;
    }

    value = temp;
    return true;
}

#endif // ARGUMENT_PARSER_H