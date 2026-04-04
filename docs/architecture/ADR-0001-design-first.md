# ADR-0001: Design-first approach before implementation

## Status
Accepted

## Decision
This project will begin with a discovery and architecture phase before meaningful implementation work starts.

## Why
The target system is not a small widget library. It combines:
- many screens and flows inspired by SeedSigner
- LVGL-based UI composition
- external flow orchestration
- embedded constraints on ESP32-P4 / S3
- camera-driven screens with explicit buffer injection needs
- future use from MicroPython

Starting implementation without a clear architecture would likely create:
- a monolithic UI runtime
- weak boundaries between business state and presentation
- expensive rewrites when camera or MicroPython integration becomes real
- unreviewable PRs

## Consequences
- Early work should prioritize inventory, API design, and module boundaries.
- Discovery and architecture work is first-class and should be tracked as issues/PRs.
- Implementation should proceed in vertical slices after risky assumptions are documented.
