#!/bin/bash
# Build SoDaRadio on Ubuntu 24.04 inside a charliecloud container and extract the DEB.
#
# Usage: build-ubuntu.sh [OUTPUT_DIR]
#   OUTPUT_DIR  where to deposit the resulting .deb  (default: ./packages)
#
# Environment variables (optional overrides):
#   REPO_URL    git URL to clone     (default: https://github.com/kb1vc/SoDaRadio.git)
#   BRANCH      branch/tag/sha       (default: dist-test)
#
# The source tree is NOT taken from the local working copy — the container
# clones the named branch from GitHub at build time so this script reproduces
# exactly what is on the remote.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"
IMAGE="sodaradio-ubuntu-build"

REPO_URL="${REPO_URL:-https://github.com/kb1vc/SoDaRadio.git}"
BRANCH="${BRANCH:-dist-test}"

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

echo "=== [2/3] Cloning $REPO_URL (branch $BRANCH) and building SoDaRadio ==="
# $REPO_URL and $BRANCH are expanded on the host and passed into the shell
# inside the container via env.  Keeping the inner shell single-quoted (and
# reading from env) avoids any quoting surprises if the URL ever contains
# characters the outer shell would reinterpret.
ch-run -s "$IMG_STORAGE" \
    --set-env="REPO_URL=$REPO_URL" \
    --set-env="BRANCH=$BRANCH" \
    -b "$BUILD_SCRATCH:/build" \
    -b "$OUTPUT_DIR:/output" \
    "$IMAGE" -- \
    sh -c '
        set -e
        echo "Cloning $REPO_URL (branch $BRANCH)..."
        git clone --depth 1 --branch "$BRANCH" "$REPO_URL" /build/source
        cd /build/source
        echo "HEAD is at: $(git log -1 --oneline)"

        mkdir -p /build/work
        cd /build/work
        cmake /build/source \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_DEB=ON \
            -DBUILD_SODALIBS=ON \
            -DBUILD_FFTW=ON \
            -DBUILD_LIBIIO=ON \
            -DBUILD_LIBRTLSDR=ON
        make -j$(nproc)
        cpack -G DEB
        cp *.deb /output/
    '

echo "=== [3/3] Results ==="
echo "DEB packages in $OUTPUT_DIR:"
ls -lh "$OUTPUT_DIR"/*.deb 2>/dev/null || echo "  (none found -- check build output above)"
