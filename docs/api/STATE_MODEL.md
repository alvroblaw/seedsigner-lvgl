# State Model

## Purpose
Define the ownership boundary between host/application state and UI/runtime state for the external control API.

This document complements `docs/api/EXTERNAL_API.md`.

---

## Core principle

The UI runtime should render and manage interaction state, but it should not become the source of truth for application flow or domain data.

In short:
- **host owns meaning**
- **UI owns presentation**

---

## State classes

## 1. Host-owned application state

This is the source of truth outside the UI module.

Examples:
- current SeedSigner workflow step
- seed/passphrase/domain data
- validation rules and validation outcomes
- QR decode progress/state machine
- camera session control state
- long-running task results
- security-sensitive values and policy decisions

Characteristics:
- survives screen transitions
- may outlive the UI runtime
- drives navigation decisions
- should be injected into screens explicitly

---

## 2. Screen view-model state

This is the structured data the host provides so the UI can render a particular screen.

Examples:
- menu item list
- title/subtitle/body text
- form field labels and current values
- button labels and enabled/disabled state
- review rows
- progress percentage and status text
- current scan overlay text/hints

Characteristics:
- scoped to one screen instance
- may be replaced wholesale or patched incrementally
- should contain display-oriented data, not hidden business logic

Recommended rule:
- if the host would care about the data after leaving the screen, it probably belongs in host-owned application state
- if the data mainly exists to render the current screen, it belongs in screen view-model state

---

## 3. UI-owned ephemeral interaction state

This state lives inside the UI runtime and should usually not be externally managed field by field.

Examples:
- focused widget/component
- highlighted list index before commit
- caret position
- scroll offset
- key repeat timers
- pressed/hovered animation state
- modal transition animation progress
- internal layout cache

Characteristics:
- highly transient
- presentation-specific
- should not require host synchronization

The host may influence some of it indirectly through route options or explicit focus hints, but should not own it.

---

## 4. Shared session/flow context

A small amount of shared context may be useful across multiple screens.

Examples:
- current network/device status
- theme selection
- locale
- battery/temperature indicators
- active camera availability

This should be modeled explicitly via `set_shared_state(scope, data)` or equivalent, not by letting screens read arbitrary global state.

---

## Data movement rules

## Rule 1: navigation carries only setup arguments

`activate(route_id, args)` should provide enough data to create the initial screen, but it should not be treated as a permanent hidden state bag.

If the screen needs later updates, use:
- `set_screen_data(...)`, or
- `patch_screen_data(...)`

---

## Rule 2: user actions exit as events

The UI runtime should emit intent to the host.

Examples:
- `submit_requested`
- `cancel_requested`
- `action_invoked`
- `field_changed`
- `modal_result`

The host then decides whether to:
- accept the action
- reject it
- navigate elsewhere
- patch the current screen
- show an error modal

---

## Rule 3: validation authority stays outside by default

The UI runtime may perform local presentational validation, such as:
- field length caps
- keyboard character filtering
- simple formatting constraints

But business validation should remain host-owned.

Examples:
- whether a PIN/passphrase is acceptable
- whether scanned data is complete
- whether a destructive action is allowed

Resulting errors should come back into the UI as state updates or modals.

---

## Rule 4: long-lived async work is host-owned

Examples:
- QR decoding pipeline
- camera acquisition/control
- cryptographic work
- storage reads/writes
- network or hardware checks

The UI runtime may display progress and let the user cancel, but should not become the job controller unless there is a very strong reason.

---

## Screen instance model

Each `activate()` creates a logical screen instance identified by a `screen_token`.

A screen instance has:
- `route_id`
- initial `args`
- current view-model data
- UI-owned ephemeral state
- zero or more attached modals

Recommended lifecycle:
1. host calls `activate(route_id, args)`
2. UI runtime creates screen instance
3. UI emits `route_activated` and/or `screen_ready`
4. host may call `set_screen_data()` or `patch_screen_data()`
5. user interaction emits events
6. host navigates away or the route is closed
7. UI emits `route_closed`

This tokenized model matters because route IDs alone may not be unique if a screen type is reused.

---

## Update patterns

## Full replacement

Use when the host owns the whole model and the screen is cheap to redraw conceptually.

Example:

```json
{
  "title": "Select Seed",
  "items": [
    {"id": "seed_1", "label": "Seed 1"},
    {"id": "seed_2", "label": "Seed 2"}
  ]
}
```

Best for:
- menus
- review screens
- static confirm pages
- simple forms

---

## Incremental patch

Use when part of the screen changes over time.

Example patch:

```json
{
  "progress": 0.8,
  "status_text": "Scanning 4/5",
  "actions.retry.enabled": false
}
```

Best for:
- progress pages
- scan overlays
- async field validation
- status banners
- enabling/disabling actions

Implementation note:
The exact patch syntax can be decided later, but it should be simple enough to bind easily.

---

## Event-driven transient updates

Use for streams or ephemeral updates that do not fit well into a stable view model.

Examples:
- camera preview frame
- temporary toast/banner message
- rapidly changing signal quality metrics

These may use topic-oriented delivery such as `publish(target, topic, payload)`.

---

## Modal state

Modal data follows the same model as screen data:
- host provides initial modal data
- UI owns focus/animation/stacking details
- user actions emit modal events
- host decides next business action

Recommended principle:
modals should not silently mutate underlying application state by themselves.

---

## Sensitive data guidance

Because SeedSigner-style flows can involve sensitive input, state design should assume some data is high risk.

Guidelines:
- keep the minimum sensitive data inside the UI runtime
- avoid duplicating secrets unnecessarily across screen, shared, and modal state
- prefer short-lived view models for sensitive entry/review screens
- make buffer/data lifetime rules explicit in later implementation docs

Phase 0 note:
This document does not define secure memory handling, but the API should avoid making it harder.

---

## Recommended defaults

When uncertain, prefer these defaults:

- flow/business state -> host-owned
- per-screen display content -> screen view model
- focus/caret/scroll/highlight -> UI-owned
- async task progress -> host-owned, reflected into screen via patch
- modal outcome -> event to host, not direct business mutation
- camera frames -> external push path, not hidden internal capture state

---

## What should stay internal even in an external-first architecture?

The UI runtime should still own enough state to feel responsive and self-contained at the interaction level.

That includes:
- focus navigation between widgets
- keyboard editing behavior
- press/release repeat handling
- local selection movement before confirmation
- LVGL object lifetime and redraw behavior

External-first should not mean micromanaging every cursor move from the host.

---

## Summary

A good split is:

- **host owns durable meaning and decisions**
- **screen view models carry renderable state in and out**
- **UI runtime owns transient interaction mechanics**

That split keeps the API practical for MicroPython, avoids a bloated internal flow engine, and fits the external-first direction already chosen in ADR-0002.
