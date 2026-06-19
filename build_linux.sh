#!/bin/bash
set -e

sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y qtbase5-dev qtdeclarative5-dev \
    qml-module-qtquick-controls2 qttools5-dev qttools5-dev-tools

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
