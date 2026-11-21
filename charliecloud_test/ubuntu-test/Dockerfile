# SoDaRadio runtime container -- Ubuntu with the project installed from a
# prebuilt .deb package.  The Ubuntu release is selected at image build
# time via the UBUNTU_VERSION build-arg; build-container.sh fills that in
# from versions.conf (or from the UBUNTU_VERSION env var set by the user).
#
# This is a RUNTIME container; the .deb is produced elsewhere (either by
# charliecloud_test/build-ubuntu.sh locally, or by the GitHub release
# workflow) and then baked in here.  Use build-container.sh to build this
# image against a chosen .deb, and run-sodaradio.sh to launch SoDaRadio
# from the result.
#
# Hardware access at run time is arranged by run-sodaradio.sh, which binds:
#   /dev/bus/usb      so libusb (Pluto, RTL-SDR, USRP B2x0) can see devices
#   /tmp/.X11-unix    so Qt GUIs can connect to the host X server
#   /run/user/$UID    so PulseAudio/PipeWire audio works
#   $HOME             so ~/.config/SoDaRadio persists across runs
# The host network namespace is implicitly shared (charliecloud does not
# create network namespaces), so UHD/ethernet to a USRP N/X/B-series over
# the LAN just works -- no extra bind required.

ARG UBUNTU_VERSION=26.04
FROM ubuntu:${UBUNTU_VERSION}

ENV DEBIAN_FRONTEND=noninteractive

# The .deb filename is normalised to sodaradio.deb by build-container.sh
# before the build context is assembled, so this COPY needs no glob.
#
# NB: we deliberately drop this in /pkg, not /tmp.  charliecloud mounts a
# fresh private tmpfs over /tmp for every RUN step during `ch-image build`,
# which means a file placed in /tmp by COPY vanishes before the next RUN
# can see it.  Any non-/tmp path works; /pkg is a short, obviously-custom
# location that is unlikely to collide with anything the base image uses.
COPY sodaradio.deb /pkg/sodaradio.deb

# Install the package.  Use the dpkg + `apt-get install -f` two-step
# rather than `apt-get install /path/to/file.deb` because recent apt
# versions (observed on Ubuntu 26.04 "resolute") have gotten pickier
# about local .deb paths on the command line and reject them with
#   E: Unsupported file /pkg/sodaradio.deb given on commandline
# even when the file is a perfectly good .deb.  The two-step idiom has
# worked since the pre-apt era and ducks the issue entirely.
#
# Classical sequence:
#   1. dpkg-deb --info      -- show package metadata; fails loudly if the
#                              .deb is malformed, so the build log pins
#                              down a "bad file" problem immediately.
#   2. dpkg -i || apt -f    -- try the install; on dep-resolution failure
#                              only, fall through to `apt-get install -f`
#                              which resolves and installs the Depends.
#   3. dpkg -s sodaradio    -- verify the final package state is
#                              "install ok installed"; fails the build
#                              if dpkg silently gave up along the way.
# Without step 3 a hard failure in step 2 (e.g. a corrupt .deb) would
# cascade into "apt -f has nothing to fix" and a false-success build --
# exactly the trap we fell into previously.
RUN apt-get update && \
    echo "=== .deb metadata ===" && dpkg-deb --info /pkg/sodaradio.deb && \
    echo "=== installing SoDaRadio ===" && \
    (dpkg -i /pkg/sodaradio.deb || apt-get install -y -f --no-install-recommends) && \
    echo "=== verifying ===" && \
    dpkg -s sodaradio | grep '^Status: install ok installed' && \
    echo "=== installing WSJT-X (digital-mode companion) ===" && \
    apt-get install -y --no-install-recommends wsjtx && \
    echo "=== generating locale ===" && \
    apt-get install -y --no-install-recommends locales && \
    locale-gen en_US.UTF-8 && \
    rm -f /pkg/sodaradio.deb && \
    rm -rf /var/lib/apt/lists/*

# Pre-create the bind-mount points charliecloud requires to exist.
# Charliecloud cannot create mount targets inside a read-only image at
# ch-run time; they have to be baked in now.
#
# USER_UID and USER_HOME are supplied by build-container.sh from the host
# user's identity (`id -u` and $HOME).  They let us create /run/user/<uid>
# and /home/<user> ahead of time so the bind mounts for the PulseAudio
# socket and the user's home directory land on existing targets.
ARG USER_UID=1000
ARG USER_HOME=/home/user
RUN mkdir -p \
        /dev/bus/usb \
        /tmp/.X11-unix \
        /run/user/${USER_UID} \
        "${USER_HOME}"
