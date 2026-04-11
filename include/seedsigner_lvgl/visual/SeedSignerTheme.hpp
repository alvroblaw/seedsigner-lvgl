#pragma once

#include <lvgl.h>

namespace seedsigner::lvgl::theme {

// ---------------------------------------------------------------------------
// Runtime-swappable theme palette
// ---------------------------------------------------------------------------

enum class ThemeVariant { Dark, Light };

struct ThemeColors {
    // Backgrounds & surfaces
    lv_color_t BLACK;
    lv_color_t SURFACE_DARK;
    lv_color_t SURFACE_MEDIUM;
    lv_color_t SURFACE_LIGHT;
    lv_color_t SURFACE_DISABLED;

    // Primary accent (Bitcoin orange)
    lv_color_t PRIMARY;
    lv_color_t PRIMARY_LIGHT;
    lv_color_t PRIMARY_DARK;

    // Text
    lv_color_t TEXT_PRIMARY;
    lv_color_t TEXT_SECONDARY;
    lv_color_t TEXT_DISABLED;

    // Semantic
    lv_color_t SUCCESS;
    lv_color_t WARNING;
    lv_color_t ERROR;
    lv_color_t INFO;

    // Borders & separators
    lv_color_t BORDER;
    lv_color_t DIVIDER;

    // QR
    lv_color_t QR_BACKGROUND;
    lv_color_t QR_FOREGROUND;
};

// Pre-built palettes (defined in SeedSignerTheme.cpp)
extern const ThemeColors DarkPalette;
extern const ThemeColors LightPalette;

// Accessor / setter
const ThemeColors& active_theme();
void set_theme(ThemeVariant variant);
ThemeVariant current_theme_variant();

// Legacy namespace kept as compile-time aliases (matches DarkPalette exactly).
// New code should prefer active_theme().
namespace colors {
    static const lv_color_t BLACK            = lv_color_hex(0x000000);
    static const lv_color_t SURFACE_DARK     = lv_color_hex(0x1a1a1a);
    static const lv_color_t SURFACE_MEDIUM   = lv_color_hex(0x222222);
    static const lv_color_t SURFACE_LIGHT    = lv_color_hex(0x2a2a2a);
    static const lv_color_t SURFACE_DISABLED = lv_color_hex(0x111111);
    static const lv_color_t PRIMARY          = lv_color_hex(0xFF9900);
    static const lv_color_t PRIMARY_LIGHT    = lv_color_hex(0xFFB340);
    static const lv_color_t PRIMARY_DARK     = lv_color_hex(0xCC7A00);
    static const lv_color_t TEXT_PRIMARY     = lv_color_hex(0xFFFFFF);
    static const lv_color_t TEXT_SECONDARY   = lv_color_hex(0xAAAAAA);
    static const lv_color_t TEXT_DISABLED    = lv_color_hex(0x555555);
    static const lv_color_t SUCCESS          = lv_color_hex(0x00AA00);
    static const lv_color_t WARNING          = lv_color_hex(0xFFAA00);
    static const lv_color_t ERROR            = lv_color_hex(0xFF3333);
    static const lv_color_t INFO             = lv_color_hex(0x3399FF);
    static const lv_color_t BORDER           = lv_color_hex(0x333333);
    static const lv_color_t DIVIDER          = lv_color_hex(0x444444);
    static const lv_color_t QR_BACKGROUND    = lv_color_hex(0xFFFFFF);
    static const lv_color_t QR_FOREGROUND    = lv_color_hex(0x000000);
}

// Typography constants
namespace typography {
    constexpr const lv_font_t* TITLE   = &lv_font_montserrat_14;
    constexpr const lv_font_t* BODY    = &lv_font_montserrat_14;
    constexpr const lv_font_t* CAPTION = &lv_font_montserrat_14;
    constexpr const lv_font_t* MONO    = &lv_font_montserrat_14;
}

// Spacing and sizing
namespace spacing {
    constexpr lv_coord_t SCREEN_PADDING     = 8;
    constexpr lv_coord_t COMPONENT_PADDING  = 6;
    constexpr lv_coord_t BUTTON_HEIGHT      = 40;
    constexpr lv_coord_t BUTTON_RADIUS      = 4;
    constexpr lv_coord_t TOPBAR_HEIGHT      = 44;
    constexpr lv_coord_t ROW_RADIUS         = 4;
    constexpr lv_coord_t ROW_PAD            = 8;
    constexpr lv_coord_t ROW_GAP            = 6;
    constexpr lv_coord_t CHIP_RADIUS        = 4;
    constexpr lv_coord_t CHIP_MARGIN        = 4;
    constexpr lv_coord_t CHIP_HEIGHT        = 36;
}

// ---------------------------------------------------------------------------
// Style helpers — now read from active_theme()
// ---------------------------------------------------------------------------

inline void apply_topbar_style(lv_obj_t* obj) {
    auto& t = active_theme();
    lv_obj_set_style_bg_color(obj, t.SURFACE_DARK, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, spacing::COMPONENT_PADDING, 0);
}

inline void apply_button_style(lv_obj_t* btn, bool is_primary = false) {
    auto& t = active_theme();
    lv_obj_set_style_radius(btn, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(btn, is_primary ? t.PRIMARY : t.SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_color(btn, is_primary ? t.PRIMARY_DARK : t.SURFACE_LIGHT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, t.SURFACE_DISABLED, LV_STATE_DISABLED);
    lv_obj_set_style_text_color(btn, t.TEXT_PRIMARY, 0);
    lv_obj_set_style_text_color(btn, t.TEXT_PRIMARY, LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btn, t.TEXT_DISABLED, LV_STATE_DISABLED);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, t.BORDER, 0);
    lv_obj_set_style_border_color(btn, is_primary ? t.PRIMARY_LIGHT : t.PRIMARY, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, t.DIVIDER, LV_STATE_DISABLED);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 16, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_spread(btn, 2, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(btn, is_primary ? t.PRIMARY : t.BLACK, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_50, LV_STATE_PRESSED);
    lv_obj_set_style_translate_y(btn, 2, LV_STATE_PRESSED);
    lv_obj_set_style_opa(btn, LV_OPA_50, LV_STATE_DISABLED);
    lv_obj_set_style_outline_width(btn, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_pad(btn, 1, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_color(btn, t.PRIMARY, LV_STATE_FOCUSED);
}

inline void apply_card_style(lv_obj_t* card) {
    auto& t = active_theme();
    lv_obj_set_style_bg_color(card, t.SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_border_color(card, t.BORDER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, spacing::COMPONENT_PADDING, 0);
}

inline void apply_warning_style(lv_obj_t* container, lv_color_t severity_color) {
    auto& t = active_theme();
    lv_obj_set_style_bg_color(container, t.SURFACE_MEDIUM, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(container, severity_color, 0);
    lv_obj_set_style_border_width(container, 3, 0);
    lv_obj_set_style_radius(container, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_pad_all(container, spacing::COMPONENT_PADDING, 0);
}

inline void apply_qr_surface_style(lv_obj_t* qr_surface) {
    auto& t = active_theme();
    lv_obj_set_style_bg_color(qr_surface, t.QR_BACKGROUND, 0);
    lv_obj_set_style_bg_opa(qr_surface, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(qr_surface, t.BORDER, 0);
    lv_obj_set_style_border_width(qr_surface, 2, 0);
    lv_obj_set_style_radius(qr_surface, spacing::BUTTON_RADIUS, 0);
    lv_obj_set_style_pad_all(qr_surface, spacing::COMPONENT_PADDING, 0);
}

} // namespace seedsigner::lvgl::theme
