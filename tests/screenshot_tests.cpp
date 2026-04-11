#include <cassert>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include <lvgl.h>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"
#include "seedsigner_lvgl/platform/FramebufferCapture.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/ResultScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsMenuScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"
#include "seedsigner_lvgl/screens/WarningScreen.hpp"
#include "seedsigner_lvgl/screens/ErrorScreen.hpp"
#include "seedsigner_lvgl/screens/DireWarningScreen.hpp"
#include "seedsigner_lvgl/screens/QRDisplayScreen.hpp"
#include "seedsigner_lvgl/screens/KeyboardScreen.hpp"
#include "seedsigner_lvgl/screens/SeedWordsScreen.hpp"
#include "seedsigner_lvgl/screens/PSBTOverviewScreen.hpp"
#include "seedsigner_lvgl/contracts/SettingsContract.hpp"
#include "seedsigner_lvgl/contracts/QRDisplayContract.hpp"
#include "seedsigner_lvgl/contracts/KeyboardContract.hpp"
#include "seedsigner_lvgl/contracts/SeedWordsContract.hpp"
#include "seedsigner_lvgl/contracts/PSBTOverviewContract.hpp"

using namespace seedsigner::lvgl;

static const char* OUT_DIR = "screenshots";

/// Each screen capture runs in a forked child to avoid LVGL state leakage.
static bool fork_capture(std::function<void(UiRuntime&)> setup, const std::string& name) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        UiRuntime rt;
        assert(rt.init());
        setup(rt);
        rt.tick(16);
        rt.refresh_now();
        while (rt.next_event()) {}
        rt.tick(16);
        rt.refresh_now();

        std::string path = std::string(OUT_DIR) + "/" + name + ".png";
        bool ok = FramebufferCapture::write_png(path, *rt.display());
        _exit(ok ? 0 : 1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
        std::printf("  %s %s\n", ok ? "[OK]" : "[FAIL]", name.c_str());
        return ok;
    }
    std::printf("  [FAIL] fork error for %s\n", name.c_str());
    return false;
}

