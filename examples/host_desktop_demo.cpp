// host_desktop_demo — Interactive SDL2 desktop runner for SeedSigner LVGL UI.
//
// Key mapping:
//   ↑/↓/←/→   — D-pad navigation
//   Enter      — Select / confirm
//   Escape     — Back / quit (quits when pressed on root screen)
//   1–5        — Switch between demo screens
//   Mouse      — Click/tap to interact with touch-oriented screens
//
// Build:
//   cmake -S . -B build-desktop -DBUILD_HOST_DESKTOP=ON
//   cmake --build build-desktop --target host_desktop_demo
//   ./build-desktop/host_desktop_demo

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>
#include <lvgl.h>

#include "seedsigner_lvgl/platform/SdlDisplay.hpp"
#include "seedsigner_lvgl/contracts/SettingsContract.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/runtime/Event.hpp"
#include "seedsigner_lvgl/runtime/InputEvent.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"
#include "seedsigner_lvgl/screens/ErrorScreen.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"
#include "seedsigner_lvgl/screens/WarningScreen.hpp"

using namespace seedsigner::lvgl;

struct DemoScreen {
    int key;
    const char* route;
    PropertyMap args;
    const char* label;
};

static const std::vector<DemoScreen> kDemoScreens = {
    {1, "demo.menu",     {{"title", "Main Menu"},
                          {"items", "settings|Settings|Configure the device|chevron\n"
                                    "scan|Scan QR|Open camera to scan a QR code|chevron\n"
                                    "tools|Tools|Seed tools and utilities|chevron\n"
                                    "info|About|Device info & version|chevron"}}, "Menu List"},
    {2, "demo.settings", make_settings_route_args(SettingDefinition{
                              .id = "demo.lang",
                              .title = "Settings",
                              .subtitle = "Language",
                              .section_title = "Display language",
                              .help_text = "Pick a language.",
                              .footer_text = "Press to apply.",
                              .value_type = SettingValueType::SingleChoice,
                              .default_values = {"en"},
                              .current_values = {"en"},
                              .items = {{.id = "en", .label = "English"},
                                        {.id = "es", .label = "Español"},
                                        {.id = "fr", .label = "Français"}}}),
                                                                           "Settings Selection"},
    {3, "demo.warning",  {{"title", "Warning"},
                          {"body", "This action cannot be undone. Are you sure?"}}, "Warning"},
    {4, "demo.error",    {{"title", "Error"},
                          {"body", "Something went wrong. Please try again."}}, "Error"},
    {5, "demo.result",   {{"title", "Success"},
                          {"body", "Operation completed. Seed verified."}}, "Result"},
};

static void register_routes(ScreenRegistry& reg) {
    reg.register_route(RouteId{"demo.menu"},     []() -> std::unique_ptr<Screen> { return std::make_unique<MenuListScreen>(); });
    reg.register_route(RouteId{"demo.settings"}, []() -> std::unique_ptr<Screen> { return std::make_unique<SettingsSelectionScreen>(); });
    reg.register_route(RouteId{"demo.warning"},  []() -> std::unique_ptr<Screen> { return std::make_unique<WarningScreen>(); });
    reg.register_route(RouteId{"demo.error"},    []() -> std::unique_ptr<Screen> { return std::make_unique<ErrorScreen>(); });
    reg.register_route(RouteId{"demo.result"},   []() -> std::unique_ptr<Screen> { return std::make_unique<ResultScreen>(); });
}

static std::optional<InputEvent> map_key(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_UP:       return InputEvent{InputKey::Up};
        case SDLK_DOWN:     return InputEvent{InputKey::Down};
        case SDLK_LEFT:     return InputEvent{InputKey::Left};
        case SDLK_RIGHT:    return InputEvent{InputKey::Right};
        case SDLK_RETURN:
        case SDLK_KP_ENTER: return InputEvent{InputKey::Press};
        case SDLK_ESCAPE:   return InputEvent{InputKey::Back};
        default:            return std::nullopt;
    }
}

int main() {
    std::printf("=== SeedSigner LVGL Interactive Desktop Demo ===\n");
    std::printf("Keys: arrows=nav  Enter=select  Esc=back/quit  1-5=switch screen\n");
    std::printf("Mouse: click/tap to interact with touch-oriented screens\n\n");

    constexpr uint32_t W = 240, H = 320, Scale = 2;

    lv_init();
    auto sdl = std::make_unique<SdlDisplay>(W, H, Scale);
    sdl->enable_pointer();

    UiRuntime runtime(RuntimeConfig{.width = W, .height = H, .skip_native_display = true});
    runtime.init();

    register_routes(runtime.screen_registry());

    auto go = [&](const DemoScreen& ds) {
        runtime.activate({.route_id = RouteId{ds.route}, .args = ds.args});
        while (runtime.next_event()) {}
    };
    go(kDemoScreens[0]);

    std::printf("Screens:\n");
    for (const auto& ds : kDemoScreens) std::printf("  [%d] %s\n", ds.key, ds.label);
    std::printf("\n");

    bool running = true;
    while (running && !sdl->should_quit()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }

            // Feed mouse/touch events to LVGL pointer indev.
            sdl->handle_mouse_event(ev);

            if (ev.type != SDL_KEYDOWN) continue;
            const SDL_Keycode sym = ev.key.keysym.sym;

            // Number keys → screen switch
            bool switched = false;
            for (const auto& ds : kDemoScreens) {
                if (sym == SDLK_0 + ds.key) {
                    std::printf("[switch] → %s\n", ds.label);
                    go(ds);
                    switched = true;
                    break;
                }
            }
            if (switched) continue;

            // Navigation keys → runtime input
            if (auto input = map_key(sym)) {
                runtime.send_input(*input);
                while (auto uev = runtime.next_event()) {
                    if (uev->type == EventType::CancelRequested) {
                        std::printf("[back] returning to menu\n");
                        go(kDemoScreens[0]);
                    } else if (uev->type == EventType::ActionInvoked && uev->meta) {
                        std::printf("[action] key=%s\n", uev->meta->key.c_str());
                    }
                }
            }
        }

        runtime.tick(8);
        sdl->refresh();

        // Small sleep to avoid busy-loop (~60 fps).
        SDL_Delay(8);
    }

    std::printf("Exiting.\n");
    return 0;
}
