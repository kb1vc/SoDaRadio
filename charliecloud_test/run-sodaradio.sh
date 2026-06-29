#!/bin/bash
# Run SoDaRadio from an Ubuntu 24.04 charliecloud container with host device access.
#
# Usage: run-sodaradio.sh [--rebuild] [SoDaRadio args...]
#   --rebuild   force a rebuild of the container image even if it exists
#
# ch-run inherits the calling shell's environment (DISPLAY, WAYLAND_DISPLAY,
# XDG_RUNTIME_DIR, PULSE_SERVER, etc.) automatically.  This script just
# bind-mounts the socket paths and device nodes that SoDaRadio needs.
#
# Hardware access provided:
#   USB devices    /dev/bus/usb      (USRP, RTL-SDR, Pluto)
#   Serial/tty     /dev/ttyUSB*  /dev/ttyACM*  /dev/ttyS0..1  (hamlib, GPS)
#   Sound          PulseAudio or PipeWire socket
#   Display        X11 socket or Wayland socket
#
# Your user account must already have permission to access the hardware on the
# host (e.g. membership in the 'plugdev', 'dialout', 'audio' groups).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/packages"
IMAGE="sodaradio-ubuntu"

IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

# Parse --rebuild flag before other args.
REBUILD=0
PASS_ARGS=()
for arg in "$@"; do
    if [[ "$arg" == "--rebuild" ]]; then
        REBUILD=1
    else
        PASS_ARGS+=("$arg")
    fi
done

# Build the runtime image if it doesn't exist or --rebuild was requested.
image_exists() {
    ch-image -s "$IMG_STORAGE" list 2>/dev/null | grep -qFx "$IMAGE"
}

if [[ "$REBUILD" -eq 1 ]] || ! image_exists; then
    DEB_FILE="$(ls -t "$PACKAGES_DIR"/*.deb 2>/dev/null | head -1)"
    if [[ -z "$DEB_FILE" || ! -f "$DEB_FILE" ]]; then
        echo "Error: no .deb found in $PACKAGES_DIR." >&2
        echo "       Run build-ubuntu.sh first." >&2
        exit 1
    fi
    DEB_BASENAME="$(basename "$DEB_FILE")"
    echo "=== Building SoDaRadio Ubuntu runtime container from $DEB_BASENAME ==="
    # Stage the .deb under a known name (the Dockerfile uses a fixed filename
    # because ch-image does not expand ARG references inside COPY).
    BUILD_CTX="$(mktemp -d /tmp/sodaradio-runtime-ctx.XXXXXX)"
    trap 'rm -rf "$BUILD_CTX"' EXIT
    cp "$DEB_FILE" "$BUILD_CTX/sodaradio.deb"
    ch-image -s "$IMG_STORAGE" build \
        -t "$IMAGE" \
        -f "$SCRIPT_DIR/ubuntu-runtime/Dockerfile" \
        "$BUILD_CTX"
    echo "=== Container image built ==="
fi

# Assemble bind mounts for hardware devices and sockets.
# ch-run inherits the host environment, so DISPLAY / WAYLAND_DISPLAY /
# XDG_RUNTIME_DIR are already set -- we only need to make the paths visible.
BINDS=()

# Bind the host's /dev wholesale.  We can't stub individual /dev/tty* files
# in the image (ch-image build bind-mounts the host /dev during RUN, so any
# touch under /dev hits real host device nodes), and host access for each
# device is still gated by group/ownership at run time.  This covers
# /dev/bus/usb (USRP, RTL-SDR, Pluto), /dev/ttyUSB*, /dev/ttyACM*, /dev/ttyS*
# (hamlib, GPS), and anything else SoDaRadio's backends may probe.
BINDS+=(-b /dev:/dev)

# Display sockets
[[ -d /tmp/.X11-unix ]] && BINDS+=(-b /tmp/.X11-unix:/tmp/.X11-unix)
if [[ -n "${WAYLAND_DISPLAY:-}" && -S "/run/user/$UID/$WAYLAND_DISPLAY" ]]; then
    BINDS+=(-b "/run/user/$UID/$WAYLAND_DISPLAY:/run/user/$UID/$WAYLAND_DISPLAY")
fi

# Audio sockets (PipeWire preferred, then PulseAudio)
if [[ -S "/run/user/$UID/pipewire-0" ]]; then
    BINDS+=(-b "/run/user/$UID/pipewire-0:/run/user/$UID/pipewire-0")
elif [[ -S "/run/user/$UID/pulse/native" ]]; then
    BINDS+=(-b "/run/user/$UID/pulse:/run/user/$UID/pulse")
fi

# Bind the user's SoDaRadio config dir so band settings persist across runs.
CONFIG_DIR="$HOME/.config/kb1vc.org"
mkdir -p "$CONFIG_DIR"
BINDS+=(-b "$CONFIG_DIR:$HOME/.config/kb1vc.org")

if [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]]; then
    echo "Warning: DISPLAY and WAYLAND_DISPLAY are both unset." >&2
    echo "         SoDaRadio requires a graphical display." >&2
fi

echo "=== Starting SoDaRadio in Ubuntu container ==="
exec ch-run -s "$IMG_STORAGE" \
    "${BINDS[@]}" \
    --home \
    "$IMAGE" -- \
    /usr/local/bin/SoDaRadio "${PASS_ARGS[@]+"${PASS_ARGS[@]}"}"
