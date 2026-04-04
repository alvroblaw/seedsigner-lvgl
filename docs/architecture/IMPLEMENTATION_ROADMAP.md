# Implementation Roadmap

## Purpose

This roadmap turns the Phase 0 scope into an implementation sequence that maximizes learning early, keeps pull requests small, and preserves the design-first direction established in the existing ADRs.

It is intentionally organized by **value delivered** and **risk retired**, not by trying to port SeedSigner screen-by-screen in bulk.

## Planning principles

Derived from `PROJECT_SCOPE.md`, `MILESTONES.md`, and the current ADRs:

- **External-first control wins over internal flow logic.** The UI module should render and emit events; higher-level orchestration should stay outside when possible.
- **Design risky interfaces before building many screens.** Camera ingestion, navigation commands, and MicroPython-friendly APIs are more important than early visual parity.
- **Ship vertical slices, not horizontal piles.** Each milestone should leave behind something demoable and reviewable.
- **Prefer reusable primitives over one-off ports.** Screen families should emerge from components and contracts.
- **Keep PRs mergeable.** Architecture docs first, then runtime skeleton, then narrow slices, then broader rollout.

## Recommended milestone ordering

## Milestone 0 — Architecture closure

**Goal:** finish the minimum design work needed so implementation can begin without creating a monolith.

**Why first:** ADR-0001 explicitly says discovery and architecture are first-class work. The biggest failure mode is building screens before the module boundary is clear.

**Required outputs:**

- screen taxonomy: which screens are mostly composition, input-heavy, or camera-aware
- draft external command/event model
- draft navigation state model
- draft camera frame contract
- implementation roadmap and validation plan

**PR shape:**

- PR 1: screen/flow inventory and grouping
- PR 2: command/event API draft
- PR 3: camera ingestion contract draft
- PR 4: roadmap + open questions consolidation

**Exit criteria:**

- the team can name the first vertical slice without debating fundamentals
- the likely public API surface is small enough to prototype
- risky assumptions are explicit

---

## Milestone 1 — Host-simulated runtime skeleton

**Goal:** create the smallest runnable C++/LVGL shell that can host externally controlled screens on a desktop/sim target before embedded hardening.

**Why now:** this delivers the fastest feedback loop and avoids discovering API problems only after ESP32 integration work.

**Scope:**

- C++ project layout and build system
- LVGL integration baseline
- `Screen` abstraction and lifecycle hooks
- minimal renderer/runtime shell
- host-side simulation harness
- placeholder external API entrypoints

**Value delivered:**

- codebase becomes runnable
- architecture starts being exercised in code
- future screen work has a stable landing zone

**PR shape:**

- PR 1: build + LVGL bootstrap + demo app
- PR 2: screen base classes and registry scaffolding
- PR 3: host simulator harness and smoke test
- PR 4: no-op external command sink and event sink interfaces

**Exit criteria:**

- a caller can boot the runtime, show a placeholder screen, and swap to another screen from outside the module

---

## Milestone 2 — External control contract and navigation core

**Goal:** prove the central architectural decision: external orchestration can drive navigation and receive user intent cleanly.

**Why before broader UI work:** if this contract is wrong, every later screen will be built on the wrong abstraction.

**Scope:**

- route/screen identifiers
- navigation commands (`show`, `push`, `pop`, `replace`, `update` or equivalent)
- event model for button selection, cancel/confirm, and text-like input completion
- active-screen payload/update contract
- internal vs external state ownership rules

**Value delivered:**

- the project’s defining differentiator is validated early
- MicroPython bridge design becomes much clearer

**PR shape:**

- PR 1: route identifiers and typed payload envelope
- PR 2: navigation controller and transition rules
- PR 3: event emission contract
- PR 4: example controller driving a toy flow end-to-end

**Exit criteria:**

- a simple multi-screen flow can be controlled entirely from outside the module
- screen interactions emit events without hidden business logic inside the UI module

---

## Milestone 3 — First vertical slice: external-controlled scan flow shell

**Recommendation:** the first vertical slice should be a **minimal scan flow shell** made of:

1. **Menu/List screen** — choose `Scan` or `Back`
2. **Camera preview screen** — accepts externally injected frames, exposes `capture` and `cancel` events
3. **Confirmation/result screen** — displays externally supplied text/status and offers continue/back actions

This is intentionally not a full QR decoding implementation. The slice is about **UI architecture, control flow, and camera contract**, not business completeness.

### Why this should be first

This slice validates the three highest-value requirements in one narrow path:

- **external-first orchestration** — menu selection and follow-up navigation are driven externally
- **camera/frame ingestion contract** — the most architecture-sensitive interface gets exercised immediately
- **representative screen diversity** — list/menu, live-updating camera view, and confirmation/status cover different UI behaviors

It also avoids premature complexity:

- no need to implement cryptography, signing, or full SeedSigner feature parity
- no need to commit to a large internal flow engine
- no need to solve every widget type up front

### Why not start with a text entry flow

A text or passphrase flow is important, but it mostly validates reusable widgets and interaction polish. The camera contract and external orchestration boundary are riskier and more likely to force rework if postponed.

### Why not start with a full seed or signing flow

That would couple UI work to domain logic too early, inflate PR size, and blur whether failures come from architecture, widgets, or business rules.

### Expected deliverable of this slice

A host-driven demo where:

- the controller shows a menu
- selecting `Scan` navigates to a camera preview screen
- the controller injects mock frames into the preview
- user input emits `capture` or `cancel`
- the controller responds by navigating to a confirmation/result screen with externally supplied text

That is enough to prove the key contracts without overbuilding.

**PR shape:**