int main() {
    mkdir(OUT_DIR, 0755);
    std::printf("=== SeedSigner-LVGL Screenshot Capture ===\n\n");

    // 1. Main Menu
    std::printf("[1/11] MenuListScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"menu.main"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<MenuListScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"menu.main"};
        rd.args = {{"title", "SeedSigner"}, {"items",
            "scan|Scan QR|Point camera at a QR code|chevron\n"
            "seeds|Seeds|Load or generate a seed|chevron\n"
            "tools|Tools|Signing tools and utilities|chevron\n"
            "settings|Settings|Device configuration|chevron\n"
            "power|Power Off|Shut down the device"}};
        rt.activate(rd);
    }, "01_main_menu");

    // 2. Settings Menu
    std::printf("[2/11] SettingsMenuScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"settings.menu"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<SettingsMenuScreen>();
        });
        auto args = make_settings_menu_route_args({
            SettingDefinition{.id = "locale", .title = "Settings", .subtitle = "Language",
                .value_type = SettingValueType::SingleChoice,
                .items = {{.id = "en", .label = "Language"}, {.id = "net", .label = "Network"}}},
            SettingDefinition{.id = "features", .title = "Settings", .subtitle = "Advanced features",
                .value_type = SettingValueType::MultiChoice,
                .items = {{.id = "dw", .label = "Dire Warnings"}}},
        });
        args["title"] = "Settings";
        args["selected_index"] = "0";
        RouteDescriptor rd;
        rd.route_id = RouteId{"settings.menu"};
        rd.args = args;
        rt.activate(rd);
    }, "02_settings_menu");

    // 3. Settings Selection
    std::printf("[3/11] SettingsSelectionScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"settings.locale"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<SettingsSelectionScreen>();
        });
        auto args = make_settings_route_args(SettingDefinition{
            .id = "locale", .title = "Settings", .subtitle = "Language",
            .section_title = "Display language",
            .help_text = "Choose one language for the active UI session.",
            .footer_text = "Press to apply. Back to cancel.",
            .value_type = SettingValueType::SingleChoice,
            .default_values = {"en"},
            .current_values = {"es"},
            .items = {
                {.id = "en", .label = "English", .secondary_text = "Default Latin font"},
                {.id = "es", .label = "Español", .secondary_text = "Accented glyphs"},
                {.id = "fr", .label = "Français", .secondary_text = "Wider Latin coverage"},
            }
        });
        args["selected_index"] = "1";
        RouteDescriptor rd;
        rd.route_id = RouteId{"settings.locale"};
        rd.args = args;
        rt.activate(rd);
    }, "03_settings_language");

    // 4. Warning
    std::printf("[4/11] WarningScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"warn"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<WarningScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"warn"};
        rd.args = {{"title", "Warning"}, {"body", "This seed was generated on this device.\nVerify the words carefully."}, {"button_text", "I Understand"}};
        rt.activate(rd);
    }, "04_warning");

    // 5. Error
    std::printf("[5/11] ErrorScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"err"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<ErrorScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"err"};
        rd.args = {{"title", "Scan Failed"}, {"body", "Could not decode QR code.\nTry again with better lighting."}, {"button_text", "Retry"}};
        rt.activate(rd);
    }, "05_error");

    // 6. Dire Warning
    std::printf("[6/11] DireWarningScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"dire"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<DireWarningScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"dire"};
        rd.args = {{"title", "DANGER"}, {"body", "You are about to sign a transaction\nthat sends ALL funds to a single address.\n\nThis cannot be undone."}, {"button_text", "I Accept the Risk"}};
        rt.activate(rd);
    }, "06_dire_warning");

    // 7. Result
    std::printf("[7/11] ResultScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"result"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<ResultScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"result"};
        rd.args = {{"title", "Signed Successfully"}, {"body", "Transaction signed and QR\ndisplayed for broadcast."}, {"continue_action", "done"}};
        rt.activate(rd);
    }, "07_result_success");

    // 8. QR Display
    std::printf("[8/11] QRDisplayScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"qr"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<QRDisplayScreen>();
        });
        QRDisplayParams qp;
        qp.qr_data = "bitcoin:bc1qexampleaddress123456789?amount=0.001";
        qp.title = "Signed PSBT";
        qp.brightness = 100;
        RouteDescriptor rd;
        rd.route_id = RouteId{"qr"};
        rd.args = make_qr_display_route_args(qp);
        rt.activate(rd);
    }, "08_qr_display");

    // 9. Keyboard
    std::printf("[9/11] KeyboardScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"kb"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<KeyboardScreen>();
        });
        KeyboardParams kp;
        kp.layout = KeyboardLayout::Lowercase;
        kp.placeholder = "Enter passphrase";
        kp.max_length = 64;
        RouteDescriptor rd;
        rd.route_id = RouteId{"kb"};
        rd.args = make_keyboard_route_args(kp);
        rt.activate(rd);
    }, "09_keyboard");

    // 10. Seed Words
    std::printf("[10/11] SeedWordsScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"seed"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<SeedWordsScreen>();
        });
        SeedWordsParams swp;
        swp.words = {"abandon","abandon","abandon","abandon","abandon","abandon",
                      "abandon","abandon","abandon","abandon","abandon","about"};
        swp.title = "Seed Phrase";
        swp.words_per_page = 6;
        RouteDescriptor rd;
        rd.route_id = RouteId{"seed"};
        rd.args = make_seed_words_route_args(swp);
        rt.activate(rd);
    }, "10_seed_words_page1");

    // 11. PSBT Overview
    std::printf("[11/11] PSBTOverviewScreen\n");
    fork_capture([](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"psbt"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<PSBTOverviewScreen>();
        });
        PSBTOverviewParams pp;
        pp.total_amount = "0.005 BTC";
        pp.fee_amount = "0.0001 BTC";
        pp.change_amount = "0.0049 BTC";
        pp.inputs_count = 2;
        pp.outputs_count = 2;
        pp.network = "mainnet";
        pp.recipient_label = "bc1qexample...";
        RouteDescriptor rd;
        rd.route_id = RouteId{"psbt"};
        rd.args = make_psbt_overview_route_args(pp);
        rt.activate(rd);
    }, "11_psbt_overview");

    std::printf("\n=== Done. Check %s/ for PNG files. ===\n", OUT_DIR);
    return 0;
}
