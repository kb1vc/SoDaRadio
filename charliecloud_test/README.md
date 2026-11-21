# charliecloud_test — build and smoke-test SoDaRadio packages

This directory contains the scripts, Dockerfiles, and config needed to
build SoDaRadio's binary RPM and DEB packages, verify that they install
cleanly on a fresh system, and (for Ubuntu) actually launch SoDaRadio
from inside the resulting container with full hardware/display/audio
access.

Everything runs under [Charliecloud](https://hpc.github.io/charliecloud/),
which is unprivileged (no root required on the host, no Docker daemon).
The GitHub Actions release workflow in `.github/workflows/release.yml`
does the same build steps in a CI runner — the scripts here let you
reproduce and debug them locally.

---

## Supported distros

`versions.conf` is the **single source of truth** for which releases
are in rotation.  Both the scripts in this directory and the CI
workflow read from it:

```
FEDORA_VERSIONS="44 45"
UBUNTU_VERSIONS="24.04 26.04"
```

When new distro releases come out and you're ready to support them,
edit those two lines, commit, and the scripts (and CI) start building
for them automatically.  The scripts default to the **last** entry in
each list; pass `FEDORA_VERSION=X` or `UBUNTU_VERSION=X.YY` to target
any other version from the list.

---

## Packaging workflow at a glance

```
+-------------------------+        +-----------------------+
| build-{fedora,ubuntu}.sh|        |  test-install-        |
|  (build .rpm / .deb)    | -----> |  {fedora,ubuntu}.sh   |
|                         |        |  (install + verify)   |
+-------------------------+        +-----------------------+
             |
             v
       packages/*.rpm, *.deb
             |
             v
+-------------------------------------------+
|  ubuntu-test/build-container.sh           |
|  ubuntu-test/run-sodaradio.sh             |
|  (actually run SoDaRadio from the .deb)   |
+-------------------------------------------+
```

The `run-all-{fedora,ubuntu}.sh` wrappers iterate every version listed
in `versions.conf`, running the build + install-test pair for each.
That's the "regression" entry point.

---

## Prerequisites

* **Charliecloud** installed on the host (`ch-image`, `ch-run`).  On
  Fedora: `dnf install charliecloud`.
* **Network access** from the host — the build containers clone
  SoDaRadio from GitHub and pull distro packages from the archive.
* About **8 GB** of scratch space under `/var/tmp/$USER.ch` (the
  default charliecloud image store; override with `CH_IMAGE_STORAGE`).

No other host packages are required — everything the build needs is
installed inside the containers.

---

## Building an RPM or DEB

### One version

```
# Default — newest version from versions.conf
./build-fedora.sh
./build-ubuntu.sh
```

### A specific version

```
FEDORA_VERSION=44    ./build-fedora.sh
UBUNTU_VERSION=24.04 ./build-ubuntu.sh
```

### Options that both build scripts share

| Env var          | Default                                      | What it does                                     |
| ---------------- | -------------------------------------------- | ------------------------------------------------ |
| `FEDORA_VERSION` | newest in `versions.conf`                    | pick a specific Fedora release to build in       |
| `UBUNTU_VERSION` | newest in `versions.conf`                    | pick a specific Ubuntu release to build in       |
| `REPO_URL`       | `https://github.com/kb1vc/SoDaRadio.git`     | upstream repo to clone inside the container      |
| `BRANCH`         | `dist-test`                                  | branch / tag / sha to build                      |

Each script:

1. Builds (or re-uses a cached) `sodaradio-{fedora,ubuntu}-build-<version>`
   charliecloud image that has all build-time deps installed.
2. Inside the container, clones the chosen branch of `REPO_URL`,
   configures with `BUILD_RPM` / `BUILD_DEB`, builds, and `cpack`s.
3. Drops the resulting `.rpm` / `.deb` into `packages/`.

The resulting filenames include the distro and version, e.g.
`SoDaRadio-12.6.3-1.x86_64.fedora-44.rpm` and
`sodaradio_12.6.3-1_amd64.Ubuntu-24.04.deb`, so builds for different
versions don't overwrite each other.

---

## Testing that a package installs cleanly

```
./test-install-fedora.sh            # picks the newest .rpm in packages/
./test-install-ubuntu.sh            # picks the newest .deb

# Specific file:
./test-install-fedora.sh packages/SoDaRadio-12.6.3-1.x86_64.fedora-44.rpm

# Different distro version (must match the file you're testing):
FEDORA_VERSION=44    ./test-install-fedora.sh
UBUNTU_VERSION=24.04 ./test-install-ubuntu.sh
```

Each script runs a five-step verification inside a **fresh minimal**
distro container (no extra deps pre-installed — only what the OS image
ships with, so the package's declared `Requires:`/`Depends:` has to
pull in everything the binaries need):

1. **Build a minimal base image** for the target distro.
2. **Install the package** using `dnf -y install <pkg>.rpm` or
   `apt-get install -y ./<pkg>.deb`.  This fails fast if the package
   declared a missing dependency.
3. **Verify the binaries landed** at `/usr/bin/{SoDaRadio,SoDaServer,
   SoDaCreateConfig}`.
4. **`ldd` completeness check** — walk each installed binary and fail
   if any shared library ends in `not found`.  This catches the case
   where CPack missed a SONAME dependency.
5. **Smoke-exec** — run `SoDaServer --help` and `SoDaCreateConfig
   --help` inside the container and fail if either produces a
   dynamic-linker error signature (`cannot open shared object`,
   `error while loading shared libraries`).

If any step fails the script exits non-zero and prints exactly which
binary and which missing library.

### Covering every supported version in one go

```
./run-all-fedora.sh
./run-all-ubuntu.sh
```

Each wrapper iterates every version in `versions.conf`, running
build-then-test for each.  Stops at the first failure so you see the
actual error in the log without having to scroll past later versions
that would have failed for the same reason.

---

## Running SoDaRadio from a Ubuntu container

The scripts under `ubuntu-test/` go one step further than
install-testing: they build a **runtime** container (Ubuntu + installed
SoDaRadio .deb + wsjtx + locales) and launch SoDaRadio from it with
proper hardware, display, and audio access.

This is the "would a user running the .deb on bare Ubuntu have working
hardware access?" end-to-end test.

### Build the runtime container

From `charliecloud_test/ubuntu-test/`:

```
./build-container.sh                        # newest Ubuntu from versions.conf
UBUNTU_VERSION=24.04 ./build-container.sh   # specific release
```

The script will:

1. Find a matching `.deb` under `../packages/` (filename must contain
   `[Uu]buntu-<UBUNTU_VERSION>` — case-insensitive).  Fails if no
   version-matching `.deb` exists, with a list of what you do have.
2. Spin up `ubuntu:<UBUNTU_VERSION>`, install the `.deb` (using the
   `dpkg -i` / `apt-get install -f` idiom because some recent apt
   versions reject local `.deb` paths), install `wsjtx` and `locales`,
   and generate `en_US.UTF-8`.
3. Bake the host user's UID and `$HOME` into the image as bind-mount
   targets (Charliecloud images are read-only at `ch-run` time; the
   mount destinations must exist up front).
4. Tag the result as `sodaradio-ubuntu-<UBUNTU_VERSION>-run` so
   multiple distro-version runtime images coexist.

### Launch SoDaRadio (or any other SoDa binary)

```
./run-sodaradio.sh                           # default: SoDaRadio, newest Ubuntu
./run-sodaradio.sh SoDaServer --help         # any binary with args
./run-sodaradio.sh SoDaCreateConfig USRP     # one-shot helper
./run-sodaradio.sh /bin/bash                 # interactive shell inside the container

UBUNTU_VERSION=24.04 ./run-sodaradio.sh      # launch the 24.04 image instead
```

The script sets up the following so the container feels native:

* **USB** — binds `/dev/bus/usb` so libusb (Pluto, RTL-SDR, USRP B2x0)
  can see devices.  The *host* user must already have read/write
  access to the USB device node — Charliecloud is unprivileged and
  can't grant permissions.  Install the vendor udev rules
  (`libiio-utils`, `rtl-sdr`, UHD packages drop them in
  `/etc/udev/rules.d/`) on the host.
* **Ethernet** — no mount needed.  Charliecloud shares the host network
  namespace, so UHD-over-Ethernet to a USRP B/N/X-series on the LAN
  just works.
* **X11** — binds `/tmp/.X11-unix` and forwards `$DISPLAY`.  Forces
  `QT_QPA_PLATFORM=xcb` so Wayland sessions use XWayland (which is
  simpler than binding the Wayland socket).
* **Audio** — binds all of `$XDG_RUNTIME_DIR` so PulseAudio, PipeWire's
  Pulse-compat socket, and any Wayland/mutter-generated `Xauthority`
  files come through in one mount.  Sets `PULSE_SERVER` when the
  Pulse socket is present.
* **Home directory** — binds `$HOME` so SoDaRadio's config under
  `~/.config/kb1vc.org/` persists across runs and file dialogs show
  the user's real tree.

### Known gotcha: run from a writable directory

SoDaRadio's default log file is a relative path (`SoDa.soda_log`),
which resolves against the container's CWD.  If you run the launcher
from inside a read-only directory the Qt message handler pops up
"Unable to open file for writing".  Workaround: `cd` to `$HOME` or
somewhere writable before running `./run-sodaradio.sh`.

### Known gotcha: wsjtx on Ubuntu 24.04

Ubuntu 24.04 ships wsjtx 2.7.0-rc3, which can't talk to SoDaRadio's
Hamlib emulator (SoDaRadio responds with the old protocol-version-0
`dump_state` and 2.7.0-rc3's bundled Hamlib doesn't accept it).
WSJT-X 3.0+ (Ubuntu 26.04, Fedora 44+) works fine.  If you need wsjtx
on 24.04, grab the upstream `.deb` from
[wsjtx.org](https://wsjt.sourceforge.io/) and install it over the
archive version.

---

## Debugging

### "The image I just rebuilt still has the old behaviour"

Charliecloud images are stored on a flat filesystem per image name.
Modifying a `RUN` step and rebuilding sometimes leaves state from the
previous image in place.  The hammer:

```
ch-image -s "${CH_IMAGE_STORAGE:-/var/tmp/$USER.ch}" delete <image-tag>
./build-container.sh   # or build-fedora.sh / build-ubuntu.sh
```

Image tags you'll see: `sodaradio-fedora-build-<n>`,
`sodaradio-ubuntu-build-<n.nn>`, `sodaradio-fedora-test-<n>`,
`sodaradio-ubuntu-test-<n.nn>`, `sodaradio-ubuntu-<n.nn>-run`.

### "What port is SoDaRadio listening on inside the container?"

Charliecloud shares the host network namespace, so a plain `ss` on the
**host** sees sockets created inside the container:

```
ss -tlnp | grep 4575      # default Hamlib server port
```

### "What does the .deb actually contain?"

```
dpkg-deb -c packages/sodaradio_*.deb | grep -E '/s?bin/'
dpkg-deb --info packages/sodaradio_*.deb
```

Same for RPM:

```
rpm -qlp packages/SoDaRadio-*.rpm | grep -E '/s?bin/'
rpm -qip packages/SoDaRadio-*.rpm
```

---

## Files

| Path                                    | What it does                                                |
| --------------------------------------- | ----------------------------------------------------------- |
| `versions.conf`                         | single source of truth for supported distro versions        |
| `build-fedora.sh`, `build-ubuntu.sh`    | build a .rpm or .deb for one version                        |
| `test-install-fedora.sh`                | install a .rpm into a fresh Fedora and verify binaries work |
| `test-install-ubuntu.sh`                | same, for .deb on Ubuntu                                    |
| `run-all-fedora.sh`, `run-all-ubuntu.sh`| build + install-test every version in `versions.conf`       |
| `fedora-build/Dockerfile`               | build-env container for Fedora (parameterized by ARG)       |
| `ubuntu-build/Dockerfile`               | build-env container for Ubuntu (parameterized by ARG)       |
| `packages/`                             | output directory for generated .rpm / .deb                  |
| `ubuntu-test/Dockerfile`                | runtime container with SoDaRadio + wsjtx installed          |
| `ubuntu-test/build-container.sh`        | build the runtime container from a chosen .deb              |
| `ubuntu-test/run-sodaradio.sh`          | launch a SoDa binary from the runtime container             |

The `*.log` files in this tree are from previous runs — nothing in the
scripts creates them; they're captured by the user with `./x.sh 2>&1
| tee x.log`.  Safe to delete.
