#!/bin/bash
set -e

sudo dnf -y update

sudo dnf -y install gcc gcc-c++ make cmake git

sudo dnf -y install qt5-qtbase-devel qt5-qtdeclarative-devel \
    qt5-qtquickcontrols2-devel qt5-qttools-devel

cd ~/media-downloader
rm -rf build
mkdir build
cd build
cmake .. -DBUILD_TESTING=ON

cmake --build . -j"$(nproc)"
ctest --output-on-failure

if [ "${RUN_AFTER_BUILD:-0}" = "1" ]; then
    chmod +x media-downloader
    ./media-downloader
fi
