# External Control API

## Status
Phase 0 design proposal.

## Purpose
Define the external API boundary for driving the LVGL UI module from a higher-level controller, with MicroPython on ESP32-P4 / ESP32-S3 as the primary future caller and native C++ as a secondary caller.

This API is for:
- screen flow control
- screen activation and replacement
- delivery of user input events to the host
- injection of dynamic state/data into screens
- modal/dialog lifecycle control
- externally driven updates such as camera frames, progress, and async results

This document does **not** define implementation details, only the external contract shape that implementation should target.

---

## Design goals

1. **MicroPython-friendly first**
   - simple method names
   - small number of core concepts
   - no heavy template/generic surface
   - minimal callback complexity

2. **C++-practical**
   - easy to implement behind a stable façade
   - easy to bind into MicroPython/C modules later
   - avoids requiring arbitrary object graphs across the language boundary

3. **External-first flow control**
   - navigation remains steerable from outside
   - UI module owns rendering and local widget behavior
   - business flow decisions stay outside the module

4. **Explicit state ownership**
   - host-owned application state
   - UI-owned ephemeral view state
   - clear contracts for replacing or patching screen data

5. **Suitable for async/embedded work**
   - camera and QR updates can arrive from outside
   - background tasks can push progress/results into active screens
   - input can be consumed as events without blocking the UI runtime

---

## Non-goals

This API should not:
- embed all app/business logic into the UI module
- require the host to manipulate LVGL widgets directly
- force one monolithic serialized protocol if an in-process C++ API is enough
- depend on exceptions, RTTI-heavy patterns, or dynamic reflection to be useful

---

## Terminology

### Host
The external controller. In the target design this is expected to be MicroPython, but it may also be native C++.

### UI runtime
The C++/LVGL module that creates screens, renders widgets, tracks focus, handles animations, and emits UI events.

### Screen
A top-level routed view such as a menu, confirmation page, keyboard entry page, scan page, review page, or status/progress page.

### Modal
A temporary overlay such as a confirmation dialog, error dialog, chooser, or transient prompt.

### Route
A screen identifier plus optional arguments used to activate a screen.

### View model
The structured data the UI runtime uses to render a screen or modal.

### Event
A structured notification emitted from the UI runtime to the host describing user actions or significant state transitions.

---

## API shape options considered

## Option A: Direct screen object API

Example shape:

```text
ui.show_scan_screen(config)
ui.show_pin_entry(config)
ui.update_scan_overlay(...)
ui.show_confirm_dialog(...)
```

### Pros
- easy to read for small demos
- straightforward for hand-written bindings
- low ceremony for common screens

### Cons
- scales poorly as screen count grows
- hard to keep behavior consistent across screens
- encourages screen-specific one-off methods
- leads to a wide binding surface
- weak fit for generic navigation/history/control concepts

### Verdict
Useful for prototypes, but not a good primary architecture for a growing screen catalog.

---

## Option B: Fully serialized command bus

Example shape:

```text
ui.send({"cmd": "activate", "route": "scan", ...})
event = ui.poll_event()
```

### Pros
- very uniform
- easy to bridge across process/language boundaries
- decouples host from concrete C++ types

### Cons
- too stringly typed if used everywhere
- poorer ergonomics for native C++ callers
- more validation burden at runtime
- easier to make mistakes on embedded targets
- debugging becomes more protocol-centric than API-centric

### Verdict
Good as a transport or debug layer, but too blunt as the main in-process API.

---

## Option C: Typed controller API with route IDs, key/value payloads, and event queue

Example shape:

```text
ui.activate("scan", args)
ui.push_modal("confirm", data)
ui.update_state("scan.preview", patch)
ui.send_input(input_event)
event = ui.next_event()
```

### Pros
- small stable surface
- route-driven, so it scales to many screens
- keeps screen-specific details inside payload schemas rather than new methods
- works well for C++ and MicroPython bindings
- easy to expose as either typed wrappers or a simple command layer
- good match for external-first orchestration

