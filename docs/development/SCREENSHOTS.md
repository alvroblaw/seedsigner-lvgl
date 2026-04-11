# Screenshot Capture System

Automated PNG capture of every SeedSigner-LVGL screen — no display server, no emulator window, just the headless framebuffer written to disk.

## What it does

The screenshot suite (`screenshot_tests`) renders each screen in isolation inside a headless LVGL runtime, then dumps the framebuffer to a PNG file. It produces a visual reference of every UI screen without needing physical hardware or a desktop simulator.

This is useful for:

- Visual regression checks during screen development
- Generating up-to-date screen images for documentation
- Validating layout, theming, and icon changes locally

## How it works

```
fork() → child process
  ├─ Create UiRuntime (headless HeadlessDisplay)
  ├─ Register route, activate screen with sample args
  ├─ Pump LVGL ticks & flush until stable
  └─ FramebufferCapture::write_png() → stb_image_write → PNG on disk
parent waits, reports [OK]/[FAIL]
```

**Fork isolation:** Each screen capture runs in its own child process. LVGL holds global state (object pools, theme caches, input drivers) that isn't designed to be torn down and recreated within a single process. Forking guarantees a clean LVGL instance per screen — no state leakage between captures.

**Framebuffer → PNG:** `FramebufferCapture` reads the raw RGB565 framebuffer from `HeadlessDisplay`, converts to 8-bit RGB, and writes via `stb_image_write`. No external image libraries required.

## Build and run

From the repo root:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/screenshot_tests
```

Prerequisites are the same as the host build — see [HOST_BUILD.md](HOST_BUILD.md).

## Output

PNG files land in `screenshots/` relative to the working directory:

```
screenshots/
├── 01_main_menu.png
├── 02_settings_menu.png
├── 03_settings_language.png
├── 04_warning.png
├── 05_error.png
├── 06_dire_warning.png
├── 07_result_success.png
├── 08_qr_display.png
├── 09_keyboard.png
├── 10_seed_words_page1.png
└── 11_psbt_overview.png
```

Run from the repo root so the `screenshots/` directory ends up in a predictable location.

## Adding a new screen capture

Open `tests/screenshot_tests.cpp` and add a new block before the final "Done" printf. Follow the existing pattern:

```cpp
// N. Your Screen
std::printf("[N/12] YourScreenName\n");
fork_capture([](UiRuntime& rt) {
    rt.screen_registry().register_route(
        RouteId{"your.route"},
        []() -> std::unique_ptr<Screen> {
            return std::make_unique<YourScreen>();
        });

    // Build your route args here
    RouteDescriptor rd;
    rd.route_id = RouteId{"your.route"};
    rd.args = {{"title", "Example"}, {"body", "Hello"}};
    rt.activate(rd);
}, "NN_your_screen_name");
```

Checklist:

1. Increment the counter in the printf (`[N/12]`, `[N/13]`, etc.)
2. Register a unique route ID
3. Populate route args with representative/realistic data
4. Pick a descriptive filename (zero-padded number prefix for sort order)
5. Build and run to verify `[OK]`

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| `[FAIL]` for every screen | Build issue — rebuild with `cmake --build build` and check for compile errors |
| Segfault in child | Route args missing or wrong type — compare with the screen's contract header |
| All-black PNGs | `UiRuntime::init()` failed or display dimensions are zero — check that `HeadlessDisplay` is linked |
| `stb_image_write` link error | Ensure `stb_image_write.h` is in the include path and `STB_IMAGE_WRITE_IMPLEMENTATION` is defined exactly once (in `FramebufferCapture.cpp`) |
| No `screenshots/` directory created | Run from repo root, or create the directory manually (`mkdir -p screenshots`) |
| Partial renders (missing text/icons) | Add extra `rt.tick()` / `rt.refresh_now()` cycles in the lambda before capture |

## Key source files

| File | Purpose |
|---|---|
| `tests/screenshot_tests.cpp` | Capture suite — one forked block per screen |
| `src/platform/FramebufferCapture.cpp` | Framebuffer → PNG conversion |
| `include/seedsigner_lvgl/platform/FramebufferCapture.hpp` | Public API |
| `include/seedsigner_lvgl/platform/HeadlessDisplay.hpp` | In-memory framebuffer display |

## CI integration

The suite exits with code 0 on full success, non-zero on any failure. It can be added to CI pipelines alongside `ctest`:

```bash
./build/screenshot_tests
if [ $? -ne 0 ]; then echo "Screenshot capture failed"; exit 1; fi
```

Future work may include golden-image comparison (pixel diff against committed baselines), but the current scope is capture-only.
