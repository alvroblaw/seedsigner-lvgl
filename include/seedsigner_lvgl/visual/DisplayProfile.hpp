#pragma once

/*
 * DisplayProfile — resolution-dependent layout scaling.
 *
 * Inspired by the profile-based approach in seedsigner-c-modules, which
 * centralises resolution-specific constants rather than scattering them
 * across individual screens.  No code was directly copied; the design
 * is adapted to fit this project's C++ / LVGL architecture.
 *
 * Usage:
 *   const auto& profile = seedsigner::lvgl::profile::active();
 *   lv_coord_t w = profile.preview_size;        // camera preview square
 *   int wpp      = profile.words_per_page;       // seed-word chips per page
 */

#include <cstdint>
#include <lvgl.h>

namespace seedsigner::lvgl::profile {

// ---------------------------------------------------------------------------
// Predefined profile identifiers
// ---------------------------------------------------------------------------
enum class ProfileId {
    Square240x240,   // Original SeedSigner hardware
    Portrait240x320, // Waveshare-style portrait hat (default host resolution)
    Custom,
};

// ---------------------------------------------------------------------------
// Resolution-dependent layout constants
// ---------------------------------------------------------------------------
struct DisplayProfile {
    ProfileId id{ProfileId::Custom};

    // Display resolution
    lv_coord_t width{240};
    lv_coord_t height{320};

    // Camera preview square (always width × width for square profiles)
    lv_coord_t preview_size{240};

    // Seed-word grid
    int words_per_page{8};      // 8 fits 240×320; 6 fits 240×240
    lv_coord_t chip_width{100};

    // Startup splash logo size
    lv_coord_t splash_logo_size{240};

    // Screensaver bouncing logo
    lv_coord_t screensaver_logo_size{80};

    // Keyboard grid
    int keyboard_rows{4};
    int keyboard_cols{10};

    // Convenience helpers
    bool is_square() const { return width == height; }
};

// ---------------------------------------------------------------------------
// Pre-built profiles
// ---------------------------------------------------------------------------
extern const DisplayProfile Square240x240;
extern const DisplayProfile Portrait240x320;

// ---------------------------------------------------------------------------
// Accessor / setter
// ---------------------------------------------------------------------------
const DisplayProfile& active();
void set_profile(ProfileId id);
void set_profile(const DisplayProfile& custom);

ProfileId current_profile_id();

// ---------------------------------------------------------------------------
// Auto-select a profile matching the given resolution (returns fallback)
// ---------------------------------------------------------------------------
const DisplayProfile& match(lv_coord_t w, lv_coord_t h);

} // namespace seedsigner::lvgl::profile
