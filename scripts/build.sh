#!/usr/bin/env bash
#
# build.sh - Configure and build HWVAL inside the stm32-dev container (CON-001)
#
# Author:  Sandesh Ghimire <sandesh@soccentric.com>
# Usage:   scripts/build.sh            configure (if needed) + build
#          scripts/build.sh clean      rm -rf build, then configure + build
#
# Outputs: build/hwval.elf, build/hwval.bin, build/hwval.hex, build/hwval.map
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEV="$SCRIPT_DIR/docker-dev.sh"

if [[ "${1:-}" == "clean" ]]; then
    rm -rf build
fi

"$DEV" run cmake -S /work -B /work/build -G Ninja \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
"$DEV" run cmake --build /work/build
"$DEV" run arm-none-eabi-size /work/build/hwval.elf