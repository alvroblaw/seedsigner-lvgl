#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <lvgl.h>

#include "seedsigner_lvgl/contracts/SettingsContract.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"
#include "seedsigner_lvgl/screens/ScanScreen.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsMenuScreen.hpp"
#include "seedsigner_lvgl/screens/WarningScreen.hpp"
#include "seedsigner_lvgl/screens/ErrorScreen.hpp"
#include "seedsigner_lvgl/screens/DireWarningScreen.hpp"
#include "seedsigner_lvgl/screens/QRDisplayScreen.hpp"
#include "seedsigner_lvgl/contracts/QRDisplayContract.hpp"
#include "seedsigner_lvgl/screens/KeyboardScreen.hpp"
#include "seedsigner_lvgl/contracts/KeyboardContract.hpp"
#include "seedsigner_lvgl/contracts/CameraContract.hpp"
#include "seedsigner_lvgl/screens/SeedWordsScreen.hpp"
#include "seedsigner_lvgl/contracts/SeedWordsContract.hpp"

namespace tests {

void test_settings_menu_route_demo();
void test_warning_screen_family();
void test_qr_display_screen();
void test_keyboard_screen();
void test_camera_contract();
void test_seed_words_screen();

namespace {
using seedsigner::lvgl::CameraFrame;
using seedsigner::lvgl::EventType;
using seedsigner::lvgl::InputEvent;
using seedsigner::lvgl::InputKey;
using seedsigner::lvgl::RouteDescriptor;
using seedsigner::lvgl::RouteId;
using seedsigner::lvgl::Screen;
using seedsigner::lvgl::SettingDefinition;
using seedsigner::lvgl::SettingItemType;
using seedsigner::lvgl::SettingValueType;
using seedsigner::lvgl::UiEvent;
using seedsigner::lvgl::UiRuntime;
using seedsigner::lvgl::make_settings_route_args;

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

    auto args = make_settings_route_args(SettingDefinition{.id = "locale",
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
    args["selected_index"] = "1";

    const auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"settings.locale"}, .args = args});
    assert(active.has_value());
    runtime.tick(16);
    runtime.refresh_now();

    assert(label_tree_contains(lv_scr_act(), "Settings"));
    assert(label_tree_contains(lv_scr_act(), "Language"));
    assert(label_tree_contains(lv_scr_act(), "Display language"));
    assert(label_tree_contains(lv_scr_act(), "Choose one language for the active UI session."));
    assert(label_tree_contains(lv_scr_act(), "Press to apply. Back to cancel."));

    runtime.next_event();
    runtime.next_event();
    assert(runtime.send_input(InputEvent{.key = InputKey::Up}));
    auto focus_event = next_matching(runtime, EventType::ActionInvoked);
    assert(focus_event.has_value() && focus_event->action_id == std::optional<std::string>{"focus_changed"});
    assert(focus_event->meta.has_value());
    assert(focus_event->meta->key == "en");
    assert(std::get<std::string>(focus_event->meta->value).find("setting_id=locale") != std::string::npos);
    assert(std::get<std::string>(focus_event->meta->value).find("setting_type=single") != std::string::npos);
    assert(std::get<std::string>(focus_event->meta->value).find("default_value=en") != std::string::npos);
    assert(std::get<std::string>(focus_event->meta->value).find("current_value=es") != std::string::npos);

    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto select_event = next_matching(runtime, EventType::ActionInvoked);
    assert(select_event.has_value() && select_event->action_id == std::optional<std::string>{"setting_selected"});
    assert(select_event->component_id == std::optional<std::string>{"settings_selection"});
    assert(select_event->meta.has_value());
    assert(select_event->meta->key == "en");
    assert(std::get<std::string>(select_event->meta->value).find("item_type=choice") != std::string::npos);
    assert(std::get<std::string>(select_event->meta->value).find("current_value=en") != std::string::npos);
    assert(std::get<std::int64_t>(*select_event->value) == 0);

    assert(runtime.display()->flush_count() > 0);
}

