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

echo "=== [1/5] Building fresh Ubuntu base image ==="
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

echo "=== [2/5] Installing DEB into a new image layer ==="
ch-image -s "$IMG_STORAGE" build \
    -t "$INST_IMAGE" \
    -f "$TMPDF" \
    "$DEB_DIR"

echo "=== [3/5] Verifying binaries are installed ==="
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

# Catches the case where the DEB declared its dependencies incompletely and
# the package installed successfully but the binary still cannot resolve one
# of its shared libraries at load time.  ldd is the dynamic linker asking the
# same question the kernel will ask when the binary is executed.
echo "=== [4/5] Checking shared-library completeness (ldd) ==="
ch-run -s "$IMG_STORAGE" "$INST_IMAGE" -- sh -c '
    ok=true
    for bin in SoDaRadio SoDaServer SoDaCreateConfig; do
        path=$(command -v "$bin" || true)
        [ -z "$path" ] && continue
        missing=$(ldd "$path" 2>/dev/null | grep "not found" || true)
        if [ -n "$missing" ]; then
            echo "  FAIL: $bin missing libraries:" >&2
            echo "$missing" | sed "s/^/    /" >&2
            ok=false
        else
            echo "  OK: $bin -- all libraries resolve"
        fi
    done
    $ok
'

# Catches runtime load failures that ldd misses: files the binary looks up
# at startup (configs, data, dlopened plugins) that were not pulled in by
# the package declaration.  SoDaRadio is skipped because it is a Qt GUI and
# may require a display even for --help.  We only fail on the dynamic-linker
# error signature; many CLIs exit non-zero on --help and that is fine.
echo "=== [5/5] Smoke-exec (verify binaries load without library errors) ==="
ch-run -s "$IMG_STORAGE" "$INST_IMAGE" -- sh -c '
    ok=true
    for bin in SoDaServer SoDaCreateConfig; do
        out=$(timeout 5s "$bin" --help 2>&1 || true)
        if echo "$out" | grep -q "error while loading shared libraries\|cannot open shared object"; then
            echo "  FAIL: $bin reported a library-load error:" >&2
            echo "$out" | grep "error while loading shared libraries\|cannot open shared object" | head -3 | sed "s/^/    /" >&2
            ok=false
        else
            echo "  OK: $bin --help ran without library-load errors"
        fi
    done
    $ok
'

echo "=== DEB install test passed ==="
