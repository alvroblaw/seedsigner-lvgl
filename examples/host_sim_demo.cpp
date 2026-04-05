#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"

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
    runtime.screen_registry().register_route(RouteId{"settings.locale"}, []() -> std::unique_ptr<Screen> { return std::make_unique<SettingsSelectionScreen>(); });

    runtime.activate({.route_id = RouteId{"settings.locale"},
                      .args = {{"title", "Settings"},
                               {"subtitle", "Language"},
                               {"section_title", "Display language"},
                               {"selected_index", "1"},
                               {"items", "en|English|Use the default Latin font stack\nes|Español|Use accented glyphs in the UI|check\nfr|Français|Preview wider Latin text coverage"}}});
    runtime.send_input(InputEvent{.key = InputKey::Up});
    const auto focus = next_matching(runtime, EventType::ActionInvoked);
    if (!focus || !focus->meta || focus->meta->key != "en") return 5;
    std::cout << "settings focus=" << focus->meta->key << "\n";

    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto setting = next_matching(runtime, EventType::ActionInvoked);
    if (!setting || setting->action_id != std::optional<std::string>{"setting_selected"} || !setting->meta || setting->meta->key != "en") return 2;
    std::cout << "settings selected=" << setting->meta->key << "\n";

    runtime.activate({.route_id = RouteId{"demo.menu"}, .args = {{"title", "Settings"}, {"items", "network|Network|Configure host bridge|chevron\ndisplay|Persistent display|Keep screen awake while plugged in|check\nscan|Scan QR demo|Open the camera preview shell|chevron"}}});
    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto menu_action = next_matching(runtime, EventType::ActionInvoked);
    if (!menu_action || !menu_action->meta || menu_action->meta->key != "network") return 6;
    std::cout << "menu selected=" << menu_action->meta->key << "\n";

    runtime.activate({.route_id = RouteId{"demo.scan"}, .args = {{"title", "Camera Preview"}, {"status", "Controller waiting for capture"}}});
    std::vector<std::uint8_t> preview_pixels(96 * 96, 0x18);
    for (std::uint32_t y = 0; y < 96; ++y) {
        for (std::uint32_t x = 32; x < 64; ++x) {
            preview_pixels[static_cast<std::size_t>(y) * 96 + x] = 0x88;
        }
        for (std::uint32_t x = 64; x < 96; ++x) {
            preview_pixels[static_cast<std::size_t>(y) * 96 + x] = 0xf0;
        }
    }
    runtime.push_frame(CameraFrame{.width = 96, .height = 96, .stride = 96, .sequence = 1, .pixels = std::move(preview_pixels)});
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
