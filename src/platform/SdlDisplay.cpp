// SdlDisplay — SDL2-backed display adapter for host desktop builds.
// See header for attribution / license notes.

#include "seedsigner_lvgl/platform/SdlDisplay.hpp"
#include "seedsigner_lvgl/runtime/InputEvent.hpp"

#include <cstdio>
#include <cstring>

// SDL2 headers — keep after project headers to avoid macro collisions.
#include <SDL.h>

namespace seedsigner::lvgl {

// -------------------------------------------------------------------------- //
// RAII wrapper for SDL state so SdlDisplay stays move-friendly internally.
// -------------------------------------------------------------------------- //
struct SdlDisplay::SdlState {
    SDL_Window*   window   {nullptr};
    SDL_Renderer* renderer {nullptr};

    SdlState() = default;
    ~SdlState() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window)   SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

// -------------------------------------------------------------------------- //
// Construction / destruction
// -------------------------------------------------------------------------- //

SdlDisplay::SdlDisplay(std::uint32_t width, std::uint32_t height, std::uint32_t pixel_scale)
    : width_(width)
    , height_(height)
    , pixel_scale_(pixel_scale)
    , draw_buf_(static_cast<std::size_t>(width) * 40)          // 40-row LVGL render scratch
    , framebuffer_(static_cast<std::size_t>(width) * height)
    , argb_buffer_(static_cast<std::size_t>(width) * height)
    , sdl_(std::make_unique<SdlState>())
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "[SdlDisplay] SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    create_sdl_window();

    // Use a small render scratch for LVGL — flush_cb copies partial areas
    // into the correct position in the full framebuffer.
    lv_disp_draw_buf_init(&draw_buffer_, draw_buf_.data(), nullptr,
                          static_cast<uint32_t>(draw_buf_.size()));
    lv_disp_drv_init(&display_driver_);
    display_driver_.hor_res  = width_;
    display_driver_.ver_res  = height_;
    display_driver_.flush_cb = flush_cb;
    display_driver_.draw_buf = &draw_buffer_;
    display_driver_.user_data = this;
    display_ = lv_disp_drv_register(&display_driver_);
}

SdlDisplay::~SdlDisplay() = default;

// -------------------------------------------------------------------------- //
// LVGL flush callback
// -------------------------------------------------------------------------- //

void SdlDisplay::flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
    auto* self = static_cast<SdlDisplay*>(disp_drv->user_data);
    if (self) {
        ++self->flush_count_;

        // Copy the partial render from LVGL's linear scratch buffer into the
        // correct position in the full framebuffer.
        const int area_w = area->x2 - area->x1 + 1;
        for (int y = area->y1; y <= area->y2; ++y) {
            std::memcpy(&self->framebuffer_[static_cast<std::size_t>(y) * self->width_ + area->x1],
                        color_p,
                        static_cast<std::size_t>(area_w) * sizeof(lv_color_t));
            color_p += area_w;
        }
        self->blit_framebuffer();
    }
    lv_disp_flush_ready(disp_drv);
}

// -------------------------------------------------------------------------- //
// Framebuffer → SDL blit
// -------------------------------------------------------------------------- //

void SdlDisplay::blit_framebuffer() {
    if (!sdl_->renderer) return;

    // Lazily create (or recreate after resolution switch) the persistent texture.
    if (!texture_) {
        texture_ = SDL_CreateTexture(
            sdl_->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            static_cast<int>(width_),
            static_cast<int>(height_));
        if (!texture_) {
            std::fprintf(stderr, "[SdlDisplay] SDL_CreateTexture failed: %s\n", SDL_GetError());
            return;
        }
    }

    // Expand RGB565 framebuffer → ARGB8888 intermediate buffer.
    for (std::size_t i = 0; i < framebuffer_.size(); ++i) {
        const lv_color_t c = framebuffer_[i];
        const auto r = static_cast<std::uint32_t>((c.full >> 11) & 0x1F) * 255 / 31;
        const auto g = static_cast<std::uint32_t>((c.full >>  5) & 0x3F) * 255 / 63;
        const auto b = static_cast<std::uint32_t>( c.full        & 0x1F) * 255 / 31;
        argb_buffer_[i] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
    }

    // Upload full buffer to the persistent texture.
    // pitch = width * 4 bytes (ARGB8888), guaranteed correct for our linear buffer.
    const int pitch = static_cast<int>(width_) * 4;
    if (SDL_UpdateTexture(texture_, nullptr, argb_buffer_.data(), pitch) != 0) {
        std::fprintf(stderr, "[SdlDisplay] SDL_UpdateTexture failed: %s\n", SDL_GetError());
        return;
    }

    int win_w, win_h;
    SDL_GetRendererOutputSize(sdl_->renderer, &win_w, &win_h);
    SDL_Rect dst{0, 0, win_w, win_h};
    SDL_RenderClear(sdl_->renderer);
    SDL_RenderCopy(sdl_->renderer, texture_, nullptr, &dst);
    SDL_RenderPresent(sdl_->renderer);
}