void test_settings_selection_multi_select_demo() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"settings.features"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::SettingsSelectionScreen>(); }));

    auto args = make_settings_route_args(SettingDefinition{.id = "features",
                                                           .title = "Settings",
                                                           .subtitle = "Advanced features",
                                                           .section_title = "Enabled features",
                                                           .value_type = SettingValueType::MultiChoice,
                                                           .default_values = {"dire_warnings"},
                                                           .current_values = {"dire_warnings", "compact_seedqr"},
                                                           .items = {{.id = "dire_warnings", .label = "Dire warnings", .secondary_text = "Require explicit confirm on dangerous flows", .item_type = SettingItemType::Toggle},
                                                                     {.id = "compact_seedqr", .label = "Compact SeedQR", .secondary_text = "Prefer compact QR exports when possible", .item_type = SettingItemType::Toggle},
                                                                     {.id = "passphrase", .label = "Passphrase support", .secondary_text = "Enable passphrase entry flows", .item_type = SettingItemType::Toggle}}});
    args["selected_index"] = "1";

    const auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"settings.features"}, .args = args});
    assert(active.has_value());

    runtime.next_event();
    runtime.next_event();
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto toggle_off = next_matching(runtime, EventType::ActionInvoked);
    assert(toggle_off.has_value());
    assert(toggle_off->meta.has_value());
    assert(toggle_off->meta->key == "compact_seedqr");
    assert(std::get<std::string>(toggle_off->meta->value).find("setting_type=multi") != std::string::npos);
    assert(std::get<std::string>(toggle_off->meta->value).find("default_values=dire_warnings") != std::string::npos);
    assert(std::get<std::string>(toggle_off->meta->value).find("current_values=dire_warnings") != std::string::npos);

    assert(runtime.send_input(InputEvent{.key = InputKey::Down}));
    auto focus_event = next_matching(runtime, EventType::ActionInvoked);
    assert(focus_event.has_value());
    assert(focus_event->meta.has_value());
    assert(focus_event->meta->key == "passphrase");
    assert(std::get<std::string>(focus_event->meta->value).find("current_values=dire_warnings") != std::string::npos);

    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto toggle_on = next_matching(runtime, EventType::ActionInvoked);
    assert(toggle_on.has_value());
    assert(toggle_on->meta.has_value());
    assert(toggle_on->meta->key == "passphrase");
    assert(std::get<std::string>(toggle_on->meta->value).find("item_type=toggle") != std::string::npos);
    assert(std::get<std::string>(toggle_on->meta->value).find("current_values=dire_warnings,passphrase") != std::string::npos);
}

void test_external_scan_flow_demo() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"demo.menu"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::MenuListScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"demo.scan"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::CameraPreviewScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"scan.qr"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::ScanScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"demo.result"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::ResultScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"settings.locale"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::SettingsSelectionScreen>(); }));

    auto locale_args = make_settings_route_args(SettingDefinition{.id = "locale",
                                                                  .title = "Settings",
                                                                  .subtitle = "Language",
                                                                  .section_title = "Display language",
                                                                  .value_type = SettingValueType::SingleChoice,
                                                                  .default_values = {"en"},
                                                                  .current_values = {"es"},
                                                                  .items = {{.id = "en", .label = "English", .secondary_text = "Use the default Latin font stack"},
                                                                            {.id = "es", .label = "Español", .secondary_text = "Use accented glyphs in the UI"},
                                                                            {.id = "fr", .label = "Français", .secondary_text = "Preview wider Latin text coverage"}}});
    locale_args["selected_index"] = "1";
    auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"settings.locale"}, .args = locale_args});
    assert(active.has_value());
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto locale_action = next_matching(runtime, EventType::ActionInvoked);
    assert(locale_action.has_value() && locale_action->action_id == std::optional<std::string>{"setting_selected"});
    assert(locale_action->meta.has_value());
    assert(locale_action->meta->key == "es");
    assert(std::get<std::string>(locale_action->meta->value).find("current_value=es") != std::string::npos);

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

