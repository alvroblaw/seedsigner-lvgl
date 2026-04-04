# Architecture

## Status
Phase 0 architectural baseline.

This document defines the intended runtime architecture for `seedsigner-lvgl`: a modern C++/LVGL UI module inspired by SeedSigner screens and flows, while keeping higher-level flow control outside the module.

It is deliberately written to guide implementation in small, reviewable milestones rather than to describe a fully built system.

## Architectural goals

1. Render SeedSigner-inspired screens and reusable UI primitives in modern C++ using LVGL.
2. Keep business flow orchestration outside the UI module whenever practical.
3. Support external control from MicroPython on ESP32-P4 / ESP32-S3.
4. Support camera-backed screens through an explicit ingestion contract.
5. Keep the runtime testable in host simulation before embedded hardening.
6. Bias toward reusable primitives and screen composition, not a pile of one-off screens.

## Primary boundary

The most important architectural decision is this split:

- **Outside the module (host/controller)**
  - business/application flow
  - route decisions
  - async task completion
  - camera pipeline ownership
  - domain state
  - navigation intent in response to user actions

- **Inside the module (UI runtime)**
  - LVGL objects and layout
  - screen lifecycle
  - focus and local interaction state
  - widget composition and animations
  - ephemeral presentation state
  - rendering of host-owned view data

This is an **external-first UI runtime**, not a full app framework.

## Runtime building blocks

## 1. `UiRuntime`
Top-level owner of the UI module.

Responsibilities:
- initialize/shutdown LVGL integration
- hold runtime-wide configuration
- own screen registry, navigation controller, and event queue
- expose the controller-facing API surface
- tick timers/animations if required by platform integration

It should be the narrow façade that both native C++ and future binding layers talk to.

## 2. `NavigationController`
Controls active route stack and modal stack.

Responsibilities:
- activate/replace/pop routes
- push/dismiss modals
- assign runtime tokens to active screen/modal instances
- enforce transition rules
- coordinate lifecycle hooks when screens become active/inactive

It should not encode business meaning like “after scanning a PSBT, go to signing review.” That belongs to the host.

## 3. `ScreenRegistry`
Maps stable route IDs to screen factories.

Responsibilities:
- register available screen types
- construct screen instances when a route is activated
- enforce route-to-screen binding consistency

The registry should make it easy to add new screen families without changing the public controller API.

## 4. `Screen`
Base abstraction for every top-level routed screen.

Minimum responsibilities:
- create/destroy its LVGL subtree
- accept initial route args
- accept later data updates/patches
- receive local input focus events if needed
- emit structured UI events upward through the runtime

Expected lifecycle shape:
- `create(context, initial_args)`
- `bind(model)`
- `patch(delta)`
- `on_activate()`
- `on_deactivate()`
- `destroy()`

The exact method names can change, but the lifecycle needs to be explicit.

## 5. `Modal`
Overlay abstraction for temporary interaction layers.

Responsibilities:
- present confirmation/warning/progress/chooser overlays
- emit modal-scoped result events
- remain separate from the base screen route stack

Modal handling should reuse as much of the screen data/event model as possible.

## 6. `Component` / `WidgetPrimitive`
Reusable LVGL-backed building blocks used by screens.

Expected families based on the SeedSigner inventory:
- top navigation
- button/list items
- large icon buttons
- status/warning blocks
- keyboard/text-entry primitives
- formatted address/data rows
- BTC amount display
- QR display surface
- camera preview surface
- progress/status indicators

The architecture should prefer composing screens from these primitives instead of recreating layout logic per screen.

## 7. `ViewModelAdapter`
Internal translation layer between external payloads and screen-local render models.

Responsibilities:
- validate incoming route args or screen data patches
- normalize external dict/list/primitives into screen-local structures
- keep schema evolution localized

This layer matters because the external API is intentionally generic, while screen code still needs predictable render data.

## 8. `EventQueue`
Outbound structured event channel from UI runtime to host.

Responsibilities:
- queue user intent events
- expose polling-based event retrieval
- optionally support callback sinks later without changing screen code

Event examples:
- `action_invoked`
- `selection_changed`
- `submit_requested`
- `cancel_requested`
- `modal_result`
- `needs_data`
- `screen_ready`

## 9. `CameraSurface`
Specialized component contract for camera-backed screens.

Responsibilities:
- accept externally submitted frames
- own display-ready buffers
- expose freeze/resume/clear behavior
- invalidate only the relevant region
- keep latest-frame-wins semantics

This should be a reusable component, not hardcoded directly into one scan screen.

## 10. `ThemeAssets`
Runtime-wide visual/theme asset access.

Responsibilities:
- fonts and locale-specific font substitution
- icon resources
- bitmap/image handles
- spacing/color/typography constants

This should allow SeedSigner-like visual identity without scattering constants across every screen.

## State ownership model

The project should work with three state layers.

## A. Host-owned application state
Examples:
- active flow meaning
- seed/PSBT/signing domain data
- async task results
- camera pipeline state
- business validation results

This state lives outside the UI runtime.

