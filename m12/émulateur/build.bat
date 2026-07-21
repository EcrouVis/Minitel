mkdir build
cmake -DUSE_TLS=On -DUSE_LIBRE_SSL=On -DCMAKE_BUILD_TYPE=Release -G Ninja -B build
cmake --build build --clean-first
cpack --config ./build/CPackConfig.cmake -G ZIP