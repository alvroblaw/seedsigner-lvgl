# Camera Frame Ingestion Contract

## Status
Proposed for Phase 0

## Purpose
Define how camera-backed screens receive image frames from an external producer while keeping the UI module reusable from both native C++ and a future MicroPython controller.

This contract is intentionally about **ingestion into the UI module**, not about camera driver details. The camera may be owned by:
- native firmware code
- a MicroPython wrapper
- a test/simulation harness
- a future transport/proxy layer

The UI module only needs a practical, explicit way to accept frames and decide when to redraw.

---

## 1. Scope

This contract applies to screens that display live or near-live camera data, including:
- preview surfaces
- alignment guides over camera imagery
- QR scanning views
- capture-review or frozen-frame views sourced from the camera pipeline

This contract does **not** define:
- camera sensor setup
- autofocus/exposure APIs
- decoder logic
- image processing internals
- persistent image storage

Those may exist elsewhere, but the UI-facing ingestion boundary should remain stable.

---

## 2. Design goals

1. **External-first ownership**  
   The UI module must accept frames from outside instead of owning the camera stack.

2. **MicroPython-friendly**  
   The minimal viable path must work even when the caller cannot safely participate in deep zero-copy pointer lifetimes.

3. **ESP32-realistic**  
   The contract must acknowledge tight RAM, DMA constraints, PSRAM bandwidth, and CPU limits on ESP32-S3 and ESP32-P4.

4. **Upgradeable path**  
   Start with a safe copied-frame model, but leave room for a native zero-copy or borrowed-buffer path later.

5. **Predictable redraw behavior**  
   Frame arrival and screen invalidation must be explicit so the UI can throttle work and avoid pathological refresh loops.

---

## 3. Recommended ingestion model

## Recommendation summary
Use a **push-based `submit_frame()` model with a UI-owned staging buffer**, plus an optional future borrowed-buffer/native fast path.

In practice:
- the external producer pushes frames into the active camera surface
- the UI module copies the payload into a buffer it owns
- the UI marks the relevant display region dirty
- LVGL renders from the last accepted frame on its own schedule
- only the latest frame matters; old frames may be dropped

This should be the **default and required baseline contract** for Phase 1+.

## Why this is the default
It is the most robust option for a MicroPython-controlled system because:
- ownership is simple
- lifetime is unambiguous
- the UI module never depends on external memory staying alive after the call returns
- it avoids exposing fragile pointer/GC interactions across binding layers
- it works with host simulation and test fixtures

## Why not require zero-copy first
Zero-copy looks attractive but is risky as a baseline because:
- LVGL rendering is not synchronous with the external producer
- the camera buffer may be recycled by the driver immediately after callback return
- MicroPython cannot be expected to guarantee stable DMA-safe backing memory or release timing
- borrowed-buffer protocols become callback-heavy and easy to misuse

Zero-copy can still be added later as an optimization for native integrations.

---

## 4. Contract model

The camera/UI boundary should conceptually expose the following objects and responsibilities.

## 4.1 Camera surface registration
Before frames are pushed, the external controller configures a camera-capable UI element/screen with:
- logical surface identifier
- intended frame dimensions
- frame pixel format
- orientation / mirroring metadata if needed
- expected update cadence range
- whether the surface is live-preview or frozen-review

This registration step lets the UI allocate the right internal buffers once instead of reallocating per frame.

## 4.2 Frame submission
For each new frame, the producer submits a frame descriptor containing at least:
- target surface id
- frame width and height
- pixel format
- stride or bytes-per-line
- payload pointer or byte span
- payload size
- monotonic frame sequence number or timestamp
- flags such as mirrored, rotated, partial, end-of-stream, or key/frozen

The UI returns an acceptance result such as:
- accepted and copied
- dropped because newer frame already pending
- rejected because format mismatch
- rejected because surface inactive
- rejected because buffer too large / unsupported

## 4.3 Invalidation and render trigger
A successful accepted frame should:
- update the surface's current image contents
- mark the corresponding UI region invalid
- avoid forcing immediate full-screen redraw when only the camera region changed

The UI runtime may coalesce multiple invalidations if frames arrive faster than LVGL refresh.

## 4.4 Freeze / resume semantics
Camera surfaces should support two modes:
- **live mode**: last submitted frame replaces prior content
- **frozen mode**: the current accepted frame stays visible until explicitly replaced or cleared

This is useful for scan success, review screens, and transitions where the controller wants to stop live updates without tearing down the whole screen.

## 4.5 Clear / detach semantics
The controller should be able to:
- clear the visible frame contents
- detach the surface from live camera input
- release large internal staging buffers when leaving camera-heavy flows

This matters on ESP32 where lingering PSRAM allocations can hurt the rest of the UI.

---

## 5. Ownership and lifetime rules

## 5.1 Baseline rule
For the baseline contract, **the submitted frame memory is borrowed only for the duration of `submit_frame()`**.

After the call returns:
- the caller may recycle or free the source buffer
- the UI module must not retain pointers into caller-owned memory
- the visible frame, if accepted, must live in UI-owned storage