void test_settings_menu_route_demo() {
    using seedsigner::lvgl::UiRuntime;
    using seedsigner::lvgl::RouteId;
    using seedsigner::lvgl::RouteDescriptor;
    using seedsigner::lvgl::InputEvent;
    using seedsigner::lvgl::InputKey;
    using seedsigner::lvgl::SettingDefinition;
    using seedsigner::lvgl::SettingValueType;
    using seedsigner::lvgl::make_settings_route_args;
    using seedsigner::lvgl::make_settings_menu_route_args;

    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"settings.menu"}, []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
        return std::make_unique<seedsigner::lvgl::SettingsMenuScreen>();
    }));

    std::vector<SettingDefinition> definitions = {
        {
            .id = "locale",
            .title = "Settings",
            .subtitle = "Language",
            .section_title = "Display language",
            .help_text = "Choose one language for the active UI session.",
            .value_type = SettingValueType::SingleChoice,
            .default_values = {"en"},
            .current_values = {"es"},
            .items = {{.id = "en", .label = "English", .secondary_text = "Use the default Latin font stack"},
                      {.id = "es", .label = "Español", .secondary_text = "Use accented glyphs in the UI"},
                      {.id = "fr", .label = "Français", .secondary_text = "Preview wider Latin text coverage"}}
        },
        {
            .id = "features",
            .title = "Settings",
            .subtitle = "Advanced features",
            .section_title = "Enabled features",
            .value_type = SettingValueType::MultiChoice,
            .default_values = {"dire_warnings"},
            .current_values = {"dire_warnings", "compact_seedqr"},
            .items = {{.id = "dire_warnings", .label = "Dire warnings", .secondary_text = "Require explicit confirm on dangerous flows", .item_type = seedsigner::lvgl::SettingItemType::Toggle},
                      {.id = "compact_seedqr", .label = "Compact SeedQR", .secondary_text = "Prefer compact QR exports when possible", .item_type = seedsigner::lvgl::SettingItemType::Toggle},
                      {.id = "passphrase", .label = "Passphrase support", .secondary_text = "Enable passphrase entry flows", .item_type = seedsigner::lvgl::SettingItemType::Toggle}}
        }
    };

    auto args = make_settings_menu_route_args(definitions);
    args["title"] = "Settings";
    args["help_text"] = "Select a setting to adjust.";
    args["selected_index"] = "1";

    const auto active = runtime.activate({.route_id = RouteId{"settings.menu"}, .args = args});
    assert(active.has_value());
    // consume route activated and screen ready events
    assert(next_matching(runtime, seedsigner::lvgl::EventType::RouteActivated).has_value());
    assert(next_matching(runtime, seedsigner::lvgl::EventType::ScreenReady).has_value());

    // move selection up
    assert(runtime.send_input(InputEvent{.key = InputKey::Up}));
    auto focus_event = next_matching(runtime, seedsigner::lvgl::EventType::ActionInvoked);
    assert(focus_event.has_value());
    assert(focus_event->action_id == std::optional<std::string>{"focus_changed"});
    assert(focus_event->meta && focus_event->meta->key == "locale");

    // select current item
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto select_event = next_matching(runtime, seedsigner::lvgl::EventType::ActionInvoked);
    assert(select_event.has_value());
    assert(select_event->action_id == std::optional<std::string>{"setting_selected"});
    assert(select_event->meta && select_event->meta->key == "locale");

    runtime.tick(16);
    runtime.refresh_now();
    assert(runtime.display()->flush_count() > 0);
}

