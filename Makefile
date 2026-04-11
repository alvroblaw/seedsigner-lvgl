# Top-level convenience targets for seedsigner-lvgl
# These wrap CMake build + screenshot tooling so you don't have to remember incantations.

BUILD_DIR ?= build

.PHONY: screenshots screenshot-diff update-screenshots clean-screenshots

# --- Build & run screenshot capture ------------------------------------------
screenshots: $(BUILD_DIR)/screenshot_tests
	@mkdir -p screenshots
	$(BUILD_DIR)/screenshot_tests

$(BUILD_DIR)/screenshot_tests:
	cmake -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON
	cmake --build $(BUILD_DIR) --target screenshot_tests

# --- Diff against baseline ---------------------------------------------------
screenshot-diff: screenshots
	python3 tools/screenshot_diff.py

# --- Refresh baselines -------------------------------------------------------
update-screenshots: screenshots
	python3 tools/screenshot_diff.py --update-baseline
	@echo "Baselines updated. Run: git add screenshots_baseline/"

# --- Clean generated screenshots (not baselines) -----------------------------
clean-screenshots:
	rm -rf screenshots screenshots_diff screenshot_diff_report.json
