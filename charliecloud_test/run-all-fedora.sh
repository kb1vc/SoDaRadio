#!/bin/bash
# Build and install-test SoDaRadio on every Fedora version listed in
# versions.conf.  Stops at the first failure.
#
# Usage: run-all-fedora.sh [OUTPUT_DIR]
#   OUTPUT_DIR  passed through to build-fedora.sh  (default: ./packages)
#
# Environment variables (optional overrides, forwarded as-is):
#   REPO_URL, BRANCH   — see build-fedora.sh
#
# Each iteration runs build-fedora.sh then test-install-fedora.sh with the
# same FEDORA_VERSION.  Because test-install-fedora.sh picks the newest
# .rpm in packages/ by default, and we build strictly in sequence, the
# test always runs against the RPM we just produced.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-$SCRIPT_DIR/packages}"

# shellcheck source=versions.conf
source "$SCRIPT_DIR/versions.conf"

for v in $FEDORA_VERSIONS; do
    echo
    echo "############################################################"
    echo "###  Fedora ${v} — build + install test"
    echo "############################################################"
    FEDORA_VERSION="$v" "$SCRIPT_DIR/build-fedora.sh" "$OUTPUT_DIR"
    FEDORA_VERSION="$v" "$SCRIPT_DIR/test-install-fedora.sh"
done

echo
echo "All Fedora versions passed: $FEDORA_VERSIONS"
