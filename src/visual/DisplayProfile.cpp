/*
 * DisplayProfile — predefined profiles and runtime switching.
 *
 * Profile constants adapted from the pattern used in
 * seedsigner-c-modules (display profile / resolution abstraction).
 * No code was directly copied; values are chosen to match the existing
 * hardcoded constants in this project's screens.
 */

#include "seedsigner_lvgl/visual/DisplayProfile.hpp"

namespace seedsigner::lvgl::profile {

// ---------------------------------------------------------------------------
// Pre-built profiles
// ---------------------------------------------------------------------------

const DisplayProfile Square240x240 = {
    /* id                */ ProfileId::Square240x240,
    /* width             */ 240,
    /* height            */ 240,
    /* preview_size      */ 240,
    /* words_per_page    */ 6,
    /* chip_width        */ 100,
    /* splash_logo_size  */ 240,
    /* screensaver_logo_size */ 80,
    /* keyboard_rows     */ 4,
    /* keyboard_cols     */ 10,
};

const DisplayProfile Portrait240x320 = {
    /* id                */ ProfileId::Portrait240x320,
    /* width             */ 240,
    /* height            */ 320,
    /* preview_size      */ 240,
    /* words_per_page    */ 8,
    /* chip_width        */ 100,
    /* splash_logo_size  */ 240,
    /* screensaver_logo_size */ 80,
    /* keyboard_rows     */ 4,
    /* keyboard_cols     */ 10,
};

// ---------------------------------------------------------------------------
// Runtime state
// ---------------------------------------------------------------------------
static const DisplayProfile* g_active = &Portrait240x320;
static DisplayProfile g_custom{ProfileId::Custom};

const DisplayProfile& active() { return *g_active; }

ProfileId current_profile_id() { return g_active->id; }

void set_profile(ProfileId id) {
    switch (id) {
        case ProfileId::Square240x240:  g_active = &Square240x240;  break;
        case ProfileId::Portrait240x320: g_active = &Portrait240x320; break;
        case ProfileId::Custom: /* must use set_profile(DisplayProfile) */ break;
    }
}

void set_profile(const DisplayProfile& custom) {
    g_custom = custom;
    g_custom.id = ProfileId::Custom;
    g_active = &g_custom;
}

const DisplayProfile& match(lv_coord_t w, lv_coord_t h) {
    if (w == 240 && h == 240) return Square240x240;
    if (w == 240 && h == 320) return Portrait240x320;
    // Fallback: pick closest
    return (w == h) ? Square240x240 : Portrait240x320;
}

} // namespace seedsigner::lvgl::profile
