cmake_minimum_required(VERSION 3.16)
project(PanoramaStitching VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(OpenCV 4 REQUIRED COMPONENTS core imgcodecs imgproc features2d calib3d highgui)
find_package(OpenMP)
find_package(Threads REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -O3 -march=native)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
endif()

if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Common source files (without main)
set(COMMON_SOURCES
    # CLI modules
    src/cli/argument_parser.cpp
    # Pipeline modules
    src/pipeline/stitching_pipeline.cpp
    # Feature detection (removed empty feature_detector.cpp)
    src/feature_detection/orb_detector.cpp
    src/feature_detection/akaze_detector.cpp
    src/feature_detection/detector_factory.cpp
    # Feature matching
    src/feature_matching/matcher.cpp
    src/feature_matching/ransac.cpp
    # Homography
    src/homography/homography_estimator.cpp
    # Stitching
    src/stitching/image_warper.cpp
    src/stitching/blender.cpp
    src/stitching/blender_factory.cpp
    # Experiments
    src/experiments/experiment_runner.cpp
    src/experiments/visualization.cpp
    src/experiments/report_generator.cpp
)

# Set RPATH handling to avoid library conflicts
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Create original executable
add_executable(panorama_stitcher src/main.cpp ${COMMON_SOURCES})

# Set properties to handle conda/system library conflicts
set_target_properties(panorama_stitcher PROPERTIES
    BUILD_RPATH_USE_ORIGIN TRUE
    INSTALL_RPATH "$ORIGIN/../lib"
)

# Link libraries with proper ordering (system libs first for OpenMP)
target_link_libraries(panorama_stitcher 
    ${OpenCV_LIBS}
    Threads::Threads
    stdc++fs  # For std::filesystem
)

if(OpenMP_CXX_FOUND)
    # Use CMake's OpenMP target for portability
    target_link_libraries(panorama_stitcher OpenMP::OpenMP_CXX)
endif()

# Installation rules
install(TARGETS panorama_stitcher DESTINATION bin)

# Custom targets for experiments
add_custom_target(run_experiments
    COMMAND ${CMAKE_BINARY_DIR}/panorama_stitcher --experiment-mode
    DEPENDS panorama_stitcher
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running all experiments..."
)

message(STATUS "OpenCV: ${OpenCV_VERSION}, OpenMP: ${OpenMP_CXX_FOUND}")