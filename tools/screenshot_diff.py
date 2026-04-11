#!/usr/bin/env python3
"""Screenshot diff / visual regression tool for seedsigner-lvgl.

Compares generated screenshots (``screenshots/``) against a golden baseline
(``screenshots_baseline/``) and produces:
  • per-image pixel-diff images in ``screenshots_diff/``
  • a compact JSON report (``screenshot_diff_report.json``)

Exit code is 0 when all images match or have zero diff; 1 on any mismatch
exceeding the threshold; 2 on missing dependencies.

Requirements: Pillow (``pip install Pillow``).

Usage examples:
  # Compare all screenshots against baseline
  python3 tools/screenshot_diff.py

  # Set current screenshots as the new baseline
  python3 tools/screenshot_diff.py --update-baseline

  # Custom threshold (0-255 mean absolute pixel difference per channel)
  python3 tools/screenshot_diff.py --threshold 2.0
"""

# ---------------------------------------------------------------------------
# Attribution: The pixel-diff approach is inspired by the screenshot regression
# pipeline used in the seedsigner-c-modules project
# (https://github.com/jdlcdl/seedsigner-c-modules), which compares LVGL
# framebuffer captures against committed golden images.  No code was directly
# copied; only the general idea of a per-pixel absolute-difference image and
# a simple threshold gate is reused here.
# ---------------------------------------------------------------------------

import argparse
import json
import os
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCREENSHOTS_DIR = ROOT / "screenshots"
BASELINE_DIR = ROOT / "screenshots_baseline"
DIFF_DIR = ROOT / "screenshots_diff"
REPORT_PATH = ROOT / "screenshot_diff_report.json"


def _ensure_pillow():
    try:
        from PIL import Image  # noqa: F811
        return Image
    except ImportError:
        print("Error: Pillow is required.  pip install Pillow", file=sys.stderr)
        sys.exit(2)


def _mean_abs_diff(a_bytes, b_bytes, count):
    """Return mean absolute difference across all bytes."""
    total = sum(abs(x - y) for x, y in zip(a_bytes, b_bytes))
    return total / max(count, 1)


def compare_images(image_path: Path, baseline_path: Path, diff_dir: Path, Image):
    """Compare *image_path* against *baseline_path*.

    Returns a dict with name, match status, diff score, and diff image path.
    """
    img = Image.open(image_path).convert("RGBA")
    base = Image.open(baseline_path).convert("RGBA")

    if img.size != base.size:
        return {
            "name": image_path.name,
            "status": "size_mismatch",
            "diff_score": -1,
            "image_size": list(img.size),
            "baseline_size": list(base.size),
        }

    w, h = img.size
    a = img.tobytes()
    b = base.tobytes()
    pixel_count = w * h
    byte_count = len(a)
    score = _mean_abs_diff(a, b, byte_count)

    # Build diff image: red channel = absolute diff, G/B from current image
    diff_rgba = bytearray(byte_count)
    for i in range(0, byte_count, 4):
        for c in range(3):
            diff_rgba[i + c] = abs(a[i + c] - b[i + c])
        diff_rgba[i + 3] = 255  # alpha

    diff_img = Image.frombytes("RGBA", (w, h), bytes(diff_rgba))
    diff_img.save(diff_dir / image_path.name)

    return {
        "name": image_path.name,
        "status": "match" if score == 0 else "diff",
        "diff_score": round(score, 4),
        "pixels": pixel_count,
    }


def update_baseline():
    """Copy current screenshots into the baseline directory."""
    if not SCREENSHOTS_DIR.exists():
        print(f"No screenshots directory at {SCREENSHOTS_DIR}", file=sys.stderr)
        sys.exit(1)
    BASELINE_DIR.mkdir(exist_ok=True)
    for f in sorted(SCREENSHOTS_DIR.glob("*.png")):
        shutil.copy2(f, BASELINE_DIR / f.name)
    print(f"Baseline updated from {SCREENSHOTS_DIR} → {BASELINE_DIR}")


def run_diff(threshold: float = 1.0):
    Image = _ensure_pillow()

    if not SCREENSHOTS_DIR.exists():
        print(f"No screenshots directory at {SCREENSHOTS_DIR}", file=sys.stderr)
        sys.exit(1)
    if not BASELINE_DIR.exists():
        print(
            f"No baseline directory at {BASELINE_DIR}.\n"
            f"Run with --update-baseline first to seed the golden images.",
            file=sys.stderr,
        )
        sys.exit(1)

    DIFF_DIR.mkdir(exist_ok=True)
    results = []
    any_fail = False
    missing = []

    current_images = sorted(SCREENSHOTS_DIR.glob("*.png"))
    baseline_images = {p.name for p in BASELINE_DIR.glob("*.png")}

    for img_path in current_images:
        baseline_path = BASELINE_DIR / img_path.name
        if not baseline_path.exists():
            missing.append(img_path.name)
            results.append({"name": img_path.name, "status": "missing_baseline"})
            any_fail = True
            continue

        r = compare_images(img_path, baseline_path, DIFF_DIR, Image)
        results.append(r)
        status_icon = "✓" if r["status"] == "match" else "✗"
        score_str = f'{r.get("diff_score", "?"):.4f}' if r.get("diff_score", 0) >= 0 else "N/A"
        print(f"  {status_icon} {r['name']:40s}  score={score_str}")
        if r["status"] != "match" and r.get("diff_score", 0) > threshold:
            any_fail = True

    # Check for baselines with no current image
    current_names = {p.name for p in current_images}
    for name in sorted(baseline_images - current_names):
        results.append({"name": name, "status": "missing_current"})

    report = {
        "threshold": threshold,
        "total": len(results),
        "matched": sum(1 for r in results if r["status"] == "match"),
        "diff": sum(1 for r in results if r["status"] == "diff"),
        "missing_baseline": len(missing),
        "results": results,
    }
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n")

    print(f"\nReport: {REPORT_PATH}")
    print(f"Diff images: {DIFF_DIR}/")
    if any_fail:
        print("Result: FAIL — visual differences detected.")
        sys.exit(1)
    else:
        print("Result: PASS — all screenshots match baseline.")


def main():
    parser = argparse.ArgumentParser(description="Screenshot visual regression diff")
    parser.add_argument(
        "--update-baseline",
        action="store_true",
        help="Copy current screenshots/ to screenshots_baseline/",
    )
    parser.add_argument(
        "--threshold",
        type=float,
        default=1.0,
        help="Mean absolute pixel diff threshold (0-255) to consider a failure (default: 1.0)",
    )
    args = parser.parse_args()

    if args.update_baseline:
        update_baseline()
    else:
        run_diff(threshold=args.threshold)


if __name__ == "__main__":
    main()