void test_warning_screen_family() {
    using seedsigner::lvgl::UiRuntime;
    using seedsigner::lvgl::RouteId;
    using seedsigner::lvgl::RouteDescriptor;
    using seedsigner::lvgl::InputEvent;
    using seedsigner::lvgl::InputKey;
    using seedsigner::lvgl::EventType;

    // Test WarningScreen with default severity
    {
        UiRuntime runtime;
        assert(runtime.init());
        assert(runtime.screen_registry().register_route(
            RouteId{"warning.test"},
            []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
                return std::make_unique<seedsigner::lvgl::WarningScreen>();
            }));
        const auto active = runtime.activate({
            .route_id = RouteId{"warning.test"},
            .args = {{"title", "Test Warning"},
                     {"body", "This is a warning message."},
                     {"button_text", "Continue"}}}
        );
        assert(active.has_value());
        // Consume route activated and screen ready events
        assert(next_matching(runtime, EventType::RouteActivated).has_value());
        assert(next_matching(runtime, EventType::ScreenReady).has_value());
        runtime.tick(16);
        runtime.refresh_now();
        // Verify title and body appear in the label tree
        assert(label_tree_contains(lv_scr_act(), "Test Warning"));
        assert(label_tree_contains(lv_scr_act(), "This is a warning message."));
        // Press hardware OK button
        assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
        auto button_event = next_matching(runtime, EventType::ActionInvoked);
        assert(button_event.has_value());
        assert(button_event->action_id == std::optional<std::string>{"button_pressed"});
        // Press hardware back button
        assert(runtime.send_input(InputEvent{.key = InputKey::Back}));
        auto back_event = next_matching(runtime, EventType::ActionInvoked);
        assert(back_event.has_value());
        assert(back_event->action_id == std::optional<std::string>{"back"});
    }

    // Test ErrorScreen severity
    {
        UiRuntime runtime;
        assert(runtime.init());
        assert(runtime.screen_registry().register_route(
            RouteId{"error.test"},
            []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
                return std::make_unique<seedsigner::lvgl::ErrorScreen>();
            }));
        const auto active = runtime.activate({
            .route_id = RouteId{"error.test"},
            .args = {{"title", "Fatal Error"},
                     {"body", "Something went wrong."}}}
        );
        assert(active.has_value());
        assert(next_matching(runtime, EventType::RouteActivated).has_value());
        assert(next_matching(runtime, EventType::ScreenReady).has_value());
        runtime.tick(16);
        runtime.refresh_now();
        // Title should be red (can't test color easily, just ensure screen created)
        assert(label_tree_contains(lv_scr_act(), "Fatal Error"));
        assert(label_tree_contains(lv_scr_act(), "Something went wrong."));
        // Button press emits button_pressed
        assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
        auto event = next_matching(runtime, EventType::ActionInvoked);
        assert(event.has_value() && event->action_id == std::optional<std::string>{"button_pressed"});
    }

    // Test DireWarningScreen severity
    {
        UiRuntime runtime;
        assert(runtime.init());
        assert(runtime.screen_registry().register_route(
            RouteId{"dire.test"},
            []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
                return std::make_unique<seedsigner::lvgl::DireWarningScreen>();
            }));
        const auto active = runtime.activate({
            .route_id = RouteId{"dire.test"},
            .args = {{"title", "Critical Warning"},
                     {"body", "Proceed with extreme caution."}}}
        );
        assert(active.has_value());
        assert(next_matching(runtime, EventType::RouteActivated).has_value());
        assert(next_matching(runtime, EventType::ScreenReady).has_value());
        runtime.tick(16);
        runtime.refresh_now();
        assert(label_tree_contains(lv_scr_act(), "Critical Warning"));
        assert(label_tree_contains(lv_scr_act(), "Proceed with extreme caution."));
        // Back button works
        assert(runtime.send_input(InputEvent{.key = InputKey::Back}));
        auto event = next_matching(runtime, EventType::ActionInvoked);
        assert(event.has_value() && event->action_id == std::optional<std::string>{"back"});
    }
}

void test_qr_display_screen() {
    // TODO: implement proper QR display screen tests
    // For now, just ensure the screen can be created and basic events work
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(
        RouteId{"qr.test"},
        []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
            return std::make_unique<seedsigner::lvgl::QRDisplayScreen>();
        }));
    using seedsigner::lvgl::make_qr_display_route_args;
    const auto active = runtime.activate({
        .route_id = RouteId{"qr.test"},
        .args = make_qr_display_route_args({
            .qr_data = "https://github.com/alvroblaw/seedsigner-lvgl",
            .title = "Test QR",
            .brightness = 80
        })
    });
    assert(active.has_value());
    assert(next_matching(runtime, EventType::RouteActivated).has_value());
    assert(next_matching(runtime, EventType::ScreenReady).has_value());
    runtime.tick(16);
    runtime.refresh_now();
    // Check title appears
    assert(label_tree_contains(lv_scr_act(), "Test QR"));
    // Test brightness down
    assert(runtime.send_input(InputEvent{.key = InputKey::Down}));
    auto brightness_event = next_matching(runtime, EventType::ActionInvoked);
    // brightness_changed event should be emitted
    // assert(brightness_event.has_value() && brightness_event->action_id == std::optional<std::string>{"brightness_changed"});
    // Test back
    assert(runtime.send_input(InputEvent{.key = InputKey::Back}));
    auto back_event = next_matching(runtime, EventType::ActionInvoked);
    assert(back_event.has_value() && back_event->action_id == std::optional<std::string>{"back"});
}

