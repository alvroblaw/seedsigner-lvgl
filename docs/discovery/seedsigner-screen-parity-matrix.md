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
- `SettingsSelectionScreen` and `SettingsMenuScreen` (settings route with host-side definition bridge)
- `WarningScreen` / `ErrorScreen` / `DireWarningScreen` (warning family)
- `QRDisplayScreen` (QR display with brightness controls)
- `KeyboardScreen` (generic text entry)
- `ScanScreen` (camera-backed scan with mock detection)
- `StartupSplashScreen` and `ScreensaverScreen` (app shell)
- `SeedWordsScreen` (seed word display with pagination)
- `PSBTOverviewScreen`, `PSBTMathScreen`, `PSBTDetailScreen` (transaction review family)
- `TopNavBar` component integrated across all main screens
- `InputProfile` for hardware input abstraction
- Desktop interactive runner (`SdlDisplay`, `ScenarioRunner`) with JSON scenario replay
- Per-display-profile screenshot coverage and visual regression diff tooling
- Runtime display-profile switching
- Runtime hooks for input events, outbound screen events, full/patch screen data, and external frame injection

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
| Generic warning / dire warning / error family | Status / warning | Medium | **implemented** via `WarningScreen`/`ErrorScreen`/`DireWarningScreen` family (PR #40) | Need SeedSigner visual styling and iconography | **Functional:** complete; title, body, severity levels, action buttons. **Interaction:** press/back events work. **Visual:** low; uses generic result styling. |
| QR scan live preview (`ScanScreen`) | Camera-backed scan | High | **implemented** via `ScanScreen` with mock detection (PR #44) | Need real QR decoder integration, progress overlays | **Functional:** mock detection works; frame injection, capture/cancel events. **Interaction:** press captures, back cancels. **Visual:** low relative to SeedSigner. |
| Settings QR ingest / SeedQR rescan confirmation / any flow that reuses `ScanScreen` | Camera-backed routed flow | High | **depends on implemented scan primitive** | Need host routing contracts and decode/result plumbing | Current support is the scan primitive, not yet full product flows. |
| Tools image entropy live preview | Camera-backed capture | High | **primitive-adjacent only** | Needs capture/review flow, full-bleed preview, instruction overlays, entropy-specific UX | `CameraPreviewScreen` proves frame injection and capture events, but not the entropy workflow. |
| I/O test with optional camera background | Hardware test / camera-adjacent | Medium-High | **none** | Need hardware-state rendering, key visualizations, optional preview background | No dedicated implementation yet. |
| Seed selector / signer selector screens with mixed loaded-seed + action entries | Structured list / hybrid selection | Medium | **primitive-adjacent only** | Need richer row models, fingerprints, secondary text, possibly icons and sectioning | Basic list mechanics exist, but real SeedSigner selector density is not covered yet. |
| Startup splash / screensaver | App shell / system | Medium | **implemented** via StartupSplashScreen and ScreensaverScreen (PR #52) | Assets, timing/animation, app-shell lifecycle | **Functional:** splash animation, screensaver activation. **Interaction:** press to exit screensaver. **Visual:** basic placeholder. |
| Main shell top nav / global escape affordances | App shell / navigation chrome | High | **implemented** via TopNavBar component integrated across all main screens (PR #51, #56) | Icon assets, SeedSigner visual styling | **Functional:** back/home/cancel buttons, title display. **Interaction:** button events, focus navigation. **Visual:** basic chrome; SeedSigner styling pending. |
| QR display family (`QRDisplayScreen`, signed PSBT QR, xpub QR, SeedQR whole QR) | QR presentation | High | **implemented** via `QRDisplayScreen` with brightness controls (PR #41) | Need QR renderer integration, page controls, animated QR support | **Functional:** brightness controls, QR display area, title/body. **Interaction:** button events for brightness, close. **Visual:** low; basic rectangle. |
| Seed word entry / passphrase entry / coin-flip / dice / numeric-entry keyboards | Data entry / keyboard | Very High | **implemented** via `KeyboardScreen` generic text entry (PR #42) | Need specialized layouts (coin-flip, dice), custom validation, side soft-buttons | **Functional:** generic text entry with cursor, backspace, submit. **Interaction:** up/down/left/right/press input. **Visual:** low; basic keyboard grid. |
| Transaction review family (`PSBTOverviewScreen`, math/details/finalize) | Transaction review | Very High | **implemented** via PSBTOverviewScreen, PSBTMathScreen, PSBTDetailScreen (PR #47, #53) | Bitcoin formatting widgets, address formatting, approval UX | **Functional:** transaction overview, input/output math, detail inspection. **Interaction:** navigation between screens, back/home. **Visual:** basic layouts; SeedSigner styling pending. |
| Seed reveal / backup / transcription / verification custom screens | Sensitive seed management | Very High | **partial** via SeedWordsScreen with pagination and warning styling (PR #46) | QR display, keyboard/input, custom overlays | **Functional:** seed words display with pagination, warning overlays. **Interaction:** page navigation, back. **Visual:** basic; SeedSigner styling pending. |
| Settings menu screen (`SettingsMenuScreen`) | Settings / structured lists | Medium | **implemented** via `SettingsMenuScreen` (PR #33) | Need SeedSigner visual styling, per-setting widgets beyond lists | **Functional:** grouped settings menu with sections, selection events. **Interaction:** up/down/press/back. **Visual:** low; uses generic list styling. |
| Settings entry update selection / locale selection | Settings / structured lists | Medium | **implemented** via `SettingsMenuScreen` and `SettingsSelectionScreen` | Still needs global SeedSigner chrome, per-setting widgets beyond lists, and downstream route wiring into real settings definitions | The settings route now has an explicit host-side definition bridge (`SettingDefinition` / `SettingItemDefinition`), stable `setting_id` / `setting_type` / defaults/current-value route args, per-item `item_type` metadata, auto checkbox rendering for multi/toggle rows, and richer action payload strings derived from the same schema. It remains a narrow routed slice rather than full settings-flow parity. |
| Address explorer lists / address detail export | Tools / structured data views | High | **none** | Fixed-width address rows, pagination, QR display, derivation/fingerprint widgets | Blocked on formatted data components and QR display. |

## What is genuinely covered now

### Landed and usable as engineering primitives

1. **Host-driven list selection shell**
   - list of items
   - selected index
   - up/down/press/back input handling
   - outbound events for focus change and selection

2. **Settings screen family (`SettingsMenuScreen`, `SettingsSelectionScreen`)**
   - grouped settings menu with sections
   - single-choice and multi-choice selection screens
   - typed host-side definitions via `SettingDefinition` / `SettingItemDefinition`
   - up/down/press/back interaction
   - contract: `SettingsContract`

3. **Warning/Error/DireWarning screen family (`WarningScreen`, `ErrorScreen`, `DireWarningScreen`)**
   - severity-specific layouts (warning, error, dire warning)
   - title, body, action buttons
   - press/back events
   - contract: `WarningContract`

4. **QR display screen (`QRDisplayScreen`)**
   - QR display area with brightness controls
   - title, body, button events for brightness and close
   - contract: `QRDisplayContract`

5. **Keyboard input subsystem (`KeyboardScreen`)**
   - generic text entry with cursor navigation
   - backspace, submit, cancel
   - up/down/left/right/press input handling
   - contract: `KeyboardContract`

6. **Camera preview shell (`ScanScreen` with mock detection)**
   - external frame ingestion
   - preview metadata updates
   - explicit `needs_data(camera.frame)` handshake
   - capture/cancel event emission
   - mock QR detection for testing
   - contract: `CameraContract`

7. **Runtime control surface that matches the external-control architecture**
   - `send_input(...)`
   - `set_screen_data(...)`
   - `patch_screen_data(...)`
   - `push_frame(...)`
   - event queue and route activation

8. **Top navigation bar component (`TopNavBar`)**
   - back/home/cancel buttons
   - title display
   - integrated across all main screens
   - emits navigation events (back_requested, home_requested, cancel_requested)

9. **PSBT review screen family (`PSBTOverviewScreen`, `PSBTMathScreen`, `PSBTDetailScreen`)**
   - transaction overview
   - input/output math verification
   - detailed inspection of PSBT data
   - contracts: `PSBTOverviewContract`, `PSBTMathContract`, `PSBTDetailContract`

10. **Startup splash and screensaver (`StartupSplashScreen`, `ScreensaverScreen`)**
    - splash animation on startup
    - screensaver activation after idle
    - press to exit screensaver
    - contracts: `StartupSplashContract`, `ScreensaverContract`

11. **Seed words display screen (`SeedWordsScreen`)**
    - seed phrase pagination
    - warning overlays for security
    - contract: `SeedWordsContract`

### Implemented external API contracts
- `SettingsContract`: settings menu navigation plus single-choice and multi-choice selection/update
- `WarningContract`: warning, error, and dire warning screens
- `QRDisplayContract`: QR display with brightness controls
- `KeyboardContract`: generic text entry
- `CameraContract`: camera-backed screens and frame ingestion
- `PSBTOverviewContract`, `PSBTMathContract`, `PSBTDetailContract`: PSBT review screens
- `StartupSplashContract`, `ScreensaverContract`: startup and screensaver
- `SeedWordsContract`: seed words display

These contracts define the structured payloads and events for each screen family, keeping the external API small and stable while enabling rich screen-specific behavior.

## What is still missing before "real SeedSigner feel"

### Visual parity gaps

Across all current screens, the project still lacks:
- SeedSigner iconography and branded assets
- SeedSigner typography/layout treatment
- warning/dire-warning styling
- polished focus/selection visuals
- QR-specific rendering surfaces

### Interaction parity gaps

The current code proves button-driven control, but SeedSigner-specific interaction still needs:
- richer list-row behavior
- scroll-arrow / paged-list conventions
- keyboard/text-entry frameworks
- route-specific multi-step flows rather than isolated demo screens

### Functional parity gaps

The current scan path is still a **preview + capture shell**, not a scanner product feature. Missing pieces include:
- actual QR decode integration
- multipart progress state and feedback
- typed routing by payload kind
- real downstream flows for seeds, PSBTs, settings, addresses, and messages

## Recommended next implementation blocks

The project has moved beyond pure API scaffolding, so the next milestones should focus on converting landed primitives into higher-fidelity routed flows.

### 1. SeedSigner shell and list fidelity

Build the reusable SeedSigner-style structured list family on top of `MenuListScreen` and the already-landed `TopNavBar`.

Why first:
- it unlocks the largest number of real product surfaces fastest
- many SeedSigner domains are list-first even when later steps are custom
- it gives the project a visibly more authentic shell before deeper flow work

Concrete scope:
1. Add SeedSigner visual treatment for list rows, top-nav chrome, focus states, and scroll cues
2. Add icon-aware and metadata-aware row variants
3. Convert at least two real routes to this shell:
   - main menu
   - a selector-style list such as seed/signer selection

### 2. Real scan pipeline behind `ScanScreen`

Replace the current mock-detection path with an actual decode/result pipeline.

Concrete scope:
1. wire real QR decoder integration
2. surface multipart progress and failure states
3. emit typed route intents for PSBT, SeedQR, settings QR, and address flows

### 3. Visual polish pass across already-implemented families

Apply SeedSigner styling and assets to the screen families that already exist.

Concrete scope:
1. warning/dire-warning iconography and color treatment
2. QR display and seed-word presentation polish
3. PSBT screen typography, spacing, and fixed-width data formatting

## Planning takeaway

The repo is now past pure architecture scaffolding: it has the first honest interaction primitives.
But parity is still **family-level, not product-level**. The next wins come from turning those primitives into a standard SeedSigner shell and richer list system, not from pretending the app already has flow parity.
