# SoDaRadio Ubuntu 24.04 build environment.
#
# Provides all build-time dependencies for SoDaRadio on Ubuntu 24.04 LTS,
# including a pre-built SoDaLibs installation.
# The source tree is NOT baked in -- it is bind-mounted at build time by
# build-ubuntu.sh so the same image can be reused across source changes.
#
# Build this image from the SoDaRadio source root:
#   ch-image build -t sodaradio-ubuntu-build \
#             -f charliecloud_test/ubuntu-build/Dockerfile .

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        cmake \
        make \
        gcc \
        g++ \
        git \
        pkg-config \
        libuhd-dev \
        libboost-dev \
        libfftw3-dev \
        libhamlib-dev \
        libgps-dev \
        libiio-dev \
        librtlsdr-dev \
        qt6-base-dev \
        qt6-multimedia-dev \
        libcups2-dev \
        dpkg-dev \
        fakeroot \
        file \
        libssl-dev \
        libreadline-dev && \
    rm -rf /var/lib/apt/lists/*

# Qwt 6.x with Qt6 support is not packaged in Ubuntu 24.04; build from source
# as a static library so we don't carry a runtime libqwt dependency in the .deb.
# qt6-base-dev (above) provides qmake6.
RUN rm -rf /tmp/qwt && \
    git clone --depth 1 https://github.com/opencor/qwt.git /tmp/qwt && \
    cd /tmp/qwt && \
    sed -i 's/^QWT_CONFIG\s*+=\s*QwtSvg/#QWT_CONFIG += QwtSvg/' qwtconfig.pri && \
    sed -i 's/^QWT_CONFIG\s*+=\s*QwtDll/#QWT_CONFIG += QwtDll/' qwtconfig.pri && \
    qmake6 QWT_INSTALL_PREFIX=/usr/local QMAKE_CXXFLAGS+=-fPIC qwt.pro && \
    make -j$(nproc) && \
    make install && \
    QWT_INST=$(ls -d /usr/local/qwt-*) && \
    ln -sf "$QWT_INST/include" /usr/local/include/qwt && \
    ln -sf "$QWT_INST/lib/libqwt.a" /usr/local/lib/libqwt.a && \
    rm -rf /tmp/qwt

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
