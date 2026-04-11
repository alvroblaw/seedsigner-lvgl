# Screenshot Visual Regression

This document describes the screenshot diff / visual regression tooling added
in support of issue #69.

## Overview

The `tools/screenshot_diff.py` script compares PNG screenshots produced by the
`screenshot_tests` binary against a committed golden baseline and generates:

- **Pixel-diff images** in `screenshots_diff/` (red intensity = magnitude of per-pixel change)
- **JSON report** at `screenshot_diff_report.json` with per-image diff scores

## Quick Start

```bash
# Build and capture screenshots
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
./build/screenshot_tests

# Seed the baseline (first time only)
python3 tools/screenshot_diff.py --update-baseline

# Run diff against baseline
python3 tools/screenshot_diff.py
```

## How It Works

1. `screenshot_tests` renders each screen to `screenshots/*.png` via the
   headless LVGL framebuffer capture path.
2. `screenshot_diff.py` compares every PNG against the matching file in
   `screenshots_baseline/`.
3. For each pair it computes a **mean absolute pixel difference** (0–255 scale
   across all RGBA bytes) and writes a diff image highlighting changed pixels.
4. If any image's score exceeds the threshold (default 1.0), the tool exits
   with code 1.

## Updating the Baseline

When intentional visual changes land:

```bash
python3 tools/screenshot_diff.py --update-baseline
git add screenshots_baseline/
```

## Threshold Tuning

```bash
# Stricter: fail on any sub-pixel difference
python3 tools/screenshot_diff.py --threshold 0.5

# Lenient: allow minor anti-aliasing changes
python3 tools/screenshot_diff.py --threshold 3.0
```

## Attribution

The pixel-diff approach is inspired by the screenshot regression pipeline used
in the [seedsigner-c-modules](https://github.com/jdlcdl/seedsigner-c-modules)
project, which compares LVGL framebuffer captures against committed golden
images. No code was directly copied; only the general idea of a per-pixel
absolute-difference image with a threshold gate is reused here.

## Files

| Path | Purpose |
|------|---------|
| `tools/screenshot_diff.py` | Diff script |
| `screenshots_baseline/` | Golden reference images (committed) |
| `screenshots_diff/` | Generated diff images (git-ignored) |
| `screenshot_diff_report.json` | Machine-readable report (git-ignored) |
