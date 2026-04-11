/*
 * InputProfile — predefined profiles and runtime switching.
 *
 * Profile constants adapted from the pattern used in
 * seedsigner-c-modules (input abstraction / hardware decoupling).
 * No code was directly copied; values are chosen to match common
 * SeedSigner hardware configurations.
 */

#include "seedsigner_lvgl/input/InputProfile.hpp"

namespace seedsigner::lvgl::input {

// ---------------------------------------------------------------------------
// Pre-built profiles
// ---------------------------------------------------------------------------

const InputProfile SeedSigner4Button = {
    /* id                */ InputProfileId::SeedSigner4Button,
    /* nav_model         */ NavModel::Linear,
    /* highlight_on_nav  */ true,
    /* repeat_interval_ms*/ 200,
    /* long_press_ms     */ 600,
    /* left_as_back      */ true,
    /* right_as_select   */ true,
    /* encoder_step      */ 1,
};

const InputProfile GenericJoystick = {
    /* id                */ InputProfileId::GenericJoystick,
    /* nav_model         */ NavModel::Free,
    /* highlight_on_nav  */ true,
    /* repeat_interval_ms*/ 150,
    /* long_press_ms     */ 500,
    /* left_as_back      */ false,
    /* right_as_select   */ false,
    /* encoder_step      */ 1,
};

const InputProfile RotaryEncoder = {
    /* id                */ InputProfileId::RotaryEncoder,
    /* nav_model         */ NavModel::Encoder,
    /* highlight_on_nav  */ true,
    /* repeat_interval_ms*/ 100,
    /* long_press_ms     */ 800,
    /* left_as_back      */ false,
    /* right_as_select   */ false,
    /* encoder_step      */ 1,
};

// ---------------------------------------------------------------------------
// Runtime state
// ---------------------------------------------------------------------------
static const InputProfile* g_active = &SeedSigner4Button;
static InputProfile g_custom{InputProfileId::Custom};

const InputProfile& active() { return *g_active; }

InputProfileId current_profile_id() { return g_active->id; }

void set_profile(InputProfileId id) {
    switch (id) {
        case InputProfileId::SeedSigner4Button:  g_active = &SeedSigner4Button;  break;
        case InputProfileId::GenericJoystick:    g_active = &GenericJoystick;    break;
        case InputProfileId::RotaryEncoder:      g_active = &RotaryEncoder;      break;
        case InputProfileId::Custom: /* must use set_profile(InputProfile) */    break;
    }
}

void set_profile(const InputProfile& custom) {
    g_custom = custom;
    g_custom.id = InputProfileId::Custom;
    g_active = &g_custom;
}

}  // namespace seedsigner::lvgl::input
