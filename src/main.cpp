#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "cli/argument_parser.h"
#include "pipeline/stitching_pipeline.h"
#include "experiments/experiment_runner.h"

int main(int argc, char** argv) {
    // Parse command line arguments
    ProgramArguments args = ArgumentParser::parse(argc, argv);

    // Show help if requested or if there was an error
    if (args.show_help) {
        ArgumentParser::printUsage(argv[0]);
        return args.mode == ProgramArguments::NONE ? 1 : 0;
    }

    // Handle different modes
    switch (args.mode) {
        case ProgramArguments::EXPERIMENT: {
            std::cout << "\n=== Running experiments ===\n";
            ExperimentRunner runner;
            runner.runAllExperiments();
            runner.generateReport("results/report.md");
            runner.exportMetricsToCSV("results/experiment_metrics.csv");
            runner.generateVisualizations("results/visualizations");
            runner.exportMatchDistances("results");
            std::cout << "Experiments completed!\n";
            return 0;
        }

        case ProgramArguments::STITCH_TWO: {
            std::cout << "\n=== Stitching two images ===\n";
            cv::Mat result = StitchingPipeline::performStitching(
                args.image_paths[0],
                args.image_paths[1],
                args.detector_type,
                args.blend_mode,
                args.ransac_threshold,
                args.max_features,
                args.visualize
            );

            if (result.empty()) {
                std::cerr << "Stitching failed!\n";
                return 1;
            }

            cv::imwrite(args.output_path, result);
            std::cout << "Panorama saved to: " << args.output_path << "\n";

            if (args.visualize) {
                cv::imshow("Panorama", result);
                std::cout << "Press any key to exit...\n";
                cv::waitKey(0);
            }

            return 0;
        }

        case ProgramArguments::STITCH_MULTIPLE: {
            std::cout << "\n=== Stitching multiple images ===\n";
            std::vector<cv::Mat> images;

            // Load all images
            for (const auto& path : args.image_paths) {
                cv::Mat img = cv::imread(path);
                if (img.empty()) {
                    std::cerr << "Error: Could not load image: " << path << "\n";
                    return 1;
                }
                images.push_back(img);
            }

            // Perform sequential stitching
            cv::Mat result = StitchingPipeline::performSequentialStitching(
                images,
                args.detector_type,
                args.blend_mode,
                args.ransac_threshold,
                args.max_features,
                args.visualize
            );

            if (result.empty()) {
                std::cerr << "Stitching failed!\n";
                return 1;
            }

            cv::imwrite(args.output_path, result);
            std::cout << "Panorama saved to: " << args.output_path << "\n";

            if (args.visualize) {
                cv::imshow("Panorama", result);
                std::cout << "Press any key to exit...\n";
                cv::waitKey(0);
            }

            return 0;
        }

        default:
            std::cerr << "Error: No valid mode specified\n";
            ArgumentParser::printUsage(argv[0]);
            return 1;
    }
}