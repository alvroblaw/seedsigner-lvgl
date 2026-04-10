# Architecture

## Status
Phase 0 architectural baseline (updated 2026-04-10).

This document defines the runtime architecture for `seedsigner-lvgl`: a modern C++/LVGL UI module inspired by SeedSigner screens and flows, while keeping higher-level flow control outside the module.

The architecture described here is **already implemented** in the current codebase and serves as the foundation for all subsequent development.

## Architectural goals

1. Render SeedSigner-inspired screens and reusable UI primitives in modern C++ using LVGL.
2. Keep business flow orchestration outside the UI module whenever practical.
3. Support external control from MicroPython on ESP32‑P4 / ESP32‑S3.
4. Support camera‑backed screens through an explicit ingestion contract.
5. Keep the runtime testable in host simulation before embedded hardening.
6. Bias toward reusable primitives and screen composition, not a pile of one‑off screens.

## Primary boundary

The most important architectural decision is this split:

- **Outside the module (host/controller)**
  - business/application flow
  - route decisions
  - async task completion
  - camera pipeline ownership
  - domain state (seed, PSBT, signing data)
  - navigation intent in response to user actions

- **Inside the module (UI runtime)**
  - LVGL objects and layout
  - screen lifecycle
  - focus and local interaction state
  - widget composition and animations
  - ephemeral presentation state
  - rendering of host‑owned view data

This is an **external‑first UI runtime**, not a full app framework.

## Runtime building blocks (implemented)

### 1. `UiRuntime`
Top‑level owner of the UI module.

**Responsibilities:**
- initialize/shutdown LVGL integration (via `HeadlessDisplay`)
- hold runtime‑wide configuration (`RuntimeConfig`)
- own screen registry, navigation controller, and event queue
- expose the controller‑facing API surface (`activate`, `replace`, `send_input`, `set_screen_data`, `push_frame`, `next_event`, etc.)
- tick timers/animations (via `tick()`)

**API shape (see `include/seedsigner_lvgl/runtime/UiRuntime.hpp`):**
- `activate`, `replace` – navigate to a route
- `send_input` – forward hardware input to the active screen
- `set_screen_data`, `patch_screen_data` – update the active screen’s view‑model
- `push_frame` – inject a camera frame into the active screen
- `emit` – internal event injection
- `next_event` – poll outbound events
- `tick` – advance internal timers
- `display` – access the underlying LVGL display driver

### 2. `NavigationController`
Controls active route stack (currently single‑screen, no stack yet).

**Responsibilities:**
- activate/replace routes
- assign runtime tokens (`ScreenToken`) to active screen instances
- coordinate screen lifecycle (`create`, `on_activate`, `on_deactivate`, `destroy`)
- forward input, data updates, and camera frames to the active screen

**Implementation notes:**
- Uses `ScreenRegistry` to instantiate screens.
- Maintains an `ActiveRoute` descriptor for the currently displayed screen.
- Does **not** encode business meaning; the host decides which route to activate.

### 3. `ScreenRegistry`
Maps stable route IDs to screen factories.

**Responsibilities:**
- register available screen types (`register_route`)
- construct screen instances when a route is activated (`create`)
- enforce route‑to‑screen binding consistency

**Route IDs** are opaque strings (e.g., `"menu"`, `"camera_preview"`, `"seed_words"`). The registry uses a hash map internally.

### 4. `Screen` (base class)
Abstract base for every top‑level routed screen.

**Lifecycle methods (see `include/seedsigner_lvgl/screen/Screen.hpp`):**
- `create` – build LVGL subtree, receive initial route arguments
- `on_activate` / `on_deactivate` – called when screen becomes visible/invisible
- `destroy` – tear down LVGL objects
- `handle_input` – process hardware input (returns `true` if consumed)
- `set_data` / `patch_data` – update screen‑local view‑model
- `push_frame` – receive a camera frame (optional)

**Screen context** (`ScreenContext`) provides:
- LVGL root object
- route ID and screen token
- `emit_event` callback (to send UI events upward)
- `now_ms` timestamp function

Screens emit structured events via `context.emit()` or its convenience wrappers (`emit_action`, `emit_cancel`, `emit_needs_data`).

### 5. Contracts
Data structures that define the shape of external inputs and events.

