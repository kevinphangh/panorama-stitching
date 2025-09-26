CMAKE := cmake
BUILD_DIR := build
BUILD_TYPE := Release

BLUE := \033[0;34m
GREEN := \033[0;32m
NC := \033[0m

.PHONY: all build run
all: build

build:
	@echo "$(BLUE)Building panorama stitcher...$(NC)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@$(MAKE) -C $(BUILD_DIR) -j$$(nproc)
	@echo "$(GREEN)✓ Build complete!$(NC)"
	@echo "Binary at: $(BUILD_DIR)/panorama_stitcher"

run: build
	@echo "$(BLUE)Running experiments...$(NC)"
	@./scripts/run-experiments.sh