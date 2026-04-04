# Milestones

## Phase 0 — Discovery and architecture
- Inventory SeedSigner screens, flows, components, and assets
- Define module boundaries
- Define external API concept
- Define camera/frame ingestion contract
- Define milestone plan and risks

## Phase 1 — Skeleton runtime
- Core C++ project structure
- LVGL integration baseline
- Screen base class + component primitives
- External API stub
- Host-side demo/simulation target

## Phase 2 — Navigation and flow model
- Screen registry
- Navigation controller
- Event dispatch model
- External command protocol

## Phase 3 — First vertical slice
- A small set of representative screens
- One input-heavy screen
- One list/menu screen
- One camera-backed screen contract demo

## Phase 4 — Reusable SeedSigner-style component set
- buttons
- lists
- key/value displays
- confirmation dialogs
- keyboard/pin/passphrase widgets
- QR / status / progress primitives

## Phase 5 — Screen family rollout
- grouped by functional domain instead of one huge port

## Phase 6 — Embedded integration hardening
- ESP32-P4 / S3 performance and memory validation
- MicroPython bridge hardening
- camera integration validation