- PR 1: menu/list screen with external item model
- PR 2: camera preview screen with mock frame injection API
- PR 3: result/confirmation screen with external payload updates
- PR 4: end-to-end demo flow and tests

**Exit criteria:**

- one end-to-end externally orchestrated flow works in the simulator
- camera screen accepts frame updates with explicit ownership/lifetime rules
- events and navigation are stable enough to build more screens on top

---

## Milestone 4 — Input-heavy primitives and forms

**Goal:** add the reusable interaction patterns needed for seed, PIN, and passphrase-adjacent screens.

**Why after the first slice:** once navigation and camera contracts are real, input widgets can be designed against a stable event/update model.

**Scope:**

- button rows and dialogs
- keyboard/text-entry widgets
- PIN/passphrase entry patterns
- key/value and status layouts
- progress and transient messaging primitives

**Value delivered:**

- unlocks a large family of non-camera SeedSigner-inspired screens
- helps normalize event payload conventions for user input

**PR shape:**

- one primitive/widget family per PR
- one reference demo screen per widget family
- keep domain logic out of the widget PRs

**Exit criteria:**

- input-heavy screens can be composed without inventing new interaction rules each time

---

## Milestone 5 — Screen family rollout by domain

**Goal:** expand coverage in coherent groups instead of porting isolated screens randomly.

**Recommended rollout order:**

1. **Navigation and settings family**
   - menus
   - settings lists
   - confirmations
   - status/info screens
2. **Data review family**
   - key/value review
   - summary screens
   - warning/error states
3. **Entry family**
   - PIN/passphrase entry
   - selection pickers
   - multi-step prompts
4. **Camera/QR family**
   - scan variants
   - camera-assisted overlays
   - QR/status combinations

**Why this order:**

- settings/review screens usually offer the fastest breadth through reusable components
- entry screens benefit from the primitives built in Milestone 4
- camera family expands only after the frame contract exists in real code

**PR shape:**

- one screen family at a time
- shared primitives extracted before adding more sibling screens
- no mega-PRs of “ten screens at once”

**Exit criteria:**

- each family is internally consistent and mostly composed from reusable building blocks

---

## Milestone 6 — Embedded and MicroPython hardening

**Goal:** validate that the architecture proven in simulation survives real target constraints.

**Scope:**

- ESP32-P4 / ESP32-S3 memory and performance validation
- buffer ownership and update cadence verification on target
- MicroPython binding/bridge hardening
- integration tests around command/event transport
- failure-path behavior under constrained resources

**Why last:** embedded optimization should refine a proven architecture, not substitute for one.

**PR shape:**

- platform bring-up and measurement PRs separated from UI feature PRs
- profiling and memory reports committed alongside code changes when useful

**Exit criteria:**

- target hardware can run the validated slices with acceptable responsiveness and memory use
- MicroPython-side control is practical, not theoretical

## Risky assumptions and validation order

The roadmap should retire assumptions in this order:

### 1. External-first navigation is expressive enough

**Risk:** the module may quietly need internal business orchestration to feel usable.

**Validate by:** implementing the Milestone 2 navigation/event contract and using it in the first vertical slice.

**Failure signal:** screen code starts depending on hidden global state or special-case callbacks.

---

### 2. The API shape is friendly to both C++ and MicroPython callers

**Risk:** a type-rich native API may become awkward or expensive to bind into MicroPython.

**Validate by:** keeping the command/event envelope minimal and building a host-side controller that mimics MicroPython usage patterns.

**Failure signal:** the API requires complex ownership semantics, deep inheritance, or many overloaded call paths.

---

### 3. Camera frame ingestion can be explicit without harming responsiveness

**Risk:** frame ownership, format conversion, or update cadence may be too awkward or too expensive for the desired targets.

**Validate by:** the first vertical slice using mock frame injection with documented lifetime rules, then measuring/update-testing on embedded targets later.

**Failure signal:** frame handoff semantics are unclear, copies are unavoidable, or the active screen contract becomes brittle.

---

### 4. Reusable component primitives will cover most SeedSigner-inspired screens

**Risk:** too many screens may need bespoke rendering, weakening the component strategy.

**Validate by:** rolling out menu/review/input screens after the first slice and tracking where composition breaks down.

**Failure signal:** every new screen requires a custom code path instead of assembling from primitives.

---

### 5. LVGL plus the chosen abstraction layer is light enough for ESP32 targets

**Risk:** the architecture may be clean but too heavy in RAM, draw cost, or update frequency.

**Validate by:** host-first implementation followed by milestone-specific profiling on ESP32-P4 / S3 once the first slices are stable.

**Failure signal:** acceptable behavior in simulation but repeated memory/performance regressions on target.

## Small, mergeable PR strategy

To preserve reviewability:

- Keep docs and ADRs in their own PRs.
- Introduce public interfaces before implementing many concrete screens.
- Land one primitive or one screen type at a time.
- Prefer demo flows that exercise architecture over unfinished broad ports.
- Extract reusable widgets only after two consumers prove the abstraction.
- Treat simulator/demo coverage as part of the definition of done for early milestones.

## Summary recommendation

If the project wants the **highest-value first implementation**, it should not begin by porting a full SeedSigner workflow.

It should begin with a **minimal external-controlled scan flow shell**:

- menu/list selection
- camera preview with injected frames
- confirmation/result screen
- external navigation and event loop end-to-end

That slice gives the best combination of:

- architectural validation
- camera contract validation
- externally controlled flow proof
- small PR boundaries
- low commitment to premature domain logic

Once that works, the project can safely expand into reusable input primitives and broader screen families without guessing at the core boundary decisions.
