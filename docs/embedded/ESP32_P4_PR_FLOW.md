# ESP32-P4 branch and PR flow

## Branch model

Embedded platform work for ESP32-P4 lives on platform branch:

- base branch: `esp32p4`

Feature work for that platform should branch from `esp32p4`, not from `main`.

Recommended naming:

- `feat/92-esp32p4-platform-bringup`
- `fix/esp32p4-...`
- `docs/esp32p4-...`

## PR target

When opening a PR for P4 work, set:

- **base:** `esp32p4`
- **head:** feature branch for P4 work

## Why

This keeps:

- P4-specific BSP and IDF work isolated from `main`
- future S3 work isolated on its own platform branch
- embedded iteration moving without blocking host/UI-only work on `main`