This single rule removes most binding and driver lifetime ambiguity.

Current host implementation note:
- `CameraPreviewScreen::push_frame(...)` immediately copies the caller-provided grayscale bytes into screen-owned storage before any LVGL redraw happens
- the screen then rescales that retained frame into its own LVGL canvas buffer for presentation
- tests validate the rendered canvas output directly, not just metadata labels

## 5.2 Internal ownership
The UI module owns:
- the staging buffer or buffers used for active camera surfaces
- any converted render-ready copies
- metadata for the currently displayed frame

The UI module is responsible for releasing those resources when the surface or screen deactivates.

## 5.3 Future optimization path
A native-only extension may support **borrowed-frame presentation** where:
- the caller loans a buffer to the UI
- the UI explicitly signals when it is done with that buffer
- the caller must not reuse the buffer until release

This should be treated as an advanced mode, not the common contract.

---

## 6. Copy vs zero-copy tradeoffs

## 6.1 Copied-frame model
### Pros
- simplest semantics
- safe for MicroPython
- no external lifetime hazards
- easier to debug
- easy to drop stale frames and keep only latest
- easier to test on desktop/host builds

### Cons
- costs memory bandwidth every frame
- may require a full second buffer in PSRAM
- format conversion may add another pass

## 6.2 Borrowed-buffer / zero-copy model
### Pros
- avoids copy bandwidth
- may reduce latency
- useful for native camera-to-display pipelines on P4-class systems

### Cons
- fragile ownership protocol
- hard to bind cleanly to MicroPython
- camera DMA buffers may have alignment/placement restrictions
- UI redraw timing can outlive driver callback timing
- often still needs conversion or cache maintenance, so true zero-copy may not exist in practice

## 6.3 Decision
For Phase 0 architecture, the project should standardize on:
- **copy-in baseline API**
- **latest-frame-wins queueing policy**
- **optional future native borrowed-buffer optimization**

---

## 7. Frame cadence and backpressure

## 7.1 Latest-frame-wins
Camera preview surfaces should behave as **streaming state**, not as an ever-growing queue.

Default policy:
- keep at most one pending fresh frame per surface beyond the currently displayed one
- if producer outruns UI consumption, discard older pending frames
- prefer lower latency over guaranteed delivery

This is the right tradeoff for preview and scanning screens.

## 7.2 Cadence expectations
Practical preview targets:
- ESP32-S3: roughly low-to-moderate FPS preview, optimized for responsiveness rather than video smoothness
- ESP32-P4: higher headroom, but still should not assume desktop-style 30/60 FPS full-frame RGB redraws

The contract should not promise a fixed FPS. Instead it should support:
- bursty producer callbacks
- UI-side throttling
- frame dropping when necessary

## 7.3 UI invalidation cadence
The camera surface should only invalidate:
- its own bounding box, or
- a small known overlay region if guides/compositing changed

Avoid full-screen invalidation on every frame unless the whole screen genuinely depends on the frame.

## 7.4 Optional rate hints
The registration/config step may include a target preview rate hint such as:
- preferred max present rate
- preferred max ingest rate
- whether frame skipping is allowed

This is especially useful when MicroPython or decoder work shares the same CPU budget.

---

## 8. Pixel format strategy

The contract should distinguish between:
- **transport/source format**: what the camera or producer actually supplies
- **presentation format**: what the UI wants for efficient drawing

The baseline contract should require the frame descriptor to identify its source format explicitly.

## 8.1 Preferred baseline source formats
Recommended candidate formats for Phase 0 consideration:
- **RGB565**: best direct-display candidate when camera or conversion pipeline can provide it
- **Grayscale 8-bit**: good for QR/scanning pipelines and lower bandwidth previews
- **JPEG**: acceptable as a transport/compressed source only if decode happens outside the UI hot path or is explicitly budgeted
- **YUV variants**: acceptable as ingest formats only if a conversion stage is clearly assigned elsewhere

## 8.2 Presentation preference
For LVGL presentation on ESP32 targets, the preferred render-ready format is typically:
- **RGB565** for direct image presentation

Reason:
- aligns with common embedded display pipelines
- predictable memory layout
- avoids palette management complexity
- generally friendlier for small LCD targets than full RGB888

## 8.3 Grayscale use
Grayscale is attractive when:
- preview quality can be reduced
- the main user task is QR alignment or rough composition
- memory bandwidth is tight
- a later processing stage already wants luminance data

But the UI still needs a defined way to display it. That means either:
- convert to RGB565 on ingest/present, or
- support grayscale-aware drawing inside the camera surface widget

The second option may be worthwhile later, but the architecture should not assume it for Phase 0.

See also [FRAME_FORMATS.md](./FRAME_FORMATS.md).

---

## 9. Memory model for ESP32-P4 / ESP32-S3

## 9.1 Why this matters
Even modest frame sizes become expensive quickly.

