#pragma once

namespace seedsigner::lvgl {

enum class InputKey {
    Up,
    Down,
    Left,
    Right,
    Press,
    Back,
    ProfileSwitch,
};

struct InputEvent {
    InputKey key{InputKey::Press};
};

}  // namespace seedsigner::lvgl
