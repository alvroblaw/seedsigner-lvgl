// profile_matrix_runner — runs the scenario suite across all display profiles.
//
// For each profile (square_240x240, portrait_240x320), runs every JSON scenario
// in SCENARIO_DIR.  Screenshots are organised under SCENARIO_OUT_DIR/<profile>/.
// Outputs a JSON matrix report with pass/fail per profile×scenario.
//
// Build:
//   cmake -S . -B build-matrix -DBUILD_HOST_DESKTOP=ON
//   cmake --build build-matrix --target profile_matrix_runner
//
// Run:
//   ./build-matrix/profile_matrix_runner
//   # or with env overrides:
//   SCENARIO_DIR=scenarios SCENARIO_OUT_DIR=matrix_out ./build-matrix/profile_matrix_runner

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "seedsigner_lvgl/desktop/ScenarioRunner.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/visual/DisplayProfile.hpp"
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
using namespace seedsigner::lvgl::profile;

struct ProfileDef {
    const char* name;
    uint32_t width;
    uint32_t height;
    ProfileId pid;
};

static const ProfileDef PROFILES[] = {
    {"square_240x240",   240, 240, ProfileId::Square240x240},
    {"portrait_240x320", 240, 320, ProfileId::Portrait240x320},
};
static constexpr size_t NUM_PROFILES = sizeof(PROFILES) / sizeof(PROFILES[0]);

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

