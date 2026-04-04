# ADR-0002: External-first flow control

## Status
Proposed

## Decision
The module should prefer external control over embedding all application flow logic internally.

## Rationale
The primary motivation for this project is to make the UI reusable from MicroPython on ESP32-P4 / S3. That strongly suggests:
- navigation should be externally steerable
- screen inputs should produce events or payloads outward
- the module should own rendering and local interaction details, but not all business orchestration

## Implications
Likely architectural needs:
- screen identifiers and route/state descriptors
- command-based or callback-based navigation API
- event sink for user interactions
- explicit APIs for feeding dynamic data into active screens
- well-defined ownership of camera/image buffers

## Open questions
- What state must remain internal for usability/performance?
- What API shape works best for MicroPython bindings?
- What is the smallest viable command/event model?