// -------------------------------------------------------------------------- //
// Input polling
// -------------------------------------------------------------------------- //

std::optional<InputEvent> SdlDisplay::poll_input(std::uint32_t timeout_ms) {
    // Drain all pending SDL events; return the first mapped input event.
    SDL_Event ev;
    while (SDL_WaitEventTimeout(&ev, static_cast<int>(timeout_ms))) {
        if (ev.type == SDL_QUIT) {
            quit_requested_ = true;
            if (quit_callback_) quit_callback_();
            return std::nullopt;
        }
        if (ev.type == SDL_KEYDOWN) {
            if (auto mapped = map_sdl_event(ev)) return mapped;
        }
    }
    return std::nullopt;
}

std::optional<InputEvent> SdlDisplay::map_sdl_event(const SDL_Event& ev) {
    switch (ev.key.keysym.sym) {
        case SDLK_UP:    return InputEvent{InputKey::Up};
        case SDLK_DOWN:  return InputEvent{InputKey::Down};
        case SDLK_LEFT:  return InputEvent{InputKey::Left};
        case SDLK_RIGHT: return InputEvent{InputKey::Right};
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
                         return InputEvent{InputKey::Press};
        case SDLK_ESCAPE:return InputEvent{InputKey::Back};
        case SDLK_F2:    return InputEvent{InputKey::ProfileSwitch};
        default:         return std::nullopt;
    }
}

void SdlDisplay::set_quit_callback(QuitCallback cb) {
    quit_callback_ = std::move(cb);
}

void SdlDisplay::refresh() {
    lv_timer_handler();
    blit_framebuffer();
}

// -------------------------------------------------------------------------- //
// Pointer (mouse/touch) indev
// -------------------------------------------------------------------------- //

void SdlDisplay::enable_pointer() {
    if (pointer_enabled_) return;
    lv_indev_drv_init(&pointer_driver_);
    pointer_driver_.type = LV_INDEV_TYPE_POINTER;
    pointer_driver_.read_cb = pointer_read_cb;
    pointer_driver_.user_data = this;
    pointer_device_ = lv_indev_drv_register(&pointer_driver_);
    pointer_enabled_ = true;
    std::fprintf(stderr, "[SdlDisplay] pointer indev registered (mouse/touch)\n");
}

void SdlDisplay::handle_mouse_event(const SDL_Event& ev) {
    if (!pointer_enabled_) return;

    switch (ev.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (ev.button.button == SDL_BUTTON_LEFT) {
                pointer_pressed_ = true;
                pointer_pos_.x = static_cast<lv_coord_t>(
                    ev.button.x / static_cast<int>(pixel_scale_));
                pointer_pos_.y = static_cast<lv_coord_t>(
                    ev.button.y / static_cast<int>(pixel_scale_));
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev.button.button == SDL_BUTTON_LEFT) {
                pointer_pressed_ = false;
                pointer_pos_.x = static_cast<lv_coord_t>(
                    ev.button.x / static_cast<int>(pixel_scale_));
                pointer_pos_.y = static_cast<lv_coord_t>(
                    ev.button.y / static_cast<int>(pixel_scale_));
            }
            break;
        case SDL_MOUSEMOTION:
            if (ev.motion.state & SDL_BUTTON_LMASK) {
                pointer_pos_.x = static_cast<lv_coord_t>(
                    ev.motion.x / static_cast<int>(pixel_scale_));
                pointer_pos_.y = static_cast<lv_coord_t>(
                    ev.motion.y / static_cast<int>(pixel_scale_));
            }
            break;
        default:
            break;
    }
}

