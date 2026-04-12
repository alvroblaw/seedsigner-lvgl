#pragma once
// SdlDisplay — SDL2-backed display adapter for host desktop builds.
// Renders the LVGL framebuffer into an SDL2 window, maps keyboard input to
// seedsigner::lvgl::InputEvent values, and optionally drives an LVGL pointer
// indev from SDL mouse / touch events so click/tap interactions work on
// touch-oriented screens.
//
// This adapter follows the same pattern as HeadlessDisplay (lv_disp_drv_t
// registration with a flat framebuffer) but adds an SDL2 window for visual
// output, keyboard polling, and mouse/touch pointer support.  No external
// code was directly adapted; the approach is idiomatic LVGL+SDL2 integration
// (see LVGL docs on "Add a display" and "Add a mouse / touchpad").
// License: same as the parent project.

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <lvgl.h>

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

namespace seedsigner::lvgl {

struct InputEvent;  // forward — full def in seedsigner_lvgl/runtime/InputEvent.hpp

/// Optional callback invoked when the user requests a quit (window close /
/// Escape on the root screen).  Return true to confirm quit.
using QuitCallback = std::function<bool()>;

class SdlDisplay {
public:
    /// Construct an SDL window of the given LVGL resolution.
    /// @param pixel_scale  Each LVGL pixel is rendered as a pixel_scale×pixel_scale
    ///                     square in the host window (default 2×).
    explicit SdlDisplay(std::uint32_t width, std::uint32_t height, std::uint32_t pixel_scale = 2);
    ~SdlDisplay();

    SdlDisplay(const SdlDisplay&) = delete;
    SdlDisplay& operator=(const SdlDisplay&) = delete;

    /// Poll SDL events for up to @p timeout_ms.  Returns the first mapped
    /// InputEvent, or std::nullopt if no input arrived.
    std::optional<InputEvent> poll_input(std::uint32_t timeout_ms = 16);

    /// Drive the LVGL refresh cycle: call lv_timer_handler() + blit the
    /// framebuffer to the SDL window.
    void refresh();

    /// Direct access to the raw framebuffer (for debugging / screenshots).
    const std::vector<lv_color_t>& framebuffer() const noexcept { return framebuffer_; }

    std::uint32_t width()  const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::uint32_t flush_count() const noexcept { return flush_count_; }

    /// Install a callback that fires when the user closes the window or hits
    /// Escape with no screen to go back to.
    void set_quit_callback(QuitCallback cb);

    /// Returns true after the SDL window has been closed.
    bool should_quit() const noexcept { return quit_requested_; }

    /// Register an LVGL pointer input device driven by SDL mouse events.
    /// After calling this, feed SDL mouse/touch events via handle_mouse_event()
    /// or let poll_all_events() do it automatically.
    void enable_pointer();

    /// Returns true after enable_pointer() was called.
    bool pointer_enabled() const noexcept { return pointer_enabled_; }

    /// Feed a single SDL_Event into the internal pointer state when it is a
    /// mouse button or motion event.  Safe to call for any event type;
    /// non-mouse events are silently ignored.
    void handle_mouse_event(const SDL_Event& ev);

    /// Combined event pump: drain all pending SDL events, feed mouse events to
    /// the pointer indev, and return the first keyboard-derived InputEvent (if
    /// any).  Replaces manual SDL_PollEvent + poll_input loops.
    std::optional<InputEvent> poll_all_events();

    /// Switch the display to a new resolution at runtime.
    /// Destroys the current LVGL display driver and SDL window, then recreates
    /// both at the new dimensions.  The caller should re-activate the current
    /// route (or any route) after calling this.
    /// @return true on success.
    bool switch_resolution(std::uint32_t new_width, std::uint32_t new_height);

private:
    static void flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
    void blit_framebuffer();
    std::optional<InputEvent> map_sdl_event(const SDL_Event& ev);
    void create_sdl_window();

    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t pixel_scale_;
    std::uint32_t flush_count_{0};

    lv_disp_draw_buf_t draw_buffer_{};
    lv_disp_drv_t display_driver_{};
    lv_disp_t* display_{nullptr};  ///< LVGL display handle (needed for removal on profile switch)
    std::vector<lv_color_t> framebuffer_;

    // SDL state — opaque pointers; lifetime managed in .cpp via RAII.
    struct SdlState;
    std::unique_ptr<SdlState> sdl_;

    QuitCallback quit_callback_;
    bool quit_requested_{false};

    // --- Pointer (mouse/touch) indev ---
    bool pointer_enabled_{false};
    lv_indev_drv_t pointer_driver_{};
    lv_indev_t* pointer_device_{nullptr};

    // Latest pointer state (LVGL coordinates).
    lv_point_t pointer_pos_{0, 0};
    bool pointer_pressed_{false};

    static void pointer_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data);
};

}  // namespace seedsigner::lvgl
