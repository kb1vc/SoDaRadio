#!/bin/bash
# Build SoDaRadio on Fedora inside a charliecloud container and extract the RPM.
#
# Usage: build-fedora.sh [OUTPUT_DIR]
#   OUTPUT_DIR  where to deposit the resulting .rpm  (default: ./packages)
#
# Run from anywhere; the script locates the SoDaRadio source automatically.
# The Fedora build environment image is rebuilt only when the Dockerfile changes
# (ch-image caches layers).  The actual cmake/make/cpack run always executes
# against the current source tree via a bind mount.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"
IMAGE="sodaradio-fedora-build"

# charliecloud's default storage is /var/tmp/$USER.ch; override with
# CH_IMAGE_STORAGE if needed.
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

echo "=== [1/3] Building Fedora build-environment image ==="
# Use charliecloud_test/ as the build context -- the Dockerfile has no COPY
# instructions, so the source tree does not need to be transferred.
ch-image -s "$IMG_STORAGE" build \
    -t "$IMAGE" \
    -f "$SCRIPT_DIR/fedora-build/Dockerfile" \
    "$SCRIPT_DIR"

# Scratch directories for the cmake build and for rpmbuild's /var/tmp.
# The charliecloud container root is read-only; rpmbuild writes to /var/tmp
# for its temp scripts, so we bind-mount a writable host tmpdir there.
BUILD_SCRATCH="$(mktemp -d /tmp/sodaradio-fedora-build.XXXXXX)"
VARTMP_SCRATCH="$(mktemp -d /tmp/sodaradio-fedora-vartmp.XXXXXX)"
trap 'rm -rf "$BUILD_SCRATCH" "$VARTMP_SCRATCH"' EXIT

mkdir -p "$OUTPUT_DIR"

echo "=== [2/3] Building SoDaRadio and generating RPM ==="
ch-run -s "$IMG_STORAGE" \
    -b "$SRC_DIR:/src" \
    -b "$BUILD_SCRATCH:/build" \
    -b "$OUTPUT_DIR:/output" \
    -b "$VARTMP_SCRATCH:/var/tmp" \
    "$IMAGE" -- \
    sh -c '
        set -e
        cd /build
        cmake /src \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_RPM=ON \
            -DBUILD_SODALIBS=ON \
            -DBUILD_FFTW=ON \
            -DBUILD_LIBIIO=ON \
            -DBUILD_LIBRTLSDR=ON
        make -j$(nproc)
        cpack -G RPM
        cp *.rpm /output/
    '

echo "=== [3/3] Results ==="
echo "RPM packages in $OUTPUT_DIR:"
ls -lh "$OUTPUT_DIR"/*.rpm 2>/dev/null || echo "  (none found -- check build output above)"
