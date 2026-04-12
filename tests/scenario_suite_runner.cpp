// scenario_suite_runner — headless scenario validation suite.
//
// Registers all screen-family routes, then runs every JSON scenario
// found in SCENARIO_DIR.  Each scenario drives activate/input/screenshot
// steps via ScenarioRunner.  Exit code 0 = all pass, 1 = any failure.
//
// Build:
//   cmake -S . -B build-suite -DBUILD_HOST_DESKTOP=ON
//   cmake --build build-suite --target scenario_suite_runner
//
// Run:
//   SCENARIO_OUT_DIR=/tmp/scenario-out ./build-suite/scenario_suite_runner

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <lvgl.h>

#include "seedsigner_lvgl/desktop/ScenarioRunner.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"
#include "seedsigner_lvgl/screens/WarningScreen.hpp"
#include "seedsigner_lvgl/screens/ErrorScreen.hpp"
#include "seedsigner_lvgl/screens/DireWarningScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"
#include "seedsigner_lvgl/screens/QRDisplayScreen.hpp"
#include "seedsigner_lvgl/screens/KeyboardScreen.hpp"
#include "seedsigner_lvgl/screens/SeedWordsScreen.hpp"
#include "seedsigner_lvgl/screens/PSBTOverviewScreen.hpp"
#include "seedsigner_lvgl/screens/StartupSplashScreen.hpp"

using namespace seedsigner::lvgl;

// Register all suite routes — mirrors the screen families used in scenarios.
static void register_suite_routes(ScreenRegistry& reg) {
    reg.register_route(RouteId{"suite.menu"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<MenuListScreen>(); });
    reg.register_route(RouteId{"suite.settings"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<SettingsSelectionScreen>(); });
    reg.register_route(RouteId{"suite.warning"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<WarningScreen>(); });
    reg.register_route(RouteId{"suite.error"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<ErrorScreen>(); });
    reg.register_route(RouteId{"suite.dire_warning"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<DireWarningScreen>(); });
    reg.register_route(RouteId{"suite.result"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<ResultScreen>(); });
    reg.register_route(RouteId{"suite.qr"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<QRDisplayScreen>(); });
    reg.register_route(RouteId{"suite.keyboard"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<KeyboardScreen>(); });
    reg.register_route(RouteId{"suite.seed_words"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<SeedWordsScreen>(); });
    reg.register_route(RouteId{"suite.psbt_overview"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<PSBTOverviewScreen>(); });
    reg.register_route(RouteId{"suite.splash"},
        []() -> std::unique_ptr<Screen> { return std::make_unique<StartupSplashScreen>(); });
}

// Discover scenario JSON files, sorted alphabetically.
static std::vector<std::string> find_scenarios(const std::string& dir) {
    std::vector<std::string> paths;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            paths.push_back(entry.path().string());
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

int main(int argc, char** argv) {
    // Scenario dir defaults to SCENARIO_DIR env or "scenarios" relative to CWD.
    const char* scenario_dir_env = std::getenv("SCENARIO_DIR");
    const char* out_dir_env = std::getenv("SCENARIO_OUT_DIR");
    std::string scenario_dir = scenario_dir_env ? scenario_dir_env : "scenarios";
    std::string out_dir = out_dir_env ? out_dir_env : "scenarios/out";

    std::printf("=== SeedSigner-LVGL Scenario Validation Suite ===\n");
    std::printf("Scenario dir: %s\n", scenario_dir.c_str());
    std::printf("Output dir:   %s\n\n", out_dir.c_str());

    // Create output directory.
    std::filesystem::create_directories(out_dir);

    auto scenarios = find_scenarios(scenario_dir);
    if (scenarios.empty()) {
        std::fprintf(stderr, "No .json scenarios found in %s\n", scenario_dir.c_str());
        return 1;
    }

    std::printf("Found %zu scenarios:\n", scenarios.size());
    for (const auto& s : scenarios) std::printf("  %s\n", s.c_str());
    std::printf("\n");

    int pass = 0, fail = 0;
    for (const auto& scenario_path : scenarios) {
        std::string name = std::filesystem::path(scenario_path).filename().string();

        // Fork to isolate LVGL state between scenarios.
        pid_t pid = fork();
        if (pid == 0) {
            // Child process.
            lv_init();
            UiRuntime rt(RuntimeConfig{.width = 240, .height = 320});
            rt.init();
            register_suite_routes(rt.screen_registry());

            auto result = ScenarioRunner::load_and_run(scenario_path, rt, 240, 320);
            if (result.ok) {
                std::printf("  [PASS] %s (%d steps)\n", name.c_str(), result.steps_run);
                _exit(0);
            } else {
                std::fprintf(stderr, "  [FAIL] %s (step %d): %s\n",
                             name.c_str(), result.steps_run, result.error.c_str());
                _exit(1);
            }
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                ++pass;
            } else {
                ++fail;
            }
        } else {
            std::fprintf(stderr, "  [FAIL] %s: fork error\n", name.c_str());
            ++fail;
        }
    }

    std::printf("\n=== Results: %d passed, %d failed, %zu total ===\n",
                pass, fail, scenarios.size());
    return fail > 0 ? 1 : 0;
}
