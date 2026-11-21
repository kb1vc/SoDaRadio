#!/bin/bash
# Build the SoDaRadio runtime container from a prebuilt .deb package.
# The Ubuntu release is selectable; everything picks the same version.
#
# Usage: build-container.sh [DEB_FILE]
#   DEB_FILE   path to the .deb to install into the image.
#              Default: newest .deb under ../packages/ whose filename
#              contains "ubuntu-<UBUNTU_VERSION>".
#
# Environment variables (optional):
#   UBUNTU_VERSION     Ubuntu release to target.  Default: newest entry in
#                      ../versions.conf.  Must appear in UBUNTU_VERSIONS.
#                      The same value is passed as a build-arg into the
#                      Dockerfile (which does `FROM ubuntu:${UBUNTU_VERSION}`)
#                      and is woven into the image tag and the default .deb
#                      filename pattern so multiple versions can coexist on
#                      one host without colliding.
#   IMAGE              charliecloud image tag.  Default:
#                      sodaradio-ubuntu-${UBUNTU_VERSION}-run
#   CH_IMAGE_STORAGE   charliecloud storage dir.  Default: /var/tmp/$USER.ch
#
# A matching .deb is produced by:
#   cd .. && UBUNTU_VERSION=<version> ./build-ubuntu.sh
# or by the CI release workflow (downloaded from the GitHub Release page).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_DEB_DIR="$SCRIPT_DIR/../packages"

# Single source of truth for supported distro versions (shared with
# ../build-ubuntu.sh, ../test-install-ubuntu.sh, and release.yml).
# shellcheck source=../versions.conf
source "$SCRIPT_DIR/../versions.conf"

# Default to the newest Ubuntu in the list (last space-separated token);
# allow override via UBUNTU_VERSION=... on the command line.
UBUNTU_VERSION="${UBUNTU_VERSION:-${UBUNTU_VERSIONS##* }}"

IMAGE="${IMAGE:-sodaradio-ubuntu-${UBUNTU_VERSION}-run}"
IMG_STORAGE="${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}"

# Pick the .deb.  If the caller passed an explicit path, trust them.
# Otherwise look in packages/ for a file whose name contains
# "[Uu]buntu-<UBUNTU_VERSION>".  CMakeLists.txt runs CPACK against
# ${OS_RELEASE_NAME} which comes from lsb_release -is on hosts that have
# it (→ "Ubuntu", capital U) and from /etc/os-release ID= on hosts that
# don't (→ "ubuntu", lower-case), so the glob matches both to spare the
# user from debugging a case-sensitivity mismatch.
#
# If nothing matches the requested version, FAIL rather than silently
# falling back to some other version's .deb -- installing a mismatched
# .deb into a container of a different distro release produces hard-to-
# diagnose SONAME failures downstream.
if [[ $# -ge 1 ]]; then
    DEB_FILE="$1"
else
    shopt -s nullglob
    _candidates=("$DEFAULT_DEB_DIR"/*[Uu]buntu-${UBUNTU_VERSION}*.deb)
    shopt -u nullglob
    if (( ${#_candidates[@]} == 0 )); then
        echo "Error: no .deb file found for Ubuntu ${UBUNTU_VERSION} under $DEFAULT_DEB_DIR" >&2
        echo "Build one with: cd .. && UBUNTU_VERSION=${UBUNTU_VERSION} ./build-ubuntu.sh" >&2
        echo "Or pass the path explicitly: $0 /path/to/sodaradio_X.Y.Z.deb" >&2
        # List what we did find, so the user can tell at a glance which
        # versions they already have built.
        if ls -1 "$DEFAULT_DEB_DIR"/*.deb >/dev/null 2>&1; then
            echo "" >&2
            echo "Available .deb files in $DEFAULT_DEB_DIR :" >&2
            ls -1 "$DEFAULT_DEB_DIR"/*.deb | sed 's|^|  |' >&2
        fi
        exit 1
    fi
    # Pick the newest among the matches, in case there are several
    # patch versions of the same Ubuntu release.
    DEB_FILE="$(ls -t "${_candidates[@]}" | head -1)"
fi

# Sanity-check: if the caller passed an explicit path (or forced one via
# the fallback), does the filename at least mention the version we're
# targeting?  Warn loudly on a mismatch but let them proceed, in case
# they genuinely know what they're doing.
if [[ "$(basename "$DEB_FILE")" != *[Uu]buntu-${UBUNTU_VERSION}*.deb ]]; then
    echo "WARNING: $(basename "$DEB_FILE") does not look like an Ubuntu ${UBUNTU_VERSION} package." >&2
    echo "         Installing a .deb built for a different distro release will probably fail" >&2
    echo "         at the dpkg step with Qt / libuhd / libssl SONAME-not-found errors." >&2
fi

echo "Using DEB: $DEB_FILE"
echo "Target   : Ubuntu ${UBUNTU_VERSION}  (image tag: $IMAGE)"

# Build context: the Dockerfile uses a COPY, so we have to assemble a
# directory containing both the Dockerfile and the .deb (renamed to a
# known filename so the COPY doesn't need a glob).
BUILD_CTX="$(mktemp -d /tmp/sodaradio-runtime-ctx.XXXXXX)"
trap 'rm -rf "$BUILD_CTX"' EXIT
cp "$SCRIPT_DIR/Dockerfile" "$BUILD_CTX/Dockerfile"
cp "$DEB_FILE" "$BUILD_CTX/sodaradio.deb"

echo "=== Building charliecloud image $IMAGE ==="
# Bake the host user's UID and $HOME into the image so run-sodaradio.sh
# can bind-mount /run/user/<uid> and $HOME onto existing directories.
# Charliecloud images are read-only at ch-run time, so the mount targets
# have to exist at build time.  UBUNTU_VERSION picks the base image.
ch-image -s "$IMG_STORAGE" build \
    --build-arg "UBUNTU_VERSION=$UBUNTU_VERSION" \
    --build-arg "USER_UID=$(id -u)" \
    --build-arg "USER_HOME=$HOME" \
    -t "$IMAGE" \
    -f "$BUILD_CTX/Dockerfile" \
    "$BUILD_CTX"

echo
echo "=== Done ==="
echo "Image: $IMAGE  (storage: $IMG_STORAGE)"
echo "Run with: UBUNTU_VERSION=${UBUNTU_VERSION} $SCRIPT_DIR/run-sodaradio.sh"
