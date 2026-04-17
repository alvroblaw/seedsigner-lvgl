#pragma once

// ---------------------------------------------------------------------------
// seedsigner_lvgl C API — MicroPython-friendly ABI
//
// Pure C header exposing UiRuntime's command/event loop.  Designed to be
// wrapped by a MicroPython user module or consumed directly from C code.
//
// Ownership:
//   - All strings returned in ss_event are owned by the runtime and valid
//     only until the next ss_next_event() call.
//   - Strings passed TO the API are copied; caller retains ownership.
// ---------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Opaque handle ---
typedef struct ss_runtime ss_runtime_t;

// --- Input keys ---
typedef enum {
    SS_KEY_UP    = 0,
    SS_KEY_DOWN  = 1,
    SS_KEY_LEFT  = 2,
    SS_KEY_RIGHT = 3,
    SS_KEY_PRESS = 4,
    SS_KEY_BACK  = 5,
} ss_input_key;

// --- Event types ---
typedef enum {
    SS_EVENT_ROUTE_ACTIVATED = 0,
    SS_EVENT_SCREEN_READY    = 1,
    SS_EVENT_ACTION_INVOKED  = 2,
    SS_EVENT_CANCEL_REQUESTED = 3,
    SS_EVENT_NEEDS_DATA      = 4,
    SS_EVENT_ERROR           = 5,
    SS_EVENT_NONE            = 255,  // No event available
} ss_event_type;

// --- Event struct (C-friendly, no C++ types) ---
typedef struct {
    ss_event_type type;
    // Action/component fields (NULL if not present)
    const char* action_id;
    const char* component_id;
    // Value: tag + union
    int value_tag;  // 0=none, 1=bool, 2=int, 3=str
    union {
        bool bool_val;
        int64_t int_val;
        const char* str_val;
    } value;
    // Meta (key/value pair, NULL if not present)
    const char* meta_key;
    const char* meta_value_str;
    int64_t meta_value_int;
    int meta_value_tag;  // 0=none, 1=str, 2=int
    // Timestamp
    uint64_t timestamp_ms;
} ss_event;

// --- Camera frame ---
typedef struct {
    const uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t stride;  // 0 = width
    uint64_t sequence;
} ss_camera_frame;

// --- Active route descriptor ---
typedef struct {
    const char* route_id;
    uint32_t screen_token;
    uint32_t stack_depth;
    int valid;  // 0 = none, 1 = populated
} ss_active_route;

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

/// Create a runtime with the given display dimensions.
ss_runtime_t* ss_create(uint32_t width, uint32_t height);

/// Destroy runtime and free all resources.
void ss_destroy(ss_runtime_t* rt);

/// Initialize LVGL and display.  Must be called once after ss_create.
/// Returns true on success.
bool ss_init(ss_runtime_t* rt);

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

/// Activate a route.  route_id and optional args are copied.
/// args is a newline-separated list of key=value pairs, e.g. "title=Menu\nitems=a|b|c"
/// Returns 0 on success, -1 on error.
int ss_activate(ss_runtime_t* rt, const char* route_id, const char* args);

/// Replace the current route.
int ss_replace(ss_runtime_t* rt, const char* route_id, const char* args);

/// Generic navigation operation.
/// Supported actions today:
///   - "back"    -> injects SS_KEY_BACK
///   - "replace" -> equivalent to ss_replace(route_id, args)
/// Returns 0 on success, -1 on error, -2 when unsupported.
int ss_navigate(ss_runtime_t* rt, const char* action, const char* route_id, const char* args);

/// Return the current active route metadata.
ss_active_route ss_get_active_route(ss_runtime_t* rt);

/// Modal APIs are reserved for future runtime support.
int ss_push_modal(ss_runtime_t* rt, const char* modal_id, const char* data);
int ss_dismiss_modal(ss_runtime_t* rt, uint32_t modal_token, const char* result);

// ---------------------------------------------------------------------------
// Input & data
// ---------------------------------------------------------------------------

/// Send a hardware input event.
void ss_send_input(ss_runtime_t* rt, ss_input_key key);

/// Set full screen data (args format: newline-separated key=value).
int ss_set_screen_data(ss_runtime_t* rt, const char* data);

/// Patch a subset of screen data (args format: newline-separated key=value).
int ss_patch_screen_data(ss_runtime_t* rt, const char* patch);

/// Push a camera frame (grayscale pixels, copied).
int ss_push_frame(ss_runtime_t* rt, const ss_camera_frame* frame);

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

/// Poll for next event.  Returns SS_EVENT_NONE if queue is empty.
/// Returned string pointers are valid until next ss_next_event() call.
ss_event ss_next_event(ss_runtime_t* rt);

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

/// Advance LVGL tick by elapsed_ms milliseconds.
void ss_tick(ss_runtime_t* rt, uint32_t elapsed_ms);

/// Force a display refresh.
void ss_refresh(ss_runtime_t* rt);

#ifdef __cplusplus
}
#endif
