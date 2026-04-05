#pragma once

namespace seedsigner::lvgl {

enum class InputKey {
    Up,
    Down,
    Left,
    Right,
    Press,
    Back,
};

struct InputEvent {
    InputKey key{InputKey::Press};
};

}  // namespace seedsigner::lvgl
