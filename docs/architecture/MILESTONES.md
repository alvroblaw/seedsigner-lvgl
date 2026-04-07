# Milestones

## Phase 0 — Discovery and architecture ✅ Mostly complete
- Inventory SeedSigner screens, flows, components, and assets **done**
- Define module boundaries **done**
- Define external API concept **done** (EXTERNAL_API.md)
- Define camera/frame ingestion contract **done** (CameraContract)
- Define milestone plan and risks **done**

**Status:** Core architecture documents are stable; camera contract implemented.

## Phase 1 — Skeleton runtime ✅ Complete
- Core C++ project structure **done**
- LVGL integration baseline **done**
- Screen base class + component primitives **done** (MenuListScreen, ResultScreen, CameraPreviewScreen)
- External API stub **done** (runtime control surface)
- Host-side demo/simulation target **done**

**Status:** Basic runtime with screen base classes, input handling, and external API stubs is operational.

## Phase 2 — Navigation and flow model ✅ Complete + extended
- Screen registry **done** (route‑based activation)
- Navigation controller **done** (activate, navigate, push_modal)
- Event dispatch model **done** (outbound event queue)
- External command protocol **done** (set_screen_data, patch_screen_data, send_input)
- Extended with settings‑family contracts (SettingsMenuContract, SettingsSelectionScreen)

**Status:** Navigation and event flow fully functional; settings family adds richer selection semantics.

## Phase 3 — First vertical slice ✅ Complete
- A small set of representative screens **done** (SettingsMenuScreen, WarningScreen family, QRDisplayScreen, KeyboardScreen, ScanScreen)
- One input‑heavy screen **done** (KeyboardScreen)
- One list/menu screen **done** (SettingsMenuScreen)
- One camera‑backed screen contract demo **done** (ScanScreen with mock detection)

**Status:** All three vertical slices implemented and integrated; each screen family has a dedicated contract.

## Phase 4 — Reusable SeedSigner‑style component set ✅ MVP complete
- buttons **partial** (basic LVGL buttons, missing SeedSigner styling)
- lists **partial** (MenuListScreen primitive, missing top‑nav and SeedSigner visuals)
- key/value displays **none**
- confirmation dialogs **partial** (WarningScreen family covers basic dialogs)
- keyboard/pin/passphrase widgets **done** (KeyboardScreen generic text entry)
- QR / status / progress primitives **partial** (QRDisplayScreen for QR, ResultScreen for status, progress missing)

**Status:** MVP complete — the project now has a working set of reusable components sufficient to build basic SeedSigner flows, though visual parity is still low.

## Phase 5 — Screen family rollout 🔄 In progress
- grouped by functional domain instead of one huge port
- **Current focus:** Settings family (SettingsMenuScreen, SettingsSelectionScreen) and warning family implemented.
- **Next:** QR display family (QRDisplayScreen) and keyboard family (KeyboardScreen) already done.
- **Remaining:** Seed selector, transaction review, seed reveal, address explorer, etc.

## Phase 6 — Embedded integration hardening ⏳ Future
- ESP32-P4 / S3 performance and memory validation **pending**
- MicroPython bridge hardening **pending**
- camera integration validation **pending** (mock detection only so far)

---
*Last updated after PRs #33, #40, #41, #42, #44 (April 2026)*
