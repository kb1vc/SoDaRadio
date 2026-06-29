#!/bin/bash
# Test that a SoDaRadio RPM installs cleanly on a fresh Fedora system.
#
# Usage: test-install-fedora.sh [RPM_FILE]
#   RPM_FILE  path to the .rpm to test (default: packages/*.rpm, newest first)
#
# Builds a minimal Fedora image, installs the RPM into a second image layer
# (using ch-image build, which has write access), and verifies that the
# SoDaRadio binaries are present.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/packages"

if [[ $# -ge 1 ]]; then
    RPM_FILE="$1"
else
    RPM_FILE="$(ls -t "$PACKAGES_DIR"/*.rpm 2>/dev/null | head -1)"
fi
if [[ -z "$RPM_FILE" || ! -f "$RPM_FILE" ]]; then
    echo "Error: no RPM found. Run build-fedora.sh first, or pass the RPM path as an argument." >&2
    exit 1
fi
echo "Testing RPM: $RPM_FILE"

BASE_IMAGE="sodaradio-fedora-test"
INST_IMAGE="sodaradio-fedora-test-installed"
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

RPM_BASENAME="$(basename "$RPM_FILE")"
RPM_DIR="$(dirname "$RPM_FILE")"

TMPDF="$(mktemp /tmp/Dockerfile-fedora-test.XXXXXX)"
trap 'rm -f "$TMPDF"' EXIT

# Step 1: Build minimal base image (cached after first run).
cat >"$TMPDF" <<DOCKERFILE
FROM fedora:latest
RUN dnf -y update && dnf clean all
DOCKERFILE

echo "=== [1/3] Building fresh Fedora base image ==="
ch-image -s "$IMG_STORAGE" build -t "$BASE_IMAGE" -f "$TMPDF" /

# Step 2: Build a second image that installs the RPM.
# ch-image build has write access to layers; ch-run does not — so the
# package installation must happen here, not in ch-run.
cat >"$TMPDF" <<DOCKERFILE
FROM ${BASE_IMAGE}
COPY ${RPM_BASENAME} /pkgs/
RUN dnf -y install /pkgs/${RPM_BASENAME}
DOCKERFILE

echo "=== [2/3] Installing RPM into a new image layer ==="
ch-image -s "$IMG_STORAGE" build \
    -t "$INST_IMAGE" \
    -f "$TMPDF" \
    "$RPM_DIR"

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

echo "=== RPM install test passed ==="
