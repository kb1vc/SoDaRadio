#!/bin/bash
# Test that a SoDaRadio DEB installs cleanly on a fresh Ubuntu 24.04 system.
#
# Usage: test-install-ubuntu.sh [DEB_FILE]
#   DEB_FILE  path to the .deb to test (default: packages/*.deb, newest first)
#
# Builds a minimal Ubuntu image, installs the DEB into it, and verifies
# that the SoDaRadio binaries are present.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/packages"

if [[ $# -ge 1 ]]; then
    DEB_FILE="$1"
else
    DEB_FILE="$(ls -t "$PACKAGES_DIR"/*.deb 2>/dev/null | head -1)"
fi
if [[ -z "$DEB_FILE" || ! -f "$DEB_FILE" ]]; then
    echo "Error: no DEB found. Run build-ubuntu.sh first, or pass the DEB path as an argument." >&2
    exit 1
fi
echo "Testing DEB: $DEB_FILE"

IMAGE="sodaradio-ubuntu-test"
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

TMPDF="$(mktemp /tmp/Dockerfile-ubuntu-test.XXXXXX)"
trap 'rm -f "$TMPDF"' EXIT
cat >"$TMPDF" <<'DOCKERFILE'
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        && rm -rf /var/lib/apt/lists/*
RUN mkdir -p /pkgs
DOCKERFILE

echo "=== [1/3] Building fresh Ubuntu test image ==="
ch-image -s "$IMG_STORAGE" build -t "$IMAGE" -f "$TMPDF" /

echo "=== [2/3] Installing DEB into the container ==="
DEB_BASENAME="$(basename "$DEB_FILE")"
ch-run -s "$IMG_STORAGE" \
    -b "$(dirname "$DEB_FILE"):/pkgs" \
    "$IMAGE" -- \
    sh -c "apt-get update && apt-get install -y /pkgs/$DEB_BASENAME"

echo "=== [3/3] Verifying installation ==="
ch-run -s "$IMG_STORAGE" "$IMAGE" -- sh -c '
    for bin in SoDaRadio SoDaServer SoDaCreateConfig; do
        found=
        for d in /usr/bin /usr/local/bin; do
            [ -x "$d/$bin" ] && found="$d/$bin"
        done
        if [ -n "$found" ]; then
            echo "  OK: $bin -> $found"
        else
            echo "  MISSING: $bin" >&2
        fi
    done
'

echo "=== DEB install test passed ==="
