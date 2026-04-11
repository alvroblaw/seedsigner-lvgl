#pragma once

#include <lvgl.h>

namespace seedsigner::lvgl::theme {

// SeedSigner color palette
namespace colors {
    // Background and surfaces
    static const lv_color_t BLACK          = lv_color_hex(0x000000);
    static const lv_color_t SURFACE_DARK   = lv_color_hex(0x1a1a1a);   // Main background
    static const lv_color_t SURFACE_MEDIUM = lv_color_hex(0x222222);   // Cards, buttons
    static const lv_color_t SURFACE_LIGHT  = lv_color_hex(0x2a2a2a);   // Pressed states
    
    // Primary accent (Bitcoin orange)
    static const lv_color_t PRIMARY        = lv_color_hex(0xFF9900);
    static const lv_color_t PRIMARY_LIGHT  = lv_color_hex(0xFFB340);
    static const lv_color_t PRIMARY_DARK   = lv_color_hex(0xCC7A00);
    
    // Text
    static const lv_color_t TEXT_PRIMARY   = lv_color_hex(0xFFFFFF);
    static const lv_color_t TEXT_SECONDARY = lv_color_hex(0xAAAAAA);
    static const lv_color_t TEXT_DISABLED  = lv_color_hex(0x666666);
    
    // Semantic colors
    static const lv_color_t SUCCESS        = lv_color_hex(0x00AA00);
    static const lv_color_t WARNING        = lv_color_hex(0xFFAA00);
    static const lv_color_t ERROR          = lv_color_hex(0xFF3333);
    static const lv_color_t INFO           = lv_color_hex(0x3399FF);
    
    // Borders and separators
    static const lv_color_t BORDER         = lv_color_hex(0x333333);
    static const lv_color_t DIVIDER        = lv_color_hex(0x444444);
    
    // Special purpose
    static const lv_color_t QR_BACKGROUND  = lv_color_hex(0xFFFFFF);  // White for QR contrast
    static const lv_color_t QR_FOREGROUND  = lv_color_hex(0x000000);  // Black for QR modules
}

// Typography constants
namespace typography {
    // Note: Only lv_font_montserrat_14 is currently available in the build
    constexpr const lv_font_t* TITLE    = &lv_font_montserrat_14;
    constexpr const lv_font_t* BODY     = &lv_font_montserrat_14;
    constexpr const lv_font_t* CAPTION  = &lv_font_montserrat_14;
    constexpr const lv_font_t* MONO     = &lv_font_montserrat_14;  // TODO: Get monospace font
}

// Spacing and sizing
namespace spacing {
    constexpr lv_coord_t SCREEN_PADDING = 12;
    constexpr lv_coord_t COMPONENT_PADDING = 8;
    constexpr lv_coord_t BUTTON_HEIGHT = 48;
    constexpr lv_coord_t BUTTON_RADIUS = 8;
    constexpr lv_coord_t TOPBAR_HEIGHT = 52;
}

// Style helpers
inline void apply_topbar_style(lv_obj_t* obj) {
    lv_obj_set_style_bg_color(obj, colors::SURFACE_DARK, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, spacing::COMPONENT_PADDING, 0);
}

inline void apply_button_style(lv_obj_t* btn, bool is_primary = false) {
    lv_obj_set_style_radius(btn, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_bg_color(btn, is_primary ? colors::PRIMARY : colors::SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_color(btn, is_primary ? colors::PRIMARY_LIGHT : colors::SURFACE_LIGHT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, colors::SURFACE_DARK, LV_STATE_DISABLED);
    lv_obj_set_style_text_color(btn, colors::TEXT_PRIMARY, 0);
    lv_obj_set_style_text_color(btn, colors::TEXT_DISABLED, LV_STATE_DISABLED);
    lv_obj_set_style_border_width(btn, 0, 0);
}

inline void apply_card_style(lv_obj_t* card) {
    lv_obj_set_style_bg_color(card, colors::SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_border_color(card, colors::BORDER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, spacing::COMPONENT_PADDING, 0);
}

inline void apply_warning_style(lv_obj_t* container, lv_color_t severity_color) {
    lv_obj_set_style_bg_color(container, colors::SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(container, severity_color, 0);
    lv_obj_set_style_border_width(container, 3, 0);
    lv_obj_set_style_radius(container, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_pad_all(container, spacing::COMPONENT_PADDING, 0);
}

inline void apply_qr_surface_style(lv_obj_t* qr_surface) {
    lv_obj_set_style_bg_color(qr_surface, colors::QR_BACKGROUND, 0);
    lv_obj_set_style_bg_opa(qr_surface, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(qr_surface, colors::BORDER, 0);
    lv_obj_set_style_border_width(qr_surface, 2, 0);
    lv_obj_set_style_radius(qr_surface, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_pad_all(qr_surface, spacing::COMPONENT_PADDING, 0);
}

} // namespace seedsigner::lvgl::theme