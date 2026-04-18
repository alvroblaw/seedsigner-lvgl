#include "seedsigner_lvgl/screens/ScanScreen.hpp"
#include "seedsigner_lvgl/visual/DisplayProfile.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include <lvgl.h>
#include <algorithm>
#include <cstdint>
#include <string>

#include "seedsigner_lvgl/components/TopNavBar.hpp"

namespace seedsigner::lvgl {
namespace {

constexpr const char* kScanCompleteAction = "scan_complete";
constexpr const char* kScanCancelledAction = "scan_cancelled";
constexpr const char* kFrameStatusUpdatedAction = "frame_status_updated";
constexpr const char* kScanScreenComponent = "scan_screen";

// Preview dimensions sourced from the active display profile
// (kept as constants for now; screens will read profile::active() directly)
static lv_coord_t preview_width() { return profile::active().preview_size; }
static lv_coord_t preview_height() { return profile::active().preview_size; }
constexpr lv_coord_t kDotSize = 12;
constexpr lv_coord_t kDotMargin = 8;
constexpr lv_coord_t kProgressBarHeight = 6;
constexpr lv_coord_t kProgressBarWidth = 200;
constexpr lv_coord_t kInstructionFontSize = 18;
constexpr lv_coord_t kProgressFontSize = 14;

lv_color_t grayscale_to_color(std::uint8_t value) {
    return lv_color_make(value, value, value);
}

}  // namespace

void ScanScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    
    // Parse route arguments
    if (const auto it = route.args.find("instruction_text"); it != route.args.end()) {
        instruction_text_ = it->second;
    }
    if (const auto it = route.args.find("scan_mode"); it != route.args.end()) {
        scan_mode_ = it->second;
    }
    if (const auto it = route.args.find("mock_mode"); it != route.args.end()) {
        mock_mode_ = (it->second == "true" || it->second == "1" || it->second == "yes");
    }
    
    // Create root container
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, seedsigner::lvgl::theme::active_theme().SURFACE_DARK, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = instruction_text_.empty() ? "Scan QR" : instruction_text_;
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    // No custom actions
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container (everything below the nav bar)
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(content_container_, seedsigner::lvgl::theme::active_theme().SURFACE_DARK, 0);
    lv_obj_set_style_border_width(content_container_, 0, 0);
    lv_obj_set_style_pad_all(content_container_, 0, 0);
    lv_obj_set_flex_grow(content_container_, 1); // Take remaining height
    