std::optional<InputEvent> SdlDisplay::poll_all_events() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            quit_requested_ = true;
            if (quit_callback_) quit_callback_();
            return std::nullopt;
        }
        // Feed mouse/touch events to the pointer indev state.
        handle_mouse_event(ev);

        if (ev.type == SDL_KEYDOWN) {
            if (auto mapped = map_sdl_event(ev)) return mapped;
        }
    }
    return std::nullopt;
}

void SdlDisplay::pointer_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    auto* self = static_cast<SdlDisplay*>(drv->user_data);
    if (!self) return;
    data->point = self->pointer_pos_;
    data->state = self->pointer_pressed_ ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    // Continuous reporting — LVGL will keep calling this cb.
    data->continue_reading = false;
}

// -------------------------------------------------------------------------- //
// SDL window (re-)creation helper
// -------------------------------------------------------------------------- //

void SdlDisplay::create_sdl_window() {
    // Destroy previous texture/window/renderer if any.
    if (texture_)      { SDL_DestroyTexture(texture_);        texture_       = nullptr; }
    if (sdl_->renderer) { SDL_DestroyRenderer(sdl_->renderer); sdl_->renderer = nullptr; }
    if (sdl_->window)   { SDL_DestroyWindow(sdl_->window);     sdl_->window   = nullptr; }

    const int win_w = static_cast<int>(width_  * pixel_scale_);
    const int win_h = static_cast<int>(height_ * pixel_scale_);

    sdl_->window = SDL_CreateWindow(
        "SeedSigner LVGL Desktop",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h,
        SDL_WINDOW_SHOWN);

    if (!sdl_->window) {
        std::fprintf(stderr, "[SdlDisplay] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }

    sdl_->renderer = SDL_CreateRenderer(sdl_->window, -1, SDL_RENDERER_SOFTWARE);
    if (!sdl_->renderer) {
        std::fprintf(stderr, "[SdlDisplay] SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return;
    }
}

// -------------------------------------------------------------------------- //
// Runtime profile / resolution switching
// -------------------------------------------------------------------------- //

bool SdlDisplay::switch_resolution(std::uint32_t new_width, std::uint32_t new_height) {
    if (new_width == width_ && new_height == height_) return true;

    std::fprintf(stderr, "[SdlDisplay] switch_resolution %ux%u → %ux%u\n",
                 width_, height_, new_width, new_height);

    // 1. Remove the old LVGL display driver.
    if (display_) {
        lv_disp_remove(display_);
        display_ = nullptr;
    }

    // 2. Update dimensions and reallocate framebuffer.
    width_  = new_width;
    height_ = new_height;
    draw_buf_.assign(static_cast<std::size_t>(width_) * 40, lv_color_t{});
    framebuffer_.assign(static_cast<std::size_t>(width_) * height_, lv_color_t{});
    argb_buffer_.assign(static_cast<std::size_t>(width_) * height_, 0);

    // 3. Recreate SDL window at new size (also resets texture_).
    create_sdl_window();
    if (!sdl_->window || !sdl_->renderer) return false;

    // 4. Re-register the LVGL display driver with the new resolution.
    lv_disp_draw_buf_init(&draw_buffer_, draw_buf_.data(), nullptr,
                          static_cast<uint32_t>(draw_buf_.size()));
    lv_disp_drv_init(&display_driver_);
    display_driver_.hor_res  = width_;
    display_driver_.ver_res  = height_;
    display_driver_.flush_cb = flush_cb;
    display_driver_.draw_buf = &draw_buffer_;
    display_driver_.user_data = this;
    display_ = lv_disp_drv_register(&display_driver_);

    // 5. Re-register pointer indev if it was previously enabled.
    if (pointer_enabled_ && pointer_device_) {
        lv_indev_delete(pointer_device_);
        pointer_device_ = nullptr;
        lv_indev_drv_init(&pointer_driver_);
        pointer_driver_.type = LV_INDEV_TYPE_POINTER;
        pointer_driver_.read_cb = pointer_read_cb;
        pointer_driver_.user_data = this;
        pointer_device_ = lv_indev_drv_register(&pointer_driver_);
    }

    // Force a full refresh.
    lv_refr_now(nullptr);

    return display_ != nullptr;
}

}  // namespace seedsigner::lvgl
