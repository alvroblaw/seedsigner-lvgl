#pragma once
// ScenarioRunner — loads a JSON scenario file and replays it against a UiRuntime.
//
// Scenario JSON schema:
// {
//   "profile": [240, 320],          // optional, default [240,320]
//   "default_delay_ms": 100,        // optional, default 50
//   "steps": [
//     { "action": "activate", "route": "demo.menu", "args": { ... } },
//     { "action": "input", "key": "Down", "delay_ms": 100 },
//     { "action": "screenshot", "path": "out/menu.png" },
//     { "action": "wait", "ms": 500 }
//   ]
// }

#include <cstdint>
#include <string>

namespace seedsigner::lvgl {

class UiRuntime;

struct ScenarioResult {
    bool ok{false};
    int steps_run{0};
    std::string error;
};

class ScenarioRunner {
public:
    /// Load and validate a scenario from `path`.
    static ScenarioResult load_and_run(const std::string& path,
                                       UiRuntime& runtime,
                                       uint32_t runtime_width,
                                       uint32_t runtime_height);
};

}  // namespace seedsigner::lvgl
