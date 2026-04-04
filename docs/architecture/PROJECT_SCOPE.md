# Project Scope

## Summary

Build a modern LVGL-based C++ module that can render and manage SeedSigner-inspired screens, components, and UI flows while exposing an external API so higher-level control can remain outside the module.

## Intended usage

- Embedded target: ESP32-P4 / ESP32-S3
- Host-side controller: MicroPython
- UI library: LVGL
- Primary mode: external orchestration of screen flow, state transitions, and data injection

## Core constraints

- The UI module should not hard-code all business flow logic internally.
- Screen transitions and significant state changes should be controllable from outside.
- Camera-consuming screens must accept external frame/buffer injection with a clear contract.
- The design should support incremental adoption by feature/screen groups.
- Architecture should prioritize maintainability and testability over brute-force parity.

## Main design questions

1. What is the right split between internal UI state and external application state?
2. Which SeedSigner screens map cleanly to reusable LVGL component patterns?
3. Which screens require custom rendering or camera-aware update paths?
4. How should the external API represent navigation, events, prompts, and payloads?
5. What frame format, ownership model, and update cadence should camera APIs use?
6. What level of abstraction is needed so the module can work from both MicroPython and native C++ callers?

## Desired outcomes

- clear architecture before implementation
- implementation milestones grouped by reviewable value
- small, mergeable PRs
- minimal early prototypes to validate risky assumptions