**Implemented contracts:**
- `RouteDescriptor` – route ID + key‑value arguments (`PropertyMap`)
- `RouteId` – opaque string identifier
- `CameraContract` – camera parameters, frame formats, serialisation helpers
- `KeyboardContract`, `PSBTDetailContract`, `PSBTMathContract`, `QRDisplayContract`, `ScreensaverContract`, `SeedWordsContract`, `SettingsContract`, `StartupSplashContract` – screen‑specific argument parsers

Contracts keep the external API generic while providing typed access to screen‑specific parameters.

### 6. `EventQueue`
Bounded FIFO outbound event channel (see ADR‑0003).

**Responsibilities:**
- queue UI events (`UiEvent`) produced by screens
- expose polling‑based retrieval (`next_event`)
- drop new events when full (push returns `false`)

**Event shape** includes:
- `EventType` (`RouteActivated`, `ScreenReady`, `ActionInvoked`, `CancelRequested`, `NeedsData`, `Error`)
- route ID, screen token
- optional component ID, action ID
- optional scalar value and meta key‑value pair
- timestamp

### 7. Components
Reusable LVGL‑backed building blocks used by screens.

**Currently implemented:**
- `TopNavBar` – top navigation bar with title, back/home/cancel buttons, custom actions

**Planned families** (based on SeedSigner inventory):
- button/list items
- large icon buttons
- status/warning blocks
- keyboard/text‑entry primitives
- formatted address/data rows
- BTC amount display
- QR display surface
- camera preview surface (`CameraSurface` – not yet implemented)
- progress/status indicators

### 8. Screens
Concrete screen implementations (see `src/screens/`).

Each screen is a subclass of `Screen` and registers itself with the registry via a factory.

**Current screen inventory:**
- `CameraPreviewScreen`
- `DireWarningScreen`
- `ErrorScreen`
- `KeyboardScreen`
- `MenuListScreen`
- `PSBTDetailScreen`
- `PSBTMathScreen`
- `PSBTOverviewScreen`
- `PlaceholderScreen`
- `QRDisplayScreen`
- `ResultScreen`
- `ScanScreen`
- `ScreensaverScreen`
- `SeedWordsScreen`
- `SettingsMenuScreen`
- `SettingsSelectionScreen`
- `StartupSplashScreen`
- `WarningScreen`

Screens are composed from LVGL widgets and reusable components; they do not contain business logic.

### 9. Platform layer
Abstraction for LVGL display and input integration.

**Currently implemented:**
- `HeadlessDisplay` – LVGL display driver that uses an off‑screen buffer (for host simulation)

Future ESP32 ports will provide a display driver that interfaces with the actual hardware.

### 10. Input and camera frames
- `InputEvent` – hardware input (buttons, encoder, touch) representation
- `CameraFrame` – pixel buffer + metadata (format, dimensions, timestamp)

## State ownership model

### A. Host‑owned application state
Examples:
- active flow meaning
- seed/PSBT/signing domain data
- async task results
- camera pipeline state
- business validation results

This state lives **outside** the UI runtime and is communicated via route arguments, `set_data`/`patch_data`, and camera frames.

### B. Screen view‑model state
Examples:
- menu items to display
- title/body/status text
- address/amount summary rows
- progress values
- visible warnings or helper text

This is **externally authored** but consumed by the UI runtime. It is passed as a `PropertyMap` and can be updated incrementally with `patch_data`.

### C. UI‑owned ephemeral state
Examples:
- focused button index
- keyboard cursor position
- scroll position
- modal animation state
- temporary widget selection highlight

This state remains **internal** to the UI runtime unless there is a clear reason to expose it. Screens manage it locally and do not persist it across route changes.

## Navigation model

The UI module currently supports:
- `activate` – replace the current screen with a new one
- `replace` – same as activate (no stack yet)

**Planned extensions:**
- back/pop
- home/reset stack
- push/dismiss modal (modal abstraction not yet implemented)

**Important principle:**
- the UI runtime manages **how** navigation is rendered (transitions, lifecycle)
- the host decides **why** navigation happens (in response to user actions, task completion, etc.)

## Screen taxonomy for architecture

Based on the current SeedSigner inventory, screens fall into a few useful architecture groups:

### 1. Composition screens
Mostly assembled from reusable primitives.
Examples:
- menus (`MenuListScreen`)
- warnings (`WarningScreen`, `DireWarningScreen`)
- settings selection (`SettingsSelectionScreen`)
- many summary/review pages (`PSBTOverviewScreen`, `PSBTDetailScreen`)

These should be cheap to add once primitives exist.