static std::vector<std::string> find_scenarios(const std::string& dir) {
    std::vector<std::string> paths;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            paths.push_back(entry.path().filename().string());
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

// Remap screenshot paths in scenario JSON to go under profile subdirectory.
// This reads the JSON, rewrites "path" fields in screenshot steps, and writes
// a temporary file.  Returns the temp file path.
static std::string rewrite_scenario_for_profile(
    const std::string& scenario_path, const std::string& profile_name, const std::string& out_dir)
{
    std::ifstream f(scenario_path);
    if (!f.is_open()) return scenario_path;

    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    f.close();

    // Simple string replacement: replace "scenarios/out/" with out_dir/profile_name/
    // and "out/" prefix in screenshot paths
    const std::string profile_out = out_dir + "/" + profile_name;
    std::filesystem::create_directories(profile_out);

    // Rewrite: "path": "scenarios/out/xxx" → "path": "<profile_out>/xxx"
    //          "path": "out/xxx"           → "path": "<profile_out>/xxx"
    std::string marker1 = "scenarios/out/";
    std::string marker2 = "out/";
    size_t pos = 0;
    while ((pos = content.find("\"path\"", pos)) != std::string::npos) {
        size_t val_start = content.find('"', pos + 6);
        if (val_start == std::string::npos) break;
        size_t val_end = content.find('"', val_start + 1);
        if (val_end == std::string::npos) break;
        std::string old_path = content.substr(val_start + 1, val_end - val_start - 1);

        std::string filename = old_path;
        auto slash = old_path.find_last_of('/');
        if (slash != std::string::npos) filename = old_path.substr(slash + 1);

        std::string new_path = profile_out + "/" + filename;
        content.replace(val_start + 1, old_path.size(), new_path);
        pos = val_start + 1 + new_path.size();
    }

    // Write temp file
    std::string tmp_path = out_dir + "/.tmp_" + profile_name + "_" +
        std::filesystem::path(scenario_path).filename().string();
    std::ofstream tmp(tmp_path);
    tmp << content;
    tmp.close();
    return tmp_path;
}

int main(int argc, char** argv) {
    const char* scenario_dir_env = std::getenv("SCENARIO_DIR");
    const char* out_dir_env = std::getenv("SCENARIO_OUT_DIR");
    std::string scenario_dir = scenario_dir_env ? scenario_dir_env : "scenarios";
    std::string out_dir = out_dir_env ? out_dir_env : "scenarios/out";

    std::printf("=== SeedSigner-LVGL Profile Validation Matrix ===\n");
    std::printf("Scenario dir: %s\n", scenario_dir.c_str());
    std::printf("Output dir:   %s\n\n", out_dir.c_str());

    std::filesystem::create_directories(out_dir);

    auto scenario_names = find_scenarios(scenario_dir);
    if (scenario_names.empty()) {
        std::fprintf(stderr, "No .json scenarios found in %s\n", scenario_dir.c_str());
        return 1;
    }

    std::printf("Found %zu scenarios, %zu profiles:\n", scenario_names.size(), NUM_PROFILES);
    for (size_t p = 0; p < NUM_PROFILES; ++p)
        std::printf("  [%s] %ux%u\n", PROFILES[p].name, PROFILES[p].width, PROFILES[p].height);
    std::printf("\n");

    // Matrix results: [profile_idx][scenario_name] → pass/fail
    int total_pass = 0, total_fail = 0;
    std::map<std::string, std::map<std::string, bool>> matrix;

    for (size_t p = 0; p < NUM_PROFILES; ++p) {
        const auto& prof = PROFILES[p];
        std::printf("--- Profile: %s (%ux%u) ---\n", prof.name, prof.width, prof.height);

        for (const auto& sc_name : scenario_names) {
            std::string sc_path = scenario_dir + "/" + sc_name;

            // Rewrite screenshot paths for this profile
            std::string tmp_path = rewrite_scenario_for_profile(sc_path, prof.name, out_dir);

            std::fflush(stdout);
            std::fflush(stderr);

            pid_t pid = fork();
            if (pid == 0) {
                // Child: set up LVGL with this profile's resolution
                set_profile(prof.pid);
                UiRuntime rt(RuntimeConfig{
                    .width = prof.width,
                    .height = prof.height
                });
                rt.init();
                register_suite_routes(rt.screen_registry());

                auto result = ScenarioRunner::load_and_run(tmp_path, rt, prof.width, prof.height);

                // Cleanup temp file
                std::filesystem::remove(tmp_path);

                if (result.ok) {
                    std::printf("  [PASS] %s/%s (%d steps)\n", prof.name, sc_name.c_str(), result.steps_run);
                    _exit(0);
                } else {
                    std::fprintf(stderr, "  [FAIL] %s/%s (step %d): %s\n",
                                 prof.name, sc_name.c_str(), result.steps_run, result.error.c_str());
                    _exit(1);
                }
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
                matrix[prof.name][sc_name] = ok;
                if (ok) ++total_pass; else ++total_fail;
            } else {
                std::fprintf(stderr, "  [FAIL] %s/%s: fork error\n", prof.name, sc_name.c_str());
                matrix[prof.name][sc_name] = false;
                ++total_fail;
            }
        }
        std::printf("\n");
    }

    // Print matrix summary
    std::printf("\n=== Profile × Scenario Matrix ===\n");
    std::printf("%-25s", "scenario");
    for (size_t p = 0; p < NUM_PROFILES; ++p)
        std::printf(" %-18s", PROFILES[p].name);
    std::printf("\n");

    for (const auto& sc_name : scenario_names) {
        std::printf("%-25s", sc_name.c_str());
        for (size_t p = 0; p < NUM_PROFILES; ++p) {
            bool ok = matrix[PROFILES[p].name].count(sc_name) && matrix[PROFILES[p].name][sc_name];
            std::printf(" %-18s", ok ? "PASS" : "FAIL");
        }
        std::printf("\n");
    }

    // Write JSON report
    std::string report_path = out_dir + "/profile_matrix_report.json";
    {
        std::ofstream rp(report_path);
        rp << "{\n  \"profiles\": [\n";
        for (size_t p = 0; p < NUM_PROFILES; ++p) {
            const auto& prof = PROFILES[p];
            rp << "    {\n";
            rp << "      \"name\": \"" << prof.name << "\",\n";
            rp << "      \"width\": " << prof.width << ",\n";
            rp << "      \"height\": " << prof.height << ",\n";
            rp << "      \"results\": {\n";
            bool first = true;
            for (const auto& sc_name : scenario_names) {
                if (!first) rp << ",\n";
                first = false;
                bool ok = matrix[prof.name].count(sc_name) && matrix[prof.name][sc_name];
                rp << "        \"" << sc_name << "\": " << (ok ? "true" : "false");
            }
            rp << "\n      }\n";
            rp << "    }" << (p + 1 < NUM_PROFILES ? "," : "") << "\n";
        }
        rp << "  ],\n";
        rp << "  \"summary\": {\n";
        rp << "    \"total\": " << (total_pass + total_fail) << ",\n";
        rp << "    \"passed\": " << total_pass << ",\n";
        rp << "    \"failed\": " << total_fail << "\n";
        rp << "  }\n";
        rp << "}\n";
    }
    std::printf("\nJSON report → %s\n", report_path.c_str());

    std::printf("\n=== Results: %d passed, %d failed, %d total ===\n",
                total_pass, total_fail, total_pass + total_fail);
    return total_fail > 0 ? 1 : 0;
}
