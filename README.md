# seedsigner-lvgl

Modern LVGL C++ module inspired by SeedSigner screens and flows, designed to be externally controlled from MicroPython on ESP32-P4 / ESP32-S3.

## Project status

Phase 1 has started with a minimal host-simulated runtime skeleton.

Current implementation scope:
- CMake-based C++/LVGL build bootstrap
- headless host display integration for smoke validation
- `UiRuntime` top-level owner with screen registry and outbound event queue scaffolding
- placeholder screen demo route and smoke test

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

The initial simulator target is deliberately headless. It uses LVGL with a dummy host display so the runtime, screen lifecycle, and draw/flush path can be exercised in CI and on developer machines without committing yet to SDL or embedded platform glue.

## Expected deliverables in this repo

- architecture docs
- ADRs
- issue backlog
- milestones and phased implementation plan
- C++ runtime skeleton, examples, and smoke tests
- later: richer screen implementations, camera integration, bindings, and target-specific platform code
