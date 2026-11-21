# SoDaRadio Ubuntu build environment (version selected via UBUNTU_VERSION ARG).
#
# Provides the packaged build-time dependencies SoDaRadio can't fetch itself
# (Qt6, UHD, FFTW).  SoDaLibs, libiio, and librtlsdr are pulled from GitHub
# at cmake configure time via BUILD_SODALIBS / BUILD_LIBIIO / BUILD_LIBRTLSDR
# and built statically into SoDaRadio, so this image no longer needs -dev
# packages for any of them.  hamlib and gpsd are also gone -- hamlib was
# replaced by vendor/hamlib/rig.h and gpsd was dropped outright.
#
# Qwt is still built here from source: Ubuntu does not package a Qt6 build
# of Qwt in either 24.04 or 26.04, and the upstream Qwt project uses qmake
# rather than CMake so it doesn't fit the FetchContent pattern used for the
# other libs.
#
# The source tree is NOT baked in -- build-ubuntu.sh clones the chosen
# branch from GitHub into a scratch directory inside the container at run
# time, so the same image can be reused across source changes and across
# different branches without a rebuild.
#
# Build this image from the SoDaRadio source root:
#   ch-image build --build-arg UBUNTU_VERSION=26.04 \
#             -t sodaradio-ubuntu-build \
#             -f charliecloud_test/ubuntu-build/Dockerfile .
# The build-ubuntu.sh wrapper fills in UBUNTU_VERSION from versions.conf.

ARG UBUNTU_VERSION=24.04
FROM ubuntu:${UBUNTU_VERSION}

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
        qt6-base-dev \
        qt6-multimedia-dev \
        libcups2-dev \
        dpkg-dev \
        fakeroot \
        file \
        libssl-dev \
        libreadline-dev \
        libusb-1.0-0-dev \
        libxml2-dev \
        flex \
        bison && \
    rm -rf /var/lib/apt/lists/*

# Qwt 6.x with Qt6 support is not packaged in Ubuntu 24.04/26.04; build from
# source as a static library so we don't carry a runtime libqwt dependency in
# the .deb.  qt6-base-dev (above) provides qmake6.
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

# Pre-create bind-mount points (charliecloud requires them to exist).
# /build receives the host scratch dir (and holds the clone + build tree);
# /output receives the host output dir for the final .deb.
RUN mkdir -p /build /output
