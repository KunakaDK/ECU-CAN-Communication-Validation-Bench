#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT_DIR/build" --parallel
ctest --test-dir "$ROOT_DIR/build" --output-on-failure
"$ROOT_DIR/build/ecu_can_bench" --demo
