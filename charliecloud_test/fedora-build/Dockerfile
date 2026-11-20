# SoDaRadio Fedora build environment.
#
# Provides the packaged build-time dependencies SoDaRadio can't fetch itself
# (Qt6, Qwt, UHD, FFTW).  SoDaLibs, libiio, and librtlsdr are pulled from
# GitHub at cmake configure time via BUILD_SODALIBS / BUILD_LIBIIO /
# BUILD_LIBRTLSDR and built statically into SoDaRadio, so this image no
# longer needs -devel packages for any of them.  hamlib and gpsd are also
# gone -- hamlib was replaced by vendor/hamlib/rig.h and gpsd was dropped
# outright.
#
# The source tree is NOT baked in -- build-fedora.sh clones the chosen
# branch from GitHub into a scratch directory inside the container at run
# time, so the same image can be reused across source changes and across
# different branches without a rebuild.
#
# Build this image from the SoDaRadio source root:
#   ch-image build -t sodaradio-fedora-build \
#             -f charliecloud_test/fedora-build/Dockerfile .

FROM fedora:43

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
        qt6-qtbase-devel \
        qt6-qtmultimedia-devel \
        qwt-qt6-devel \
        rpm-build \
        openssl-devel \
        readline-devel \
        libusbx-devel \
        libxml2-devel \
        flex \
        bison && \
    dnf clean all

# Pre-create bind-mount points (charliecloud requires them to exist).
# /build receives the host scratch dir (and holds the clone + build tree);
# /output receives the host output dir for the final .rpm.
RUN mkdir -p /build /output
