#!/bin/bash
# Build and install-test SoDaRadio on every Ubuntu version listed in
# versions.conf.  Stops at the first failure.
#
# Usage: run-all-ubuntu.sh [OUTPUT_DIR]
#   OUTPUT_DIR  passed through to build-ubuntu.sh  (default: ./packages)
#
# Environment variables (optional overrides, forwarded as-is):
#   REPO_URL, BRANCH   — see build-ubuntu.sh
#
# Each iteration runs build-ubuntu.sh then test-install-ubuntu.sh with the
# same UBUNTU_VERSION.  Because test-install-ubuntu.sh picks the newest
# .deb in packages/ by default, and we build strictly in sequence, the
# test always runs against the DEB we just produced.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"

# shellcheck source=versions.conf
source "$SCRIPT_DIR/versions.conf"

for v in $UBUNTU_VERSIONS; do
    echo
    echo "############################################################"
    echo "###  Ubuntu ${v} — build + install test"
    echo "############################################################"
    UBUNTU_VERSION="$v" "$SCRIPT_DIR/build-ubuntu.sh" "$OUTPUT_DIR"
    UBUNTU_VERSION="$v" "$SCRIPT_DIR/test-install-ubuntu.sh"
done

echo
echo "All Ubuntu versions passed: $UBUNTU_VERSIONS"
