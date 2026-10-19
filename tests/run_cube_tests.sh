#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Standalone Cube Tutor tests; no downloaded models or hardware required.
set -euo pipefail
cd "$(dirname "$0")/.."
cube_test_build=$(mktemp -d "${TMPDIR:-/tmp}/cube-tutor-tests.XXXXXX")
flags=(-g -O1 -Wall -Wextra)
if [[ "${SANITIZE:-0}" == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
includes=(
    -I source/hal/source/components/camera/source/alif/source
    -I source/hal/source/components/camera/source/alif/include
    -I source/app/use_case/alif_object_detection/include
)
count=0
for source in tests/cube_preview/*.c tests/cube_cross_hint_test.c tests/test_cube_*.c; do
    binary="$cube_test_build/$(basename "$source" .c)"
    "${CC:-cc}" -std=c11 "${flags[@]}" "${includes[@]}" "$source" -lm -o "$binary"
    "$binary"
    count=$((count + 1))
done
for rotation in 90 180 270; do
    binary="$cube_test_build/preview-$rotation"
    "${CC:-cc}" -std=c11 "${flags[@]}" "${includes[@]}" \
        -DCUBE_DISPLAY_ROTATION="$rotation" tests/cube_preview/test_cube_preview.c -lm -o "$binary"
    "$binary"
    count=$((count + 1))
done
for source in tests/cube_face*test.cpp; do
    binary="$cube_test_build/$(basename "$source" .cpp)"
    "${CXX:-c++}" -std=c++17 "${flags[@]}" "${includes[@]}" "$source" -o "$binary"
    "$binary"
    count=$((count + 1))
done
echo "PASS: $count Cube Tutor host executions (includes known-defect characterization)."
echo "Binaries retained in $cube_test_build"
