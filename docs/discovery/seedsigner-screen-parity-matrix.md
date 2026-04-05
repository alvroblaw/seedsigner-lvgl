# SeedSigner screen parity matrix

First practical parity matrix for `seedsigner-lvgl` after landing the initial reusable menu/result/scan slices.

This is intentionally conservative:
- it tracks what is actually present in `main`
- it distinguishes functional vs interaction vs visual parity
- it calls out reusable-family coverage separately from full SeedSigner flow parity
- it does **not** claim business-flow parity where only a primitive exists

## Current implementation baseline

Implemented in `main` today:
- `MenuListScreen`
- `ResultScreen`
- `CameraPreviewScreen`
- `PlaceholderScreen` (development stub, not a parity target)
- `SettingsSelectionScreen` (first real settings-route slice)
- runtime hooks for input events, outbound screen events, full/patch screen data, and external frame injection

## Status scale

- **none** — no meaningful LVGL implementation yet
- **primitive** — reusable building block exists, but not a SeedSigner-specific screen family yet
- **partial** — some real user-facing behavior exists, but major flow or UI gaps remain
- **near** — mostly usable for the intended family, with a known short gap list

## Parity dimensions

- **functional parity** — can it do the core job?
- **interaction parity** — does button/focus/navigation behavior resemble SeedSigner?
- **visual parity** — does it look materially like SeedSigner rather than just proving the flow?

## Matrix

| SeedSigner screen / family | Category | Complexity | Current support status | Blockers / dependencies | Parity notes |
| --- | --- | --- | --- | --- | --- |
| Main menu / generic button-list family (`MainMenuScreen`, Seeds/Tools/Settings menus, signer pickers, simple selectors) | Reusable list / navigation | Medium | **partial** via `MenuListScreen` primitive | Need top nav, icons, scroll affordances, long-label handling, per-item metadata, consistent focus visuals | **Functional:** partial for simple lists. **Interaction:** partial; up/down/press/back exist. **Visual:** low; current screen is a plain LVGL list, not SeedSigner-styled. |
| Power options / other simple 2-item action lists | Reusable list / navigation | Low | **partial** via `MenuListScreen` primitive | Need large-button variant and top-nav affordances | Likely easy consumers of the current primitive, but not implemented as real app routes yet. |
| Generic result / success / informational status screens | Status / result | Low-Medium | **partial** via `ResultScreen` primitive | Need warning/error variants, icons, richer layout states, action button styling | **Functional:** partial; title/body/continue action works. **Interaction:** partial; press/back events work. **Visual:** low. |
| Generic warning / dire warning / error family | Status / warning | Medium | **primitive-adjacent only** | Need severity-specific layouts, warning chrome, iconography, sensitive-state styling | `ResultScreen` is a base, but current main does not yet represent SeedSigner warning screens faithfully. |
| QR scan live preview (`ScanScreen`) | Camera-backed scan | High | **partial** via `CameraPreviewScreen` | Need real preview rendering path, scan progress overlay, multipart decode feedback, back overlay, scanner/decoder integration | **Functional:** partial only as an externally driven preview/capture shell. **Interaction:** partial; press emits `capture`, back cancels. **Visual:** very low relative to SeedSigner. |
| Settings QR ingest / SeedQR rescan confirmation / any flow that reuses `ScanScreen` | Camera-backed routed flow | High | **depends on partial scan primitive** | Need host routing contracts and decode/result plumbing | Current support is only the shared preview primitive, not the actual routed product flows. |
| Tools image entropy live preview | Camera-backed capture | High | **primitive-adjacent only** | Needs capture/review flow, full-bleed preview, instruction overlays, entropy-specific UX | `CameraPreviewScreen` proves frame injection and capture events, but not the entropy workflow. |
| I/O test with optional camera background | Hardware test / camera-adjacent | Medium-High | **none** | Need hardware-state rendering, key visualizations, optional preview background | No dedicated implementation yet. |
| Seed selector / signer selector screens with mixed loaded-seed + action entries | Structured list / hybrid selection | Medium | **primitive-adjacent only** | Need richer row models, fingerprints, secondary text, possibly icons and sectioning | Basic list mechanics exist, but real SeedSigner selector density is not covered yet. |
| Startup splash / screensaver | App shell / system | Medium | **none** | Assets, timing/animation, app-shell lifecycle | No parity work started. |
| Main shell top nav / global escape affordances | App shell / navigation chrome | High | **none** | Shared shell layout, icon assets, nav model, focus rules | This is a broad dependency for many screens feeling like SeedSigner. |
| QR display family (`QRDisplayScreen`, signed PSBT QR, xpub QR, SeedQR whole QR) | QR presentation | High | **none** | QR renderer, animated/static QR handling, sizing/brightness policy, page controls | Major parity block for PSBT/export flows. |
| Seed word entry / passphrase entry / coin-flip / dice / numeric-entry keyboards | Data entry / keyboard | Very High | **none** | Reusable keyboard framework, alternate layouts, cursor model, text editing, side soft-buttons | Biggest missing interaction family after simple menus/scan shell. |
| Transaction review family (`PSBTOverviewScreen`, math/details/finalize) | Transaction review | Very High | **none** | Bitcoin formatting widgets, diagrams, address formatting, review pagination, approval UX | No meaningful parity started. |
| Seed reveal / backup / transcription / verification custom screens | Sensitive seed management | Very High | **none** | Warning chrome, pagination, QR display, keyboard/input, custom overlays | Large domain still untouched beyond generic primitives. |
| Settings entry update selection / locale selection | Settings / structured lists | Medium | **partial** via `SettingsSelectionScreen` | Still needs global SeedSigner chrome, per-setting widgets beyond lists, and downstream route wiring into real settings definitions | The settings route now has an explicit host-side definition bridge (`SettingDefinition` / `SettingItemDefinition`), stable `setting_id` / `setting_type` / defaults/current-value route args, per-item `item_type` metadata, auto checkbox rendering for multi/toggle rows, and richer action payload strings derived from the same schema. It remains a narrow routed slice rather than full settings-flow parity. |
| Address explorer lists / address detail export | Tools / structured data views | High | **none** | Fixed-width address rows, pagination, QR display, derivation/fingerprint widgets | Blocked on formatted data components and QR display. |

