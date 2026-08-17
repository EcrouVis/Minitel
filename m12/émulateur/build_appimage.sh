#!/bin/sh
mkdir build
cmake -DUSE_TLS=On -DUSE_OPEN_SSL=On -DCMAKE_BUILD_TYPE=Release -DUSE_SSE2=On -DUSE_AVX2=On -DAPPIMAGE_BUILD=On -G Ninja -B build
cmake --build build --clean-first
cpack --config ./build/CPackConfig.cmake -G AppImage