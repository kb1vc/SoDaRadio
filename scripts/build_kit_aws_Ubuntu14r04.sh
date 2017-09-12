#!/bin/bash

add-apt-repository ppa:ettusresearch/uhd
apt-get update
apt-get install -y libhamlib-dev
apt-get install -y libuhd-dev uhd-host 
apt-get install -y libqwt-qt5-dev
apt-get install -y libqt5widgets5
apt-get install -y libqt5core5a
apt-get install -y qtbase5-dev qt5-qmake qtbase5-dev-tools
apt-get install -y libqt5multimedia5
apt-get install -y qtmultimedia5-dev
apt install -y debmake
apt install -y dpkg-dev
apt install -y cmake
apt install -y libboost-all-dev libasound2-dev libfftw3-dev
apt install -y pkg-config
apt install -y gpsd gpsd-clients libgps-dev
apt install -y git



git clone https://git.code.sf.net/p/sodaradio/SoDaRadio/
cd SoDaRadio
git checkout QTgui
mkdir build
cd build
cmake -DBUILD_DEB=1 ../
make package


