/*
 * InputProfile unit tests.
 *
 * Verifies profile switching, remapping, and default selection.
 */

#include <cassert>
#include <cstdio>
#include <cstring>

#include "seedsigner_lvgl/input/InputProfile.hpp"
#include "seedsigner_lvgl/input/InputMapper.hpp"

using namespace seedsigner::lvgl;
using namespace seedsigner::lvgl::input;

static int tests_run = 0;
static int tests_passed = 0;

#define CHECK(cond, msg)                                                \
    do {                                                                \
        ++tests_run;                                                    \
        if (cond) { ++tests_passed; }                                   \
        else { fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
    } while (0)

int main() {
    // Default profile is SeedSigner4Button
    set_profile(InputProfileId::SeedSigner4Button);
    CHECK(active().id == InputProfileId::SeedSigner4Button,
          "default profile is SeedSigner4Button");
    CHECK(active().left_as_back == true, "SeedSigner left_as_back=true");
    CHECK(active().right_as_select == true, "SeedSigner right_as_select=true");
    CHECK(active().nav_model == NavModel::Linear, "SeedSigner nav_model=Linear");

    // Test remap: Left -> Back
    InputEvent ev{InputKey::Left};
    bool changed = remap(ev);
    CHECK(changed, "left_as_back remap changed");
    CHECK(ev.key == InputKey::Back, "Left remapped to Back");

    // Test remap: Right -> Press
    ev = InputEvent{InputKey::Right};
    changed = remap(ev);
    CHECK(changed, "right_as_select remap changed");
    CHECK(ev.key == InputKey::Press, "Right remapped to Press");

    // Test no remap on Press
    ev = InputEvent{InputKey::Press};
    changed = remap(ev);
    CHECK(!changed, "Press not remapped");

    // Switch to GenericJoystick — no remapping
    set_profile(InputProfileId::GenericJoystick);
    CHECK(active().id == InputProfileId::GenericJoystick, "switched to joystick");
    CHECK(active().nav_model == NavModel::Free, "joystick nav_model=Free");
    CHECK(active().left_as_back == false, "joystick left_as_back=false");

    ev = InputEvent{InputKey::Left};
    changed = remap(ev);
    CHECK(!changed, "joystick Left not remapped");
    CHECK(ev.key == InputKey::Left, "joystick Left stays Left");

    // Switch to RotaryEncoder
    set_profile(InputProfileId::RotaryEncoder);
    CHECK(active().nav_model == NavModel::Encoder, "encoder nav_model=Encoder");

    // Custom profile
    InputProfile custom;
    custom.nav_model = NavModel::Free;
    custom.left_as_back = true;
    custom.right_as_select = false;
    set_profile(custom);
    CHECK(active().id == InputProfileId::Custom, "custom profile id");
    CHECK(active().nav_model == NavModel::Free, "custom nav_model");

    ev = InputEvent{InputKey::Left};
    remap(ev);
    CHECK(ev.key == InputKey::Back, "custom left_as_back works");

    ev = InputEvent{InputKey::Right};
    remap(ev);
    CHECK(ev.key == InputKey::Right, "custom right_as_select=false keeps Right");

    // current_profile_id
    CHECK(current_profile_id() == InputProfileId::Custom, "current_profile_id");
    set_profile(InputProfileId::SeedSigner4Button);
    CHECK(current_profile_id() == InputProfileId::SeedSigner4Button, "reset profile id");

    fprintf(stderr, "\n[input_profile_tests] %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
