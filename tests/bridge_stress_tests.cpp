#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace seedsigner::lvgl;

namespace {

class StressFrameSinkScreen : public Screen {
public:
    void create(const ScreenContext&, const RouteDescriptor&) override {}
    bool push_frame(const CameraFrame& frame) override {
        frames_seen_++;
        last_sequence_ = frame.sequence;
        last_bytes_ = frame.pixels.size();
        return true;
    }

private:
    std::size_t frames_seen_{0};
    std::uint64_t last_sequence_{0};
    std::size_t last_bytes_{0};
};

} // namespace

static void register_test_routes(ScreenRegistry& reg) {
    reg.register_route(RouteId{"stress.menu"}, []() -> std::unique_ptr<Screen> {
        return std::make_unique<MenuListScreen>();
    });
    reg.register_route(RouteId{"stress.scan"}, []() -> std::unique_ptr<Screen> {
        return std::make_unique<StressFrameSinkScreen>();
    });
    reg.register_route(RouteId{"stress.settings"}, []() -> std::unique_ptr<Screen> {
        return std::make_unique<SettingsSelectionScreen>();
    });
}

static int drain_events(UiRuntime& rt) {
    int count = 0;
    while (rt.next_event().has_value()) {
        ++count;
    }
    return count;
}

static void test_event_queue_overflow() {
    std::printf("[bridge_stress] test_event_queue_overflow...\n");
    UiRuntime rt(RuntimeConfig{.width = 240, .height = 320, .event_queue_capacity = 4});
    assert(rt.init());
    register_test_routes(rt.screen_registry());

    RouteDescriptor rd;
    rd.route_id = RouteId{"stress.menu"};
    rd.args = {{"title", "Stress"}, {"items", "a|A\nb|B\nc|C\nd|D"}};
    auto active = rt.activate(rd);
    assert(active.has_value());

    // Generate more events than capacity by hammering navigation input.
    for (int i = 0; i < 20; ++i) {
        rt.send_input(InputEvent{InputKey::Down});
    }

    int drained = drain_events(rt);
    // Queue is bounded. We should not see corruption or crashes.
    assert(drained <= 4);
    std::printf("  drained=%d (capacity=4)\n", drained);
}

static void test_rapid_navigation_cycles() {
    std::printf("[bridge_stress] test_rapid_navigation_cycles...\n");
    UiRuntime rt(RuntimeConfig{.width = 240, .height = 320, .event_queue_capacity = 32});
    assert(rt.init());
    register_test_routes(rt.screen_registry());

    RouteDescriptor menu{RouteId{"stress.menu"}, {{"title", "Menu"}, {"items", "a|A\nb|B"}}};
    RouteDescriptor settings{RouteId{"stress.settings"}, {{"title", "Settings"}, {"items", "lang|Language\nnet|Network"}}};

    for (int i = 0; i < 25; ++i) {
        auto r1 = rt.activate(menu);
        assert(r1.has_value());
        auto r2 = rt.replace(settings);
        assert(r2.has_value());
        auto r3 = rt.replace(menu);
        assert(r3.has_value());
        rt.tick(1);
    }

    auto active = rt.get_active_route();
    assert(active.has_value());
    assert(active->route_id == RouteId{"stress.menu"});
    drain_events(rt);
    std::printf("  25 navigation cycles OK\n");
}

static void test_concurrent_frames_while_navigating() {
    std::printf("[bridge_stress] test_concurrent_frames_while_navigating...\n");
    UiRuntime rt(RuntimeConfig{.width = 240, .height = 320, .event_queue_capacity = 32});
    assert(rt.init());
    register_test_routes(rt.screen_registry());

    RouteDescriptor scan{RouteId{"stress.scan"}, {{"instruction_text", "Scan"}, {"mock_mode", "true"}}};
    RouteDescriptor menu{RouteId{"stress.menu"}, {{"title", "Menu"}, {"items", "a|A\nb|B"}}};

    std::vector<uint8_t> pixels(64 * 64, 127);
    CameraFrame frame;
    frame.width = 64;
    frame.height = 64;
    frame.stride = 64;
    frame.sequence = 1;
    frame.pixels = pixels;

    for (int i = 0; i < 25; ++i) {
        auto r = rt.activate(scan);
        assert(r.has_value());
        bool pushed = rt.push_frame(frame);
        assert(pushed);
        auto r2 = rt.replace(menu);
        assert(r2.has_value());
        rt.tick(1);
    }

    drain_events(rt);
    std::printf("  25 frame injection + navigate away/back cycles OK\n");
}

static void test_large_payload_screen_data() {
    std::printf("[bridge_stress] test_large_payload_screen_data...\n");
    UiRuntime rt(RuntimeConfig{.width = 240, .height = 320, .event_queue_capacity = 16});
    assert(rt.init());
    register_test_routes(rt.screen_registry());

    RouteDescriptor rd{RouteId{"stress.settings"}, {{"title", "Settings"}, {"items", "lang|Language\nnet|Network"}}};
    auto active = rt.activate(rd);
    assert(active.has_value());

    std::string huge(4096, 'x');
    PropertyMap full{{"title", huge}, {"items", "lang|Language\nnet|Network"}};
    bool ok = rt.set_screen_data(full);
    assert(ok);

    drain_events(rt);
    std::printf("  4KB set_screen_data payload OK\n");
}

int main() {
    test_event_queue_overflow();
    test_rapid_navigation_cycles();
    test_concurrent_frames_while_navigating();
    test_large_payload_screen_data();
    std::printf("[bridge_stress] all tests passed\n");
    return 0;
}