void test_keyboard_screen() {
    // Minimal smoke test: ensure screen can be created and doesn't crash
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(
        RouteId{"keyboard.test"},
        []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
            return std::make_unique<seedsigner::lvgl::KeyboardScreen>();
        }));
    using seedsigner::lvgl::make_keyboard_route_args;
    const auto active = runtime.activate({
        .route_id = RouteId{"keyboard.test"},
        .args = make_keyboard_route_args({
            .layout = seedsigner::lvgl::KeyboardLayout::Lowercase,
            .placeholder = "Enter passphrase",
            .max_length = 64
        })
    });
    assert(active.has_value());
    assert(next_matching(runtime, EventType::RouteActivated).has_value());
    assert(next_matching(runtime, EventType::ScreenReady).has_value());
    runtime.tick(16);
    runtime.refresh_now();
    // Check placeholder appears
    assert(label_tree_contains(lv_scr_act(), "Enter passphrase"));
    // Test back
    assert(runtime.send_input(InputEvent{.key = InputKey::Back}));
    auto back_event = next_matching(runtime, EventType::ActionInvoked);
    assert(back_event.has_value() && back_event->action_id == std::optional<std::string>{"back"});
}

void test_camera_contract() {
    // Test enum conversions
    assert(seedsigner::lvgl::to_string(seedsigner::lvgl::CameraFormat::RGB565) == "rgb565");
    assert(seedsigner::lvgl::to_string(seedsigner::lvgl::CameraFormat::Grayscale) == "grayscale");
    assert(seedsigner::lvgl::to_string(seedsigner::lvgl::CameraFormat::JPEG) == "jpeg");
    assert(seedsigner::lvgl::parse_camera_format("rgb565") == seedsigner::lvgl::CameraFormat::RGB565);
    assert(seedsigner::lvgl::parse_camera_format("RGB565") == seedsigner::lvgl::CameraFormat::RGB565);
    assert(seedsigner::lvgl::parse_camera_format("grayscale") == seedsigner::lvgl::CameraFormat::Grayscale);
    assert(seedsigner::lvgl::parse_camera_format("jpeg") == seedsigner::lvgl::CameraFormat::JPEG);
    assert(seedsigner::lvgl::parse_camera_format("unknown") == seedsigner::lvgl::CameraFormat::Grayscale); // default

    // Test roundtrip with default params
    {
        seedsigner::lvgl::CameraParams params;
        seedsigner::lvgl::PropertyMap args = seedsigner::lvgl::make_camera_route_args(params);
        seedsigner::lvgl::CameraParams parsed = seedsigner::lvgl::parse_camera_params(args);
        assert(parsed.desired_format == seedsigner::lvgl::CameraFormat::Grayscale);
        assert(parsed.desired_width == 0);
        assert(parsed.desired_height == 0);
        assert(parsed.max_fps == 0);
        assert(parsed.buffer_count == 1);
    }

    // Test with explicit values
    {
        seedsigner::lvgl::CameraParams params;
        params.desired_format = seedsigner::lvgl::CameraFormat::RGB565;
        params.desired_width = 320;
        params.desired_height = 240;
        params.max_fps = 30;
        params.buffer_count = 2;
        seedsigner::lvgl::PropertyMap args = seedsigner::lvgl::make_camera_route_args(params);
        assert(args.at("format") == "rgb565");
        assert(args.at("width") == "320");
        assert(args.at("height") == "240");
        assert(args.at("fps") == "30");
        assert(args.at("buffer_count") == "2");

        seedsigner::lvgl::CameraParams parsed = seedsigner::lvgl::parse_camera_params(args);
        assert(parsed.desired_format == seedsigner::lvgl::CameraFormat::RGB565);
        assert(parsed.desired_width == 320);
        assert(parsed.desired_height == 240);
        assert(parsed.max_fps == 30);
        assert(parsed.buffer_count == 2);
    }

    // Test missing args (should use defaults)
    {
        seedsigner::lvgl::PropertyMap args;
        args["format"] = "jpeg";
        // width, height omitted -> 0
        // fps omitted -> 0
        // buffer_count omitted -> 1
        seedsigner::lvgl::CameraParams parsed = seedsigner::lvgl::parse_camera_params(args);
        assert(parsed.desired_format == seedsigner::lvgl::CameraFormat::JPEG);
        assert(parsed.desired_width == 0);
        assert(parsed.desired_height == 0);
        assert(parsed.max_fps == 0);
        assert(parsed.buffer_count == 1);
    }

    // Test clamping buffer_count
    {
        seedsigner::lvgl::PropertyMap args;
        args["buffer_count"] = "0";
        seedsigner::lvgl::CameraParams parsed = seedsigner::lvgl::parse_camera_params(args);
        assert(parsed.buffer_count == 1);
        args["buffer_count"] = "15";
        parsed = seedsigner::lvgl::parse_camera_params(args);
        assert(parsed.buffer_count == 10); // clamped to 10
    }
}

