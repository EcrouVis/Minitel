cmake -DUSE_TLS=On -DCMAKE_BUILD_TYPE=Release -DUSE_SSE2=On -DUSE_AVX2=On -DPORTABLE_BUILD=Off -DUSE_WINMAIN=On -G Ninja -B build
cmake --build build --clean-first
cpack --config ./build/CPackConfig.cmake -G NSIS