#!/bin/bash
# Build SoDaRadio on Ubuntu inside a charliecloud container and extract the DEB.
#
# Usage: build-ubuntu.sh [OUTPUT_DIR]
#   OUTPUT_DIR  where to deposit the resulting .deb  (default: ./packages)
#
# Environment variables (optional overrides):
#   UBUNTU_VERSION  Ubuntu release to build in.  Default: newest entry in
#                   versions.conf.  Must appear in UBUNTU_VERSIONS there.
#   REPO_URL        git URL to clone   (default: https://github.com/kb1vc/SoDaRadio.git)
#   BRANCH          branch/tag/sha    (default: dist-test)
#
# The source tree is NOT taken from the local working copy — the container
# clones the named branch from GitHub at build time so this script reproduces
# exactly what is on the remote.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"

# Single source of truth for supported distro versions.
# shellcheck source=versions.conf
source "$SCRIPT_DIR/versions.conf"

# Default to the newest Ubuntu in the list (last space-separated token).
UBUNTU_VERSION="${UBUNTU_VERSION:-${UBUNTU_VERSIONS##* }}"

# One cached image per Ubuntu release so different versions coexist.
IMAGE="sodaradio-ubuntu-build-${UBUNTU_VERSION}"

REPO_URL="${REPO_URL:-https://github.com/kb1vc/SoDaRadio.git}"
BRANCH="${BRANCH:-dist-test}"

IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

echo "=== [1/3] Building Ubuntu ${UBUNTU_VERSION} build-environment image ==="
# The Dockerfile has no COPY instructions, so the source tree does not need
# to be transferred — charliecloud_test/ is just the build context.  The
# UBUNTU_VERSION build-arg selects the base image tag.
ch-image -s "$IMG_STORAGE" build \
    --build-arg UBUNTU_VERSION="$UBUNTU_VERSION" \
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
