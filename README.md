# Panorama Stitching

Feature-based panorama stitching pipeline for the Visual Computing assignment. It detects keypoints (ORB, AKAZE, SIFT), matches descriptors, estimates homographies with RANSAC, warps images, and blends overlaps. Batch scripts automate experiment sweeps and analysis.

## Requirements

- Linux with CMake ≥ 3.16 and GCC/Clang with C++17 support
- OpenCV 4 development headers (`libopencv-dev` on Debian/Ubuntu)
- Python 3 with `pip`
- Python packages: `pandas`, `matplotlib`, `numpy`

Install the prerequisites on Debian/Ubuntu:

```bash
sudo apt-get install cmake g++ libopencv-dev python3 python3-pip
pip3 install --user pandas matplotlib numpy
```

## Build

```bash
make build
```

This configures CMake in `build/` and compiles the `panorama_stitcher` executable.

## Run Experiments

```bash
make run
```

`make run` rebuilds the project if needed and executes the scripted experiment suite (105 runs) covering detector comparisons, RANSAC thresholds, blending modes, and multi-image stitching. Metrics and panoramas are written to `results/`, and a summarized analysis (including `metrics_analysis.png`) is produced in `results_analysis/`.

Datasets are expected under `datasets/<scene>/img1.jpg`, `img2.jpg`, `img3.jpg` for the three provided scenes (`indoor_scene`, `outdoor_scene1`, `outdoor_scene2`).

To regenerate the analysis independently:

```bash
python3 scripts/analysis_pipeline.py
```

## Outputs

- `results/` – panoramas, visualizations, and `metrics.csv`
- `results_analysis/` – organized panoramas, plots, and `metrics_analysis.png`

Remove the build artifacts manually if needed: `rm -rf build/`.
