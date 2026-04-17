// Minimal test for the C API binding layer.
// Exercises: create → init → activate menu → send input → read events → destroy

#include "seedsigner_lvgl/bindings/c_api.h"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

#include <cstdio>
#include <cstring>
#include <cassert>

using namespace seedsigner::lvgl;

// Register a simple menu route so ss_activate can resolve it.
static void register_test_routes(ss_runtime_t* rt) {
    // We need access to the underlying UiRuntime to register routes.
    // For the C API test, we'll use ss_activate with a route that the
    // internal registry doesn't know about — the activate will fail but
    // the API surface is exercised.  A full test would need a registration
    // function in the C API, which is planned for #95.
}

int main() {
    std::printf("[c_api_test] Creating runtime...\n");
    ss_runtime_t* rt = ss_create(240, 320);
    assert(rt != nullptr);

    std::printf("[c_api_test] Initializing...\n");
    bool ok = ss_init(rt);
    assert(ok);
    std::printf("[c_api_test] Init OK\n");

    // Activate a route (will fail because no routes registered, but tests the call)
    std::printf("[c_api_test] Activating route...\n");
    int ret = ss_activate(rt, "test.menu", "title=Test\nitems=a|Item A|chevron\nb|Item B|chevron");
    std::printf("[c_api_test] ss_activate returned %d (expected -1, no route registered)\n", ret);

    ss_active_route active = ss_get_active_route(rt);
    std::printf("[c_api_test] active.valid=%d\n", active.valid);

    ret = ss_navigate(rt, "back", nullptr, nullptr);
    std::printf("[c_api_test] ss_navigate(back) returned %d\n", ret);

    ret = ss_push_modal(rt, "confirm", "title=Test");
    std::printf("[c_api_test] ss_push_modal returned %d (expected unsupported=-2)\n", ret);

    // Send input
    std::printf("[c_api_test] Sending input...\n");
    ss_send_input(rt, SS_KEY_DOWN);
    ss_send_input(rt, SS_KEY_PRESS);
    ss_send_input(rt, SS_KEY_BACK);

    // Tick and refresh
    ss_tick(rt, 16);
    ss_refresh(rt);

    // Poll events
    std::printf("[c_api_test] Polling events...\n");
    int event_count = 0;
    for (int i = 0; i < 20; ++i) {
        ss_event ev = ss_next_event(rt);
        if (ev.type == SS_EVENT_NONE) break;
        event_count++;
        std::printf("[c_api_test] Event %d: type=%d action=%s component=%s\n",
                    i, ev.type,
                    ev.action_id ? ev.action_id : "(null)",
                    ev.component_id ? ev.component_id : "(null)");
    }
    std::printf("[c_api_test] Got %d events\n", event_count);

    // Push a dummy frame
    ss_camera_frame frame;
    uint8_t pixels[10 * 10];
    memset(pixels, 128, sizeof(pixels));
    frame.pixels = pixels;
    frame.width = 10;
    frame.height = 10;
    frame.stride = 10;
    frame.sequence = 1;
    ret = ss_push_frame(rt, &frame);
    std::printf("[c_api_test] ss_push_frame returned %d\n", ret);

    // Set screen data
    ret = ss_set_screen_data(rt, "title=Updated");
    std::printf("[c_api_test] ss_set_screen_data returned %d\n", ret);

    ret = ss_patch_screen_data(rt, "title=Patched");
    std::printf("[c_api_test] ss_patch_screen_data returned %d\n", ret);

    // Destroy
    std::printf("[c_api_test] Destroying runtime...\n");
    ss_destroy(rt);

    std::printf("[c_api_test] All assertions passed.\n");
    return 0;
}