Approximate raw frame sizes:
- 240 x 240 RGB565 ≈ 115 KB
- 320 x 240 RGB565 ≈ 150 KB
- 400 x 400 RGB565 ≈ 312 KB
- 240 x 240 grayscale ≈ 58 KB
- 320 x 240 grayscale ≈ 75 KB

Double-buffering or conversion staging can multiply those costs.

## 9.2 ESP32-S3 implications
On S3, assume tighter constraints:
- PSRAM likely needed for camera buffers and large UI assets
- bandwidth is limited enough that full-frame copy + convert every refresh can dominate
- preview resolution may need to be intentionally capped
- one stable UI-owned frame buffer per active camera surface is preferable to deeper queues

Practical consequence:
- avoid multiple full-size retained camera buffers unless justified
- prefer small overlay redraws over full-screen recomposition
- be cautious with JPEG decode inside the same critical path as UI refresh

## 9.3 ESP32-P4 implications
On P4, expect more headroom, but not unlimited headroom:
- more capable CPU/memory subsystem can tolerate higher preview rates and conversion work
- still beneficial to avoid unnecessary copies
- future native optimization path is more realistic here

Practical consequence:
- the same contract should scale upward
- P4 can exploit optional borrowed-buffer or hardware-assisted conversion later without changing the external API shape

## 9.4 Allocation guidance
The camera contract should encourage:
- allocate surface buffers at screen activation, not per frame
- reuse buffers across frame submissions while the same surface config remains active
- tear down aggressively when leaving camera flows
- fail early and explicitly if requested dimensions/format exceed supported memory budget

---

## 10. MicroPython practicality

## 10.1 Baseline expectation
MicroPython should not need to hold raw frame pointers alive across asynchronous LVGL usage.

Instead, the binding should allow something conceptually like:
- provide bytes / bytearray / memoryview for one frame
- call into native UI module
- native side copies and returns success/failure immediately

This is the most realistic interoperability model.

## 10.2 Why it matters
MicroPython callers are likely to struggle with:
- stable pinning of buffers
- DMA-safe allocation
- deterministic release callbacks
- pointer ownership bugs hidden by GC timing

A copy-in contract avoids designing the whole UI stack around those constraints.

## 10.3 Native escape hatch
If a future system runs most camera logic in native code, a second advanced API can expose borrowed/native buffer ingestion. But that should remain optional and invisible to simple MicroPython callers.

---

## 11. Suggested surface states

A camera surface should behave roughly like this:

1. **unconfigured**  
   no buffers allocated, no frames accepted

2. **configured**  
   format/dimensions known, buffers allocated, awaiting frames

3. **live**  
   new frames may replace current frame

4. **frozen**  
   current frame remains visible, new live frames rejected or ignored until resume

5. **detached**  
   surface still exists visually but no active producer is attached

6. **released**  
   buffers freed; reconfiguration required before use

This keeps lifecycle explicit without binding the project to a specific class hierarchy.

---

## 12. Error handling expectations

The ingestion boundary should report enough detail for debugging and adaptive behavior.

Useful rejection reasons include:
- unsupported pixel format
- dimensions exceed configured bounds
- stride mismatch
- surface not active
- frame arrived while frozen
- out of memory
- internal conversion unavailable
- producer too fast / frame dropped

For MicroPython integration, errors should be mappable to simple return codes or lightweight exceptions.

---

## 13. Contract decisions for Phase 0

## Adopt now
1. Camera frames are **pushed in from outside**.
2. The **default ingestion mode copies frame data into UI-owned memory**.
3. The submission lifetime is **call-scoped only**.
4. Camera preview uses **latest-frame-wins**, not deep queuing.
5. Successful frame acceptance triggers **region-scoped invalidation**.
6. The baseline preferred presentation format is **RGB565**.
7. The contract must also allow **grayscale** and explicitly tagged non-RGB sources.
8. Surface configuration/allocation happens **before streaming**, not per frame.
9. Freeze, clear, detach, and release semantics are first-class.

## Defer for later
1. Native borrowed-buffer / release-callback fast path
2. Partial-tile updates
3. Hardware-specific color conversion acceleration
4. Multi-surface concurrent camera composition
5. On-UI JPEG decode guarantees

---

## 14. Recommended first implementation shape

When implementation starts, prefer this order:

1. one camera surface widget/screen contract
2. configure once with width/height/format
3. submit copied RGB565 frames
4. mark only the camera region dirty
5. drop stale frames if producer outruns rendering
6. add grayscale support second
7. evaluate native borrowed-buffer optimization only after profiling on hardware

This gets to a usable and debuggable camera-backed screen with the fewest architecture regrets.

---

## 15. Final recommendation

For `seedsigner-lvgl`, the recommended ingestion contract is:

> **External producer pushes frames into a configured camera surface using a copy-in `submit_frame()` API. The UI module owns the displayed frame buffer, treats frame memory as valid only for the call duration, presents only the newest useful frame, and invalidates only the affected region.**

That is the best default fit for:
- LVGL integration
- ESP32-S3 memory realities
- future ESP32-P4 optimization
- MicroPython control
- host-side testing and simulation
