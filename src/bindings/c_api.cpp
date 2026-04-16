#include "seedsigner_lvgl/bindings/c_api.h"
#include "seedsigner_lvgl/runtime/UiRuntime.hpp"
#include "seedsigner_lvgl/runtime/Event.hpp"
#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/input/InputProfile.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace seedsigner::lvgl;

// ---------------------------------------------------------------------------
// Opaque runtime wrapper
// ---------------------------------------------------------------------------
struct ss_runtime {
    std::unique_ptr<UiRuntime> rt;
    // Cached event strings (valid until next ss_next_event)
    std::string cached_action_id;
    std::string cached_component_id;
    std::string cached_value_str;
    std::string cached_meta_key;
    std::string cached_meta_value_str;
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static PropertyMap parse_args(const char* args) {
    PropertyMap map;
    if (!args) return map;
    std::istringstream stream(args);
    std::string line;
    while (std::getline(stream, line)) {
        auto eq = line.find('=');
        if (eq != std::string::npos && eq > 0) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            map[key] = val;
        }
    }
    return map;
}

static ss_event_type to_c_type(EventType t) {
    switch (t) {
        case EventType::RouteActivated:  return SS_EVENT_ROUTE_ACTIVATED;
        case EventType::ScreenReady:     return SS_EVENT_SCREEN_READY;
        case EventType::ActionInvoked:   return SS_EVENT_ACTION_INVOKED;
        case EventType::CancelRequested: return SS_EVENT_CANCEL_REQUESTED;
        case EventType::NeedsData:       return SS_EVENT_NEEDS_DATA;
        case EventType::Error:           return SS_EVENT_ERROR;
    }
    return SS_EVENT_NONE;
}

static InputKey to_cpp_key(ss_input_key k) {
    switch (k) {
        case SS_KEY_UP:    return InputKey::Up;
        case SS_KEY_DOWN:  return InputKey::Down;
        case SS_KEY_LEFT:  return InputKey::Left;
        case SS_KEY_RIGHT: return InputKey::Right;
        case SS_KEY_PRESS: return InputKey::Press;
        case SS_KEY_BACK:  return InputKey::Back;
    }
    return InputKey::Press;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

ss_runtime_t* ss_create(uint32_t width, uint32_t height) {
    auto* r = new ss_runtime();
    r->rt = std::make_unique<UiRuntime>(RuntimeConfig{
        .width = width,
        .height = height,
        .skip_native_display = false,
    });
    return r;
}

void ss_destroy(ss_runtime_t* rt) {
    delete rt;
}

bool ss_init(ss_runtime_t* rt) {
    if (!rt || !rt->rt) return false;
    return rt->rt->init();
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

int ss_activate(ss_runtime_t* rt, const char* route_id, const char* args) {
    if (!rt || !rt->rt || !route_id) return -1;
    RouteDescriptor desc;
    desc.route_id = RouteId{std::string(route_id)};
    if (args) desc.args = parse_args(args);
    auto result = rt->rt->activate(desc);
    return result.has_value() ? 0 : -1;
}

int ss_replace(ss_runtime_t* rt, const char* route_id, const char* args) {
    if (!rt || !rt->rt || !route_id) return -1;
    RouteDescriptor desc;
    desc.route_id = RouteId{std::string(route_id)};
    if (args) desc.args = parse_args(args);
    auto result = rt->rt->replace(desc);
    return result.has_value() ? 0 : -1;
}

// ---------------------------------------------------------------------------
// Input & data
// ---------------------------------------------------------------------------

void ss_send_input(ss_runtime_t* rt, ss_input_key key) {
    if (!rt || !rt->rt) return;
    rt->rt->send_input(InputEvent{to_cpp_key(key)});
}

int ss_set_screen_data(ss_runtime_t* rt, const char* data) {
    if (!rt || !rt->rt || !data) return -1;
    auto map = parse_args(data);
    return rt->rt->set_screen_data(map) ? 0 : -1;
}

int ss_push_frame(ss_runtime_t* rt, const ss_camera_frame* frame) {
    if (!rt || !rt->rt || !frame) return -1;
    CameraFrame cf;
    cf.width = frame->width;
    cf.height = frame->height;
    cf.stride = frame->stride;
    cf.sequence = frame->sequence;
    // Copy pixel data
    uint32_t sz = frame->width * frame->height;
    cf.pixels.assign(frame->pixels, frame->pixels + sz);
    return rt->rt->push_frame(cf) ? 0 : -1;
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

ss_event ss_next_event(ss_runtime_t* rt) {
    ss_event ev{};
    ev.type = SS_EVENT_NONE;

    if (!rt || !rt->rt) return ev;

    auto ui_ev = rt->rt->next_event();
    if (!ui_ev.has_value()) return ev;

    ev.type = to_c_type(ui_ev->type);
    ev.timestamp_ms = ui_ev->timestamp_ms;

    // Action ID
    if (ui_ev->action_id.has_value()) {
        rt->cached_action_id = *ui_ev->action_id;
        ev.action_id = rt->cached_action_id.c_str();
    }

    // Component ID
    if (ui_ev->component_id.has_value()) {
        rt->cached_component_id = *ui_ev->component_id;
        ev.component_id = rt->cached_component_id.c_str();
    }

    // Value
    if (ui_ev->value.has_value()) {
        const auto& v = *ui_ev->value;
        if (std::holds_alternative<bool>(v)) {
            ev.value_tag = 1;
            ev.value.bool_val = std::get<bool>(v);
        } else if (std::holds_alternative<int64_t>(v)) {
            ev.value_tag = 2;
            ev.value.int_val = std::get<int64_t>(v);
        } else if (std::holds_alternative<std::string>(v)) {
            ev.value_tag = 3;
            rt->cached_value_str = std::get<std::string>(v);
            ev.value.str_val = rt->cached_value_str.c_str();
        }
    }

    // Meta
    if (ui_ev->meta.has_value()) {
        rt->cached_meta_key = ui_ev->meta->key;
        ev.meta_key = rt->cached_meta_key.c_str();

        const auto& mv = ui_ev->meta->value;
        if (std::holds_alternative<std::string>(mv)) {
            ev.meta_value_tag = 1;
            rt->cached_meta_value_str = std::get<std::string>(mv);
            ev.meta_value_str = rt->cached_meta_value_str.c_str();
        } else if (std::holds_alternative<int64_t>(mv)) {
            ev.meta_value_tag = 2;
            ev.meta_value_int = std::get<int64_t>(mv);
        } else if (std::holds_alternative<bool>(mv)) {
            ev.meta_value_tag = 2;
            ev.meta_value_int = std::get<bool>(mv) ? 1 : 0;
        }
    }

    return ev;
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void ss_tick(ss_runtime_t* rt, uint32_t elapsed_ms) {
    if (!rt || !rt->rt) return;
    rt->rt->tick(elapsed_ms);
}

void ss_refresh(ss_runtime_t* rt) {
    if (!rt || !rt->rt) return;
    rt->rt->refresh_now();
}