    // Create preview canvas (initially black)
    preview_canvas_buffer_.resize(static_cast<std::size_t>(preview_width()) * static_cast<std::size_t>(preview_height()));
    preview_img_ = lv_canvas_create(content_container_);
    lv_canvas_set_buffer(preview_img_, preview_canvas_buffer_.data(), preview_width(), preview_height(), LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_size(preview_img_, preview_width(), preview_height());
    lv_obj_align(preview_img_, LV_ALIGN_CENTER, 0, 0);
    lv_canvas_fill_bg(preview_img_, seedsigner::lvgl::theme::active_theme().BLACK, LV_OPA_COVER);
    
    // Instruction label removed – title is shown in TopNavBar
    instruction_label_ = nullptr;
    
    // Progress bar (initially hidden)
    progress_bar_ = lv_bar_create(content_container_);
    lv_obj_set_size(progress_bar_, kProgressBarWidth, kProgressBarHeight);
    lv_obj_align(progress_bar_, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_bar_set_range(progress_bar_, 0, 100);
    lv_bar_set_value(progress_bar_, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progress_bar_, seedsigner::lvgl::theme::active_theme().SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_opa(progress_bar_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(progress_bar_, seedsigner::lvgl::theme::active_theme().SUCCESS, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(progress_bar_, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_add_flag(progress_bar_, LV_OBJ_FLAG_HIDDEN);
    
    // Optional progress label
    progress_label_ = lv_label_create(content_container_);
    lv_label_set_text(progress_label_, "0%");
    lv_obj_align_to(progress_label_, progress_bar_, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_style_text_color(progress_label_, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(progress_label_, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(progress_label_, LV_OBJ_FLAG_HIDDEN);
    
    // Frame status dot (bottom-right corner)
    frame_status_dot_ = lv_obj_create(content_container_);
    lv_obj_set_size(frame_status_dot_, kDotSize, kDotSize);
    lv_obj_align(frame_status_dot_, LV_ALIGN_BOTTOM_RIGHT, -kDotMargin, -kDotMargin);
    lv_obj_set_style_radius(frame_status_dot_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(frame_status_dot_, seedsigner::lvgl::theme::active_theme().ERROR, 0); // red initially
    lv_obj_set_style_bg_opa(frame_status_dot_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(frame_status_dot_, 0, 0);
    lv_obj_clear_flag(frame_status_dot_, LV_OBJ_FLAG_CLICKABLE);
    
    // Initially hide progress elements until multipart QR detected
    // For now, we keep them hidden.
    
    // Request camera frames
    context_.emit_needs_data("camera.frame", kScanScreenComponent);
}

void ScanScreen::destroy() {
    // TopNavBar must be cleared before its parent container is deleted.
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    content_container_ = nullptr;
    preview_img_ = nullptr;
    instruction_label_ = nullptr;
    progress_bar_ = nullptr;
    progress_label_ = nullptr;
    frame_status_dot_ = nullptr;
    context_ = {};
    latest_frame_.clear();
    preview_canvas_buffer_.clear();
}

bool ScanScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
    switch (input.key) {
    case InputKey::Back:
        emit_scan_cancelled();
        return true;
    case InputKey::Press:
        // Optionally pause/resume scanning? Not required for MVP.
        return false;
    case InputKey::Up:
    case InputKey::Down:
    case InputKey::Left:
    case InputKey::Right:
        // Ignored
        return false;
    }
    return false;
}

bool ScanScreen::push_frame(const CameraFrame& frame) {
    // Store frame data for preview rendering
    frame_width_ = frame.width;
    frame_height_ = frame.height;
    frame_sequence_ = frame.sequence;
    latest_frame_ = frame.pixels; // copy
    
    // Convert frame to LVGL canvas (grayscale -> color)
    if (!mock_mode_ && preview_img_ && !latest_frame_.empty() && frame_width_ > 0 && frame_height_ > 0) {
        // Ensure canvas buffer exists (we use canvas, not image)
        // We'll draw scaled preview similar to CameraPreviewScreen
        const auto source_stride = frame.stride == 0 ? frame_width_ : frame.stride;
        if (source_stride < frame_width_) return false;
        if (latest_frame_.size() < static_cast<std::size_t>(source_stride) * static_cast<std::size_t>(frame_height_)) return false;
        
        const auto target_width = static_cast<std::uint32_t>(preview_width());
        const auto target_height = static_cast<std::uint32_t>(preview_height());
        const std::uint32_t scaled_width =
            std::max<std::uint32_t>(1, std::min<std::uint32_t>(target_width, (frame_width_ * target_height) / frame_height_));
        const std::uint32_t scaled_height =
            std::max<std::uint32_t>(1, std::min<std::uint32_t>(target_height, (frame_height_ * target_width) / frame_width_));
        const std::uint32_t x_offset = (target_width - scaled_width) / 2;
        const std::uint32_t y_offset = (target_height - scaled_height) / 2;
        
        // Fill background black
        lv_canvas_fill_bg(preview_img_, seedsigner::lvgl::theme::active_theme().BLACK, LV_OPA_COVER);
        
        // Draw scaled grayscale pixels
        for (std::uint32_t y = 0; y < scaled_height; ++y) {
            const auto src_y = std::min<std::uint32_t>(frame_height_ - 1, (y * frame_height_) / scaled_height);
            for (std::uint32_t x = 0; x < scaled_width; ++x) {
                const auto src_x = std::min<std::uint32_t>(frame_width_ - 1, (x * frame_width_) / scaled_width);
                const auto value = latest_frame_[static_cast<std::size_t>(src_y) * source_stride + src_x];
                lv_canvas_set_px(preview_img_, static_cast<lv_coord_t>(x + x_offset), static_cast<lv_coord_t>(y + y_offset),
                                 grayscale_to_color(value));
            }
        }
        lv_obj_invalidate(preview_img_);
    }
    
    // Mock QR detection logic
    // Every frame we toggle frame status dot randomly (simulate readability)
    mock_frames_received_++;
    bool mock_valid = (mock_frames_received_ % 3) != 0; // 2 out of 3 frames "valid"
    update_frame_status(mock_valid);
    
    // Simulate multipart QR progress: after certain number of frames, progress increases
    if (!scan_complete_ && mock_frames_received_ % 2 == 0) {
        progress_percent_ = std::min(100u, progress_percent_ + 10);
        update_progress(progress_percent_);
    }
    
    // Simulate QR detection after N frames
    if (!scan_complete_ && mock_frames_received_ >= MOCK_FRAMES_TO_DETECTION) {
        scan_complete_ = true;
        // Freeze preview (optional: keep last frame)
        emit_scan_complete("mock_qr_data_here");
    }
    
    return true;
}

void ScanScreen::update_progress(unsigned int percent) {
    progress_percent_ = percent;
    // Show progress bar if percent > 0
    if (percent > 0) {
        lv_obj_clear_flag(progress_bar_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(progress_label_, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(progress_bar_, static_cast<lv_coord_t>(percent), LV_ANIM_ON);
        lv_label_set_text_fmt(progress_label_, "%u%%", percent);
    } else {
        lv_obj_add_flag(progress_bar_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(progress_label_, LV_OBJ_FLAG_HIDDEN);
    }
}

void ScanScreen::update_frame_status(bool valid) {
    frame_valid_ = valid;
    lv_obj_set_style_bg_color(frame_status_dot_, valid ? seedsigner::lvgl::theme::active_theme().SUCCESS : seedsigner::lvgl::theme::active_theme().ERROR, 0);
    // Optionally emit event
    context_.emit_action(kFrameStatusUpdatedAction, kScanScreenComponent, EventValue{valid});
}

void ScanScreen::emit_scan_complete(const std::string& qr_data) {
    // Emit action with QR data as value
    context_.emit_action(kScanCompleteAction, kScanScreenComponent, EventValue{qr_data});
}

void ScanScreen::emit_scan_cancelled() {
    context_.emit_action(kScanCancelledAction, kScanScreenComponent);
}

void ScanScreen::refresh_ui() {
    // Update instruction label if text changed (not needed now)
    if (instruction_label_) {
        lv_label_set_text(instruction_label_, instruction_text_.c_str());
    }
}

}  // namespace seedsigner::lvgl