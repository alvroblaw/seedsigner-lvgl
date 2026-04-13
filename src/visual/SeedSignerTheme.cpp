#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

namespace seedsigner::lvgl::theme {

// ---------------------------------------------------------------------------
// Dark palette — exact visual parity with the original constants
// ---------------------------------------------------------------------------
const ThemeColors DarkPalette = {
    /* BLACK           */ lv_color_hex(0x000000),
    /* SURFACE_DARK    */ lv_color_hex(0x1a1a1a),
    /* SURFACE_MEDIUM  */ lv_color_hex(0x2C2C2C),  // SeedSigner BUTTON_BACKGROUND
    /* SURFACE_LIGHT   */ lv_color_hex(0x333333),
    /* SURFACE_DISABLED*/ lv_color_hex(0x111111),

    /* PRIMARY         */ lv_color_hex(0xFF9F0A),  // SeedSigner ACCENT_COLOR
    /* PRIMARY_LIGHT   */ lv_color_hex(0xFFB840),
    /* PRIMARY_DARK    */ lv_color_hex(0xCC7F08),

    /* TEXT_PRIMARY    */ lv_color_hex(0xFCFCFC),  // SeedSigner BODY_FONT_COLOR
    /* TEXT_SECONDARY  */ lv_color_hex(0x777777),  // SeedSigner LABEL_FONT_COLOR
    /* TEXT_DISABLED   */ lv_color_hex(0x555555),

    /* SUCCESS         */ lv_color_hex(0x30D158),  // SeedSigner SUCCESS_COLOR
    /* WARNING         */ lv_color_hex(0xFFD60A),  // SeedSigner WARNING_COLOR
    /* ERROR           */ lv_color_hex(0xFF1B0A),  // SeedSigner ERROR_COLOR
    /* INFO            */ lv_color_hex(0x409CFF),  // SeedSigner INFO_COLOR

    /* BORDER          */ lv_color_hex(0x414141),  // SeedSigner INACTIVE_COLOR
    /* DIVIDER         */ lv_color_hex(0x414141),

    /* QR_BACKGROUND   */ lv_color_hex(0xFFFFFF),
    /* QR_FOREGROUND   */ lv_color_hex(0x000000),
};

// ---------------------------------------------------------------------------
// Light palette — inverted contrast, readable on bright backgrounds
// ---------------------------------------------------------------------------
const ThemeColors LightPalette = {
    /* BLACK           */ lv_color_hex(0xFFFFFF),
    /* SURFACE_DARK    */ lv_color_hex(0xF0F0F0),
    /* SURFACE_MEDIUM  */ lv_color_hex(0xE0E0E0),
    /* SURFACE_LIGHT   */ lv_color_hex(0xD0D0D0),
    /* SURFACE_DISABLED*/ lv_color_hex(0xC0C0C0),

    /* PRIMARY         */ lv_color_hex(0xE68A00),
    /* PRIMARY_LIGHT   */ lv_color_hex(0xCC7A00),
    /* PRIMARY_DARK    */ lv_color_hex(0xFF9900),

    /* TEXT_PRIMARY    */ lv_color_hex(0x111111),
    /* TEXT_SECONDARY  */ lv_color_hex(0x555555),
    /* TEXT_DISABLED   */ lv_color_hex(0x999999),

    /* SUCCESS         */ lv_color_hex(0x008800),
    /* WARNING         */ lv_color_hex(0xCC8800),
    /* ERROR           */ lv_color_hex(0xCC0000),
    /* INFO            */ lv_color_hex(0x2266CC),

    /* BORDER          */ lv_color_hex(0xCCCCCC),
    /* DIVIDER         */ lv_color_hex(0xBBBBBB),

    /* QR_BACKGROUND   */ lv_color_hex(0xFFFFFF),
    /* QR_FOREGROUND   */ lv_color_hex(0x000000),
};

// ---------------------------------------------------------------------------
// Runtime state
// ---------------------------------------------------------------------------
static const ThemeColors* g_active = &DarkPalette;
static ThemeVariant g_variant = ThemeVariant::Dark;

const ThemeColors& active_theme() { return *g_active; }

ThemeVariant current_theme_variant() { return g_variant; }

void set_theme(ThemeVariant variant) {
    g_variant = variant;
    g_active = (variant == ThemeVariant::Light) ? &LightPalette : &DarkPalette;
    // Invalidate the active screen so LVGL repaints with new colors
    if (lv_obj_t* scr = lv_scr_act()) {
        lv_obj_invalidate(scr);
    }
}

} // namespace seedsigner::lvgl::theme
