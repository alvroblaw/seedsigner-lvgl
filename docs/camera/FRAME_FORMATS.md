# Camera Frame Format Notes

## Purpose
This note compares likely frame formats for camera-backed screens in `seedsigner-lvgl`.

It supports the main recommendation in [CAMERA_CONTRACT.md](./CAMERA_CONTRACT.md):
- baseline ingestion should be copy-in
- baseline presentation should prefer RGB565
- grayscale should be supported as an important secondary path
- compressed or planar formats should only be used when the conversion/decode owner is explicit

---

## Evaluation criteria

Formats are judged by:
- display friendliness in LVGL
- memory footprint
- copy bandwidth
- conversion cost
- camera availability on embedded sensors/modules
- practicality for MicroPython bindings
- suitability for ESP32-S3 vs ESP32-P4

---

## Summary table

| Format | Bytes/pixel | Typical role | Strengths | Main costs / risks | Recommendation |
|---|---:|---|---|---|---|
| RGB565 | 2 | direct presentation | simple, display-friendly, no palette issues | larger than grayscale, still expensive at higher resolutions | **Primary presentation format** |
| Grayscale 8-bit | 1 | preview / scan-oriented ingest | half the bandwidth of RGB565, good for QR-style use | usually needs conversion or grayscale-aware widget rendering | **Important secondary ingest format** |
| RGB888 | 3 | high-fidelity intermediate | simple semantics | too heavy for routine embedded preview on these targets | Avoid as baseline |
| JPEG | variable | compressed transport / snapshot | small transport size, common camera output option | decode cost, latency variance, extra scratch buffers | Use only with explicit decode ownership |
| YUV422 / YUYV | 2 | sensor/native camera output | common from camera interfaces | not directly ideal for LVGL presentation, conversion needed | Accept only if conversion plan is clear |
| YUV420 planar | 1.5 | codec/vision-oriented pipeline | storage-efficient | awkward layout, conversion complexity, multiple planes | Not a baseline UI format |
| Monochrome 1-bit | 0.125 | highly specialized | tiny footprint | too lossy for camera preview, poor generality | Not recommended |

---

## RGB565

## Why it fits
RGB565 is the most practical render-ready format for embedded LVGL previews because:
- it matches common display color depth expectations
- pixel addressing is simple
- it avoids palette handling
- it is widely understood by embedded graphics stacks

## Costs
The main cost is raw size.

Examples:
- 240 x 240 = about 115 KB
- 320 x 240 = about 150 KB
- 480 x 320 = about 300 KB

If the UI keeps one displayed frame plus one pending frame, cost doubles quickly.

## Recommendation
Use RGB565 as the **preferred presentation format**, and as the easiest first implementation format for camera-backed screens.

---

## Grayscale 8-bit

## Why it matters
Grayscale is compelling for SeedSigner-like camera use because many camera tasks are not about rich color fidelity. They are about:
- QR alignment
- framing and rough composition
- scan feedback
- low-cost live preview

For those tasks, grayscale often cuts memory and bandwidth in half versus RGB565.

## Costs
LVGL presentation is less straightforward unless:
- the UI converts grayscale to RGB565 on ingest, or
- the camera widget knows how to draw grayscale data directly

Either way, there is some additional architectural choice.

## Recommendation
Treat grayscale as the **best secondary ingest format** and likely the first format worth supporting after RGB565.

It is especially attractive on ESP32-S3 where memory bandwidth is tight.

---

## JPEG

## Why it is tempting
JPEG is attractive because many camera modules or upstream pipelines can provide compressed frames, reducing transport/storage size.

## Why it is dangerous as a baseline
JPEG is not a free win on ESP32-class systems because decode requires:
- CPU time
- temporary buffers
- latency budget
- predictable ownership of decoded output

If decode lands inside the same path that also drives UI updates, frame cadence becomes much less predictable.

## Recommendation
Allow JPEG only when the design explicitly says who decodes it:
- external producer decodes to RGB565/grayscale before submitting, or
- a dedicated native stage decodes with a known budget

Do **not** make JPEG the default camera UI contract.

---

## YUV family formats

## Why they appear
Many camera interfaces naturally produce YUV variants like YUYV or other packed/planar formats.

## Challenge
These are workable ingest formats but not ideal presentation formats. The UI stack still needs a conversion strategy before or during rendering.

## Recommendation
Permit them only as **explicitly tagged source formats** in the contract. Do not assume the camera surface can efficiently render them directly in the first implementation.

Good architecture rule:
- source format may be YUV
- presentation format should still converge toward RGB565 or a well-defined grayscale path

---

## RGB888

RGB888 is easy to reason about but usually too expensive here.

At 320 x 240 it is about 225 KB per frame, before any extra staging.
That makes it hard to justify for routine live preview on ESP32-S3 and still not especially appealing on P4.

Recommendation:
- avoid RGB888 as a standard live-preview format
- tolerate it only in host-side simulation or isolated tooling if needed

---

## Format guidance by target

## ESP32-S3
Prefer, in order:
1. grayscale 8-bit when user experience allows it
2. RGB565 when direct presentation simplicity matters
3. avoid JPEG decode in the hot path unless proven acceptable
4. avoid RGB888 for live preview

## ESP32-P4
Prefer, in order:
1. RGB565 as baseline presentation format
2. grayscale where bandwidth savings help the overall pipeline
3. consider YUV/JPEG native optimizations later, after profiling
4. still avoid defaulting to RGB888 for live preview

---

## Practical project guidance

For this repo, the most pragmatic progression is:
1. define contract around explicit source format metadata
2. implement copied RGB565 ingestion first
3. add copied grayscale ingestion second
4. defer YUV/JPEG handling until the conversion/decode owner is specified
5. only add zero-copy optimizations after hardware profiling proves the copy path is the bottleneck

---

## Final recommendation

If only one format is chosen for the first camera-backed screen path, choose:
- **RGB565** for baseline presentation

If one additional format is designed in from the start, choose:
- **Grayscale 8-bit** as the secondary ingest format

That pair gives the project the best balance of:
- UI simplicity
- memory realism
- MicroPython friendliness
- future optimization flexibility
