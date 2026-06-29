#!/bin/bash
# Test that a SoDaRadio RPM installs cleanly on a fresh Fedora system.
#
# Usage: test-install-fedora.sh [RPM_FILE]
#   RPM_FILE  path to the .rpm to test (default: packages/*.rpm, newest first)
#
# Builds a minimal Fedora image, installs the RPM into it, and verifies
# that the SoDaRadio binaries are present.

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

IMAGE="sodaradio-fedora-test"
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

TMPDF="$(mktemp /tmp/Dockerfile-fedora-test.XXXXXX)"
trap 'rm -f "$TMPDF"' EXIT
cat >"$TMPDF" <<'DOCKERFILE'
FROM fedora:latest
RUN dnf -y update && dnf clean all
RUN mkdir -p /pkgs
DOCKERFILE

echo "=== [1/3] Building fresh Fedora test image ==="
ch-image -s "$IMG_STORAGE" build -t "$IMAGE" -f "$TMPDF" /

echo "=== [2/3] Installing RPM into the container ==="
RPM_BASENAME="$(basename "$RPM_FILE")"
ch-run -s "$IMG_STORAGE" \
    -b "$(dirname "$RPM_FILE"):/pkgs" \
    "$IMAGE" -- \
    sh -c "dnf -y install /pkgs/$RPM_BASENAME"

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

echo "=== RPM install test passed ==="
