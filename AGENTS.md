# Repository Guidelines

## Project Structure & Module Organization
This CMake-based C++17 project keeps core code in `src/`, organized by concern (`pipeline/`, `feature_detection/`, `feature_matching/`, `stitching/`, `experiments/`, and CLI glue). Automation lives in `scripts/`, datasets for quick validation in `datasets/`, and generated artifacts in `results/` or `results_analysis/`. Leave `build/` for CMake output and keep it out of version control.

## Build, Test & Development Commands
Use `make build` to configure and compile (binary: `build/panorama_stitcher`). `make test` runs the quick ORB-based stitch and writes `test_panorama.jpg`. `make run` executes the full experiment suite, while `make analyze` turns outputs into charts in `results_analysis/`. Iterate with `./scripts/run_panorama.sh --stitch <imgA> <imgB> --output <file>` for ad-hoc trials. Produce updated PDFs through `make report`, and reset artifacts with `make clean` or `make clean-all`.

## Coding Style & Naming Conventions
Match the existing four-space indentation and brace placement (`if (...) {`). Prefer `PascalCase` for classes (`StitchingPipeline`), `camelCase` for variables and functions, and `UPPER_SNAKE_CASE` for compile-time constants. Favor standard library containers and RAII resource management. Run `clang-format -style=Google -i <files>` before sending patches to stay aligned with the current layout.

## Testing Guidelines
Changes that touch stitching logic must pass `make test`; update the indoor scene pair only if your feature demands new inputs. For algorithmic work, capture `make run` metrics and leave CSVs in `results/` for reviewers. Include panoramas or keypoint overlays from `results/visualizations/` whenever behavior changes.

## Commit & Pull Request Guidelines
Recent history mixes short imperatives ("add sift") with typed summaries (`feat: ...`); prefer the latter: `feat: improve homography blending` stays scannable. Keep one logical change per commit and add a short body when context helps. Pull requests should explain intent, list validation commands (`make test`, `make analyze`), link issues, and include visuals or result paths when image quality shifts. Exclude large binaries and call out dataset edits.

## Data & Results Management
Keep raw assets in `datasets/` and avoid overwriting originals. Store generated panoramas, CSVs, and analysis artifacts under `results*/` so they are easy to clean. Before pushing, prune temporary exports and run `make clean` if needed to prevent accidental binary uploads.
