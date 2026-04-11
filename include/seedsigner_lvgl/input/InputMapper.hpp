#pragma once

/*
 * InputMapper — translates InputKey according to the active InputProfile.
 *
 * Adapted from the input-remapping concept in seedsigner-c-modules.
 * No code was directly copied; the remapping logic is specific to
 * this project's InputKey enum and InputProfile schema.
 */

#include "seedsigner_lvgl/runtime/InputEvent.hpp"
#include "seedsigner_lvgl/input/InputProfile.hpp"

namespace seedsigner::lvgl::input {

/// Remap an incoming InputEvent in-place according to the active profile.
/// Returns true if the event was remapped.
///
/// Example: with left_as_back=true, InputKey::Left becomes InputKey::Back.
/// Example: with right_as_select=true, InputKey::Right becomes InputKey::Press.
inline bool remap(InputEvent& event) {
    const auto& p = active();
    bool changed = false;

    if (p.left_as_back && event.key == InputKey::Left) {
        event.key = InputKey::Back;
        changed = true;
    } else if (p.right_as_select && event.key == InputKey::Right) {
        event.key = InputKey::Press;
        changed = true;
    }

    return changed;
}

}  // namespace seedsigner::lvgl::input
