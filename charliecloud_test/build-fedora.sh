#!/bin/bash
# Build SoDaRadio on Fedora inside a charliecloud container and extract the RPM.
#
# Usage: build-fedora.sh [OUTPUT_DIR]
#   OUTPUT_DIR  where to deposit the resulting .rpm  (default: ./packages)
#
# Environment variables (optional overrides):
#   FEDORA_VERSION  Fedora release to build in.  Default: newest entry in
#                   versions.conf.  Must appear in FEDORA_VERSIONS there.
#   REPO_URL        git URL to clone   (default: https://github.com/kb1vc/SoDaRadio.git)
#   BRANCH          branch/tag/sha    (default: dist-test)
#
# The source tree is NOT taken from the local working copy — the container
# clones the named branch from GitHub at build time so this script reproduces
# exactly what is on the remote.  The Fedora build-environment image is
# rebuilt only when the Dockerfile or FEDORA_VERSION changes (ch-image caches
# layers per build-arg).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"

# Single source of truth for supported distro versions.
# shellcheck source=versions.conf
source "$SCRIPT_DIR/versions.conf"

# Default to the newest Fedora in the list (last space-separated token).
FEDORA_VERSION="${FEDORA_VERSION:-${FEDORA_VERSIONS##* }}"

# One cached image per Fedora release so different versions coexist.
IMAGE="sodaradio-fedora-build-${FEDORA_VERSION}"

REPO_URL="${REPO_URL:-https://github.com/kb1vc/SoDaRadio.git}"
BRANCH="${BRANCH:-dist-test}"

# charliecloud's default storage is /var/tmp/$USER.ch; override with
# CH_IMAGE_STORAGE if needed.
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

echo "=== [1/3] Building Fedora ${FEDORA_VERSION} build-environment image ==="
# The Dockerfile has no COPY instructions, so the source tree does not need
# to be transferred — charliecloud_test/ is just the build context.  The
# FEDORA_VERSION build-arg selects the base image tag.
ch-image -s "$IMG_STORAGE" build \
    --build-arg FEDORA_VERSION="$FEDORA_VERSION" \
    -t "$IMAGE" \
    -f "$SCRIPT_DIR/fedora-build/Dockerfile" \
    "$SCRIPT_DIR"

# Scratch directories for the git clone + cmake build and for rpmbuild's
# /var/tmp.  The charliecloud container root is read-only; rpmbuild writes to
# /var/tmp for its temp scripts, so we bind-mount a writable host tmpdir there.
BUILD_SCRATCH="$(mktemp -d /tmp/sodaradio-fedora-build.XXXXXX)"
VARTMP_SCRATCH="$(mktemp -d /tmp/sodaradio-fedora-vartmp.XXXXXX)"
trap 'rm -rf "$BUILD_SCRATCH" "$VARTMP_SCRATCH"' EXIT

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
    -b "$VARTMP_SCRATCH:/var/tmp" \
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
