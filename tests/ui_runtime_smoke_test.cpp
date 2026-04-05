#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <lvgl.h>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"

namespace tests {

namespace {
using seedsigner::lvgl::CameraFrame;
using seedsigner::lvgl::EventType;
using seedsigner::lvgl::InputEvent;
using seedsigner::lvgl::InputKey;
using seedsigner::lvgl::RouteDescriptor;
using seedsigner::lvgl::RouteId;
using seedsigner::lvgl::Screen;
using seedsigner::lvgl::UiEvent;
using seedsigner::lvgl::UiRuntime;

std::optional<UiEvent> next_matching(UiRuntime& runtime, EventType type) {
    while (const auto event = runtime.next_event()) {
        if (event->type == type) return event;
    }
    return std::nullopt;
}

std::unique_ptr<Screen> make_placeholder() {
    return std::make_unique<seedsigner::lvgl::PlaceholderScreen>();
}

bool label_tree_contains(lv_obj_t* root, const std::string& text) {
    if (root == nullptr) {
        return false;
    }
    if (lv_obj_check_type(root, &lv_label_class)) {
        const char* current = lv_label_get_text(root);
        if (current != nullptr && text == current) {
            return true;
        }
    }

    const auto child_count = lv_obj_get_child_cnt(root);
    for (std::uint32_t i = 0; i < child_count; ++i) {
        if (label_tree_contains(lv_obj_get_child(root, i), text)) {
            return true;
        }
    }
    return false;
}

lv_obj_t* find_first_canvas(lv_obj_t* root) {
    if (root == nullptr) {
        return nullptr;
    }
    if (lv_obj_check_type(root, &lv_canvas_class)) {
        return root;
    }

    const auto child_count = lv_obj_get_child_cnt(root);
    for (std::uint32_t i = 0; i < child_count; ++i) {
        if (auto* found = find_first_canvas(lv_obj_get_child(root, i)); found != nullptr) {
            return found;
        }
    }
    return nullptr;
}
}  // namespace

void test_headless_runtime_bootstrap() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"demo.placeholder"}, make_placeholder));
    const auto active = runtime.activate({.route_id = RouteId{"demo.placeholder"}, .args = {{"title", "Smoke Test"}, {"body", "LVGL flush should happen."}}});
    assert(active.has_value());
    runtime.tick(16);
    runtime.refresh_now();
    assert(runtime.display() != nullptr);
    assert(runtime.display()->flush_count() > 0);
    assert(runtime.next_event()->type == EventType::RouteActivated);
    assert(runtime.next_event()->type == EventType::ScreenReady);
}

void test_settings_selection_route_demo() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"settings.locale"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::SettingsSelectionScreen>(); }));

    const auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"settings.locale"},
                                                         .args = {{"title", "Settings"},
                                                                  {"subtitle", "Language"},
                                                                  {"section_title", "Display language"},
                                                                  {"selected_index", "1"},
                                                                  {"items", "en|English|Use the default Latin font stack\nes|Español|Use accented glyphs in the UI|check\nfr|Français|Preview wider Latin text coverage"}}});
    assert(active.has_value());
    runtime.tick(16);
    runtime.refresh_now();

    assert(label_tree_contains(lv_scr_act(), "Settings"));
    assert(label_tree_contains(lv_scr_act(), "Language"));
    assert(label_tree_contains(lv_scr_act(), "Display language"));

    runtime.next_event();
    runtime.next_event();
    assert(runtime.send_input(InputEvent{.key = InputKey::Up}));
    auto focus_event = next_matching(runtime, EventType::ActionInvoked);
    assert(focus_event.has_value() && focus_event->action_id == std::optional<std::string>{"focus_changed"});
    assert(focus_event->meta.has_value());
    assert(focus_event->meta->key == "en");

    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto select_event = next_matching(runtime, EventType::ActionInvoked);
    assert(select_event.has_value() && select_event->action_id == std::optional<std::string>{"setting_selected"});
    assert(select_event->component_id == std::optional<std::string>{"settings_selection"});
    assert(select_event->meta.has_value());
    assert(select_event->meta->key == "en");

    assert(runtime.display()->flush_count() > 0);
}

