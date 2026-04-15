#include <cassert>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>
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
#include "seedsigner_lvgl/visual/DisplayProfile.hpp"

using namespace seedsigner::lvgl;
using namespace seedsigner::lvgl::profile;

// Resolve output directory relative to repo root (where this file's parent directory lives), falling back to CWD-relative "screenshots" if unavailable.
// This ensures correct output regardless of where the binary is invoked from.
#include <libgen.h>
static std::string _detect_repo_root() {
    // Try SOURCE_ROOT define (set by CMake) first
    #ifdef SOURCE_ROOT
    return std::string(SOURCE_ROOT);
    #else
    // Walk up from /proc/self/exe to find a marker file
    char exe[4096];
    ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (len > 0) {
        exe[len] = '\0';
        std::string path = exe;
        for (int i = 0; i < 5; ++i) {  // walk up at most 5 levels
            auto slash = path.rfind('/');
            if (slash == std::string::npos) break;
            path = path.substr(0, slash);
            struct stat st{};
            std::string marker = path + "/CMakeLists.txt";
            if (stat(marker.c_str(), &st) == 0) return path;
        }
    }
    return ".";  // fallback: CWD
    #endif
}
static const std::string REPO_ROOT = _detect_repo_root();

/// Profile descriptor for screenshot iteration.
struct ProfileInfo {
    ProfileId id;
    const char* dir_name;    // subdirectory under screenshots/
    uint32_t width;
    uint32_t height;
};

static const ProfileInfo PROFILES[] = {
    {ProfileId::Square240x240,  "square_240x240",  240, 240},
    {ProfileId::Portrait240x320, "portrait_240x320", 240, 320},
};
static constexpr int NUM_PROFILES = sizeof(PROFILES) / sizeof(PROFILES[0]);

/// Each screen capture runs in a forked child to avoid LVGL state leakage.
static bool fork_capture(std::function<void(UiRuntime&)> setup, const std::string& name, const std::string& out_dir, uint32_t w, uint32_t h) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        RuntimeConfig cfg;
        cfg.width = w;
        cfg.height = h;
        UiRuntime rt(std::move(cfg));
        assert(rt.init());
        setup(rt);
        rt.tick(16);
        rt.refresh_now();
        while (rt.next_event()) {}
        rt.tick(16);
        rt.refresh_now();

        std::string path = out_dir + "/" + name + ".png";
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

/// Run all 11 screen captures for one profile.
static int run_profile_captures(const ProfileInfo& pi) {
    std::string screenshots_root = REPO_ROOT + "/screenshots";
    std::string profile_dir = screenshots_root + "/" + pi.dir_name;
    mkdir(screenshots_root.c_str(), 0755);
    mkdir(profile_dir.c_str(), 0755);

    std::printf("\n--- Profile: %s (%ux%u) ---\n", pi.dir_name, pi.width, pi.height);
    int failures = 0;
    auto cap = [&](int n, const char* label, std::function<void(UiRuntime&)> setup, const char* name) {
        std::printf("[%d/11] %s\n", n, label);
        if (!fork_capture(setup, name, profile_dir, pi.width, pi.height)) ++failures;
    };

    // 1. Main Menu
    cap(1, "MenuListScreen", [](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"menu.main"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<MenuListScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"menu.main"};
        rd.args = {{"title", "SeedSigner"}, {"items",
            "---|Wallets\n"
            "scan|Scan QR|Point camera at a QR code|chevron\n"
            "seeds|Seeds|Load or generate a seed|chevron\n"
            "---|Utilities\n"
            "tools|Tools|Signing tools and utilities|chevron\n"
            "settings|Settings|Device configuration|chevron\n"
            "power|Power Off|Shut down the device"}};
        rt.activate(rd);
    }, "01_main_menu");

    // 2. Settings Menu
    cap(2, "SettingsMenuScreen", [](UiRuntime& rt) {
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
    cap(3, "SettingsSelectionScreen", [](UiRuntime& rt) {
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
    cap(4, "WarningScreen", [](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"warn"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<WarningScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"warn"};
        rd.args = {{"title", "Warning"}, {"body", "This seed was generated on this device.\nVerify the words carefully."}, {"button_text", "I Understand"}};
        rt.activate(rd);
    }, "04_warning");

    // 5. Error
    cap(5, "ErrorScreen", [](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"err"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<ErrorScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"err"};
        rd.args = {{"title", "Scan Failed"}, {"body", "Could not decode QR code.\nTry again with better lighting."}, {"button_text", "Retry"}};
        rt.activate(rd);
    }, "05_error");

    // 6. Dire Warning
    cap(6, "DireWarningScreen", [](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"dire"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<DireWarningScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"dire"};
        rd.args = {{"title", "DANGER"}, {"body", "You are about to sign a transaction\nthat sends ALL funds to a single address.\n\nThis cannot be undone."}, {"button_text", "I Accept the Risk"}};
        rt.activate(rd);
    }, "06_dire_warning");

    // 7. Result
    cap(7, "ResultScreen", [](UiRuntime& rt) {
        rt.screen_registry().register_route(RouteId{"result"}, []() -> std::unique_ptr<Screen> {
            return std::make_unique<ResultScreen>();
        });
        RouteDescriptor rd;
        rd.route_id = RouteId{"result"};
        rd.args = {{"title", "Signed Successfully"}, {"body", "Transaction signed and QR\ndisplayed for broadcast."}, {"continue_action", "done"}};
        rt.activate(rd);
    }, "07_result_success");

    // 8. QR Display
    cap(8, "QRDisplayScreen", [](UiRuntime& rt) {
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
    cap(9, "KeyboardScreen", [](UiRuntime& rt) {
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
    cap(10, "SeedWordsScreen", [](UiRuntime& rt) {
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
    cap(11, "PSBTOverviewScreen", [](UiRuntime& rt) {
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

    return failures;
}

int main() {
    std::printf("=== SeedSigner-LVGL Screenshot Capture (multi-profile) ===\n");
    int total_failures = 0;
    for (int i = 0; i < NUM_PROFILES; ++i) {
        total_failures += run_profile_captures(PROFILES[i]);
    }

    // Backward-compatible flat copy: symlink/copy the default profile (portrait) into screenshots/ root
    std::string screenshots_root = REPO_ROOT + "/screenshots";
    std::string default_dir = screenshots_root + "/portrait_240x320";
    for (auto& entry : std::filesystem::directory_iterator(default_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string dest = screenshots_root + "/" + entry.path().filename().string();
            // Remove old link/file if present
            std::remove(dest.c_str());
            std::filesystem::copy_file(entry.path(), dest);
        }
    }

    std::printf("\n=== Done. Check %s/ for PNG files (profile subdirs + flat backward-compat copy). ===\n", screenshots_root.c_str());
    return total_failures > 0 ? 1 : 0;
}