## What is genuinely covered now

### Landed and usable as engineering primitives

1. **Host-driven list selection shell**
   - list of items
   - selected index
   - up/down/press/back input handling
   - outbound events for focus change and selection

2. **First real settings-selection route**
   - title + subtitle framing
   - optional section heading
   - richer rows with current-value/accessory semantics
   - explicit `single` vs `multi` selection modes
   - `current_value` / `current_values` route args for current state
   - checkbox rendering for multi-select rows
   - optional help/footer copy regions
   - single-select and multi-select `setting_selected` outbound action payloads

3. **Host-driven result/info shell**
   - title/body rendering
   - full replace + patch update paths
   - confirm/cancel style event emission

4. **Host-driven camera preview shell**
   - external frame ingestion
   - preview metadata updates
   - explicit `needs_data(camera.frame)` handshake
   - capture/cancel event emission

5. **Runtime control surface that matches the external-control architecture**
   - `send_input(...)`
   - `set_screen_data(...)`
   - `patch_screen_data(...)`
   - `push_frame(...)`

## What is still missing before "real SeedSigner feel"

### Visual parity gaps

Across all current screens, the project still lacks:
- top navigation bar and escape affordances
- SeedSigner iconography and branded assets
- SeedSigner typography/layout treatment
- warning/dire-warning styling
- polished focus/selection visuals
- QR-specific rendering surfaces

### Interaction parity gaps

The current code proves button-driven control, but SeedSigner-specific interaction still needs:
- richer list-row behavior
- scroll-arrow / paged-list conventions
- soft-key and top-nav conventions
- keyboard/text-entry frameworks
- route-specific multi-step flows rather than isolated demo screens

### Functional parity gaps

The current scan path is still a **preview + capture shell**, not a scanner product feature. Missing pieces include:
- actual QR decode integration
- multipart progress state and feedback
- typed routing by payload kind
- real downstream flows for seeds, PSBTs, settings, addresses, and messages

## Recommended next implementation block

**Build the reusable SeedSigner-style structured list family on top of `MenuListScreen`.**

Why this next:
- it unlocks the largest number of real product surfaces fastest
- many SeedSigner domains are list-first even when the later steps are custom
- it is materially cheaper than jumping straight into keyboard or PSBT review complexity
- it gives a better shell for Seeds / Tools / Settings / signer selection before deeper flow work

### Concrete scope for that block

1. Add a **top-nav + standard screen chrome** wrapper
2. Finish the list-family shell with:
   - optional icon support
   - scroll cues / pagination behavior
   - tighter SeedSigner visual treatment around the already-landed selection semantics
3. Add at least 2 real route implementations using the family:
   - main menu
   - settings menu or seed/signer selector
4. Keep event contracts external-controller-friendly

## Planning takeaway

The repo is now past pure architecture scaffolding: it has the first honest interaction primitives.
But parity is still **family-level, not product-level**. The next wins come from turning those primitives into a standard SeedSigner shell and richer list system, not from pretending the app already has flow parity.
