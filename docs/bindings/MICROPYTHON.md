# MicroPython Bindings — uiseedsigner

## Overview

`uiseedsigner` is a MicroPython user module that wraps the `seedsigner_lvgl` C API, providing a Pythonic interface for UI runtime control on embedded targets (ESP32-P4/S3).

## Installation

Build as a MicroPython user module.

### Unix-port validation flow

Current unix validation is a two-step setup:

1. Build reproducible host-side link libraries from this repo.
2. Build upstream MicroPython `ports/unix` with `uiseedsigner` as a user module.

#### 1) Build host link libraries

From the `seedsigner-lvgl` repo:

```bash
./tools/build_micropython_host_libs.sh
```

That creates a stable build directory at `build-micropython-host/` containing the libraries used by the unix-port validation build:

- `build-micropython-host/libseedsigner_lvgl.a`
- `build-micropython-host/lib/liblvgl.a`

If needed, you can override the output directory:

```bash
./tools/build_micropython_host_libs.sh /tmp/seedsigner-mpy-build
```

#### 2) Build MicroPython unix port with `uiseedsigner`

```bash
cd /path/to/micropython
make -C mpy-cross -j$(nproc)
make -C ports/unix -j$(nproc) \
  USER_C_MODULES=/path/to/seedsigner-lvgl/bindings
```

If you built the host libs into a non-default path, pass it through to the MicroPython make invocation:

```bash
make -C ports/unix -j$(nproc) \
  USER_C_MODULES=/path/to/seedsigner-lvgl/bindings \
  UISEEDSIGNER_BUILD_DIR=/tmp/seedsigner-mpy-build
```

#### Why unix setup works this way

`uiseedsigner` is a MicroPython user module, but its ABI surface is backed by the real `seedsigner_lvgl` C/C++ runtime.

For `ports/unix`, that means MicroPython is not enough by itself. The module must also link against host-built versions of:

- `libseedsigner_lvgl.a`
- `liblvgl.a`
- `-lstdc++`

That is why unix validation is split into:

- build repo host libs first
- then build MicroPython unix port with `USER_C_MODULES`

This is validation setup for host testing, not final embedded packaging.

Important notes:

- Use `USER_C_MODULES=/path/to/seedsigner-lvgl/bindings`, not `bindings/micropython`.
- The actual build wrapper lives in `bindings/usermod_uiseedsigner/`.
- `bindings/micropython/` contains the module source, docs, and examples.
- The split avoids a unix-port path collision where a module directory named `micropython/` can clash with the output binary path `build-standard/micropython`.
- `bindings/usermod_uiseedsigner/micropython.mk` defaults to `build-micropython-host/`, but `UISEEDSIGNER_BUILD_DIR` can override it.

#### Smoke test

```bash
ports/unix/build-standard/micropython - <<'PY'
import uiseedsigner
rt = uiseedsigner.UiRuntime(240, 240)
rt.init()
print("ok")
PY
```

### Other build systems

For custom firmware/CMake integration, use the top-level `bindings/micropython.cmake`, which includes the wrapper module definition under `bindings/usermod_uiseedsigner/`.

## API Reference

### UiRuntime

```python
import uiseedsigner as ui

# Context manager (auto-cleanup)
with ui.UiRuntime(width=240, height=320) as rt:
    rt.init()
    ...

# Manual lifecycle
rt = ui.UiRuntime(240, 320)
rt.init()
...
del rt  # or let GC collect
```

#### Methods

| Method | Parameters | Returns | Description |
|--------|-----------|---------|-------------|
| `init()` | — | None | Initialize LVGL and display |
| `activate(route_id, args?)` | `str, str` | `int` | Activate a screen route |
| `replace(route_id, args?)` | `str, str` | `int` | Replace active route |
| `navigate(action, route_id=None, args=None)` | `str, str?, str?` | `int` | Generic navigation, currently `back` or `replace` |
| `get_active_route()` | — | `(route_id, screen_token, stack_depth)` or `None` | Active route metadata |
| `send_input(key)` | `int` (KEY_*) | None | Send hardware input |
| `next_event()` | — | `UiEvent` or `None` | Poll next event |
| `tick(ms)` | `int` | None | Advance LVGL tick |
| `refresh()` | — | None | Force display refresh |
| `push_frame(pixels, w, h)` | `bytes, int, int` | `int` | Push grayscale camera frame |
| `set_screen_data(data)` | `str` | `int` | Update screen data |
| `patch_screen_data(data)` | `str` | `int` | Patch active screen data |
| `push_modal(modal_id, data?)` | `str, str?` | `int` | Reserved, currently returns unsupported |
| `dismiss_modal(token, result?)` | `int, str?` | `int` | Reserved, currently returns unsupported |

### UiEvent

Event object returned by `next_event()`.

| Property | Type | Description |
|----------|------|-------------|
| `type` | `int` | EVENT_* constant |
| `action_id` | `str` or `None` | Action identifier |
| `value` | `bool/int/str/None` | Event value |
| `component_id` | `str` or `None` | Source component |
| `meta_key` | `str` or `None` | Meta key |
| `meta_value` | `int/str/None` | Meta value |

### Constants

**Input keys:**
- `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`, `KEY_PRESS`, `KEY_BACK`

**Event types:**
- `EVENT_ROUTE_ACTIVATED`, `EVENT_SCREEN_READY`, `EVENT_ACTION_INVOKED`
- `EVENT_CANCEL_REQUESTED`, `EVENT_NEEDS_DATA`, `EVENT_ERROR`, `EVENT_NONE`

### Args format

Route args use newline-separated `key=value` pairs:

```python
args = "title=Menu\nitems=a|Item A|chevron\nb|Item B|chevron"
rt.activate("menu.main", args)
```

## Examples

See `bindings/micropython/examples/`:
- `menu_navigation.py` — menu with key navigation
- `scan_flow.py` — camera frame injection and QR scan

## Architecture

```
Python (uiseedsigner)
  ↕ MicroPython C API
mod_uiseedsigner.c
  ↕ C API (c_api.h)
c_api.cpp
  ↕ C++ API
UiRuntime / ScreenRegistry / etc.
```

## Notes on EXTERNAL_API alignment

This module follows `docs/api/EXTERNAL_API.md` for the currently implemented runtime surface.

- Implemented now: `activate`, `replace`, `navigate`, `get_active_route`, `set_screen_data`, `patch_screen_data`, `send_input`, `next_event`, `tick`, `push_frame`
- Reserved but not implemented by the runtime yet: `push_modal`, `dismiss_modal`

Those modal methods are exposed in the ABI so the binding shape stays aligned, but they currently return an unsupported status until modal runtime support lands.
