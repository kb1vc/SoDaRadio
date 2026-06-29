# SoDaRadio Ubuntu 24.04 runtime container.
#
# Builds SoDaRadio from the source tree and installs it under /usr/local.
# The resulting image is used by run-sodaradio.sh to run SoDaRadio with
# host USB, serial, and display access.
#
# Build from the SoDaRadio source root (context = whole source tree):
#   ch-image build -t sodaradio-ubuntu \
#             -f charliecloud_test/ubuntu-runtime/Dockerfile .
#
# Run with device and display access:
#   charliecloud_test/run-sodaradio.sh [SoDaRadio args...]

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Build and runtime dependencies
RUN apt-get update && \
    apt-get install -y \
        cmake \
        make \
        gcc \
        g++ \
        git \
        pkg-config \
        libuhd-dev \
        uhd-host \
        libboost-dev \
        libfftw3-dev \
        libhamlib-dev \
        libgps-dev \
        libiio-dev \
        librtlsdr-dev \
        qt6-base-dev \
        qt6-multimedia-dev \
        libqwt-qt6-dev \
        libcups2-dev \
        libx11-6 \
        libxext6 \
        libxrender1 \
        libgl1 \
        libssl-dev \
        libreadline-dev && \
    rm -rf /var/lib/apt/lists/*

# Build and install SoDaLibs before copying the SoDaRadio source so this
# layer is cached independently of source changes.
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

# Copy source and build SoDaRadio
COPY . /src
RUN cmake -S /src -B /build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build /build --parallel $(nproc) && \
    cmake --install /build

WORKDIR /home
