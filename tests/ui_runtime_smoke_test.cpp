#include <cassert>
#include <memory>
#include <optional>
#include <string>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"

namespace tests {

namespace {

std::unique_ptr<seedsigner::lvgl::Screen> make_placeholder() {
    return std::make_unique<seedsigner::lvgl::PlaceholderScreen>();
}

std::unique_ptr<seedsigner::lvgl::Screen> make_menu() {
    return std::make_unique<seedsigner::lvgl::MenuListScreen>();
}

std::optional<std::int64_t> event_index(const seedsigner::lvgl::UiEvent& event) {
    if (!event.value.has_value()) {
        return std::nullopt;
    }
    return std::get<std::int64_t>(*event.value);
}

}  // namespace

void test_headless_runtime_bootstrap() {
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    const bool registered = runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"demo.placeholder"}, make_placeholder);
    assert(registered);

    const auto active = runtime.activate({
        .route_id = seedsigner::lvgl::RouteId{"demo.placeholder"},
        .args = {{"title", "Smoke Test"}, {"body", "LVGL flush should happen."}},
    });
    assert(active.has_value());

    runtime.tick(16);
    runtime.refresh_now();

    assert(runtime.display() != nullptr);
    assert(runtime.display()->flush_count() > 0);

    const auto route_activated = runtime.next_event();
    assert(route_activated.has_value());
    assert(route_activated->type == seedsigner::lvgl::EventType::RouteActivated);
    assert(route_activated->screen_token == active->screen_token);

    const auto screen_ready = runtime.next_event();
    assert(screen_ready.has_value());
    assert(screen_ready->type == seedsigner::lvgl::EventType::ScreenReady);
}

void test_menu_screen_navigation_events() {
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    const bool registered = runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"demo.menu"}, make_menu);
    assert(registered);

    const auto active = runtime.activate({
        .route_id = seedsigner::lvgl::RouteId{"demo.menu"},
        .args = {{"title", "SeedSigner"},
                 {"subtitle", "Choose an action to continue"},
                 {"top_nav_label", "POWER"},
                 {"items",
                  "scan|Scan|Scan QR codes\nseeds|Seeds|Manage loaded seeds\nsettings|Settings|Device preferences\npower|Power Off|Shutdown menu|true"},
                 {"selected_index", "1"}},
    });
    assert(active.has_value());

    assert(runtime.send_input({.key = seedsigner::lvgl::InputKey::Down}));
    auto event = runtime.next_event();
    assert(event.has_value());
    assert(event->type == seedsigner::lvgl::EventType::RouteActivated);
    event = runtime.next_event();
    assert(event.has_value());
    assert(event->type == seedsigner::lvgl::EventType::ScreenReady);

    event = runtime.next_event();
    assert(event.has_value());
    assert(event->type == seedsigner::lvgl::EventType::ActionInvoked);
    assert(event->action_id == std::optional<std::string>{"focus_changed"});
    assert(event->meta.has_value());
    assert(event->meta->key == "settings");
    assert(event_index(*event) == 2);

    assert(runtime.send_input({.key = seedsigner::lvgl::InputKey::Press}));
    event = runtime.next_event();
    assert(event.has_value());
    assert(event->type == seedsigner::lvgl::EventType::ActionInvoked);
    assert(event->action_id == std::optional<std::string>{"item_selected"});
    assert(event->meta.has_value());
    assert(event->meta->key == "settings");
    assert(event_index(*event) == 2);

    assert(runtime.send_input({.key = seedsigner::lvgl::InputKey::Back}));
    event = runtime.next_event();
    assert(event.has_value());
    assert(event->type == seedsigner::lvgl::EventType::CancelRequested);
    assert(event->component_id == std::optional<std::string>{"menu_list"});
}

}  // namespace tests
