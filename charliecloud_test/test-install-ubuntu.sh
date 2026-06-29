#!/bin/bash
# Test that a SoDaRadio DEB installs cleanly on a fresh Ubuntu 24.04 system.
#
# Usage: test-install-ubuntu.sh [DEB_FILE]
#   DEB_FILE  path to the .deb to test (default: packages/*.deb, newest first)
#
# Builds a minimal Ubuntu image, installs the DEB into a second image layer
# (using ch-image build, which has write access), and verifies that the
# SoDaRadio binaries are present.

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

BASE_IMAGE="sodaradio-ubuntu-test"
INST_IMAGE="sodaradio-ubuntu-test-installed"
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

DEB_BASENAME="$(basename "$DEB_FILE")"
DEB_DIR="$(dirname "$DEB_FILE")"

TMPDF="$(mktemp /tmp/Dockerfile-ubuntu-test.XXXXXX)"
trap 'rm -f "$TMPDF"' EXIT

# Step 1: Build minimal base image (cached after first run).
cat >"$TMPDF" <<'DOCKERFILE'
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        && rm -rf /var/lib/apt/lists/*
DOCKERFILE

echo "=== [1/3] Building fresh Ubuntu base image ==="
ch-image -s "$IMG_STORAGE" build -t "$BASE_IMAGE" -f "$TMPDF" /

# Step 2: Build a second image that installs the DEB.
# ch-image build has write access to layers; ch-run does not — so the
# package installation must happen here, not in ch-run.
cat >"$TMPDF" <<DOCKERFILE
FROM ${BASE_IMAGE}
ENV DEBIAN_FRONTEND=noninteractive
COPY ${DEB_BASENAME} /pkgs/
RUN apt-get update && apt-get install -y /pkgs/${DEB_BASENAME} && rm -rf /var/lib/apt/lists/*
DOCKERFILE

echo "=== [2/3] Installing DEB into a new image layer ==="
ch-image -s "$IMG_STORAGE" build \
    -t "$INST_IMAGE" \
    -f "$TMPDF" \
    "$DEB_DIR"

echo "=== [3/3] Verifying installation ==="
ch-run -s "$IMG_STORAGE" "$INST_IMAGE" -- sh -c '
    ok=true
    for bin in SoDaRadio SoDaServer SoDaCreateConfig; do
        found=
        for d in /usr/bin /usr/local/bin; do
            [ -x "$d/$bin" ] && found="$d/$bin"
        done
        if [ -n "$found" ]; then
            echo "  OK: $bin -> $found"
        else
            echo "  MISSING: $bin" >&2
            ok=false
        fi
    done
    $ok
'

echo "=== DEB install test passed ==="
