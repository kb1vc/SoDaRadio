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

echo "=== [1/5] Building fresh Fedora base image ==="
ch-image -s "$IMG_STORAGE" build -t "$BASE_IMAGE" -f "$TMPDF" /

# Step 2: Build a second image that installs the RPM.
# ch-image build has write access to layers; ch-run does not — so the
# package installation must happen here, not in ch-run.
cat >"$TMPDF" <<DOCKERFILE
FROM ${BASE_IMAGE}
COPY ${RPM_BASENAME} /pkgs/
RUN dnf -y install /pkgs/${RPM_BASENAME}
DOCKERFILE

echo "=== [2/5] Installing RPM into a new image layer ==="
ch-image -s "$IMG_STORAGE" build \
    -t "$INST_IMAGE" \
    -f "$TMPDF" \
    "$RPM_DIR"

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

# Catches the case where the RPM declared its dependencies incompletely and
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

echo "=== RPM install test passed ==="
