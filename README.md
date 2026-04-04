# seedsigner-lvgl

Modern LVGL C++ module inspired by SeedSigner screens and flows, designed to be externally controlled from MicroPython on ESP32-P4 / ESP32-S3.

## Project status

Phase 0 — discovery and architecture.

No implementation claims yet. The current goal is to:
- inventory SeedSigner screens, flows, components, and data formats
- define the C++/LVGL architecture
- define the external control API
- define camera frame injection requirements
- split implementation into reviewable milestones

## Goals

- Recreate SeedSigner-style screens and UI building blocks in modern C++ using LVGL
- Keep screen flow and state orchestration controllable from outside the module
- Support camera-backed screens through explicit frame/buffer injection APIs
- Be suitable for MicroPython-driven use on ESP32-P4 / ESP32-S3
- Preserve room for host-side simulation/testing

## Non-goals for Phase 0

- full UI parity in one pass
- direct port of all Python code into C++
- premature hardware-specific optimizations without architecture justification

## Planned workstreams

1. SeedSigner UI inventory
2. Flow and state model
3. LVGL component architecture
4. External control API
5. Camera integration contract
6. Memory/performance strategy for ESP32 targets
7. Incremental implementation roadmap

## Expected deliverables in this repo

- architecture docs
- ADRs
- issue backlog
- milestones and phased implementation plan
- later: C++ module, examples, tests, integration code
