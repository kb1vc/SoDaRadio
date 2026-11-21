#!/bin/bash
# Launch SoDaRadio (or another SoDa binary) from the Ubuntu runtime
# container, with the bind mounts and env needed for USB/ethernet
# hardware, X11 display, and PulseAudio audio.  The Ubuntu release is
# selectable; the image tag is version-scoped so multiple versions coexist.
#
# Usage: run-sodaradio.sh [COMMAND [ARGS...]]
#   COMMAND  binary to run inside the container  (default: SoDaRadio)
#            try: SoDaServer, SoDaCreateConfig, /bin/bash
#
# Environment variables (optional):
#   UBUNTU_VERSION     Ubuntu release to launch.  Default: newest entry in
#                      ../versions.conf.  Must match the version
#                      build-container.sh built the image against (the
#                      default image tag includes the version).
#   IMAGE              charliecloud image tag.  Default:
#                      sodaradio-ubuntu-${UBUNTU_VERSION}-run
#   CH_IMAGE_STORAGE   charliecloud storage dir.  Default: /var/tmp/$USER.ch
#
# Hardware access notes:
#   USRP B/N/X-series over ETHERNET works implicitly: charliecloud shares
#     the host network namespace, so UHD sees the LAN directly.
#   USB devices (Pluto, RTL-SDR, USRP B2x0) work only if the host user
#     already has read/write access to the device node under /dev/bus/usb.
#     That means the host must have the right udev rules installed (the
#     libiio, librtlsdr, and/or UHD packages all drop these in
#     /etc/udev/rules.d/).  Charliecloud is unprivileged and cannot grant
#     permissions you don't already have.
#
# GUI / audio notes:
#   Under X11 this script binds /tmp/.X11-unix and forwards DISPLAY.
#   Under Wayland, QT_QPA_PLATFORM is forced to xcb so SoDaRadio uses
#   XWayland rather than trying to connect to the Wayland socket (which
#   would need a different bind mount).
#   PulseAudio and PipeWire's Pulse-compat socket both live at the same
#   path -- $XDG_RUNTIME_DIR/pulse/native -- so one mount covers both.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Single source of truth for supported distro versions.
# shellcheck source=../versions.conf
source "$SCRIPT_DIR/../versions.conf"

# Default to the newest Ubuntu in the list; allow override via
# UBUNTU_VERSION=... on the command line.
UBUNTU_VERSION="${UBUNTU_VERSION:-${UBUNTU_VERSIONS##* }}"

IMAGE="${IMAGE:-sodaradio-ubuntu-${UBUNTU_VERSION}-run}"
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"
XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/run/user/$UID}"

CMD="${1:-SoDaRadio}"
shift || true

# Build up the ch-run argument list in stages so we can conditionally add
# bind mounts/env based on what actually exists on the host.
mounts=()
envs=()

# --- USB bus ------------------------------------------------------------
# Covers Pluto, RTL-SDR, and USRP B2x0.  Harmless if no devices are plugged
# in -- just an empty directory.
if [ -d /dev/bus/usb ]; then
    mounts+=(-b /dev/bus/usb:/dev/bus/usb)
else
    echo "Warning: /dev/bus/usb does not exist on this host; USB SDRs will not be visible." >&2
fi

# --- X11 display --------------------------------------------------------
if [ -d /tmp/.X11-unix ]; then
    mounts+=(-b /tmp/.X11-unix:/tmp/.X11-unix)
    envs+=(--set-env="DISPLAY=${DISPLAY:-:0}")
    # Force the xcb Qt platform plugin.  Under Wayland sessions this uses
    # XWayland; under native X11 it is the only sensible choice anyway.
    envs+=(--set-env="QT_QPA_PLATFORM=xcb")
else
    echo "Warning: /tmp/.X11-unix not found; the GUI will not be able to connect." >&2
fi

# --- XDG runtime dir (audio + Wayland + dbus + mutter-xauth) ------------
# Bind the whole per-user runtime dir rather than just $XDG_RUNTIME_DIR/pulse.
# A single mount covers the PulseAudio socket, PipeWire's Pulse-compat
# socket, any Wayland socket, and the Xauthority files that GNOME/mutter
# drop under /run/user/<uid>/ on Wayland sessions.  The Dockerfile creates
# /run/user/<uid> at image-build time via the USER_UID build arg so this
# destination always exists.
if [ -d "$XDG_RUNTIME_DIR" ]; then
    mounts+=(-b "$XDG_RUNTIME_DIR":"$XDG_RUNTIME_DIR")
    envs+=(--set-env="XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR")
    if [ -S "$XDG_RUNTIME_DIR/pulse/native" ]; then
        envs+=(--set-env="PULSE_SERVER=unix:$XDG_RUNTIME_DIR/pulse/native")
    else
        echo "Warning: no PulseAudio socket at $XDG_RUNTIME_DIR/pulse/native; audio will be silent." >&2
    fi
else
    echo "Warning: $XDG_RUNTIME_DIR does not exist on this host; audio and Wayland will be unavailable." >&2
fi

# Xauthority -- just pass the env var through.  The credential file
# itself is reachable via one of the mounts we already have: $HOME covers
# ~/.Xauthority, and $XDG_RUNTIME_DIR covers the Wayland/mutter-generated
# variants under /run/user/<uid>/.
if [ -n "${XAUTHORITY:-}" ] && [ -f "$XAUTHORITY" ]; then
    envs+=(--set-env="XAUTHORITY=$XAUTHORITY")
fi

# --- Home directory -----------------------------------------------------
# SoDaRadio keeps its config under ~/.config/SoDaRadio.  Bind the whole
# $HOME so settings persist across runs and so file dialogs show the
# user's real tree.  This is the usual charliecloud workstation idiom.
if [ -d "${HOME:-}" ]; then
    mounts+=(-b "$HOME:$HOME")
    envs+=(--set-env="HOME=$HOME")
fi

# --- Launch -------------------------------------------------------------
echo "=== Launching '$CMD' from $IMAGE ==="
exec ch-run -s "$IMG_STORAGE" \
    "${mounts[@]}" \
    "${envs[@]}" \
    "$IMAGE" -- \
    "$CMD" "$@"