### Cons
- requires disciplined schema docs per screen/modal family
- still uses IDs/keys, so naming consistency matters
- some validation moves from compile time to runtime

### Verdict
**Recommended.** This is the best balance for Phase 0 and future implementation.

---

## Recommendation

Adopt **Option C**:

- a **small controller façade** with a handful of stable operations
- **route IDs** for screens and modal IDs for overlays
- **structured map/list payloads** for arguments and view models
- **event queue / callback sink** for outbound user events
- **state patching** for externally driven updates

This keeps the binding surface small while avoiding a giant stringly protocol for everything.

---

## Recommended top-level interface

The exact naming can change in implementation, but the external behavior should map to the following operations.

## Lifecycle

### `init(config)`
Initialize the UI runtime.

Config may include:
- display parameters
- theme/skin selection
- memory limits or feature flags
- event queue sizing
- optional platform hooks

### `tick(now_ms)`
Advance timers/animations and process deferred work if the platform requires explicit pumping.

### `deinit()`
Tear down the runtime.

---

## Navigation and screen activation

### `activate(route_id, args=None, options=None) -> screen_token`
Activate a screen as the primary visible route.

Semantics:
- if there is an existing active screen, it is replaced or transitioned away according to `options`
- `args` contains route arguments used for initial setup
- returns a `screen_token` for future targeted updates if needed

Typical uses:
- open a menu screen
- open PIN/passphrase entry
- open QR scan screen
- open review/confirm screen
- open loading/progress screen

`options` may include:
- `transition`: none, push, replace, fade
- `reset_history`: bool
- `focus_target`: optional field/component id
- `reason`: optional host-visible string for logs/debugging

### `navigate(action, options=None)`
Generic navigation operation.

Actions should cover:
- `back`
- `home`
- `pop_to(route_id)`
- `dismiss_active_modal`
- `replace(route_id, args)`

This avoids proliferating special-case methods.

### `get_active_route() -> route_descriptor`
Return current active screen metadata.

Descriptor should include:
- `route_id`
- `screen_token`
- `stack_depth`
- `modal_count`

---

## Modal/dialog interaction

### `push_modal(modal_id, data=None, options=None) -> modal_token`
Show a modal above the active screen.

Examples:
- confirmation dialog
- warning/error dialog
- simple chooser
- progress overlay
- passphrase suggestion picker

### `update_modal(modal_token=None, data=None, patch=None)`
Update the contents of an existing modal.

### `dismiss_modal(modal_token=None, result=None)`
Dismiss the top modal or a specific modal.

### Modal rules
- modals emit their own events just like screens
- modal dismissal may emit `modal_closed` or `modal_result`
- host may choose whether modal actions immediately trigger navigation or only report intent

---

## Input delivery and user intent

There are two distinct directions:

1. **Physical/local input enters the UI runtime** from hardware drivers.
2. **User intent exits the UI runtime** as structured events to the host.

The external control API must also support synthetic input for testing or remote control.

### `send_input(input_event)`
Inject an input event into the currently active UI context.

Supported input classes should include:
- `key`: up, down, left, right, press, back, home
- `encoder`: delta, press, long_press
- `touch`: tap, long_press, swipe, coordinates
- `text`: character insertion, deletion, submit
- `system`: timeout, wake, sleep, camera_ready

This method is mainly for:
- simulation/testing
- non-standard external controllers
- host-driven replay or automation

### Outbound event model
The UI runtime should emit structured events to the host through either:
- `next_event()` polling, or
- registered callback/sink

The core contract should not depend on callbacks being the only mechanism. Polling is easier for MicroPython.

### `next_event() -> event | None`
Return the next pending event, if any.

At minimum, events should cover:
- `route_activated`
- `route_closed`
- `nav_requested`
- `action_invoked`
- `field_changed`
- `selection_changed`
- `submit_requested`
- `cancel_requested`
- `modal_opened`
- `modal_result`
- `error`
- `needs_data`
- `screen_ready`

