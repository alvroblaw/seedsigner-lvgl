# ADR-0004: Screen context for runtime services

## Status
Accepted

## Decision
Pass a `ScreenContext` struct to each screen during creation, providing runtime services (event emission, timestamps) as callbacks, rather than letting screens directly reference the `UiRuntime` or other global singletons.

## Why
Screens need to emit UI events and obtain timestamps, but tight coupling to the runtime would make them difficult to test, reuse, or move to a different runtime architecture later.

The `ScreenContext` approach:
- decouples screen implementations from the concrete runtime
- allows unit tests to supply mock callbacks without linking the full UI runtime
- makes the service dependencies explicit (event emission, time)
- keeps the screen API surface small (no need to pass many separate parameters)

Alternatives considered:
1. **Screens hold a reference to `UiRuntime`** – leads to circular dependencies and makes testing harder.
2. **Global event bus** – hides dependencies and makes sequencing/ownership unclear.
3. **Pass individual callbacks as separate parameters** – clutters the `create` signature and is less readable.

`ScreenContext` groups related runtime services into a single, copyable struct that can be extended later without breaking existing screens.

## Consequences
### Positive
- Screens are fully decoupled from the runtime; they only depend on `ScreenContext`.
- Unit tests can easily verify event emission by providing a capturing callback.
- Adding new runtime services (e.g., logging, asset loading) only requires extending the struct, not every screen factory.

### Tradeoffs
- `ScreenContext` must be kept small to avoid passing unnecessary baggage to every screen.
- The `emit_event` callback is a `std::function`, which adds a small runtime overhead compared to a direct function pointer or virtual call. This is acceptable given the low frequency of event emission.
- Screens must store the context if they need to emit events later (e.g., in response to input). This is intentional – the context is part of the screen’s environment.

## Notes
The context currently provides:
- `root` – LVGL root object for the screen
- `route_id` – identifier of the active route
- `screen_token` – unique token assigned by the navigation controller
- `emit_event` – callback to send a `UiEvent` upward
- `now_ms` – function returning current monotonic time in milliseconds

Future extensions could include:
- theme access
- asset loader
- platform‑specific helpers (e.g., vibration, sound)
- logging sink

Adding new fields should be done with care to avoid bloating the struct for screens that don’t need them.