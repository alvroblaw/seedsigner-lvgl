#include <iostream>
#include <memory>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"

namespace {

std::unique_ptr<seedsigner::lvgl::Screen> make_placeholder() {
    return std::make_unique<seedsigner::lvgl::PlaceholderScreen>();
}

}  // namespace

int main() {
    seedsigner::lvgl::UiRuntime runtime;
    if (!runtime.init()) {
        std::cerr << "failed to initialize runtime\n";
        return 1;
    }

    runtime.screen_registry().register_route(seedsigner::lvgl::RouteId{"demo.placeholder"}, make_placeholder);
    const auto active = runtime.activate({
        .route_id = seedsigner::lvgl::RouteId{"demo.placeholder"},
        .args = {{"title", "SeedSigner LVGL"}, {"body", "Headless host demo rendered through LVGL."}},
    });

    if (!active) {
        std::cerr << "failed to activate placeholder route\n";
        return 1;
    }

    runtime.tick(16);
    runtime.refresh_now();

    std::cout << "route=" << active->route_id.value() << " token=" << active->screen_token
              << " flushes=" << runtime.display()->flush_count() << "\n";
    return 0;
}
