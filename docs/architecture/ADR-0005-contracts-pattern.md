# ADR-0005: Contract headers for screen‑specific arguments

## Status
Accepted

## Decision
Place screen‑specific argument parsing and serialisation in separate **contract headers** (e.g., `CameraContract.hpp`, `SeedWordsContract.hpp`) rather than embedding that logic directly inside screen classes.

## Why
The external API uses a generic `PropertyMap` (string‑keyed dictionary) for route arguments and screen data updates. Screens, however, need typed, validated structures to operate reliably.

Centralising this parsing in contract headers:
- keeps screen implementations focused on UI construction and interaction
- allows multiple screens (or other parts of the runtime) to reuse the same argument shapes
- provides a single place to evolve the serialisation format without touching screen code
- makes the mapping between external API and internal types explicit and searchable

Without contracts, each screen would duplicate parsing code, leading to inconsistency and harder maintenance.

## Consequences
### Positive
- Screens receive already‑validated, typed structures (e.g., `CameraParams`, `SeedWordsConfig`).
- Contract headers can be unit‑tested independently of UI rendering.
- The same contract can be used for both route arguments (`RouteDescriptor::args`) and screen data updates (`set_data`/`patch_data`).
- Adding a new screen only requires writing a new contract header if its argument shape isn’t already covered.

### Tradeoffs
- A small amount of boilerplate is needed for each new contract (typically a struct, a `make_*` function, and a `parse_*` function).
- Screens must include the contract header, creating a source‑level dependency. This is acceptable because the contract is part of the screen’s public interface.
- If a screen’s argument shape changes, both the contract header and the screen implementation must be updated. This is intentional – it forces explicit versioning of the external API.

## Examples
Current contract headers:
- `CameraContract` – camera format, resolution, FPS, buffer count
- `KeyboardContract` – keyboard layout, initial text, max length
- `PSBTDetailContract` – PSBT field selection, display flags
- `QRDisplayContract` – QR content, error correction level, size
- `SeedWordsContract` – word list, allowed lengths, validation rules
- `SettingsContract` – available settings entries, current values
- `StartupSplashContract` – version string, logo asset reference

Each contract provides:
- a `make_*` function that turns a typed struct into a `PropertyMap`
- a `parse_*` function that extracts a typed struct from a `PropertyMap`
- helper enums and constants relevant to that screen family

## Notes
Contracts are **not** responsible for rendering, layout, or interaction. They are purely data‑shape adapters between the external generic API and internal typed C++ structures.

Future work may add schema‑validation helpers or code generation for contracts, but the manual pattern is sufficient for the expected number of screen families.