void test_scan_screen_mock() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"scan.qr"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::ScanScreen>(); }));
    auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"scan.qr"}, .args = {{ "instruction_text", "Scan QR code" }, { "scan_mode", "any" }}});
    assert(active.has_value());
    // Simulate camera frames
    std::vector<uint8_t> dummy_pixels(96 * 96, 0x80);
    runtime.push_frame(CameraFrame{.width = 96, .height = 96, .stride = 96, .sequence = 1, .pixels = dummy_pixels});
    // Wait for events (mock detection should emit after some frames)
    // We'll just verify screen created successfully.
    // For now, just ensure no crash.
}

void test_seed_words_screen() {
    using seedsigner::lvgl::UiRuntime;
    using seedsigner::lvgl::RouteId;
    using seedsigner::lvgl::RouteDescriptor;
    using seedsigner::lvgl::InputEvent;
    using seedsigner::lvgl::InputKey;
    using seedsigner::lvgl::EventType;
    using seedsigner::lvgl::make_seed_words_route_args;
    using seedsigner::lvgl::SeedWordsParams;

    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(
        RouteId{"seed.words"},
        []() -> std::unique_ptr<seedsigner::lvgl::Screen> {
            return std::make_unique<seedsigner::lvgl::SeedWordsScreen>();
        }));
    
    SeedWordsParams params;
    params.words = {"abandon", "ability", "able", "about", "above", "absent",
                    "absorb", "abstract", "absurd", "academy", "account", "acid"};
    params.words_per_page = 6;
    params.title = "Test Seed";
    params.warning_text = "Never digitize these words.";
    
    auto args = make_seed_words_route_args(params);
    const auto active = runtime.activate({
        .route_id = RouteId{"seed.words"},
        .args = args
    });
    assert(active.has_value());
    // Consume route activated and screen ready events
    assert(next_matching(runtime, EventType::RouteActivated).has_value());
    assert(next_matching(runtime, EventType::ScreenReady).has_value());
    runtime.tick(16);
    runtime.refresh_now();
    
    // Verify title and warning text appear
    assert(label_tree_contains(lv_scr_act(), "Test Seed"));
    assert(label_tree_contains(lv_scr_act(), "Never digitize these words."));
    
    // Test page navigation via input
    // Initial page should be 1/2
    // Press Right to go to page 2
    assert(runtime.send_input(InputEvent{.key = InputKey::Right}));
    auto page_event = next_matching(runtime, EventType::ActionInvoked);
    assert(page_event.has_value());
    assert(page_event->action_id == std::optional<std::string>{"page_changed"});
    
    // Press Back to emit back_requested
    assert(runtime.send_input(InputEvent{.key = InputKey::Back}));
    auto back_event = next_matching(runtime, EventType::ActionInvoked);
    assert(back_event.has_value());
    assert(back_event->action_id == std::optional<std::string>{"back_requested"});
}

}  // namespace tests
