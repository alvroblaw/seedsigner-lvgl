#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"

namespace {
using namespace seedsigner::lvgl;

std::optional<UiEvent> next_matching(UiRuntime& runtime, EventType type) {
    while (const auto event = runtime.next_event()) {
        if (event->type == type) return event;
    }
    return std::nullopt;
}
}

int main() {
    UiRuntime runtime;
    if (!runtime.init()) return 1;

    runtime.screen_registry().register_route(RouteId{"demo.menu"}, []() -> std::unique_ptr<Screen> { return std::make_unique<MenuListScreen>(); });
    runtime.screen_registry().register_route(RouteId{"demo.scan"}, []() -> std::unique_ptr<Screen> { return std::make_unique<CameraPreviewScreen>(); });
    runtime.screen_registry().register_route(RouteId{"demo.result"}, []() -> std::unique_ptr<Screen> { return std::make_unique<ResultScreen>(); });

    runtime.activate({.route_id = RouteId{"demo.menu"}, .args = {{"title", "SeedSigner Demo"}, {"items", "scan|Scan QR\nback|Back"}}});
    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto menu_action = next_matching(runtime, EventType::ActionInvoked);
    if (!menu_action || !menu_action->meta || menu_action->meta->key != "scan") return 2;
    std::cout << "menu selected=" << menu_action->meta->key << "\n";

    runtime.activate({.route_id = RouteId{"demo.scan"}, .args = {{"title", "Camera Preview"}, {"status", "Controller waiting for capture"}}});
    runtime.push_frame(CameraFrame{.width = 96, .height = 96, .stride = 96, .sequence = 1, .pixels = std::vector<std::uint8_t>(96 * 96, 0x7f)});
    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto capture = next_matching(runtime, EventType::ActionInvoked);
    if (!capture || capture->action_id != std::optional<std::string>{"capture"}) return 3;
    std::cout << "captured frame=" << std::get<std::int64_t>(*capture->value) << "\n";

    runtime.activate({.route_id = RouteId{"demo.result"}, .args = {{"title", "Capture Result"}, {"body", "Captured mock frame #1. No QR decoding yet."}}});
    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto done = next_matching(runtime, EventType::ActionInvoked);
    if (!done) return 4;

    runtime.tick(16);
    runtime.refresh_now();
    std::cout << "route=" << runtime.get_active_route()->route_id.value() << " token=" << runtime.get_active_route()->screen_token << " flushes=" << runtime.display()->flush_count() << "\n";
    return 0;
}