Event fields should generally include:
- `type`
- `route_id`
- `screen_token`
- `component_id` if relevant
- `action_id` if relevant
- `value` if relevant
- `meta` for optional structured details
- `timestamp_ms`

See also `docs/api/STATE_MODEL.md` for ownership rules.
For the first concrete settings-family payload/schema bridge, see `docs/api/SETTINGS_CONTRACT.md`.

---

## State and data injection

The external API needs two data update patterns.

### Pattern 1: full view-model replacement
Used when the host owns the complete state for a screen.

### `set_screen_data(target, data)`
Replace the full render model for a screen.

Where `target` may be:
- active screen
- `screen_token`
- route_id if unique enough

Good for:
- menus
- review/summary screens
- confirmation screens
- status pages

### Pattern 2: incremental patch/update
Used when the screen is long-lived and only some fields change.

### `patch_screen_data(target, patch)`
Merge or patch a subset of fields.

Good for:
- progress updates
- camera scan hints/status
- passphrase suggestion updates
- asynchronously loaded list items
- validation errors

### `set_shared_state(scope, data)`
Set data not tied to a single screen.

Possible scopes:
- `session`
- `flow`
- `theme`
- `device`
- `camera`

This should be used sparingly. Prefer screen-local models unless the data is genuinely cross-screen.

---

## Externally driven streaming or high-frequency updates

Some screens must be updated frequently by outside systems.

## Camera/frame updates

### `push_frame(target, frame_descriptor, frame_buffer)`
Provide a camera/image frame to a screen or widget that consumes external image data.

The detailed frame memory contract should live in a camera-specific document, but the control API should assume:
- explicit pixel format
- dimensions and stride
- ownership/lifetime rules are documented
- frame dropping is allowed under load
- frame arrival does not itself imply navigation

## Progress/status updates

### `publish(target, topic, payload)`
Generic externally driven update for non-frame streams.

Typical topics:
- `progress`
- `status`
- `hint`
- `validation`
- `preview`
- `result`

This generic form is useful when `patch_screen_data()` is too tied to static view models.

Recommendation: keep `publish()` available, but use it only for streaming or transient updates. Prefer `set_screen_data()` / `patch_screen_data()` for stable screen state.

---

## Result and request patterns

The UI runtime should mostly emit intent, not decide business outcomes.

Examples:
- user presses confirm on a review screen -> emit `submit_requested`
- user selects a menu item -> emit `action_invoked` or `selection_committed`
- user closes a warning modal -> emit `modal_result`
- a screen needs data not yet present -> emit `needs_data`

The host decides what happens next:
- navigate elsewhere
- patch the active screen
- open a modal
- reject an action with an error state

This split keeps the UI reusable and thin.

---

## Suggested data conventions

To make binding generation practical, payloads should use a constrained data model.

Allowed value kinds:
- null
- bool
- int
- string
- bytes
- list of allowed values
- map/dict with string keys and allowed values

Avoid exposing arbitrary custom classes across the boundary.

### IDs
Use stable string IDs for:
- `route_id`
- `modal_id`
- `component_id`
- `action_id`
- `topic`

Rationale:
- easiest for MicroPython
- easiest for schema docs
- can still map internally to enums or hashed IDs in C++

### Tokens
Use opaque numeric or small-string tokens for runtime instances:
- `screen_token`
- `modal_token`

Rationale:
- avoids leaking internal pointers/handles
- easy to bind in C and MicroPython

---

## Minimal Phase 1 API surface

If Phase 1 needs an intentionally tiny surface, the minimum useful set is:

- `init(config)`
- `activate(route_id, args=None, options=None)`
- `navigate(action, options=None)`
- `push_modal(modal_id, data=None, options=None)`
- `dismiss_modal(modal_token=None, result=None)`
- `set_screen_data(target, data)`
- `patch_screen_data(target, patch)`
- `send_input(input_event)`
- `next_event()`
- `tick(now_ms)`