void test_external_scan_flow_demo() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"demo.menu"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::MenuListScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"demo.scan"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::CameraPreviewScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"demo.result"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::ResultScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"settings.locale"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::SettingsSelectionScreen>(); }));

    auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"settings.locale"}, .args = {{"title", "Settings"}, {"subtitle", "Language"}, {"section_title", "Display language"}, {"selected_index", "1"}, {"items", "en|English|Use the default Latin font stack\nes|Español|Use accented glyphs in the UI|check\nfr|Français|Preview wider Latin text coverage"}}});
    assert(active.has_value());
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto locale_action = next_matching(runtime, EventType::ActionInvoked);
    assert(locale_action.has_value() && locale_action->action_id == std::optional<std::string>{"setting_selected"});
    assert(locale_action->meta.has_value());
    assert(locale_action->meta->key == "es");


    active = runtime.activate(RouteDescriptor{.route_id = RouteId{"demo.menu"}, .args = {{"title", "Settings"}, {"items", "network|Network|Configure host bridge|chevron\ndisplay|Persistent display|Keep screen awake while plugged in|check\nback|Back"}}});
    assert(active.has_value());
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto menu_action = next_matching(runtime, EventType::ActionInvoked);
    assert(menu_action.has_value() && menu_action->action_id == std::optional<std::string>{"item_selected"});
    assert(menu_action->meta.has_value());
    assert(menu_action->meta->key == "network");

    assert(runtime.send_input(InputEvent{.key = InputKey::Down}));
    auto focus_event = next_matching(runtime, EventType::ActionInvoked);
    assert(focus_event.has_value() && focus_event->action_id == std::optional<std::string>{"focus_changed"});
    assert(focus_event->meta.has_value());
    assert(focus_event->meta->key == "display");

    active = runtime.activate(RouteDescriptor{.route_id = RouteId{"demo.scan"}, .args = {{"title", "Scan QR"}, {"status", "Waiting for host capture command"}}});
    assert(active.has_value());

    std::vector<std::uint8_t> pixels(8 * 8, 0x10);
    for (std::uint32_t y = 0; y < 8; ++y) {
        for (std::uint32_t x = 4; x < 8; ++x) {
            pixels[static_cast<std::size_t>(y) * 8 + x] = 0xf0;
        }
    }

    assert(runtime.push_frame(CameraFrame{.width = 8, .height = 8, .stride = 8, .sequence = 3, .pixels = pixels}));
    runtime.refresh_now();

    auto* canvas = find_first_canvas(lv_scr_act());
    assert(canvas != nullptr);
    const auto left_px = lv_canvas_get_px(canvas, 40, 72);
    const auto right_px = lv_canvas_get_px(canvas, 176, 72);
    assert(left_px.ch.red < right_px.ch.red);
    assert(left_px.ch.green < right_px.ch.green);
    assert(left_px.ch.blue < right_px.ch.blue);

    assert(runtime.patch_screen_data({{"status", "Frame 3 ready for capture"}}));
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto capture_action = next_matching(runtime, EventType::ActionInvoked);
    assert(capture_action.has_value() && capture_action->action_id == std::optional<std::string>{"capture"});
    assert(std::get<std::int64_t>(*capture_action->value) == 3);

    active = runtime.activate(RouteDescriptor{.route_id = RouteId{"demo.result"}, .args = {{"title", "Capture Result"}, {"body", "Mock frame #3 captured. No QR decode in this slice."}}});
    assert(active.has_value());
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto result_action = next_matching(runtime, EventType::ActionInvoked);
    assert(result_action.has_value() && result_action->action_id == std::optional<std::string>{"continue"});

    runtime.tick(16);
    runtime.refresh_now();
    assert(runtime.display()->flush_count() > 0);
}

}  // namespace tests
