#include "seedsigner_lvgl/desktop/ScenarioRunner.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/platform/FramebufferCapture.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/runtime/InputEvent.hpp"

#include <cstdio>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <thread>
#include <chrono>

namespace seedsigner::lvgl {
namespace {

using json = nlohmann::json;

InputKey parse_input_key(const std::string& name) {
    if (name == "Up")    return InputKey::Up;
    if (name == "Down")  return InputKey::Down;
    if (name == "Left")  return InputKey::Left;
    if (name == "Right") return InputKey::Right;
    if (name == "Press") return InputKey::Press;
    if (name == "Back")  return InputKey::Back;
    throw std::runtime_error("unknown input key: " + name);
}

PropertyMap json_to_props(const json& obj) {
    PropertyMap m;
    if (!obj.is_object()) return m;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.value().is_string()) {
            m[it.key()] = it.value().get<std::string>();
        } else if (it.value().is_number()) {
            m[it.key()] = std::to_string(it.value().get<int>());
        } else if (it.value().is_boolean()) {
            m[it.key()] = it.value().get<bool>() ? "true" : "false";
        }
    }
    return m;
}

void drain_events(UiRuntime& rt) {
    while (rt.next_event()) {}
}

}  // namespace

ScenarioResult ScenarioRunner::load_and_run(const std::string& path,
                                             UiRuntime& runtime,
                                             uint32_t /*runtime_width*/,
                                             uint32_t /*runtime_height*/) {
    ScenarioResult result;

    // Load file
    std::ifstream f(path);
    if (!f.is_open()) {
        result.error = "cannot open scenario file: " + path;
        return result;
    }

    json scenario;
    try {
        scenario = json::parse(f);
    } catch (const json::parse_error& e) {
        result.error = std::string("JSON parse error: ") + e.what();
        return result;
    }

    // Validate top-level
    if (!scenario.contains("steps") || !scenario["steps"].is_array()) {
        result.error = "scenario must contain a 'steps' array";
        return result;
    }

    const uint32_t default_delay = scenario.value("default_delay_ms", 50u);
    const auto& steps = scenario["steps"];

    for (const auto& step : steps) {
        if (!step.contains("action") || !step["action"].is_string()) {
            result.error = "each step must have a string 'action' field";
            return result;
        }
        const std::string action = step["action"];

        try {
            if (action == "activate") {
                if (!step.contains("route") || !step["route"].is_string()) {
                    result.error = "'activate' step needs a string 'route' field";
                    return result;
                }
                RouteDescriptor desc;
                desc.route_id = RouteId{step["route"].get<std::string>()};
                if (step.contains("args") && step["args"].is_object()) {
                    desc.args = json_to_props(step["args"]);
                }
                runtime.activate(desc);
                drain_events(runtime);

            } else if (action == "input") {
                if (!step.contains("key") || !step["key"].is_string()) {
                    result.error = "'input' step needs a string 'key' field";
                    return result;
                }
                InputKey key = parse_input_key(step["key"].get<std::string>());
                runtime.send_input(InputEvent{key});
                drain_events(runtime);

            } else if (action == "screenshot") {
                if (!step.contains("path") || !step["path"].is_string()) {
                    result.error = "'screenshot' step needs a string 'path' field";
                    return result;
                }
                const std::string sp = step["path"].get<std::string>();
                runtime.refresh_now();
                const auto* disp = runtime.display();
                if (!disp) {
                    result.error = "no headless display available for screenshot";
                    return result;
                }
                if (!FramebufferCapture::write_png(sp, *disp)) {
                    result.error = "failed to write screenshot: " + sp;
                    return result;
                }
                std::printf("[scenario] screenshot → %s\n", sp.c_str());

            } else if (action == "wait") {
                uint32_t ms = step.value("ms", 100u);
                runtime.tick(ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));

            } else {
                result.error = "unknown action: " + action;
                return result;
            }
        } catch (const std::exception& e) {
            result.error = std::string("step error (") + action + "): " + e.what();
            return result;
        }

        ++result.steps_run;

        // Inter-step delay
        const uint32_t delay = step.value("delay_ms", default_delay);
        if (delay > 0) {
            runtime.tick(delay);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    result.ok = true;
    return result;
}

}  // namespace seedsigner::lvgl
