#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-$ROOT_DIR/build-micropython-host}"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DBUILD_HOST_DESKTOP=ON
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "Built MicroPython host link libs in: $BUILD_DIR"
