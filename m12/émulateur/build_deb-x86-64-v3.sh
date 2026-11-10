#!/bin/sh
mkdir build
cmake -DUSE_TLS=On -DUSE_OPEN_SSL=On -DCMAKE_BUILD_TYPE=Release -DAPPIMAGE_BUILD=On -DCPU_ARCHITECTURE="x86-64-v3" -G Ninja -B build
cmake --build build --clean-first
cpack --config ./build/CPackConfig.cmake -G DEB