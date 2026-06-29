# SoDaRadio Ubuntu 24.04 runtime container.
#
# Installs SoDaRadio from a pre-built .deb (produced by build-ubuntu.sh) into
# a clean Ubuntu 24.04 image.  All runtime dependencies (libuhd, qt6,
# hamlib, ...) are pulled in by apt from the .deb's Depends: field.
#
# The Dockerfile expects 'sodaradio.deb' in the build context.  run-sodaradio.sh
# stages the latest .deb under that name in a temp directory and uses it as
# the build context.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Stage the .deb outside /tmp: charliecloud bind-mounts the host's /tmp over
# the container's /tmp during ch-image build RUN steps, which would hide the
# file we just COPYed there.
COPY sodaradio.deb /var/cache/sodaradio.deb

# uhd-host is not pulled in by shlibdeps but is needed at runtime for USRP
# firmware and the device-discovery helpers.
#
# dpkg -i is used instead of `apt-get install /var/cache/sodaradio.deb` because
# apt only recognises local .deb files named in the canonical name_version_arch
# format; we install via dpkg and let apt-get install -f pull in the .deb's
# Depends: afterwards.
RUN apt-get update && \
    apt-get install -y ca-certificates uhd-host && \
    (dpkg -i /var/cache/sodaradio.deb || true) && \
    apt-get install -y -f && \
    rm -rf /var/lib/apt/lists/* /var/cache/sodaradio.deb

# Pre-create bind-mount targets.  ch-run is read-only at run time and
# requires every destination passed via -b to exist in the image.
# run-sodaradio.sh binds the host's /dev wholesale (USRP, RTL-SDR, Pluto,
# hamlib tty, GPS), the X11/Wayland and audio sockets, and the user's
# SoDaRadio config dir.
#
# Stubbing individual /dev/tty* files here would fail: ch-image build also
# bind-mounts the host /dev over the container /dev during RUN, so any
# touch in /dev would hit the host's real device nodes.
RUN mkdir -p /run/user /tmp/.X11-unix

WORKDIR /home
