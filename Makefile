.PHONY: build configure run run-single clean distclean

CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Release
BUILD_DIR ?= build
NPROC ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

build: configure
	cmake --build $(BUILD_DIR) -- -j$(NPROC)

run: build
	./scripts/run-experiments.sh

run-single: build
	@if [ -z "$(ARGS)" ]; then \
		echo "Usage: make run-single ARGS=\"--stitch datasets/<scene>/img1.jpg datasets/<scene>/img2.jpg ...\""; \
		exit 1; \
	fi
	./scripts/run_panorama.sh $(ARGS)

clean:
	@if [ -d $(BUILD_DIR) ]; then \
		cmake --build $(BUILD_DIR) --target clean; \
	fi
	rm -rf results results_analysis

distclean:
	rm -rf $(BUILD_DIR) results results_analysis
