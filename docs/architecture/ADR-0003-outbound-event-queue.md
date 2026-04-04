# ADR-0003: Outbound UI event queue

## Status
Accepted

## Context
Phase 1 needs the first concrete runtime contract that proves the project can be externally orchestrated. The runtime must report screen lifecycle and user intent back to a host controller without depending on callback-heavy integration. That matters both for host simulation and for future MicroPython bindings on ESP32 targets.

The architectural docs already prefer polling as the primary retrieval path and explicitly call out an `EventQueue` runtime building block.

## Decision
Implement a small FIFO outbound event queue as a core `UiRuntime` dependency.

### Event transport
- Primary path: `UiRuntime::next_event()` polling
- Internal production path: `UiRuntime::emit(UiEvent)`
- Queue behavior: bounded FIFO with explicit push failure when full

### Event shape
Use a compact structured event model with:
- `EventType`
- `route_id`
- `screen_token`
- optional `component_id`
- optional `action_id`
- optional scalar `value`
- optional single `meta` key/value pair
- `timestamp_ms`

### Initial event types
Only define the minimum needed for early runtime work:
- `RouteActivated`
- `ScreenReady`
- `ActionInvoked`
- `CancelRequested`
- `NeedsData`
- `Error`

## Consequences
### Positive
- Matches the external-first architecture without overdesigning a large event taxonomy
- Keeps the binding surface simple for MicroPython and C-style shims
- Makes polling the default, while preserving room to add callback sinks later
- Gives placeholder screens and future real screens a common outbound path immediately

### Tradeoffs
- The initial scalar `value` and single `meta` entry are intentionally limited and may need widening later
- Full-queue behavior currently drops the new event by returning `false`; callers must decide whether to retry, log, or surface that condition

## Notes
This ADR intentionally stops short of defining generic patch semantics, callback registration, or a full navigation stack. Those remain separate milestones.
