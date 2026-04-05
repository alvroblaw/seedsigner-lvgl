#include <iostream>
#include <memory>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"

namespace {

std::unique_ptr<seedsigner::lvgl::Screen> make_menu() {
    return std::make_unique<seedsigner::lvgl::MenuListScreen>();
}

void drain_events(seedsigner::lvgl::UiRuntime& runtime) {
    while (const auto event = runtime.next_event()) {
        std::cout << "event=" << static_cast<int>(event->type)
                  << " route=" << event->route_id.value()
                  << " token=" << event->screen_token;

        if (event->component_id) {
            std::cout << " component=" << *event->component_id;
        }
        if (event->action_id) {
            std::cout << " action=" << *event->action_id;
        }
        if (event->meta) {
            std::cout << " meta.key=" << event->meta->key;
        }
        std::cout << '\n';
    }
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
        .args = {{"title", "SeedSigner"},
                 {"subtitle", "Host demo: SeedSigner-style list progression"},
                 {"top_nav_label", "POWER"},
                 {"items",
                  "scan|Scan|Scan QR codes\nseeds|Seeds|Load or review active seeds\ntools|Tools|Entropy and explorer tools\nsettings|Settings|Device preferences\npower|Power Off|Shutdown menu|true"},
                 {"selected_index", "0"}},
    });

    if (!active) {
        std::cerr << "failed to activate menu route\n";
        return 1;
    }

    runtime.tick(16);
    runtime.refresh_now();
    std::cout << "flushes=" << runtime.display()->flush_count() << '\n';
    drain_events(runtime);

    runtime.send_input({.key = seedsigner::lvgl::InputKey::Down});
    runtime.send_input({.key = seedsigner::lvgl::InputKey::Down});
    runtime.send_input({.key = seedsigner::lvgl::InputKey::Press});
    runtime.send_input({.key = seedsigner::lvgl::InputKey::Back});
    drain_events(runtime);

    return 0;
}
