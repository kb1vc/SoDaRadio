#!/bin/bash
# Build SoDaRadio on Ubuntu 24.04 inside a charliecloud container and extract the DEB.
#
# Usage: build-ubuntu.sh [OUTPUT_DIR]
#   OUTPUT_DIR  where to deposit the resulting .deb  (default: ./packages)
#
# Run from anywhere; the script locates the SoDaRadio source automatically.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"
IMAGE="sodaradio-ubuntu-build"

IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

echo "=== [1/3] Building Ubuntu build-environment image ==="
# Use charliecloud_test/ as the build context -- the Dockerfile has no COPY
# instructions, so the source tree does not need to be transferred.
ch-image -s "$IMG_STORAGE" build \
    -t "$IMAGE" \
    -f "$SCRIPT_DIR/ubuntu-build/Dockerfile" \
    "$SCRIPT_DIR"

BUILD_SCRATCH="$(mktemp -d /tmp/sodaradio-ubuntu-build.XXXXXX)"
trap 'rm -rf "$BUILD_SCRATCH"' EXIT

mkdir -p "$OUTPUT_DIR"

echo "=== [2/3] Building SoDaRadio and generating DEB ==="
ch-run -s "$IMG_STORAGE" \
    -b "$SRC_DIR:/src" \
    -b "$BUILD_SCRATCH:/build" \
    -b "$OUTPUT_DIR:/output" \
    "$IMAGE" -- \
    sh -c '
        set -e
        cd /build
        cmake /src \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_DEB=ON
        make -j$(nproc)
        cpack -G DEB
        cp *.deb /output/
    '

echo "=== [3/3] Results ==="
echo "DEB packages in $OUTPUT_DIR:"
ls -lh "$OUTPUT_DIR"/*.deb 2>/dev/null || echo "  (none found -- check build output above)"
