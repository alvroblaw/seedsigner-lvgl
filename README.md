# seedsigner-lvgl

Modern LVGL C++ module inspired by SeedSigner screens and flows, designed to be externally controlled from MicroPython on ESP32-P4 / ESP32-S3.

## Project status

Phase 1 has started with a minimal host-simulated runtime skeleton.

Current implementation scope:
- CMake-based C++/LVGL build bootstrap
- headless host display integration for smoke validation
- `UiRuntime` top-level owner with screen registry and outbound event queue scaffolding
- structured SeedSigner-style menu/list screen with simple top-nav chrome, two-line rows, and accessory/checkmark support for settings-style screens
- host demo route and smoke tests for placeholder and menu flows

Still intentionally out of scope:
- embedded bring-up
- full navigation stack
- camera ingestion
- SeedSigner screen parity

## Goals

- Recreate SeedSigner-style screens and UI building blocks in modern C++ using LVGL
- Keep screen flow and state orchestration controllable from outside the module
- Support camera-backed screens through explicit frame/buffer injection APIs
- Be suitable for MicroPython-driven use on ESP32-P4 / ESP32-S3
- Preserve room for host-side simulation/testing

## Non-goals for Phase 1 bootstrap

- full UI parity in one pass
- direct port of all Python code into C++
- premature hardware-specific optimizations without architecture justification
- choosing the long-term desktop simulator frontend before the runtime core exists

## Build the host skeleton

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/host_sim_demo
```

The initial simulator target is deliberately headless. It uses LVGL with a dummy host display so the runtime, screen lifecycle, draw/flush path, and menu input/event behavior can be exercised in CI and on developer machines without committing yet to SDL or embedded platform glue.

`MenuListScreen` now accepts either simple rows (`id|Label`) or richer structured rows (`id|Label|Secondary text|accessory`). The current accessory shortcuts are `check` and `chevron`, which are enough to start building settings-family and selector-family screens without inventing a new payload format yet.

The repo now carries its own LVGL configuration in [`config/lv_conf.h`](config/lv_conf.h), so host builds do not require contributors to copy `lv_conf_template.h` or maintain a machine-local `lv_conf.h`.

For Linux/macOS requirements and host-run caveats, see [`docs/development/HOST_BUILD.md`](docs/development/HOST_BUILD.md).

## Expected deliverables in this repo

- architecture docs
- ADRs
- issue backlog
- milestones and phased implementation plan
- C++ runtime skeleton, examples, and smoke tests
- later: richer screen implementations, camera integration, bindings, and target-specific platform code
