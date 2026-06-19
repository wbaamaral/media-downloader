#!/bin/bash
set -e

sudo pacman -Syu --needed --noconfirm --noprogressbar \
    base-devel cmake git \
    qt5-base qt5-declarative qt5-quickcontrols2 qt5-tools

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
