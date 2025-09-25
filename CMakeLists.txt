cmake_minimum_required(VERSION 3.16)
project(PanoramaStitching VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV 4 REQUIRED COMPONENTS core imgcodecs imgproc features2d calib3d highgui)
find_package(Threads REQUIRED)

add_executable(panorama_stitcher
    src/main.cpp
    src/cli/argument_parser.cpp
    src/pipeline/stitching_pipeline.cpp
    src/feature_detection/orb_detector.cpp
    src/feature_detection/akaze_detector.cpp
    src/feature_detection/sift_detector.cpp
    src/feature_detection/detector_factory.cpp
    src/feature_matching/matcher.cpp
    src/feature_matching/ransac.cpp
    src/homography/homography_estimator.cpp
    src/stitching/image_warper.cpp
    src/stitching/blender.cpp
    src/stitching/blender_factory.cpp
    src/experiments/experiment_runner.cpp
    src/experiments/visualization.cpp
    src/experiments/report_generator.cpp
)

target_include_directories(panorama_stitcher PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(panorama_stitcher PRIVATE -Wall -Wextra -O3 -march=native)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(panorama_stitcher PRIVATE -g -O0)
    endif()
endif()

target_link_libraries(panorama_stitcher PRIVATE
    ${OpenCV_LIBS}
    Threads::Threads
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(panorama_stitcher PRIVATE stdc++fs)
endif()

install(TARGETS panorama_stitcher DESTINATION bin)

message(STATUS "OpenCV ${OpenCV_VERSION} detected")