## B. Screen view-model state
Examples:
- menu items to display
- title/body/status text
- address/amount summary rows
- progress values
- visible warnings or helper text

This is externally authored but consumed by the UI runtime.

## C. UI-owned ephemeral state
Examples:
- focused button index
- keyboard cursor position
- scroll position
- modal animation state
- temporary widget selection highlight

This state should remain internal unless there is a clear reason to expose it.

## Navigation model

The UI module should support a small set of route semantics:
- activate/replace screen
- back/pop
- home/reset stack
- push/dismiss modal

Important principle:
- the UI runtime manages **how** navigation is rendered
- the host decides **why** navigation happens

This mirrors how SeedSigner separates flow-oriented view logic from rendered screen surfaces.

## Screen taxonomy for architecture

Based on the current SeedSigner inventory, screens fall into a few useful architecture groups:

### 1. Composition screens
Mostly assembled from reusable primitives.
Examples:
- menus
- warnings
- confirmations
- settings selection
- many summary/review pages

These should be cheap to add once primitives exist.

### 2. Input-heavy screens
Need richer local interaction handling.
Examples:
- mnemonic entry
- passphrase entry
- custom derivation entry
- numeric/coin-flip/dice entry

These justify dedicated input primitives and state machines.

### 3. Data-dense technical review screens
Need careful formatting but not necessarily unusual input.
Examples:
- PSBT overview
- PSBT math
- xpub details
- address verification details
- SeedQR explain/summary screens

These likely depend on reusable formatting primitives and responsive layout policies.

### 4. Camera-backed screens
Need explicit ingestion and invalidation logic.
Examples:
- scan preview
- image entropy preview
- I/O camera test
- any future live preview overlays

These should be built around `CameraSurface`, not ad hoc image updates.

## Recommended module layering

A clean implementation should roughly layer like this:

1. **Platform integration layer**
   - LVGL setup
   - display/input hooks
   - memory/platform helpers

2. **UI runtime core**
   - `UiRuntime`
   - navigation controller
   - event queue
   - screen registry

3. **Contracts layer**
   - route descriptors
   - event types
   - screen data envelopes
   - camera frame descriptors

4. **Component layer**
   - reusable primitives/widgets

5. **Screen layer**
   - route-bound screen implementations composed from components

6. **Binding/host layer**
   - native controller wrapper
   - C ABI shim if needed
   - MicroPython binding later

Avoid letting the binding layer reach directly into component internals.

## Host simulation and embedded strategy

The architecture should be exercised in this order:

### Host simulation first
Needed to validate:
- route activation
- screen lifecycle
- event emission
- generic data patching
- camera frame ingestion behavior with mock frames

Why first:
- faster iteration
- easier debugging
- avoids conflating architecture bugs with ESP32 bring-up issues

### Embedded validation second
Needed to validate:
- memory footprint
- frame copy cost
- LVGL draw behavior on target
- binding practicality from MicroPython

This means the architecture must not assume Linux/desktop-only helpers or heavy STL patterns that make binding/hardening painful later.

## Camera integration boundary

The UI runtime should not own the camera driver.

Instead:
- the host configures a screen or camera surface
- an external producer pushes frames into that surface
- the runtime owns the rendered copy/buffer
- the runtime invalidates the affected region
- the screen emits user intent (`capture`, `cancel`, etc.) back to the host

This boundary keeps scanning/image workflows reusable and matches the project’s MicroPython-oriented goal.

## Why not embed full flow logic into screens

That would create the wrong project.

If screens start deciding things like:
- what route comes after scan success
- how seed/business objects are mutated
- how long-lived flow state is stored

then the module becomes a partial app clone instead of a reusable UI runtime.

SeedSigner inspiration should come from:
- screen surfaces
- reusable components
- interaction feel
- route/view separation

not from porting all application flow into C++ UI classes.

## First implementation implications

This architecture strongly supports the currently recommended first vertical slice:
- menu/list screen
- camera preview screen using `CameraSurface`
- confirmation/result screen
- all driven by external route activation + outbound events

That slice validates:
- route-driven navigation
- event queue shape
- screen composition model
- camera contract
- host/controller orchestration

without prematurely coupling to seed or signing domain logic.

## Open architectural questions

These remain for later ADRs or milestone-specific docs:
- whether route IDs should internally map to enums or hashed IDs
- exact patch semantics for screen data updates
- whether some high-frequency updates need a fast path separate from generic patching
- how far theme/font abstraction should go before localization work starts
- whether modal handling should be fully generic or have a small typed helper layer
- whether host simulation should use SDL, PC simulator glue, or another LVGL-friendly target

## Final recommendation

Build `seedsigner-lvgl` as a **thin externally controlled UI runtime** with:
- a small controller-facing API
- explicit route and modal management
- reusable LVGL component primitives
- screen implementations composed from those primitives
- UI-owned ephemeral state
- host-owned business/application state
- specialized but reusable camera surface support

That is the architecture most likely to stay maintainable, bind cleanly into MicroPython, and scale from early host simulation to ESP32-P4 / S3 deployment without rewriting the project boundary later.
