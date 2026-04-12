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
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>
#include <lvgl.h>

#include "seedsigner_lvgl/desktop/ScenarioRunner.hpp"

#include "seedsigner_lvgl/platform/SdlDisplay.hpp"
#include "seedsigner_lvgl/visual/DisplayProfile.hpp"
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

static std::string find_arg(int argc, char** argv, const std::string& flag) {
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] == flag) return argv[i + 1];
    }
    return {};
}

int main(int argc, char** argv) {
    const std::string scenario_path = find_arg(argc, argv, "--scenario");
    const bool interactive = (scenario_path.empty()) || (find_arg(argc, argv, "--interactive") == "true");

    std::printf("=== SeedSigner LVGL Interactive Desktop Demo ===\n");
    if (!scenario_path.empty()) {
        std::printf("Scenario mode: %s\n", scenario_path.c_str());
    }
    std::printf("Keys: arrows=nav  Enter=select  Esc=back/quit  1-5=switch screen  F2=switch profile\n");
    std::printf("Mouse: click/tap to interact with touch-oriented screens\n\n");

    // Available display profiles to cycle through with F2.
    struct ProfileDef { uint32_t w, h; const char* name; };
    static const ProfileDef kProfiles[] = {
        {240, 320, "Portrait 240×320"},
        {240, 240, "Square 240×240"},
    };
    int profile_idx = 0;

    constexpr uint32_t Scale = 2;

    // ---- Scenario mode (headless) ----
    if (!scenario_path.empty()) {
        // Run scenario in headless mode (no SDL window) so screenshots work.
        UiRuntime headless_rt(RuntimeConfig{.width = kProfiles[0].w, .height = kProfiles[0].h});
        headless_rt.init();
        register_routes(headless_rt.screen_registry());

        auto res = ScenarioRunner::load_and_run(scenario_path, headless_rt,
                                                 kProfiles[0].w, kProfiles[0].h);
        if (!res.ok) {
            std::fprintf(stderr, "Scenario failed after %d steps: %s\n",
                         res.steps_run, res.error.c_str());
            return 1;
        }
        std::printf("Scenario completed: %d steps\n", res.steps_run);
        if (!interactive) return 0;
        std::printf("Dropping into interactive mode...\n\n");
    }
    // ---- End scenario mode ----

    struct ActiveSession {
        std::unique_ptr<SdlDisplay> sdl;
        std::unique_ptr<UiRuntime> runtime;
        std::function<void(const DemoScreen&)> go;
    };

    auto make_session = [&](int idx) -> ActiveSession {
        const auto& p = kProfiles[idx];
        lv_init();

        auto sdl = std::make_unique<SdlDisplay>(p.w, p.h, Scale);
        sdl->enable_pointer();

        auto runtime = std::make_unique<UiRuntime>(RuntimeConfig{
            .width = p.w,
            .height = p.h,
            .skip_native_display = true,
        });
        runtime->init();
        register_routes(runtime->screen_registry());

        auto go = [runtime_ptr = runtime.get()](const DemoScreen& ds) {
            runtime_ptr->activate({.route_id = RouteId{ds.route}, .args = ds.args});
            while (runtime_ptr->next_event()) {}
        };

        return ActiveSession{
            .sdl = std::move(sdl),
            .runtime = std::move(runtime),
            .go = std::move(go),
        };
    };

    auto session = make_session(profile_idx);
    session.go(kDemoScreens[0]);

    std::printf("Screens:\n");
    for (const auto& ds : kDemoScreens) std::printf("  [%d] %s\n", ds.key, ds.label);
    std::printf("\n");

    bool running = true;
    while (running && !session.sdl->should_quit()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }

            session.sdl->handle_mouse_event(ev);

            if (ev.type != SDL_KEYDOWN) continue;
            const SDL_Keycode sym = ev.key.keysym.sym;

            bool switched = false;
            for (const auto& ds : kDemoScreens) {
                if (sym == SDLK_0 + ds.key) {
                    std::printf("[switch] → %s\n", ds.label);
                    session.go(ds);
                    switched = true;
                    break;
                }
            }
            if (switched) continue;

            // Full teardown + reinit is safer than hot-swapping LVGL's
            // display driver, which left stale state and caused corruption
            // after subsequent input interaction.
            if (sym == SDLK_F2) {
                profile_idx = (profile_idx + 1) % 2;
                const auto& p = kProfiles[profile_idx];
                std::printf("[profile] full reinit → %s\n", p.name);

                session.go = {};
                session.runtime.reset();
                session.sdl.reset();
                session = make_session(profile_idx);
                session.go(kDemoScreens[0]);
                continue;
            }

            if (auto input = map_key(sym)) {
                session.runtime->send_input(*input);
                while (auto uev = session.runtime->next_event()) {
                    if (uev->type == EventType::CancelRequested) {
                        std::printf("[back] returning to menu\n");
                        session.go(kDemoScreens[0]);
                    } else if (uev->type == EventType::ActionInvoked && uev->meta) {
                        std::printf("[action] key=%s\n", uev->meta->key.c_str());
                    }
                }
            }
        }

        session.runtime->tick(8);
        session.sdl->refresh();
        SDL_Delay(8);
    }

    std::printf("Exiting.\n");
    return 0;
}
