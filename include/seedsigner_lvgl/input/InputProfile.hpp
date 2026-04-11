#pragma once

/*
 * InputProfile — hardware-dependent input behaviour abstraction.
 *
 * Design adapted from seedsigner-c-modules input abstraction layer,
 * which decouples physical hardware (buttons, joystick, encoder) from
 * the LVGL key/encoder event pipeline.  No code was directly copied;
 * the architecture is reshaped to fit this project's C++ / LVGL stack.
 *
 * Usage:
 *   const auto& profile = seedsigner::lvgl::input::active();
 *   if (profile.highlight_on_nav) { /* show focus ring *\/ }
 */

#include <cstdint>

namespace seedsigner::lvgl::input {

// ---------------------------------------------------------------------------
// Predefined input profile identifiers
// ---------------------------------------------------------------------------
enum class InputProfileId {
    SeedSigner4Button,  // Original SeedSigner 4-button panel
    GenericJoystick,    // 5-way joystick (up/down/left/right/press)
    RotaryEncoder,      // Rotary encoder + press button
    Custom,
};

// ---------------------------------------------------------------------------
// Navigation model — how input keys map to list/grid traversal
// ---------------------------------------------------------------------------
enum class NavModel {
    Linear,     // Up/Down scrolls list; Left/Right maps to Back/Select
    Free,       // All four directions navigate freely (grid / joystick)
    Encoder,    // CW/CCW scrolls; press selects; long-press = back
};

// ---------------------------------------------------------------------------
// Input-profile constants
// ---------------------------------------------------------------------------
struct InputProfile {
    InputProfileId id{InputProfileId::Custom};

    /// Navigation model for list and grid screens
    NavModel nav_model{NavModel::Linear};

    /// Show a visible highlight ring on the focused item during navigation
    bool highlight_on_nav{true};

    /// Auto-advance repeat interval in ms (0 = no auto-repeat)
    std::uint32_t repeat_interval_ms{0};

    /// Long-press threshold in ms (0 = no long-press detection)
    std::uint32_t long_press_ms{0};

    /// Map Left to Back (common for 4-button panels that lack a dedicated Back key)
    bool left_as_back{false};

    /// Map Right to Select/Press (complement of left_as_back)
    bool right_as_select{false};

    /// Number of items to jump per encoder tick (rotary encoder profiles)
    int encoder_step{1};
};

// ---------------------------------------------------------------------------
// Pre-built profiles
// ---------------------------------------------------------------------------
extern const InputProfile SeedSigner4Button;
extern const InputProfile GenericJoystick;
extern const InputProfile RotaryEncoder;

// ---------------------------------------------------------------------------
// Accessor / setter
// ---------------------------------------------------------------------------
const InputProfile& active();
void set_profile(InputProfileId id);
void set_profile(const InputProfile& custom);

InputProfileId current_profile_id();

}  // namespace seedsigner::lvgl::input
