#include <iostream>
#include <memory>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"

namespace {

std::unique_ptr<seedsigner::lvgl::Screen> make_menu() {
    return std::make_unique<seedsigner::lvgl::MenuListScreen>();
}

}  // namespace

int main() {
    seedsigner::lvgl::UiRuntime runtime;
    if (!runtime.init()) {
        std::cerr << "failed to initialize runtime\n";
        return 1;
    }

    runtime.screen_registry().register_route(seedsigner::lvgl::RouteId{"demo.menu"}, make_menu);
    const auto active = runtime.activate({
        .route_id = seedsigner::lvgl::RouteId{"demo.menu"},
        .args = {{"title", "SeedSigner LVGL"},
                 {"items", "scan|Scan QR\nsettings|Settings\npower|Power Off"},
                 {"selected_index", "0"}},
    });

    if (!active) {
        std::cerr << "failed to activate menu route\n";
        return 1;
    }

    runtime.tick(16);
    runtime.refresh_now();

    std::cout << "route=" << active->route_id.value() << " token=" << active->screen_token
              << " flushes=" << runtime.display()->flush_count() << "\n";
    return 0;
}
