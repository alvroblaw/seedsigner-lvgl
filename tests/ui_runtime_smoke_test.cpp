#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"

namespace tests {

namespace {
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

void test_external_scan_flow_demo() {
    UiRuntime runtime;
    assert(runtime.init());
    assert(runtime.screen_registry().register_route(RouteId{"demo.menu"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::MenuListScreen>(); }));
    assert(runtime.screen_registry().register_route(RouteId{"demo.result"}, []() -> std::unique_ptr<Screen> { return std::make_unique<seedsigner::lvgl::ResultScreen>(); }));

    auto active = runtime.activate(RouteDescriptor{.route_id = RouteId{"demo.menu"}, .args = {{"title", "Main Menu"}, {"items", "scan|Scan QR\nback|Back"}}});
    assert(active.has_value());
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto menu_action = next_matching(runtime, EventType::ActionInvoked);
    assert(menu_action.has_value() && menu_action->action_id == std::optional<std::string>{"item_selected"});
    assert(menu_action->meta.has_value() && menu_action->meta->key == "scan");

    active = runtime.activate(RouteDescriptor{.route_id = RouteId{"demo.result"}, .args = {{"title", "Menu Result"}, {"body", "Selected Scan QR from reusable menu."}, {"continue_action", "done"}}});
    assert(active.has_value());
    assert(runtime.send_input(InputEvent{.key = InputKey::Press}));
    auto result_action = next_matching(runtime, EventType::ActionInvoked);
    assert(result_action.has_value() && result_action->action_id == std::optional<std::string>{"done"});

    runtime.tick(16);
    runtime.refresh_now();
    assert(runtime.display()->flush_count() > 0);
}

}  // namespace tests
