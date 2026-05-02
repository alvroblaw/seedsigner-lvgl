// SdlDisplay — SDL2-backed display adapter for host desktop builds.
// Uses the LVGL v9 display + indev API (lv_display_create / lv_indev_create).

#include "seedsigner_lvgl/platform/SdlDisplay.hpp"
#include "seedsigner_lvgl/runtime/InputEvent.hpp"

#include <cstdio>
#include <cstring>

// SDL2 headers — keep after project headers to avoid macro collisions.
#include <SDL.h>

namespace seedsigner::lvgl {

// 2 bytes per pixel for RGB565 (LV_COLOR_DEPTH = 16).
static constexpr std::size_t kBytesPerPixel = 2;

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
    , draw_buf_(static_cast<std::size_t>(width) * 40 * kBytesPerPixel)
    , framebuffer_(static_cast<std::size_t>(width) * height * kBytesPerPixel, 0)
    , argb_buffer_(static_cast<std::size_t>(width) * height)
    , sdl_(std::make_unique<SdlState>())
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "[SdlDisplay] SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    create_sdl_window();

    display_ = lv_display_create(static_cast<int32_t>(width_),
                                  static_cast<int32_t>(height_));
    lv_display_set_user_data(display_, this);
    lv_display_set_flush_cb(display_, flush_cb);
    lv_display_set_buffers(display_,
                                 draw_buf_.data(), nullptr,
                                 static_cast<uint32_t>(draw_buf_.size()),
                                 LV_DISPLAY_RENDER_MODE_PARTIAL);
}

SdlDisplay::~SdlDisplay() = default;

// -------------------------------------------------------------------------- //
// LVGL flush callback
// -------------------------------------------------------------------------- //

void SdlDisplay::flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    auto* self = static_cast<SdlDisplay*>(lv_display_get_user_data(disp));
    if (self) {
        ++self->flush_count_;

        const int area_w = area->x2 - area->x1 + 1;
        const std::size_t row_bytes = static_cast<std::size_t>(area_w) * kBytesPerPixel;

        for (int y = area->y1; y <= area->y2; ++y) {
            const std::size_t dst_offset =
                (static_cast<std::size_t>(y) * self->width_ + static_cast<std::size_t>(area->x1))
                * kBytesPerPixel;
            std::memcpy(self->framebuffer_.data() + dst_offset, px_map, row_bytes);
            px_map += row_bytes;
        }
        self->blit_framebuffer();
    }
    lv_display_flush_ready(disp);
}

// -------------------------------------------------------------------------- //
// Framebuffer → SDL blit
// -------------------------------------------------------------------------- //

void SdlDisplay::blit_framebuffer() {
    if (!sdl_->renderer) return;

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

    // Expand RGB565 framebuffer (raw bytes) → ARGB8888 intermediate buffer.
    const std::size_t num_pixels = static_cast<std::size_t>(width_) * height_;
    const auto* fb = reinterpret_cast<const std::uint16_t*>(framebuffer_.data());
    for (std::size_t i = 0; i < num_pixels; ++i) {
        const std::uint16_t c = fb[i];
        const auto r = static_cast<std::uint32_t>((c >> 11) & 0x1F) * 255 / 31;
        const auto g = static_cast<std::uint32_t>((c >>  5) & 0x3F) * 255 / 63;
        const auto b = static_cast<std::uint32_t>( c        & 0x1F) * 255 / 31;
        argb_buffer_[i] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
    }

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
// Pointer (mouse/touch) indev — LVGL v9 API
// -------------------------------------------------------------------------- //

void SdlDisplay::enable_pointer() {
    if (pointer_enabled_) return;
    pointer_device_ = lv_indev_create();
    lv_indev_set_type(pointer_device_, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(pointer_device_, pointer_read_cb);
    lv_indev_set_user_data(pointer_device_, this);
    pointer_enabled_ = true;
    std::fprintf(stderr, "[SdlDisplay] pointer indev registered (mouse/touch)\n");
}

void SdlDisplay::handle_mouse_event(const SDL_Event& ev) {
    if (!pointer_enabled_) return;

    switch (ev.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (ev.button.button == SDL_BUTTON_LEFT) {
                pointer_pressed_ = true;
                pointer_pos_.x = static_cast<int32_t>(
                    ev.button.x / static_cast<int>(pixel_scale_));
                pointer_pos_.y = static_cast<int32_t>(
                    ev.button.y / static_cast<int>(pixel_scale_));
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev.button.button == SDL_BUTTON_LEFT) {
                pointer_pressed_ = false;
                pointer_pos_.x = static_cast<int32_t>(
                    ev.button.x / static_cast<int>(pixel_scale_));
                pointer_pos_.y = static_cast<int32_t>(
                    ev.button.y / static_cast<int>(pixel_scale_));
            }
            break;
        case SDL_MOUSEMOTION:
            if (ev.motion.state & SDL_BUTTON_LMASK) {
                pointer_pos_.x = static_cast<int32_t>(
                    ev.motion.x / static_cast<int>(pixel_scale_));
                pointer_pos_.y = static_cast<int32_t>(
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
        handle_mouse_event(ev);
        if (ev.type == SDL_KEYDOWN) {
            if (auto mapped = map_sdl_event(ev)) return mapped;
        }
    }
    return std::nullopt;
}

void SdlDisplay::pointer_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    auto* self = static_cast<SdlDisplay*>(lv_indev_get_user_data(indev));
    if (!self) return;
    data->point = self->pointer_pos_;
    data->state = self->pointer_pressed_ ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->continue_reading = false;
}

// -------------------------------------------------------------------------- //
// SDL window (re-)creation helper
// -------------------------------------------------------------------------- //

void SdlDisplay::create_sdl_window() {
    if (texture_)       { SDL_DestroyTexture(texture_);        texture_       = nullptr; }
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

    // 1. Remove the old LVGL display.
    if (display_) {
        lv_display_delete(display_);
        display_ = nullptr;
    }

    // 2. Update dimensions and reallocate framebuffer.
    width_  = new_width;
    height_ = new_height;
    draw_buf_.assign(static_cast<std::size_t>(width_) * 40 * kBytesPerPixel, 0);
    framebuffer_.assign(static_cast<std::size_t>(width_) * height_ * kBytesPerPixel, 0);
    argb_buffer_.assign(static_cast<std::size_t>(width_) * height_, 0);

    // 3. Recreate SDL window at new size (also resets texture_).
    create_sdl_window();
    if (!sdl_->window || !sdl_->renderer) return false;

    // 4. Re-register the LVGL display with the new resolution.
    display_ = lv_display_create(static_cast<int32_t>(width_),
                                  static_cast<int32_t>(height_));
    lv_display_set_user_data(display_, this);
    lv_display_set_flush_cb(display_, flush_cb);
    lv_display_set_buffers(display_,
                                 draw_buf_.data(), nullptr,
                                 static_cast<uint32_t>(draw_buf_.size()),
                                 LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 5. Re-register pointer indev if it was previously enabled.
    if (pointer_enabled_ && pointer_device_) {
        lv_indev_delete(pointer_device_);
        pointer_device_ = nullptr;
        pointer_device_ = lv_indev_create();
        lv_indev_set_type(pointer_device_, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(pointer_device_, pointer_read_cb);
        lv_indev_set_user_data(pointer_device_, this);
    }

    // Force a full refresh.
    lv_refr_now(display_);

    return display_ != nullptr;
}

}  // namespace seedsigner::lvgl
