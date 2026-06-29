# SoDaRadio Fedora build environment.
#
# Provides all build-time dependencies for SoDaRadio on current Fedora,
# including a pre-built SoDaLibs installation.
# The source tree is NOT baked in -- it is bind-mounted at build time by
# build-fedora.sh so the same image can be reused across source changes.
#
# Build this image from the SoDaRadio source root:
#   ch-image build -t sodaradio-fedora-build \
#             -f charliecloud_test/fedora-build/Dockerfile .

FROM fedora:latest

RUN dnf -y update && \
    dnf -y install \
        cmake \
        make \
        gcc-c++ \
        git \
        pkg-config \
        python3 \
        uhd-devel \
        boost-devel \
        fftw-devel \
        hamlib-devel \
        gpsd-devel \
        libiio-devel \
        rtl-sdr-devel \
        qt6-qtbase-devel \
        qt6-qtmultimedia-devel \
        qwt-qt6-devel \
        rpm-build \
        openssl-devel \
        readline-devel && \
    dnf clean all

# Build and install SoDaLibs (provides SoDaUtils, Signals, JsonCpp).
# Statically linked into SoDaRadio, so not needed at runtime.
RUN rm -rf /tmp/SoDaLibs /tmp/SoDaLibs-build && \
    git clone https://github.com/kb1vc/SoDaLibs.git /tmp/SoDaLibs && \
    mkdir -p /tmp/SoDaLibs/Utils/include/SoDa && \
    (cd /tmp/SoDaLibs/Utils/include && for h in *.hxx; do ln -sf "../$h" "SoDa/$h"; done) && \
    cmake -S /tmp/SoDaLibs -B /tmp/SoDaLibs-build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build /tmp/SoDaLibs-build --parallel $(nproc) && \
    cmake --install /tmp/SoDaLibs-build && \
    rm -rf /tmp/SoDaLibs /tmp/SoDaLibs-build

# Pre-create bind-mount points (charliecloud requires them to exist).
RUN mkdir -p /src /build /output
