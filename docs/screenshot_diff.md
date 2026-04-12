# Screenshot Visual Regression

This document describes the screenshot diff / visual regression tooling added
in support of issue #69, with multi-profile coverage added in issue #73.

## Overview

The `tools/screenshot_diff.py` script compares PNG screenshots produced by the
`screenshot_tests` binary against a committed golden baseline and generates:

- **Pixel-diff images** in `screenshots_diff/<profile>/` (red intensity = magnitude of per-pixel change)
- **JSON report** at `screenshot_diff_report.json` with per-image diff scores, grouped by display profile

## Display Profiles

Screenshots are captured **per display profile**, producing output in
profile-scoped subdirectories under `screenshots/`:

| Profile | Directory | Resolution |
|---------|-----------|------------|
| Square 240×240 | `screenshots/square_240x240/` | Original SeedSigner hardware |
| Portrait 240×320 | `screenshots/portrait_240x320/` | Waveshare-style portrait hat (default) |

The **default portrait profile** is also copied as flat files into `screenshots/`
for backward compatibility with tools or scripts that expect the old flat layout.

## Quick Start

The top-level `Makefile` provides convenience targets:

```bash
# Build, capture screenshots, and diff against baseline (one command)
make screenshot-diff

# Just build and capture screenshots
make screenshots

# Refresh baselines after intentional UI changes
make update-screenshots
git add screenshots_baseline/

# Clean generated screenshots (keeps baselines)
make clean-screenshots
```

### Manual / step-by-step

```bash
# Build and capture screenshots
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
./build/screenshot_tests      # writes to screenshots/ at repo root

# Seed the baseline (first time only)
python3 tools/screenshot_diff.py --update-baseline

# Run diff against baseline
python3 tools/screenshot_diff.py
```

## How It Works

1. `screenshot_tests` renders each screen for **each display profile** to
   `screenshots/<profile>/*.png` via the headless LVGL framebuffer capture path.
   Each screen runs in a **forked child process** to prevent LVGL state leakage
   between captures. The `RuntimeConfig` (width/height) is set per profile so
   that LVGL and the `DisplayProfile` system configure correctly.
2. Output is always written under `screenshots/` **at the repo root**, regardless
   of the build directory or CWD. This is achieved via a `SOURCE_ROOT` compile
   definition injected by CMake. The default portrait profile is also copied as
   flat files into `screenshots/` for backward compatibility.
3. `screenshot_diff.py` iterates over profile subdirectories, comparing every
   PNG against the matching file in `screenshots_baseline/<profile>/`.
4. For each pair it computes a **mean absolute pixel difference** (0–255 scale
   across all RGBA bytes) and writes a diff image highlighting changed pixels.
5. If any image's score exceeds the threshold (default 1.0), the tool exits
   with code 1.

## Updating the Baseline

When intentional visual changes land:

```bash
make update-screenshots
git add screenshots_baseline/
```

## Threshold Tuning

```bash
# Stricter: fail on any sub-pixel difference
python3 tools/screenshot_diff.py --threshold 0.5

# Lenient: allow minor anti-aliasing changes
python3 tools/screenshot_diff.py --threshold 3.0

# Relaxed mode (threshold 5.0): for cross-platform / cross-compiler builds
python3 tools/screenshot_diff.py --relaxed
```

## Non-Deterministic Rendering

Pixel-exact reproducibility across different build environments is **not
guaranteed**. Known sources of variation include:

- **Font hinting / anti-aliasing**: Different FreeType builds or LVGL font
  configs may produce subtly different glyph rasterisation.
- **Floating-point rounding**: LVGL's drawing pipeline may vary by platform or
  compiler optimisation level.
- **QR code rendering**: `QRDisplayScreen` output depends on the QR library's
  module sizing, which should be deterministic but is worth re-baselining after
  dependency upgrades.

### Recommendations

- **CI baselines should be generated on the same platform/toolchain** used for
  regular builds. The `Makefile` targets ensure the output path is consistent.
- **Use `--relaxed`** in CI if builds run on heterogeneous runners (e.g., ARM
  vs x86, different GCC versions). This raises the threshold to 5.0, tolerating
  minor rendering differences while still catching real regressions.
- **Re-baseline after dependency bumps** (LVGL, FreeType, etc.) — the diff tool
  will flag these, and `make update-screenshots` makes refreshing painless.

## Attribution

The pixel-diff approach is inspired by the screenshot regression pipeline used
in the [seedsigner-c-modules](https://github.com/jdlcdl/seedsigner-c-modules)
project, which compares LVGL framebuffer captures against committed golden
images. No code was directly copied; only the general idea of a per-pixel
absolute-difference image with a threshold gate is reused here.

## Files

| Path | Purpose |
|------|---------|
| `Makefile` | Top-level convenience targets |
| `tools/screenshot_diff.py` | Diff script (multi-profile aware) |
| `screenshots/<profile>/` | Profile-scoped generated screenshots |
| `screenshots/*.png` | Flat backward-compat copy of default profile |
| `screenshots_baseline/<profile>/` | Golden reference images per profile (committed) |
| `screenshots_diff/<profile>/` | Generated diff images per profile (git-ignored) |
| `screenshot_diff_report.json` | Machine-readable report (git-ignored) |