This is enough to validate the architecture before adding specialized helpers.

---

## Example flows

## Example 1: menu screen

Host:
1. `activate("main_menu", {items: [...]})`
2. UI displays menu.
3. User selects item `scan`.
4. UI emits:

```json
{
  "type": "action_invoked",
  "route_id": "main_menu",
  "action_id": "scan",
  "screen_token": 12
}
```

5. Host decides next route:
   `activate("scan_qr", {mode: "seed"})`

## Example 2: confirm modal

Host:
1. `push_modal("confirm", {title: "Erase data?", confirm_label: "Erase"})`
2. User presses confirm.
3. UI emits:

```json
{
  "type": "modal_result",
  "modal_id": "confirm",
  "result": "confirm",
  "modal_token": 4
}
```

4. Host performs business action and then:
   `dismiss_modal(4)` or `activate("erase_progress", ...)`

## Example 3: async scan status update

Host:
1. `activate("scan_qr", {title: "Scan QR"})`
2. Camera subsystem starts pushing frames.
3. Decoder finds partial progress.
4. Host updates UI:
   `patch_screen_data(active, {progress: 0.6, status_text: "Scanning 3/5"})`
5. User presses cancel.
6. UI emits `cancel_requested`.
7. Host stops scanner and navigates back.

---

## Error handling

Methods should return compact status information suitable for embedded use.

Recommended pattern:
- success/failure boolean or small result code
- optional error code enum/string
- no requirement for exceptions across the API boundary

Likely error classes:
- unknown route
- invalid payload
- invalid token
- busy/incompatible state
- unsupported operation
- resource limit exceeded

The UI runtime may also emit `error` events for recoverable UI-level problems.

---

## Binding guidance

The implementation should keep a narrow ABI-oriented core.

Recommended internal layering:

1. **Core controller interface**
   - stable C++ class surface
2. **Optional C ABI shim**
   - flat functions + opaque handles
3. **MicroPython binding layer**
   - converts dict/list/primitives into the core value model

This layering keeps the main runtime clean and makes binding generation realistic.

### Why not expose many typed C++ structs directly?
Because the likely long-term caller is MicroPython, and binding dozens of nested structs is more brittle than binding a small number of value containers.

### Why not go fully JSON?
Because an in-memory dict/list/value model is cheaper and cleaner than forcing serialization/deserialization at every call.

---

## Recommended ownership split

### Host owns
- business flow decisions
- domain/application state
- async task results
- camera pipeline control
- validation and action consequences

### UI runtime owns
- LVGL objects and widget trees
- focus state
- local interaction state
- transient animations
- view-only cached layout state
- temporary modal presentation state

### Shared by contract
- screen view models
- event payloads
- route arguments
- frame descriptors

---

## Open questions for later phases

1. Which routes deserve explicit typed wrappers in addition to the generic API?
2. Do some high-frequency topics need a separate fast path from generic `publish()`?
3. Should `next_event()` support filtering by event type for tight polling loops?
4. What exact patch semantics are best: merge-patch, replace-by-key, or typed field setters?
5. Which screens need a stronger state schema document in Phase 1?
6. How should camera frame ownership interact with LVGL image buffer expectations?

---

## Final recommendation

Build around a **controller + route + payload + event queue** model.

Concretely:
- use a **small set of generic control methods**
- use **stable string IDs** for routes, modals, components, and actions
- use **dict/list/primitive payloads** rather than a huge screen-specific method surface
- use **pollable structured events** as the default outbound mechanism
- support **full set** and **incremental patch** update paths
- reserve specialized APIs only for clearly performance-sensitive paths such as frame ingestion

This is the most practical API shape for:
- future MicroPython on ESP32-P4 / S3
- maintainable C++ implementation
- later binding generation
- keeping business flow outside the UI module
