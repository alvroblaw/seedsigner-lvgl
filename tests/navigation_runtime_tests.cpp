#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <lvgl.h>

#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace tests {
void test_headless_runtime_bootstrap();
}

namespace {

struct LifecycleLog {
    std::vector<std::string> entries;
};

class RecordingScreen : public seedsigner::lvgl::Screen {
public:
    explicit RecordingScreen(LifecycleLog& log) : log_(log) {}

    void create(const seedsigner::lvgl::ScreenContext& context, const seedsigner::lvgl::RouteDescriptor& route) override {
        assert(context.root != nullptr);
        log_.entries.push_back("create:" + route.route_id.value());
        log_.entries.push_back("token:" + std::to_string(context.screen_token));
        if (const auto it = route.args.find("title"); it != route.args.end()) {
            log_.entries.push_back("arg:title=" + it->second);
        }
    }

    void on_activate() override {
        log_.entries.push_back("activate");
    }

    void on_deactivate() override {
        log_.entries.push_back("deactivate");
    }

    void destroy() override {
        log_.entries.push_back("destroy");
    }

private:
    LifecycleLog& log_;
};

class EventEmittingScreen : public seedsigner::lvgl::Screen {
public:
    void create(const seedsigner::lvgl::ScreenContext& context, const seedsigner::lvgl::RouteDescriptor&) override {
        assert(context.root != nullptr);
        const bool emitted = context.emit_action(
            "confirm",
            std::string{"primary_button"},
            seedsigner::lvgl::EventValue{std::int64_t{1}},
            seedsigner::lvgl::EventMeta{"source", std::string{"screen"}});
        assert(emitted);
    }
};

void test_registry_and_activation() {
    LifecycleLog log;
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    const bool registered = runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"main_menu"},
        [&log]() { return std::make_unique<RecordingScreen>(log); });

    assert(registered);
    assert(runtime.screen_registry().has_route(seedsigner::lvgl::RouteId{"main_menu"}));

    const auto active = runtime.activate({
        .route_id = seedsigner::lvgl::RouteId{"main_menu"},
        .args = {{"title", "Main Menu"}},
    });

    assert(active.has_value());
    assert(active->route_id.value() == "main_menu");
    assert(active->screen_token == 1);
    assert(runtime.get_active_route()->route_id.value() == "main_menu");

    assert((log.entries == std::vector<std::string>{
        "create:main_menu",
        "token:1",
        "arg:title=Main Menu",
        "activate",
    }));
}

void test_replace_tears_down_previous_screen() {
    LifecycleLog first_log;
    LifecycleLog second_log;
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"main_menu"},
        [&first_log]() { return std::make_unique<RecordingScreen>(first_log); });
    runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"scan_qr"},
        [&second_log]() { return std::make_unique<RecordingScreen>(second_log); });

    const auto first = runtime.activate({.route_id = seedsigner::lvgl::RouteId{"main_menu"}});
    const auto second = runtime.replace({.route_id = seedsigner::lvgl::RouteId{"scan_qr"}});

    assert(first.has_value());
    assert(second.has_value());
    assert(first->screen_token == 1);
    assert(second->screen_token == 2);
    assert(runtime.get_active_route()->route_id.value() == "scan_qr");

    assert((first_log.entries == std::vector<std::string>{
        "create:main_menu",
        "token:1",
        "activate",
        "deactivate",
        "destroy",
    }));
    assert((second_log.entries == std::vector<std::string>{
        "create:scan_qr",
        "token:2",
        "activate",
    }));
}

void test_unknown_route_does_not_install_screen() {
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    const auto missing = runtime.activate({.route_id = seedsigner::lvgl::RouteId{"missing"}});

    assert(!missing.has_value());
    assert(!runtime.get_active_route().has_value());

    const auto error = runtime.next_event();
    assert(error.has_value());
    assert(error->type == seedsigner::lvgl::EventType::Error);
}

void test_failed_replace_keeps_existing_screen() {
    LifecycleLog log;
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"main_menu"},
        [&log]() { return std::make_unique<RecordingScreen>(log); });

    const auto first = runtime.activate({.route_id = seedsigner::lvgl::RouteId{"main_menu"}});
    const auto missing = runtime.replace({.route_id = seedsigner::lvgl::RouteId{"missing"}});

    assert(first.has_value());
    assert(!missing.has_value());
    assert(runtime.get_active_route()->route_id.value() == "main_menu");
    assert((log.entries == std::vector<std::string>{
        "create:main_menu",
        "token:1",
        "activate",
    }));
}

void test_screen_can_emit_outbound_action_event() {
    seedsigner::lvgl::UiRuntime runtime;
    assert(runtime.init());

    runtime.screen_registry().register_route(
        seedsigner::lvgl::RouteId{"emit_action"},
        []() { return std::make_unique<EventEmittingScreen>(); });

    const auto active = runtime.activate({.route_id = seedsigner::lvgl::RouteId{"emit_action"}});
    assert(active.has_value());

    const auto screen_action = runtime.next_event();
    assert(screen_action.has_value());
    assert(screen_action->type == seedsigner::lvgl::EventType::ActionInvoked);
    
    const auto route_activated = runtime.next_event();
    assert(route_activated.has_value());
    assert(route_activated->type == seedsigner::lvgl::EventType::RouteActivated);
    assert(screen_action->route_id.value() == "emit_action");
    assert(screen_action->screen_token == active->screen_token);
    assert(screen_action->component_id == std::optional<std::string>{"primary_button"});
    assert(screen_action->action_id == std::optional<std::string>{"confirm"});
    assert(std::get<std::int64_t>(*screen_action->value) == 1);
    assert(screen_action->meta.has_value());
    assert(screen_action->meta->key == "source");
    assert(std::get<std::string>(screen_action->meta->value) == "screen");

    const auto screen_ready = runtime.next_event();
    assert(screen_ready.has_value());
    assert(screen_ready->type == seedsigner::lvgl::EventType::ScreenReady);
}

void test_event_queue_overflow_keeps_fifo_order() {
    seedsigner::lvgl::EventQueue queue{2};

    assert(queue.push({.type = seedsigner::lvgl::EventType::RouteActivated,
                       .route_id = seedsigner::lvgl::RouteId{"one"},
                       .screen_token = 1,
                       .timestamp_ms = 10}));
    assert(queue.push({.type = seedsigner::lvgl::EventType::ScreenReady,
                       .route_id = seedsigner::lvgl::RouteId{"one"},
                       .screen_token = 1,
                       .timestamp_ms = 11}));
    assert(!queue.push({.type = seedsigner::lvgl::EventType::ActionInvoked,
                        .route_id = seedsigner::lvgl::RouteId{"one"},
                        .screen_token = 1,
                        .timestamp_ms = 12}));

    const auto first = queue.next();
    const auto second = queue.next();
    const auto empty = queue.next();

    assert(first.has_value());
    assert(first->type == seedsigner::lvgl::EventType::RouteActivated);
    assert(second.has_value());
    assert(second->type == seedsigner::lvgl::EventType::ScreenReady);
    assert(!empty.has_value());
}

}  // namespace

int main() {
    test_registry_and_activation();
    test_replace_tears_down_previous_screen();
    test_unknown_route_does_not_install_screen();
    test_failed_replace_keeps_existing_screen();
    test_screen_can_emit_outbound_action_event();
    test_event_queue_overflow_keeps_fifo_order();
    tests::test_headless_runtime_bootstrap();
    return 0;
}
