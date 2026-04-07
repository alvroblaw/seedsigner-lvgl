#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/contracts/SettingsContract.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"
#include "seedsigner_lvgl/screens/ScanScreen.hpp"
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
    runtime.screen_registry().register_route(RouteId{"scan.qr"}, []() -> std::unique_ptr<Screen> { return std::make_unique<ScanScreen>(); });
    runtime.screen_registry().register_route(RouteId{"demo.result"}, []() -> std::unique_ptr<Screen> { return std::make_unique<ResultScreen>(); });
    runtime.screen_registry().register_route(RouteId{"settings.locale"}, []() -> std::unique_ptr<Screen> { return std::make_unique<SettingsSelectionScreen>(); });
    runtime.screen_registry().register_route(RouteId{"settings.features"}, []() -> std::unique_ptr<Screen> { return std::make_unique<SettingsSelectionScreen>(); });

    auto locale_args = make_settings_route_args(SettingDefinition{.id = "locale",
                                                                  .title = "Settings",
                                                                  .subtitle = "Language",
                                                                  .section_title = "Display language",
                                                                  .help_text = "Choose one language for the active UI session.",
                                                                  .footer_text = "Press to apply. Back to cancel.",
                                                                  .value_type = SettingValueType::SingleChoice,
                                                                  .default_values = {"en"},
                                                                  .current_values = {"es"},
                                                                  .items = {{.id = "en", .label = "English", .secondary_text = "Use the default Latin font stack"},
                                                                            {.id = "es", .label = "Español", .secondary_text = "Use accented glyphs in the UI"},
                                                                            {.id = "fr", .label = "Français", .secondary_text = "Preview wider Latin text coverage"}}});
    locale_args["selected_index"] = "1";
    runtime.activate({.route_id = RouteId{"settings.locale"}, .args = locale_args});
    runtime.send_input(InputEvent{.key = InputKey::Up});
    const auto focus = next_matching(runtime, EventType::ActionInvoked);
    if (!focus || !focus->meta || focus->meta->key != "en") return 5;
    std::cout << "settings focus=" << focus->meta->key << " payload=" << std::get<std::string>(focus->meta->value) << "\n";

    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto setting = next_matching(runtime, EventType::ActionInvoked);
    if (!setting || setting->action_id != std::optional<std::string>{"setting_selected"} || !setting->meta || setting->meta->key != "en") return 2;
    std::cout << "settings selected=" << setting->meta->key << " payload=" << std::get<std::string>(setting->meta->value) << "\n";

    auto features_args = make_settings_route_args(SettingDefinition{.id = "features",
                                                                    .title = "Settings",
                                                                    .subtitle = "Advanced features",
                                                                    .section_title = "Enabled features",
                                                                    .value_type = SettingValueType::MultiChoice,
                                                                    .default_values = {"dire_warnings"},
                                                                    .current_values = {"dire_warnings", "compact_seedqr"},
                                                                    .items = {{.id = "dire_warnings", .label = "Dire warnings", .secondary_text = "Require explicit confirm on dangerous flows", .item_type = SettingItemType::Toggle},
                                                                              {.id = "compact_seedqr", .label = "Compact SeedQR", .secondary_text = "Prefer compact QR exports when possible", .item_type = SettingItemType::Toggle},
                                                                              {.id = "passphrase", .label = "Passphrase support", .secondary_text = "Enable passphrase entry flows", .item_type = SettingItemType::Toggle}}});
    features_args["selected_index"] = "1";
    runtime.activate({.route_id = RouteId{"settings.features"}, .args = features_args});
    runtime.send_input(InputEvent{.key = InputKey::Press});
    const auto feature_toggle = next_matching(runtime, EventType::ActionInvoked);
    if (!feature_toggle || !feature_toggle->meta || feature_toggle->meta->key != "compact_seedqr") return 7;
    std::cout << "feature toggled=" << feature_toggle->meta->key << " payload=" << std::get<std::string>(feature_toggle->meta->value) << "\n";

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