### 2. Input‑heavy screens
Need richer local interaction handling.
Examples:
- mnemonic entry (`SeedWordsScreen`)
- passphrase entry (not yet implemented)
- custom derivation entry (not yet implemented)
- numeric/coin‑flip/dice entry (not yet implemented)

These justify dedicated input primitives and state machines.

### 3. Data‑dense technical review screens
Need careful formatting but not necessarily unusual input.
Examples:
- PSBT math (`PSBTMathScreen`)
- xpub details (not yet implemented)
- address verification details (not yet implemented)
- SeedQR explain/summary screens (not yet implemented)

These likely depend on reusable formatting primitives and responsive layout policies.

### 4. Camera‑backed screens
Need explicit ingestion and invalidation logic.
Examples:
- scan preview (`CameraPreviewScreen`, `ScanScreen`)
- image entropy preview (not yet implemented)
- I/O camera test (not yet implemented)

These should be built around a reusable `CameraSurface` component (not yet implemented) rather than ad‑hoc image updates.

## Module layering

A clean implementation follows this layering:

1. **Platform integration layer**
   - LVGL setup (`HeadlessDisplay`)
   - display/input hooks (future)
   - memory/platform helpers (future)

2. **UI runtime core**
   - `UiRuntime`
   - `NavigationController`
   - `EventQueue`
   - `ScreenRegistry`

3. **Contracts layer**
   - route descriptors, event types, screen data envelopes
   - camera frame descriptors
   - screen‑specific argument parsers

4. **Component layer**
   - reusable primitives/widgets (`TopNavBar`, future `CameraSurface`, etc.)

5. **Screen layer**
   - route‑bound screen implementations composed from components

6. **Binding/host layer**
   - native controller wrapper (C++ `UiRuntime` interface)
   - C ABI shim (if needed)
   - MicroPython binding (future)

Avoid letting the binding layer reach directly into component internals.

## Host simulation and embedded strategy

The architecture is exercised in this order:

### Host simulation first
Already validated with:
- route activation
- screen lifecycle
- event emission
- generic data patching
- camera frame ingestion with mock frames

**Why first:**
- faster iteration
- easier debugging
- avoids conflating architecture bugs with ESP32 bring‑up issues

### Embedded validation second
Planned for later milestones:
- memory footprint
- frame copy cost
- LVGL draw behavior on target
- binding practicality from MicroPython

The architecture must not assume Linux/desktop‑only helpers or heavy STL patterns that make binding/hardening painful later.

## Camera integration boundary

The UI runtime does **not** own the camera driver.

Instead:
- the host configures a screen (via `CameraParams` in route arguments)
- an external producer pushes frames via `push_frame()`
- the runtime owns the rendered copy/buffer (screen may copy or reference)
- the runtime invalidates the affected region
- the screen emits user intent (`capture`, `cancel`, etc.) back to the host

This boundary keeps scanning/image workflows reusable and matches the project’s MicroPython‑oriented goal.

## Why not embed full flow logic into screens

That would create the wrong project.

If screens start deciding things like:
- what route comes after scan success
- how seed/business objects are mutated
- how long‑lived flow state is stored

then the module becomes a partial app clone instead of a reusable UI runtime.

SeedSigner inspiration should come from:
- screen surfaces
- reusable components
- interaction feel
- route/view separation

not from porting all application flow into C++ UI classes.

## Open architectural questions

These remain for later ADRs or milestone‑specific docs:
- whether route IDs should internally map to enums or remain hashed strings (currently strings)
- exact patch semantics for screen data updates (currently whole‑map replacement or partial merge?)
- whether some high‑frequency updates need a fast path separate from generic patching
- how far theme/font abstraction should go before localization work starts
- whether modal handling should be fully generic or have a small typed helper layer
- whether host simulation should use SDL, PC simulator glue, or another LVGL‑friendly target

## Final recommendation

`seedsigner‑lvgl` is a **thin externally controlled UI runtime** with:
- a small controller‑facing API
- explicit route and modal management (modal still pending)
- reusable LVGL component primitives
- screen implementations composed from those primitives
- UI‑owned ephemeral state
- host‑owned business/application state
- specialized but reusable camera surface support (pending)

This architecture is already implemented and provides a solid foundation for scaling from host simulation to ESP32‑P4 / S3 deployment without rewriting the project boundary later.

---

*This document updates the earlier Phase 0 architectural baseline (2026‑04‑04) to reflect the current implementation.*