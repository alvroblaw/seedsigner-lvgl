#include <cassert>
#include <memory>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"

namespace tests {

namespace {

std::unique_ptr<seedsigner::lvgl::Screen> make_placeholder() {
    return std::make_unique<seedsigner::lvgl::PlaceholderScreen>();
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

}  // namespace tests
